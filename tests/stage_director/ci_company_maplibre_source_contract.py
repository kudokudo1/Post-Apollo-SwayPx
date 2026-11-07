#!/usr/bin/env python3
"""M30e: provenance gate for the *actual* Company MapLibre v3 Qt source.

This reads a separately checked-out, exact-commit Company repository and
the current SwayPX source. It never executes Qt, the user's native binary,
a plugin, a shell, a renderer or an installation command.
A positive result is SOURCE-CONTRACT-ONLY, NEVER ELF/actor approval.
"""
from __future__ import annotations

import argparse
import ast
import json
from pathlib import Path
import re
import subprocess
import sys

COMPANY_SOURCE_COMMIT = "e3595b0035bb3d89aa87c6ba2bd1aac04a975181"
LIMIT = 256 * 1024

REQUIRED_COMPANY_FILES = (
    "experiments/company-vector-qt/CMakeLists.txt",
    "experiments/company-vector-qt/main.cpp",
    "experiments/company-vector-qt/main.qml",
    "experiments/company-vector-qt/style-fixture.json",
    "experiments/company-vector-qt/CompanyVectorHostBridge.py",
    "experiments/company-vector-qt/install_host_bridge_m15.py",
    "services/company/CompanyVectorSidecarLaunch.py",
    "services/company/CompanyVectorSidecarController.qml",
    "widgets/CompanyVectorBridgeLink.qml",
    "widgets/CompanyNetworkW.qml",
)
REQUIRED_SWAY_FILES = (
    "sway/stage_director_company_executable.c",
    "sway/stage_director_company_digest.c",
)


def read_exact(root: Path, rel: str) -> str:
    """Read only known source paths with no symlink or size escape."""
    path = root / rel
    if path.is_symlink() or not path.is_file():
        raise ValueError("MISSING_OR_SYMLINKED:" + rel)
    if path.stat().st_size > LIMIT:
        raise ValueError("OVERSIZED:" + rel)
    return path.read_text(encoding="utf-8")


def assigned_literal(py: str, name: str):
    """Parse a constant tuple from code, never import or run the bridge."""
    tree = ast.parse(py)
    for node in tree.body:
        if isinstance(node, ast.Assign):
            if any(isinstance(t, ast.Name) and t.id == name
                   for t in node.targets):
                return ast.literal_eval(node.value)
    raise ValueError("MISSING_LITERAL:" + name)


