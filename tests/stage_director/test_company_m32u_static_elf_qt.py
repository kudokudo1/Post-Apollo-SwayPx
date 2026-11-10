#!/usr/bin/env python3
"""M32u synthetic ELF readelf closure and negative source contracts.

All binaries are test fixtures compiled in the CI runner, NEVER executed.
This does not prove real Qt plugin discovery or installed MapLibre identity.
"""
from __future__ import annotations

import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "tools/company/m32u_elf_qt_audit.py"
spec = importlib.util.spec_from_file_location("company_m32u_audit", SCRIPT)
assert spec and spec.loader
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


def compile_elf(*args: str) -> None:
    subprocess.run(["cc", *map(str, args)], check=True, timeout=20)


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def make_manifest(folder: Path) -> dict:
    return {"schema": module.SCHEMA, "entries": [
        {"group": group, "name": name, "sha256": sha(folder / group / name)}
        for group, name in [
            ("bin", "company-vector-probe"),
            ("lib", "libmapfixture.so"),
            ("plugins", "libqfixture.so"),
        ]
    ]}


def ensure_denied(result: dict, reason: str) -> None:
    assert result["status"] == "REFUSED", result
    assert reason in result["reason"], result
    assert not result["listed_elf_dependencies_candidate"]
    no_grants(result)


def no_grants(result: dict) -> None:
    for attr in (
        "qt_plugin_discovery_verified", "system_loader_verified",
        "immutable_image_verified", "operator_authenticated",
        "launch_authenticated", "can_spawn", "can_apply_scene",
        "can_accept_input", "can_focus_keyboard", "can_show_popup",
    ):
        assert result[attr] is False, (attr, result)


