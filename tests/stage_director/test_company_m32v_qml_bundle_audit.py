#!/usr/bin/env python3
"""M32v synthetic read-only QtLocation/MapLibre file manifest regression.

Compiles minimal fixture ELF bytes; NEVER executes MapLibre or any fixture.
Checks no authority even when every declared file and qmldir matches.
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
SCRIPT = ROOT / "tools/company/m32v_qml_bundle_audit.py"
spec = importlib.util.spec_from_file_location("m32v", SCRIPT)
assert spec and spec.loader
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


def sha(p: Path) -> str:
    h = hashlib.sha256()
    with p.open("rb") as f:
        for block in iter(lambda: f.read(65536), b""):
            h.update(block)
    return h.hexdigest()


def manifest(bundle: Path) -> dict:
    leafs = sorted(x for x in bundle.rglob("*") if x.is_file())
    return {"schema": m.SCHEMA, "files": [
        {"path": str(p.relative_to(bundle)), "sha256": sha(p)}
        for p in leafs
    ]}


def deny(out: dict, code: str) -> None:
    assert out["status"] == "REFUSED", out
    assert code in out["reason"], out
    assert not out["listed_qml_files_candidate"]
    no_authority(out)


def no_authority(out: dict) -> None:
    for key in m.AUTHORITY_FALSE:
        assert out[key] is False, (key, out)


def compile_fixture(cc: str, source: Path, path: Path, executable: bool) -> None:
    args = [cc, "-o", str(path), str(source)]
    if not executable:
        args = [cc, "-shared", "-fPIC", "-o", str(path), str(source)]
    subprocess.run(args, check=True, timeout=15)
    path.chmod(0o700 if executable else 0o600)


def main() -> None:
    src = SCRIPT.read_text(encoding="utf-8")
    for check in (
        "os.O_NOFOLLOW", "dir_fd=fd", "os.O_NONBLOCK",
        "stat.S_ISREG", "metadata(os.fstat(filefd)) != metadata(current)",
        "set(os.listdir(fd)) != expected_children[parent_path]",
        "QTLOCATION_MODULE_INCOMPLETE", "QML_PLUGIN_BINARY_UNLISTED",
        "QML_LISTED_FILES_CANDIDATE_ONLY", "trusted_distribution_verified",
        "runtime_qml_and_system_loader_unverified" if False else "RUNTIME_QML_AND_SYSTEM_LOADER_UNVERIFIED",
    ):
        assert check in src, check
    for forbidden in ("os.system(", "os.execv(", "subprocess.run(",
                      "posix_spawn(", "fexecve(", "can_spawn\": True"):
        assert forbidden not in src, forbidden

    with tempfile.TemporaryDirectory(prefix="company-m32v-") as d:
        root = Path(d) / "bundle"
        dirs = ("bin", "lib64", "qml/QtLocation", "qml/MapLibre", "plugins/geoservices")
        for x in dirs:
            (root / x).mkdir(parents=True, mode=0o700, exist_ok=True)
        source = Path(d) / "safe.c"
        source.write_text("int main(void) { return 0; }\n", encoding="utf-8")
        so_source = Path(d) / "plugin.c"
        so_source.write_text("int harmless(void) { return 1; }\n", encoding="utf-8")
        cc = "cc"
        objects = (
            ("bin/company-vector-probe", True),
            ("lib64/libQt6Location.so.6", False),
            ("qml/QtLocation/libdeclarative_locationplugin.so", False),
            ("qml/MapLibre/libmaplibreplugin.so", False),
            ("plugins/geoservices/libqtgeoservices_maplibre.so", False),
        )
        for rel, executable in objects:
            compile_fixture(cc, source if executable else so_source,
                            root / rel, executable)

        (root / "qml/QtLocation/qmldir").write_text(
            "module QtLocation\nplugin declarative_locationplugin\n"
            "typeinfo plugins.qmltypes\n", encoding="utf-8")
        (root / "qml/QtLocation/MapView.qml").write_text(
            "import QtQuick 6.5\nItem {}\n", encoding="utf-8")
        (root / "qml/QtLocation/plugins.qmltypes").write_text(
            "import QtQuick.tooling 1.2\nModule {}\n", encoding="utf-8")
        (root / "qml/MapLibre/qmldir").write_text(
            "module MapLibre\nplugin maplibreplugin\n", encoding="utf-8")

        doc = manifest(root)
        assert len(doc["files"]) == 9, doc
        passed = m.audit(root, doc)
        assert passed["status"] == "QML_LISTED_FILES_CANDIDATE_ONLY", passed
        assert passed["listed_qml_files_candidate"]
        assert passed["modules_declared"] == ["QtLocation", "MapLibre"]
        assert len(passed["candidate_elf_files"]) == 5
        assert "QtPositioning" in passed["unresolved_external_modules"]
        no_authority(passed)

        # A legitimate-looking file manifest with unknown user-host runtime
        # dependencies must still NEVER yield authorization.
        unknown = json.loads(json.dumps(doc))
        unknown["files"][0]["sha256"] = "0"*64
        deny(m.audit(root, unknown), "INVALID_FILE_ENTRY")
        unknown["files"][0]["sha256"] = "1"*64
        deny(m.audit(root, unknown), "HASH_MISMATCH")

        missing = json.loads(json.dumps(doc))
        missing["files"] = [x for x in missing["files"]
                            if x["path"] != "qml/QtLocation/MapView.qml"]
        deny(m.audit(root, missing), "MISSING_REQUIRED_QML_ARTIFACT")

        duplicate = json.loads(json.dumps(doc))
        duplicate["files"].append(duplicate["files"][0])
        deny(m.audit(root, duplicate), "DUPLICATE_FILE")

        traversal = json.loads(json.dumps(doc))
        traversal["files"][0]["path"] = "qml/../etc/passwd"
        deny(m.audit(root, traversal), "INVALID_FILE_ENTRY")

        rogue = root / "qml/QtLocation/rogue.so"
        rogue.write_bytes(b"rogue")
        deny(m.audit(root, doc), "UNLISTED_OR_MISSING")
        rogue.unlink()

        qt_dir = root / "qml/QtLocation/qmldir"
        good = qt_dir.read_bytes()
        qt_dir.write_text("module QtLocation\nplugin malicious\n"
                          "typeinfo plugins.qmltypes\n")
        changed = manifest(root)
        deny(m.audit(root, changed), "QTLOCATION_MODULE_INCOMPLETE")
        qt_dir.write_bytes(good)

        ml_dir = root / "qml/MapLibre/qmldir"
        good = ml_dir.read_bytes()
        ml_dir.write_text("module MapLibre\nplugin unknown\n")
        changed = manifest(root)
        deny(m.audit(root, changed), "QML_PLUGIN_BINARY_UNLISTED")
        ml_dir.write_bytes(good)

        plugin = root / "qml/MapLibre/libmaplibreplugin.so"
        plugin.unlink()
        plugin.symlink_to(root / "qml/QtLocation/libdeclarative_locationplugin.so")
        deny(m.audit(root, doc), "UNSAFE_FILE")
        plugin.unlink()
        compile_fixture(cc, so_source, plugin, False)
        doc = manifest(root)

        plugin.chmod(0o666)
        deny(m.audit(root, doc), "UNSAFE_FILE")
        plugin.chmod(0o600)

        # A fake extra QML import directory must not be ignored.
        (root / "qml/QtPositioning").mkdir()
        deny(m.audit(root, doc), "UNLISTED_OR_MISSING")
        (root / "qml/QtPositioning").rmdir()

        assert m.audit(root, doc)["listed_qml_files_candidate"]
        no_authority(m.audit(root, doc))

    print("SD_COMPANY_M32V_QML synthetic_bundle=PASS "
          "QtLocation=LISTED MapLibre=LISTED QtPositioning=UNVERIFIED "
          "rogue_plugin_symlink_tamper=DENIED real_artifact=NOT_INSPECTED "
          "launch=OFF scene=OFF PASS")


if __name__ == "__main__":
    main()
