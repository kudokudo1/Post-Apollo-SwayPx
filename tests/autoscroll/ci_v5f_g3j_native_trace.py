#!/usr/bin/env python3
"""G3J: actual kernel -> libinput -> wlroots -> SwayPX native callback.
CI ONLY; runner root helpers constrained to ephemeral uinput & seatd.
This explicitly virtual kernel device must NEVER be trusted physical.
"""
from __future__ import annotations
import grp
import os
from pathlib import Path
import pwd
import re
import select
import shutil
import signal
import socket
import stat
import subprocess
import tempfile
import time

import ci_v5f_c1_native_trace as c1
import ci_v5f_g2_native_button as g2

EVENT=re.compile(
    r"PA_G3J_REAL_COMPOSITOR_LIBINPUT_MIDDLE "
    r"pressed=(\d+) native_libinput=1 real_handle=1 "
    r"kernel_event=(\d+) kernel_virtual=(\d+) registered=(\d+) "
    r"device_epoch=(\d+) seat_epoch=(\d+) "
    r"owner_snapshot=(\d+) target_epoch=(\d+) output_epoch=(\d+) "
    r"os_source_candidate=(\d+) weather=(\d+) company=(\d+) "
    r"time_nonzero=(\d+) physical_attestation=NONE "
    r"can_consume=0 can_axis=0")
G3H=re.compile(
    r"PA_G3H_NATIVE_OS_DIAG phase=(\d+) virtual=(\d+) "
    r"source_candidate=(\d+) os_diag=(\d+) owner_snapshot=(\d+) "
    r"device_epoch=(\d+) seat_epoch=(\d+) stage_refusal_clear=(\d+) "
    r"os_mode=TEST_ONLY can_consume=(\d+) can_axis=(\d+) "
    r"production=HARD_OFF")

def guard():
    tmp,ws=g2.guard_ci()
    e=os.environ
    if not(e.get("RUNNER_ENVIRONMENT")=="github-hosted" and
           e.get("PA_G3J_NATIVE_LIBINPUT_DIAG")=="1" and
           e.get("PA_G3J_KERNEL_ACTOR")=="1" and
           e.get("PA_G3E_REAL_MIDDLE_NEGATIVE")=="1" and
           e.get("PA_G3H_NATIVE_OS_DIAG")=="1" and
           os.getuid()!=0):
        raise RuntimeError("BLOCKED_PA_G3J_NOT_DISPOSABLE_CI")
    sway=Path(e.get("PA_G2_SWAY","/missing"))
    foot=Path(e.get("PA_G3E_FOOT","/missing"))
    swaymsg=Path(e.get("PA_G3E_SWAYMSG","/missing"))
    mesa=Path(e.get("PA_G2_MESA","/missing"))
    actor=Path(e.get("PA_G3J_ACTOR_BINARY","/missing"))
    paths=(sway,foot,swaymsg,mesa)
    if not (all(str(x).startswith("/nix/store/") for x in paths) and
            all(x.is_file() and os.access(x,os.X_OK)
                for x in (sway,foot,swaymsg)) and
            (mesa/"share/glvnd/egl_vendor.d/50_mesa.json").is_file() and
            (mesa/"lib/dri").is_dir() and
            str(actor).startswith(str(tmp)+"/") and
            actor.is_file() and os.access(actor,os.X_OK)):
        raise RuntimeError("PA_G3J_UNPINNED_TEST_BINARY")
    return tmp,ws,sway,foot,swaymsg,mesa,actor

def expect_prefix(proc,prefix,timeout=15):
    ready,_,_=select.select([proc.stdout],[],[],timeout)
    if not ready:
        raise RuntimeError("PA_G3J_ACTOR_PROTOCOL_TIMEOUT "+prefix)
    line=proc.stdout.readline().strip()
    if not line.startswith(prefix):
        raise RuntimeError("PA_G3J_ACTOR_PROTOCOL_UNEXPECTED "+repr(line))
    return line

def find_one_event_node(sysname):
    if not re.fullmatch(r"input[0-9]+",sysname):
        raise RuntimeError("PA_G3J_KERNEL_SYSNAME_INVALID")
    root=Path("/sys/devices/virtual/input")/sysname
    for _ in range(140):
        events=list(root.glob("event[0-9]*"))
        if len(events)==1:
            event=events[0].name
            node=Path("/dev/input")/event
            if node.is_char_device():
                return node
        time.sleep(.05)
    raise RuntimeError("PA_G3J_NO_REAL_KERNEL_EVENT_NODE")