def verify(company: Path, sway: Path) -> list[str]:
    failures: list[str] = []
    try:
        c = {name: read_exact(company, name)
             for name in REQUIRED_COMPANY_FILES}
        s = {name: read_exact(sway, name)
             for name in REQUIRED_SWAY_FILES}
    except (ValueError, OSError, UnicodeError) as e:
        return [str(e)]

    def needs(ok: bool, code: str) -> None:
        if not ok:
            failures.append(code)

    qtbase = "experiments/company-vector-qt/"
    cmake = c[qtbase + "CMakeLists.txt"]
    cpp = c[qtbase + "main.cpp"]
    qml = c[qtbase + "main.qml"]
    launch = c["services/company/CompanyVectorSidecarLaunch.py"]
    host = c[qtbase + "CompanyVectorHostBridge.py"]
    installer = c[qtbase + "install_host_bridge_m15.py"]
    controller = c["services/company/CompanyVectorSidecarController.qml"]
    bridge = c["widgets/CompanyVectorBridgeLink.qml"]
    company_ui = c["widgets/CompanyNetworkW.qml"]
    exe = s["sway/stage_director_company_executable.c"]
    digest = s["sway/stage_director_company_digest.c"]

    # This is the original MapLibre QtLocation app, not M30d's generic
    # QWidget proof or a generated imitation.
    for token in ("find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Location Positioning)",
                  "find_package(QMapLibre 3.0 REQUIRED COMPONENTS Location)",
                  "qt_add_executable(company-vector-probe main.cpp)",
                  "QMapLibre::Location",
                  "qmaplibre_location_setup_plugins(company-vector-probe)",
                  'COMPANY_PROBE_STYLE_PATH="',
                  'COMPANY_PROBE_ONLINE_STYLE_PATH="'):
        needs(token in cmake, "MAPLIBRE_CMAKE_CONTRACT:" + token[:45])
    for token in ("import QtLocation 6.5", "import QtPositioning 6.5",
                  "import MapLibre 3.0", 'name: "maplibre"',
                  'name: "maplibre.map.styles"', "MapView {",
                  "map.plugin: mapPlugin", "MapLibre.style: Style {}"):
        needs(token in qml, "MAPLIBRE_QML_CONTRACT:" + token[:45])

    rhi = cpp.find("QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi)")
    app = cpp.find("QGuiApplication app(argc, argv)")
    needs(rhi >= 0 and app > rhi, "GL_API_NOT_PROCESS_LOCAL_BEFORE_QT")
    needs('const bool online = args.contains("--online");' in cpp and
          'const bool styledOnline = args.contains("--styled-online");' in cpp and
          "const bool anyOnline = online || styledOnline;" in cpp,
          "EXPLICIT_ONLINE_CONSENT_MISSING")
    # Check exact offline branch. If a remote style can become default by
    # editing the Qt expression, no source-contract acceptance is allowed.
    style_switch = (
        r'const\s+QUrl\s+styleUrl\s*=\s*styledOnline\s*\?\s*'
        r'QUrl::fromLocalFile\(styledPath\)\s*:\s*online\s*\?\s*'
        r'QUrl\(QStringLiteral\("https://tiles\.openfreemap\.org/styles/liberty"\)\)'
        r'\s*:\s*QUrl::fromLocalFile\(localPath\)\s*;'
    )
    needs(re.search(style_switch, cpp) is not None, "OFFLINE_DEFAULT_REPLACED")
    needs('engine.rootContext()->setContextProperty("probeOnline", anyOnline)' in cpp,
          "QML_ONLINE_MODE_NOT_BOUND")
    needs('"--owner-local-hubs"' in cpp, "OWNER_MODE_SOURCE_MISSING")
    needs("ownerLocalMarkers" in qml and "fixtureMarkers" in qml and
          "SYNTHETIC VECTOR GEOMETRY" in qml,
          "MARKER_DATA_BOUNDARIES_MISSING")

    try:
        style = json.loads(c[qtbase + "style-fixture.json"])
        needs(style.get("version") == 8 and
              style.get("metadata", {}).get("not-real-streets") is True and
              style.get("metadata", {}).get("post-apollo-fixture") is True,
              "OFFLINE_SYNTHETIC_METADATA_LOST")
        sources = style.get("sources", {})
        only = sources.get("probe-features", {})
        needs(set(sources) == {"probe-features"} and
              only.get("type") == "geojson" and
              isinstance(only.get("data"), dict) and
              only["data"].get("type") == "FeatureCollection" and
              isinstance(only["data"].get("features"), list) and
              len(only["data"]["features"]) >= 4,
              "OFFLINE_NO_LONGER_INLINE_GEOJSON")
        needs("http" not in json.dumps(style).lower(),
              "OFFLINE_STYLE_UNEXPECTED_NETWORK")
        needs("glyphs" not in style and "sprite" not in style,
              "OFFLINE_STYLE_REMOTE_ASSETS")
    except (ValueError, TypeError, AttributeError, KeyError):
        failures.append("MALFORMED_OFFLINE_STYLE")

    # Actual M13 -> M15 -> M14 chain: one PID through execv then execve.
    # The M13 launcher installs a PYTHON BRIDGE under ~/.local/libexec,
    # whose final ELF target lives under ~/postapollo-vector-lab.
    try:
        first = assigned_literal(launch, "RELATIVE_EXECUTABLE")
        target = assigned_literal(installer, "TARGET_PARTS")
        lab = assigned_literal(host, "LAB_FILES")
        needs(first == target ==
              (".local", "libexec", "post-apollo", "company-vector-probe"),
              "M13_M15_INSTALLED_BRIDGE_PATH_DRIFT")
        needs(lab[0] == ("postapollo-vector-lab", "company-vector-build",
                          "company-vector-probe"),
              "M15_FINAL_ELF_TARGET_DRIFT")
    except (SyntaxError, ValueError, TypeError, IndexError):
        failures.append("M15_LAUNCH_PATH_LITERAL_UNREADABLE")
    needs("os.execv(str(executable)" in launch and
          "os.execve(str(binary)" in host and
          "vectorRunner.processId" in controller and
          "controller.nativeChildStarted(attachedPid)" in controller,
          "ATTACHED_SAME_PID_EXEC_CHAIN_BROKEN")
    needs('property bool vectorBridgeOptIn: false' in company_ui and
          'property bool bridgeEnabled: false' in bridge and
          'Qt.createQmlObject(' in bridge and
          'function reportChildPid(pid)' in bridge,
          "COMPANY_NATIVE_BRIDGE_NOT_DEFAULT_OFF")
    needs('open_owned_dir(home, "postapollo-vector-lab"' in exe and
          'open_owned_dir(lab, "company-vector-build"' in exe and
          'openat(build, "company-vector-probe"' in exe and
          "AT_SYMLINK_NOFOLLOW" in exe and
          "same_file(&still_installed, &executing)" in exe,
          "M29_FINAL_ELF_INODE_CONTRACT_DRIFT")
    needs('company_m30_manifest_elf_sha256[] = "";' in digest,
          "M30A_UNAPPROVED_DIGEST_PIN")
    for token in ("wlr_scene_node_set_", "wlr_seat_", "posix_spawn(", "wl_display_connect("):
        needs(token not in launch + host + controller + bridge,
              "CLIENT_BYPASS:" + token)

    return failures


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--company", type=Path, required=True)
    p.add_argument("--sway", type=Path, required=True)
    args = p.parse_args()
    commit = subprocess.run(
        ["git", "-C", str(args.company), "rev-parse", "HEAD"],
        capture_output=True, text=True, timeout=6, check=False)
    if commit.returncode or commit.stdout.strip() != COMPANY_SOURCE_COMMIT:
        print("M30E_SOURCE_BLOCKED: UNAPPROVED_COMPANY_COMMIT")
        return 2
    dirty = subprocess.run(
        ["git", "-C", str(args.company), "status", "--porcelain"],
        capture_output=True, text=True, timeout=6, check=False)
    if dirty.returncode or dirty.stdout.strip():
        print("M30E_SOURCE_BLOCKED: MODIFIED_COMPANY_SOURCE")
        return 2
    failures = verify(args.company, args.sway)
    if failures:
        for code in failures:
            print("M30E_SOURCE_BLOCKED:", code)
        return 1
    print("M30E_MAPLIBRE_SOURCE_CONTRACT PASS "
          "real_company_code=1 source_sha_pinned=1 offline_default=1 "
          "m13_m15_m29_paths_match=1 digest=UNPINNED "
          "real_maplibre_binary=NOT_EXECUTED custody=NONE authority=NONE")
    return 0


if __name__ == "__main__":
    sys.exit(main())
