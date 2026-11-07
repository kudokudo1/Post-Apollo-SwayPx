#!/usr/bin/env python3
"""M30j: real Company Quickshell + native Qt crop + actual Wayland input.

One private headless SwayPX compositor holds BOTH the original source-pinned
CompanyNetworkW.qml and an oversized 840x540 native Qt Wayland client.
Actual runtime QML layout must match M30i's standard 573x369 Canvas. An
isolated SwayPX-only map hook moves ONLY the kernel-matched Qt test actor
above the mapped real Company layer, cropped to that exact Canvas.

No fake 'native running' flag, selected production host, MapLibre, user
desktop, credentials, online tiles or production surface authority.
"""
from __future__ import annotations

import os
from pathlib import Path
import re
import subprocess
import sys
import time
import json

import ci_company_real_quickshell as real
from ci_company_overlay_prototype import call, nodes, ppm, rgb
from ci_company_input_containment import EVENT, read_until, near
from ci_company_source_crop import orange

TITLE = "SD-CI-CompanyVectorProbe-crop"
CANVAS_SIZE = (573, 369)
QT_SOURCE = (840, 540)
READY_MARKER = "SD_CI_COMPANY_M30J_REAL_PANEL "
FRAME_LOG = re.compile(
    r"SD_COMPANY_M30J_COMPANY_FRAME_PRESS x=(-?\d+) y=(-?\d+)")
CYAN = (85, 207, 202)


def pixel_distance(a, b) -> int:
    return sum(abs(int(x) - int(y)) for x, y in zip(a, b))


def canvas_points(origin):
    x, y = origin
    return [(x + 32, y + 32), (x + 200, y + 110),
            (x + 400, y + 270), (x + 540, y + 335)]


def check_display_change(before, during, origin):
    # PPM samples are genuine compositor output, NOT fabricated map pixels.
    for x, y in canvas_points(origin):
        value = rgb(during, x, y)
        if pixel_distance(value, CYAN) > 24:
            raise RuntimeError(f"M30J_REAL_QT_NOT_VISIBLE_AT_CANVAS {x},{y} "
                               f"actual={value} expected={CYAN}")
    # Right/bottom are STILL inside the source Qt window but outside the
    # 573x369 real Company Canvas. Require exact background restoration.
    x, y = origin
    outside = [(x + 588, y + 100), (x + 110, y + 385),
               (x - 30, y + 180), (x + 300, y - 25)]
    for px, py in outside:
        pixel = rgb(during, px, py)
        old = rgb(before, px, py)
        if pixel_distance(pixel, old) > 12:
            raise RuntimeError(f"M30J_REAL_COMPANY_FRAME_OBSCURED {px},{py} "
                               f"before={old} after={pixel}")
        if orange(pixel):
            raise RuntimeError(f"M30J_ORANGE_OVERFLOW_AT_{px}_{py}")
    print("SD_COMPANY_M30J_NATIVE_PIXELS inside_qt_cyan=4 "
          "outside_frame_unchanged=4 orange_overflow=0 "
          "real_company_layer=1 PASS", flush=True)


def check_restored(before, after, origin):
    for x, y in canvas_points(origin):
        if pixel_distance(rgb(before, x, y), rgb(after, x, y)) > 12:
            raise RuntimeError(f"M30J_NATIVE_SURFACE_STALE_AFTER_CLOSE {x},{y}")
    print("SD_COMPANY_M30J_RESTORED actual_company_canvas=1 "
          "qt_stale_pixels=0 PASS", flush=True)


