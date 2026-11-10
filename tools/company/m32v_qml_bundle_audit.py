#!/usr/bin/env python3
"""M32v // Read-only QtLocation/MapLibre QML bundle inventory, not launch approval.

Audits a *supplied exported candidate* against an explicit SHA-256 manifest.
No installation, mutation, dynamic loading, QML execution or process spawn.
A passing file list is NOT an authenticated runtime or complete dependency
closure: QtPositioning/system drivers/loader and dlopen remain unverified.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import stat

SCHEMA = "post-apollo.company-maplibre-m32v-qml-inventory.v1"
MAX_FILES = 64
MAX_FILE_BYTES = 512 * 1024 * 1024
MAX_MANIFEST_BYTES = 32768
SAFE_COMPONENT = re.compile(r"[A-Za-z0-9_][A-Za-z0-9_.+\-]{0,94}\Z")
HEX = re.compile(r"[0-9a-f]{64}\Z")
QML_PLUGIN = re.compile(r"[A-Za-z_][A-Za-z0-9_]*\Z")
REQUIRED = frozenset((
    "bin/company-vector-probe",
    "lib64/libQt6Location.so.6",
    "qml/QtLocation/qmldir",
    "qml/QtLocation/plugins.qmltypes",
    "qml/QtLocation/MapView.qml",
    "qml/QtLocation/libdeclarative_locationplugin.so",
    "qml/MapLibre/qmldir",
    "plugins/geoservices/libqtgeoservices_maplibre.so",
))
# Explicit top-level paths; unknown runtime additions are REFUSED rather
# than loaded. QtPositioning and host driver closure are never assumed.
DIRECTORIES = frozenset((
    "", "bin", "lib64", "qml", "qml/QtLocation",
    "qml/MapLibre", "plugins", "plugins/geoservices",
))
ELF_HEADER = b"\x7fELF"
AUTHORITY_FALSE = (
    "complete_loader_closure_verified",
    "qml_runtime_imports_verified",
    "qt_positioning_verified",
    "system_libraries_verified",
    "trusted_distribution_verified",
    "immutable_bundle_verified",
    "operator_authenticated",
    "launch_authenticated",
    "can_spawn", "can_apply_scene", "can_accept_input",
    "can_focus_keyboard", "can_show_popup",
)


class Refused(Exception):
    pass


def valid_relative(path: object) -> bool:
    if not isinstance(path, str) or len(path) > 270:
        return False
    parts = path.split("/")
    return (2 <= len(parts) <= 4 and
            all(SAFE_COMPONENT.fullmatch(part) is not None
                and part not in (".", "..") and ".." not in part
                for part in parts)
            and "/".join(parts[:-1]) in DIRECTORIES)


def manifest_paths(doc: object) -> dict[str, str]:
    if not isinstance(doc, dict) or set(doc) != {"schema", "files"} or doc["schema"] != SCHEMA:
        raise Refused("INVALID_MANIFEST")
    files = doc["files"]
    if not isinstance(files, list) or not (len(REQUIRED) <= len(files) <= MAX_FILES):
        raise Refused("FILE_COUNT")
    seen: dict[str, str] = {}
    for entry in files:
        if not isinstance(entry, dict) or set(entry) != {"path", "sha256"}:
            raise Refused("INVALID_FILE_ENTRY")
        name, digest = entry["path"], entry["sha256"]
        if not valid_relative(name) or not isinstance(digest, str) or not HEX.fullmatch(digest) or digest == "0"*64:
            raise Refused("INVALID_FILE_ENTRY")
        if name in seen:
            raise Refused("DUPLICATE_FILE")
        if (name.split("/",1)[0] == "bin" and name != "bin/company-vector-probe"):
            raise Refused("UNEXPECTED_EXECUTABLE")
        seen[name] = digest
    if not REQUIRED.issubset(seen):
        raise Refused("MISSING_REQUIRED_QML_ARTIFACT")
    return seen


def metadata(st: os.stat_result) -> tuple:
    return (st.st_dev, st.st_ino, st.st_uid, st.st_mode, st.st_nlink,
            st.st_size, st.st_mtime_ns, st.st_ctime_ns)


def check_dir(fd: int) -> os.stat_result:
    st = os.fstat(fd)
    if not stat.S_ISDIR(st.st_mode) or st.st_uid != os.getuid() or st.st_mode & 0o022:
        raise Refused("UNSAFE_DIRECTORY")
    return st


def check_file(fd: int, name: str, expected: str, executable: bool) -> tuple[bytes, bool]:
    first = os.fstat(fd)
    if (not stat.S_ISREG(first.st_mode) or first.st_uid != os.getuid() or
            first.st_mode & 0o6022 or first.st_nlink != 1 or
            first.st_size <= 0 or first.st_size > MAX_FILE_BYTES or
            (executable and not first.st_mode & stat.S_IXUSR)):
        raise Refused("UNSAFE_FILE:" + name)
    sha = hashlib.sha256()
    prefix = bytearray()
    contents = bytearray()
    is_text = name.endswith((".qml", ".qmltypes")) or name.endswith("/qmldir")
    while True:
        block = os.read(fd, 65536)
        if not block:
            break
        if len(prefix) < 4:
            prefix.extend(block[:4-len(prefix)])
        if is_text:
            if len(contents) + len(block) > 1024 * 1024:
                raise Refused("TEXT_TOO_LARGE:" + name)
            contents.extend(block)
        sha.update(block)
    after = os.fstat(fd)
    if metadata(first) != metadata(after):
        raise Refused("FILE_CHANGED:" + name)
    if sha.hexdigest() != expected:
        raise Refused("HASH_MISMATCH:" + name)
    needs_elf = (name.startswith(("bin/", "lib64/", "plugins/")) or name.endswith(".so"))
    if needs_elf and bytes(prefix) != ELF_HEADER:
        raise Refused("NOT_ELF:" + name)
    if is_text:
        try:
            return bytes(contents), needs_elf
        except Exception as exc:
            raise Refused("BAD_TEXT:" + name) from exc
    return b"", needs_elf


def inspect_tree(root: Path, hashes: dict[str, str]) -> dict[str, bytes]:
    # Pin each ancestor via openat O_NOFOLLOW, not a check-then-path-open.
    flags = os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW | os.O_CLOEXEC
    rootfd = os.open(root, flags)
    try:
        root_before = check_dir(rootfd)
        expected_children: dict[str, set[str]] = {folder:set() for folder in DIRECTORIES}
        for folder in DIRECTORIES:
            if folder:
                parent, _, name = folder.rpartition("/")
                expected_children[parent].add(name)
        for name in hashes:
            parent, _, leaf = name.rpartition("/")
            expected_children[parent].add(leaf)

        visited: dict[str, bytes] = {}
        def scan(fd: int, parent_path: str) -> None:
            before = check_dir(fd)
            if set(os.listdir(fd)) != expected_children[parent_path]:
                raise Refused("UNLISTED_OR_MISSING:" + (parent_path or "ROOT"))
            for child in sorted(expected_children[parent_path]):
                rel = parent_path + "/" + child if parent_path else child
                if rel in DIRECTORIES:
                    nextfd = os.open(child, flags, dir_fd=fd)
                    try:
                        scan(nextfd, rel)
                        actual = os.stat(child, dir_fd=fd, follow_symlinks=False)
                        if metadata(os.fstat(nextfd)) != metadata(actual):
                            raise Refused("DIRECTORY_SWAPPED:" + rel)
                    finally:
                        os.close(nextfd)
                else:
                    filefd = os.open(child,
                        os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK | os.O_CLOEXEC,
                        dir_fd=fd)
                    try:
                        raw, _ = check_file(filefd, rel, hashes[rel],
                            rel == "bin/company-vector-probe")
                        current = os.stat(child, dir_fd=fd, follow_symlinks=False)
                        if metadata(os.fstat(filefd)) != metadata(current):
                            raise Refused("FILE_REPLACED:" + rel)
                        visited[rel] = raw
                    finally:
                        os.close(filefd)
            if metadata(before) != metadata(os.fstat(fd)):
                raise Refused("DIRECTORY_CHANGED:" + (parent_path or "ROOT"))
        scan(rootfd, "")
        current_root = root.lstat()
        if metadata(root_before) != metadata(current_root):
            raise Refused("ROOT_REPLACED")
        return visited
    finally:
        os.close(rootfd)


def parse_qmldir(raw: bytes, module: str) -> list[str]:
    try:
        lines = raw.decode("utf-8", "strict").splitlines()
    except UnicodeDecodeError as exc:
        raise Refused("INVALID_QMLDIR_UTF8:" + module) from exc
    entries = [line.split("#",1)[0].strip() for line in lines]
    entries = [line for line in entries if line]
    if entries.count("module " + module) != 1:
        raise Refused("INVALID_QML_MODULE:" + module)
    plugins: list[str] = []
    typeinfo = False
    for line in entries:
        fields = line.split()
        if fields[0] == "plugin":
            if len(fields) != 2 or not QML_PLUGIN.fullmatch(fields[1]):
                raise Refused("UNSAFE_QML_PLUGIN:" + module)
            plugins.append(fields[1])
        elif fields[0] == "typeinfo" and fields[1:] == ["plugins.qmltypes"]:
            typeinfo = True
    if not plugins or len(set(plugins)) != len(plugins):
        raise Refused("MISSING_QML_PLUGIN:" + module)
    if module == "QtLocation" and (plugins != ["declarative_locationplugin"] or not typeinfo):
        raise Refused("QTLOCATION_MODULE_INCOMPLETE")
    return plugins


def audit(root: Path, manifest: object) -> dict:
    out: dict = {
        "schema": SCHEMA, "status": "REFUSED", "reason": None,
        "listed_qml_files_candidate": False,
        "modules_declared": [], "qml_plugins_declared": [],
        "candidate_elf_files": [], "unresolved_external_modules": [
            "QtPositioning", "host Qt runtime", "QMapLibre dependent libraries",
            "ELF transitive dependencies", "Qt platform/graphics driver plugins",
        ],
    }
    out.update({name: False for name in AUTHORITY_FALSE})
    try:
        hashes = manifest_paths(manifest)
        files = inspect_tree(root, hashes)
        for module in ("QtLocation", "MapLibre"):
            qmldir = files["qml/" + module + "/qmldir"]
            plugins = parse_qmldir(qmldir, module)
            expected = {"qml/" + module + "/lib" + name + ".so" for name in plugins}
            if not expected.issubset(hashes):
                raise Refused("QML_PLUGIN_BINARY_UNLISTED:" + module)
            out["modules_declared"].append(module)
            out["qml_plugins_declared"].extend(sorted(expected))
        for name in sorted(hashes):
            if (name.startswith(("bin/", "lib64/", "plugins/")) or name.endswith(".so")):
                out["candidate_elf_files"].append(name)
        out["listed_qml_files_candidate"] = True
        out["status"] = "QML_LISTED_FILES_CANDIDATE_ONLY"
        out["reason"] = "RUNTIME_QML_AND_SYSTEM_LOADER_UNVERIFIED"
    except (Refused, OSError, ValueError) as exc:
        out["reason"] = str(exc) if isinstance(exc, Refused) else "INPUT_UNAVAILABLE"
    return out


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bundle", required=True, type=Path)
    parser.add_argument("--manifest", required=True, type=Path)
    args = parser.parse_args()
    try:
        if args.manifest.stat().st_size > MAX_MANIFEST_BYTES:
            raise ValueError("manifest too large")
        doc = json.loads(args.manifest.read_text(encoding="utf-8"))
    except (OSError, ValueError, UnicodeDecodeError):
        doc = None
    result = audit(args.bundle, doc)
    print(json.dumps(result, sort_keys=True, indent=2))
    # A candidate is still not an approved, executable image.
    return 0 if result["listed_qml_files_candidate"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
