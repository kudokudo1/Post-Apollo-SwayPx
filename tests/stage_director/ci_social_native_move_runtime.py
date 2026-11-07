#!/usr/bin/env python3
"""Real native seatop_move_floating pair witness; disposable patched SwayPX CI.

Runs the unmodified production test fixture clients against a disposable
source-overlay compositor, then drives real Sway seatop_pointer_motion via a
test-only cursor event entrypoint. Only move is in scope; resize isn't yet.
"""
from __future__ import annotations
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time

import ci_social_real_wayland_pair as b

def ci_require():
    e=os.environ
    if not (e.get("GITHUB_ACTIONS")=="true" and e.get("CI")=="true" and
            e.get("STAGE_DIRECTOR_CI_SOCIAL_NATIVE_MOVE")=="1" and
            e.get("STAGE_DIRECTOR_CI_SOCIAL_REAL_PAIR")=="1" and
            e.get("RUNNER_TEMP")=="/home/runner/work/_temp" and
            os.getuid()!=0):
        raise RuntimeError("BLOCKED_SOCIAL_NATIVE_MOVE_NOT_EPHEMERAL_CI")
    return b.guard()

def exact(swaymsg, ipc, env, app, pid, rect):
    return b.wait_for(swaymsg,ipc,env,app,pid,rect)

def cmd(swaymsg,ipc,env,action):
    return b.sway_command(swaymsg,ipc,env,action)

def ci_leased_move(swaymsg,ipc,env,leader_id,leader_app,leader_proc,
                   follower_app,follower_proc,steps):
    c=f'[con_id="{leader_id}"]'
    cmd(swaymsg,ipc,env,f"{c} focus")
    leader=b.select(b.read_tree(swaymsg,ipc,env),leader_app,leader_proc.pid)
    if leader is None or leader["id"]!=leader_id:
        raise RuntimeError("SD_CI_NATIVE_MOVE_INVALID_LEADER")
    box=leader["rect"]
    cmd(swaymsg,ipc,env,
        f'seat - cursor set {box["x"]+45} {box["y"]+135}')
    cmd(swaymsg,ipc,env,f"{c} focus")
    cmd(swaymsg,ipc,env,"seat - cursor ci-social-start-move 1")
    try:
        for dx,dy in steps:
            cmd(swaymsg,ipc,env,f"seat - cursor ci-social-step {dx} {dy}")
    finally:
        cmd(swaymsg,ipc,env,"seat - cursor ci-social-end 1")
    return sum(x for x,_ in steps),sum(y for _,y in steps)


