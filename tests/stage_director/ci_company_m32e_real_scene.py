#!/usr/bin/env python3
"""M32e: real Company Quickshell + offline MapLibre via M32c scene setter.

Strict disposable GitHub CI. Old reviewed M30s source tools are read-only
fixtures; live SwayPX comes from this PR's private CI binary. Synthetic
operator/launch/lease evidence exists ONLY in that private C test build.
"""
from __future__ import annotations

import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time


CANVAS = (166, 203, 1493, 788)
MARKER = "[stage-director][COMPANY_M32E][CI_ONLY] native_scene_apply=PASS"
QML_MARKER = "SD_COMPANY_M32E_REAL_QML_FULLSCREEN canvas=166,203,1493,788"
EVENT_NAMES = ("press", "motion", "release", "wheel")
QML = """import QtQuick
import Quickshell
import "widgets"
ShellRoot {
    id: proof
    CompanyNetworkW {
        id: company
        menuOpen: true
        coverageLoaded: true
        chassisLevel: "fullscreen"
        dockCenterX: 960
        chassisTopY: 160
        vectorBridgeOptIn: false
    }
    Timer {
        interval: 1900
        running: true
        repeat: false
        onTriggered: {
            const c=company.ciM30iActualCanvasHostRect;
            console.log("SD_COMPANY_M32E_REAL_QML_FULLSCREEN canvas=" +
                Math.round(c.x)+","+Math.round(c.y)+","+
                Math.round(c.width)+","+Math.round(c.height));
        }
    }
}
"""


def wait_file(path: Path, marker: str, seconds: float, process=None):
    end=time.monotonic()+seconds
    while time.monotonic()<end:
        content=path.read_text(errors="replace") if path.is_file() else ""
        if marker in content:
            return content
        if process is not None and process.poll() is not None:
            raise RuntimeError("M32E_PROCESS_EXITED_EARLY "+content[-2400:])
        time.sleep(.15)
    tail=path.read_text(errors="replace")[-5000:] if path.is_file() else ""
    raise RuntimeError("M32E_MARKER_TIMEOUT "+marker+" "+tail)


def wait_new_marker(path: Path, marker: str, before_chars: int,
                    seconds: float, process=None):
    end=time.monotonic()+seconds
    while time.monotonic()<end:
        data=path.read_text(errors="replace") if path.is_file() else ""
        if marker in data[before_chars:]:
            return data[before_chars:]
        if process is not None and process.poll() is not None:
            raise RuntimeError("M32F_PROCESS_EXITED_BEFORE_NEW_MARKER "+data[-2400:])
        time.sleep(.15)
    tail=path.read_text(errors="replace")[-5000:] if path.is_file() else ""
    raise RuntimeError("M32F_NEW_MARKER_TIMEOUT "+marker+" "+tail)


def compare_pixels(before, after, rgb):
    x,y,w,h=CANVAS
    # Positive proof of *this exact pinned* MapLibre offline source:
    # its bright cyan header rail and gold VECTOR PROOF title appear in
    # the actual Qt screenshot, not in Company's original raster map.
    # The center fixture must separately show true geographic strokes.
    # This is stronger than a sparse uniform-background delta counter,
    # which missed the original dark MapLibre fixture (63/85 samples).
    def features(image, rectangle, step=2):
        left,top,right,bottom=rectangle
        cyan=gold=0
        for py in range(top,bottom,step):
            for px in range(left,right,step):
                r,g,b=rgb(image,px,py)
                cyan+=int(g>120 and b>110 and 10*g>13*r)
                gold+=int(r>130 and g>70 and b<150)
        return cyan,gold

    top=(x+12,y,x+w-12,y+90)
    geographic=(x+170,y+290,x+590,y+560)
    baseline_header=features(before,top)
    native_header=features(after,top)
    baseline_geo=features(before,geographic)
    native_geo=features(after,geographic)
    if not (native_header[0]>=250 and native_header[1]>=100 and
            baseline_header[0]<30 and baseline_header[1]<20 and
            native_geo[0]>=35 and native_geo[1]>=10 and
            baseline_geo[0]<20 and baseline_geo[1]<10):
        raise RuntimeError("M32E_NATIVE_MAPLIBRE_GEO_PALETTE_MISSING "
            f"header={native_header} baseline_header={baseline_header} "
            f"geo={native_geo} baseline_geo={baseline_geo}")
    print("SD_COMPANY_M32E_MAPLIBRE_SOURCE_PIXELS "
          f"cyan_header={native_header[0]} gold_header={native_header[1]} "
          f"cyan_geography={native_geo[0]} gold_geography={native_geo[1]} "
          "real_MapLibre_Qt_fixture=PASS",flush=True)
    outside_ok=0
    for px,py in ((x-20,y+120),(x-20,y+320),(x+w+20,y+120),
                  (x+w+20,y+320),(x+100,y-18),(x+300,y-18),
                  (x+100,y+h+18),(x+300,y+h+18)):
        a,b=rgb(before,px,py),rgb(after,px,py)
        outside_ok+=sum(abs(t-u) for t,u in zip(a,b))<60
    if outside_ok<7:
        raise RuntimeError(f"M32E_NATIVE_SCENE_OVERFLOW outside_ok={outside_ok}")
    print(f"SD_COMPANY_M32E_REAL_PIXELS maplibre_geo_present=1 "
          f"outside_unchanged={outside_ok}/8 PASS",flush=True)


