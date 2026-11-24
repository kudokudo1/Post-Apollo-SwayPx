#!/usr/bin/env python3
"""Actual XDG + output token turnover on a disposable GitHub SwayPX.

Only the isolated build includes the log-only owner callback overlay. The
repository's ordinary production implementation is byte-for-byte unchanged.
This does NOT test real mouse provenance, all four epoch domains or an
authorized scroll guard.
"""
from __future__ import annotations
import json
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import tempfile
import time

APP = "pa-fc1-xdg-lifetime"
TARGET_BIND = re.compile(r"PA_FC1_NATIVE TARGET_BIND node=(\d+) epoch=(\d+)")
TARGET_REVOKE = re.compile(
    r"PA_FC1_NATIVE TARGET_REVOKE node=(\d+) before=(\d+) after=(\d+)")
OUTPUT_ENABLE = re.compile(r"PA_FC1_NATIVE OUTPUT_ENABLE epoch=(\d+)")
OUTPUT_CONFIG = re.compile(r"PA_FC1_NATIVE OUTPUT_CONFIG old=(\d+) new=(\d+)")
OUTPUT_DISABLE = re.compile(r"PA_FC1_NATIVE OUTPUT_DISABLE before=(\d+) after=(\d+)")

def require_disposable() -> tuple[Path, Path, Path, Path]:
    e = os.environ
    temp = Path(e.get("RUNNER_TEMP", "/missing"))
    keys = ("PA_FC1_SWAY", "PA_FC1_SWAYMSG", "PA_FC1_FOOT", "PA_FC1_MESA")
    values = tuple(Path(e.get(k, "/missing")) for k in keys)
    if not (e.get("CI") == "true" and e.get("GITHUB_ACTIONS") == "true"
            and e.get("PA_AUTOSCROLL_CI_NATIVE_TRACE") == "1"
            and str(temp) == "/home/runner/work/_temp"
            and temp.is_dir() and os.getuid() != 0
            and all(str(v).startswith("/nix/store/") for v in values)
            and all(v.is_file() and os.access(v, os.X_OK)
                    for v in values[:3])
            and (values[3] / "share/glvnd/egl_vendor.d/50_mesa.json").is_file()
            and (values[3] / "lib/dri").is_dir()):
        raise RuntimeError("BLOCKED_PA_FC1_RUNTIME_NOT_DISPOSABLE_PINNED_CI")
    return values

def log_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")

def run_ipc(swaymsg: Path, socket: Path, env: dict[str, str],
            *params: str) -> object:
    proc = subprocess.run(
        [str(swaymsg), "-s", str(socket), "-r", *params],
        capture_output=True, text=True, timeout=12, env=env)
    if proc.returncode:
        raise RuntimeError("PA_FC1_IPC_ERROR " + proc.stderr[-900:])
    try:
        return json.loads(proc.stdout)
    except json.JSONDecodeError as exc:
        raise RuntimeError("PA_FC1_IPC_NOT_JSON " + proc.stdout[-500:]) from exc

def find_app(node: object) -> list[dict]:
    if not isinstance(node, dict):
        return []
    matched = [node] if node.get("app_id") == APP and \
        node.get("shell") == "xdg_shell" else []
    for key in ("nodes", "floating_nodes"):
        for child in node.get(key, []):
            matched += find_app(child)
    return matched

def wait_for_window(swaymsg: Path, ipc: Path, env: dict,
                    proc: subprocess.Popen, exists: bool) -> int | None:
    last_error = ""
    for _ in range(180):
        try:
            tree = run_ipc(swaymsg, ipc, env, "-t", "get_tree")
            nodes = [n for n in find_app(tree) if n.get("pid") == proc.pid]
            if (len(nodes) == 1) == exists:
                return nodes[0]["id"] if exists else None
        except (RuntimeError, subprocess.TimeoutExpired) as ex:
            last_error = str(ex)
        if exists and proc.poll() is not None:
            raise RuntimeError("PA_FC1_FOOT_EXITED_BEFORE_MAP")
        time.sleep(0.07)
    raise RuntimeError(f"PA_FC1_XDG_WINDOW_TIMEOUT expected={exists} "
                       f"pid={proc.pid} ipc_error={last_error}")

def start_foot(foot: Path, runtime: Path, env: dict, title: str):
    return subprocess.Popen(
        [str(foot), f"--app-id={APP}", f"--title={title}",
         "--window-size-pixels=620x450", "/usr/bin/sleep", "55"],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, env=env)

def stop(proc: subprocess.Popen | None) -> None:
    if proc is None:
        return
    if proc.poll() is None:
        proc.terminate()
    try:
        proc.wait(timeout=5)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait(timeout=5)

def assert_window_pairs(records: str, count: int) -> None:
    bindings = [(int(n), int(e)) for n, e in TARGET_BIND.findall(records)
                if int(e) > 0]
    revokes = [(int(n), int(b), int(a))
               for n, b, a in TARGET_REVOKE.findall(records)
               if int(b) > 0]
    if len(bindings) < count or len(revokes) < count:
        raise RuntimeError(f"PA_FC1_REAL_TARGET_MISSING bind={bindings} "
                           f"revoke={revokes}")
    found = []
    for n, e in bindings:
        if (n, e, 0) in revokes and (n, e) not in found:
            found.append((n, e))
    if len(found) < count or len({e for _, e in found}) < count:
        raise RuntimeError(f"PA_FC1_NATIVE_TARGET_REUSE_OR_UNPAIRED {found}")
    print("PA_FC1_NATIVE_TARGET_CREATE_REVOKE "
          f"mapped_XDG={count} same_node_receipt={count} "
          "revoked_epoch_zero=1 fresh_epoch_per_remap=1 PASS", flush=True)

