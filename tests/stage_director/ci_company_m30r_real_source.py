#!/usr/bin/env python3
"""M30r: native Quickshell acceptance of merged Company M30q QML as-is.

Reuses M30i's true headless Wayland/Quickshell pixel and geometry tests,
but deliberately DOES NOT apply M30i's two old candidate-coordinate
patches. Both fixes must be in the actual reviewed Company source.
This wrapper adds only read-only, temporary CI-copy observability.
No MapLibre launch, actor admission, native input grant or desktop write.
"""
from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess
import sys

import ci_company_real_quickshell as original

COMPANY_MERGED_SHA = "877eb00d0f260f9fdda52712f35abec59b2f83b3"
EXPECTED_QML_BLOBS = {
    "widgets/CompanyCoveragePane.qml":
        "7f6ceaa578be942fde4728ff0ca36bd18752aa44",
    "widgets/CompanyNetworkW.qml":
        "432573110a65e6e7a5dd94da9f958f80261e227d",
}


def verify_company_original(checkout: Path):
    if not (os.environ.get("GITHUB_ACTIONS") == "true" and
            os.environ.get("CI") == "true" and
            os.environ.get("STAGE_DIRECTOR_CI_COMPANY_QUICKSHELL") == "1" and
            os.environ.get("STAGE_DIRECTOR_CI_COMPANY_M30R") == "1"):
        raise RuntimeError("M30R_NOT_EXACT_PRIVATE_CI")
    original.PIN = COMPANY_MERGED_SHA
    for path, expected in EXPECTED_QML_BLOBS.items():
        cmd = subprocess.run(
            ["git", "-C", str(checkout), "hash-object", path],
            text=True, capture_output=True, check=False, timeout=8)
        if cmd.returncode or cmd.stdout.strip() != expected:
            raise RuntimeError("M30R_ORIGINAL_QML_SOURCE_DRIFT_" +
                               path.replace("/", "_"))
    network = (checkout / "widgets/CompanyNetworkW.qml").read_text()
    coverage = (checkout / "widgets/CompanyCoveragePane.qml").read_text()
    if not (
        "const origin = companyFrame.mapToItem(null, 0, 0);" in network and
        "const origin = companyFrame.mapToItem(root, 0, 0);" not in network and
        "const origin = Qt.point(" in coverage and
        "column.x + mapRow.x + mapSurface.x + mapCanvas.x" in coverage and
        "column.y + mapRow.y + mapSurface.y + mapCanvas.y" in coverage and
        "mapCanvas.parent !== mapSurface" in coverage and
        "const origin = mapCanvas.mapToItem(root, 0, 0);" not in coverage and
        "property bool vectorBridgeOptIn: false" in network and
        "CompanyVectorSidecarController {" in coverage):
        raise RuntimeError("M30R_COMPANY_OWNER_QML_REPAIR_NOT_LANDED")
    print("SD_COMPANY_M30R_SOURCE "
          "exact_merged_company_commit=1 exact_qml_blobs=1 "
          "M30q_geometry_repairs_in_original=1 bridge_default_off=1 "
          "no_ci_geometry_replacement=1 PASS", flush=True)


def instrument_without_geometry_repair(source: Path, out: Path) -> Path:
    verify_company_original(source)
    if out.exists() or out.is_symlink():
        raise RuntimeError("M30R_DISPOSABLE_COPY_ALREADY_EXISTS")
    out.mkdir(parents=True, mode=0o700)
    shutil.copytree(source, out / "company",
                    ignore=shutil.ignore_patterns(".git", "__pycache__"),
                    symlinks=False, dirs_exist_ok=False)
    temp = out / "company"
    # Verify the original coordinate logic is still intact in the actual
    # copy. The only permitted CI edits insert read-only diagnostic fields.
    for path in EXPECTED_QML_BLOBS:
        before=(source / path).read_bytes()
        after=(temp / path).read_bytes()
        if before != after:
            raise RuntimeError("M30R_COPIED_COMPANY_QML_MISMATCH")

    original.patch_read_only(
        temp / "widgets/CompanyCoveragePane.qml",
        "Item {\n    id: root\n", original.INJECT_COVERAGE)
    original.patch_read_only(
        temp / "widgets/CompanyNetworkW.qml",
        "PanelWindow {\n    id: root\n", original.INJECT_NETWORK)
    network = (temp / "widgets/CompanyNetworkW.qml").read_text()
    coverage = (temp / "widgets/CompanyCoveragePane.qml").read_text()
    if network.count("const origin = companyFrame.mapToItem(null, 0, 0);") != 1 or (
            coverage.count("const origin = Qt.point(") != 1 or
            "mapCanvas.mapToItem(root, 0, 0)" in coverage):
        raise RuntimeError("M30R_SOURCE_GEOMETRY_WAS_REPLACED_IN_CI")
    shell_source = Path(
        "tests/stage_director/ci_company_real_quickshell_shell.qml")
    if not shell_source.is_file() or shell_source.is_symlink():
        raise RuntimeError("M30R_REAL_QUICKSHELL_SHELL_MISSING")
    (temp / "shell.qml").write_bytes(shell_source.read_bytes())
    print("SD_COMPANY_M30R_CI_COPY "
          "only_readonly_qml_geometry_probes=1 "
          "original_Company_M30q_transform_unmodified=1 "
          "actual_test_runner=ORIGINAL_M30I_NATIVE_WAYLAND PASS",flush=True)
    return temp / "shell.qml"


def main():
    # Original guard still checks private CI socket, runner identity,
    # actual original checkout git SHA/cleanliness, Nix tool paths,
    # non-root and exactly one Sway IPC socket.
    original.PIN = COMPANY_MERGED_SHA
    original.prepare = instrument_without_geometry_repair
    original.main()
    print("SD_COMPANY_M30R_MERGED_COMPANY_NATIVE_QML "
          "source_geometry_not_patched=1 real_Quickshell=1 "
          "real_Wayland_pixels=1 standard_compact_fullscreen_close=PASS "
          "native_child=NONE bridge=OFF compositor_scene_input=OFF PASS",
          flush=True)


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print("SD_COMPANY_M30R_BLOCKED:", str(exc), file=sys.stderr, flush=True)
        raise SystemExit(1)
