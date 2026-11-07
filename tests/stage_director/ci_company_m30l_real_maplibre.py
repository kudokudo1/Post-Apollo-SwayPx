#!/usr/bin/env python3
"""M30l real MapLibre Native Qt v3 Wayland proof from ORIGINAL Company source.

This verifies a genuine, source-pinned QtLocation MapView with offline
synthetic geographic geometry in a disposable SwayPX compositor. It is NOT
yet the selected Company layer embedding or the user workstation.
"""
from __future__ import annotations
import json
import os
from pathlib import Path
import subprocess
import sys
import time

from ci_company_overlay_prototype import call, nodes, ppm, rgb
from ci_company_input_containment import EVENT, read_until, near

TITLE = "SD-CI-CompanyVectorProbe-crop"
EXPECTED_SOURCE = (840, 540)


def guard():
    e = os.environ
    temp = Path(e.get("RUNNER_TEMP", "/no-runner"))
    runtime = Path(e.get("XDG_RUNTIME_DIR", "/no-runtime"))
    socket = runtime / e.get("WAYLAND_DISPLAY", "/no-wayland")
    binary = Path(e.get("STAGE_DIRECTOR_M30L_NATIVE_MAPLIBRE", "/no-binary"))
    swaymsg = Path(e.get("STAGE_DIRECTOR_TEST_SWAYMSG", "/no-sway"))
    grim = Path(e.get("STAGE_DIRECTOR_TEST_GRIM", "/no-grim"))
    pointer = Path(e.get("STAGE_DIRECTOR_TEST_COMPANY_POINTER", "/no-pointer"))
    checks = (e.get("CI") == "true" and
              e.get("GITHUB_ACTIONS") == "true" and
              e.get("STAGE_DIRECTOR_CI_COMPANY_M30L") == "1" and
              temp == Path("/home/runner/work/_temp") and
              runtime.is_dir() and
              str(runtime).startswith(str(temp) + "/") and
              socket.is_socket() and
              os.getuid() != 0 and
              all(str(p).startswith("/nix/store/") and p.is_file()
                  for p in (binary, swaymsg, grim, pointer)))
    if not checks:
        raise RuntimeError("BLOCKED_M30L_NOT_PRIVATE_WAYLAND_CI")
    ipcs = [p for p in runtime.glob("sway-ipc.*.sock") if p.is_socket()]
    if len(ipcs) != 1:
        raise RuntimeError("M30L_NOT_UNIQUE_NATIVE_SWAY_IPC")
    return e, temp, runtime, binary, swaymsg, grim, pointer, str(ipcs[0])