def check_input(qt_file: Path, company_file: Path, actor_pid: int,
                source_origin, canvas):
    events = []
    for line in qt_file.read_text(errors="replace").splitlines():
        m = EVENT.search(line)
        if not m:
            continue
        (target, name, qgx, qgy, lx, ly, root_x, root_y,
         buttons, pid) = m.groups()
        if target != "crop" or int(pid) != actor_pid:
            raise RuntimeError("M30J_NATIVE_INPUT_WRONG_PROCESS " + line)
        px = source_origin[0] + int(root_x)
        py = source_origin[1] + int(root_y)
        events.append((name, px, py, int(buttons)))
    x, y = source_origin
    expect = (x + 140, y + 120)
    move_to = (x + 260, y + 190)
    required = (("press", expect), ("motion", move_to),
                ("release", move_to), ("wheel", move_to))
    for name, pt in required:
        if not any(event == name and near(px, py, pt, 7) and
                   (name != "motion" or held != 0)
                   for event, px, py, held in events):
            raise RuntimeError(f"M30J_QT_INPUT_MISSING_{name} events={events}")
    # Input in source overflow is FORBIDDEN even though actual QWidget
    # dimensions extend into that screen region.
    x, y, w, h = canvas
    leakage = [(event, px, py) for event, px, py, _ in events
               if not (x - 6 <= px < x + w + 6 and
                       y - 6 <= py < y + h + 6)]
    if leakage:
        raise RuntimeError("M30J_QT_EVENTS_OUTSIDE_REAL_CANVAS " + str(leakage))
    frame_events = []
    for line in company_file.read_text(errors="replace").splitlines():
        m = FRAME_LOG.search(line)
        if m:
            frame_events.append((int(m.group(1)), int(m.group(2))))
    # MouseArea is *inside* CompanyFrame at frame-local (0,0). Positives
    # show a real QML receiving layer, not absence of map events alone.
    frame_origin = (real.EXPECTED["standard"]["origin"][0] + 12,
                    real.EXPECTED["standard"]["origin"][1] + 9)
    outside = [(x + w + 17, y + 90), (x + 160, y + h + 19)]
    for expected in outside:
        if not any(near(frame_origin[0] + px, frame_origin[1] + py,
                        expected, 10) for px, py in frame_events):
            raise RuntimeError(f"M30J_COMPANY_OUTSIDE_CLICK_NOT_DELIVERED "
                               f"expect={expected} observed={frame_events}")
    print("SD_COMPANY_M30J_NATIVE_QT_INPUT real_wayland_press_drag_release_wheel=1 "
          "real_quickshell_outside_clicks=2 "
          "qt_hidden_overflow_events=0 PASS", flush=True)


