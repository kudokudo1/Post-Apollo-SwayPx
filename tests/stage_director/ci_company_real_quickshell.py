#!/usr/bin/env python3
"""M30i: real pinned Company Quickshell geometry and screenshot witness.

This copies the ORIGINAL reviewed Company source into disposable CI storage,
adds two *read-only* geometry introspection properties to the CI copy, and
runs its real CompanyNetworkW PanelWindow with a real Quickshell Wayland
process. It does NOT launch MapLibre, spoof 'vectorController.running',
publish the optional native bridge, or grant scene/input authority.

No user desktop, credentials, client data, or canonical Company checkout is
modified. Source remains pinned independently by M30e before copying.
"""
from __future__ import annotations

import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import time

from ci_company_overlay_prototype import call, ppm, rgb

PIN = "e3595b0035bb3d89aa87c6ba2bd1aac04a975181"
GEOMETRY = re.compile(
    r"SD_COMPANY_M30I_QML_GEOMETRY phase=(standard|compact|fullscreen|closed) "
    r"level=(compact|standard|fullscreen) open=([01]) "
    r"panel=(\d+),(\d+) "
    r"frame=(-?\d+),(-?\d+),(\d+),(\d+) "
    r"canvas=(-?\d+),(-?\d+),(\d+),(\d+) "
    r"candidate=(-?\d+),(-?\d+),(\d+),(\d+) "
    r"bridge_enabled=([01])"
)
EXPECTED = {
    "standard": {"level": "standard", "panel": (1000, 700),
                 "frame": (12, 9, 976, 679), "origin": (460, 148)},
    "compact": {"level": "compact", "panel": (640, 490),
                "frame": (12, 9, 616, 469), "origin": (640, 148)},
    "fullscreen": {"level": "fullscreen", "panel": (1920, 1080),
                   "frame": (12, 9, 1896, 1059), "origin": (0, 0)},
    "closed": {"level": "fullscreen", "panel": (1920, 1080),
               "frame": (0, 0, 0, 0), "origin": (0, 0)},
}
INJECT_COVERAGE = '''    // M30i disposable CI-copy observability ONLY; not installed in Company.
    // This reports the actual Canvas and its mapToItem transform even
    // while the native-running policy correctly denies bridge geometry.
    readonly property rect ciM30iActualCanvasRect: {
        if (!mapCanvas.visible || mapCanvas.width <= 0 ||
                mapCanvas.height <= 0)
            return Qt.rect(0, 0, 0, 0);
        const p = mapCanvas.mapToItem(root, 0, 0);
        return Qt.rect(p.x, p.y, mapCanvas.width, mapCanvas.height);
    }
'''
INJECT_NETWORK = '''    // CI-copy only: read native map canvas geometry without granting
    // viewport ownership; activeVectorViewportRect stays protected.
    readonly property rect ciM30iActualCanvasHostRect: {
        if (!root.menuOpen || !coverageLoader.active ||
                !coverageLoader.item || !coverageLoader.item.visible)
            return Qt.rect(0, 0, 0, 0);
        const canvas = coverageLoader.item.ciM30iActualCanvasRect;
        if (canvas.width <= 0 || canvas.height <= 0)
            return Qt.rect(0, 0, 0, 0);
        const p = coverageLoader.item.mapToItem(root, canvas.x, canvas.y);
        return Qt.rect(p.x, p.y, canvas.width, canvas.height);
    }
'''


