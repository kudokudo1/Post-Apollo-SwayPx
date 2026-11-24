#!/usr/bin/env python3
"""G3C actual FOUR-owner compositor epochs in ONE native negative session.

A Wayland virtual pointer and transient seat remain physical-ineligible.
The native C probe uses a counterfeit positive start ONLY in copied CI
to test revocation. This harness does not send input or claim attestation.
"""
from __future__ import annotations
import os
from pathlib import Path
import re
import select
import shutil
import subprocess
import tempfile
import time

import ci_v5f_c1_native_trace as c1

BIND = re.compile(
    r"PA_G3C_NATIVE_VIRTUAL_BIND device=(\d+) seat=(\d+) eligible=(\d+) virtual=(\d+)")
BASE = re.compile(
    r"PA_G3C_NATIVE_FOUR_REAL_EPOCHS node=(\d+) device=(\d+) "
    r"seat=(\d+) target=(\d+) output=(\d+) "
    r"virtual_eligible=0 origin=FORGED_TEST_ONLY "
    r"scene_layout=FAKE_TEST_ONLY axis_frames=0")
DENY = re.compile(
    r"PA_G3C_NATIVE_DENY reason=(\w+) "
    r"device=(\d+):(\d+) seat=(\d+):(\d+) "
    r"target=(\d+):(\d+) output=(\d+):(\d+) "
    r"canceled=1 axis_frames=0")
C2_SEAT = re.compile(r"PA_FC2_NATIVE SEAT_BIND epoch=(\d+)")

def require_ci():
    e = os.environ
    sway, swaymsg, foot, mesa = c1.require_disposable()
    temp = Path(e.get("RUNNER_TEMP", "/missing"))
    client = Path(e.get("PA_G3C_CLIENT", "/missing"))
    if not (e.get("PA_G3C_NATIVE_FOUR_OWNER") == "1"
            and str(client).startswith(str(temp) + "/")
            and client.is_file() and os.access(client, os.X_OK)):
        raise RuntimeError("BLOCKED_PA_G3C_NATIVE_CLIENT_NOT_PINNED")
    return sway, swaymsg, foot, mesa, client

def readlog(log):
    return log.read_text(encoding="utf-8", errors="replace")

def stop(proc):
    c1.stop(proc)

def client_command(proc, command, expected):
    if proc.poll() is not None:
        raise RuntimeError("PA_G3C_CLIENT_PREMATURE_EXIT " + command)
    proc.stdin.write(command + "\n")
    proc.stdin.flush()
    readable, _, _ = select.select([proc.stdout], [], [], 15)
    if not readable:
        raise RuntimeError("PA_G3C_CLIENT_COMMAND_TIMEOUT " + command)
    line = proc.stdout.readline().strip()
    if line != expected:
        raise RuntimeError("PA_G3C_CLIENT_COMMAND_MISMATCH "
                           + command + " got=" + repr(line))

def wait_match(log, pattern, prior, predicate, label):
    for _ in range(180):
        matches = [
            tuple(int(x) if x.isdecimal() else x for x in found)
            for found in pattern.findall(readlog(log))
        ]
        fresh = [m for m in matches if m not in prior and predicate(m)]
        if len(fresh) == 1:
            return fresh[0]
        time.sleep(0.065)
    raise RuntimeError("PA_G3C_REAL_CALLBACK_MISSING " + label
                       + " " + repr(pattern.findall(readlog(log))[-12:]))

def snapshots(log, pattern):
    return {
        tuple(int(x) if x.isdecimal() else x for x in found)
        for found in pattern.findall(readlog(log))
    }

def start_client(binary, env, log):
    old = snapshots(log, BIND)
    proc = subprocess.Popen([str(binary)], env=env,
        stdin=subprocess.PIPE, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE, text=True, bufsize=1)
    readable, _, _ = select.select([proc.stdout], [], [], 15)
    if not readable or proc.stdout.readline().strip() != (
            "PA_G3C_PROTOCOL_READY real_transient=1 real_virtual=1"):
        raise RuntimeError("PA_G3C_REAL_WAYLAND_CLIENT_NOT_READY")
    device, seat, eligible, virtual = wait_match(
        log, BIND, old, lambda m: True, "device-seat-bind")
    if not (device > 0 and seat > 0 and device != seat
            and eligible == 0 and virtual == 1
            and seat in [int(x) for x in C2_SEAT.findall(readlog(log))]):
        raise RuntimeError("PA_G3C_REAL_VIRTUAL_DEVICE_ELIGIBILITY_ERROR "
                           + repr((device, seat, eligible, virtual)))
    return proc, device, seat

