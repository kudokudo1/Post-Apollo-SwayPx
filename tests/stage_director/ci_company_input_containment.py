#!/usr/bin/env python3
"""M30h: real wlroots virtual-pointer -> Sway seat -> Qt input containment.

Four proof layers: real protocol ACK, actual Qt QPA eventFilter logs,
native mapped XDG/pidfd equality, and pre/post real compositor pixels.
A Sway focus command alone is never accepted as mouse input.
"""
from __future__ import annotations

import json
import os
from pathlib import Path
import re
import select
import subprocess
import sys
import time

from ci_company_overlay_prototype import call, nodes, ppm
from ci_company_source_crop import verify_pixels

FRAME = "SD-CI-CompanyWidgetFrame"
CROP = "SD-CI-CompanyVectorProbe-crop"
FRAME_RECT = (360, 180, 1100, 740)
CROP_RECT = (520, 300, 840, 540)
VIEWPORT = (520, 300, 600, 380)
EVENT = re.compile(r"SD_CI_COMPANY_QT_INPUT target=(crop|frame) "
                   r"event=(press|release|motion|wheel) "
                   r"global=(-?\d+),(-?\d+) local=(-?\d+),(-?\d+) "
                   r"rootlocal=(-?\d+),(-?\d+) "
                   r"buttons=(\d+) pid=(\d+)")

def near(x, y, pt, fuzz=5):
    return abs(x - pt[0]) <= fuzz and abs(y - pt[1]) <= fuzz


def read_until(stream, prefix: str, timeout: float = 8) -> str:
    end = time.monotonic() + timeout
    while time.monotonic() < end:
        ready, _, _ = select.select([stream], [], [],
                                    max(0.01, end - time.monotonic()))
        if not ready:
            continue
        line = stream.readline()
        if not line:
            raise RuntimeError("M30H_POINTER_PROCESS_CLOSED " + prefix)
        if line.startswith(prefix):
            return line.strip()
        if line.startswith("M30H_ERROR") or line.startswith("BLOCKED"):
            raise RuntimeError(line.strip())
    raise RuntimeError("M30H_POINTER_ACK_TIMEOUT " + prefix)