def guard():
    e = os.environ
    temp = Path(e.get("RUNNER_TEMP", "/not-ci"))
    runtime = Path(e.get("XDG_RUNTIME_DIR", "/no-runtime"))
    display = runtime / e.get("WAYLAND_DISPLAY", "/no-wayland")
    checkout = Path(e.get("STAGE_DIRECTOR_COMPANY_PINNED_SOURCE", "/no-source"))
    qs = Path(e.get("STAGE_DIRECTOR_TEST_QUICKSHELL", "/no-qs"))
    sway = Path(e.get("STAGE_DIRECTOR_TEST_SWAYMSG", "/no-sway"))
    grim = Path(e.get("STAGE_DIRECTOR_TEST_GRIM", "/no-grim"))
    ok = (e.get("GITHUB_ACTIONS") == "true" and e.get("CI") == "true"
          and e.get("STAGE_DIRECTOR_CI_COMPANY_QUICKSHELL") == "1"
          and temp == Path("/home/runner/work/_temp")
          and str(runtime).startswith(str(temp) + "/") and runtime.is_dir()
          and display.is_socket() and os.getuid() != 0
          and checkout.is_dir() and (checkout / ".git").exists()
          and all(str(p).startswith("/nix/store/") and p.is_file()
                  for p in (qs, sway, grim)))
    if not ok:
        raise RuntimeError("BLOCKED_M30I_NOT_DISPOSABLE_QUICKSHELL")
    head = subprocess.run(["git", "-C", str(checkout), "rev-parse", "HEAD"],
        capture_output=True, text=True, check=False, timeout=8)
    dirty = subprocess.run(["git", "-C", str(checkout), "status", "--porcelain"],
        capture_output=True, text=True, check=False, timeout=8)
    if head.returncode or head.stdout.strip() != PIN or (
            dirty.returncode or dirty.stdout.strip()):
        raise RuntimeError("BLOCKED_M30I_COMPANY_SOURCE_NOT_EXACT_AND_CLEAN")
    sockets = [x for x in runtime.glob("sway-ipc.*.sock") if x.is_socket()]
    if len(sockets) != 1:
        raise RuntimeError("BLOCKED_M30I_PRIVATE_SWAY_IPC_NOT_UNIQUE")
    return e, temp, runtime, checkout, qs, sway, grim, str(sockets[0])


def patch_read_only(target: Path, old: str, addition: str):
    if target.is_symlink() or not target.is_file() or target.stat().st_size > 200000:
        raise RuntimeError("M30I_DISALLOWED_QML_SOURCE")
    content = target.read_text(encoding="utf-8")
    if content.count(old) != 1 or "ciM30i" in content:
        raise RuntimeError("M30I_QML_INTROSPECTION_ANCHOR_DRIFT")
    # The reviewed source is never edited: target is the disposable copy.
    target.write_text(content.replace(old, old + addition, 1), encoding="utf-8")


def prepare(source: Path, out: Path) -> Path:
    if out.exists():
        raise RuntimeError("BLOCKED_M30I_EXPERIMENT_ALREADY_PRESENT")
    out.mkdir(parents=True, mode=0o700)
    # Preserves exact original component structure without invoking shell.qml
    # or touching the operator's installed Company. No git metadata.
    shutil.copytree(source, out / "company",
                    ignore=shutil.ignore_patterns(".git", "__pycache__"),
                    symlinks=False, dirs_exist_ok=False)
    temp = out / "company"
    patch_read_only(temp / "widgets/CompanyCoveragePane.qml",
                    "Item {\n    id: root\n", INJECT_COVERAGE)
    patch_read_only(temp / "widgets/CompanyNetworkW.qml",
                    "PanelWindow {\n    id: root\n", INJECT_NETWORK)
    original = Path("tests/stage_director/ci_company_real_quickshell_shell.qml")
    if not original.is_file() or original.is_symlink():
        raise RuntimeError("M30I_MISSING_CI_ONLY_SHELL")
    (temp / "shell.qml").write_text(original.read_text(encoding="utf-8"),
                                    encoding="utf-8")
    return temp / "shell.qml"