def assert_output(records: str, before_config: int) -> None:
    enabled = [int(e) for e in OUTPUT_ENABLE.findall(records)
               if int(e) > 0]
    configs = [(int(o), int(n)) for o, n in OUTPUT_CONFIG.findall(records)
               if int(o) > 0 and int(n) > 0 and int(o) != int(n)]
    disables = [(int(b), int(a)) for b, a in OUTPUT_DISABLE.findall(records)
                if int(b) > 0 and int(a) == 0]
    if not enabled or len(configs) <= before_config or not disables:
        raise RuntimeError("PA_FC1_REAL_OUTPUT_MISSING "
                           f"enable={enabled} config={configs} "
                           f"disable={disables}")
    if not any(o in enabled or any(n == o for _, n in configs)
               for o, _ in configs):
        raise RuntimeError("PA_FC1_OUTPUT_CONFIG_NOT_SAME_OWNER")
    if not any(b == n for _, n in configs for b, _ in disables):
        raise RuntimeError("PA_FC1_OUTPUT_DISABLE_NOT_LATEST_TOKEN")
    print("PA_FC1_NATIVE_OUTPUT_ENABLE_CONFIG_DISABLE "
          "initial_valid=1 reconfiguration_rotated=1 "
          "disable_revoked_to_zero=1 PASS", flush=True)

def main() -> None:
    sway, swaymsg, foot, mesa = require_disposable()
    runtime = Path(tempfile.mkdtemp(
        prefix="pa-fc1-real-native-", dir=os.environ["RUNNER_TEMP"]))
    runtime.chmod(0o700)
    conf = runtime / "sway.conf"
    conf.write_text(
        "font pango:monospace 10\n"
        "output * bg #000000 solid_color\n"
        "output * resolution 1920x1080\n"
        "focus_follows_mouse no\n"
        f'for_window [app_id="{APP}"] floating enable, border none\n',
        encoding="utf-8")
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
        "LD_LIBRARY_PATH": str(mesa / "lib") + (
            ":" + env["LD_LIBRARY_PATH"] if env.get("LD_LIBRARY_PATH") else ""),
    })
    for k in ("DISPLAY", "WAYLAND_DISPLAY", "WAYLAND_SOCKET", "SWAYSOCK",
              "WLR_RENDERER", "WLR_RENDER_DRM_DEVICE"):
        env.pop(k, None)
    actor = None
    compositor = None
    log = runtime / "sway.log"
    log_file = log.open("wb")
    try:
        compositor = subprocess.Popen(
            [str(sway), "-d", "-c", str(conf)], env=env,
            stdout=log_file, stderr=subprocess.STDOUT)
        ipc = display = None
        for _ in range(220):
            if compositor.poll() is not None:
                raise RuntimeError("PA_FC1_COMPOSITOR_DIED_DURING_STARTUP")
            outputs = [x for x in runtime.glob("wayland-*") if x.is_socket()]
            ipcs = [x for x in runtime.glob("sway-ipc.*.sock") if x.is_socket()]
            if len(outputs) == 1 and len(ipcs) == 1:
                display, ipc = outputs[0], ipcs[0]
                break
            time.sleep(0.07)
        if not ipc or not display:
            raise RuntimeError("PA_FC1_PRIVATE_IPC_OR_WAYLAND_NOT_CREATED")
        env["WAYLAND_DISPLAY"] = display.name
        for _ in range(80):
            try:
                tree = run_ipc(swaymsg, ipc, env, "-t", "get_tree")
                if isinstance(tree, dict):
                    break
            except (RuntimeError, subprocess.TimeoutExpired):
                pass
            time.sleep(0.08)
        else:
            raise RuntimeError("PA_FC1_IPC_NOT_READY")
        for i in (1, 2):
            actor = start_foot(foot, runtime, env, f"PA-LIFETIME-{i}")
            nid = wait_for_window(swaymsg, ipc, env, actor, exists=True)
            if type(nid) is not int or nid <= 0:
                raise RuntimeError("PA_FC1_NOT_REAL_MAPPED_XDG")
            if i == 1:
                pre = len([1 for old, new in OUTPUT_CONFIG.findall(
                    log_text(log)) if int(old) > 0 and int(new) > 0
                    and old != new])
                result = run_ipc(
                    swaymsg, ipc, env, "output HEADLESS-1 scale 1")
                if not (isinstance(result, list) and result
                        and result[0].get("success") is True):
                    raise RuntimeError("PA_FC1_REAL_OUTPUT_CONFIG_DENIED")
            stop(actor)
            wait_for_window(swaymsg, ipc, env, actor, exists=False)
            actor = None
        assert_window_pairs(log_text(log), 2)

        result = run_ipc(swaymsg, ipc, env, "output HEADLESS-1 disable")
        if not (isinstance(result, list) and result
                and result[0].get("success") is True):
            raise RuntimeError("PA_FC1_OUTPUT_DISABLE_DENIED")
        assert_output(log_text(log), pre)
        print("PA_FC1_REAL_COMPOSITOR_LIFETIME_EVIDENCE_PASS "
              "physical_pointer=NOT_TESTED seat=NOT_TESTED "
              "guard_positive=DENIED axis_frames=ZERO", flush=True)
    except Exception:
        print("PA_FC1_DISPOSABLE_COMPOSITOR_LOG_TAIL", flush=True)
        print("\n".join(log_text(log).splitlines()[-90:]), flush=True)
        raise
    finally:
        stop(actor)
        stop(compositor)
        log_file.close()
        shutil.rmtree(runtime, ignore_errors=True)

if __name__ == "__main__":
    main()
