#!/usr/bin/env python3
"""V5F-G3B: REAL compositor target/output epoch revocations, CI-only.

The embedded G3A policy was started by intentionally forged TEST-ONLY
device, seat, origin and layout facts. Only target and output lifetimes
are read from real native SwayPX owners. No input, axis, frame or mouse
is invoked. No production build changes.
"""
from __future__ import annotations
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import time

import ci_v5f_c1_native_trace as c1

BASELINE = re.compile(
    r"PA_G3B_NATIVE_SYNTHETIC_BASELINE node=(\d+) "
    r"target_epoch=(\d+) output_epoch=(\d+) "
    r"device_seat=TEST_ONLY origin=FORGED_TEST_ONLY axis_frames=0")
DENIAL = re.compile(
    r"PA_G3B_NATIVE_DENY reason=(\w+) old_target=(\d+) "
    r"now_target=(\d+) old_output=(\d+) now_output=(\d+) "
    r"canceled=1 axis_frames=0")

def ci_guard() -> tuple[Path, Path, Path, Path]:
    if os.environ.get("PA_G3B_NATIVE_REVOCATION") != "1":
        raise RuntimeError("BLOCKED_PA_G3B_NON_DISPOSABLE_RUNNER")
    return c1.require_disposable()

def log_text(log: Path) -> str:
    return log.read_text(encoding="utf-8", errors="replace")

def wait_baseline(log: Path, nid: int, old: set[tuple]) -> tuple[int, int]:
    for _ in range(160):
        vals = [
            (int(n), int(t), int(o))
            for n, t, o in BASELINE.findall(log_text(log))
            if int(n) == nid and (int(n), int(t), int(o)) not in old
        ]
        if len(vals) == 1 and vals[0][1] > 0 and vals[0][2] > 0:
            return vals[0][1], vals[0][2]
        time.sleep(0.06)
    raise RuntimeError("PA_G3B_REAL_NATIVE_BASELINE_MISSING node=" + str(nid))

def wait_denial(log: Path, reason: str, old: set[tuple]) -> tuple[int, int, int, int]:
    for _ in range(160):
        vals = [
            (int(t), int(nt), int(o), int(no))
            for name, t, nt, o, no in DENIAL.findall(log_text(log))
            if name == reason and (name, int(t), int(nt), int(o), int(no)) not in old
        ]
        if len(vals) == 1:
            return vals[0]
        time.sleep(0.06)
    raise RuntimeError("PA_G3B_EXPECTED_REAL_NATIVE_CANCELLATION_MISSING " + reason)

def all_baselines(log: Path) -> set[tuple]:
    return {(int(n), int(t), int(o))
            for n, t, o in BASELINE.findall(log_text(log))}

def all_denials(log: Path) -> set[tuple]:
    return {(n, int(t), int(nt), int(o), int(no))
            for n, t, nt, o, no in DENIAL.findall(log_text(log))}

def command(swaymsg: Path, ipc: Path, env: dict, *parts: str) -> None:
    result = c1.run_ipc(swaymsg, ipc, env, *parts)
    if not (isinstance(result, list) and result
            and result[0].get("success") is True):
        raise RuntimeError("PA_G3B_REAL_IPC_COMMAND_REFUSED "
                           + repr(parts) + " " + repr(result))

