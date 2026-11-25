#!/usr/bin/env python3
"""G3M: observe live second-keyboard hotplug/unplug across G3L native scene.
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

G3M=re.compile(
    r"PA_G3M_NATIVE_MULTIKEYBOARD phase=(\d+) "
    r"keyboards=(\d+) primary=(\d+) second=(\d+) unexpected=(\d+) "
    r"groups=(\d+) primary_led=(-?\d+) second_led=(-?\d+) "
    r"known_test_topology=(\d+) leds_observed=(\d+) "
    r"global_lock_clear_authorized=(\d+) "
    r"lock_authority=UNKNOWN_DENY origin=SOFTWARE_DENIED "
    r"can_consume=0 can_axis=0")
SECONDKEY=re.compile(
    r"PA_G3M_SECOND_NATIVE_SCROLLLOCK "
    r"pressed=(\d+) keycode=(\d+) xkb_led=(-?\d+) "
    r"origin=SOFTWARE_DENIED lock_authority=UNKNOWN_DENY can_axis=0")
EVENT=re.compile(
    r"PA_G3J_REAL_COMPOSITOR_LIBINPUT_MIDDLE "
    r"pressed=(\d+) native_libinput=1 real_handle=1 "
    r"kernel_event=(\d+) kernel_virtual=(\d+) registered=(\d+) "
    r"device_epoch=(\d+) seat_epoch=(\d+) "
    r"owner_snapshot=(\d+) target_epoch=(\d+) output_epoch=(\d+) "
    r"os_source_candidate=(\d+) weather=(\d+) company=(\d+) "
    r"time_nonzero=(\d+) physical_attestation=NONE "
    r"can_consume=0 can_axis=0")
SCROLLKEY=re.compile(
    r"PA_G3L_REAL_COMPOSITOR_SCROLLLOCK_KEY "
    r"pressed=(\d+) code=(\d+) xkb_led=(-?\d+) "
    r"kernel_source=SYNTHETIC native_keyboard=1 "
    r"native_scroll_policy=UNKNOWN_DENY can_axis=0")
G3L=re.compile(
    r"PA_G3L_NATIVE_XKB_LAYOUT phase=(\d+) "
    r"keyboard_count=(\d+) xkb_scroll_led=(-?\d+) "
    r"layout_x=(-?\d+) layout_y=(-?\d+) "
    r"layout_w=(\d+) layout_h=(\d+) "
    r"source_candidate=(\d+) native_scroll_policy=UNKNOWN_DENY "
    r"layout_generation=UNATTESTED physical_hid=NOT_ATTESTED "
    r"can_consume=0 can_axis=0")
G3K=re.compile(
    r"PA_G3K_NATIVE_GUARD_NEGATIVE phase=(\d+) "
    r"native_libinput=1 owner=(\d+) "
    r"target_epoch=(\d+) output_epoch=(\d+) "
    r"source_candidate=(\d+) began=(\d+) ticked=(\d+) active=(\d+) "
    r"layout_witness=UNKNOWN_DENY scroll_lock=UNKNOWN_DENY "
    r"physical_origin=UNKNOWN_DENY can_consume=0 can_axis=0")
G3H=re.compile(
    r"PA_G3H_NATIVE_OS_DIAG phase=(\d+) virtual=(\d+) "
    r"source_candidate=(\d+) os_diag=(\d+) owner_snapshot=(\d+) "
    r"device_epoch=(\d+) seat_epoch=(\d+) stage_refusal_clear=(\d+) "
    r"os_mode=TEST_ONLY can_consume=(\d+) can_axis=(\d+) "
    r"production=HARD_OFF")

def guard():
    tmp,ws=g2.guard_ci()
    e=os.environ
    if not(e.get("PA_G3M_NATIVE_OBSERVE")=="1" and
           e.get("PA_G3M_KERNEL_ACTOR")=="1" and
           e.get("PA_G3L_NATIVE_OBSERVE")=="1" and
           e.get("PA_G3K_NATIVE_GUARD")=="1" and
           e.get("PA_G3K_KERNEL_ACTOR")=="1" and
           e.get("RUNNER_ENVIRONMENT")=="github-hosted" and
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
    if os.environ.get("PA_G4E6_RUNTIME")!="1" or os.environ.get("PA_G4E5_PAIR")!="1" or os.environ.get("PA_G4E4_MOTION")!="1" or os.environ.get("PA_G4E3_RECEIPTS")!="1" or os.environ.get("PA_G4E1_SCENE")!="1" or os.environ.get("PA_G4D3_KEYS")!="1" or os.environ.get("PA_G4D2_KEYS")!="1" or os.environ.get("PA_G4C2_NATIVE")!="1" or os.environ.get("PA_G3T_SHADOW")!="1" or os.environ.get("PA_G3S_LOCK")!="1" or os.environ.get("PA_G3R_SEAT")!="1" or os.environ.get("PA_G3Q_GROUP")!="1" or os.environ.get("PA_G3P_POST_ABA")!="1" or os.environ.get("PA_G3O_NATIVE_EPOCH")!="1":
        raise RuntimeError("BLOCKED_G3O_TRACE")
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
            "PA_G3J_KERNEL_ACTOR","PA_G3K_KERNEL_ACTOR",
             "PA_G3M_KERNEL_ACTOR")}
        actor=subprocess.Popen(["sudo","-n","env"]+
            [f"{k}={v}" for k,v in root_env.items()]+[str(actor_bin)],
            stdin=subprocess.PIPE,stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,text=True,bufsize=1)
        hello=expect_prefix(actor,"PA_G3K_KERNEL_UINPUT_READY input=")
        match=re.fullmatch(
            r"PA_G3K_KERNEL_UINPUT_READY input=(input[0-9]+) "
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
        # Require the SECOND real kernel uinput device as an actual
        # compositor keyboard, not inferred from an XKB debug string.
        if not any(isinstance(i,dict) and
                   i.get("name")=="PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE" and
                   i.get("type")=="keyboard" for i in inputs):
            for _ in range(100):
                inputs=c1.run_ipc(swaymsg,ipc,env,"-t","get_inputs")
                if any(isinstance(i,dict) and
                       i.get("name")=="PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE" and
                       i.get("type")=="keyboard" for i in inputs):
                    break
                time.sleep(.05)
            else:
                raise RuntimeError("PA_G3L_NATIVE_KEYBOARD_NOT_REGISTERED")
        print("PA_G3L_REAL_NATIVE_KEYBOARD_REGISTERED_PASS",flush=True)


        client_log=runtime/"g4e3-foot-wayland.log"
        client_env=dict(env,WAYLAND_DEBUG="client")
        with client_log.open("wb") as client_out:
            window=subprocess.Popen([str(foot),f"--app-id={c1.APP}",
                "--title=PA-G3J-REAL-LIBINPUT-XDG","--window-size-pixels=620x450",
                "/usr/bin/sleep","55"],stdout=subprocess.DEVNULL,stderr=client_out,env=client_env)
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

        button_re=re.compile(r"PA_G4E6_BUTTON n=(\d+) press=(\d+) consumed=(\d+) active=(\d+) lock=(\d+) debt=(\d+) event=(\d+) read=(\d+) timer=(\d+)")
        key_re=re.compile(r"PA_G4E6_KEY n=(\d+) code=(\d+) press=(\d+) active=(\d+) lock=(\d+) timer=(\d+) debt=(\d+)")
        batch_re=re.compile(r"PA_G4E6_BATCH time=(\d+) target=(\d+) read=(\d+) event=(\d+) vertical=([0-9.]+) horizontal=([0-9.]+)")
        client_re=re.compile(r"wl_pointer(?:@|#)(\d+)\.button\(\s*\d+,\s*\d+,\s*274,\s*(\d+)\)")
        axis_re=re.compile(r"wl_pointer(?:@|#)(\d+)\.axis\(\s*(\d+),\s*(\d+),\s*(-?[0-9.]+)\)")
        frame_re=re.compile(r"wl_pointer(?:@|#)(\d+)\.frame\(\)")
        source_re=re.compile(r"wl_pointer(?:@|#)(\d+)\.axis_source\(\s*(\d+)\)")
        def receipts():
            pending={};sources={};frames=[]
            for line in client_log.read_text(errors="replace").splitlines():
                a=axis_re.search(line);z=source_re.search(line);fr=frame_re.search(line)
                if z:sources[z[1]]=int(z[2])
                if a:pending.setdefault(a[1],[]).append((int(a[2]),int(a[3]),float(a[4])))
                if fr:
                    vals=pending.pop(fr[1],[]);source=sources.pop(fr[1],None)
                    if vals:frames.append((fr[1],source,vals))
            return frames,pending
        def current():
            raw=log.read_text(errors="replace")
            if "PA_G4E6_FAILED" in raw:raise RuntimeError("G4E6_NATIVE_FAILURE "+raw[-5000:])
            return ([tuple(map(int,r)) for r in button_re.findall(raw)],
                    [tuple(map(int,r)) for r in key_re.findall(raw)],batch_re.findall(raw))
        def command(name,nb,nk):
            actor.stdin.write(name+"\n");actor.stdin.flush()
            expect_prefix(actor,"PA_G4E5_ACTOR_COMMAND_PASS")
            for _ in range(120):
                b,k,axes=current()
                if len(b)==nb and len(k)==nk:return b,k,axes
                time.sleep(.025)
            raise RuntimeError("G4E6_CALLBACK_TIMEOUT "+repr(current()))
        command("down",1,0);command("up",2,0)
        command("lock",2,2)
        b,k,axes=command("down",3,2)
        if b[-1][2:5]!=(1,1,2) or not b[-1][5] or b[-1][8]!=1 or axes:
            raise RuntimeError("G4E6_START "+repr((b,k,axes)))
        command("move",3,2)
        for _ in range(120):
            if len(current()[2])>=8 and len(receipts()[0])>=8:break
            time.sleep(.025)
        else:raise RuntimeError("G4E6_SCROLL_TIMEOUT "+repr(current()))
        b,k,axes=command("escape",3,4)
        if k[2][3:6]!=(0,2,0) or k[2][6]!=b[-1][5]:
            raise RuntimeError("G4E6_ESCAPE_HELD "+repr((b,k)))
        count=len(axes)
        time.sleep(.3)
        if count<8 or len(current()[2])!=count or len(receipts()[0])!=count:
            raise RuntimeError("G4E6_EXTRA_AFTER_ESCAPE")
        command("up",4,4)
        command("down",5,4)
        b,k,axes=command("lock",5,6)
        if k[4][3:6]!=(0,1,0) or k[4][6]!=b[-1][5]:
            raise RuntimeError("G4E6_DISABLE_HELD "+repr((b,k)))
        command("up",6,6)
        command("down",7,6)
        b,k,axes=command("up",8,6)
        if [r[1] for r in b]!=[1,0]*4 or [r[2] for r in b]!=[0,0,1,1,1,1,0,0] or \
                [r[3] for r in b]!=[0,0,1,0,1,0,0,0] or \
                [r[4] for r in b]!=[1,1,2,2,2,1,1,1] or \
                [r[8] for r in b]!=[0,0,1,0,1,0,0,0] or \
                any(b[i][5] for i in (0,1,3,5,6,7)):
            raise RuntimeError("G4E6_BUTTON_PAIRING "+repr(b))
        frames,pending=receipts()
        if len(axes)!=count or len(frames)!=count or pending:raise RuntimeError("G4E6_RECEIPT_COUNT")
        for batch,frame in zip(axes,frames):
            stamp,target,read,event=map(int,batch[:4])
            vertical,horizontal=map(float,batch[4:])
            _,source,values=frame
            if not target or source!=2 or len(values)!=2 or \
                    [a[:2] for a in values]!=[(stamp,0),(stamp,1)] or \
                    abs(values[0][2]-vertical)>0.01 or abs(values[1][2]-horizontal)>0.01:
                raise RuntimeError("G4E6_RECEIPT_MISMATCH "+repr((batch,frame)))
        if len({r[3] for r in axes})!=1 or any(int(q[2])<=int(p[2]) for p,q in zip(axes,axes[1:])):
            raise RuntimeError("G4E6_SHARED_SEQUENCE")
        for _ in range(120):
            client=client_re.findall(client_log.read_text(errors="replace"))
            if len(client)>=4:break
            time.sleep(.025)
        if len(client)!=4 or [int(r[1]) for r in client]!=[1,0,1,0]:
            raise RuntimeError("G4E6_CLIENT_BUTTONS "+repr(client))
        raw=log.read_text(errors="replace")
        if len(re.findall(r"PA_G4E6_MOTION n=\d+ consumed=1",raw))!=1:
            raise RuntimeError("G4E6_MOTION_CALLBACK")
        if any(raw.count(f"PA_G2_NATIVE_FROM_SEATOP state={phase}")!=2 for phase in (0,1)):
            raise RuntimeError("G4E6_ORDINARY_DISPATCH")
        print(f"PA_G4E6_SEAT_RUNTIME_PASS frames={count} axes={count*2} "
              "native_middle=8 consumed=4 ordinary_buttons=4 escape_held=1 lock_held=1 "
              "timer_removed=2 extra_axes=0 source_grant=CI_ONLY",flush=True)
        actor.stdin.write("stop\n")
        actor.stdin.flush()
        end,err=actor.communicate(timeout=12)
        if actor.returncode!=0 or "PA_G3K_KERNEL_UINPUT_DESTROYED_PASS" not in end:
            raise RuntimeError("G4E6_CLEANUP "+repr((end,err)))
        actor=None
        for _ in range(100):
            active=c1.run_ipc(swaymsg,ipc,env,"-t","get_inputs")
            if not any(isinstance(i,dict) and i.get("name") in (
                "PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL","PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE")
                for i in active):break
            time.sleep(.05)
        else:
            raise RuntimeError("G4E6_DEVICE_CLEANUP")
        print("PA_G4E6_NATIVE_DEVICES_REMOVED_PASS",flush=True)
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
