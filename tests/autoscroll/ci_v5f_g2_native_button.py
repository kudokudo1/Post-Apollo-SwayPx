#!/usr/bin/env python3
"""V5F-G2: disposable native Wayland virtual middle-click negative proof.

No local desktop access. Instruments a COPY of the already verified C2
compositor, then observes the real SwayPX cursor/seatop button path.
The focused application receiving a button is NOT claimed by this test.
"""
from __future__ import annotations

import os
from pathlib import Path
import re
import select
import shutil
import subprocess
import sys
import tempfile
import time

C2_DEVICE = "sway/input/input-manager.c"
C2_SEAT = "sway/input/seat.c"
CURSOR = "sway/input/cursor.c"
COPY_NAME = "pa-fc2-native-src"

def replace_one(source: str, anchor: str, replacement: str, label: str) -> str:
    if source.count(anchor) != 1 or replacement in source:
        raise RuntimeError("PA_G2_OVERLAY_ANCHOR_DRIFT " + label)
    return source.replace(anchor, replacement, 1)

def cursor_patch(source: str) -> str:
    # C2 already instruments actual device+seat registration and destruction.
    # No test override may change the native hard-OFF runtime or guard.
    source = replace_one(source,
        '\tcursor_handle_activity_from_device(cursor, &event->pointer->base);\n'
        '\t/* Evaluate both physical pairing ceremonies on a press/release.',
        '\tcursor_handle_activity_from_device(cursor, &event->pointer->base);\n'
        '\tif (event->button == BTN_MIDDLE) {\n'
        '\t\tsway_log(SWAY_INFO, "PA_G2_NATIVE_INGRESS state=%d autoscroll_null=%d",\n'
        '\t\t\tevent->state == WL_POINTER_BUTTON_STATE_PRESSED,\n'
        '\t\t\tcursor->autoscroll == NULL);\n'
        '\t}\n'
        '\t/* Evaluate both physical pairing ceremonies on a press/release.',
        "cursor_ingress")
    source = replace_one(source,
        '\tif (weather_paired || company_paired) {\n'
        '\t\treturn;\n'
        '\t}\n'
        '\t/* V5E-B shared cursor seam',
        '\tif (event->button == BTN_MIDDLE) {\n'
        '\t\tsway_log(SWAY_INFO, "PA_G2_NATIVE_PAIR state=%d weather=%d company=%d",\n'
        '\t\t\tevent->state == WL_POINTER_BUTTON_STATE_PRESSED,\n'
        '\t\t\tweather_paired, company_paired);\n'
        '\t}\n'
        '\tif (weather_paired || company_paired) {\n'
        '\t\treturn;\n'
        '\t}\n'
        '\t/* V5E-B shared cursor seam',
        "first_refusal")
    source = replace_one(source,
        '\tseatop_button(cursor->seat, time_msec, device, button, state);\n}',
        '\tif (button == BTN_MIDDLE) {\n'
        '\t\tsway_log(SWAY_INFO, "PA_G2_NATIVE_TO_SEATOP state=%d",\n'
        '\t\t\tstate == WL_POINTER_BUTTON_STATE_PRESSED);\n'
        '\t}\n'
        '\tseatop_button(cursor->seat, time_msec, device, button, state);\n'
        '\tif (button == BTN_MIDDLE) {\n'
        '\t\tsway_log(SWAY_INFO, "PA_G2_NATIVE_FROM_SEATOP state=%d",\n'
        '\t\t\tstate == WL_POINTER_BUTTON_STATE_PRESSED);\n'
        '\t}\n}',
        "normal_seatop_path")
    return source

def guard_ci():
    e = os.environ
    tmp = Path(e.get("RUNNER_TEMP", "/missing"))
    ws = Path(e.get("GITHUB_WORKSPACE", "/missing"))
    if not (e.get("GITHUB_ACTIONS") == "true" and e.get("CI") == "true"
            and e.get("PA_AUTOSCROLL_CI_NATIVE_TRACE") == "2"
            and e.get("PA_G2_NATIVE_BUTTON") == "2"
            and str(tmp) == "/home/runner/work/_temp"
            and os.getuid() != 0 and tmp.is_dir() and ws.is_dir()):
        raise RuntimeError("BLOCKED_PA_G2_NON_DISPOSABLE_RUNNER")
    return tmp, ws