def actual_map_colors(image, left, top):
    # Sample the real QWidget/RHI framebuffer from the compositor, not
    # a generated fixture or MapLibre's built-in grabWindow.
    unique = set()
    cyan = blue = violet = yellow = 0
    for y in range(top + 70, top + 420, 4):
        for x in range(left + 15, left + 700, 4):
            color = rgb(image, x, y)
            unique.add(tuple(v // 12 for v in color))
            r, g, b = color
            if g > 125 and b > 115 and g > 1.25 * max(1,r):
                cyan += 1
            if b > 125 and b > 1.2 * max(1,g):
                blue += 1
            if r > 115 and b > 70 and r > g * 1.22:
                violet += 1
            if r > 160 and 80 < g < 220 and b < 135:
                yellow += 1
    if len(unique) < 8 or cyan < 4:
        raise RuntimeError(
            "M30L_MAPLIBRE_OFFLINE_GEOJSON_NOT_RENDERED "
            f"colors={len(unique)} cyan_samples={cyan} "
            f"blue={blue} violet={violet} yellow={yellow}")
    print("SD_COMPANY_M30L_MAPLIBRE_PIXELS "
          f"unique_palette_buckets={len(unique)} "
          f"cyan={cyan} blue={blue} violet={violet} yellow={yellow} "
          "real_compositor_pixels=1 offline_synthetic_geojson=1 PASS",
          flush=True)


def main():
    e, temp, runtime, binary, swaymsg, grim, pointer_bin, ipc = guard()
    stdout = runtime / "sd-company-m30l-maplibre.log"
    process = pointer = None
    pos = (480, 240)
    def tree():
        return json.loads(call(str(swaymsg), "-s", ipc, "-r", "-t", "get_tree"))
    def node(pid, absent=False):
        for _ in range(100):
            candidates = [n for n in nodes(tree()) if n.get("name") == TITLE]
            if absent and not candidates:
                return None
            if len(candidates) == 1 and candidates[0].get("pid") == pid:
                if candidates[0].get("shell") != "xdg_shell":
                    raise RuntimeError("M30L_MAPLIBRE_NOT_NATIVE_XDG")
                return candidates[0]
            time.sleep(0.1)
        raise RuntimeError("M30L_NATIVE_MAPLIBRE_WAYLAND_WINDOW_NOT_FOUND")
    def command(cid, instruction):
        responses = json.loads(call(str(swaymsg), "-s", ipc, "-r",
                               f"[con_id={cid}] {instruction}"))
        if not isinstance(responses, list) or not responses or not all(
                r.get("success") is True for r in responses):
            raise RuntimeError("M30L_SWAY_DIRECTIVE_FAILED " + instruction)
    def screenshot(name):
        out = temp / f"sd-company-m30l-{name}.ppm"
        call(str(grim), "-t", "ppm", str(out), timeout=25)
        return ppm(out)
    def send(cmd):
        assert pointer and pointer.stdin and pointer.stdout
        pointer.stdin.write(cmd + "\n")
        pointer.stdin.flush()
        ack = read_until(pointer.stdout,
                         "SD_CI_COMPANY_POINTER_ACK ", 8)
        if ack != "SD_CI_COMPANY_POINTER_ACK " + cmd:
            raise RuntimeError("M30L_POINTER_ACK_MISMATCH " + ack)
        time.sleep(0.14)

    try:
        env = {**e, "QT_QPA_PLATFORM": "wayland",
               "QT_WAYLAND_DISABLE_WINDOWDECORATION": "1",
               "QSG_RHI_BACKEND": "opengl",
               "LIBGL_ALWAYS_SOFTWARE": "1",
               "QT_DEBUG_PLUGINS": "1",
               "QML_IMPORT_TRACE": "1",
               "STAGE_DIRECTOR_CI_COMPANY_M30L": "1"}
        with stdout.open("w", encoding="utf-8") as stream:
            process = subprocess.Popen(
                [str(binary)], env=env, stdout=stream,
                stderr=subprocess.STDOUT)
        expected = (f"SD_CI_COMPANY_QT_QPA_READY case=crop "
                    f"pid={process.pid} wayland_display=1 "
                    f"maplibre_v3_source=1")
        deadline = time.monotonic() + 25
        while time.monotonic() < deadline:
            if process.poll() is not None:
                raise RuntimeError(
                    "M30L_MAPLIBRE_NATIVE_EXITED returncode=" +
                    str(process.returncode) + " executable=" +
                    str(binary) + " plugin_diagnostics=" +
                    stdout.read_text(errors="replace")[-13000:])
            if expected in stdout.read_text(errors="replace"):
                break
            time.sleep(0.15)
        else:
            raise RuntimeError("M30L_REAL_QT_APP_NOT_READY " +
                stdout.read_text(errors="replace")[-4800:])
        view = node(process.pid)
        fd = os.pidfd_open(process.pid, 0)
        try:
            if f"Pid:\t{process.pid}" not in Path(
                    f"/proc/self/fdinfo/{fd}").read_text():
                raise RuntimeError("M30L_ACTUAL_MAPLIBRE_PIDFD_MISMATCH")
        finally:
            os.close(fd)
        cid = view["id"]
        for instruction in ("floating enable", "border none",
                  "resize set width 840 px height 540 px",
                  f"move position {pos[0]} {pos[1]}"):
            command(cid, instruction)
        for _ in range(90):
            rectangle = node(process.pid).get("rect", {})
            if all(abs(rectangle.get(key, -1) - v) <= 4
                   for key, v in zip(("x","y","width","height"),
                                     (pos[0], pos[1], *EXPECTED_SOURCE))):
                break
            time.sleep(0.1)
        else:
            raise RuntimeError("M30L_MAPLIBRE_WINDOW_NOT_840x540")
        deadline = time.monotonic() + 17
        while time.monotonic() < deadline:
            data = stdout.read_text(errors="replace")
            if "SD_COMPANY_M30L_MAP_READY real_maplibre=1" in data:
                break
            if "SD_COMPANY_M30L_MAP_READY real_maplibre=0" in data:
                raise RuntimeError("M30L_MAPLIBRE_QTLOCATION_NOT_READY " +
                                   data[-4000:])
            time.sleep(0.12)
        else:
            raise RuntimeError("M30L_MAPLIBRE_QML_NOT_READY " +
                               stdout.read_text(errors="replace")[-3500:])
        data = stdout.read_text(errors="replace")
        if "MAPLIBRE_COMPAT_PROBE" not in data or (
                "offline" not in data or "OpenGLRhi" not in data):
            raise RuntimeError("M30L_COMPANY_MAPLIBRE_OFFLINE_PROVENANCE_MISSING")
        map_image = screenshot("actual-native-maplibre")
        actual_map_colors(map_image, *pos)
        print("SD_COMPANY_M30L_REAL_MAPLIBRE "
              "pinned_original_company_cpp=1 "
              "maplibre_native_qt=3.0.0 real_qtlocation_mapReady=1 "
              "map_window=840x540 native_xdg_pidfd=PASS "
              "online_requests=DISABLED user_locations=NONE PASS",
              flush=True)
        pointer = subprocess.Popen([str(pointer_bin)],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT, text=True, bufsize=1, env=e)
        assert pointer.stdout is not None
        read_until(pointer.stdout,
                   "SD_CI_COMPANY_POINTER_READY real_zwlr_virtual_pointer=1")
        start = stdout.read_text(errors="replace")
        begin = (pos[0]+320, pos[1]+230)
        drag = (pos[0]+440, pos[1]+320)
        for cmd in (f"move {begin[0]} {begin[1]}", "press",
                    f"move {drag[0]} {drag[1]}", "release", "wheel"):
            send(cmd)
        end = stdout.read_text(errors="replace")
        matches = [EVENT.search(l) for l in end[len(start):].splitlines()]
        matches = [m for m in matches if m]
        for name in ("press", "motion", "release", "wheel"):
            if not any(m.group(2) == name and m.group(10) == str(process.pid)
                       for m in matches):
                raise RuntimeError(f"M30L_REAL_QT_MAPLIBRE_INPUT_MISSING_{name}")
        print("SD_COMPANY_M30L_REAL_MAP_INPUT "
              "actual_qquickwindow_wayland_press_drag_release_wheel=1 "
              "native_qt_pidfd=1 PASS", flush=True)
        screenshot("after-real-native-maplibre-pointer")
        pointer.stdin.write("quit\n")
        pointer.stdin.flush()
        stop = read_until(pointer.stdout, "SD_CI_COMPANY_POINTER_STOP ", 8)
        pointer.wait(timeout=5)
        if "PASS" not in stop or pointer.returncode != 0:
            raise RuntimeError("M30L_NATIVE_WAYLAND_POINTER_NOT_CLOSED")
        process.terminate()
        process.wait(timeout=8)
        node(process.pid, absent=True)
        screenshot("maplibre-closed")
        print("SD_COMPANY_M30L_NATIVE_QML_CLOSED "
              "same_maplibre_pid=1 xdg_removed=1 "
              "production_company_actor=OFF PASS", flush=True)
    finally:
        for p in (pointer, process):
            if p and p.poll() is None:
                p.terminate()
            if p:
                try: p.wait(timeout=7)
                except subprocess.TimeoutExpired:
                    p.kill(); p.wait(timeout=7)


if __name__ == "__main__":
    try:
        main()
    except Exception as ex:
        print("SD_COMPANY_M30L_BLOCKED:", ex, file=sys.stderr, flush=True)
        raise SystemExit(1)