def main():
    e = os.environ
    temp = Path(e.get("RUNNER_TEMP", "/no-runner"))
    runtime = Path(e.get("XDG_RUNTIME_DIR", "/no-runtime"))
    qt = Path(e.get("STAGE_DIRECTOR_TEST_QT_COMPANY_WINDOW", "/no-qt"))
    pointer_bin = Path(e.get("STAGE_DIRECTOR_TEST_COMPANY_POINTER", "/no-pointer"))
    swaymsg = Path(e.get("STAGE_DIRECTOR_TEST_SWAYMSG", "/no-swaymsg"))
    grim = Path(e.get("STAGE_DIRECTOR_TEST_GRIM", "/no-grim"))
    socket = runtime / e.get("WAYLAND_DISPLAY", "/no-wayland")
    if not (e.get("GITHUB_ACTIONS") == "true" and e.get("CI") == "true"
            and e.get("STAGE_DIRECTOR_CI_COMPANY_INPUT") == "1"
            and e.get("STAGE_DIRECTOR_CI_COMPANY_CROP") == "1"
            and temp == Path("/home/runner/work/_temp")
            and str(runtime).startswith(str(temp) + "/")
            and runtime.is_dir() and socket.is_socket()
            and all(str(p).startswith("/nix/store/") and p.is_file()
                    for p in (qt, pointer_bin, swaymsg, grim))
            and os.getuid() != 0):
        raise RuntimeError("BLOCKED_M30H_NON_DISPOSABLE_POINTER_TEST")
    ipc_candidates = [p for p in runtime.glob("sway-ipc.*.sock")
                      if p.is_socket()]
    if len(ipc_candidates) != 1:
        raise RuntimeError("BLOCKED_M30H_PRIVATE_SWAY_IPC")
    ipc = str(ipc_candidates[0])
    processes = []
    pointer = None

    def tree():
        return json.loads(call(str(swaymsg), "-s", ipc, "-r", "-t", "get_tree"))

    def wait_xdg(title, pid, absent=False):
        for _ in range(100):
            views = [n for n in nodes(tree()) if n.get("name") == title]
            if absent and not views:
                return None
            if len(views) == 1 and views[0].get("pid") == pid:
                if views[0].get("shell") != "xdg_shell":
                    raise RuntimeError("M30H_NOT_WAYLAND_XDG")
                return views[0]
            time.sleep(0.1)
        raise RuntimeError("M30H_XDG_MISSING_OR_STALE " + title)

    def launch(which):
        logfile = runtime / f"sd-company-m30h-{which}.log"
        with logfile.open("w") as out:
            p = subprocess.Popen([str(qt), which], stdout=out,
                stderr=subprocess.STDOUT,
                env={**e, "QT_QPA_PLATFORM": "wayland",
                     "QT_WAYLAND_DISABLE_WINDOWDECORATION": "1"})
        processes.append(p)
        ready = (f"SD_CI_COMPANY_QT_QPA_READY case={which} pid={p.pid} "
                 "wayland_display=1")
        for _ in range(100):
            if p.poll() is not None:
                raise RuntimeError("M30H_QT_EXITED " + logfile.read_text()[-2400:])
            if ready in logfile.read_text():
                break
            time.sleep(0.1)
        else:
            raise RuntimeError("M30H_QT_NOT_READY " + logfile.read_text()[-2200:])
        view = wait_xdg(FRAME if which == "frame" else CROP, p.pid)
        fd = os.pidfd_open(p.pid, 0)
        try:
            info = Path(f"/proc/self/fdinfo/{fd}").read_text()
            if f"Pid:\t{p.pid}" not in info:
                raise RuntimeError("M30H_PIDFD_NATIVE_MISMATCH")
        finally:
            os.close(fd)
        return p, view, logfile

    def command(cid, directive):
        results = json.loads(call(str(swaymsg), "-s", ipc, "-r",
                                  f"[con_id={cid}] {directive}"))
        if not isinstance(results, list) or not results or not all(
                item.get("success") is True for item in results):
            raise RuntimeError(f"M30H_IPC_FAILED {directive} {results}")

    def place(title, pid, cid, geometry):
        x, y, w, h = geometry
        command(cid, "floating enable")
        command(cid, "border none")
        command(cid, f"resize set width {w} px height {h} px")
        command(cid, f"move position {x} {y}")
        for _ in range(100):
            rect = wait_xdg(title, pid).get("rect", {})
            if all(abs(rect.get(k, -9999) - value) <= 4
                    for k, value in zip(("x", "y", "width", "height"),
                                        geometry)):
                return
            time.sleep(0.1)
        raise RuntimeError(f"M30H_LAYOUT_MISMATCH {geometry} {rect}")

    def screenshot(stage, origin):
        file = temp / f"sd-company-m30h-{stage}.ppm"
        call(str(grim), "-t", "ppm", str(file), timeout=25)
        verify_pixels(ppm(file), stage, origin)

    def send(action):
        assert pointer is not None and pointer.stdin and pointer.stdout
        if pointer.poll() is not None:
            raise RuntimeError("M30H_VIRTUAL_POINTER_EXITED")
        pointer.stdin.write(action + "\n")
        pointer.stdin.flush()
        ack = read_until(pointer.stdout, "SD_CI_COMPANY_POINTER_ACK ", 10)
        if ack != "SD_CI_COMPANY_POINTER_ACK " + action:
            raise RuntimeError("M30H_VIRTUAL_POINTER_WRONG_ACK " + ack)
        time.sleep(0.09)

    try:
        frame, fv, frame_log = launch("frame")
        place(FRAME, frame.pid, fv["id"], FRAME_RECT)
        command(fv["id"], "focus")
        time.sleep(0.3)
        screenshot("frame", None)

        map_p, mv, crop_log = launch("crop")
        place(CROP, map_p.pid, mv["id"], CROP_RECT)
        command(mv["id"], "focus")
        time.sleep(0.45)
        compositor_log = (runtime / "sway.log").read_text(errors="replace")
        if "SD_CI_COMPANY_CROP_FRAME real_native_qt=1" not in compositor_log:
            raise RuntimeError("M30H_CROP_NOT_APPLIED")
        screenshot("first", (520, 300))

        pointer = subprocess.Popen([str(pointer_bin)], stdin=subprocess.PIPE,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
            bufsize=1, env=e)
        assert pointer.stdout is not None
        read_until(pointer.stdout,
                   "SD_CI_COMPANY_POINTER_READY real_zwlr_virtual_pointer=1")
        # Inside the *cropped* visible area; real pointer button, drag, scroll.
        for event in ("move 700 430", "press", "move 820 510",
                      "release", "wheel"):
            send(event)
        time.sleep(0.25)
        # Both points occupy the Qt 840x540 source rectangle BUT lie beyond
        # its 600x380 visible compositor crop. The frame must get these,
        # and the map must never see any press/wheel/motion there.
        for event in ("move 1210 450", "press", "release", "wheel",
                      "move 720 755", "press", "release", "wheel",
                      "move 400 260", "press", "release"):
            send(event)
        time.sleep(0.3)
        pointer.stdin.write("quit\n")
        pointer.stdin.flush()
        stop = read_until(pointer.stdout, "SD_CI_COMPANY_POINTER_STOP ")
        if "PASS" not in stop:
            raise RuntimeError("M30H_VIRTUAL_POINTER_NOT_CLEANED")
        pointer.wait(timeout=5)
        if pointer.returncode != 0:
            raise RuntimeError("M30H_VIRTUAL_POINTER_EXIT_FAILED")

        events = []
        for log, role, pid in ((crop_log, "crop", map_p.pid),
                               (frame_log, "frame", frame.pid)):
            data = log.read_text(errors="replace")
            for line in data.splitlines():
                match = EVENT.search(line)
                if match is None:
                    continue
                (target, event, qt_global_x, qt_global_y, lx, ly,
                 rootx, rooty, buttons, actual_pid) = match.groups()
                if target != role or int(actual_pid) != pid:
                    raise RuntimeError("M30H_EVENT_WRONG_QT_PID_OR_TARGET " + line)
                # QtWayland cannot know global window position. Compute
                # absolute desktop coordinates from Qt's actual ROOT-WIDGET
                # local position and Sway's independently observed XDG rect.
                # Never treat Qt "globalPosition" as authoritative.
                root_xy = (int(rootx), int(rooty))
                bounds = CROP_RECT if role == "crop" else FRAME_RECT
                if not (0 <= root_xy[0] < bounds[2] and
                        0 <= root_xy[1] < bounds[3]):
                    raise RuntimeError("M30H_QT_ROOT_LOCAL_OUT_OF_BOUNDS " + line)
                desktop_x = bounds[0] + root_xy[0]
                desktop_y = bounds[1] + root_xy[1]
                events.append((target, event, desktop_x, desktop_y, int(buttons)))
        qt_map = [r for r in events if r[0] == "crop"]
        qt_frame = [r for r in events if r[0] == "frame"]
        if not qt_map or not qt_frame:
            raise RuntimeError(f"M30H_QT_INPUT_MISSING map={qt_map} frame={qt_frame}")
        if not any(e == "press" and near(x, y, (700, 430))
                   for _, e, x, y, _ in qt_map):
            raise RuntimeError("M30H_INSIDE_QT_PRESS_NOT_DELIVERED " + str(qt_map))
        if not any(e == "motion" and held != 0 and near(x, y, (820, 510))
                   for _, e, x, y, held in qt_map):
            raise RuntimeError("M30H_INSIDE_DRAG_MOTION_NOT_DELIVERED " + str(qt_map))
        if not any(e == "release" and near(x, y, (820, 510))
                   for _, e, x, y, _ in qt_map):
            raise RuntimeError("M30H_INSIDE_RELEASE_NOT_DELIVERED " + str(qt_map))
        if not any(e == "wheel" and near(x, y, (820, 510))
                   for _, e, x, y, _ in qt_map):
            raise RuntimeError("M30H_INSIDE_QT_WHEEL_NOT_DELIVERED " + str(qt_map))

        # Do not accept lack of events as proof: require the OUTSIDE frame
        # receives at least one press on each adversarial overflow region.
        for point in ((1210, 450), (720, 755), (400, 260)):
            if not any(e == "press" and near(x, y, point)
                       for _, e, x, y, _ in qt_frame):
                raise RuntimeError(f"M30H_FRAME_NOT_RECEIVING_OUTSIDE_{point} "
                                   f"observed={qt_frame}")
        leakage = [(e, x, y) for _, e, x, y, _ in qt_map
                   if not (VIEWPORT[0] - 6 <= x < VIEWPORT[0] +
                           VIEWPORT[2] + 6 and
                           VIEWPORT[1] - 6 <= y < VIEWPORT[1] +
                           VIEWPORT[3] + 6)]
        if leakage:
            raise RuntimeError("M30H_POINTER_LEAKS_OUTSIDE_CROP " + str(leakage))

        map_p.terminate()
        map_p.wait(timeout=8)
        wait_xdg(CROP, map_p.pid, absent=True)
        time.sleep(0.3)
        screenshot("closed", None)
        if wait_xdg(FRAME, frame.pid)["id"] != fv["id"]:
            raise RuntimeError("M30H_FRAME_LOST")
        print("SD_COMPANY_M30H_QT_INPUT: real_virtual_pointer=1 "
              "qt_native_press_drag_release_wheel=PASS "
              "orange_overflow_events_to_map=0 "
              "outside_press_delivered_to_frame=3 "
              f"crop_events={len(qt_map)} frame_events={len(qt_frame)} "
              "actual_wayland_screenshot=PASS owner_authority=NONE "
              "maplibre=NOT_RUN actual_company_qml=NOT_RUN PASS",
              flush=True)
    finally:
        if pointer is not None and pointer.poll() is None:
            pointer.terminate()
            try:
                pointer.wait(timeout=5)
            except subprocess.TimeoutExpired:
                pointer.kill()
                pointer.wait(timeout=5)
        for p in reversed(processes):
            if p.poll() is None:
                p.terminate()
        for p in reversed(processes):
            try:
                p.wait(timeout=6)
            except subprocess.TimeoutExpired:
                p.kill()
                p.wait(timeout=6)


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print("SD_COMPANY_M30H_BLOCKED:", exc, file=sys.stderr, flush=True)
        raise SystemExit(1)