def check_geometry(match, phase: str):
    values = match.groups()
    level = values[1]
    opened = values[2] == "1"
    panel = tuple(map(int, values[3:5]))
    frame = tuple(map(int, values[5:9]))
    canvas = tuple(map(int, values[9:13]))
    candidate = tuple(map(int, values[13:17]))
    bridge = values[17]
    expected = EXPECTED[phase]
    if level != expected["level"] or panel != expected["panel"]:
        raise AssertionError(f"M30I_COMPANY_CHASSIS_GEOMETRY_{phase} "
                             f"level={level} panel={panel} want={expected}")
    if bridge != "0":
        raise AssertionError("M30I_PRODUCTION_BRIDGE_ACTIVATED")
    if candidate != (0, 0, 0, 0):
        raise AssertionError("M30I_NATIVE_CANDIDATE_WITHOUT_NATIVE_CHILD")
    if frame != expected["frame"]:
        raise AssertionError(f"M30I_FRAME_IN_HOST_DRIFT_{phase} {frame}")
    if opened == (phase == "closed"):
        raise AssertionError("M30I_COMPANY_MENU_VISIBILITY_DRIFT")
    if phase == "closed":
        if canvas != (0, 0, 0, 0):
            raise AssertionError("M30I_CANVAS_NOT_REVOKED_AFTER_CLOSE")
    else:
        fx, fy, fw, fh = frame
        x, y, w, h = canvas
        if not (w >= 100 and h >= 100 and
                fx <= x and fy <= y and
                x + w <= fx + fw and y + h <= fy + fh):
            raise AssertionError(f"M30I_REAL_CANVAS_OUTSIDE_FRAME {phase} "
                                 f"frame={frame} canvas={canvas}")
    return {"phase": phase, "level": level, "panel": panel,
            "frame": frame, "canvas": canvas, "candidate": candidate,
            "origin": expected["origin"]}


def screenshot(grim: Path, temp: Path, entry):
    phase = entry["phase"]
    path = temp / f"sd-company-m30i-{phase}.ppm"
    call(str(grim), "-t", "ppm", str(path), timeout=25)
    img = ppm(path)
    if phase == "closed":
        # With menuOpen=false the real PanelWindow mask is empty. Sampling
        # the prior Company core must reveal no still-visible yellow/cyan.
        for x, y in ((350, 250), (750, 300), (1000, 600), (1400, 800)):
            c = rgb(img, x, y)
            if max(c) > 60:
                raise AssertionError(f"M30I_CLOSED_PANEL_VISIBLE {x},{y}:{c}")
    else:
        origin = entry["origin"]
        fx, fy, fw, fh = entry["frame"]
        left = origin[0] + fx
        top = origin[1] + fy
        if not (0 <= left < 1920 and 0 <= top < 1080 and
                left + fw <= 1920 and top + fh <= 1080):
            raise AssertionError("M30I_VIEWPORT_OFF_SCREEN")
        # This pixel evidence is from the *actual* Quickshell/Company
        # compositor output; the screenshot is NOT synthetic.
        occupied = 0
        cyan = 0
        for y in range(top + 8, top + fh - 8, 8):
            for x in range(left + 8, left + fw - 8, 8):
                c = rgb(img, x, y)
                if max(c) > 25:
                    occupied += 1
                if c[1] > 150 and c[2] > 150 and c[1] > c[0] * 1.35:
                    cyan += 1
        if occupied < 50 or cyan < 5:
            raise AssertionError(f"M30I_REAL_COMPANY_NOT_PAINTED {phase} "
                                 f"occupied={occupied} cyan={cyan}")
        print(f"SD_COMPANY_M30I_REAL_QUICKSHELL_PIXELS phase={phase} "
              f"occupied={occupied} cyan={cyan} source=PINNED_COMPANY_UI PASS",
              flush=True)
        return
    print("SD_COMPANY_M30I_REAL_QUICKSHELL_PIXELS "
          "phase=closed mask=ZERO still_visible_panel=0 PASS", flush=True)