def overlay():
    tmp, ws = guard_ci()
    from ci_v5f_c2_native_overlay import main as c2_overlay_main
    c2_overlay_main()
    dst = tmp / COPY_NAME
    target = dst / CURSOR
    target.write_text(cursor_patch(target.read_text(encoding="utf-8")),
        encoding="utf-8")
    changed = set(subprocess.check_output([
        "git", "-C", str(dst), "diff", "--name-only"], text=True).splitlines())
    if changed != {C2_DEVICE, C2_SEAT, CURSOR}:
        raise RuntimeError("PA_G2_OVERLAY_FOREIGN_CHANGES " + repr(changed))
    if subprocess.check_output(
            ["git", "-C", str(ws), "status", "--porcelain"], text=True):
        raise RuntimeError("PA_G2_CHECKOUT_DIRTY_AFTER_OVERLAY")
    print("PA_G2_DISPOSABLE_CURSOR_INSTRUMENTATION_ONLY_PASS "
          "normal_checkout=UNCHANGED native_runtime=HARD_OFF", flush=True)

def stop(proc):
    if proc is None:
        return
    if proc.poll() is None:
        proc.terminate()
    try:
        proc.wait(timeout=5)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait(timeout=5)

def read_ready(proc, expected, timeout=14):
    ready, _, _ = select.select([proc.stdout], [], [], timeout)
    if not ready:
        raise RuntimeError("PA_G2_PROTOCOL_TIMEOUT " + expected)
    line = proc.stdout.readline().strip()
    if line != expected:
        raise RuntimeError("PA_G2_PROTOCOL_UNEXPECTED " + repr(line)
                           + " expected=" + expected)

def wait_log(log, token, timeout=10):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        text = log.read_text(encoding="utf-8", errors="replace")
        if token in text:
            return text
        time.sleep(0.05)
    raise RuntimeError("PA_G2_MISSING_NATIVE_LOG " + token)

