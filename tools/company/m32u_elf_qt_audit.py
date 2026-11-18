#!/usr/bin/env python3
"""M32u: READ-ONLY offline static ELF / explicit Qt plugin audit.

Never runs untrusted ELF code: uses readelf (NOT ldd), hashes only a finite
manifest, and reports unresolved DT_NEEDED and runtime Qt discovery as
unverified. The caller's JSON and bundle location are NOT trusted authority.
No approval, production loader, process birth, environment mutation or scene.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import stat
import subprocess
import sys

SCHEMA = "post-apollo.company-maplibre-m32u-static-elf.v1"
MAX_ENTRIES = 24
MAX_BYTES = 512 * 1024 * 1024
ALLOWED_GROUPS = ("bin", "lib", "plugins")
SAFE_NAME = re.compile(r"[A-Za-z0-9_][A-Za-z0-9_.+-]{0,94}\Z")
HEX = re.compile(r"[a-f0-9]{64}\Z")
DYNAMIC = re.compile(r"\((NEEDED|SONAME|RPATH|RUNPATH)\)[^\n]*?\[([^\]\n]*)\]")
INTERP = re.compile(r"Requesting program interpreter:\s*([^\]\n]+)")
KNOWN_ORIGIN_PATHS = ("$ORIGIN", "$ORIGIN/../lib")
FIXED_EXE = "company-vector-probe"


class AuditFailure(Exception):
    pass


def name_ok(name: object) -> bool:
    return (isinstance(name, str) and SAFE_NAME.fullmatch(name) is not None
            and ".." not in name and not name.startswith("."))


def manifest_entries(manifest: object) -> list[dict]:
    if not isinstance(manifest, dict) or set(manifest) != {"schema", "entries"}:
        raise AuditFailure("INVALID_MANIFEST_SHAPE")
    if manifest["schema"] != SCHEMA:
        raise AuditFailure("UNKNOWN_MANIFEST_SCHEMA")
    entries = manifest["entries"]
    if not isinstance(entries, list) or not (3 <= len(entries) <= MAX_ENTRIES):
        raise AuditFailure("INVALID_ENTRY_COUNT")
    seen = set()
    groups = {g: [] for g in ALLOWED_GROUPS}
    for entry in entries:
        if not isinstance(entry, dict) or set(entry) != {"group", "name", "sha256"}:
            raise AuditFailure("INVALID_ENTRY_FIELDS")
        group, name, digest = entry["group"], entry["name"], entry["sha256"]
        if (group not in ALLOWED_GROUPS or not name_ok(name) or
                not isinstance(digest, str) or not HEX.fullmatch(digest) or
                digest == "0" * 64):
            raise AuditFailure("INVALID_ENTRY_VALUE")
        if (group, name) in seen:
            raise AuditFailure("DUPLICATE_ENTRY")
        seen.add((group, name))
        groups[group].append(name)
    if groups["bin"] != [FIXED_EXE] or not groups["lib"] or not groups["plugins"]:
        raise AuditFailure("INCOMPLETE_STATIC_FILE_DECLARATION")
    # There can be only one name for a SONAME. A loader must not silently
    # search two different directories for an identically named ELF.
    names = [entry["name"] for entry in entries]
    if len(set(names)) != len(names):
        raise AuditFailure("AMBIGUOUS_SONAME")
    return entries


def metadata(st: os.stat_result) -> tuple:
    return (st.st_dev, st.st_ino, st.st_uid, st.st_mode, st.st_nlink,
            st.st_size, st.st_mtime_ns, st.st_ctime_ns)


def check_directory(path: Path) -> None:
    st = path.lstat()
    if (not stat.S_ISDIR(st.st_mode) or st.st_uid != os.getuid()
            or st.st_mode & 0o022):
        raise AuditFailure("UNSAFE_DIRECTORY:" + path.name)


def read_hashed_file(path: Path, expected: str, is_executable: bool) -> None:
    old = path.lstat()
    if (not stat.S_ISREG(old.st_mode) or old.st_uid != os.getuid()
            or old.st_mode & 0o6022 or old.st_nlink != 1
            or old.st_size <= 0 or old.st_size > MAX_BYTES
            or (is_executable and not old.st_mode & stat.S_IXUSR)):
        raise AuditFailure("UNSAFE_FILE:" + path.name)
    fd = os.open(path, os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK | os.O_CLOEXEC)
    try:
        a = os.fstat(fd)
        if metadata(a) != metadata(old):
            raise AuditFailure("FILE_CHANGED:" + path.name)
        h = hashlib.sha256()
        while True:
            block = os.read(fd, 65536)
            if not block:
                break
            h.update(block)
        b = os.fstat(fd)
        c = path.lstat()
        if metadata(a) != metadata(b) or metadata(b) != metadata(c):
            raise AuditFailure("FILE_CHANGED:" + path.name)
        if h.hexdigest() != expected:
            raise AuditFailure("DIGEST_MISMATCH:" + path.name)
    finally:
        os.close(fd)


def inspect_elf(path: Path) -> dict:
    # readelf parses bytes. Never call ldd: it may invoke the dynamic linker.
    try:
        process = subprocess.run(
            ["readelf", "-W", "-h", "-l", "-d", "--", str(path)],
            check=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            text=True, timeout=8, env={"LC_ALL": "C"}, shell=False)
    except (OSError, subprocess.TimeoutExpired) as exc:
        raise AuditFailure("READELF_UNAVAILABLE:" + path.name) from exc
    if process.returncode or len(process.stdout) > 1_048_576:
        raise AuditFailure("INVALID_ELF:" + path.name)
    output = process.stdout
    cls = re.search(r"^\s*Class:\s*(ELF32|ELF64)\s*$", output, re.MULTILINE)
    machine = re.search(r"^\s*Machine:\s*([^\n]+)", output, re.MULTILINE)
    kind = re.search(r"^\s*Type:\s*(EXEC|DYN)\s", output, re.MULTILINE)
    if not cls or not machine or not kind:
        raise AuditFailure("UNKNOWN_ELF_IDENTITY:" + path.name)
    if "There is no dynamic section" in output:
        raise AuditFailure("NO_DYNAMIC_SECTION:" + path.name)
    records = DYNAMIC.findall(output)
    def tags(tag: str) -> list[str]:
        return [value for key, value in records if key == tag]
    sonames, rpaths, runpaths = tags("SONAME"), tags("RPATH"), tags("RUNPATH")
    if len(sonames) > 1 or len(rpaths) > 1 or len(runpaths) > 1:
        raise AuditFailure("AMBIGUOUS_DYNAMIC_TAGS:" + path.name)
    # Reject arbitrary search paths, empty path elements, absolute paths,
    # ld.so injection via embedded RPATH and all custom loader locations.
    if rpaths or any(item not in KNOWN_ORIGIN_PATHS
                     for p in runpaths for item in p.split(":")):
        raise AuditFailure("UNSAFE_RPATH:" + path.name)
    interpreter = INTERP.search(output)
    return {
        "elf_class": cls.group(1),
        "machine": machine.group(1).strip(),
        "type": kind.group(1),
        "needed": tags("NEEDED"),
        "soname": sonames[0] if sonames else None,
        "runpath": runpaths[0] if runpaths else None,
        "interpreter": interpreter.group(1).strip() if interpreter else None,
    }


def audit(bundle: Path, manifest: object) -> dict:
    # This is a negative-only discovery tool. Even a perfectly closed graph
    # of DT_NEEDED entries cannot prove Qt dynamic plugin discovery or
    # dynamically loaded/remote/system resources, nor approve launch.
    result = {
        "schema": SCHEMA, "status": "REFUSED",
        "listed_elf_dependencies_candidate": False,
        "qt_plugin_discovery_verified": False,
        "system_loader_verified": False,
        "immutable_image_verified": False,
        "operator_authenticated": False,
        "launch_authenticated": False,
        "can_spawn": False, "can_apply_scene": False,
        "can_accept_input": False, "can_focus_keyboard": False,
        "can_show_popup": False,
        "unresolved": [], "objects": [], "reason": None,
    }
    try:
        entries = manifest_entries(manifest)
        root = Path(bundle)
        check_directory(root)
        wanted = {g: set() for g in ALLOWED_GROUPS}
        for entry in entries:
            wanted[entry["group"]].add(entry["name"])
        for group in ALLOWED_GROUPS:
            folder = root / group
            check_directory(folder)
            # Unexpected unlisted libraries or Qt plugins are not ignored:
            # later Qt dlopen could load them without M32s hashes.
            if set(os.listdir(folder)) != wanted[group]:
                raise AuditFailure("UNLISTED_OR_MISSING_FILE:" + group)
        identities: set[tuple[str, str]] = set()
        for entry in entries:
            group, name = entry["group"], entry["name"]
            path = root / group / name
            read_hashed_file(path, entry["sha256"], group == "bin")
            data = inspect_elf(path)
            if group != "bin" and data["type"] != "DYN":
                raise AuditFailure("NON_LIBRARY_ELF:" + name)
            if data["soname"] and data["soname"] != name:
                raise AuditFailure("SONAME_MISMATCH:" + name)
            identities.add((data["elf_class"], data["machine"]))
            result["objects"].append({"group": group, "name": name, **data})
        if len(identities) != 1:
            raise AuditFailure("MIXED_ELF_ABI")
        bundled_names = wanted["lib"]
        for obj in result["objects"]:
            for needed in obj["needed"]:
                if needed not in bundled_names:
                    result["unresolved"].append({"origin": obj["name"], "needed": needed})
        if result["unresolved"]:
            result["reason"] = "UNRESOLVED_DT_NEEDED"
        else:
            result["status"] = "LISTED_ELF_NEEDED_CANDIDATE_ONLY"
            result["listed_elf_dependencies_candidate"] = True
            result["reason"] = "QT_DLOPEN_AND_SYSTEM_LOADER_UNVERIFIED"
    except (AuditFailure, OSError) as exc:
        result["reason"] = str(exc) if isinstance(exc, AuditFailure) else "FILESYSTEM_UNAVAILABLE"
    return result


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bundle", required=True, type=Path)
    parser.add_argument("--manifest", required=True, type=Path)
    args = parser.parse_args(argv)
    try:
        if args.manifest.stat().st_size > 32768:
            raise ValueError("manifest too large")
        manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    except (ValueError, OSError, UnicodeDecodeError) as exc:
        manifest = None
    report = audit(args.bundle, manifest)
    print(json.dumps(report, sort_keys=True, indent=2))
    return 0 if report["listed_elf_dependencies_candidate"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
