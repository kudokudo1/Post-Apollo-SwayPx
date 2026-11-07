#!/usr/bin/env python3
"""Real Wayland two-window Social geometry witness, disposable GitHub CI only.

This exercise is deliberately NOT a same-frame group implementation. The
current Sway IPC geometry path moves one independent client at a time.
We verify real compositor/client geometry and settled pixels, and witness
the intermediate unpaired position that a frame-atomic owner must remove.
No local workstation or application credentials are ever touched.
"""
from __future__ import annotations

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time

WIDTH, HEIGHT = 1920, 1080
APP_HOST = "sd-ci-social-host"
APP_GUEST = "sd-ci-social-guest"
APP_OTHER = "sd-ci-social-unrelated"
CHASSIS = (100, 60, 1000, 700)
RAIL = 110
INSET = 1
HOST_COLOR = (48, 20, 102)
GUEST_COLOR = (18, 170, 135)
OTHER_COLOR = (196, 98, 21)


def guard() -> tuple[Path, Path, Path, Path, Path]:
    e = os.environ
    temp = Path(e.get("RUNNER_TEMP", "/missing"))
    if (e.get("CI") != "true" or e.get("GITHUB_ACTIONS") != "true"
            or e.get("STAGE_DIRECTOR_CI_SOCIAL_REAL_PAIR") != "1"
            or str(temp) != "/home/runner/work/_temp"
            or os.getuid() == 0 or not temp.is_dir()):
        raise RuntimeError("BLOCKED_SOCIAL_REAL_PAIR_NOT_CI")
    binary_keys = ("SD_SOCIAL_CI_SWAY", "SD_SOCIAL_CI_SWAYMSG",
                   "SD_SOCIAL_CI_FOOT", "SD_SOCIAL_CI_GRIM",
                   "SD_SOCIAL_CI_MESA")
    sources = [Path(e.get(key, "/missing")) for key in binary_keys]
    if not all(str(x).startswith("/nix/store/") for x in sources):
        raise RuntimeError("BLOCKED_SOCIAL_REAL_PAIR_UNPINNED_TOOL")
    if not all(x.is_file() and os.access(x, os.X_OK) for x in sources[:4]):
        raise RuntimeError("BLOCKED_SOCIAL_REAL_PAIR_MISSING_BINARY")
    mesa = sources[-1]
    for subpath in ("share/glvnd/egl_vendor.d/50_mesa.json",
                    "lib/libEGL_mesa.so.0"):
        if not (mesa / subpath).is_file():
            raise RuntimeError("BLOCKED_SOCIAL_REAL_PAIR_MESA")
    if not (mesa / "lib/dri").is_dir():
        raise RuntimeError("BLOCKED_SOCIAL_REAL_PAIR_MESA_DRIVERS")
    return (*sources[:4], mesa)


def run(*args, env=None, timeout=12):
    p = subprocess.run(args, capture_output=True, text=True,
                       timeout=timeout, env=env, check=False)
    if p.returncode:
        raise RuntimeError("CI_COMMAND_FAILED " + " ".join(map(str, args[:3])) +
                           " code=" + str(p.returncode) + " " +
                           p.stderr[-1200:] + " " + p.stdout[-800:])
    return p.stdout.strip()


def walk(node):
    if isinstance(node, dict):
        yield node
        for key in ("nodes", "floating_nodes"):
            for child in node.get(key, []):
                yield from walk(child)


def read_tree(swaymsg, ipc, env):
    data = json.loads(run(str(swaymsg), "-s", str(ipc), "-r",
                          "-t", "get_tree", env=env))
    if not isinstance(data, dict):
        raise RuntimeError("CI_INVALID_SWAY_TREE")
    return data


def select(tree, app_id, pid=None):
    candidates = [n for n in walk(tree)
                  if n.get("app_id") == app_id
                  and type(n.get("id")) is int and n["id"] > 0
                  and n.get("shell") == "xdg_shell"]
    if pid is not None:
        candidates = [n for n in candidates if n.get("pid") == pid]
    if len(candidates) != 1:
        return None
    return candidates[0]