def main():
    e, temp, runtime, checkout, qs, sway, grim, ipc = guard()
    shell = prepare(checkout, temp / "sd-company-m30i")
    home = temp / "sd-company-m30i-home"
    for path in (home, home / ".config", home / ".local/share",
                 home / ".cache"):
        path.mkdir(parents=True, exist_ok=True, mode=0o700)
    compat_qml = Path(e.get("STAGE_DIRECTOR_M30I_QML_IMPORT_PATH", "/no-qtqml"))
    if not (str(compat_qml).startswith("/nix/store/") and
            (compat_qml / "Qt5Compat/GraphicalEffects/qmldir").is_file()):
        raise RuntimeError("BLOCKED_M30I_PINNED_QT_COMPAT_MISSING")
    old_qml_paths = e.get("QML_IMPORT_PATH", "")
    qml_paths = str(compat_qml) + (":" + old_qml_paths if old_qml_paths else "")
    environment = {**e,
        "QML_IMPORT_PATH": qml_paths,
        "QML2_IMPORT_PATH": qml_paths,
        "HOME": str(home),
        "XDG_CONFIG_HOME": str(home / ".config"),
        "XDG_DATA_HOME": str(home / ".local/share"),
        "XDG_CACHE_HOME": str(home / ".cache"),
        "QT_QPA_PLATFORM": "wayland",
        "QT_WAYLAND_DISABLE_WINDOWDECORATION": "1",
        "NO_COLOR": "1"}
    logfile = runtime / "m30i-real-company-quickshell.log"
    with logfile.open("w", encoding="utf-8") as stream:
        process = subprocess.Popen([str(qs), "--no-color", "-p", str(shell)],
            stdout=stream, stderr=subprocess.STDOUT,
            env=environment)
    expected = ["standard", "compact", "fullscreen", "closed"]
    last = 0
    results = []
    try:
        deadline = time.monotonic() + 55
        while len(results) < 4 and time.monotonic() < deadline:
            if process.poll() is not None:
                raise RuntimeError("M30I_QUICKSHELL_EXITED:\n" +
                                   logfile.read_text(errors="replace")[-8000:])
            text = logfile.read_text(errors="replace")
            lines = text.splitlines()
            for line in lines[last:]:
                if "SD_COMPANY_M30I_QML_GEOMETRY" not in line:
                    if re.search(r"(QQml|Type .* unavailable|module .* not installed|Error loading|Failed to load)", line, re.I):
                        # Some warnings may coexist with actual successful
                        # startup; keep logs as evidence, not auto-success.
                        print("M30I_DIAGNOSTIC", line[:350], flush=True)
                    continue
                m = GEOMETRY.search(line)
                if not m:
                    raise RuntimeError("M30I_QML_GEOMETRY_MALFORMED " + line)
                phase = expected[len(results)]
                if m.group(1) != phase:
                    raise RuntimeError(f"M30I_QML_PHASE_OUT_OF_ORDER {line}")
                print("SD_COMPANY_M30I_QML_RAW " + line.strip(), flush=True)
                result = check_geometry(m, phase)
                time.sleep(0.15)
                screenshot(grim, temp, result)
                results.append(result)
                print("SD_COMPANY_M30I_ACTUAL_QML_GEOMETRY " +
                      f"phase={phase} frame={result['frame']} "
                      f"canvas={result['canvas']} candidate=ZERO "
                      f"bridge=DISABLED PASS", flush=True)
            last = len(lines)
            time.sleep(0.12)
        if len(results) != 4:
            raise RuntimeError("M30I_INCOMPLETE_REAL_QML_GEOMETRY " +
                               logfile.read_text(errors="replace")[-10000:])
        # This test should launch only the real pinned Company PanelWindow,
        # not the MapLibre actor. A standalone XDG map root would mean
        # unintended sidecar launch from Company UI.
        tree = call(str(sway), "-s", ipc, "-r", "-t", "get_tree")
        if "SD-CI-CompanyVectorProbe-" in tree or "company-vector-probe" in tree:
            raise RuntimeError("M30I_UNEXPECTED_NATIVE_MAP_ELF_LAUNCH")
        print("SD_COMPANY_M30I_REAL_COMPANY_QUICKSHELL "
              "real_layer_shell=1 levels=standard,compact,fullscreen "
              "canvas_geometry_from_actual_pinned_qml=1 "
              "closed_candidate_revoked=1 native_child=NOT_STARTED "
              "bridge=DEFAULT_OFF maplibre=NOT_EXECUTED "
              "native_actor_scene=OFF input=OFF PASS", flush=True)
    finally:
        if process.poll() is None:
            process.terminate()
        try:
            process.wait(timeout=8)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait(timeout=8)


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print("SD_COMPANY_M30I_BLOCKED:", error, file=sys.stderr, flush=True)
        raise SystemExit(1)
