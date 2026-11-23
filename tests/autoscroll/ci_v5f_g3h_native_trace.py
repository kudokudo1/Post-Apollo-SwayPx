#!/usr/bin/env python3
"""G3H: G3E real virtual middle press/release and native OS diagnostic.

Only a disposable GitHub Nix SwayPX copy is instrumented. The client is
a virtual-pointer / transient-seat Wayland client; it cannot attest HID.
This test MUST NOT arm an autoscroll guard, consume events, send axes/frames
or claim delivery to an app's own mouse handler.
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
import ci_v5f_g2_native_button as g2

EVENT=re.compile(
    r"PA_G3E_REAL_NATIVE_MIDDLE_NEGATIVE "
    r"pressed=(\d+) virtual=(\d+) raw_device=(\d+) eligible=(\d+) "
    r"seat=(\d+) target=(\d+) output=(\d+) "
    r"scene=(\d+) layout_same=(\d+) source_candidate=(\d+) "
    r"snapshot_admitted=(\d+) weather=(\d+) company=(\d+) "
    r"lock=(UNKNOWN|OFF|ON) trust=DENIED axis_frames=0")

G3H_EVENT = re.compile(
    r"PA_G3H_NATIVE_OS_DIAG phase=(\d+) virtual=(\d+) "
    r"source_candidate=(\d+) os_diag=(\d+) owner_snapshot=(\d+) "
    r"device_epoch=(\d+) seat_epoch=(\d+) stage_refusal_clear=(\d+) "
    r"os_mode=TEST_ONLY can_consume=(\d+) can_axis=(\d+) "
    r"production=HARD_OFF")

def require_ci():
    tmp, _ = g2.guard_ci()
    e=os.environ
    keys=("PA_G2_SWAY","PA_G2_CLIENT","PA_G2_MESA",
          "PA_G3E_SWAYMSG","PA_G3E_FOOT")
    paths=[Path(e.get(x,"/missing")) for x in keys]
    if not (e.get("PA_G3E_REAL_MIDDLE_NEGATIVE")=="1"
            and all(str(p).startswith("/nix/store/") for i,p in enumerate(paths)
                    if i != 1)
            and str(paths[1]).startswith(str(tmp)+"/")
            and all(x.is_file() and os.access(x,os.X_OK)
                    for i,x in enumerate(paths) if i!=2)
            and (paths[2]/"share/glvnd/egl_vendor.d/50_mesa.json").is_file()
            and (paths[2]/"lib/dri").is_dir()):
        raise RuntimeError("BLOCKED_PA_G3E_CLIENT_OR_BINARIES_UNPINNED")
    return tmp,paths

def find_transient_seat(swaymsg,ipc,env):
    for _ in range(140):
        seats=c1.run_ipc(swaymsg,ipc,env,"-t","get_seats")
        if isinstance(seats,list):
            transient=[s.get("name") for s in seats
                       if isinstance(s,dict) and
                       str(s.get("name","")).startswith("transient-")]
            if len(transient)==1 and re.fullmatch(
                    r"transient-[a-zA-Z0-9._-]+",transient[0]):
                return transient[0]
        time.sleep(0.05)
    raise RuntimeError("PA_G3E_NATIVE_REAL_TRANSIENT_SEAT_NOT_IDENTIFIED")

def main():
    tmp, (sway,client,mesa,swaymsg,foot)=require_ci()
    runtime=Path(tempfile.mkdtemp(prefix="pa-g3e-virtual-middle-",dir=tmp))
    runtime.chmod(0o700)
    conf=runtime/"sway.conf"
    conf.write_text(
        "font pango:monospace 10\n"
        "output * bg #000000 solid_color\n"
        "output * resolution 1920x1080\n"
        "focus_follows_mouse no\n"
        f'for_window [app_id="{c1.APP}"] floating enable, border none\n',
        encoding="utf-8")
    env=dict(os.environ)
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
        "LD_LIBRARY_PATH":str(mesa/"lib")+
            (":"+env["LD_LIBRARY_PATH"] if env.get("LD_LIBRARY_PATH") else ""),
    })
    for k in ("DISPLAY","WAYLAND_DISPLAY","WAYLAND_SOCKET","SWAYSOCK",
              "WLR_RENDERER","WLR_RENDER_DRM_DEVICE"):
        env.pop(k,None)
    logfile=runtime/"sway.log"
    stream=logfile.open("wb")
    compositor=actor=child=None
    try:
        compositor=subprocess.Popen([str(sway),"-d","-c",str(conf)],
            env=env,stdout=stream,stderr=subprocess.STDOUT)
        ipc=display=None
        for _ in range(220):
            if compositor.poll() is not None:
                raise RuntimeError("PA_G3E_REAL_COMPOSITOR_CRASHED")
            ws=[p for p in runtime.glob("wayland-*") if p.is_socket()]
            ipcs=[p for p in runtime.glob("sway-ipc.*.sock") if p.is_socket()]
            if len(ws)==1 and len(ipcs)==1:
                display,ipc=ws[0],ipcs[0]
                break
            time.sleep(0.07)
        if display is None or ipc is None:
            raise RuntimeError("PA_G3E_REAL_COMPOSITOR_NO_SOCKETS")
        env["WAYLAND_DISPLAY"]=display.name
        for _ in range(90):
            try:
                if isinstance(c1.run_ipc(swaymsg,ipc,env,"-t","get_tree"),dict):
                    break
            except (RuntimeError,subprocess.TimeoutExpired):
                pass
            time.sleep(0.07)
        else:
            raise RuntimeError("PA_G3E_NATIVE_IPC_NOT_READY")

        actor=c1.start_foot(foot,runtime,env,"PA-G3E-REAL-DIRECT-XDG")
        nodeid=c1.wait_for_window(swaymsg,ipc,env,actor,exists=True)
        tree=c1.run_ipc(swaymsg,ipc,env,"-t","get_tree")
        windows=[n for n in c1.find_app(tree)
                 if n.get("pid")==actor.pid and n.get("id")==nodeid]
        if len(windows)!=1:
            raise RuntimeError("PA_G3E_DIRECT_XDG_NOT_UNIQUE")
        rect=windows[0].get("rect",{})
        if not all(isinstance(rect.get(x),int)
                   for x in ("x","y","width","height")):
            raise RuntimeError("PA_G3E_DIRECT_XDG_RECT_ABSENT")
        x=rect["x"]+rect["width"]//2
        y=rect["y"]+rect["height"]//2
        if not (10<x<1910 and 10<y<1070):
            raise RuntimeError("PA_G3E_DIRECT_XDG_NOT_ON_HEADLESS_OUTPUT")

        child=subprocess.Popen([str(client)],env=env,
            stdin=subprocess.PIPE,stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,text=True,bufsize=1)
        g2.read_ready(child,"PA_G2_PROTOCOL_READY transient=1 virtual=1")
        seat=find_transient_seat(swaymsg,ipc,env)
        command=f"seat {seat} cursor set {x} {y}"
        response=c1.run_ipc(swaymsg,ipc,env,command)
        if not (isinstance(response,list) and response and
                response[0].get("success") is True):
            raise RuntimeError("PA_G3E_NATIVE_TRANSIENT_SEAT_CURSOR_WARP_FAILED "
                               +repr(response))
        print("PA_G3E_REAL_NATIVE_TRANSIENT_CURSOR_SCENE_SET_PASS "
              "native_wayland_seat=1 actual_XDG=1",flush=True)

        child.stdin.write("click\n")
        child.stdin.flush()
        g2.read_ready(child,
            "PA_G2_PROTOCOL_MIDDLE_SENT pressed=1 released=1 axis=0")
        raw=g2.wait_log(logfile,
            "PA_G2_NATIVE_FROM_SEATOP state=0")
        os_events=[tuple(int(v) for v in x)
                   for x in G3H_EVENT.findall(raw)]
        if len(os_events) != 2 or [x[0] for x in os_events] != [1,0]:
            raise RuntimeError(
                "PA_G3H_NATIVE_OS_OBSERVER_NOT_RECEIVING_MIDDLE "
                +repr(os_events))
        for row in os_events:
            phase,virtual,source_candidate,diag,snapshot,device_epoch,seat_epoch,first_refusal,consume,axis=row
            if not (virtual==1 and source_candidate==0 and diag==0 and
                    snapshot==0 and device_epoch==0 and seat_epoch>0
                    and first_refusal==1 and consume==0 and axis==0):
                raise RuntimeError("PA_G3H_REAL_OS_DIAGNOSTIC_PROMOTED "
                                   +repr(row))
        if os_events[0][1:] != os_events[1][1:]:
            raise RuntimeError("PA_G3H_REAL_OS_OBSERVER_OWNER_DRIFT "
                               +repr(os_events))
        print("PA_G3H_REAL_NATIVE_OS_MANAGED_DIAGNOSTIC_REFUSED_PASS "
              "real_callback_press_release=2 "
              "virtual_input=1 os_source_candidate=0 "
              "owner_snapshot=DENIED os_diag=DENIED "
              "test_mode=OS_MANAGED no_button_consume=1 no_axis=1",
              flush=True)
        events=[tuple(int(v) if i<13 else v
                      for i,v in enumerate(m)) for m in EVENT.findall(raw)]
        if len(events)!=2 or [event[0] for event in events]!=[1,0]:
            raise RuntimeError("PA_G3E_REAL_NATIVE_MIDDLE_NOT_PAIRED "
                               +repr(events))
        for data in events:
            pressed,virt,raw_device,eligible,seat_epoch,target,output,scene,layout,                candidate,snapshot,weather,company,lock=data
            if not (virt==1 and raw_device>0 and eligible==0 and
                    seat_epoch>0 and target>0 and output>0 and
                    len({raw_device,seat_epoch,target,output})==4 and
                    scene==1 and layout==1 and candidate==0 and
                    snapshot==0 and weather==0 and company==0 and
                    lock=="UNKNOWN"):
                raise RuntimeError("PA_G3E_NATIVE_VIRTUAL_SOURCE_PROMOTED "
                                   +repr(data))
        if events[0][1:13]!=events[1][1:13]:
            raise RuntimeError("PA_G3E_MIDDLE_PAIR_OWNER_OR_SCENE_DRIFT")
        for phase in ("1","0"):
            for token in (
                f"PA_G2_NATIVE_INGRESS state={phase} autoscroll_null=1",
                f"PA_G2_NATIVE_PAIR state={phase} weather=0 company=0",
                f"PA_G2_NATIVE_TO_SEATOP state={phase}",
                f"PA_G2_NATIVE_FROM_SEATOP state={phase}",
            ):
                if raw.count(token)!=1:
                    raise RuntimeError("PA_G3E_NORMAL_BUTTON_PATH_REGRESSED "
                                       +repr((token,raw.count(token))))
        print("PA_G3E_REAL_VIRTUAL_MIDDLE_FOUR_OWNER_SCENE_DENIAL_PASS "
              "pressed_and_released=2 real_native_epochs_four=1 "
              "direct_XDG_hit=1 real_layout_match=1 "
              "libinput_candidate=0 virtual_eligible=0 owner_snapshot=DENIED "
              "physical_origin=UNATTESTED normal_seatop_calls=2 "
              "native_lock=UNKNOWN production_axis_frames=ZERO",
              flush=True)

        child.stdin.write("stop\n")
        child.stdin.flush()
        data,err=child.communicate(timeout=14)
        if child.returncode!=0 or (
                "PA_G2_PROTOCOL_REVOKED device_before_seat=1" not in data):
            raise RuntimeError("PA_G3E_REAL_VIRTUAL_CLIENT_REVOCATION_FAILED "
                               +repr((child.returncode,err[-800:])))
        g2.wait_log(logfile,"PA_FC2_NATIVE DEVICE_REVOKE")
        g2.wait_log(logfile,"PA_FC2_NATIVE SEAT_REVOKE")
        child=None
        print("PA_G3E_REAL_VIRTUAL_MIDDLE_INGRESS_NEGATIVE_PASS "
              "physical_HID=ABSENT synthesized_libinput=NOT_TESTED "
              "no_scroll_axis=1 no_event_swallow=1 "
              "no_production_runtime_activation=1",flush=True)
    except Exception:
        print("PA_G3E_NATIVE_PRIVATE_COMPOSITOR_LOG_TAIL",flush=True)
        print("\n".join(logfile.read_text(
            encoding="utf-8",errors="replace").splitlines()[-125:]),flush=True)
        if child is not None:
            print("PA_G3E_CLIENT_RETURN",child.poll(),flush=True)
        raise
    finally:
        g2.stop(child)
        c1.stop(actor)
        c1.stop(compositor)
        stream.close()
        shutil.rmtree(runtime,ignore_errors=True)

if __name__=="__main__":
    main()