def start_window(foot, swaymsg, ipc, env, log, name, pair):
    previous = snapshots(log, BASE)
    proc = c1.start_foot(foot, Path(env["XDG_RUNTIME_DIR"]), env, name)
    node = c1.wait_for_window(swaymsg, ipc, env, proc, exists=True)
    actual = wait_match(log, BASE, previous, lambda m: m[0] == node,
                        "four-owner-baseline")
    if not (actual[0] == node
            and actual[1] == pair[0] and actual[2] == pair[1]
            and all(x > 0 for x in actual[1:])
            and len(set(actual[1:])) == 4):
        raise RuntimeError("PA_G3C_FOUR_NATIVE_OWNER_IDS_NOT_ATOMICALLY_PRESENT "
                           + repr(actual) + " pair=" + repr(pair))
    return proc, actual

def denial(log, reason, base, before):
    result = wait_match(log, DENY, before, lambda m: m[0] == reason,
                        "native-negative-" + reason)
    actual = {
        "device": result[1:3], "seat": result[3:5],
        "target": result[5:7], "output": result[7:9],
    }
    for key, (old, new) in actual.items():
        original = base[{"device": 1, "seat": 2,
                         "target": 3, "output": 4}[key]]
        if old != original:
            raise RuntimeError("PA_G3C_ORIGINAL_EPOCH_MISMATCH "
                               + key + " " + repr(result))
        changing = {
            "device_destroy": "device",
            "seat_destroy": "seat",
            "target_unmap": "target",
            "output_disable": "output",
            "output_reconfigure": "output",
        }[reason]
        if key == changing:
            if reason == "output_reconfigure":
                if not (new > 0 and new != old):
                    raise RuntimeError("PA_G3C_OUTPUT_ROTATION_NOT_OBSERVED")
            elif new != 0:
                raise RuntimeError("PA_G3C_OWNER_NOT_REVOKED "
                                   + key + " " + repr(result))
        elif old != new:
            raise RuntimeError("PA_G3C_UNRELATED_OWNER_CHANGED "
                               + key + " " + repr(result))
    print("PA_G3C_REAL_FOUR_OWNER_" + reason.upper() + "_CANCEL_PASS "
          "all_four_original_real=1 changed_domain_denied=1 "
          "other_domains_unchanged=1 axis_frames=0", flush=True)
    return result

def run_ipc(swaymsg, ipc, env, cmd):
    reply = c1.run_ipc(swaymsg, ipc, env, cmd)
    if not (isinstance(reply, list) and reply
            and reply[0].get("success") is True):
        raise RuntimeError("PA_G3C_NATIVE_OUTPUT_COMMAND_FAILED " + repr(reply))

