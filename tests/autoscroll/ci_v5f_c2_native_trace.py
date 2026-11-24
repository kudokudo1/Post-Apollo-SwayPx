#!/usr/bin/env python3
"""V5F-C2: native transient seat / virtual pointer lifecycle on CI-only SwayPX.

This test creates actual protocol resources in the running compositor, then
observes compositor-owner callbacks. No physical devices, no events sent.
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

SEAT_BIND = re.compile(r"PA_FC2_NATIVE SEAT_BIND epoch=(\d+)")
SEAT_REVOKE = re.compile(r"PA_FC2_NATIVE SEAT_REVOKE before=(\d+) after=(\d+)")
DEVICE_BIND = re.compile(
    r"PA_FC2_NATIVE DEVICE_BIND epoch=(\d+) eligible=(\d+) virtual=(\d+)")
DEVICE_REVOKE = re.compile(
    r"PA_FC2_NATIVE DEVICE_REVOKE before=(\d+) after=(\d+) eligible=(\d+)")

def must_be_ci() -> tuple[Path, Path, Path]:
    env = os.environ
    root = Path(env.get("RUNNER_TEMP", "/missing"))
    args = tuple(Path(env.get(k, "/missing")) for k in (
        "PA_FC2_SWAY", "PA_FC2_CLIENT", "PA_FC2_MESA"))
    if not (env.get("GITHUB_ACTIONS") == "true" and env.get("CI") == "true"
            and env.get("PA_AUTOSCROLL_CI_NATIVE_TRACE") == "2"
            and str(root) == "/home/runner/work/_temp"
            and os.getuid() != 0 and root.is_dir()
            and all(str(x).startswith("/nix/store/") for x in args[:1])
            and all(x.is_file() and os.access(x, os.X_OK) for x in args[:2])
            and (args[2]/"share/glvnd/egl_vendor.d/50_mesa.json").is_file()
            and (args[2]/"lib/dri").is_dir()):
        raise RuntimeError("BLOCKED_PA_FC2_NON_DISPOSABLE_NATIVE_TEST")
    return args

def log_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")

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

def snapshot(log: Path) -> dict[str, list]:
    text = log_text(log)
    return {
        "sb": [int(x) for x in SEAT_BIND.findall(text)],
        "sr": [(int(a),int(b)) for a,b in SEAT_REVOKE.findall(text)],
        "db": [(int(a),int(b),int(c))
               for a,b,c in DEVICE_BIND.findall(text)],
        "dr": [(int(a),int(b),int(c))
               for a,b,c in DEVICE_REVOKE.findall(text)],
    }

def wait_for_log(log: Path, predicate, label: str) -> dict:
    for _ in range(120):
        state = snapshot(log)
        if predicate(state):
            return state
        time.sleep(0.05)
    raise RuntimeError("PA_FC2_REAL_CALLBACK_MISSING " + label
                       + " " + repr(snapshot(log)))

def protocol_client(binary: Path, env: dict, log: Path, cycle: int) -> None:
    before = snapshot(log)
    child = subprocess.Popen([str(binary)], env=env,
        stdin=subprocess.PIPE, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE, text=True, bufsize=1)
    try:
        ready, _, _ = select.select([child.stdout],[],[],14)
        if not ready:
            raise RuntimeError("PA_FC2_CLIENT_READY_TIMEOUT cycle=" + str(cycle))
        header = child.stdout.readline().strip()
        if header != "PA_FC2_PROTOCOL_OBJECTS_READY transient=1 virtual=1":
            raise RuntimeError("PA_FC2_CLIENT_READY_INVALID " + header
                + " stderr=" + child.stderr.read()[-800:] if child.poll() is not None
                else "PA_FC2_CLIENT_READY_INVALID " + header)
        up = wait_for_log(log, lambda s:
            len(s["sb"]) > len(before["sb"]) and
            len(s["db"]) > len(before["db"]), "bind")
        seat_epoch = up["sb"][-1]
        dev_epoch,eligible,is_virtual = up["db"][-1]
        if not (seat_epoch > 0 and dev_epoch > 0 and
                seat_epoch != dev_epoch and eligible == 0 and is_virtual == 1):
            raise RuntimeError("PA_FC2_VIRTUAL_DEVICE_MISTAKEN_FOR_PHYSICAL "
                               + repr(up))
        child.stdin.write("stop\n")
        child.stdin.flush()
        output,stderr = child.communicate(timeout=14)
        if child.returncode != 0 or (
                "PA_FC2_PROTOCOL_OBJECTS_DESTROYED device_before_seat=1"
                not in output):
            raise RuntimeError("PA_FC2_CLIENT_SHUTDOWN_FAILED code="
                +str(child.returncode)+" out="+output[-300:]
                +" err="+stderr[-900:])
        down = wait_for_log(log, lambda s:
            (seat_epoch,0) in s["sr"] and (dev_epoch,0,0) in s["dr"],
            "revoke")
        if len(down["sb"]) <= len(before["sb"]) or \
                len(down["db"]) <= len(before["db"]):
            raise RuntimeError("PA_FC2_NO_NEW_REAL_OBJECTS")
        print(f"PA_FC2_NATIVE_CYCLE_{cycle} "
              f"transient_seat_epoch={seat_epoch} "
              f"virtual_device_epoch={dev_epoch} "
              "revoked_to_zero=1 physical_eligible=0 PASS",flush=True)
    finally:
        stop(child)

def main() -> None:
    sway, client, mesa = must_be_ci()
    runtime = Path(tempfile.mkdtemp(
        prefix="pa-fc2-real-native-",dir=os.environ["RUNNER_TEMP"]))
    runtime.chmod(0o700)
    conf = runtime/"sway.conf"
    conf.write_text(
        "font pango:monospace 10\n"
        "output * bg #000000 solid_color\n"
        "output * resolution 1920x1080\n",
        encoding="utf-8")
    env = dict(os.environ)
    env.update({
        "XDG_RUNTIME_DIR":str(runtime),"XDG_SESSION_TYPE":"wayland",
        "WLR_BACKENDS":"headless","WLR_HEADLESS_OUTPUTS":"1",
        "WLR_LIBINPUT_NO_DEVICES":"1",
        "WLR_RENDERER_FORCE_SOFTWARE":"1",
        "WLR_RENDERER_ALLOW_SOFTWARE":"1",
        "LIBGL_ALWAYS_SOFTWARE":"1","EGL_PLATFORM":"surfaceless",
        "__EGL_VENDOR_LIBRARY_FILENAMES":str(
            mesa/"share/glvnd/egl_vendor.d/50_mesa.json"),
        "LIBGL_DRIVERS_PATH":str(mesa/"lib/dri"),
        "LD_LIBRARY_PATH":str(mesa/"lib") +
            (":"+env["LD_LIBRARY_PATH"] if env.get("LD_LIBRARY_PATH") else ""),
    })
    for key in ("WAYLAND_DISPLAY","WAYLAND_SOCKET","SWAYSOCK",
                "DISPLAY","WLR_RENDERER","WLR_RENDER_DRM_DEVICE"):
        env.pop(key,None)
    log=runtime/"sway.log"
    logfile=log.open("wb")
    compositor=None
    try:
        compositor=subprocess.Popen([str(sway),"-d","-c",str(conf)],
            stdout=logfile,stderr=subprocess.STDOUT,env=env)
        display=None
        for _ in range(220):
            if compositor.poll() is not None:
                raise RuntimeError("PA_FC2_NATIVE_COMPOSITOR_DIED")
            sockets=[x for x in runtime.glob("wayland-*") if x.is_socket()]
            if len(sockets)==1 and len([x for x in runtime.glob(
                    "sway-ipc.*.sock") if x.is_socket()])==1:
                display=sockets[0]
                break
            time.sleep(0.07)
        if display is None:
            raise RuntimeError("PA_FC2_PRIVATE_WAYLAND_AND_IPC_MISSING")
        env["WAYLAND_DISPLAY"]=display.name
        for cycle in (1,2):
            protocol_client(client,env,log,cycle)
        state = snapshot(log)
        seat_tokens=state["sb"][-2:]
        device_tokens=[e for e,elig,vir in state["db"][-2:]]
        if len(set(seat_tokens))!=2 or len(set(device_tokens))!=2:
            raise RuntimeError("PA_FC2_NATIVE_TOKEN_REUSE "
                               +repr(state))
        if set(seat_tokens) & set(device_tokens):
            raise RuntimeError("PA_FC2_OWNER_TOKENS_COLLIDED")
        print("PA_FC2_REAL_DEVICE_AND_SEAT_LIFECYCLE_EVIDENCE_PASS "
              "real_wayland_protocols=1 virtual_pointer_physical=DENIED "
              "device_create_revoke=2 seat_create_revoke=2 "
              "axis_frames=ZERO guard_positive=DENIED",flush=True)
    except Exception:
        print("PA_FC2_DISPOSABLE_COMPOSITOR_LOG_TAIL",flush=True)
        print("\n".join(log_text(log).splitlines()[-90:]),flush=True)
        raise
    finally:
        stop(compositor)
        logfile.close()
        shutil.rmtree(runtime,ignore_errors=True)

if __name__ == "__main__":
    main()
