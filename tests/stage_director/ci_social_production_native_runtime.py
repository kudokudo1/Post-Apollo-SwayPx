#!/usr/bin/env python3
"""Real XDG Social + Vesktop native pairing through production SwayPX.

Only CI's private cursor test-adapter is instrumented; the source of
production native seatop move/resize and Stage Director group is UNMODIFIED.
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
import ci_social_production_fixture as b


def command(swaymsg,ipc,env,action,success=True):
    result=json.loads(b.run(str(swaymsg),"-s",str(ipc),"-r",action,env=env))
    passed=isinstance(result,list) and len(result)==1 and isinstance(
        result[0],dict) and result[0].get("success") is True
    if passed is not success:
        raise RuntimeError("SD_SOCIAL_PRODUCTION_COMMAND_UNEXPECTED "
                           f"expect={success} action={action} result={result}")
    return passed


def gesture(swaymsg,ipc,env,leader_id,leader_app,proc,kind,dx,dy):
    c=f'[con_id="{leader_id}"]'
    command(swaymsg,ipc,env,f"{c} focus")
    node=b.select(b.read_tree(swaymsg,ipc,env),leader_app,proc.pid)
    if node is None or node["id"]!=leader_id:
        raise RuntimeError("SD_SOCIAL_PRODUCTION_WRONG_LEADER")
    rect=node["rect"]
    command(swaymsg,ipc,env,
            f'seat - cursor set {rect["x"]+45} {rect["y"]+135}')
    command(swaymsg,ipc,env,f"{c} focus")
    command(swaymsg,ipc,env,"seat - cursor ci-social-start-"+kind+" 1")
    try:
        command(swaymsg,ipc,env,f"seat - cursor ci-social-step {dx} {dy}")
    finally:
        command(swaymsg,ipc,env,"seat - cursor ci-social-end 1")


def main():
    if (os.environ.get("STAGE_DIRECTOR_CI_SOCIAL_NATIVE_MOVE")!="1" or
            os.environ.get("STAGE_DIRECTOR_CI_SOCIAL_REAL_PAIR")!="1" or
            os.environ.get("GITHUB_ACTIONS")!="true"):
        raise RuntimeError("BLOCKED_SD_SOCIAL_PRODUCTION_TEST_NOT_CI")
    sway,swaymsg,foot,grim,mesa=b.guard()
    runtime=Path(tempfile.mkdtemp(prefix="social-production-native-",
                                  dir=os.environ["RUNNER_TEMP"]))
    runtime.chmod(0o700)
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
    })
    for name in ("WAYLAND_DISPLAY","WAYLAND_SOCKET","DISPLAY","SWAYSOCK",
                 "WLR_RENDERER","WLR_RENDER_DRM_DEVICE"):
        env.pop(name,None)
    config=runtime/"sway.conf"
    config.write_text(
        "font pango:monospace 10\n"
        "output * bg #000000 solid_color\n"
        "output * resolution 1920x1080\n"
        "focus_follows_mouse no\n"
        'for_window [app_id="sd-ci-social-host"] floating enable, border none\n'
        'for_window [app_id="vesktop"] floating enable, border none\n'
        'for_window [app_id="sd-ci-social-unrelated"] floating enable, border none\n',
        encoding="utf-8")
    actors=[]
    compositor=None
    log=(runtime/"sway.log").open("wb")
    try:
        compositor=subprocess.Popen([str(sway),"-d","-c",str(config)],
            stdout=log,stderr=subprocess.STDOUT,env=env)
        ipc=display=None
        for _ in range(160):
            if compositor.poll() is not None:
                raise RuntimeError("SD_SOCIAL_PRODUCTION_SWAY_EXITED")
            sockets=[q for q in runtime.glob("wayland-*") if q.is_socket()]
            ipcs=[q for q in runtime.glob("sway-ipc.*.sock") if q.is_socket()]
            if len(sockets)==1 and len(ipcs)==1:
                display,ipc=sockets[0],ipcs[0]
                break
            time.sleep(0.07)
        if not display or not ipc:
            raise RuntimeError("SD_SOCIAL_PRODUCTION_NO_PRIVATE_SOCKETS")
        env["WAYLAND_DISPLAY"]=display.name
        host_id="sd-ci-social-host";guest_id="vesktop";other_id="sd-ci-social-unrelated"
        host=b.foot_process(foot,host_id,"QS_SOCIAL_MENU",
                            b.HOST_COLOR,runtime,env)
        guest=b.foot_process(foot,guest_id,"VESKTOP MAIN",
                             b.GUEST_COLOR,runtime,env)
        unrelated=b.foot_process(foot,other_id,"UNRELATED",
                                 b.OTHER_COLOR,runtime,env)
        actors=[host,guest,unrelated]
        ids={}
        for app,proc in ((host_id,host),(guest_id,guest),
                         (other_id,unrelated)):
            n=b.wait_for(swaymsg,ipc,env,app,proc.pid)
            ids[app]=n["id"]
        hc,gc,uc=ids[host_id],ids[guest_id],ids[other_id]
        outer=(120,110,1000,700)
        slot=b.guest_from_chassis(outer)
        other=(1520,660,310,240)
        for app,rect in ((host_id,outer),(guest_id,slot),(other_id,other)):
            b.locate(swaymsg,ipc,env,ids[app],rect)
        for app,proc,rect in ((host_id,host,outer),(guest_id,guest,slot),
                              (other_id,unrelated,other)):
            b.wait_for(swaymsg,ipc,env,app,proc.pid,rect)

        command(swaymsg,ipc,env,"stage_director_social status",False)
        command(swaymsg,ipc,env,
                f"stage_director_social pair {hc} {uc} 110 1",False)
        command(swaymsg,ipc,env,
                f"stage_director_social pair {hc} {gc} 110 1")
        command(swaymsg,ipc,env,"stage_director_social status")
        print("SD_SOCIAL_PRODUCTION_NATIVE_PAIR accepted=1 "
              "exact_mapped_XDG=1 distinct_clients=1",flush=True)

        # Move HOST through the REAL production seatop, not two IPC moves.
        gesture(swaymsg,ipc,env,hc,host_id,host,"start-move" if False else "move",35,20)
        outer=(outer[0]+35,outer[1]+20,*outer[2:])
        slot=b.guest_from_chassis(outer)
        b.wait_for(swaymsg,ipc,env,host_id,host.pid,outer)
        b.wait_for(swaymsg,ipc,env,guest_id,guest.pid,slot)
        b.wait_for(swaymsg,ipc,env,other_id,unrelated.pid,other)
        b.capture(grim,runtime/"host-move.ppm",env,[
            ("rail",(outer[0]+25,outer[1]+140),b.HOST_COLOR),
            ("guest",(slot[0]+170,slot[1]+140),b.GUEST_COLOR),
            ("unrelated",(other[0]+130,other[1]+110),b.OTHER_COLOR),
            ("background",(25,25),(0,0,0)),
        ])
        print("SD_SOCIAL_PRODUCTION_NATIVE_HOST_MOVE "
              "geometry=PASS pixels=PASS",flush=True)

        gesture(swaymsg,ipc,env,gc,guest_id,guest,"move",-15,12)
        outer=(outer[0]-15,outer[1]+12,*outer[2:])
        slot=b.guest_from_chassis(outer)
        b.wait_for(swaymsg,ipc,env,host_id,host.pid,outer)
        b.wait_for(swaymsg,ipc,env,guest_id,guest.pid,slot)
        print("SD_SOCIAL_PRODUCTION_NATIVE_GUEST_MOVE geometry=PASS",
              flush=True)

        gesture(swaymsg,ipc,env,hc,host_id,host,"resize",45,25)
        outer=(outer[0],outer[1],outer[2]+45,outer[3]+25)
        slot=b.guest_from_chassis(outer)
        b.wait_for(swaymsg,ipc,env,host_id,host.pid,outer)
        b.wait_for(swaymsg,ipc,env,guest_id,guest.pid,slot)
        b.capture(grim,runtime/"host-resize.ppm",env,[
            ("rail",(outer[0]+25,outer[1]+140),b.HOST_COLOR),
            ("guest",(slot[0]+170,slot[1]+140),b.GUEST_COLOR),
            ("unrelated",(other[0]+130,other[1]+110),b.OTHER_COLOR),
            ("background",(25,25),(0,0,0)),
        ])
        print("SD_SOCIAL_PRODUCTION_NATIVE_HOST_RESIZE "
              "geometry=PASS pixels=PASS",flush=True)

        gesture(swaymsg,ipc,env,gc,guest_id,guest,"resize",-20,-15)
        outer=(outer[0],outer[1],outer[2]-20,outer[3]-15)
        slot=b.guest_from_chassis(outer)
        b.wait_for(swaymsg,ipc,env,host_id,host.pid,outer)
        b.wait_for(swaymsg,ipc,env,guest_id,guest.pid,slot)
        b.wait_for(swaymsg,ipc,env,other_id,unrelated.pid,other)
        print("SD_SOCIAL_PRODUCTION_NATIVE_GUEST_RESIZE geometry=PASS",
              flush=True)

        command(swaymsg,ipc,env,"stage_director_social clear")
        command(swaymsg,ipc,env,"stage_director_social status",False)
        gesture(swaymsg,ipc,env,hc,host_id,host,"move",20,5)
        outer=(outer[0]+20,outer[1]+5,*outer[2:])
        b.wait_for(swaymsg,ipc,env,host_id,host.pid,outer)
        b.wait_for(swaymsg,ipc,env,guest_id,guest.pid,slot)
        print("SD_SOCIAL_PRODUCTION_NATIVE_CLEAR "
              "reverts_to_independent_geometry=PASS",flush=True)

        # A gone XDG member loses identity; new same-app-ID process does
        # not inherit the pairing and cannot pass 'status'.
        b.quiet_stop(guest)
        b.wait_for(swaymsg,ipc,env,guest_id,guest.pid,present=False)
        replacement=b.foot_process(foot,guest_id,"REPLACEMENT MAIN",
                                   b.GUEST_COLOR,runtime,env)
        actors.append(replacement)
        new=b.wait_for(swaymsg,ipc,env,guest_id,replacement.pid)
        if new["id"]==gc:
            raise RuntimeError("SD_SOCIAL_PRODUCTION_STALE_ID_REUSED")
        command(swaymsg,ipc,env,"stage_director_social status",False)
        command(swaymsg,ipc,env,
                f"stage_director_social pair {hc} {gc} 110 1",False)
        b.locate(swaymsg,ipc,env,new["id"],b.guest_from_chassis(outer))
        b.wait_for(swaymsg,ipc,env,guest_id,replacement.pid,
                   b.guest_from_chassis(outer))
        command(swaymsg,ipc,env,
                f'stage_director_social pair {hc} {new["id"]} 110 1')
        command(swaymsg,ipc,env,"stage_director_social status")
        print("SD_SOCIAL_PRODUCTION_NATIVE_REPLACEMENT "
              "old_id=DENIED new_exact_pair=PASS",flush=True)
        log.flush()
        output_log=(runtime/"sway.log").read_text(
            encoding="utf-8",errors="replace")
        if output_log.count("[stage-director][social-native] paired") < 2:
            raise RuntimeError("SD_SOCIAL_PRODUCTION_MISSING_NATIVE_PAIR_LOG")
        print("SD_SOCIAL_PRODUCTION_NATIVE final=PASS "
              "actual_production_seatops=1 bi_directional_move_resize=1 "
              "real_XDG_pixels=PASS replacement_rejected=1 "
              "physical_mouse=NOT_TESTED live_desktop=UNCHANGED",
              flush=True)
    except Exception:
        log.flush()
        try:
            print((runtime/"sway.log").read_text(
                encoding="utf-8",errors="replace")[-8500:],file=sys.stderr)
        except OSError:
            pass
        raise
    finally:
        for proc in reversed(actors):
            b.quiet_stop(proc)
        if compositor is not None:
            b.quiet_stop(compositor)
        log.close()
        shutil.rmtree(runtime,ignore_errors=True)


if __name__=="__main__":
    main()