def main():
    sway,swaymsg,foot,grim,mesa=ci_require()
    runtime=Path(tempfile.mkdtemp(prefix="social-native-seatop-",
                                  dir=os.environ["RUNNER_TEMP"]))
    runtime.chmod(0o700)
    lease=runtime/"pair-lease.txt"
    env=dict(os.environ)
    env.update({
        "XDG_RUNTIME_DIR":str(runtime),"XDG_SESSION_TYPE":"wayland",
        "WLR_BACKENDS":"headless","WLR_HEADLESS_OUTPUTS":"1",
        "WLR_LIBINPUT_NO_DEVICES":"1","WLR_RENDERER_FORCE_SOFTWARE":"1",
        "WLR_RENDERER_ALLOW_SOFTWARE":"1","LIBGL_ALWAYS_SOFTWARE":"1",
        "EGL_PLATFORM":"surfaceless",
        "__EGL_VENDOR_LIBRARY_FILENAMES":str(
            mesa/"share/glvnd/egl_vendor.d/50_mesa.json"),
        "LIBGL_DRIVERS_PATH":str(mesa/"lib/dri"),
        "LD_LIBRARY_PATH":str(mesa/"lib")+(
            ":"+env["LD_LIBRARY_PATH"] if env.get("LD_LIBRARY_PATH") else ""),
        "SD_SOCIAL_CI_LEASE_FILE":str(lease),
    })
    for key in ("WAYLAND_DISPLAY","WAYLAND_SOCKET","DISPLAY","SWAYSOCK",
                "WLR_RENDERER","WLR_RENDER_DRM_DEVICE"):
        env.pop(key,None)
    config=runtime/"sway.conf"
    config.write_text(
        "font pango:monospace 10\n"
        "output * bg #000000 solid_color\n"
        "output * resolution 1920x1080\n"
        "focus_follows_mouse no\n"
        'for_window [app_id="sd-ci-social-host"] floating enable, border none\n'
        'for_window [app_id="sd-ci-social-guest"] floating enable, border none\n'
        'for_window [app_id="sd-ci-social-unrelated"] floating enable, border none\n',
        encoding="utf-8")
    actors=[]
    compositor=None
    log=(runtime/"sway.log").open("wb")
    try:
        compositor=subprocess.Popen([str(sway),"-d","-c",str(config)],
            stdout=log,stderr=subprocess.STDOUT,env=env)
        ipc=None
        display=None
        for _ in range(160):
            if compositor.poll() is not None:
                raise RuntimeError("SD_CI_SOCIAL_NATIVE_COMPOSITOR_EXITED")
            sockets=[s for s in runtime.glob("wayland-*") if s.is_socket()]
            ipc_s=[s for s in runtime.glob("sway-ipc.*.sock") if s.is_socket()]
            if len(sockets)==1 and len(ipc_s)==1:
                display,ipc=sockets[0],ipc_s[0]
                break
            time.sleep(0.07)
        if ipc is None or display is None:
            raise RuntimeError("SD_CI_SOCIAL_NATIVE_MISSING_PRIVATE_SOCKETS")
        env["WAYLAND_DISPLAY"]=display.name
        host=b.foot_process(foot,b.APP_HOST,"SOCIAL-CI-HOST",
                            b.HOST_COLOR,runtime,env)
        guest=b.foot_process(foot,b.APP_GUEST,"SOCIAL-CI-GUEST",
                             b.GUEST_COLOR,runtime,env)
        other=b.foot_process(foot,b.APP_OTHER,"SOCIAL-CI-UNRELATED",
                             b.OTHER_COLOR,runtime,env)
        actors=[host,guest,other]
        ids={}
        for app,proc in ((b.APP_HOST,host),(b.APP_GUEST,guest),(b.APP_OTHER,other)):
            found=b.wait_for(swaymsg,ipc,env,app,proc.pid)
            ids[app]=found["id"]
        if len(set(ids.values()))!=3:
            raise RuntimeError("SD_CI_SOCIAL_NATIVE_DUPLICATE_IDS")
        host_box=(120,110,1000,700)
        guest_box=b.guest_from_chassis(host_box)
        unrelated_box=(1520,660,310,240)
        for app,rect in ((b.APP_HOST,host_box),
                         (b.APP_GUEST,guest_box),
                         (b.APP_OTHER,unrelated_box)):
            b.locate(swaymsg,ipc,env,ids[app],rect)
        exact(swaymsg,ipc,env,b.APP_HOST,host.pid,host_box)
        exact(swaymsg,ipc,env,b.APP_GUEST,guest.pid,guest_box)
        exact(swaymsg,ipc,env,b.APP_OTHER,other.pid,unrelated_box)
        lease.write_text(f"{ids[b.APP_HOST]} {ids[b.APP_GUEST]} "
                         f"{host.pid} {guest.pid}\n",encoding="utf-8")
        lease.chmod(0o600)

        # HOST-LED: the test-only cursor starter enters *real native seatop*.
        # Every subsequent step executes the native pointer-motion handler.
        dx,dy=ci_leased_move(swaymsg,ipc,env,ids[b.APP_HOST],b.APP_HOST,
                            host,b.APP_GUEST,guest,[(55,25),(40,20)])
        host_box=(host_box[0]+dx,host_box[1]+dy,*host_box[2:])
        guest_box=b.guest_from_chassis(host_box)
        exact(swaymsg,ipc,env,b.APP_HOST,host.pid,host_box)
        exact(swaymsg,ipc,env,b.APP_GUEST,guest.pid,guest_box)
        exact(swaymsg,ipc,env,b.APP_OTHER,other.pid,unrelated_box)
        cmd(swaymsg,ipc,env,f'[con_id="{ids[b.APP_GUEST]}"] focus')
        b.capture(grim,runtime/"host-native.ppm",env,[
            ("rail",(host_box[0]+26,host_box[1]+150),b.HOST_COLOR),
            ("guest",(guest_box[0]+170,guest_box[1]+150),b.GUEST_COLOR),
            ("unrelated",(unrelated_box[0]+130,unrelated_box[1]+110),
             b.OTHER_COLOR),
            ("background",(25,25),(0,0,0)),
        ])
        print("SD_CI_SOCIAL_NATIVE_HOST_DRAG seatop=1 peer_follow=1 "
              "exact_geometry=PASS pixels=PASS",flush=True)

        # GUEST-LED: native seatop must carry chassis using the same lease.
        dx,dy=ci_leased_move(swaymsg,ipc,env,ids[b.APP_GUEST],b.APP_GUEST,
                            guest,b.APP_HOST,host,[(-35,35),(-20,10)])
        host_box=(host_box[0]+dx,host_box[1]+dy,*host_box[2:])
        guest_box=b.guest_from_chassis(host_box)
        exact(swaymsg,ipc,env,b.APP_HOST,host.pid,host_box)
        exact(swaymsg,ipc,env,b.APP_GUEST,guest.pid,guest_box)
        exact(swaymsg,ipc,env,b.APP_OTHER,other.pid,unrelated_box)
        cmd(swaymsg,ipc,env,f'[con_id="{ids[b.APP_GUEST]}"] focus')
        b.capture(grim,runtime/"guest-native.ppm",env,[
            ("rail",(host_box[0]+25,host_box[1]+155),b.HOST_COLOR),
            ("guest",(guest_box[0]+170,guest_box[1]+155),b.GUEST_COLOR),
            ("unrelated",(unrelated_box[0]+130,unrelated_box[1]+110),
             b.OTHER_COLOR),
            ("background",(25,25),(0,0,0)),
        ])
        print("SD_CI_SOCIAL_NATIVE_GUEST_DRAG seatop=1 peer_follow=1 "
              "exact_geometry=PASS pixels=PASS",flush=True)

        # Negative: a stale guest PID must not attach the host to another
        # window, even though the host itself may still be dragged.
        lease.write_text(f"{ids[b.APP_HOST]} {ids[b.APP_GUEST]} "
                         f"{host.pid} {guest.pid+1000}\n",encoding="utf-8")
        lease.chmod(0o600)
        old_guest=guest_box
        dx,dy=ci_leased_move(swaymsg,ipc,env,ids[b.APP_HOST],b.APP_HOST,
                            host,b.APP_GUEST,guest,[(20,5)])
        host_box=(host_box[0]+dx,host_box[1]+dy,*host_box[2:])
        exact(swaymsg,ipc,env,b.APP_HOST,host.pid,host_box)
        exact(swaymsg,ipc,env,b.APP_GUEST,guest.pid,old_guest)
        exact(swaymsg,ipc,env,b.APP_OTHER,other.pid,unrelated_box)
        print("SD_CI_SOCIAL_NATIVE_STALE_LEASE stale_pid=REJECTED "
              "unrelated_unchanged=1 guest_unchanged=1",flush=True)
        log.flush()
        logs=(runtime/"sway.log").read_text(encoding="utf-8",errors="replace")
        if logs.count("SD_CI_SOCIAL_NATIVE_ATTACH accepted=1") < 2:
            raise RuntimeError("SD_CI_SOCIAL_NATIVE_MISSING_REAL_ATTACH")
        if logs.count("SD_CI_SOCIAL_NATIVE_FOLLOW before_commit=1") < 4:
            raise RuntimeError("SD_CI_SOCIAL_NATIVE_MISSING_PRECOMMIT_FOLLOW")
        if "SD_CI_SOCIAL_NATIVE_ATTACH denied=1" not in logs:
            raise RuntimeError("SD_CI_SOCIAL_NATIVE_STALE_NOT_DENIED")
        print("SD_CI_SOCIAL_NATIVE_MOVE final=PASS "
              "native_seatop=1 same_precommit=1 "
              "real_physical_pointer=NOT_TESTED "
              "real_resize=NOT_IMPLEMENTED "
              "live_desktop=UNCHANGED",flush=True)
    except Exception:
        log.flush()
        try:
            print((runtime/"sway.log").read_text(
                encoding="utf-8",errors="replace")[-6500:],file=sys.stderr)
        except OSError:
            pass
        raise
    finally:
        for p in reversed(actors):
            b.quiet_stop(p)
        if compositor is not None:
            b.quiet_stop(compositor)
        log.close()
        shutil.rmtree(runtime,ignore_errors=True)

if __name__=="__main__":
    main()