def main() -> None:
    src = SCRIPT.read_text()
    for required in (
        '["readelf", "-W", "-h", "-l", "-d", "--", str(path)]',
        'shell=False', 'os.O_NOFOLLOW', 'os.O_NONBLOCK',
        'group == "bin"', 'stat.S_ISREG(old.st_mode)', "st.st_nlink != 1",
        "set(os.listdir(folder)) != wanted[group]",
        'result["unresolved"].append(', 'qt_plugin_discovery_verified": False',
        'can_spawn": False', 'can_apply_scene": False',
    ):
        assert required in src, required
    for forbidden in ("[\"ldd\",", "subprocess.Popen(",
                      "os.system(", "os.exec", "posix_spawn("):
        assert forbidden not in src, forbidden

    with tempfile.TemporaryDirectory(prefix="company-m32u-") as temp:
        work = Path(temp)
        bundle = work / "bundle"
        for name in module.ALLOWED_GROUPS:
            (bundle / name).mkdir(parents=True, mode=0o700)
        lib = bundle / "lib/libmapfixture.so"
        plugin = bundle / "plugins/libqfixture.so"
        exe = bundle / "bin/company-vector-probe"
        dep_src = work / "dep.c"
        dep_src.write_text("int fixture(void) { return 19; }\n")
        main_src = work / "main.c"
        main_src.write_text("extern int fixture(void); int _start(void) { return fixture(); }\n")

        compile_elf("-shared", "-fPIC", "-nostdlib",
                    "-Wl,-soname,libmapfixture.so",
                    "-o", str(lib), str(dep_src))
        compile_elf("-shared", "-fPIC", "-nostdlib",
                    "-Wl,--no-as-needed", "-L" + str(bundle / "lib"),
                    "-lmapfixture", "-o", str(plugin), str(dep_src))
        compile_elf("-nostdlib", "-no-pie", "-Wl,-e,_start",
                    "-Wl,--no-as-needed", "-Wl,-rpath,$ORIGIN/../lib",
                    "-L" + str(bundle / "lib"), "-o", str(exe),
                    str(main_src), "-lmapfixture")
        manifest = make_manifest(bundle)

        candidate = module.audit(bundle, manifest)
        assert candidate["status"] == "LISTED_ELF_NEEDED_CANDIDATE_ONLY", candidate
        assert candidate["listed_elf_dependencies_candidate"]
        assert candidate["objects"][0]["needed"] == ["libmapfixture.so"]
        assert candidate["objects"][0]["interpreter"]
        assert candidate["unresolved"] == []
        no_grants(candidate)

        changed = json.loads(json.dumps(manifest))
        changed["entries"][1]["sha256"] = "0" * 64
        ensure_denied(module.audit(bundle, changed), "INVALID_ENTRY_VALUE")
        changed["entries"][1]["sha256"] = "1" * 64
        ensure_denied(module.audit(bundle, changed), "DIGEST_MISMATCH")

        duplicate = json.loads(json.dumps(manifest))
        duplicate["entries"].append(duplicate["entries"][1])
        ensure_denied(module.audit(bundle, duplicate), "DUPLICATE_ENTRY")
        traversal = json.loads(json.dumps(manifest))
        traversal["entries"][1]["name"] = "../libmapfixture.so"
        ensure_denied(module.audit(bundle, traversal), "INVALID_ENTRY_VALUE")
        missing_plugin = json.loads(json.dumps(manifest))
        missing_plugin["entries"] = missing_plugin["entries"][:2]
        ensure_denied(module.audit(bundle, missing_plugin), "INVALID_ENTRY_COUNT")

        rogue = bundle / "plugins/libunexpected.so"
        rogue.write_bytes(b"not declared")
        ensure_denied(module.audit(bundle, manifest), "UNLISTED_OR_MISSING_FILE")
        rogue.unlink()

        old = plugin.read_bytes()
        plugin.unlink()
        plugin.symlink_to(lib)
        ensure_denied(module.audit(bundle, manifest), "UNSAFE_FILE")
        plugin.unlink()
        plugin.write_bytes(old)
        plugin.chmod(0o600)

        plugin.chmod(0o666)
        ensure_denied(module.audit(bundle, manifest), "UNSAFE_FILE")
        plugin.chmod(0o600)

        # Existing source allows an unexpected loader path? Fail closed.
        compile_elf("-nostdlib", "-no-pie", "-Wl,-e,_start",
                    "-Wl,--no-as-needed", "-Wl,-rpath,/tmp/unsafe",
                    "-L" + str(bundle / "lib"), "-o", str(exe),
                    str(main_src), "-lmapfixture")
        manifest = make_manifest(bundle)
        ensure_denied(module.audit(bundle, manifest), "UNSAFE_RPATH")

        # Link an additional .so outside the declared bundle; static
        # dependency graph must NOT silently accept the host's library.
        outside = work / "liboutside.so"
        compile_elf("-shared", "-fPIC", "-nostdlib",
                    "-Wl,-soname,liboutside.so", "-o", str(outside), str(dep_src))
        compile_elf("-nostdlib", "-no-pie", "-Wl,-e,_start",
                    "-Wl,--no-as-needed", "-Wl,-rpath,$ORIGIN/../lib",
                    "-L" + str(bundle / "lib"), "-L" + str(work),
                    "-o", str(exe), str(main_src), "-lmapfixture", "-loutside")
        manifest = make_manifest(bundle)
        unresolved = module.audit(bundle, manifest)
        ensure_denied(unresolved, "UNRESOLVED_DT_NEEDED")
        assert {"origin": "company-vector-probe", "needed": "liboutside.so"} in unresolved["unresolved"]

        # A forged self-reported manifest and arbitrary bundle location
        # never produce authority, even for the passing synthetic case.
        shutil.copyfile(lib, work / "different.so")
        manifest["entries"][0]["sha256"] = "0" * 64
        ensure_denied(module.audit(bundle, manifest), "INVALID_ENTRY_VALUE")

    print("SD_COMPANY_M32U readelf_only=PASS positive_fixture=STATIC_ONLY "
          "external_soname=UNRESOLVED rpath_traversal_symlink=DENIED "
          "qt_dlopen=UNVERIFIED spawn=OFF scene=OFF PASS")


if __name__ == "__main__":
    main()