def main() -> None:
    sway, swaymsg, foot, mesa = ci_guard()
    runtime = Path(tempfile.mkdtemp(
        prefix="pa-g3b-real-native-", dir=os.environ["RUNNER_TEMP"]))
    runtime.chmod(0o700)
    conf = runtime / "sway.conf"
    conf.write_text(
        "font pango:monospace 10\n"
        "output * bg #000000 solid_color\n"
        "output * resolution 1920x1080\n"
        "focus_follows_mouse no\n"
        f'for_window [app_id="{c1.APP}"] floating enable, border none\n',
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
        "LD_LIBRARY_PATH": str(mesa / "lib") +
            (":" + env["LD_LIBRARY_PATH"] if env.get("LD_LIBRARY_PATH") else ""),
    })
    for k in ("DISPLAY", "WAYLAND_DISPLAY", "WAYLAND_SOCKET", "SWAYSOCK",
              "WLR_RENDERER", "WLR_RENDER_DRM_DEVICE"):
        env.pop(k, None)
    compositor = None
    actor = None
    log = runtime / "sway.log"
    logfile = log.open("wb")
    try:
        compositor = subprocess.Popen(
            [str(sway), "-d", "-c", str(conf)],
            stdout=logfile, stderr=subprocess.STDOUT, env=env)
        ipc = display = None
        for _ in range(220):
            if compositor.poll() is not None:
                raise RuntimeError("PA_G3B_NATIVE_COMPOSITOR_EXITED")
            w = [x for x in runtime.glob("wayland-*") if x.is_socket()]
            i = [x for x in runtime.glob("sway-ipc.*.sock") if x.is_socket()]
            if len(w) == 1 and len(i) == 1:
                display, ipc = w[0], i[0]
                break
            time.sleep(0.07)
        if display is None or ipc is None:
            raise RuntimeError("PA_G3B_PRIVATE_WAYLAND_AND_IPC_MISSING")
        env["WAYLAND_DISPLAY"] = display.name
        seen_ids = set()
        seen_target_epochs = set()

        # 1: Actual XDG window mapped, then actual output config rotated.
        old_baselines = all_baselines(log)
        actor = c1.start_foot(foot, runtime, env, "PA-G3B-REAL-OUTPUT-CONFIG")
        id1 = c1.wait_for_window(swaymsg, ipc, env, actor, exists=True)
        base_t1, base_o1 = wait_baseline(log, id1, old_baselines)
        seen_ids.add(id1)
        seen_target_epochs.add(base_t1)
        old_denials = all_denials(log)
        command(swaymsg, ipc, env, "output HEADLESS-1 scale 1")
        bt, nt, bo, no = wait_denial(log, "output_reconfigure", old_denials)
        if not (bt == nt == base_t1 and bo == base_o1 and
                no > 0 and no != bo):
            raise RuntimeError("PA_G3B_CONFIG_DID_NOT_ROTATE_REAL_OUTPUT "
                               + repr((bt, nt, bo, no)))
        print("PA_G3B_REAL_NATIVE_OUTPUT_CONFIG_CHANGE_CANCEL_PASS "
              "old_output_nonzero=1 new_output_nonzero=1 rotated=1 "
              "target_unchanged=1 axis_frames=0", flush=True)
        c1.stop(actor)
        c1.wait_for_window(swaymsg, ipc, env, actor, exists=False)
        actor = None

        # 2: Fresh actual XDG window mapped, then actual target teardown.
        old_baselines = all_baselines(log)
        actor = c1.start_foot(foot, runtime, env, "PA-G3B-REAL-TARGET-UNMAP")
        id2 = c1.wait_for_window(swaymsg, ipc, env, actor, exists=True)
        base_t2, base_o2 = wait_baseline(log, id2, old_baselines)
        if base_t2 in seen_target_epochs or id2 in seen_ids:
            raise RuntimeError("PA_G3B_REAL_TARGET_LIFETIME_REUSED")
        seen_ids.add(id2)
        seen_target_epochs.add(base_t2)
        old_denials = all_denials(log)
        c1.stop(actor)
        c1.wait_for_window(swaymsg, ipc, env, actor, exists=False)
        actor = None
        bt, nt, bo, no = wait_denial(log, "target_unmap", old_denials)
        if not (bt == base_t2 and bt > 0 and nt == 0 and
                bo == no == base_o2 and bo > 0):
            raise RuntimeError("PA_G3B_TARGET_NOT_REVOKED_IN_REAL_OWNER "
                               + repr((bt, nt, bo, no)))
        print("PA_G3B_REAL_NATIVE_TARGET_UNMAP_CANCEL_PASS "
              "original_target_nonzero=1 revoked_to_zero=1 "
              "output_unchanged=1 axis_frames=0", flush=True)

        # 3: Third mapped actual XDG window, then actual output disable.
        old_baselines = all_baselines(log)
        actor = c1.start_foot(foot, runtime, env, "PA-G3B-REAL-OUTPUT-DISABLE")
        id3 = c1.wait_for_window(swaymsg, ipc, env, actor, exists=True)
        base_t3, base_o3 = wait_baseline(log, id3, old_baselines)
        if base_t3 in seen_target_epochs or id3 in seen_ids:
            raise RuntimeError("PA_G3B_REAL_THIRD_TARGET_LIFETIME_REUSED")
        old_denials = all_denials(log)
        command(swaymsg, ipc, env, "output HEADLESS-1 disable")
        bt, nt, bo, no = wait_denial(log, "output_disable", old_denials)
        if not (bt == nt == base_t3 and bt > 0 and bo == base_o3
                and bo > 0 and no == 0):
            raise RuntimeError("PA_G3B_OUTPUT_NOT_REVOKED_IN_REAL_OWNER "
                               + repr((bt, nt, bo, no)))
        print("PA_G3B_REAL_NATIVE_OUTPUT_DISABLE_CANCEL_PASS "
              "original_output_nonzero=1 revoked_to_zero=1 "
              "target_unchanged=1 axis_frames=0", flush=True)

        records = log_text(log)
        if "PA_G3B_NATIVE_POLICY_DENIAL_FAILED" in records or (
                "PA_G3B_NATIVE_SYNTHETIC_START_REFUSED" in records or
                "PA_G3B_NATIVE_BASELINE_POLICY_FAILED" in records):
            raise RuntimeError("PA_G3B_NATIVE_POLICY_REPORTED_FAILURE")
        if len(BASELINE.findall(records)) != 3 or len(DENIAL.findall(records)) != 3:
            raise RuntimeError("PA_G3B_NATIVE_UNEXPECTED_SESSION_COUNT "
                               + str((len(BASELINE.findall(records)),
                                      len(DENIAL.findall(records)))))
        print("PA_G3B_REAL_NATIVE_TARGET_OUTPUT_REVOCATION_PASS "
              "three_real_xdg=1 real_output_reconfig=1 real_output_disable=1 "
              "native_callback_policy_denials=3 "
              "physical_device_and_seat=SYNTHETIC_NO_ATTESTATION "
              "production_axis_frames=ZERO", flush=True)
    except Exception:
        print("PA_G3B_DISPOSABLE_COMPOSITOR_LOG_TAIL", flush=True)
        print("\n".join(log_text(log).splitlines()[-110:]), flush=True)
        raise
    finally:
        c1.stop(actor)
        c1.stop(compositor)
        logfile.close()
        shutil.rmtree(runtime, ignore_errors=True)

if __name__ == "__main__":
    main()