def main():
    sway, swaymsg, foot, mesa, client_binary = require_ci()
    runtime = Path(tempfile.mkdtemp(
        prefix="pa-g3c-four-native-", dir=os.environ["RUNNER_TEMP"]))
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
    log = runtime / "sway.log"
    logfile = log.open("wb")
    compositor = None
    actor = None
    child = None
    clients = []
    try:
        compositor = subprocess.Popen([str(sway), "-d", "-c", str(conf)],
            stdout=logfile, stderr=subprocess.STDOUT, env=env)
        ipc = display = None
        for _ in range(220):
            if compositor.poll() is not None:
                raise RuntimeError("PA_G3C_REAL_COMPOSITOR_STARTUP_FAILED")
            w = [x for x in runtime.glob("wayland-*") if x.is_socket()]
            i = [x for x in runtime.glob("sway-ipc.*.sock") if x.is_socket()]
            if len(w) == 1 and len(i) == 1:
                display, ipc = w[0], i[0]
                break
            time.sleep(0.07)
        if display is None or ipc is None:
            raise RuntimeError("PA_G3C_PRIVATE_WAYLAND_IPC_MISSING")
        env["WAYLAND_DISPLAY"] = display.name

        # One native compositor instance, owner tokens from all FOUR domains.
        child, dev1, seat1 = start_client(client_binary, env, log)
        clients.append(child)
        actor, first = start_window(foot,swaymsg,ipc,env,log,
                                    "PA-G3C-OUTPUT-CONFIG", (dev1,seat1))
        old = snapshots(log,DENY)
        run_ipc(swaymsg,ipc,env,"output HEADLESS-1 scale 1")
        denial(log,"output_reconfigure",first,old)
        stop(actor); c1.wait_for_window(swaymsg,ipc,env,actor,exists=False)
        actor=None

        actor, second = start_window(foot,swaymsg,ipc,env,log,
                                     "PA-G3C-TARGET-UNMAP", (dev1,seat1))
        old=snapshots(log,DENY)
        stop(actor); c1.wait_for_window(swaymsg,ipc,env,actor,exists=False)
        actor=None
        denial(log,"target_unmap",second,old)

        actor, third=start_window(foot,swaymsg,ipc,env,log,
                                   "PA-G3C-DEVICE-DESTROY",(dev1,seat1))
        old=snapshots(log,DENY)
        client_command(child,"device","PA_G3C_PROTOCOL_DEVICE_REVOKED")
        denial(log,"device_destroy",third,old)
        stop(actor); c1.wait_for_window(swaymsg,ipc,env,actor,exists=False)
        actor=None
        client_command(child,"seat","PA_G3C_PROTOCOL_SEAT_REVOKED")
        client_command(child,"stop","PA_G3C_PROTOCOL_DONE no_input_events=1")
        if child.wait(timeout=12)!=0:
            raise RuntimeError("PA_G3C_FIRST_CLIENT_CLEANUP_FAILED")
        child=None

        # Native transient SEAT-first destruction while virtual device exists.
        child, dev2, seat2 = start_client(client_binary,env,log)
        clients.append(child)
        if dev2 in (dev1,seat1) or seat2 in (dev1,seat1,dev2):
            raise RuntimeError("PA_G3C_NATIVE_REBOUND_TOKEN_REUSED")
        actor, fourth=start_window(foot,swaymsg,ipc,env,log,
                                    "PA-G3C-SEAT-DESTROY",(dev2,seat2))
        old=snapshots(log,DENY)
        client_command(child,"seat","PA_G3C_PROTOCOL_SEAT_REVOKED")
        denial(log,"seat_destroy",fourth,old)
        client_command(child,"device","PA_G3C_PROTOCOL_DEVICE_REVOKED")
        stop(actor); c1.wait_for_window(swaymsg,ipc,env,actor,exists=False)
        actor=None
        client_command(child,"stop","PA_G3C_PROTOCOL_DONE no_input_events=1")
        if child.wait(timeout=12)!=0:
            raise RuntimeError("PA_G3C_SECOND_CLIENT_CLEANUP_FAILED")
        child=None

        child, dev3, seat3 = start_client(client_binary,env,log)
        clients.append(child)
        if len({dev1,seat1,dev2,seat2,dev3,seat3}) != 6:
            raise RuntimeError("PA_G3C_NATIVE_OWNER_ISSUER_TOKEN_REUSE")
        actor, fifth=start_window(foot,swaymsg,ipc,env,log,
                                   "PA-G3C-OUTPUT-DISABLE",(dev3,seat3))
        old=snapshots(log,DENY)
        run_ipc(swaymsg,ipc,env,"output HEADLESS-1 disable")
        denial(log,"output_disable",fifth,old)
        stop(actor);actor=None
        client_command(child,"stop","PA_G3C_PROTOCOL_DONE no_input_events=1")
        if child.wait(timeout=12)!=0:
            raise RuntimeError("PA_G3C_THIRD_CLIENT_CLEANUP_FAILED")
        child=None

        raw=readlog(log)
        if any(token in raw for token in (
                "PA_G3C_NATIVE_DENIAL_FAILURE",
                "PA_G3C_NATIVE_FAKE_START_FAILURE",
                "PA_G3C_NATIVE_FAKE_BASELINE_FAILURE",
                "PA_G3C_VIRTUAL_MISTAKEN_AS_PHYSICAL")):
            raise RuntimeError("PA_G3C_NATIVE_PROBE_NEGATIVE_INVARIANT_BROKEN")
        if len(BASE.findall(raw))!=5 or len(DENY.findall(raw))!=5:
            raise RuntimeError("PA_G3C_FIVE_NATIVE_SESSIONS_NOT_OBSERVED "
                               + repr((len(BASE.findall(raw)),
                                       len(DENY.findall(raw)))))
        print("PA_G3C_REAL_FOUR_OWNER_NEGATIVE_LIFECYCLE_PASS "
              "all_four_real_epochs_per_session=1 five_native_revocations=5 "
              "device_seat_from_REAL_virtual_wayland=1 "
              "virtual_eligible=0 start_origin=FORGED_TEST_ONLY "
              "real_hardware_provenance=ABSENT axis_frames=ZERO",flush=True)
    except Exception:
        print("PA_G3C_PRIVATE_NATIVE_LOG_TAIL",flush=True)
        print("\n".join(readlog(log).splitlines()[-135:]),flush=True)
        raise
    finally:
        stop(actor)
        for p in clients:
            stop(p)
        stop(compositor)
        logfile.close()
        shutil.rmtree(runtime,ignore_errors=True)

if __name__ == "__main__":
    main()