def wait_for(swaymsg, ipc, env, app_id, pid, expected=None, present=True):
    last_node = None
    last_got = None
    for _ in range(100):
        tree = read_tree(swaymsg, ipc, env)
        node = select(tree, app_id, pid)
        if not present and node is None:
            return None
        if present and node is not None:
            rect = node.get("rect") or {}
            got = tuple(rect.get(k) for k in ("x", "y", "width", "height"))
            last_node = node
            last_got = got
            if expected is None or got == expected:
                return node
        time.sleep(0.07)
    raise RuntimeError(f"CI_SOCIAL_ACTOR_NOT_SETTLED app={app_id} "
                       f"expected={expected} actual={last_got} "
                       f"node={json.dumps(last_node, sort_keys=True)[:1200]} "
                       f"present={present}")


def sway_command(swaymsg, ipc, env, command):
    result = json.loads(run(str(swaymsg), "-s", str(ipc), "-r",
                            command, env=env))
    if not isinstance(result, list) or not result or not all(
            isinstance(x, dict) and x.get("success") is True
            for x in result):
        raise RuntimeError("CI_SOCIAL_SWAY_COMMAND_DENIED " + str(result))


def locate(swaymsg, ipc, env, cid, rect):
    x, y, width, height = rect
    if any(type(v) is not int for v in rect) or not (
            width > 0 and height > 0):
        raise RuntimeError("CI_SOCIAL_INVALID_TARGET")
    criteria = f'[con_id="{cid}"]'
    sway_command(swaymsg, ipc, env,
        f"{criteria} floating enable; {criteria} border none; "
        f"{criteria} resize set width {width} px height {height} px; "
        f"{criteria} move absolute position {x} px {y} px")


def guest_from_chassis(rect):
    x, y, w, h = rect
    return (x + RAIL + INSET, y + INSET,
            w - RAIL - 2 * INSET, h - 2 * INSET)


def chassis_from_guest(rect):
    x, y, w, h = rect
    return (x - RAIL - INSET, y - INSET,
            w + RAIL + 2 * INSET, h + 2 * INSET)


def make_foot_config(path, color):
    path.write_text(
        "[main]\nfont=monospace:size=12\n"
        "[colors]\n"
        f"background={color[0]:02x}{color[1]:02x}{color[2]:02x}\n"
        "foreground=ffffff\n", encoding="utf-8")
    path.chmod(0o600)


def foot_process(foot, app_id, title, color, runtime, env):
    config = runtime / f"{app_id}.ini"
    make_foot_config(config, color)
    return subprocess.Popen(
        [str(foot), f"--config={config}", f"--app-id={app_id}",
         f"--title={title}", "--window-size-pixels=620x450",
         "/usr/bin/sleep", "55"],
        stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
        env=env)


def ppm_pixel(blob, x, y):
    if not blob.startswith(b"P6\n"):
        raise RuntimeError("CI_SOCIAL_SCREENSHOT_NOT_PPM")
    # Grim's PPM output has a plain binary P6 header.
    cursor = 3
    tokens = []
    while len(tokens) < 3:
        while blob[cursor] in b" \t\r\n":
            cursor += 1
        if blob[cursor] == ord("#"):
            while blob[cursor] != 10:
                cursor += 1
            continue
        start = cursor
        while blob[cursor] not in b" \t\r\n":
            cursor += 1
        tokens.append(int(blob[start:cursor]))
    cursor += 1
    w, h, maximum = tokens
    if (w, h, maximum) != (WIDTH, HEIGHT, 255) or not (
            0 <= x < w and 0 <= y < h):
        raise RuntimeError("CI_SOCIAL_SCREENSHOT_GEOMETRY_INVALID")
    off = cursor + (y * w + x) * 3
    if off + 3 > len(blob):
        raise RuntimeError("CI_SOCIAL_SCREENSHOT_TRUNCATED")
    return tuple(blob[off:off + 3])


def capture(grim, path, env, samples):
    run(str(grim), "-t", "ppm", str(path), env=env)
    if path.is_symlink() or not path.is_file():
        raise RuntimeError("CI_SOCIAL_SCREENSHOT_MISSING")
    pixels = path.read_bytes()
    for name, (x, y), expected in samples:
        value = ppm_pixel(pixels, x, y)
        if max(abs(a - b) for a, b in zip(value, expected)) > 25:
            raise RuntimeError("CI_SOCIAL_SCREENSHOT_PIXEL_MISMATCH "
                               f"sample={name} at={x},{y} "
                               f"observed={value} expected={expected}")
    print("SD_SOCIAL_CI_REAL_PIXELS " +
          " ".join(name + "=PASS" for name, _, _ in samples), flush=True)


