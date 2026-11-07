#!/usr/bin/env python3
"""M30g CI ONLY: verify real 840x540 Qt buffer cropped to 600x380.

Unlike the passing M30f placement probe, the native source never shrinks.
Orange Qt child widgets occupy every region OUTSIDE the intended viewport.
Real SwayPX screenshots must show cyan only inside the crop, no orange at
all, and full original frame restoration after the actor exits.

Never runs against the operator's workstation or real Company/MapLibre.
"""
from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import sys
import time

from ci_company_overlay_prototype import call, ppm, rgb, cyan, purple, black, nodes

FRAME = "SD-CI-CompanyWidgetFrame"
ACTOR = "SD-CI-CompanyVectorProbe-crop"
FRAME_RECT = (360, 180, 1100, 740)
SOURCE_RECTS = ((520, 300, 840, 540), (650, 350, 840, 540))
CLIP_SIZE = (600, 380)


def orange(pixel):
    return pixel[0] > 120 and pixel[1] > 65 and pixel[2] < 110 and pixel[0] > 1.5 * pixel[2]


def verify_pixels(image, stage, origin=None):
    expected = {
        "frame": [(375, 240, purple), (900, 600, purple),
                  (45, 45, black)],
        "first": [(375, 240, purple), (600, 350, cyan),
                  (850, 570, cyan), (1135, 460, purple),
                  (700, 715, purple), (45, 45, black)],
        "moved": [(375, 240, purple), (600, 350, purple),
                  (700, 400, cyan), (1050, 630, cyan),
                  (1265, 450, purple), (850, 760, purple),
                  (45, 45, black)],
        "closed": [(375, 240, purple), (600, 350, purple),
                   (1050, 630, purple), (45, 45, black)],
    }
    tests = expected.get(stage)
    if tests is None:
        raise ValueError("INVALID_CROP_SCREENSHOT_PHASE")
    for x, y, predicate in tests:
        actual = rgb(image, x, y)
        if not predicate(actual):
            raise AssertionError(f"M30G_{stage}_PIXEL_FAIL at={x},{y} "
                                 f"actual={actual} expected={predicate.__name__}")
    leaked = 0
    orange_count = 0
    internal = 0
    for y in range(16, 1080, 16):
        for x in range(16, 1920, 16):
            pixel = rgb(image, x, y)
            if orange(pixel):
                orange_count += 1
            if not cyan(pixel):
                continue
            expected_inside = (origin is not None and
                               origin[0] <= x < origin[0] + CLIP_SIZE[0] and
                               origin[1] <= y < origin[1] + CLIP_SIZE[1])
            if expected_inside:
                internal += 1
            else:
                leaked += 1
    if leaked != 0 or orange_count != 0 or (
            origin is not None and internal < 400) or (
            origin is None and internal != 0):
        raise AssertionError(f"M30G_{stage}_SOURCE_CLIP_FAIL "
                             f"outside_cyan={leaked} orange={orange_count} "
                             f"inside_cyan={internal}")
    print(f"SD_COMPANY_M30G_ACTUAL_PIXELS stage={stage} source=840x540 "
          f"viewport=600x380 outside_cyan={leaked} "
          f"orange_overflow={orange_count} inside_cyan={internal} PASS",
          flush=True)