def native():
    tmp, _ = guard_ci()
    sway = Path(os.environ.get("PA_G2_SWAY", "/missing"))
    client = Path(os.environ.get("PA_G2_CLIENT", "/missing"))
    mesa = Path(os.environ.get("PA_G2_MESA", "/missing"))
    if not (str(sway).startswith("/nix/store/") and sway.is_file()
            and os.access(sway, os.X_OK) and client.is_file()
            and os.access(client, os.X_OK)
            and str(client).startswith(str(tmp) + "/")
            and str(mesa).startswith("/nix/store/")
            and (mesa / "share/glvnd/egl_vendor.d/50_mesa.json").is_file()
            and (mesa / "lib/dri").is_dir()):
        raise RuntimeError("BLOCKED_PA_G2_UNPINNED_BINARIES")
    runtime = Path(tempfile.mkdtemp(prefix="pa-g2-native-", dir=tmp))
    runtime.chmod(0o700)
    conf = runtime / "sway.conf"
    conf.write_text("font pango:monospace 10\n"
        "output * bg #000000 solid_color\n"
        "output * resolution 1920x1080\n", encoding="utf-8")
    env = dict(os.environ)
    env.update({
        "XDG_RUNTIME_DIR": str(runtime), "XDG_SESSION_TYPE": "wayland",
        "WLR_BACKENDS": "headless", "WLR_HEADLESS_OUTPUTS": "1",
        "WLR_LIBINPUT_NO_DEVICES": "1",
        "WLR_RENDERER_FORCE_SOFTWARE": "1",
        "WLR_RENDERER_ALLOW_SOFTWARE": "1",
        "LIBGL_ALWAYS_SOFTWARE": "1", "EGL_PLATFORM": "surfaceless",
        "__EGL_VENDOR_LIBRARY_FILENAMES": str(
            mesa / "share/glvnd/egl_vendor.d/50_mesa.json"),
        "LIBGL_DRIVERS_PATH": str(mesa / "lib/dri"),
        "LD_LIBRARY_PATH": str(mesa / "lib") +
            (":" + env["LD_LIBRARY_PATH"] if env.get("LD_LIBRARY_PATH") else ""),
    })
    for key in ("WAYLAND_DISPLAY", "WAYLAND_SOCKET", "SWAYSOCK", "DISPLAY",
                "WLR_RENDERER", "WLR_RENDER_DRM_DEVICE"):
        env.pop(key, None)
    log = runtime / "sway.log"
    compositor = None
    child = None
    f = log.open("wb")
    try:
        compositor = subprocess.Popen([str(sway), "-d", "-c", str(conf)],
            stdout=f, stderr=subprocess.STDOUT, env=env)
        display = None
        for _ in range(220):
            if compositor.poll() is not None:
                raise RuntimeError("PA_G2_NATIVE_COMPOSITOR_EXITED")
            sockets = [x for x in runtime.glob("wayland-*") if x.is_socket()]
            ipc = [x for x in runtime.glob("sway-ipc.*.sock") if x.is_socket()]
            if len(sockets) == 1 and len(ipc) == 1:
                display = sockets[0].name
                break
            time.sleep(0.07)
        if display is None:
            raise RuntimeError("PA_G2_PRIVATE_WAYLAND_MISSING")
        env["WAYLAND_DISPLAY"] = display
        child = subprocess.Popen([str(client)], env=env,
            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, text=True, bufsize=1)
        read_ready(child, "PA_G2_PROTOCOL_READY transient=1 virtual=1")
        text = wait_log(log, "PA_FC2_NATIVE DEVICE_BIND")
        if not re.search(
                r"PA_FC2_NATIVE DEVICE_BIND epoch=[1-9][0-9]* eligible=0 virtual=1",
                text):
            raise RuntimeError("PA_G2_VIRTUAL_POINTER_PHYSICAL_ELIGIBILITY")
        if not re.search(r"PA_FC2_NATIVE SEAT_BIND epoch=[1-9][0-9]*", text):
            raise RuntimeError("PA_G2_NATIVE_TRANSIENT_SEAT_MISSING")
        child.stdin.write("click\n")
        child.stdin.flush()
        read_ready(child, "PA_G2_PROTOCOL_MIDDLE_SENT pressed=1 released=1 axis=0")
        text = wait_log(log, "PA_G2_NATIVE_FROM_SEATOP state=0")
        for state in ("1", "0"):
            for key in (
                f"PA_G2_NATIVE_INGRESS state={state} autoscroll_null=1",
                f"PA_G2_NATIVE_PAIR state={state} weather=0 company=0",
                f"PA_G2_NATIVE_TO_SEATOP state={state}",
                f"PA_G2_NATIVE_FROM_SEATOP state={state}",
            ):
                if text.count(key) != 1:
                    raise RuntimeError("PA_G2_NATIVE_DISPATCH_COUNT_INVALID "
                                       + key + " count=" + str(text.count(key)))
        child.stdin.write("stop\n")
        child.stdin.flush()
        output, errors = child.communicate(timeout=14)
        if child.returncode != 0 or (
                "PA_G2_PROTOCOL_REVOKED device_before_seat=1" not in output):
            raise RuntimeError("PA_G2_CLIENT_DESTROY_FAILED "
                               + str(child.returncode) + " " + errors[-900:])
        wait_log(log, "PA_FC2_NATIVE DEVICE_REVOKE")
        wait_log(log, "PA_FC2_NATIVE SEAT_REVOKE")
        print("PA_G2_REAL_VIRTUAL_MIDDLE_DENIED_NO_EVENT_CONSUMPTION_PASS "
              "native_pressed=1 native_released=1 seatop_calls=2 "
              "runtime_allocated=0 virtual_eligible=0 axis_injected=0",
              flush=True)
    except Exception:
        print("PA_G2_DISPOSABLE_COMPOSITOR_LOG_TAIL", flush=True)
        print("\n".join(log.read_text(
            encoding="utf-8", errors="replace").splitlines()[-100:]),
            flush=True)
        raise
    finally:
        stop(child)
        stop(compositor)
        f.close()
        shutil.rmtree(runtime, ignore_errors=True)

if __name__ == "__main__":
    if len(sys.argv) != 2 or sys.argv[1] not in ("overlay", "native"):
        raise RuntimeError("PA_G2_EXPLICIT_MODE_REQUIRED")
    (overlay if sys.argv[1] == "overlay" else native)()