def main():
    env=os.environ
    temp=Path(env.get("RUNNER_TEMP","/not-ci"))
    legacy=Path(env.get("STAGE_DIRECTOR_M32E_LEGACY","/not-legacy"))
    if not (env.get("CI")=="true" and env.get("GITHUB_ACTIONS")=="true"
            and env.get("STAGE_DIRECTOR_CI_COMPANY_M32E")=="1"
            and env.get("STAGE_DIRECTOR_CI_COMPANY_M32E_REAL")=="1"
            and temp==Path("/home/runner/work/_temp")
            and legacy.is_dir() and (legacy/".git").is_dir()
            and os.getuid()!=0):
        raise RuntimeError("BLOCKED_M32E_NOT_EXACT_DISPOSABLE_CI")
    historical_sha=subprocess.check_output(
        ["git","-C",str(legacy),"rev-parse","HEAD"],text=True).strip()
    if historical_sha!="1c9e1104fffd688e15f0b1f3b77715e812d0f0f8":
        raise RuntimeError("BLOCKED_M32E_UNPINNED_HISTORICAL_FIXTURE")
    sys.path.insert(0,str(legacy/"tests/stage_director"))
    import ci_company_real_quickshell as real
    import ci_company_m30r_real_source as merged
    from ci_company_overlay_prototype import call,nodes,ppm,rgb
    from ci_company_input_containment import EVENT,read_until

    # The historical helper loads a QML shell via a relative file path.
    # Change only this Python process's CWD to the immutable legacy checkout.
    os.chdir(legacy)
    real.PIN=merged.COMPANY_MERGED_SHA
    e,temp,runtime,checkout,qs,swaymsg,grim,ipc=real.guard()
    qt=Path(env.get("STAGE_DIRECTOR_M30L_NATIVE_MAPLIBRE","/missing"))
    pointer_bin=Path(env.get("STAGE_DIRECTOR_TEST_COMPANY_POINTER","/missing"))
    compat=Path(env.get("STAGE_DIRECTOR_M30I_QML_IMPORT_PATH","/missing"))
    if not all(str(p).startswith("/nix/store/") and p.is_file()
               for p in (qt,pointer_bin)):
        raise RuntimeError("BLOCKED_M32E_NATIVE_QT_OR_POINTER_NOT_NIX")
    if not (str(compat).startswith("/nix/store/") and
            (compat/"Qt5Compat/GraphicalEffects/qmldir").is_file()):
        raise RuntimeError("BLOCKED_M32E_COMPAT_MISSING")
    shell=merged.instrument_without_geometry_repair(
        checkout,temp/"sd-company-m32e-quickshell-copy")
    shell.write_text(QML,encoding="utf-8")
    qml_paths=str(compat)+(":"+env["QML_IMPORT_PATH"]
                            if env.get("QML_IMPORT_PATH") else "")
    home=temp/"sd-company-m32e-home"
    for folder in ("",".config",".cache",".local/share"):
        (home/folder).mkdir(parents=True,exist_ok=True,mode=0o700)
    qsenv={**env,"HOME":str(home),
           "XDG_CONFIG_HOME":str(home/".config"),
           "XDG_DATA_HOME":str(home/".local/share"),
           "XDG_CACHE_HOME":str(home/".cache"),
           "QML_IMPORT_PATH":qml_paths,"QML2_IMPORT_PATH":qml_paths,
           "QT_QPA_PLATFORM":"wayland","NO_COLOR":"1"}
    company_log=runtime/"sd-company-m32e-quickshell.log"
    qt_log=runtime/"sd-company-m32e-maplibre.log"
    sway_log=runtime/"sway.log"
    company=actor=pointer=None

    def screenshot(label):
        image_path=temp/f"sd-company-m32e-{label}.ppm"
        call(str(grim),"-t","ppm",str(image_path),timeout=25)
        return ppm(image_path)

    try:
        with company_log.open("w") as log:
            company=subprocess.Popen(
                [str(qs),"--no-color","-p",str(shell)],
                env=qsenv,stdout=log,stderr=subprocess.STDOUT)
        wait_file(company_log,QML_MARKER,35,company)
        before=screenshot("before")
        print("SD_COMPANY_M32E_REAL_COMPANY original_merged_QML=1 "
              "actual_fullscreen_canvas=1493x788 PASS",flush=True)

        with qt_log.open("w") as log:
            actor=subprocess.Popen([str(qt)],stdout=log,
                stderr=subprocess.STDOUT,
                env={**env,"QT_QPA_PLATFORM":"wayland",
                     "QT_WAYLAND_DISABLE_WINDOWDECORATION":"1",
                     "QSG_RHI_BACKEND":"opengl",
                     "LIBGL_ALWAYS_SOFTWARE":"1",
                     "STAGE_DIRECTOR_CI_COMPANY_M30L":"1",
                     "STAGE_DIRECTOR_CI_COMPANY_M30N":"1",
                     "STAGE_DIRECTOR_CI_COMPANY_M30O":"1"})
        wait_file(sway_log,MARKER,35,actor)
        wait_file(qt_log,"SD_COMPANY_M30L_MAP_READY real_maplibre=1",35,actor)
        wait_file(qt_log,"SD_COMPANY_M30L_FRAME_SWAPPED count=",15,actor)
        time.sleep(.8)
        after=screenshot("embedded-maplibre")
        compare_pixels(before,after,rgb)
        print("SD_COMPANY_M32E_ACTUAL_SCENE real_wlroots_apply=1 "
              "actual_Company_QML=1 real_MapLibre=1 "
              "test_authority=SYNTHETIC_CI production=OFF PASS",flush=True)

        # M32g: command the *real* Sway focus path against the native Qt
        # container. Merely reading the pure focus policy is insufficient:
        # prove the compositor intercepts an actual IPC focus request.
        def native_actor_node():
            tree=json.loads(call(str(swaymsg),"-s",ipc,"-r","-t","get_tree"))
            candidates=[n for n in nodes(tree)
                        if n.get("pid")==actor.pid and
                           n.get("shell")=="xdg_shell"]
            if len(candidates)!=1:
                raise RuntimeError(
                    "M32G_NATIVE_QT_TREE_NOT_UNIQUE "+
                    str([(n.get("id"),n.get("pid")) for n in candidates]))
            return candidates[0]
        native_node=native_actor_node()
        if native_node.get("focused"):
            raise RuntimeError("M32G_NATIVE_KEYBOARD_ALREADY_FOCUSED")
        before_focus_log=len(sway_log.read_text(errors="replace"))
        reply=json.loads(call(str(swaymsg),"-s",ipc,"-r",
                              f"[con_id={native_node['id']}] focus"))
        if not isinstance(reply,list) or not reply or not all(
                x.get("success") is True for x in reply):
            raise RuntimeError("M32G_IPC_FOCUS_COMMAND_REFUSED "+str(reply))
        wait_new_marker(
            sway_log,
            "[stage-director][COMPANY_M32G][CI_ONLY] "
            "native_ipc_keyboard_focus=DENIED actor_embedded=1",
            before_focus_log,12,actor)
        if native_actor_node().get("focused"):
            raise RuntimeError("M32G_NATIVE_KEYBOARD_FOCUS_LEAK")
        after_focus=screenshot("after-rejected-keyboard-focus")
        compare_pixels(before,after_focus,rgb)
        print("SD_COMPANY_M32G_NATIVE_KEYBOARD "
              "real_IPC_focus_attempt=1 compositor_refused=1 "
              "actor_keyboard_focused=0 scene_intact=1 PASS",flush=True)

        if env.get("STAGE_DIRECTOR_CI_COMPANY_INPUT")!="1":
            raise RuntimeError("BLOCKED_M32E_POINTER_REQUIRED_CI_GATE_MISSING")
        with (runtime/"sd-company-m32e-pointer.log").open("w") as ptrlog:
            pointer=subprocess.Popen([str(pointer_bin)],
                stdin=subprocess.PIPE,stdout=subprocess.PIPE,
                stderr=ptrlog,text=True,bufsize=1,env=env)
        assert pointer.stdout and pointer.stdin
        read_until(pointer.stdout,
                   "SD_CI_COMPANY_POINTER_READY real_zwlr_virtual_pointer=1")
        initial=qt_log.read_text(errors="replace")
        # Native Sway button-down operation must deliver the whole
        # in-map drag. Re-enter the known native-hit location for
        # wheel, which is a new, non-grab pointer action.
        for action in ("move 500 450","press","move 610 490",
                       "release","move 500 450","wheel"):
            pointer.stdin.write(action+"\n")
            pointer.stdin.flush()
            observed=read_until(pointer.stdout,"SD_CI_COMPANY_POINTER_ACK ",8)
            if observed!="SD_CI_COMPANY_POINTER_ACK "+action:
                raise RuntimeError("M32E_POINTER_ACK_INVALID "+observed)
            time.sleep(.15)
        # Clicking and dragging can trigger a Sway focus/arrange
        # transaction. Require the native map to remain in the same
        # original Company map slot afterward, not only before input.
        after_drag=screenshot("after-native-drag")
        compare_pixels(before,after_drag,rgb)
        print("SD_COMPANY_M32E_POST_DRAG_SCENE "
              "native_map_still_in_company_canvas=PASS",flush=True)
        diff=qt_log.read_text(errors="replace")[len(initial):]
        receipts=[m.group(2) for line in diff.splitlines()
                  if (m:=EVENT.search(line))]
        if not all(event in receipts for event in EVENT_NAMES):
            raise RuntimeError("M32E_ACTUAL_MAP_INPUT_MISSING "+str(receipts))
        print("SD_COMPANY_M32E_REAL_INPUT actual_qt_press_drag_release_wheel=PASS",
              flush=True)

        # Negative input boundary: a press entirely outside the current
        # map must not reach the native Qt actor even after a valid drag
        # established pointer focus. The coordinates are outside both
        # axes of Company's 166,203..1659,991 half-open map rectangle.
        outside_start=len(qt_log.read_text(errors="replace"))
        for action in ("move 1810 1040","press","release"):
            pointer.stdin.write(action+"\n")
            pointer.stdin.flush()
            observed=read_until(pointer.stdout,"SD_CI_COMPANY_POINTER_ACK ",8)
            if observed!="SD_CI_COMPANY_POINTER_ACK "+action:
                raise RuntimeError("M32F_OUTSIDE_POINTER_ACK_INVALID "+observed)
            time.sleep(.15)
        outside_log=qt_log.read_text(errors="replace")[outside_start:]
        outside_events=[m.group(2) for line in outside_log.splitlines()
                        if (m:=EVENT.search(line))]
        if any(event in ("press","release","wheel") for event in outside_events):
            raise RuntimeError("M32F_OUTSIDE_INPUT_LEAK "+str(outside_events))
        print("SD_COMPANY_M32F_OUTSIDE_POINTER native_click_leak=0 "
              "outside_map=1810,1040 PASS",flush=True)

        pointer.stdin.write("quit\n")
        pointer.stdin.flush()
        read_until(pointer.stdout,"SD_CI_COMPANY_POINTER_STOP ",8)
        pointer.wait(timeout=6)
        pointer=None
        actor.terminate()
        actor.wait(timeout=10)
        actor=None
        time.sleep(.8)
        final=screenshot("after-close")
        # Some QML glow animations vary. Require the former map pixels to
        # return to the original Company region at most sampled positions.
        restored=0
        x,y,w,h=CANVAS
        for py in range(y+70,y+h-70,42):
            for px in range(x+90,x+w-90,43):
                if sum(abs(a-b) for a,b in
                       zip(rgb(before,px,py),rgb(final,px,py)))<45:
                    restored+=1
        if restored<220:
            raise RuntimeError("M32E_NATIVE_MAP_NOT_CLEARED "+str(restored))
        print(f"SD_COMPANY_M32E_REAL_TEARDOWN restored_samples={restored} "
              "embedded_native_actor_hidden=PASS",flush=True)

        # M32h: change the active Sway workspace while native MapLibre
        # is still mapped in the original Company viewport. This is a real
        # compositor output-arrangement transition, not a pure policy test.
        workspaces=json.loads(call(str(swaymsg),"-s",ipc,
                                   "-r","-t","get_workspaces"))
        focused=[ws for ws in workspaces if ws.get("focused")]
        if len(focused)!=1 or not focused[0].get("name"):
            raise RuntimeError("M32H_ORIGINAL_WORKSPACE_NOT_UNIQUE")
        original_workspace=focused[0]["name"]
        second_map_log=len(sway_log.read_text(errors="replace"))
        with qt_log.open("a") as log:
            actor=subprocess.Popen([str(qt)],stdout=log,
                stderr=subprocess.STDOUT,
                env={**env,"QT_QPA_PLATFORM":"wayland",
                     "QT_WAYLAND_DISABLE_WINDOWDECORATION":"1",
                     "QSG_RHI_BACKEND":"opengl",
                     "LIBGL_ALWAYS_SOFTWARE":"1",
                     "STAGE_DIRECTOR_CI_COMPANY_M30L":"1",
                     "STAGE_DIRECTOR_CI_COMPANY_M30N":"1",
                     "STAGE_DIRECTOR_CI_COMPANY_M30O":"1"})
        wait_new_marker(sway_log,MARKER,second_map_log,35,actor)
        time.sleep(.6)
        compare_pixels(before,screenshot("before-workspace-loss"),rgb)
        before_switch=len(sway_log.read_text(errors="replace"))
        reply=json.loads(call(str(swaymsg),"-s",ipc,"-r",
                             "workspace SD-CI-Company-Other"))
        if not isinstance(reply,list) or not reply or not all(
                item.get("success") is True for item in reply):
            raise RuntimeError("M32H_WORKSPACE_SWITCH_FAILED "+str(reply))
        wait_new_marker(
            sway_log,
            "[stage-director][COMPANY_M32H][CI_ONLY] "
            "native_workspace_deactivated=1 scene_revoke=PASS",
            before_switch,20,actor)
        if actor.poll() is not None or company.poll() is not None:
            raise RuntimeError("M32H_ACTOR_OR_COMPANY_EXITED_ON_SWITCH")
        time.sleep(.5)
        workspace_image=screenshot("inactive-workspace")
        x,y,w,h=CANVAS
        cyan=gold=0
        for py in range(y+4,y+88,3):
            for px in range(x+16,x+w-16,3):
                rr,gg,bb=rgb(workspace_image,px,py)
                cyan+=int(gg>120 and bb>110 and 10*gg>13*rr)
                gold+=int(rr>130 and gg>70 and bb<150)
        if cyan>=65 or gold>=65:
            raise RuntimeError(
                f"M32H_WORKSPACE_ACTOR_LEAK cyan={cyan} gold={gold}")
        print("SD_COMPANY_M32H_WORKSPACE_LOSS "
              f"real_workspace_switch=1 native_actor_alive=1 "
              f"scene_hidden=1 cyan={cyan} gold={gold} PASS",flush=True)
        reply=json.loads(call(str(swaymsg),"-s",ipc,"-r",
                             "workspace "+original_workspace))
        if not isinstance(reply,list) or not reply or not all(
                item.get("success") is True for item in reply):
            raise RuntimeError("M32H_WORKSPACE_RETURN_FAILED "+str(reply))
        actor.terminate()
        actor.wait(timeout=10)
        actor=None
        time.sleep(.4)

        # M32f host-loss scenario: start another real native MapLibre actor
        # after the first was normally closed, but now terminate Company
        # WHILE the independent Qt process is still alive. The compositor
        # must revoke on the actual native Company layer unmap/destroy and
        # must not expose the former actor as a floating desktop window.
        prior_scene_log=len(sway_log.read_text(errors="replace"))
        with qt_log.open("a") as log:
            actor=subprocess.Popen([str(qt)],stdout=log,
                stderr=subprocess.STDOUT,
                env={**env,"QT_QPA_PLATFORM":"wayland",
                     "QT_WAYLAND_DISABLE_WINDOWDECORATION":"1",
                     "QSG_RHI_BACKEND":"opengl",
                     "LIBGL_ALWAYS_SOFTWARE":"1",
                     "STAGE_DIRECTOR_CI_COMPANY_M30L":"1",
                     "STAGE_DIRECTOR_CI_COMPANY_M30N":"1",
                     "STAGE_DIRECTOR_CI_COMPANY_M30O":"1"})
        wait_new_marker(sway_log,MARKER,prior_scene_log,35,actor)
        time.sleep(.8)
        before_host_loss=screenshot("before-host-loss")
        compare_pixels(before,before_host_loss,rgb)
        if actor.poll() is not None:
            raise RuntimeError("M32F_NATIVE_ACTOR_DIED_BEFORE_HOST_LOSS")
        prior_host_log=len(sway_log.read_text(errors="replace"))
        company.terminate()
        company.wait(timeout=10)
        company=None
        wait_new_marker(
            sway_log,
            "[stage-director][COMPANY_M32F][CI_ONLY] "
            "native_company_host_lost=1 scene_revoke=PASS",
            prior_host_log,20,actor)
        if actor.poll() is not None:
            raise RuntimeError("M32F_NATIVE_ACTOR_DIED_WITH_HOST")
        time.sleep(.8)
        lost_image=screenshot("after-host-loss")
        x,y,w,h=CANVAS
        cyan=gold=0
        for py in range(y+4,y+88,3):
            for px in range(x+16,x+w-16,3):
                rr,gg,bb=rgb(lost_image,px,py)
                cyan+=int(gg>120 and bb>110 and 10*gg>13*rr)
                gold+=int(rr>130 and gg>70 and bb<150)
        if cyan>=65 or gold>=65:
            raise RuntimeError(
                f"M32F_NATIVE_ACTOR_EXPOSED_AFTER_HOST_LOSS "
                f"cyan={cyan} gold={gold}")
        print(f"SD_COMPANY_M32F_HOST_LOSS "
              f"host_native_unmap=OBSERVED actor_still_running=1 "
              f"map_pixels_hidden=1 cyan={cyan} gold={gold} PASS",flush=True)
        actor.terminate()
        actor.wait(timeout=10)
        actor=None
    finally:
        if pointer is not None:
            if pointer.stdin:
                try: pointer.stdin.write("quit\n");pointer.stdin.flush()
                except (BrokenPipeError,OSError): pass
            try: pointer.wait(timeout=4)
            except subprocess.TimeoutExpired: pointer.kill();pointer.wait()
        if actor is not None:
            actor.terminate()
            try: actor.wait(timeout=8)
            except subprocess.TimeoutExpired: actor.kill();actor.wait()
        if company is not None:
            company.terminate()
            try: company.wait(timeout=8)
            except subprocess.TimeoutExpired: company.kill();company.wait()
        for name,path in (("company",company_log),("maplibre",qt_log),
                          ("sway",sway_log),
                          ("pointer",runtime/"sd-company-m32e-pointer.log")):
            if path.is_file():
                (temp/f"sd-company-m32e-{name}.log").write_text(
                    path.read_text(errors="replace")[-65000:],
                    encoding="utf-8")
    print("SD_COMPANY_M32E_REAL_WAYLAND isolated_pixels_input_teardown=PASS "
          "operator_Fedora_visual=NOT_TESTED production=OFF",flush=True)


if __name__=="__main__":
    try:
        main()
    except Exception as exc:
        print("SD_COMPANY_M32E_REAL_BLOCKED:",str(exc),file=sys.stderr,flush=True)
        raise SystemExit(1)
