#!/usr/bin/env python3
"""M30f CI-only real Qt XDG overlay geometry/pixel experiment.

This deliberately does NOT embed a MapLibre ELF into Quickshell or grant
scene authority. It tests the cheapest visible "put the Qt window there"
route with an independently painted synthetic Company-sized frame.
No production configuration, CI compositor source, local Fedora or client
company records are modified.
"""
from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import sys
import time

FRAME_TITLE = "SD-CI-CompanyWidgetFrame"
MAP_TITLE = "SD-CI-CompanyVectorProbe-map"
FRAME_RECT = (360, 180, 1100, 740)
MAP_RECT_1 = (520, 300, 600, 380)
MAP_RECT_2 = (520, 300, 780, 460)


def call(*cmd: str, timeout: int = 20) -> str:
    response = subprocess.run(cmd, text=True, capture_output=True,
                              timeout=timeout, check=False)
    if response.returncode != 0:
        raise RuntimeError("COMMAND_FAILED: " + " ".join(cmd[:3]) +
                           "\n" + response.stderr[-2000:] +
                           "\n" + response.stdout[-1200:])
    return response.stdout.strip()


def ppm(path: Path):
    data = path.read_bytes()
    pos = 0

    def next_token():
        nonlocal pos
        while pos < len(data):
            if data[pos] == ord("#"):
                pos = data.find(b"\n", pos) + 1
                if pos == 0:
                    raise ValueError("PPM_UNTERMINATED_COMMENT")
            elif data[pos] in b" \t\r\n":
                pos += 1
            else:
                break
        end = pos
        while end < len(data) and data[end] not in b" \t\r\n#":
            end += 1
        if end == pos:
            raise ValueError("PPM_MISSING_TOKEN")
        token = data[pos:end]
        pos = end
        return token

    if next_token() != b"P6":
        raise ValueError("PPM_NOT_RAW_RGB")
    width, height, scale = (int(next_token()), int(next_token()),
                            int(next_token()))
    if width != 1920 or height != 1080 or scale != 255:
        raise ValueError(f"PPM_UNEXPECTED_HEADER {width}x{height} max={scale}")
    if pos >= len(data) or data[pos] not in b" \t\r\n":
        raise ValueError("PPM_MISSING_HEADER_DELIMITER")
    pos += 1
    if len(data) - pos != width * height * 3:
        raise ValueError("PPM_INCORRECT_RGB_LENGTH")
    return width, height, memoryview(data)[pos:]


def rgb(image, x: int, y: int):
    width, height, pixels = image
    if not (0 <= x < width and 0 <= y < height):
        raise AssertionError("PIXEL_OUTSIDE_OUTPUT")
    start = 3 * (y * width + x)
    return tuple(pixels[start:start + 3])


def cyan(c):
    return c[1] > 145 and c[2] > 130 and c[1] > c[0] * 1.35


def purple(c):
    return 10 <= c[0] < 80 and c[1] < 60 and 15 <= c[2] < 110


def black(c):
    return max(c) < 50


def check_pixels(image, *, stage: str):
    """Fail when the actual screenshot does not show its expected layers."""
    sample = {
        "frame": [(375, 240, purple), (900, 600, purple),
                  (45, 45, black)],
        "initial": [(375, 240, purple), (600, 350, cyan),
                    (850, 570, cyan), (1200, 500, purple),
                    (45, 45, black)],
        "expanded": [(375, 240, purple), (600, 350, cyan),
                     (850, 570, cyan), (1200, 500, cyan),
                     (1350, 570, purple), (45, 45, black)],
        "closed": [(375, 240, purple), (600, 350, purple),
                   (850, 570, purple), (1200, 500, purple),
                   (45, 45, black)],
    }
    if stage not in sample:
        raise ValueError("UNKNOWN_PIXEL_STAGE")
    for x, y, pred in sample[stage]:
        color = rgb(image, x, y)
        if not pred(color):
            raise AssertionError(
                f"PIXEL_{stage.upper()}_FAIL x={x} y={y} "
                f"rgb={color} expected={pred.__name__}")
    # The compositor's screenshot must not show cyan outside the intended
    # stand-in map rectangle. This is bounded viewport placement, NOT
    # source clipping of an oversized surface.
    active = (MAP_RECT_1 if stage == "initial" else MAP_RECT_2
              if stage == "expanded" else None)
    leaking = 0
    inside = 0
    for y in range(16, 1080, 16):
        for x in range(16, 1920, 16):
            if not cyan(rgb(image, x, y)):
                continue
            expected = (active is not None and
                        active[0] - 8 <= x < active[0] + active[2] + 8 and
                        active[1] - 8 <= y < active[1] + active[3] + 8)
            if expected:
                inside += 1
            else:
                leaking += 1
    if leaking != 0 or (active is not None and inside < 400) or (
            active is None and inside != 0):
        raise AssertionError(
            f"PIXEL_{stage.upper()}_LEAK_OR_EMPTY "
            f"unexpected_cyan={leaking} inside_cyan={inside}")
    print(f"SD_COMPANY_M30F_PIXELS stage={stage} "
          f"real_output=1920x1080 unexpected_cyan={leaking} "
          f"inside_cyan={inside} PASS", flush=True)


