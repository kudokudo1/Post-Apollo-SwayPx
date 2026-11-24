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
    if os.environ.get("PA_G3R_SEAT")!="1" or os.environ.get("PA_G3Q_GROUP")!="1" or os.environ.get("PA_G3P_POST_ABA")!="1" or os.environ.get("PA_G3O_NATIVE_EPOCH")!="1":
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
        sent=expect_prefix(actor,"PA_G3K_KERNEL_MIDDLE_SENT ")
        if sent!="PA_G3K_KERNEL_MIDDLE_SENT round=1 pressed=1 released=1 axes=0":
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

        # First half has a real mapped direct XDG target. Remove that
        # specific process before the second native input pair. This is
        # actual compositor state, not a fabricated owner-revoked flag.
        first_owner_pid=window.pid
        c1.stop(window)
        window=None
        for _ in range(120):
            tree=c1.run_ipc(swaymsg,ipc,env,"-t","get_tree")
            if not any(i.get("id")==nodeid or i.get("pid")==first_owner_pid
                       for i in c1.find_app(tree)):
                break
            time.sleep(.05)
        else:
            raise RuntimeError("PA_G3K_NATIVE_XDG_TARGET_STILL_LIVE")
        print("PA_G3K_NATIVE_XDG_TARGET_UNMAPPED_PASS",flush=True)
        actor.stdin.write("click\n")
        actor.stdin.flush()
        sent=expect_prefix(actor,"PA_G3K_KERNEL_MIDDLE_SENT ")
        if sent!="PA_G3K_KERNEL_MIDDLE_SENT round=2 pressed=1 released=1 axes=0":
            raise RuntimeError("PA_G3K_SECOND_NATIVE_ACTOR_RESULT_INCONSISTENT")
        deadline=time.monotonic()+15
        while True:
            raw=log.read_text(errors="replace")
            guards=[tuple(map(int,x)) for x in G3K.findall(raw)]
            if len(guards)>=4 and raw.count(
                    "PA_G2_NATIVE_FROM_SEATOP state=0")>=2:
                break
            if time.monotonic()>deadline:
                raise RuntimeError("PA_G3K_NATIVE_SECOND_PAIR_NOT_OBSERVED "+
                                   repr(guards))
            time.sleep(.05)
        if len(guards)!=4 or [x[0] for x in guards]!=[1,0,1,0]:
            raise RuntimeError("PA_G3K_NATIVE_GUARD_COUNT_OR_PHASE_INVALID "+
                               repr(guards))
        first=guards[:2]
        after=guards[2:]
        if not all(x[1]==1 and x[2]>0 and x[3]>0 for x in first):
            raise RuntimeError("PA_G3K_FIRST_NATIVE_OWNER_NOT_LIVE "+
                               repr(first))
        if not all(x[1]==0 and x[2]==0 and x[3]==0 for x in after):
            raise RuntimeError("PA_G3K_REVOKED_NATIVE_TARGET_MISTAKEN_LIVE "+
                               repr(after))
        if any(any(x[i]!=0 for i in (4,5,6,7)) for x in guards):
            raise RuntimeError("PA_G3K_NATIVE_UNKNOWN_LOCK_OR_LAYOUT_PROMOTED "+
                               repr(guards))
        if any(raw.count(f"PA_G2_NATIVE_FROM_SEATOP state={phase}")!=2
               for phase in (1,0)):
            raise RuntimeError("PA_G3K_NORMAL_INPUT_DISPATCH_LOST_AFTER_UNMAP")
        if len(EVENT.findall(raw))!=4:
            raise RuntimeError("PA_G3K_ACTUAL_LIBINPUT_CALLBACK_MISSING")
        print("PA_G3K_NATIVE_OWNER_TARGET_REVOKED_GUARD_DENIED_PASS "
              "libinput_middle_callbacks=4 direct_XDG_first=1 "
              "owner_after_unmap=0 guard_begin_allowed=0 "
              "guard_tick_allowed=0 native_scroll_lock=UNKNOWN_DENY "
              "layout_generation=UNKNOWN_DENY "
              "normal_middle_dispatch=4 axis_frames=0",flush=True)

        # Change keyboard XKB LED with a REAL separate kernel uinput
        # keyboard. This is a diagnostic, NOT trusted multi-keyboard
        # Scroll Lock policy or the physical origin of any key.
        # This really creates another Linux kernel /dev/uinput keyboard.
        # Observe SwayPX get_inputs before sending ANY further mouse event.
        actor.stdin.write("plug\n")
        actor.stdin.flush()
        if expect_prefix(actor,"PA_G3M_SECOND_KEYBOARD_PLUGGED_PASS") != \
                "PA_G3M_SECOND_KEYBOARD_PLUGGED_PASS":
            raise RuntimeError("PA_G3M_SECOND_KERNEL_CREATE_NOT_PROVEN")
        for _ in range(120):
            inputs=c1.run_ipc(swaymsg,ipc,env,"-t","get_inputs")
            second=[i for i in inputs if isinstance(i,dict) and
                    i.get("name")=="PA-G3M-SECOND-SYNTHETIC-KEYBOARD" and
                    i.get("type")=="keyboard"]
            if len(second)==1:
                break
            time.sleep(.05)
        else:
            raise RuntimeError("PA_G3M_SECOND_NATIVE_KEYBOARD_NOT_REGISTERED "+
                               repr(inputs))
        print("PA_G3M_SECOND_NATIVE_INPUT_REGISTERED_PASS "
              "source=KERNEL_SYNTHETIC independent_device=1",flush=True)

        actor.stdin.write("second_key\n")
        actor.stdin.flush()
        if expect_prefix(actor,"PA_G3M_SECOND_KERNEL_SCROLLLOCK_SENT_PASS") != \
                "PA_G3M_SECOND_KERNEL_SCROLLLOCK_SENT_PASS":
            raise RuntimeError("PA_G3M_SECOND_KERNEL_KEY_NOT_EMITTED")
        actor.stdin.write("lock\n")
        actor.stdin.flush()
        toggled=expect_prefix(actor,
            "PA_G3L_REAL_KERNEL_SCROLL_LOCK_KEY_TOGGLED_PASS ")
        if toggled != ("PA_G3L_REAL_KERNEL_SCROLL_LOCK_KEY_TOGGLED_PASS "
                       "press=1 release=1 axis=0"):
            raise RuntimeError("PA_G3L_REAL_SCROLLLOCK_TOGGLE_NOT_SENT")
        actor.stdin.write("click\n")
        actor.stdin.flush()
        third=expect_prefix(actor,"PA_G3K_KERNEL_MIDDLE_SENT ")
        if third!="PA_G3K_KERNEL_MIDDLE_SENT round=3 pressed=1 released=1 axes=0":
            raise RuntimeError("PA_G3L_THIRD_MOUSE_PAIR_UNCONFIRMED")
        limit=time.monotonic()+15
        while True:
            raw=log.read_text(errors="replace")
            leds=[tuple(map(int,v)) for v in G3L.findall(raw)]
            if len(leds)>=6:
                break
            if time.monotonic()>limit:
                raise RuntimeError("PA_G3L_REAL_XKB_POST_KEY_CALLBACK_MISSING "+
                                   repr(leds))
            time.sleep(.05)
        if len(leds)!=6 or [v[0] for v in leds]!=[1,0]*3:
            raise RuntimeError("PA_G3L_REAL_CALLBACK_SEQUENCE_INVALID "+repr(leds))
        if not all(v[1]==1 and v[2]==0 and v[7]==0 and
                   v[5]>0 and v[6]>0 for v in leds[:4]):
            raise RuntimeError("PA_G3L_INITIAL_REAL_XKB_LED_NOT_OFF "+repr(leds[:4]))
        # The compositor must actually RECEIVE the key events. The XKB
        # LED may remain off under its default keymap; OFF is not proof of
        # globally unlocked Scroll Lock. Report that as UNKNOWN/DENY.
        raw=log.read_text(errors="replace")
        scrollkeys=[tuple(map(int,v)) for v in SCROLLKEY.findall(raw)]
        if len(scrollkeys)!=2 or [v[0] for v in scrollkeys]!=[1,0] or \
           any(v[1]!=70 or v[2] not in (0,1) for v in scrollkeys):
            raise RuntimeError("PA_G3L_NATIVE_SCROLLLOCK_KEY_CALLBACK_ABSENT "+
                               repr(scrollkeys))
        if not all(v[1]==1 and v[2] in (0,1) and v[7]==0
                   for v in leds[4:]):
            raise RuntimeError("PA_G3L_NATIVE_XKB_LED_UNOBSERVABLE "+
                               repr(leds[4:]))
        if leds[4][2]!=leds[5][2]:
            raise RuntimeError("PA_G3L_NATIVE_XKB_LED_UNSTABLE "+
                               repr(leds[4:]))
        geometry=(leds[0][3],leds[0][4],leds[0][5],leds[0][6])
        if any((v[3],v[4],v[5],v[6])!=geometry for v in leds):
            raise RuntimeError("PA_G3L_LAYOUT_CHANGED_WITHOUT_REQUEST "+repr(leds))
        print("PA_G3L_NATIVE_SCROLL_LOCK_OBSERVED_UNTRUSTED_PASS "
              "real_kernel_key_callbacks=2 "
              "xkb_led_before=0 xkb_led_after="+str(leds[4][2])+
              " global_scroll_lock_policy=UNKNOWN_DENY "
              "no_unverified_lock_clearance=1 source=DENIED",flush=True)

        # Now mutate REAL compositor output state through its own IPC
        # command. Compare live output rect, not a guessed monotonic epoch.
        outputs=c1.run_ipc(swaymsg,ipc,env,"-t","get_outputs")
        enabled=[o for o in outputs if isinstance(o,dict) and
                 o.get("active") is True and isinstance(o.get("rect"),dict)]
        if len(enabled)!=1 or not enabled[0].get("name"):
            raise RuntimeError("PA_G3L_EXACT_ONE_OUTPUT_NOT_CONFIRMED "+repr(outputs))
        output=enabled[0]["name"]
        old_x=enabled[0]["rect"].get("x")
        if not isinstance(old_x,int):
            raise RuntimeError("PA_G3L_OLD_OUTPUT_RECT_UNKNOWN")
        new_x=137 if old_x!=137 else 173
        result=c1.run_ipc(swaymsg,ipc,env,
                          f"output {output} pos {new_x} 0")
        if not (isinstance(result,list) and len(result)==1 and
                result[0].get("success") is True):
            raise RuntimeError("PA_G3L_OUTPUT_POSITION_COMMIT_REFUSED "+
                               repr(result))
        for _ in range(100):
            changed=c1.run_ipc(swaymsg,ipc,env,"-t","get_outputs")
            matched=[o for o in changed if isinstance(o,dict) and
                     o.get("name")==output and
                     isinstance(o.get("rect"),dict)]
            if len(matched)==1 and matched[0]["rect"].get("x")==new_x:
                break
            time.sleep(.05)
        else:
            raise RuntimeError("PA_G3L_NATIVE_OUTPUT_LAYOUT_DID_NOT_MOVE "+
                               repr(changed))
        # Real hot-unplug, validated from the compositor's native input
        # manager, BEFORE the final middle-button callbacks.
        actor.stdin.write("unplug\n")
        actor.stdin.flush()
        if expect_prefix(actor,"PA_G3M_SECOND_KEYBOARD_UNPLUGGED_PASS") != \
                "PA_G3M_SECOND_KEYBOARD_UNPLUGGED_PASS":
            raise RuntimeError("PA_G3M_SECOND_KERNEL_REMOVAL_NOT_PROVEN")
        for _ in range(120):
            inputs=c1.run_ipc(swaymsg,ipc,env,"-t","get_inputs")
            if not any(isinstance(i,dict) and
                       i.get("name")=="PA-G3M-SECOND-SYNTHETIC-KEYBOARD"
                       for i in inputs):
                break
            time.sleep(.05)
        else:
            raise RuntimeError("PA_G3M_SECOND_NATIVE_INPUT_STILL_PRESENT")
        print("PA_G3M_SECOND_NATIVE_INPUT_REMOVED_PASS "
              "origin=KERNEL_SYNTHETIC stale_keyboard_not_reused=1",flush=True)

        actor.stdin.write("click\n")
        actor.stdin.flush()
        fourth=expect_prefix(actor,"PA_G3K_KERNEL_MIDDLE_SENT ")
        if fourth!="PA_G3K_KERNEL_MIDDLE_SENT round=4 pressed=1 released=1 axes=0":
            raise RuntimeError("PA_G3L_FOURTH_MOUSE_PAIR_UNCONFIRMED")
        limit=time.monotonic()+15
        while True:
            raw=log.read_text(errors="replace")
            leds=[tuple(map(int,v)) for v in G3L.findall(raw)]
            if len(leds)>=8 and all(
                    raw.count(f"PA_G2_NATIVE_FROM_SEATOP state={phase}")>=4
                    for phase in (0,1)):
                break
            if time.monotonic()>limit:
                raise RuntimeError("PA_G3L_POST_LAYOUT_NATIVE_CALLBACK_MISSING "+
                                   repr(leds))
            time.sleep(.05)
        if len(leds)!=8 or [v[0] for v in leds]!=[1,0]*4:
            raise RuntimeError("PA_G3L_EXTRA_OR_MISSING_NATIVE_EVENTS "+repr(leds))
        if not all(v[1]==1 and v[2]==leds[4][2] and v[3]==new_x and
                   v[4]==0 and v[5]==geometry[2] and
                   v[6]==geometry[3] and v[7]==0 for v in leds[6:]):
            raise RuntimeError("PA_G3L_NATIVE_LAYOUT_NOT_CURRENT "+repr(leds[6:]))
        if len(G3K.findall(raw))!=8 or len(EVENT.findall(raw))!=8:
            raise RuntimeError("PA_G3L_NATIVE_GUARD_OR_LIBINPUT_LOST")
        guards=[tuple(map(int,v)) for v in G3K.findall(raw)]
        if any(any(g[i] for i in (4,5,6,7)) for g in guards):
            raise RuntimeError("PA_G3L_NATIVE_GUARD_PROMOTED")
        if any(raw.count(f"PA_G2_NATIVE_FROM_SEATOP state={p}")!=4
               for p in (0,1)):
            raise RuntimeError("PA_G3L_NORMAL_MIDDLE_DISPATCH_CHANGED")
        print("PA_G3L_REAL_NATIVE_OUTPUT_LAYOUT_CHANGED_GUARD_DENIED_PASS "
              "layout_moved=1 before_x="+str(old_x)+" after_x="+str(new_x)+
              " native_middle_callbacks=8 normal_dispatch=8 axis_frames=0 "
              "layout_generation=UNATTESTED",flush=True)

        # 8 real cursor events span one keyboard, two keyboards and
        # one again. Presence/absence must be directly observed on the seat.
        native=[tuple(map(int,x)) for x in G3M.findall(raw)]
        if len(native)!=8 or [v[0] for v in native]!=[1,0]*4:
            raise RuntimeError("PA_G3M_NATIVE_MIDDLE_EVENT_MISSING "+
                               repr(native))
        # Observe the real seat baseline, not an invented single-keyboard
        # world. SwayPX may own a preexisting keyboard group or native
        # device. Those remain untrusted and must never be silently omitted.
        baseline=native[0][1]
        other=native[0][4]
        groups=native[0][5]
        expected=[baseline]*4+[baseline+1]*2+[baseline]*2
        if baseline<1 or [v[1] for v in native]!=expected or \
           [v[2] for v in native]!=[1]*8 or \
           [v[3] for v in native]!=[0,0,0,0,1,1,0,0] or \
           any(v[4]!=other or v[5]!=groups for v in native):
            raise RuntimeError("PA_G3M_ACTUAL_SEAT_KEYBOARD_HOTPLUG_WRONG "+
                               repr(native))
        if any(v[6] not in (0,1) or
               (v[3]==0 and v[7]!=-1) or
               (v[3]==1 and v[7] not in (0,1)) or
               v[10]!=0 for v in native):
            raise RuntimeError("PA_G3M_UNKNOWN_CONFLICT_GRANTED_CLEARANCE "+
                               repr(native))
        if any(v[8]!=0 for v in native) and (other or groups):
            raise RuntimeError("PA_G3M_UNRECOGNIZED_GROUP_MISTAKEN_CLEAN "+
                               repr(native))
        secondkeys=[tuple(map(int,x)) for x in SECONDKEY.findall(raw)]
        if len(secondkeys)!=2 or [v[0] for v in secondkeys]!=[1,0] or \
           any(v[1]!=70 or v[2] not in (0,1) for v in secondkeys):
            raise RuntimeError("PA_G3M_SECOND_NATIVE_KEY_EVENTS_MISSING "+
                               repr(secondkeys))
        if len(G3K.findall(raw))!=8 or len(EVENT.findall(raw))!=8 or \
           any(raw.count(f"PA_G2_NATIVE_FROM_SEATOP state={p}")!=4
               for p in (0,1)):
            raise RuntimeError("PA_G3M_NATIVE_INPUT_PATH_CHANGED")
        if any(any(v[i]!=0 for i in (4,5,6,7))
               for v in (tuple(map(int,x)) for x in G3K.findall(raw))):
            raise RuntimeError("PA_G3M_GUARD_BECAME_ADMITTING")
        print("PA_G3M_REAL_NATIVE_MULTIKEYBOARD_HOTPLUG_DENIED_PASS "
              "keyboard_baseline="+str(baseline)+
              " keyboard_with_second="+str(baseline+1)+
              " unexpected_baseline="+str(other)+
              " keyboard_groups="+str(groups)+
              " second_scroll_lock_callbacks=2 "
              "native_middle_callbacks=8 normal_dispatch=8 axis_frames=0 "
              "global_lock_clearance=DENIED",flush=True)


        # G3O: real callback serials, not counts sampled by IPC.
        middle_re = re.compile(
            r"PA_G3O_MIDDLE pressed=(\d+) serial=(\d+) initial=(\d+) "
            r"stale=(\d+) can_consume=0 can_axis=0")
        event_re = re.compile(
            r"PA_G3O_EVENT kind=(\w+) serial=(\d+) poisoned=(\d+) "
            r"stale=(\d+) can_axis=0")
        raw = log.read_text(errors="replace")
        clicks = [tuple(map(int,m)) for m in middle_re.findall(raw)]
        if len(clicks) != 8 or [m[0] for m in clicks] != [1,0]*4:
            raise RuntimeError("G3O_NATIVE_MIDDLE_TRACE_MISSING "+repr(clicks))
        if any(m[2] != clicks[0][1] for m in clicks) or \
                clicks[0][3] != 0 or any(m[3] != 1 for m in clicks[4:]) or \
                not clicks[4][1] > clicks[2][1] or \
                not clicks[6][1] > clicks[4][1]:
            raise RuntimeError("G3O_STALE_EVENT_SERIAL_REUSED "+repr(clicks))
        events = [(k,int(n),int(p),int(s)) for k,n,p,s in event_re.findall(raw)]
        if not events or any(e[2] for e in events) or \
                any(b[1] != a[1]+1 for a,b in zip(events,events[1:])):
            raise RuntimeError("G3O_EVENT_SERIAL_NOT_MONOTONIC "+repr(events))
        def event_between(kind, lo, hi):
            return any(e[0]==kind and lo < e[1] <= hi and e[3]==1
                       for e in events)
        if not event_between("second_add",clicks[3][1],clicks[4][1]) or \
                not event_between("second_remove",clicks[5][1],clicks[6][1]) or \
                not event_between("layout_change",clicks[5][1],clicks[6][1]):
            raise RuntimeError("G3O_REAL_NATIVE_TRANSITION_MISSING "+repr(events))

        # Return to the initial geometry AFTER the fourth native pair.
        # A layout change signal must still advance the diagnostic serial.
        prior = events[-1][1]
        restored = c1.run_ipc(swaymsg,ipc,env,
                             f"output {output} pos {old_x} {geometry[1]}")
        if not (isinstance(restored,list) and len(restored)==1 and
                restored[0].get("success") is True):
            raise RuntimeError("G3O_LAYOUT_RESTORE_REFUSED "+repr(restored))
        for _ in range(120):
            state = c1.run_ipc(swaymsg,ipc,env,"-t","get_outputs")
            raw = log.read_text(errors="replace")
            after = [(k,int(n),int(p),int(s))
                     for k,n,p,s in event_re.findall(raw)]
            rects = [v.get("rect",{}) for v in state if isinstance(v,dict)
                     and v.get("name")==output and v.get("active") is True]
            if len(rects)==1 and tuple(rects[0].get(k) for k in
                    ("x","y","width","height"))==geometry and any(
                    k=="layout_change" and n>prior and p==0 and s==1
                    for k,n,p,s in after):
                break
            time.sleep(.05)
        else:
            raise RuntimeError("G3O_NATIVE_LAYOUT_ABA_NOT_OBSERVED")
        if len(middle_re.findall(raw))!=8 or any(
                b[1]!=a[1]+1 for a,b in zip(after,after[1:])):
            raise RuntimeError("G3O_EXTRA_INPUT_OR_SERIAL_GAP")
        print("PA_G3O_NATIVE_EVENT_SERIAL_ABA_DENIED_PASS "
              "keyboard_add_remove=REAL layout_move_restore=REAL "
              "captured_serial_stale=1 native_middle_callbacks=8 "
              "normal_dispatch=8 authorization=NONE",flush=True)


        # G3P: the old serial must remain stale on both real middle
        # callbacks AFTER the geometry and keyboard count have returned.
        restore_serial = max(n for k,n,p,s in after
                             if k=="layout_change" and n>prior and not p and s)
        actor.stdin.write("click\n")
        actor.stdin.flush()
        fifth = expect_prefix(actor,"PA_G3K_KERNEL_MIDDLE_SENT ")
        if fifth != "PA_G3K_KERNEL_MIDDLE_SENT round=5 pressed=1 released=1 axes=0":
            raise RuntimeError("G3P_FIFTH_KERNEL_PAIR_NOT_SENT "+repr(fifth))
        deadline = time.monotonic()+15
        while True:
            raw=log.read_text(errors="replace")
            clicks5=[tuple(map(int,m)) for m in middle_re.findall(raw)]
            if len(clicks5)>=10 and all(raw.count(
                    f"PA_G2_NATIVE_FROM_SEATOP state={phase}")>=5
                    for phase in (0,1)):
                break
            if time.monotonic()>deadline:
                raise RuntimeError("G3P_POST_RESTORE_NATIVE_CALLBACK_TIMEOUT "+repr(clicks5))
            time.sleep(.05)
        if len(clicks5)!=10 or clicks5[:8]!=clicks or \
                [m[0] for m in clicks5[8:]]!=[1,0] or any(
                    m[1]<restore_serial or m[1]<=clicks[7][1] or
                    m[2]!=clicks[0][1] or m[3]!=1 for m in clicks5[8:]):
            raise RuntimeError("G3P_RESTORED_GEOMETRY_STALE_SERIAL_REUSED "+repr(clicks5))
        events5=[(k,int(n),int(p),int(s)) for k,n,p,s in event_re.findall(raw)]
        if any(e[2] for e in events5) or any(
                b[1]!=a[1]+1 for a,b in zip(events5,events5[1:])):
            raise RuntimeError("G3P_EVENT_SERIAL_POISONED_OR_GAP")
        layout5=[tuple(map(int,m)) for m in G3L.findall(raw)]
        keys5=[tuple(map(int,m)) for m in G3M.findall(raw)]
        guards5=[tuple(map(int,m)) for m in G3K.findall(raw)]
        source5=[tuple(map(int,m)) for m in EVENT.findall(raw)]
        os5=[tuple(map(int,m)) for m in G3H.findall(raw)]
        for rows in (layout5,keys5,guards5,source5,os5):
            if len(rows)!=10 or [r[0] for r in rows]!=[1,0]*5:
                raise RuntimeError("G3P_NATIVE_PATH_COUNT_INVALID "+repr(rows))
        if any(tuple(r[3:7])!=geometry or r[7]!=0 for r in layout5[8:]):
            raise RuntimeError("G3P_CURSOR_CALLBACK_GEOMETRY_NOT_RESTORED")
        if any(r[1]!=baseline or r[2]!=1 or r[3]!=0 or r[4]!=other or
               r[5]!=groups or r[10]!=0 for r in keys5[8:]):
            raise RuntimeError("G3P_CURSOR_CALLBACK_KEYBOARD_BASELINE_NOT_RESTORED")
        if any(any(r[i]!=0 for i in (4,5,6,7)) for r in guards5):
            raise RuntimeError("G3P_NATIVE_GUARD_ADMITTED")
        if any(r[1]!=1 or r[2]!=1 or r[3]!=1 or r[9]!=0 or
               r[10]!=0 or r[11]!=0 or r[12]!=1 for r in source5):
            raise RuntimeError("G3P_NATIVE_SOURCE_DENIAL_CHANGED")
        if any(r[8]!=0 or r[9]!=0 for r in os5):
            raise RuntimeError("G3P_OS_DIAGNOSTIC_AUTHORIZED")
        for phase in (0,1):
            for token in (
                f"PA_G2_NATIVE_INGRESS state={phase} autoscroll_null=1",
                f"PA_G2_NATIVE_PAIR state={phase} weather=0 company=0",
                f"PA_G2_NATIVE_TO_SEATOP state={phase}",
                f"PA_G2_NATIVE_FROM_SEATOP state={phase}",
            ):
                if raw.count(token)!=5:
                    raise RuntimeError("G3P_NORMAL_DISPATCH_CHANGED "+repr(token))
        print("PA_G3P_POST_ABA_NATIVE_INPUT_DENIED_PASS "
              "restored_layout=1 restored_keyboard_count=1 "
              "original_serial_stale=1 native_middle_callbacks=10 "
              "normal_dispatch=10 guard_begin_tick_denied=1 authorization=NONE",
              flush=True)


        # G3Q: keep devices connected and toggle actual seat grouping.
        snap_re=re.compile(r"PA_G3Q_GROUP_SNAPSHOT phase=(\d+) primary=(\d+) "
            r"member=(\d+) members=(\d+) groups=(\d+) can_consume=0 can_axis=0")
        trans_re=re.compile(r"PA_G3Q_PRIMARY_TRANSITION add=(\d+) before=(\d+) "
            r"after=(\d+) was=(\d+) member=(\d+) changed=(\d+) can_axis=0")
        def rows(pattern, data):
            return [tuple(map(int,m)) for m in pattern.findall(data)]
        def input_ids():
            devices=c1.run_ipc(swaymsg,ipc,env,"-t","get_inputs")
            return sorted((i["identifier"],i["type"]) for i in devices
                          if isinstance(i,dict))
        original_ids=input_ids()
        original=rows(snap_re,raw)
        if len(original)!=10 or any(r[1]!=1 or r[2]!=1 or r[3]<1 or r[4]<1
                                   for r in original[-2:]):
            raise RuntimeError("G3Q_PRIMARY_GROUP_BASELINE_MISSING "+repr(original))
        seats=c1.run_ipc(swaymsg,ipc,env,"-t","get_seats")
        if len(seats)!=1 or not re.fullmatch(r"[A-Za-z0-9_-]+",seats[0]["name"]):
            raise RuntimeError("G3Q_TEST_SEAT_NOT_UNIQUE")
        seat_name=seats[0]["name"]
        previous=clicks5[-1][1]
        for round_no,mode,member,kind in (
                (6,"none",0,"group_member_removed"),(7,"smart",1,"group_member_added")):
            before_log=log.read_text(errors="replace")
            prior_trans=len(rows(trans_re,before_log))
            result=c1.run_ipc(swaymsg,ipc,env,f"seat {seat_name} keyboard_grouping {mode}")
            if not (isinstance(result,list) and len(result)==1 and
                    result[0].get("success") is True):
                raise RuntimeError("G3Q_GROUPING_IPC_REFUSED "+repr(result))
            # Require actual membership pointer and list cardinality change
            # before sending another input, not just command success.
            deadline=time.monotonic()+10
            while True:
                raw=log.read_text(errors="replace")
                transitions=rows(trans_re,raw)[prior_trans:]
                if any(t[0]==member and t[3]==1-member and t[4]==member and
                       t[5]==1 and t[2]-t[1]==(1 if member else -1)
                       for t in transitions):
                    break
                if time.monotonic()>deadline:
                    raise RuntimeError("G3Q_ACTUAL_GROUP_TRANSITION_MISSING "+repr(transitions))
                time.sleep(.05)
            if input_ids()!=original_ids:
                raise RuntimeError("G3Q_DEVICE_INVENTORY_CHANGED")
            actor.stdin.write("click\n")
            actor.stdin.flush()
            sent=expect_prefix(actor,"PA_G3K_KERNEL_MIDDLE_SENT ")
            if sent!=f"PA_G3K_KERNEL_MIDDLE_SENT round={round_no} pressed=1 released=1 axes=0":
                raise RuntimeError("G3Q_WRONG_ACTOR_ROUND "+repr(sent))
            deadline=time.monotonic()+15
            while True:
                raw=log.read_text(errors="replace")
                snapshots=rows(snap_re,raw)
                mids=rows(middle_re,raw)
                if len(snapshots)>=round_no*2 and len(mids)>=round_no*2 and all(
                    raw.count(f"PA_G2_NATIVE_FROM_SEATOP state={ph}")>=round_no
                    for ph in (0,1)):
                    break
                if time.monotonic()>deadline:
                    raise RuntimeError("G3Q_NATIVE_PAIR_TIMEOUT")
                time.sleep(.05)
            if len(snapshots)!=round_no*2 or len(mids)!=round_no*2:
                raise RuntimeError("G3Q_EXTRA_NATIVE_INPUT")
            if [r[0] for r in snapshots[-2:]]!=[1,0] or any(
                    r[1]!=1 or r[2]!=member or
                    (member==0 and (r[3]!=0 or r[4]!=0)) or
                    (member==1 and r[3:]!=original[-1][3:])
                    for r in snapshots[-2:]):
                raise RuntimeError("G3Q_GROUP_MEMBERSHIP_NOT_RESTORED "+repr(snapshots[-2:]))
            if any(m[1]<=previous or m[2]!=clicks5[0][1] or m[3]!=1
                   for m in mids[-2:]):
                raise RuntimeError("G3Q_SERIAL_NOT_FRESH_OR_ORIGINAL_NOT_STALE")
            native_events=[(k,int(n),int(po),int(st))
                           for k,n,po,st in event_re.findall(raw)]
            if not any(k==kind and previous<n<=mids[-2][1] and po==0 and st==1
                       for k,n,po,st in native_events):
                raise RuntimeError("G3Q_CONFIRMED_MEMBERSHIP_SERIAL_MISSING")
            if any(po for k,n,po,st in native_events) or any(
                    b[1]!=a[1]+1 for a,b in zip(native_events,native_events[1:])):
                raise RuntimeError("G3Q_SERIAL_GAP_OR_POISON")
            if input_ids()!=original_ids:
                raise RuntimeError("G3Q_DEVICE_INVENTORY_CHANGED_AFTER_CLICK")
            previous=mids[-1][1]
        for pattern in (G3K, G3L, G3M, EVENT, G3H):
            native_rows=rows(pattern,raw)
            if len(native_rows)!=14 or [r[0] for r in native_rows]!=[1,0]*7:
                raise RuntimeError("G3Q_NATIVE_DENIAL_TRACE_COUNT")
        if any(any(r[i]!=0 for i in (4,5,6,7)) for r in rows(G3K,raw)):
            raise RuntimeError("G3Q_GUARD_ADMITTED")
        if any(r[8]!=0 or r[9]!=0 for r in rows(G3H,raw)):
            raise RuntimeError("G3Q_OS_AUTHORIZED")
        if any(r[1]!=baseline or r[2]!=1 or r[3]!=0 or r[4]!=other or r[10]!=0
               for r in rows(G3M,raw)[10:]):
            raise RuntimeError("G3Q_SEAT_MEMBERSHIP_CHANGED")
        for ph in (0,1):
            for token in (
                f"PA_G2_NATIVE_INGRESS state={ph} autoscroll_null=1",
                f"PA_G2_NATIVE_PAIR state={ph} weather=0 company=0",
                f"PA_G2_NATIVE_TO_SEATOP state={ph}",
                f"PA_G2_NATIVE_FROM_SEATOP state={ph}",
            ):
                if raw.count(token)!=7:
                    raise RuntimeError("G3Q_NORMAL_DISPATCH_CHANGED "+token)
        print("PA_G3Q_NATIVE_GROUP_ABA_DENIED_PASS "
              "group_membership=1-0-1 device_inventory=UNCHANGED "
              "native_middle_callbacks=14 normal_dispatch=14 authorization=NONE",
              flush=True)


        # G3R: create, destroy, recreate and destroy the SAME named empty seat.
        # Use actual config commands and reload, not simulated object deletion.
        # The pointer stays on the original seat throughout.
        transient="pa-g3r-transient"
        create_re=re.compile(r"PA_G3R_SEAT_CREATED name=pa-g3r-transient epoch=(\d+) devices=(\d+) can_axis=0")
        revoke_re=re.compile(r"PA_G3R_SEAT_REVOKED name=pa-g3r-transient before=(\d+) after=(\d+) devices=(\d+) can_axis=0")
        cursor_seat_re=re.compile(r"PA_G3R_CURSOR_SEAT phase=(\d+) name=([A-Za-z0-9_-]+) epoch=(\d+) devices=(\d+) can_consume=0 can_axis=0")
        def seat_rows(data):
            return [(int(ph),name,int(epoch),int(count))
                    for ph,name,epoch,count in cursor_seat_re.findall(data)]
        originals=seat_rows(raw)
        if len(originals)!=14 or any(r[1]!=seat_name or r[2]<=0 or r[2]!=originals[0][2]
                                    or r[3]<1 for r in originals):
            raise RuntimeError("G3R_ORIGINAL_SEAT_EPOCH_INVALID "+repr(originals))
        initial_device_count=originals[-1][3]
        # Preserve default seat configuration explicitly across reload. This
        # config is the already isolated tempfile passed to sway -c.
        conf.write_text(conf.read_text()+f"\nseat {seat_name} fallback true\n"
                        f"seat {seat_name} keyboard_grouping smart\n")
        def seat_inventory():
            result=c1.run_ipc(swaymsg,ipc,env,"-t","get_seats")
            return {v["name"]:sorted(d["identifier"] for d in v["devices"])
                    for v in result}
        initial_seats=seat_inventory()
        if set(initial_seats)!={seat_name}:
            raise RuntimeError("G3R_BASELINE_SEAT_NOT_UNIQUE")
        created_epochs=[]
        for round_no,present in ((8,True),(9,False),(10,True),(11,False)):
            command=f"seat {transient} fallback false" if present else "reload"
            result=c1.run_ipc(swaymsg,ipc,env,command)
            if not (isinstance(result,list) and len(result)==1 and result[0].get("success") is True):
                raise RuntimeError("G3R_SEAT_COMMAND_REFUSED "+repr(result))
            expected=dict(initial_seats)
            if present: expected[transient]=[]
            deadline=time.monotonic()+15
            while True:
                raw=log.read_text(errors="replace")
                creates=rows(create_re,raw)
                revokes=rows(revoke_re,raw)
                count=(round_no-6)//2
                if seat_inventory()==expected and len(creates)==count and len(revokes)==(round_no-7)//2:
                    break
                if time.monotonic()>deadline:
                    raise RuntimeError("G3R_NATIVE_LIFECYCLE_TIMEOUT "+repr((seat_inventory(),creates,revokes)))
                time.sleep(.05)
            if input_ids()!=original_ids:
                raise RuntimeError("G3R_DEVICE_INVENTORY_CHANGED")
            if present:
                epoch,devices=creates[-1]
                if devices or epoch<=originals[0][2] or (created_epochs and epoch<=created_epochs[-1]):
                    raise RuntimeError("G3R_RECREATED_SEAT_EPOCH_REUSED "+repr(creates))
                created_epochs.append(epoch)
            elif revokes[-1]!=(created_epochs[-1],0,0):
                raise RuntimeError("G3R_SEAT_NOT_REVOKED_BEFORE_TEARDOWN "+repr(revokes))
            actor.stdin.write("click\n")
            actor.stdin.flush()
            sent=expect_prefix(actor,"PA_G3K_KERNEL_MIDDLE_SENT ")
            if sent!=f"PA_G3K_KERNEL_MIDDLE_SENT round={round_no} pressed=1 released=1 axes=0":
                raise RuntimeError("G3R_WRONG_ACTOR_ROUND "+repr(sent))
            deadline=time.monotonic()+15
            while True:
                raw=log.read_text(errors="replace")
                mids=rows(middle_re,raw)
                cursor_seats=seat_rows(raw)
                if len(mids)>=round_no*2 and len(cursor_seats)>=round_no*2 and all(
                    raw.count(f"PA_G2_NATIVE_FROM_SEATOP state={ph}")>=round_no
                    for ph in (0,1)):
                    break
                if time.monotonic()>deadline:
                    raise RuntimeError("G3R_NATIVE_PAIR_TIMEOUT")
                time.sleep(.05)
            if len(mids)!=round_no*2 or len(cursor_seats)!=round_no*2 or \
                    [r[0] for r in cursor_seats[-2:]]!=[1,0] or any(
                    r[1:]!=(seat_name,originals[0][2],initial_device_count)
                    for r in cursor_seats[-2:]):
                raise RuntimeError("G3R_ORIGINAL_POINTER_SEAT_CHANGED")
            if any(m[1]<=previous or m[2]!=clicks5[0][1] or m[3]!=1 for m in mids[-2:]):
                raise RuntimeError("G3R_STALE_SERIAL_REUSED")
            native_events=[(k,int(n),int(po),int(st)) for k,n,po,st in event_re.findall(raw)]
            kind="seat_created" if present else "seat_destroy"
            if not any(k==kind and previous<n<=mids[-2][1] and not po and st
                       for k,n,po,st in native_events):
                raise RuntimeError("G3R_NATIVE_SEAT_SERIAL_MISSING")
            if any(po for k,n,po,st in native_events) or any(
                    b[1]!=a[1]+1 for a,b in zip(native_events,native_events[1:])):
                raise RuntimeError("G3R_SERIAL_GAP_OR_POISON")
            if seat_inventory()!=expected or input_ids()!=original_ids:
                raise RuntimeError("G3R_INVENTORY_CHANGED_AFTER_CLICK")
            previous=mids[-1][1]
        for pattern in (G3K,G3L,G3M,EVENT,G3H):
            native_rows=rows(pattern,raw)
            if len(native_rows)!=22 or [r[0] for r in native_rows]!=[1,0]*11:
                raise RuntimeError("G3R_NATIVE_DENIAL_TRACE_COUNT")
        if any(any(r[i]!=0 for i in (4,5,6,7)) for r in rows(G3K,raw)) or any(
                r[8]!=0 or r[9]!=0 for r in rows(G3H,raw)):
            raise RuntimeError("G3R_GUARD_ADMITTED")
        for ph in (0,1):
            for token in (
                f"PA_G2_NATIVE_INGRESS state={ph} autoscroll_null=1",
                f"PA_G2_NATIVE_PAIR state={ph} weather=0 company=0",
                f"PA_G2_NATIVE_TO_SEATOP state={ph}",
                f"PA_G2_NATIVE_FROM_SEATOP state={ph}",
            ):
                if raw.count(token)!=11:
                    raise RuntimeError("G3R_NORMAL_DISPATCH_CHANGED "+token)
        print("PA_G3R_NATIVE_SEAT_LIFECYCLE_DENIED_PASS "
              f"first_epoch={created_epochs[0]} second_epoch={created_epochs[1]} "
              "revoked_before_teardown=2 original_pointer_seat=UNCHANGED "
              "native_middle_callbacks=22 normal_dispatch=22 authorization=NONE",
              flush=True)

        actor.stdin.write("stop\n")
        actor.stdin.flush()
        end,out=actor.communicate(timeout=12)
        if actor.returncode!=0 or \
           "PA_G3K_KERNEL_UINPUT_DESTROYED_PASS" not in end:
            raise RuntimeError("PA_G3J_KERNEL_DEVICE_CLEANUP_FAILURE "+
                               repr((actor.returncode,end,out[-1000:])))
        actor=None
        # Observe actual input-manager removal after kernel UI_DEV_DESTROY.
        for _ in range(100):
            active=c1.run_ipc(swaymsg,ipc,env,"-t","get_inputs")
            if isinstance(active,list) and not any(
                    isinstance(i,dict) and
                    i.get("name")=="PA-G3J-KERNEL-SYNTHETIC-NOT-PHYSICAL"
                    for i in active):
                break
            time.sleep(.05)
        else:
            raise RuntimeError("PA_G3K_NATIVE_REMOVED_DEVICE_STILL_REGISTERED")
        print("PA_G3K_NATIVE_LIBINPUT_DEVICE_LIFETIME_REVOKED_PASS "
              "input_manager_registration=REMOVED",flush=True)
        inputs=c1.run_ipc(swaymsg,ipc,env,"-t","get_inputs")
        if any(isinstance(i,dict) and
               i.get("name")=="PA-G3L-SYNTHETIC-KEYBOARD-NOT-HARDWARE"
               for i in inputs):
            raise RuntimeError("PA_G3L_SYNTHETIC_KEYBOARD_NOT_REMOVED")
        print("PA_G3L_KERNEL_KEYBOARD_AND_MOUSE_DESTROYED_PASS",flush=True)
        if any(isinstance(i,dict) and
               i.get("name")=="PA-G3M-SECOND-SYNTHETIC-KEYBOARD"
               for i in inputs):
            raise RuntimeError("PA_G3M_SECOND_REAPPEARED_AFTER_DESTROY")
        print("PA_G3M_NATIVE_ALL_SYNTHETIC_INPUTS_REMOVED_PASS",flush=True)
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