def main():
    env, temp, runtime, checkout, qs, swaymsg, grim, ipc = real.guard()
    if not (env.get("STAGE_DIRECTOR_CI_COMPANY_M30J") == "1"
            and env.get("STAGE_DIRECTOR_CI_COMPANY_CROP") == "1"
            and env.get("STAGE_DIRECTOR_CI_COMPANY_INPUT") == "1"
            and env.get("STAGE_DIRECTOR_CI_COMPANY_QUICKSHELL") == "1"):
        raise RuntimeError("BLOCKED_M30J_NOT_EXPLICIT_GITHUB_TEST")
    qt = Path(env.get("STAGE_DIRECTOR_TEST_QT_COMPANY_WINDOW", "/missing"))
    pointer_path = Path(env.get("STAGE_DIRECTOR_TEST_COMPANY_POINTER", "/missing"))
    if not all(str(p).startswith("/nix/store/") and p.is_file()
               for p in (qt, pointer_path)):
        raise RuntimeError("BLOCKED_M30J_UNPINNED_NATIVE_PROBE")
    home = temp / "sd-company-m30j-home"
    for path in (home, home / ".config", home / ".cache", home / ".local/share"):
        path.mkdir(parents=True, exist_ok=True, mode=0o700)
    compat = Path(env.get("STAGE_DIRECTOR_M30I_QML_IMPORT_PATH", "/no-qt"))
    if not (str(compat).startswith("/nix/store/") and
            (compat / "Qt5Compat/GraphicalEffects/qmldir").is_file()):
        raise RuntimeError("BLOCKED_M30J_QT_COMPAT")
    qmlpath = str(compat) + (":" + env["QML_IMPORT_PATH"]
                              if env.get("QML_IMPORT_PATH") else "")
    qsenv = {**env, "HOME": str(home),
             "XDG_CONFIG_HOME": str(home / ".config"),
             "XDG_DATA_HOME": str(home / ".local/share"),
             "XDG_CACHE_HOME": str(home / ".cache"),
             "QML_IMPORT_PATH": qmlpath, "QML2_IMPORT_PATH": qmlpath,
             "QT_QPA_PLATFORM": "wayland", "NO_COLOR": "1"}
    shell = real.prepare(checkout, temp / "sd-company-m30j")
    source = Path("tests/stage_director/ci_company_m30j_real_shell.qml")
    if source.is_symlink() or not source.is_file():
        raise RuntimeError("BLOCKED_M30J_MISSING_QML_FIXTURE")
    # Only additive test observability, in the disposable COPY.
    network = shell.parent / "widgets/CompanyNetworkW.qml"
    before = network.read_text(encoding="utf-8")
    anchor = "WindowPanelFrame {\n        id: companyFrame\n"
    if before.count(anchor) != 1:
        raise RuntimeError("M30J_REAL_COMPANY_FRAME_SOURCE_DRIFT")
    extra = (
        "        // M30j temporary positive outside-click witness.\n"
        "        MouseArea {\n"
        "            anchors.fill: parent\n"
        "            z: 9999\n"
        "            acceptedButtons: Qt.LeftButton\n"
        "            onPressed: function(mouse) {\n"
        "                console.log(\"SD_COMPANY_M30J_COMPANY_FRAME_PRESS x=\" +\n"
        "                    Math.round(mouse.x) + \" y=\" + Math.round(mouse.y));\n"
        "                mouse.accepted = true;\n"
        "            }\n"
        "        }\n")
    network.write_text(before.replace(anchor, anchor + extra, 1),
                       encoding="utf-8")
    shell.write_text(source.read_text(encoding="utf-8"), encoding="utf-8")
    qslog = runtime / "sd-company-m30j-real-company.log"
    qtlog = runtime / "sd-company-m30j-real-qt.log"
    process = None
    actor = None
    pointer = None

    def screenshot(stage):
        filename = temp / f"sd-company-m30j-{stage}.ppm"
        call(str(grim), "-t", "ppm", str(filename), timeout=25)
        return ppm(filename)

    def tree():
        return json.loads(call(str(swaymsg), "-s", ipc, "-r", "-t", "get_tree"))

    def wait_actor(pid, absent=False):
        for _ in range(90):
            matches = [n for n in nodes(tree()) if n.get("name") == TITLE]
            if absent and not matches:
                return None
            if len(matches) == 1 and matches[0].get("pid") == pid:
                if matches[0].get("shell") != "xdg_shell":
                    raise RuntimeError("M30J_ACTOR_NOT_NATIVE_XDG")
                return matches[0]
            time.sleep(0.12)
        raise RuntimeError("M30J_ACTOR_NOT_IDENTIFIED")

    def run_sway(conid, instruction):
        result = json.loads(call(str(swaymsg), "-s", ipc, "-r",
                                 f"[con_id={conid}] {instruction}"))
        if not isinstance(result, list) or not result or not all(
                x.get("success") is True for x in result):
            raise RuntimeError(f"M30J_SWAY_IPC_REFUSED {instruction} {result}")

    def send(cmd):
        assert pointer and pointer.stdin and pointer.stdout
        pointer.stdin.write(cmd + "\n")
        pointer.stdin.flush()
        marker = read_until(pointer.stdout, "SD_CI_COMPANY_POINTER_ACK ", 8)
        if marker != "SD_CI_COMPANY_POINTER_ACK " + cmd:
            raise RuntimeError("M30J_WAYLAND_POINTER_WRONG_ACK " + marker)
        time.sleep(0.11)

    try:
        with qslog.open("w") as out:
            process = subprocess.Popen([str(qs), "--no-color", "-p", str(shell)],
                stdout=out, stderr=subprocess.STDOUT, env=qsenv)
        qml = None
        deadline = time.monotonic() + 13
        while time.monotonic() < deadline:
            if process.poll() is not None:
                raise RuntimeError("M30J_REAL_COMPANY_EXITED " +
                                   qslog.read_text(errors="replace")[-3500:])
            for line in qslog.read_text(errors="replace").splitlines():
                m = real.GEOMETRY.search(line)
                if m and m.group(1) == "standard":
                    qml = real.check_geometry(m, "standard")
                    break
            if qml:
                break
            time.sleep(0.12)
        if qml is None:
            raise RuntimeError("M30J_REAL_COMPANY_QML_GEOMETRY_MISSING " +
                               qslog.read_text(errors="replace")[-3500:])
        x, y, w, h = qml["canvas"]
        if (w, h) != CANVAS_SIZE:
            raise RuntimeError("M30J_QML_CANVAS_CHANGED_REQUIRES_NEW_CROP_PIN")
        panel_x, panel_y = qml["origin"]
        actual = (panel_x + x, panel_y + y, w, h)
        if actual != (626, 390, 573, 369):
            raise RuntimeError("M30J_UNEXPECTED_STANDARD_CANVAS_ORIGIN")
        time.sleep(0.2)
        baseline = screenshot("real-company")
        print(f"SD_COMPANY_M30J_REAL_SOURCE_GEOMETRY "
              f"CompanyCanvasGlobal={actual} frame={qml['frame']} "
              "bridge=OFF real_map_child=ABSENT candidate=ZERO PASS",
              flush=True)
        with qtlog.open("w") as out:
            actor = subprocess.Popen([str(qt), "crop"], stdout=out,
                stderr=subprocess.STDOUT,
                env={**env, "QT_QPA_PLATFORM": "wayland",
                     "QT_WAYLAND_DISABLE_WINDOWDECORATION": "1"})
        marker = f"SD_CI_COMPANY_QT_QPA_READY case=crop pid={actor.pid}"
        for _ in range(90):
            if actor.poll() is not None:
                raise RuntimeError("M30J_NATIVE_QT_EXIT " +
                                   qtlog.read_text()[-3000:])
            if marker in qtlog.read_text():
                break
            time.sleep(0.08)
        else:
            raise RuntimeError("M30J_QT_NOT_READY")
        node = wait_actor(actor.pid)
        # Real kernel pidfd, not title or IPC numeric PID alone.
        fd = os.pidfd_open(actor.pid, 0)
        try:
            if f"Pid:\t{actor.pid}" not in Path(f"/proc/self/fdinfo/{fd}").read_text():
                raise RuntimeError("M30J_NATIVE_ACTOR_PIDFD_MISMATCH")
        finally:
            os.close(fd)
        mid = node["id"]
        run_sway(mid, "floating enable")
        run_sway(mid, "border none")
        run_sway(mid, f"resize set width {QT_SOURCE[0]} px height {QT_SOURCE[1]} px")
        run_sway(mid, f"move position {actual[0]} {actual[1]}")
        for _ in range(90):
            r = wait_actor(actor.pid).get("rect", {})
            if all(abs(r.get(field, -9999) - value) <= 4 for field, value
                   in zip(("x", "y", "width", "height"),
                          (actual[0], actual[1], *QT_SOURCE))):
                break
            time.sleep(0.09)
        else:
            raise RuntimeError("M30J_QT_SOURCE_GEOMETRY_NOT_PRESERVED " + str(r))
        time.sleep(0.5)
        if READY_MARKER not in (runtime / "sway.log").read_text(errors="replace"):
            raise RuntimeError("M30J_CI_COMPOSITOR_REFUSED_REAL_COMPANY_LAYER " +
                               (runtime / "sway.log").read_text()[-2500:])
        during = screenshot("real-company-plus-native")
        check_display_change(baseline, during, actual[:2])
        pointer = subprocess.Popen([str(pointer_path)], stdin=subprocess.PIPE,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            text=True, bufsize=1, env=env)
        assert pointer.stdout is not None
        read_until(pointer.stdout,
                   "SD_CI_COMPANY_POINTER_READY real_zwlr_virtual_pointer=1")
        inside = (actual[0] + 140, actual[1] + 120)
        drag = (actual[0] + 260, actual[1] + 190)
        right = (actual[0] + w + 17, actual[1] + 90)
        bottom = (actual[0] + 160, actual[1] + h + 19)
        for action in (f"move {inside[0]} {inside[1]}", "press",
                       f"move {drag[0]} {drag[1]}", "release", "wheel",
                       f"move {right[0]} {right[1]}", "press", "release",
                       f"move {bottom[0]} {bottom[1]}", "press", "release"):
            send(action)
        pointer.stdin.write("quit\n")
        pointer.stdin.flush()
        stop = read_until(pointer.stdout, "SD_CI_COMPANY_POINTER_STOP ")
        pointer.wait(timeout=5)
        if "PASS" not in stop or pointer.returncode != 0:
            raise RuntimeError("M30J_POINTER_HELPER_NOT_CLEAN")
        time.sleep(0.25)
        check_input(qtlog, qslog, actor.pid, actual[:2], actual)
        actor.terminate()
        actor.wait(timeout=8)
        wait_actor(actor.pid, absent=True)
        time.sleep(0.35)
        check_restored(baseline, screenshot("native-closed"), actual[:2])
        # The actual Company ShellRoot, not a fake frame, controls close.
        deadline = time.monotonic() + 30
        while time.monotonic() < deadline:
            for line in qslog.read_text(errors="replace").splitlines():
                m = real.GEOMETRY.search(line)
                if m and m.group(1) == "closed":
                    if (tuple(map(int, m.groups()[5:9])) != (0,0,0,0)
                            or tuple(map(int, m.groups()[9:13])) != (0,0,0,0)
                            or tuple(map(int, m.groups()[13:17])) != (0,0,0,0)
                            or m.group(17) != "0"):
                        raise RuntimeError("M30J_REAL_COMPANY_CLOSE_STALE")
                    screenshot("company-closed")
                    print("SD_COMPANY_M30J_REAL_COMPANY_CLOSED "
                          "frame=ZERO canvas=ZERO candidate=ZERO bridge=OFF "
                          "map_unmapped=1 PASS", flush=True)
                    print("SD_COMPANY_M30J_COMBINED real_company_qml=1 "
                          "native_qt_source=840x540 crop_from_company=573x369 "
                          "real_wayland_input=1 positive_company_clicks=2 "
                          "maplibre=NOT_RUN selected_production_host=NONE "
                          "scene_and_input=CI_ONLY PASS", flush=True)
                    return
            time.sleep(0.12)
        raise RuntimeError("M30J_REAL_COMPANY_NEVER_CLOSED")
    finally:
        if pointer and pointer.poll() is None:
            pointer.terminate()
        if actor and actor.poll() is None:
            actor.terminate()
        if process and process.poll() is None:
            process.terminate()
        for p in (pointer, actor, process):
            if p:
                try:
                    p.wait(timeout=7)
                except subprocess.TimeoutExpired:
                    p.kill()
                    p.wait(timeout=7)


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print("SD_COMPANY_M30J_BLOCKED:", str(exc), file=sys.stderr, flush=True)
        raise SystemExit(1)