def nodes(root: dict):
    yield root
    for kind in ("nodes", "floating_nodes"):
        for child in root.get(kind, []):
            yield from nodes(child)


def main():
    env = os.environ
    runtime = Path(env.get("XDG_RUNTIME_DIR", "/no-runtime"))
    temp = Path(env.get("RUNNER_TEMP", "/no-runner"))
    qt = Path(env.get("STAGE_DIRECTOR_TEST_QT_COMPANY_WINDOW", "/no-qt"))
    swaymsg = Path(env.get("STAGE_DIRECTOR_TEST_SWAYMSG", "/no-swaymsg"))
    grim = Path(env.get("STAGE_DIRECTOR_TEST_GRIM", "/no-grim"))
    display = env.get("WAYLAND_DISPLAY", "")
    if not (env.get("GITHUB_ACTIONS") == "true" and
            env.get("CI") == "true" and
            runtime.is_dir() and str(runtime).startswith(str(temp) + "/") and
            display.startswith("wayland-") and (runtime / display).is_socket() and
            str(qt).startswith("/nix/store/") and qt.is_file() and
            str(swaymsg).startswith("/nix/store/") and swaymsg.is_file() and
            str(grim).startswith("/nix/store/") and grim.is_file() and
            os.getuid() != 0):
        raise RuntimeError("BLOCKED_OUTSIDE_DISPOSABLE_QT_WAYLAND_CI")

    sockets = list(runtime.glob("sway-ipc.*.sock"))
    if len(sockets) != 1 or not sockets[0].is_socket():
        raise RuntimeError("BLOCKED_MISSING_UNIQUE_PRIVATE_SWAY_IPC")
    ipc = str(sockets[0])
    procs: list[subprocess.Popen] = []

    def tree():
        return json.loads(call(str(swaymsg), "-s", ipc, "-r", "-t", "get_tree"))

    def wait_view(title, pid, *, absent=False):
        for _ in range(100):
            found = [n for n in nodes(tree())
                     if n.get("name") == title]
            if absent and not found:
                return None
            if not absent and len(found) == 1 and found[0].get("pid") == pid:
                con = found[0]
                if con.get("shell") != "xdg_shell" or con.get("window") is not None:
                    raise RuntimeError("NOT_A_REAL_NATIVE_QT_XDG")
                return con
            time.sleep(0.1)
        raise RuntimeError(f"REAL_XDG_NOT_{'REMOVED' if absent else 'MAPPED'} {title}")

    def launch(which):
        log = runtime / f"m30f-{which}.log"
        handle = log.open("w", encoding="utf-8")
        try:
            p = subprocess.Popen([str(qt), which], stdout=handle, stderr=subprocess.STDOUT,
                                 env={**env, "QT_QPA_PLATFORM": "wayland",
                                      "QT_WAYLAND_DISABLE_WINDOWDECORATION": "1"},
                                 start_new_session=False)
        finally:
            handle.close()
        procs.append(p)
        for _ in range(100):
            if p.poll() is not None:
                raise RuntimeError("QT_EXITED " + log.read_text()[-2400:])
            if log.exists() and (
                    f"SD_CI_COMPANY_QT_QPA_READY case={which} "
                    f"pid={p.pid} wayland_display=1") in log.read_text():
                break
            time.sleep(0.1)
        else:
            raise RuntimeError("QT_QPA_NOT_READY " + log.read_text()[-2400:])
        title = FRAME_TITLE if which == "frame" else MAP_TITLE
        view = wait_view(title, p.pid)
        fd = os.pidfd_open(p.pid, 0)
        try:
            info = Path(f"/proc/self/fdinfo/{fd}").read_text()
            if f"Pid:\t{p.pid}" not in info:
                raise RuntimeError("KERNEL_PIDFD_MISMATCH")
        finally:
            os.close(fd)
        return p, view

    def cmd(cid, action):
        answer = json.loads(call(str(swaymsg), "-s", ipc, "-r",
                                 f"[con_id={cid}] {action}"))
        if not isinstance(answer, list) or not answer or not all(
                x.get("success") is True for x in answer):
            raise RuntimeError(f"SWAY_COMMAND_REFUSED {action} {answer}")

    def placement(title, pid, cid, rect):
        x, y, w, h = rect
        cmd(cid, "floating enable")
        cmd(cid, "border none")
        cmd(cid, f"resize set width {w} px height {h} px")
        cmd(cid, f"move position {x} {y}")
        for _ in range(100):
            v = wait_view(title, pid)
            actual = v.get("rect", {})
            if all(abs(actual.get(k, -10000) - value) <= 4 for k, value in
                   zip(("x", "y", "width", "height"), rect)):
                return v
            time.sleep(0.1)
        raise RuntimeError(f"LAYOUT_NOT_IN_COMPANY_VIEWPORT expected={rect} got={actual}")

    def screenshot(stage):
        shot = temp / f"sd-company-m30f-{stage}.ppm"
        call(str(grim), "-t", "ppm", str(shot), timeout=25)
        check_pixels(ppm(shot), stage=stage)

    try:
        frame, frameview = launch("frame")
        placement(FRAME_TITLE, frame.pid, frameview["id"], FRAME_RECT)
        cmd(frameview["id"], "focus")
        time.sleep(0.5)
        screenshot("frame")
        native, mapview = launch("map")
        cid = mapview["id"]
        placement(MAP_TITLE, native.pid, cid, MAP_RECT_1)
        cmd(cid, "focus")
        time.sleep(0.5)
        current = wait_view(MAP_TITLE, native.pid)
        if not current.get("focused"):
            raise RuntimeError("MAP_XDG_FOCUS_COMMAND_DID_NOT_SELECT")
        screenshot("initial")
        placement(MAP_TITLE, native.pid, cid, MAP_RECT_2)
        cmd(cid, "focus")
        time.sleep(0.5)
        current = wait_view(MAP_TITLE, native.pid)
        if current.get("id") != cid:
            raise RuntimeError("MAP_XDG_CONTAINER_CHANGED_ON_RESIZE")
        screenshot("expanded")
        native.terminate()
        native.wait(timeout=8)
        wait_view(MAP_TITLE, native.pid, absent=True)
        time.sleep(0.5)
        screenshot("closed")
        if wait_view(FRAME_TITLE, frame.pid)["id"] != frameview["id"]:
            raise RuntimeError("SIMULATED_FRAME_LOST_ON_MAP_CLOSE")
        print("SD_COMPANY_M30F_NATIVE_OVERLAY real_qt_wayland=2 "
              "frame=stable viewport_move_and_resize=2 "
              "focus_select=1 close_restores_frame=1 "
              "pixels=ACTUAL_OUTPUT performance=UNMEASURED "
              "maplibre=NOT_EXECUTED actual_company_qml=NOT_EXECUTED "
              "compositor_clipping=NOT_PROVEN "
              "pointer_input=NOT_PROVEN scene_authority=NONE PASS",
              flush=True)
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
    except Exception as exc:
        print("SD_COMPANY_M30F_BLOCKED:", exc, file=sys.stderr, flush=True)
        raise SystemExit(1)