def quiet_stop(proc):
    if proc is None:
        return
    if proc.poll() is None:
        proc.terminate()
    try:
        proc.communicate(timeout=4)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.communicate(timeout=4)


def main():
    sway, swaymsg, foot, grim, mesa = guard()
    root = Path(tempfile.mkdtemp(prefix="social-real-pair-",
                                dir=os.environ["RUNNER_TEMP"]))
    root.chmod(0o700)
    env = dict(os.environ)
    env.update({
        "XDG_RUNTIME_DIR": str(root), "XDG_SESSION_TYPE": "wayland",
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
    for key in ("WAYLAND_DISPLAY", "WAYLAND_SOCKET", "DISPLAY", "SWAYSOCK",
                "WLR_RENDERER", "WLR_RENDER_DRM_DEVICE"):
        env.pop(key, None)
    config = root / "sway.conf"
    config.write_text(
        "font pango:monospace 10\n"
        "output * bg #000000 solid_color\n"
        "output * resolution 1920x1080\n"
        "focus_follows_mouse no\n"
        "for_window [app_id=\"sd-ci-social-host\"] floating enable, border none\n"
        "for_window [app_id=\"sd-ci-social-guest\"] floating enable, border none\n"
        "for_window [app_id=\"sd-ci-social-unrelated\"] floating enable, border none\n",
        encoding="utf-8")
    clients = []
    compositor = None
    log = (root / "sway.log").open("wb")
    try:
        compositor = subprocess.Popen(
            [str(sway), "-d", "-c", str(config)], stdout=log,
            stderr=subprocess.STDOUT, env=env)
        ipc = None
        display = None
        for _ in range(150):
            if compositor.poll() is not None:
                raise RuntimeError("CI_SOCIAL_SWAY_EXITED_BEFORE_READY")
            ds = [s for s in root.glob("wayland-*") if s.is_socket()]
            ips = [s for s in root.glob("sway-ipc.*.sock") if s.is_socket()]
            if len(ds) == 1 and len(ips) == 1:
                display, ipc = ds[0], ips[0]
                break
            time.sleep(0.1)
        if ipc is None or display is None:
            raise RuntimeError("CI_SOCIAL_SWAY_SOCKET_NOT_READY")
        env["WAYLAND_DISPLAY"] = display.name

        host = foot_process(foot, APP_HOST, "SOCIAL-CI-HOST", HOST_COLOR, root, env)
        guest = foot_process(foot, APP_GUEST, "SOCIAL-CI-GUEST", GUEST_COLOR, root, env)
        other = foot_process(foot, APP_OTHER, "SOCIAL-CI-UNRELATED",
                             OTHER_COLOR, root, env)
        clients += [host, guest, other]
        ids = {}
        for app, proc in ((APP_HOST, host), (APP_GUEST, guest), (APP_OTHER, other)):
            node = wait_for(swaymsg, ipc, env, app, proc.pid)
            if proc.poll() is not None:
                raise RuntimeError("CI_SOCIAL_FOOT_EARLY_EXIT " + app)
            ids[app] = node["id"]
        if len(set(ids.values())) != 3:
            raise RuntimeError("CI_SOCIAL_WINDOW_ID_COLLISION")
        other_rect = (1490, 570, 320, 240)
        locate(swaymsg, ipc, env, ids[APP_OTHER], other_rect)
        wait_for(swaymsg, ipc, env, APP_OTHER, other.pid, other_rect)

        initial_guest = guest_from_chassis(CHASSIS)
        locate(swaymsg, ipc, env, ids[APP_HOST], CHASSIS)
        locate(swaymsg, ipc, env, ids[APP_GUEST], initial_guest)
        wait_for(swaymsg, ipc, env, APP_HOST, host.pid, CHASSIS)
        wait_for(swaymsg, ipc, env, APP_GUEST, guest.pid, initial_guest)
        capture(grim, root / "initial.ppm", env, [
            ("rail", (CHASSIS[0] + 25, CHASSIS[1] + 160), HOST_COLOR),
            ("guest", (initial_guest[0] + 180, initial_guest[1] + 160),
             GUEST_COLOR),
            ("unrelated", (other_rect[0] + 130, other_rect[1] + 110),
             OTHER_COLOR),
            ("background", (20, 20), (0, 0, 0)),
        ])
        print("SD_SOCIAL_CI_REAL_INITIAL mapped=3 isolated=1 "
              "real_xdg=1 compositor=SWAYPX result=PASS", flush=True)

        # Witness the current architecture's structural split: a compositor
        # command to one window changes it before the other is commanded.
        # This proves two independent Sway windows, not measured frame delay.
        moved_host = (245, 110, 1060, 720)
        locate(swaymsg, ipc, env, ids[APP_HOST], moved_host)
        wait_for(swaymsg, ipc, env, APP_HOST, host.pid, moved_host)
        wait_for(swaymsg, ipc, env, APP_GUEST, guest.pid, initial_guest)
        print("SD_SOCIAL_CI_LEGACY_SPLIT possible=1 "
              "actor_moved_before_follower=1 measured_lag_ms=UNAVAILABLE",
              flush=True)
        moved_guest = guest_from_chassis(moved_host)
        locate(swaymsg, ipc, env, ids[APP_GUEST], moved_guest)
        wait_for(swaymsg, ipc, env, APP_GUEST, guest.pid, moved_guest)
        assert chassis_from_guest(moved_guest) == moved_host
        print("SD_SOCIAL_CI_HOST_LEADER move_resize=PASS "
              "settled_pair=1 frame_atomic=UNVERIFIED", flush=True)

        # Guest-led changes must preserve outer shell relation.
        guest_target = (690, 230, 760, 520)
        new_host = chassis_from_guest(guest_target)
        locate(swaymsg, ipc, env, ids[APP_GUEST], guest_target)
        wait_for(swaymsg, ipc, env, APP_GUEST, guest.pid, guest_target)
        locate(swaymsg, ipc, env, ids[APP_HOST], new_host)
        wait_for(swaymsg, ipc, env, APP_HOST, host.pid, new_host)
        wait_for(swaymsg, ipc, env, APP_OTHER, other.pid, other_rect)
        capture(grim, root / "guest-led.ppm", env, [
            ("rail", (new_host[0] + 28, new_host[1] + 140), HOST_COLOR),
            ("guest", (guest_target[0] + 160, guest_target[1] + 140),
             GUEST_COLOR),
            ("unrelated", (other_rect[0] + 130, other_rect[1] + 110),
             OTHER_COLOR),
            ("background", (20, 20), (0, 0, 0)),
        ])
        print("SD_SOCIAL_CI_GUEST_LEADER move_resize=PASS "
              "settled_pixels=1 unrelated_untouched=1", flush=True)

        # Guest exits and returns with a different Sway identity. Its old
        # container is never targeted and canonical host size stays put.
        old_id = ids[APP_GUEST]
        quiet_stop(guest)
        wait_for(swaymsg, ipc, env, APP_GUEST, guest.pid, present=False)
        wait_for(swaymsg, ipc, env, APP_HOST, host.pid, new_host)
        guest2 = foot_process(foot, APP_GUEST, "SOCIAL-CI-GUEST-RETURN",
                              GUEST_COLOR, root, env)
        clients.append(guest2)
        node = wait_for(swaymsg, ipc, env, APP_GUEST, guest2.pid)
        if node["id"] == old_id:
            raise RuntimeError("CI_SOCIAL_STALE_GUEST_ID_REUSED")
        locate(swaymsg, ipc, env, node["id"], guest_target)
        wait_for(swaymsg, ipc, env, APP_GUEST, guest2.pid, guest_target)
        wait_for(swaymsg, ipc, env, APP_HOST, host.pid, new_host)
        print("SD_SOCIAL_CI_GUEST_REPLACEMENT new_container=1 "
              "host_geometry_preserved=1 result=PASS", flush=True)

    except Exception:
        log.flush()
        try:
            print((root / "sway.log").read_text(
                encoding="utf-8", errors="replace")[-7000:], file=sys.stderr)
        except OSError:
            pass
        raise
    finally:
        for proc in reversed(clients):
            quiet_stop(proc)
        if compositor is not None:
            quiet_stop(compositor)
        log.close()
        shutil.rmtree(root, ignore_errors=True)
    print("SD_SOCIAL_CI_REAL_PAIR final=PASS "
          "same_frame_rendering=NOT_IMPLEMENTED live_desktop=UNCHANGED",
          flush=True)


if __name__ == "__main__":
    main()