def wait_socket(proc,path,timeout=12):
    for _ in range(int(timeout/.05)):
        if proc.poll() is not None:
            raise RuntimeError("PA_G3J_PRIVATE_SEATD_EXITED")
        if path.exists() and stat.S_ISSOCK(path.stat().st_mode):
            return
        time.sleep(.05)
    raise RuntimeError("PA_G3J_PRIVATE_SEATD_SOCKET_NOT_READY")

def main():
    tmp,ws,sway,foot,swaymsg,mesa,actor_bin=guard()
    seatd_bin=Path(os.environ.get("PA_G3J_SEATD","/missing"))
    if not (str(seatd_bin).startswith("/nix/store/") and
            seatd_bin.is_file() and os.access(seatd_bin,os.X_OK)):
        raise RuntimeError("PA_G3J_PRIVATE_SEATD_BINARY_UNPINNED")
    seatd_socket=Path("/run/seatd.sock")
    if seatd_socket.exists():
        raise RuntimeError("PA_G3J_PREEXISTING_SEATD_REFUSE_TO_TOUCH")
    runtime=Path(tempfile.mkdtemp(prefix="pa-g3j-headless-libinput-",dir=tmp))
    runtime.chmod(0o700)
    conf=runtime/"sway.conf"
    conf.write_text(
        "font pango:monospace 10\n"
        "output * bg #000000 solid_color\n"
        "output * resolution 1920x1080\n"
        "focus_follows_mouse no\n"
        f'for_window [app_id="{c1.APP}"] floating enable, border none\n'
    )
    actor=None
    seatd=None
    compositor=None
    window=None
    log=runtime/"sway.log"
    f=log.open("wb")
    try:
        # CREATE one real synthetic kernel device, before libinput scans udev.
        root_env={key:os.environ[key] for key in (
            "CI","GITHUB_ACTIONS","RUNNER_ENVIRONMENT","RUNNER_TEMP",
            "PA_G3J_KERNEL_ACTOR")}
        actor=subprocess.Popen(["sudo","-n","env"]+
            [f"{k}={v}" for k,v in root_env.items()]+[str(actor_bin)],
            stdin=subprocess.PIPE,stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,text=True,bufsize=1)
        hello=expect_prefix(actor,"PA_G3J_KERNEL_UINPUT_READY input=")
        match=re.fullmatch(
            r"PA_G3J_KERNEL_UINPUT_READY input=(input[0-9]+) "
            r"fake_bus_USB=1",hello)
        if match is None:
            raise RuntimeError("PA_G3J_ACTOR_INIT_NOT_KERNEL_EVIDENCE")
        node=find_one_event_node(match.group(1))
        settle=subprocess.run(["udevadm","settle","--timeout=15"],
            capture_output=True,text=True,timeout=20)
        if settle.returncode!=0:
            raise RuntimeError("PA_G3J_UDEV_SETTLE_FAILED "+settle.stderr)
        props=subprocess.run(["udevadm","info","--query=property",
                              f"--name={node}"],
            capture_output=True,text=True,timeout=10,check=True).stdout
        if "ID_INPUT_MOUSE=1" not in props.splitlines():
            raise RuntimeError("PA_G3J_NATIVE_UDEV_MOUSE_TAG_MISSING")
        print("PA_G3J_REAL_KERNEL_UDEV_INPUT_READY_PASS "
              "fake_bus_USB=1 virtual_kernel=1 device_tags=REAL",
              flush=True)

        # Spawn a temporary seat daemon ONLY after refusing existing seatd.
        # Its default /run/seatd.sock remains in this disposable runner VM.
        user=pwd.getpwuid(os.getuid()).pw_name
        group=grp.getgrgid(os.getgid()).gr_name
        seatd=subprocess.Popen(
            ["sudo","-n","env","SEATD_VTBOUND=0",
             str(seatd_bin),"-u",user,"-g",group,"-l","error"],
            stdout=subprocess.DEVNULL,stderr=subprocess.PIPE,
            start_new_session=True)
        wait_socket(seatd,seatd_socket)
        print("PA_G3J_PRIVATE_SEATD_READY_PASS github_runner=1",flush=True)

        env=dict(os.environ)
        env.update({
            "XDG_RUNTIME_DIR":str(runtime),"XDG_SESSION_TYPE":"wayland",
            "LIBSEAT_BACKEND":"seatd","SEATD_VTBOUND":"0",
            "WLR_BACKENDS":"headless,libinput","WLR_HEADLESS_OUTPUTS":"1",
            "WLR_RENDERER_FORCE_SOFTWARE":"1",
            "WLR_RENDERER_ALLOW_SOFTWARE":"1",
            "LIBGL_ALWAYS_SOFTWARE":"1","EGL_PLATFORM":"surfaceless",
            "__EGL_VENDOR_LIBRARY_FILENAMES":str(
                mesa/"share/glvnd/egl_vendor.d/50_mesa.json"),
            "LIBGL_DRIVERS_PATH":str(mesa/"lib/dri"),
            "LD_LIBRARY_PATH":str(mesa/"lib")+
                (":"+env["LD_LIBRARY_PATH"]
                    if env.get("LD_LIBRARY_PATH") else ""),
        })
        env.pop("WLR_LIBINPUT_NO_DEVICES",None)
        for k in ("DISPLAY","WAYLAND_DISPLAY","WAYLAND_SOCKET",
                  "SWAYSOCK","WLR_RENDERER","WLR_RENDER_DRM_DEVICE"):
            env.pop(k,None)
        compositor=subprocess.Popen([str(sway),"-d","-c",str(conf)],
            stdout=f,stderr=subprocess.STDOUT,env=env)
        ipc=display=None
        for _ in range(250):
            if compositor.poll() is not None:
                raise RuntimeError("PA_G3J_REAL_LIBINPUT_COMPOSITOR_EXITED")
            wsockets=[x for x in runtime.glob("wayland-*") if x.is_socket()]
            ipcs=[x for x in runtime.glob("sway-ipc.*.sock") if x.is_socket()]
            if len(wsockets)==1 and len(ipcs)==1:
                display,ipc=wsockets[0],ipcs[0]
                break
            time.sleep(.06)
        if display is None or ipc is None:
            raise RuntimeError("PA_G3J_NO_NATIVE_COMPOSITOR_IPC")
        env["WAYLAND_DISPLAY"]=display.name

        inputs=None
        for _ in range(160):
            try:
                inputs=c1.run_ipc(swaymsg,ipc,env,"-t","get_inputs")
                if isinstance(inputs,list) and any(
                    isinstance(i,dict) and
                    i.get("name")=="PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL" and
                    i.get("type")=="pointer" for i in inputs):
                    break
            except (RuntimeError,subprocess.TimeoutExpired):
                pass
            time.sleep(.05)
        else:
            raise RuntimeError("PA_G3J_REAL_SAYPX_LIBINPUT_NOT_REGISTERED "+
                               repr(inputs))
        print("PA_G3J_REAL_COMPOSITOR_LIBINPUT_REGISTERED_PASS "
              "native_input_manager=1 not_wayland_virtual=1",flush=True)

        window=c1.start_foot(foot,runtime,env,"PA-G3J-REAL-LIBINPUT-XDG")
        nodeid=c1.wait_for_window(swaymsg,ipc,env,window,exists=True)
        tree=c1.run_ipc(swaymsg,ipc,env,"-t","get_tree")
        target=[i for i in c1.find_app(tree) if
                i.get("pid")==window.pid and i.get("id")==nodeid]
        if len(target)!=1:
            raise RuntimeError("PA_G3J_DIRECT_XDG_NOT_UNIQUE")
        rect=target[0].get("rect",{})
        if not all(isinstance(rect.get(k),int)
                   for k in ("x","y","width","height")):
            raise RuntimeError("PA_G3J_TARGET_RECT_UNKNOWN")
        x=rect["x"]+rect["width"]//2
        y=rect["y"]+rect["height"]//2
        if not (10<x<1910 and 10<y<1070):
            raise RuntimeError("PA_G3J_CURSOR_TARGET_OUTSIDE_OUTPUT")
        response=c1.run_ipc(swaymsg,ipc,env,
                            f"seat seat0 cursor set {x} {y}")
        if not (isinstance(response,list) and response and
                response[0].get("success") is True):
            raise RuntimeError("PA_G3J_NATIVE_SEAT_CURSOR_SET_FAILED "+
                               repr(response))
        print("PA_G3J_NATIVE_DIRECT_XDG_CURSOR_READY_PASS",flush=True)

        actor.stdin.write("click\n")
        actor.stdin.flush()
        sent=expect_prefix(actor,"PA_G3J_KERNEL_MIDDLE_SENT ")
        if sent!="PA_G3J_KERNEL_MIDDLE_SENT pressed=1 released=1 axes=0":
            raise RuntimeError("PA_G3J_MIDDLE_ACTOR_RESULT_INCONSISTENT")
        raw=g2.wait_log(log,"PA_G2_NATIVE_FROM_SEATOP state=0",timeout=15)
        matches=[tuple(map(int,m)) for m in EVENT.findall(raw)]
        if len(matches)!=2 or [row[0] for row in matches]!=[1,0]:
            raise RuntimeError("PA_G3J_REAL_COMPOSITOR_CALLBACK_COUNT "+
                               repr(matches))
        for row in matches:
            (press,event,virtual,registered,device_epoch,seat_epoch,
             owner,target_epoch,output_epoch,candidate,weather,company,
             nonzero)=row
            if not(event==1 and virtual==1 and registered==1 and
                   device_epoch>0 and seat_epoch>0 and
                   candidate==0 and weather==0 and company==0 and
                   nonzero==1):
                raise RuntimeError("PA_G3J_NATIVE_LIBINPUT_SOURCE_NOT_DENIED "+
                                   repr(row))
        for phase in (1,0):
            for token in (
                f"PA_G2_NATIVE_INGRESS state={phase} autoscroll_null=1",
                f"PA_G2_NATIVE_PAIR state={phase} weather=0 company=0",
                f"PA_G2_NATIVE_TO_SEATOP state={phase}",
                f"PA_G2_NATIVE_FROM_SEATOP state={phase}"
            ):
                if raw.count(token)!=1:
                    raise RuntimeError("PA_G3J_NORMAL_INPUT_NOT_PRESERVED "+
                                       repr((token,raw.count(token))))
        os_events=[tuple(map(int,m)) for m in G3H.findall(raw)]
        if len(os_events)!=2 or [e[0] for e in os_events]!=[1,0] or any(
            e[1]!=0 or e[2]!=0 or e[3]!=0 or e[8]!=0 or e[9]!=0
            for e in os_events):
            raise RuntimeError("PA_G3J_REAL_OS_MANAGED_MODEL_PROMOTED "+
                               repr(os_events))
        print("PA_G3J_REAL_COMPOSITOR_LIBINPUT_MIDDLE_SOURCE_DENIED_PASS "
              "real_wlroots_libinput_callbacks=2 kernel_virtual=1 "
              "source_candidate=0 model_denied=1 "
              "normal_middle_dispatch=2 axis_frames=0",flush=True)

        actor.stdin.write("stop\n")
        actor.stdin.flush()
        end,out=actor.communicate(timeout=12)
        if actor.returncode!=0 or \
           "PA_G3J_KERNEL_UINPUT_DESTROYED_PASS" not in end:
            raise RuntimeError("PA_G3J_KERNEL_DEVICE_CLEANUP_FAILURE "+
                               repr((actor.returncode,end,out[-1000:])))
        actor=None
        print("PA_G3J_REAL_NATIVE_INPUT_DEVICE_DESTROYED_PASS",flush=True)
    except Exception:
        print("PA_G3J_REAL_NATIVE_COMPOSITOR_LOG_TAIL",flush=True)
        print("\n".join(log.read_text(errors="replace").splitlines()[-120:]),
              flush=True)
        raise
    finally:
        g2.stop(actor)
        c1.stop(window)
        c1.stop(compositor)
        f.close()
        if seatd is not None:
            # Only this unique Popen process group, on disposable VM.
            pgid=os.getpgid(seatd.pid) if seatd.poll() is None else None
            if pgid is not None:
                subprocess.run(
                    ["sudo","-n","kill","-TERM","--",f"-{pgid}"],
                    stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,
                    timeout=8)
            try:
                seatd.wait(timeout=6)
            except subprocess.TimeoutExpired:
                seatd.kill()
                seatd.wait(timeout=6)
        shutil.rmtree(runtime,ignore_errors=True)
if __name__=="__main__":
    main()
