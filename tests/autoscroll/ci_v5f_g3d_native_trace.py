#!/usr/bin/env python3
"""G3D real headless SwayPX native scene + Scroll Lock LED observation.

No mouse event, positive autoscroll, physical HID, axis or frame injection.
Only existing "seat seat0 cursor set" IPC, actual Foot XDG scene, and an
isolated Wayland virtual keyboard's real XKB Scroll Lock LED transition.
The XKB diagnostic NEVER becomes production authorization.
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

SAMPLE=re.compile(
    r"PA_G3D_NATIVE_SAMPLE scene=(\d+) direct=(\d+) layout_same=(\d+) "
    r"target_node=(\d+) target_epoch=(\d+) output_epoch=(\d+) "
    r"lock=(UNKNOWN|OFF|ON) lock_authority=DIAGNOSTIC_ONLY "
    r"layout_generation=UNVERIFIED physical_origin=UNKNOWN axis_frames=0")

def samples(log):
    text=log.read_text(encoding="utf-8",errors="replace")
    return [
        tuple(int(v) if i<6 else v for i,v in enumerate(groups))
        for groups in SAMPLE.findall(text)
    ]

def assert_cmd(swaymsg,ipc,env,command):
    out=c1.run_ipc(swaymsg,ipc,env,command)
    if not (isinstance(out,list) and out and out[0].get("success") is True):
        raise RuntimeError("PA_G3D_IPC_FAILED "+command+" "+repr(out))

def sample_at(swaymsg,ipc,env,log,x,y,scene,lock):
    old=len(samples(log))
    assert_cmd(swaymsg,ipc,env,f"seat seat0 cursor set {x} {y}")
    for _ in range(120):
        vals=samples(log)
        if len(vals)>old:
            record=vals[-1]
            expected_scene=int(scene)
            if record[0]!=expected_scene or record[6]!=lock:
                raise RuntimeError("PA_G3D_NATIVE_UNEXPECTED_SCENE_OR_LED "
                    +f"expected=({scene},{lock}) received={record}")
            if scene and not (record[1]==1 and record[2]==1 and
                    all(k>0 for k in record[3:6])):
                raise RuntimeError("PA_G3D_NATIVE_HIT_OR_LAYOUT_INCOMPLETE "
                                   +repr(record))
            if not scene and record[3]!=0:
                raise RuntimeError("PA_G3D_NATIVE_UNEXPECTED_APP_TARGET "
                                   +repr(record))
            return record
        time.sleep(0.04)
    raise RuntimeError("PA_G3D_REAL_NATIVE_CURSOR_SAMPLE_TIMEOUT")

def wait_client_line(proc,expected):
    readable,_,_=select.select([proc.stdout],[],[],15)
    if not readable:
        raise RuntimeError("PA_G3D_VIRTUAL_KEYBOARD_CLIENT_TIMEOUT "+expected)
    line=proc.stdout.readline().strip()
    if line!=expected:
        raise RuntimeError("PA_G3D_VIRTUAL_KEYBOARD_RECEIPT_MISMATCH "
                           +repr((expected,line)))

def command_client(proc,cmd,expected):
    if proc.poll() is not None:
        raise RuntimeError("PA_G3D_VIRTUAL_KEYBOARD_EXITED")
    proc.stdin.write(cmd+"\n");proc.stdin.flush()
    wait_client_line(proc,expected)

def require_ci():
    e=os.environ
    sway,swaymsg,foot,mesa=c1.require_disposable()
    kb=Path(e.get("PA_G3D_VK_CLIENT","/missing"))
    tmp=Path(e.get("RUNNER_TEMP","/missing"))
    if not (e.get("PA_G3D_NATIVE_SCENE_AUDIT")=="1"
            and str(kb).startswith(str(tmp)+"/")
            and kb.is_file() and os.access(kb,os.X_OK)):
        raise RuntimeError("BLOCKED_PA_G3D_NON_DISPOSABLE_KEYBOARD")
    return sway,swaymsg,foot,mesa,kb

def main():
    sway,swaymsg,foot,mesa,kb=require_ci()
    runtime=Path(tempfile.mkdtemp(
        prefix="pa-g3d-scene-native-",dir=os.environ["RUNNER_TEMP"]))
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
    for k in ("DISPLAY","WAYLAND_DISPLAY","WAYLAND_SOCKET",
              "SWAYSOCK","WLR_RENDERER","WLR_RENDER_DRM_DEVICE"):
        env.pop(k,None)
    logfile=runtime/"sway.log"
    opened=logfile.open("wb")
    comp=footproc=kbproc=None
    try:
        comp=subprocess.Popen([str(sway),"-d","-c",str(conf)],
            stdout=opened,stderr=subprocess.STDOUT,env=env)
        ipc=display=None
        for _ in range(220):
            if comp.poll() is not None:
                raise RuntimeError("PA_G3D_COMPOSITOR_PREMATURE_EXIT")
            sock=[x for x in runtime.glob("wayland-*") if x.is_socket()]
            ipcs=[x for x in runtime.glob("sway-ipc.*.sock") if x.is_socket()]
            if len(sock)==1 and len(ipcs)==1:
                display,ipc=sock[0],ipcs[0];break
            time.sleep(0.07)
        if ipc is None or display is None:
            raise RuntimeError("PA_G3D_COMPOSITOR_SOCKET_NOT_READY")
        env["WAYLAND_DISPLAY"]=display.name
        for _ in range(80):
            try:
                if isinstance(c1.run_ipc(swaymsg,ipc,env,"-t","get_tree"),dict):
                    break
            except (RuntimeError,subprocess.TimeoutExpired):
                pass
            time.sleep(0.07)
        else:
            raise RuntimeError("PA_G3D_COMPOSITOR_IPC_NOT_READY")

        # No keyboard at all is UNKNOWN, not an inferred Scroll Lock OFF.
        sample_at(swaymsg,ipc,env,logfile,2,2,False,"UNKNOWN")
        print("PA_G3D_REAL_NATIVE_NO_KEYBOARD_SCROLL_LOCK_UNKNOWN_FAIL_CLOSED_PASS",
              flush=True)

        footproc=c1.start_foot(foot,runtime,env,"PA-G3D-XDG-DIRECT-SCENE")
        nodeid=c1.wait_for_window(swaymsg,ipc,env,footproc,exists=True)
        if not isinstance(nodeid,int):
            raise RuntimeError("PA_G3D_REAL_XDG_NODE_ID_ABSENT")
        tree=c1.run_ipc(swaymsg,ipc,env,"-t","get_tree")
        matched=[x for x in c1.find_app(tree)
                 if x.get("pid")==footproc.pid and x.get("id")==nodeid]
        if len(matched)!=1:
            raise RuntimeError("PA_G3D_XDG_TREE_TARGET_AMBIGUOUS")
        rect=matched[0].get("rect",{})
        if not all(isinstance(rect.get(k),int) for k in ("x","y","width","height")):
            raise RuntimeError("PA_G3D_REAL_XDG_RECT_MISSING")
        cx=rect["x"]+rect["width"]//2
        cy=rect["y"]+rect["height"]//2
        if not(8<cx<1920 and 8<cy<1080 and rect["width"]>100
               and rect["height"]>100):
            raise RuntimeError("PA_G3D_XDG_RECT_OUTSIDE_HEADLESS_OUTPUT")
        scene=sample_at(swaymsg,ipc,env,logfile,cx,cy,True,"UNKNOWN")
        if scene[3]!=nodeid:
            raise RuntimeError("PA_G3D_REAL_NATIVE_HIT_NOT_THE_MAPPED_XDG "
                               +repr((nodeid,scene)))
        print("PA_G3D_REAL_NATIVE_DIRECT_XDG_AND_LAYOUT_HIT_PASS "
              "same_target=1 same_enabled_output=1 real_epoch_nonzero=1 "
              "scroll_lock=UNKNOWN",flush=True)

        # A real Wayland virtual-keyboard *protocol object* establishes
        # native XKB state. Still never a physical provenance witness.
        kbproc=subprocess.Popen([str(kb)],env=env,stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,stderr=subprocess.PIPE,
            text=True,bufsize=1)
        wait_client_line(kbproc,
            "PA_G3D_VIRTUAL_KEYBOARD_READY physical_eligible=0 scroll_led=OFF")
        sample_at(swaymsg,ipc,env,logfile,cx,cy,True,"OFF")
        print("PA_G3D_REAL_NATIVE_XKB_SCROLL_LOCK_OFF_OBSERVED_PASS "
              "virtual_keyboard_not_physical=1",flush=True)

        command_client(kbproc,"toggle",
                       "PA_G3D_VIRTUAL_KEYBOARD_TOGGLED expected=ON")
        sample_at(swaymsg,ipc,env,logfile,cx,cy,True,"ON")
        print("PA_G3D_REAL_NATIVE_XKB_SCROLL_LOCK_ON_DENIAL_WITNESS_PASS "
              "actual_compositor_LED=ON axis_frames=0",flush=True)

        command_client(kbproc,"toggle",
                       "PA_G3D_VIRTUAL_KEYBOARD_TOGGLED expected=OFF")
        sample_at(swaymsg,ipc,env,logfile,cx,cy,True,"OFF")
        print("PA_G3D_REAL_NATIVE_XKB_SCROLL_LOCK_OFF_AFTER_TOGGLE_PASS "
              "actual_compositor_LED=OFF no_authorization=1",flush=True)

        sample_at(swaymsg,ipc,env,logfile,2,2,False,"OFF")
        print("PA_G3D_REAL_NATIVE_SCENE_MISS_DENIED_PASS "
              "scroll_led_known_off=1 no_target=1",flush=True)
        c1.stop(footproc)
        c1.wait_for_window(swaymsg,ipc,env,footproc,exists=False)
        footproc=None
        sample_at(swaymsg,ipc,env,logfile,cx,cy,False,"OFF")
        print("PA_G3D_REAL_NATIVE_XDG_UNMAP_SCENE_MISS_PASS "
              "no_stale_target=1",flush=True)

        command_client(kbproc,"stop",
                       "PA_G3D_VIRTUAL_KEYBOARD_DONE no_pointer_events=1")
        if kbproc.wait(timeout=12)!=0:
            raise RuntimeError("PA_G3D_VIRTUAL_KEYBOARD_EXITED_ERROR")
        kbproc=None
        sample_at(swaymsg,ipc,env,logfile,2,2,False,"UNKNOWN")
        print("PA_G3D_REAL_NATIVE_KEYBOARD_REMOVE_TO_UNKNOWN_PASS "
              "no_keymap_assumed_unlocked=1",flush=True)

        raw=logfile.read_text(encoding="utf-8",errors="replace")
        if "PA_G3D_NATIVE_SAMPLE scene=1" not in raw:
            raise RuntimeError("PA_G3D_REAL_DIRECT_SCENE_NOT_LOGGED")
        if len(samples(logfile))<8:
            raise RuntimeError("PA_G3D_REAL_NATIVE_SCENE_OBSERVATION_COUNT_SHORT")
        print("PA_G3D_REAL_SCENE_AND_XKB_LED_NEGATIVE_EVIDENCE_PASS "
              "direct_xdg_hit=1 layout_output_match=1 "
              "scroll_lock_unknown_off_on_off_unknown=1 "
              "virtual_keyboard_untrusted=1 physical_origin=UNKNOWN "
              "layout_generation=UNVERIFIED production_axis_frames=ZERO",
              flush=True)
    except Exception:
        print("PA_G3D_PRIVATE_COMPOSITOR_LOG_TAIL",flush=True)
        print("\n".join(logfile.read_text(encoding="utf-8",errors="replace")
                        .splitlines()[-120:]),flush=True)
        if kbproc is not None:
            print("PA_G3D_VIRTUAL_KEYBOARD_RETURN",kbproc.poll(),flush=True)
        raise
    finally:
        c1.stop(footproc)
        c1.stop(kbproc)
        c1.stop(comp)
        opened.close()
        shutil.rmtree(runtime,ignore_errors=True)

if __name__=="__main__":
    main()