def main():
    env = os.environ
    temp = Path(env.get("RUNNER_TEMP", "/not-ci"))
    runtime = Path(env.get("XDG_RUNTIME_DIR", "/not-runtime"))
    qt = Path(env.get("STAGE_DIRECTOR_TEST_QT_COMPANY_WINDOW", "/not-qt"))
    swaymsg = Path(env.get("STAGE_DIRECTOR_TEST_SWAYMSG", "/not-swaymsg"))
    grim = Path(env.get("STAGE_DIRECTOR_TEST_GRIM", "/not-grim"))
    socket = runtime / env.get("WAYLAND_DISPLAY", "/not-wayland")
    if not (env.get("GITHUB_ACTIONS") == "true" and env.get("CI") == "true"
            and env.get("STAGE_DIRECTOR_CI_COMPANY_CROP") == "1"
            and temp == Path("/home/runner/work/_temp")
            and str(runtime).startswith(str(temp) + "/")
            and runtime.is_dir() and socket.is_socket()
            and str(qt).startswith("/nix/store/") and qt.is_file()
            and str(swaymsg).startswith("/nix/store/") and swaymsg.is_file()
            and str(grim).startswith("/nix/store/") and grim.is_file()
            and os.getuid() != 0):
        raise RuntimeError("BLOCKED_M30G_NOT_ISOLATED_GITHUB_QT")

    ipc = [x for x in runtime.glob("sway-ipc.*.sock") if x.is_socket()]
    if len(ipc) != 1:
        raise RuntimeError("BLOCKED_M30G_PRIVATE_IPC_NOT_UNIQUE")
    ipc = str(ipc[0])
    procs: list[subprocess.Popen] = []

    def tree():
        return json.loads(call(str(swaymsg), "-s", ipc, "-r", "-t", "get_tree"))

    def wait_view(title, pid, *, absent=False):
        for _ in range(100):
            found = [n for n in nodes(tree()) if n.get("name") == title]
            if absent and not found:
                return None
            if len(found) == 1 and found[0].get("pid") == pid:
                if found[0].get("shell") != "xdg_shell" or found[0].get("window") is not None:
                    raise RuntimeError("M30G_NOT_NATIVE_WAYLAND_QPA")
                return found[0]
            time.sleep(0.1)
        raise RuntimeError("M30G_XDG_NOT_MAPPED_OR_UNMAPPED " + title)

    def launch(which):
        path = runtime / f"m30g-{which}.log"
        with path.open("w") as stream:
            p = subprocess.Popen([str(qt), which], stdout=stream,
                                 stderr=subprocess.STDOUT,
                                 env={**env, "QT_QPA_PLATFORM": "wayland",
                                      "QT_WAYLAND_DISABLE_WINDOWDECORATION": "1"})
        procs.append(p)
        ready = f"SD_CI_COMPANY_QT_QPA_READY case={which} pid={p.pid} wayland_display=1"
        for _ in range(100):
            if p.poll() is not None:
                raise RuntimeError("M30G_QT_EXITED " + path.read_text()[-1600:])
            if ready in path.read_text():
                break
            time.sleep(0.1)
        else:
            raise RuntimeError("M30G_NO_NATIVE_QT_QPA " + path.read_text()[-2000:])
        title = FRAME if which == "frame" else ACTOR
        n = wait_view(title, p.pid)
        fd = os.pidfd_open(p.pid, 0)
        try:
            if f"Pid:\t{p.pid}" not in Path(f"/proc/self/fdinfo/{fd}").read_text():
                raise RuntimeError("M30G_KERNEL_PIDFD_MISMATCH")
        finally:
            os.close(fd)
        return p, n

    def command(con_id, directive):
        output = json.loads(call(str(swaymsg), "-s", ipc, "-r",
                                 f"[con_id={con_id}] {directive}"))
        if not isinstance(output, list) or not output or not all(
                x.get("success") is True for x in output):
            raise RuntimeError(f"M30G_IPC_REFUSED {directive}: {output}")

    def place(title, pid, con_id, geometry):
        x, y, w, h = geometry
        command(con_id, "floating enable")
        command(con_id, "border none")
        command(con_id, f"resize set width {w} px height {h} px")
        command(con_id, f"move position {x} {y}")
        for _ in range(100):
            node = wait_view(title, pid)
            r = node.get("rect", {})
            if all(abs(r.get(k, -10000) - value) <= 4
                   for k, value in zip(("x", "y", "width", "height"), geometry)):
                return node
            time.sleep(0.1)
        raise RuntimeError(f"M30G_SOURCE_GEOMETRY_CHANGED expected={geometry} actual={r}")

    def screenshot(stage, source=None):
        path = temp / f"sd-company-m30g-{stage}.ppm"
        call(str(grim), "-t", "ppm", str(path), timeout=25)
        verify_pixels(ppm(path), stage, source)

    try:
        frame, view = launch("frame")
        fid = view["id"]
        place(FRAME, frame.pid, fid, FRAME_RECT)
        command(fid, "focus")
        time.sleep(0.5)
        screenshot("frame")
        actor, native = launch("crop")
        mid = native["id"]
        place(ACTOR, actor.pid, mid, SOURCE_RECTS[0])
        command(mid, "focus")
        time.sleep(0.6)
        if wait_view(ACTOR, actor.pid).get("id") != mid:
            raise RuntimeError("M30G_ACTOR_ID_CHANGED")
        compositor_log = (runtime / "sway.log").read_text(errors="replace")
        if "SD_CI_COMPANY_CROP_FRAME real_native_qt=1" not in compositor_log:
            raise RuntimeError("M30G_SOURCE_CROP_NOT_REACHED " +
                               compositor_log[-2400:])
        screenshot("first", SOURCE_RECTS[0][:2])
        place(ACTOR, actor.pid, mid, SOURCE_RECTS[1])
        command(mid, "focus")
        time.sleep(0.6)
        if wait_view(ACTOR, actor.pid).get("id") != mid:
            raise RuntimeError("M30G_ACTOR_ID_CHANGED_ON_MOVE")
        screenshot("moved", SOURCE_RECTS[1][:2])
        actor.terminate()
        actor.wait(timeout=8)
        wait_view(ACTOR, actor.pid, absent=True)
        time.sleep(0.5)
        screenshot("closed")
        if wait_view(FRAME, frame.pid)["id"] != fid:
            raise RuntimeError("M30G_FRAME_LOST")
        print("SD_COMPANY_M30G_SOURCE_CROP real_qt_xdg=2 "
              "source_geometry_preserved=840x540 visible=600x380 "
              "native_root_moved_same_id=1 focus_selected=1 "
              "real_screenshots=4 orange_outside=0 stale_pixels=0 "
              "maplibre=NOT_RUN company_qml=NOT_RUN "
              "pointer_delivery=NOT_TESTED popup_containment=NOT_TESTED "
              "authority=CI_ONLY PASS", flush=True)
    finally:
        for p in reversed(procs):
            if p.poll() is None:
                p.terminate()
        for p in reversed(procs):
            try:
                p.wait(timeout=6)
            except subprocess.TimeoutExpired:
                p.kill()
                p.wait(timeout=6)


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print("SD_COMPANY_M30G_FAIL:", error, file=sys.stderr, flush=True)
        raise SystemExit(1)
