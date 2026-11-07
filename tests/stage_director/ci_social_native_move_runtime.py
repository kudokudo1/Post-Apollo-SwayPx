#!/usr/bin/env python3
"""Real native seatop_move_floating pair witness; disposable patched SwayPX CI.

Runs the unmodified production test fixture clients against a disposable
source-overlay compositor, then drives real Sway seatop_pointer_motion via a
test-only cursor event entrypoint. Only move is in scope; resize isn't yet.
"""
from __future__ import annotations
import os
import re
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time

import ci_social_real_wayland_pair as b
from ci_social_streaming_seam_witness import StreamingSeamWitness

def check_inflight_seam(grim, path, env, host_rect, guest_rect, label):
    """Sample actual rendered perimeter pixels while native seatop is active.

    This checks eight deliberate steps, NOT every presentation frame.
    Such captures cannot prove the operator's zero-gap requirement by
    themselves, but catch wrong stacking and exposed boundary gaps.
    """
    if guest_rect != b.guest_from_chassis(host_rect):
        raise RuntimeError("SD_CI_SOCIAL_INFLIGHT_PLAN_MISMATCH "+label)
    x,y,w,h=host_rect
    gx,gy,gw,gh=guest_rect
    if not (0 <= x < gx < x+w <= b.WIDTH and
            0 <= y < gy < y+h <= b.HEIGHT and
            w > 500 and h > 400):
        raise RuntimeError("SD_CI_SOCIAL_INFLIGHT_OUTSIDE_OUTPUT "+label)

    samples=[]
    def expect(name,px,py,color):
        samples.append((name,(px,py),color))
    for row in (y+40,y+h//2,y+h-41):
        for offset in (4,55,109,110):
            expect("rail",x+offset,row,b.HOST_COLOR)
        for px in (gx+2,gx+gw//2,gx+gw-2):
            expect("guest",px,row,b.GUEST_COLOR)
        expect("right-border",x+w-1,row,b.HOST_COLOR)
    for px in (gx+30,gx+gw//2,gx+gw-30):
        expect("top-border",px,y,b.HOST_COLOR)
        expect("top-guest",px,gy+2,b.GUEST_COLOR)
        expect("bottom-guest",px,gy+gh-3,b.GUEST_COLOR)
        expect("bottom-border",px,y+h-1,b.HOST_COLOR)
    expect("unrelated",1650,770,b.OTHER_COLOR)
    expect("background",25,25,(0,0,0))
    b.run(str(grim),"-t","ppm",str(path),env=env)
    data=path.read_bytes()
    for name,(px,py),expected in samples:
        actual=b.ppm_pixel(data,px,py)
        if max(abs(got-want) for got,want in zip(actual,expected))>25:
            raise RuntimeError(
                "SD_CI_SOCIAL_INFLIGHT_SEAM_FAIL "
                f"stage={label} area={name} point={px},{py} "
                f"observed={actual} expected={expected}")
    print(f"SD_CI_SOCIAL_INFLIGHT_SEAM stage={label} "
          f"samples={len(samples)} rendered_pixels=PASS "
          "snapshot_between_steps=1 all_frames=NOT_VERIFIED",
          flush=True)


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
                   follower_app,follower_proc,steps,witness=None,stream=None,pace=0.0):
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
    running_dx=running_dy=0
    streaming=False
    try:
        if stream is not None:
            stream.start()
            streaming=True
            time.sleep(0.09)
        for step,(dx,dy) in enumerate(steps,start=1):
            cmd(swaymsg,ipc,env,f"seat - cursor ci-social-step {dx} {dy}")
            running_dx+=dx
            running_dy+=dy
            if witness is not None:
                witness(running_dx,running_dy,step)
            if pace:
                time.sleep(pace)
        if streaming:
            time.sleep(0.09)
    finally:
        try:
            if streaming:
                stream.stop()
        finally:
            cmd(swaymsg,ipc,env,"seat - cursor ci-social-end 1")
    return running_dx,running_dy


def ci_leased_resize(swaymsg, ipc, env, leader_id, leader_app, leader_proc,
                     follower_app, follower_proc, steps, witness=None,
                     stream=None, pace=0.0):
    c=f'[con_id="{leader_id}"]'
    cmd(swaymsg,ipc,env,f"{c} focus")
    leader=b.select(b.read_tree(swaymsg,ipc,env),leader_app,leader_proc.pid)
    if leader is None or leader["id"] != leader_id:
        raise RuntimeError("SD_CI_NATIVE_RESIZE_INVALID_LEADER")
    rect=leader["rect"]
    cmd(swaymsg,ipc,env,
        f'seat - cursor set {rect["x"]+60} {rect["y"]+140}')
    cmd(swaymsg,ipc,env,f"{c} focus")
    cmd(swaymsg,ipc,env,"seat - cursor ci-social-start-resize 1")
    running_dx=running_dy=0
    streaming=False
    try:
        if stream is not None:
            stream.start()
            streaming=True
            time.sleep(0.09)
        for step,(dx,dy) in enumerate(steps,start=1):
            cmd(swaymsg,ipc,env,f"seat - cursor ci-social-step {dx} {dy}")
            running_dx+=dx
            running_dy+=dy
            target=(rect["x"],rect["y"],
                    rect["width"]+running_dx,
                    rect["height"]+running_dy)
            other=(b.guest_from_chassis(target)
                   if leader_app==b.APP_HOST else b.chassis_from_guest(target))
            # The real Wayland clients must ACK each configured rectangle.
            # We do NOT call this uninterrupted continuous resize: only
            # screenshot sampling overlaps the input steps concurrently.
            b.wait_for(swaymsg,ipc,env,leader_app,leader_proc.pid,target)
            b.wait_for(swaymsg,ipc,env,follower_app,follower_proc.pid,other)
            if witness is not None:
                if leader_app==b.APP_HOST:
                    witness(target,other,step)
                else:
                    witness(other,target,step)
            if pace:
                time.sleep(pace)
            print(f"SD_CI_SOCIAL_NATIVE_RESIZE_STEP leader={leader_app} "
                  f"step={step} settled_host_guest=1 "
                  f"leader_size={target[2]}x{target[3]}",flush=True)
        if streaming:
            time.sleep(0.09)
    finally:
        try:
            if streaming:
                stream.stop()
        finally:
            cmd(swaymsg,ipc,env,"seat - cursor ci-social-end 1")
    return running_dx,running_dy


def main():
    sway,swaymsg,foot,grim,mesa=ci_require()
    runtime=Path(tempfile.mkdtemp(prefix="social-native-seatop-",
                                  dir=os.environ["RUNNER_TEMP"]))
    runtime.chmod(0o700)
    lease=runtime/"pair-lease.txt"
    env=dict(os.environ)
    if env.get("STAGE_DIRECTOR_CI_SOCIAL_FRAME_TRACE") != "1":
        raise RuntimeError("BLOCKED_SOCIAL_OUTPUT_FRAME_TRACE_NOT_CI")
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

        def in_flight(hrect,grect,stage):
            b.wait_for(swaymsg,ipc,env,b.APP_HOST,host.pid,hrect)
            b.wait_for(swaymsg,ipc,env,b.APP_GUEST,guest.pid,grect)
            b.wait_for(swaymsg,ipc,env,b.APP_OTHER,other.pid,unrelated_box)
            check_inflight_seam(
                grim,runtime/f"{stage}.ppm",env,hrect,grect,stage)

        # HOST-LED: the test-only cursor starter enters *real native seatop*.
        # Every subsequent step executes the native pointer-motion handler.
        dx,dy=ci_leased_move(swaymsg,ipc,env,ids[b.APP_HOST],b.APP_HOST,
                            host,b.APP_GUEST,guest,[(55,25),(40,20)],
                            witness=lambda ox,oy,step: in_flight(
                                (host_box[0]+ox,host_box[1]+oy,*host_box[2:]),
                                b.guest_from_chassis((
                                    host_box[0]+ox,host_box[1]+oy,*host_box[2:])),
                                f"host-drag-{step}"))
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
                            guest,b.APP_HOST,host,[(-35,35),(-20,10)],
                            witness=lambda ox,oy,step: in_flight(
                                (host_box[0]+ox,host_box[1]+oy,*host_box[2:]),
                                b.guest_from_chassis((
                                    host_box[0]+ox,host_box[1]+oy,*host_box[2:])),
                                f"guest-drag-{step}"))
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

        # HOST-LED NATIVE RESIZE. The guest slot follows the actual
        # native leader's pending size within one seatop commit.
        dw,dh=ci_leased_resize(swaymsg,ipc,env,ids[b.APP_HOST],b.APP_HOST,
                              host,b.APP_GUEST,guest,[(70,40),(20,20)],
                              witness=lambda hr,gr,step: in_flight(
                                  hr,gr,f"host-resize-{step}"))
        host_box=(host_box[0],host_box[1],
                  host_box[2]+dw,host_box[3]+dh)
        guest_box=b.guest_from_chassis(host_box)
        exact(swaymsg,ipc,env,b.APP_HOST,host.pid,host_box)
        exact(swaymsg,ipc,env,b.APP_GUEST,guest.pid,guest_box)
        exact(swaymsg,ipc,env,b.APP_OTHER,other.pid,unrelated_box)
        cmd(swaymsg,ipc,env,f'[con_id="{ids[b.APP_GUEST]}"] focus')
        b.capture(grim,runtime/"host-native-resize.ppm",env,[
            ("rail",(host_box[0]+25,host_box[1]+155),b.HOST_COLOR),
            ("guest",(guest_box[0]+170,guest_box[1]+155),b.GUEST_COLOR),
            ("unrelated",(unrelated_box[0]+130,unrelated_box[1]+110),
             b.OTHER_COLOR),
            ("background",(25,25),(0,0,0)),
        ])
        print("SD_CI_SOCIAL_NATIVE_HOST_RESIZE seatop=1 "
              "peer_resize_before_commit=1 exact_geometry=PASS pixels=PASS",
              flush=True)

        # GUEST-LED NATIVE RESIZE. Its content rect stays right of the
        # chassis rail, retaining exact selector+inset and outer geometry.
        dw,dh=ci_leased_resize(swaymsg,ipc,env,ids[b.APP_GUEST],b.APP_GUEST,
                              guest,b.APP_HOST,host,[(-35,-15),(-20,-15)],
                              witness=lambda hr,gr,step: in_flight(
                                  hr,gr,f"guest-resize-{step}"))
        host_box=(host_box[0],host_box[1],
                  host_box[2]+dw,host_box[3]+dh)
        guest_box=b.guest_from_chassis(host_box)
        exact(swaymsg,ipc,env,b.APP_HOST,host.pid,host_box)
        exact(swaymsg,ipc,env,b.APP_GUEST,guest.pid,guest_box)
        exact(swaymsg,ipc,env,b.APP_OTHER,other.pid,unrelated_box)
        cmd(swaymsg,ipc,env,f'[con_id="{ids[b.APP_GUEST]}"] focus')
        b.capture(grim,runtime/"guest-native-resize.ppm",env,[
            ("rail",(host_box[0]+25,host_box[1]+155),b.HOST_COLOR),
            ("guest",(guest_box[0]+170,guest_box[1]+155),b.GUEST_COLOR),
            ("unrelated",(unrelated_box[0]+130,unrelated_box[1]+110),
             b.OTHER_COLOR),
            ("background",(25,25),(0,0,0)),
        ])
        print("SD_CI_SOCIAL_NATIVE_GUEST_RESIZE seatop=1 "
              "peer_resize_before_commit=1 exact_geometry=PASS pixels=PASS",
              flush=True)

        # Bounded streaming witness: screenshots are captured by another
        # process while the real native move/resize seatop receives several
        # pointer steps. Pair geometry is read before/after each screenshot;
        # changed captures are marked transitions, not visual passes.
        def stream(name):
            return StreamingSeamWitness(
                name=name,swaymsg=swaymsg,ipc=ipc,grim=grim,env=env,
                root=runtime,ids=ids,
                pids={b.APP_HOST:host.pid,b.APP_GUEST:guest.pid,
                      b.APP_OTHER:other.pid},
                unrelated_rect=unrelated_box,
                min_frames=4,min_positions=3)

        dx,dy=ci_leased_move(
            swaymsg,ipc,env,ids[b.APP_HOST],b.APP_HOST,host,
            b.APP_GUEST,guest,[(12,4)]*12,
            stream=stream("host-move"),pace=0.065)
        host_box=(host_box[0]+dx,host_box[1]+dy,*host_box[2:])
        guest_box=b.guest_from_chassis(host_box)
        exact(swaymsg,ipc,env,b.APP_HOST,host.pid,host_box)
        exact(swaymsg,ipc,env,b.APP_GUEST,guest.pid,guest_box)

        dx,dy=ci_leased_move(
            swaymsg,ipc,env,ids[b.APP_GUEST],b.APP_GUEST,guest,
            b.APP_HOST,host,[(-6,3)]*12,
            stream=stream("guest-move"),pace=0.065)
        host_box=(host_box[0]+dx,host_box[1]+dy,*host_box[2:])
        guest_box=b.guest_from_chassis(host_box)
        exact(swaymsg,ipc,env,b.APP_HOST,host.pid,host_box)
        exact(swaymsg,ipc,env,b.APP_GUEST,guest.pid,guest_box)

        dw,dh=ci_leased_resize(
            swaymsg,ipc,env,ids[b.APP_HOST],b.APP_HOST,host,
            b.APP_GUEST,guest,[(6,3)]*10,
            stream=stream("host-resize"),pace=0.065)
        host_box=(host_box[0],host_box[1],
                  host_box[2]+dw,host_box[3]+dh)
        guest_box=b.guest_from_chassis(host_box)
        exact(swaymsg,ipc,env,b.APP_HOST,host.pid,host_box)
        exact(swaymsg,ipc,env,b.APP_GUEST,guest.pid,guest_box)

        dw,dh=ci_leased_resize(
            swaymsg,ipc,env,ids[b.APP_GUEST],b.APP_GUEST,guest,
            b.APP_HOST,host,[(-4,-2)]*10,
            stream=stream("guest-resize"),pace=0.065)
        host_box=(host_box[0],host_box[1],
                  host_box[2]+dw,host_box[3]+dh)
        guest_box=b.guest_from_chassis(host_box)
        exact(swaymsg,ipc,env,b.APP_HOST,host.pid,host_box)
        exact(swaymsg,ipc,env,b.APP_GUEST,guest.pid,guest_box)
        exact(swaymsg,ipc,env,b.APP_OTHER,other.pid,unrelated_box)
        print("SD_CI_SOCIAL_STREAMING_SUMMARY "
              "directions=4 expected_poses_per_mode=10 "
              "concurrent_presentation_sampling=1 "
              "all_presented_frames=NOT_OBSERVED",flush=True)

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
        if logs.count("SD_CI_SOCIAL_NATIVE_RESIZE_BEFORE_COMMIT") < 4:
            raise RuntimeError("SD_CI_SOCIAL_NATIVE_RESIZE_NOT_PRECOMMITTED")
        # One host-leading correction in the paused witness and one in
        # the concurrent stream. Guest-leading work must not raise twice.
        if logs.count("SD_CI_SOCIAL_GUEST_STACKING during_move=1") != 2:
            raise RuntimeError("SD_CI_SOCIAL_MOVE_GUEST_LAYER_NOT_ENFORCED")
        if logs.count("SD_CI_SOCIAL_GUEST_STACKING during_resize=1") != 2:
            raise RuntimeError("SD_CI_SOCIAL_RESIZE_GUEST_LAYER_NOT_ENFORCED")

        # Trace from the actual output repaint/commit path in the disposable
        # compositor, not an IPC tree snapshot taken after rendering.
        render_lines = re.findall(r"SD_CI_SOCIAL_OUTPUT_FRAME [^\n]+", logs)
        if len(render_lines) < 16:
            raise RuntimeError(
                "SD_CI_SOCIAL_OUTPUT_FRAMES_UNDER_SAMPLED "
                + str(len(render_lines)))
        render_ids=[]
        render_poses=set()
        for entry in render_lines:
            if ("committed=1" not in entry or "rect=PASS" not in entry
                    or "scene=PASS" not in entry
                    or "SPLIT" in entry):
                raise RuntimeError(
                    "SD_CI_SOCIAL_OUTPUT_FRAME_GEOMETRY_OR_SCENE_SPLIT "
                    + entry[:900])
            m = re.search(
                r"frame=(\d+) committed=1 host_id=(\d+) "
                r"guest_id=(\d+) .*host=([\d.,-]+) "
                r"guest=([\d.,-]+)", entry)
            if not m:
                raise RuntimeError(
                    "SD_CI_SOCIAL_OUTPUT_FRAME_PARSE_FAILED "+entry[:900])
            if (int(m.group(2)) != ids[b.APP_HOST] or
                    int(m.group(3)) != ids[b.APP_GUEST]):
                raise RuntimeError(
                    "SD_CI_SOCIAL_OUTPUT_FRAME_STALE_MEMBER")
            render_ids.append(int(m.group(1)))
            render_poses.add(m.group(4))
        if any(a >= b for a,b in zip(render_ids,render_ids[1:])):
            raise RuntimeError("SD_CI_SOCIAL_OUTPUT_FRAME_IDS_NONMONOTONIC")
        if len(render_poses) < 8:
            raise RuntimeError(
                "SD_CI_SOCIAL_OUTPUT_FRAME_POSES_UNDER_SAMPLED "
                + str(len(render_poses)))
        print("SD_CI_SOCIAL_OUTPUT_COMMIT_SUMMARY "
              f"real_committed_frames={len(render_lines)} "
              f"distinct_host_poses={len(render_poses)} "
              "rect_and_scene=PASS "
              "every_physical_display_presentation=NOT_PROVEN",
              flush=True)
        print("SD_CI_SOCIAL_INFLIGHT_SUMMARY "
              "move_samples=4 resize_samples=4 "
              "pixel_seam_checked_each_step=1 "
              "all_compositor_frames=NOT_VERIFIED",flush=True)
        print("SD_CI_SOCIAL_NATIVE_MOVE final=PASS "
              "native_seatop=1 same_precommit=1 "
              "real_physical_pointer=NOT_TESTED "
              "native_resize=CI_PROVEN "
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
