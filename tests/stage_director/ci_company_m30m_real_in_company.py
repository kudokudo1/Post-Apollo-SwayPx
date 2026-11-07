#!/usr/bin/env python3
"""M30m: actual MapLibre Native Qt v3 inside real source-pinned Company QML.

CI-only dynamic compositor crop/input from M30k, genuine offline geographic
renderer from M30l. No user records, online requests, real production
scene/custody rights, local install or changes to original Company source.
Fullscreen intentionally hides undersized 840x540 real MapLibre source.
"""
from __future__ import annotations

import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time

import ci_company_real_quickshell as real
import ci_company_m30j_combined as m30j
from ci_company_overlay_prototype import call, nodes, ppm, rgb
from ci_company_input_containment import EVENT, read_until, near

QT_SOURCE = (840, 540)
CANVAS = {
    "standard": (626, 390, 573, 369),
    "compact": (806, 429, 441, 128),
    "fullscreen": (166, 203, 1493, 788),
}
EXPECTED_ORDER = ("standard", "compact", "fullscreen", "standard", "closed")
PROFILE_LOG = {
    "standard": "SD_CI_COMPANY_M30K_FOLLOW profile=1 crop=573x369 target=626,390",
    "compact": "SD_CI_COMPANY_M30K_FOLLOW profile=2 crop=441x128 target=806,429",
}
COMPANY_PRESS = re.compile(
    r"SD_COMPANY_M30J_COMPANY_FRAME_PRESS x=(-?\d+) y=(-?\d+)")


def wait_qml(logfile, index, timeout):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        lines = logfile.read_text(errors="replace").splitlines()
        matches = [m for line in lines
                   if (m := real.GEOMETRY.search(line))]
        if len(matches) > index:
            m = matches[index]
            expected = EXPECTED_ORDER[index]
            if m.group(1) != expected:
                raise RuntimeError(f"M30K_QML_ORDER_MISMATCH "
                                   f"index={index} got={m.group(1)} want={expected}")
            if expected == "closed":
                fields = m.groups()
                frame = tuple(map(int, fields[5:9]))
                canvas = tuple(map(int, fields[9:13]))
                candidate = tuple(map(int, fields[13:17]))
                if (fields[1] != "standard" or fields[2] != "0" or
                        fields[17] != "0" or
                        frame != (0, 0, 0, 0) or
                        canvas != (0, 0, 0, 0) or
                        candidate != (0, 0, 0, 0)):
                    raise RuntimeError("M30K_CLOSED_REAL_COMPANY_STALE " + str(fields))
                result = {"phase": "closed", "canvas": canvas,
                          "candidate": candidate, "origin": (460, 148)}
            else:
                result = real.check_geometry(m, expected)
            if expected != "closed":
                x, y, w, h = result["canvas"]
                ox, oy = result["origin"]
                actual = (ox + x, oy + y, w, h)
                if actual != CANVAS[expected]:
                    raise RuntimeError(f"M30K_QML_SOURCE_CHANGED "
                                       f"{expected} actual={actual} expected={CANVAS[expected]}")
            print(f"SD_COMPANY_M30K_REAL_QML phase={expected} "
                  f"canvas={result['canvas']} candidate=ZERO "
                  f"bridge=OFF source=PINNED_COMPANY_QML PASS", flush=True)
            return result
        time.sleep(0.08)
    raise RuntimeError(f"M30K_QML_PHASE_TIMEOUT index={index} " +
                       logfile.read_text(errors="replace")[-3800:])


def assert_no_unwanted_qt(events, size):
    for line in events:
        m = EVENT.search(line)
        if not m:
            continue
        (name, action, qgx, qgy, localx, localy,
         rootx, rooty, buttons, pid) = m.groups()
        if name != "crop":
            raise RuntimeError("M30K_QT_WRONG_TARGET " + line)
        if not (0 <= int(rootx) < size[0] + 6 and
                0 <= int(rooty) < size[1] + 6):
            raise RuntimeError("M30K_QT_INPUT_OUTSIDE_VISIBLE_CROP " + line)


def check_real_geographic_pixels(image, rect, phase):
    x, y, w, h = rect
    if w < 530 or h < 350:
        raise RuntimeError("M30M_MAP_PIXEL_REGION_NOT_STANDARD")
    cyan = blue = yellow = 0
    palette = set()
    # Actual offline synthetic GeoJSON centroid is source-local, entirely
    # inside the real Company's 573x369 standard crop.
    for py in range(y+170, y+348, 3):
        for px in range(x+280, x+530, 3):
            r,g,b = rgb(image,px,py)
            palette.add((r//12,g//12,b//12))
            cyan += int(g>125 and b>115 and g>1.25*max(1,r))
            blue += int(b>125 and b>1.2*max(1,g))
            yellow += int(r>160 and 80<g<220 and b<135)
    if len(palette)<8 or cyan<4 or blue<2 or yellow<1:
        raise RuntimeError(f"M30M_REAL_MAPLIBRE_GEO_NOT_VISIBLE phase={phase} "
                           f"palette={len(palette)} cyan={cyan} blue={blue} "
                           f"orange={yellow}")
    print(f"SD_COMPANY_M30M_REAL_GEO phase={phase} palette={len(palette)} "
          f"cyan={cyan} blue={blue} orange={yellow} "
          "real_company_crop=1 real_maplibre_geo=1 PASS",flush=True)


def source_band(image, rect):
    x,y,w,h=rect
    return [rgb(image,x+dx,y+dy)
            for dy in (7,28,45,75,95,110)
            for dx in (12,40,100,190,290,390,425)
            if dx<w-3 and dy<h-3]


def check_source_continuity(reference, image, rect, phase):
    observed=source_band(image,rect)
    matched=sum(m30j.pixel_distance(a,b)<=18
                for a,b in zip(reference,observed))
    if len(reference)!=len(observed) or matched<len(reference)*0.8:
        raise RuntimeError(f"M30M_NATIVE_SOURCE_NOT_CONTINUOUS phase={phase} "
                           f"matched={matched}/{len(reference)}")
    print(f"SD_COMPANY_M30M_SOURCE_CONTINUOUS phase={phase} "
          f"same_pixels={matched}/{len(reference)} PASS",flush=True)


def check_no_overflow(before, after, rect, phase):
    x,y,w,h=rect
    for px,py in ((x+w+8,y+min(h//2,75)),
                  (x+min(w//2,125),y+h+9),
                  (x-30,y+min(h//2,180)),
                  (x+min(w//2,300),y-25)):
        if not (0<=px<1920 and 0<=py<1080):
            continue
        a,b=rgb(before,px,py),rgb(after,px,py)
        if m30j.pixel_distance(a,b)>15:
            raise RuntimeError(f"M30M_NATIVE_OVERFLOW phase={phase} "
                               f"point={px},{py} before={a} after={b}")


def frame_presses(text):
    return [(int(a), int(b))
            for a, b in COMPANY_PRESS.findall(text)]


def main():
    env, temp, runtime, checkout, qs, swaymsg, grim, ipc = real.guard()
    flags = ("STAGE_DIRECTOR_CI_COMPANY_M30J",
             "STAGE_DIRECTOR_CI_COMPANY_M30K",
             "STAGE_DIRECTOR_CI_COMPANY_M30M",
             "STAGE_DIRECTOR_CI_COMPANY_CROP",
             "STAGE_DIRECTOR_CI_COMPANY_INPUT",
             "STAGE_DIRECTOR_CI_COMPANY_QUICKSHELL")
    if any(env.get(flag) != "1" for flag in flags):
        raise RuntimeError("BLOCKED_M30K_CI_FLAGS_MISSING")
    qt = Path(env.get("STAGE_DIRECTOR_M30L_NATIVE_MAPLIBRE", "/no-maplibre"))
    pointer_bin = Path(env.get("STAGE_DIRECTOR_TEST_COMPANY_POINTER", "/no-pointer"))
    if not all(str(p).startswith("/nix/store/") and p.is_file()
               for p in (qt, pointer_bin)):
        raise RuntimeError("BLOCKED_M30K_NATIVE_PROBE_NOT_PINNED")
    compat = Path(env.get("STAGE_DIRECTOR_M30I_QML_IMPORT_PATH", "/no-compat"))
    if not (str(compat).startswith("/nix/store/") and
            (compat / "Qt5Compat/GraphicalEffects/qmldir").is_file()):
        raise RuntimeError("BLOCKED_M30K_QT5COMPAT_NOT_PINNED")
    tmp_home = temp / "sd-company-m30m-home"
    for sub in ("", ".config", ".cache", ".local/share"):
        (tmp_home / sub).mkdir(parents=True, exist_ok=True, mode=0o700)
    qml_paths = str(compat) + (":" + env["QML_IMPORT_PATH"]
                               if env.get("QML_IMPORT_PATH") else "")
    qsenv = {**env, "HOME": str(tmp_home),
             "XDG_CONFIG_HOME": str(tmp_home / ".config"),
             "XDG_DATA_HOME": str(tmp_home / ".local/share"),
             "XDG_CACHE_HOME": str(tmp_home / ".cache"),
             "QML_IMPORT_PATH": qml_paths, "QML2_IMPORT_PATH": qml_paths,
             "QT_QPA_PLATFORM": "wayland", "NO_COLOR": "1"}
    shell = real.prepare(checkout, temp / "sd-company-m30m-copy")
    qml_source = Path("tests/stage_director/ci_company_m30m_real_shell.qml")
    if not qml_source.is_file() or qml_source.is_symlink():
        raise RuntimeError("BLOCKED_M30K_SHELL_MISSING")
    # Positive outside-pointer witness, inserted ONLY into the temporary
    # Company QML copy. No original Git checkout or production files touched.
    network = shell.parent / "widgets/CompanyNetworkW.qml"
    text = network.read_text(encoding="utf-8")
    anchor = "WindowPanelFrame {\n        id: companyFrame\n"
    if text.count(anchor) != 1:
        raise RuntimeError("M30K_COMPANY_FRAME_WITNESS_SOURCE_CHANGED")
    inject = (
        "        // M30k CI-only external-click observer.\n"
        "        MouseArea {\n"
        "            anchors.fill: parent\n"
        "            z: 9999\n"
        "            acceptedButtons: Qt.LeftButton\n"
        "            onPressed: function(mouse) {\n"
        '                console.log("SD_COMPANY_M30J_COMPANY_FRAME_PRESS x=" +\n'
        '                    Math.round(mouse.x) + " y=" + Math.round(mouse.y));\n'
        "                mouse.accepted = true;\n"
        "            }\n"
        "        }\n")
    network.write_text(text.replace(anchor, anchor + inject, 1),
                       encoding="utf-8")
    shell.write_text(qml_source.read_text(encoding="utf-8"), encoding="utf-8")
    qt_log = runtime / "sd-company-m30m-native-maplibre.log"
    company_log = runtime / "sd-company-m30m-company.log"
    company = None
    actor = None
    pointer = None

    def screenshot(phase):
        file = temp / f"sd-company-m30m-{phase}.ppm"
        call(str(grim), "-t", "ppm", str(file), timeout=25)
        return ppm(file)

    def tree():
        return json.loads(call(str(swaymsg), "-s", ipc, "-r", "-t", "get_tree"))

    def native_actor(pid, absent=False):
        for _ in range(85):
            v = [n for n in nodes(tree())
                 if n.get("name") == m30j.TITLE]
            if absent and not v:
                return None
            if len(v) == 1 and v[0].get("pid") == pid:
                if v[0].get("shell") != "xdg_shell":
                    raise RuntimeError("M30K_NOT_NATIVE_XDG")
                return v[0]
            time.sleep(0.08)
        raise RuntimeError("M30K_ACTOR_IDENTITY_OR_LIFETIME_MISMATCH")

    def sway_control(conid, instruction):
        result = json.loads(call(str(swaymsg), "-s", ipc, "-r",
                                 f"[con_id={conid}] {instruction}"))
        if not isinstance(result, list) or not result or not all(
                x.get("success") is True for x in result):
            raise RuntimeError(f"M30K_NATIVE_IPC_REFUSED {instruction} {result}")

    def send(cmd):
        if not pointer or not pointer.stdin or not pointer.stdout:
            raise RuntimeError("M30K_MISSING_REAL_POINTER")
        pointer.stdin.write(cmd + "\n")
        pointer.stdin.flush()
        ack = read_until(pointer.stdout, "SD_CI_COMPANY_POINTER_ACK ", 8)
        if ack != "SD_CI_COMPANY_POINTER_ACK " + cmd:
            raise RuntimeError("M30K_REAL_POINTER_ACK_WRONG " + ack)
        time.sleep(0.1)

    def log_checkpoint():
        return qt_log.read_text(errors="replace"), company_log.read_text(
            errors="replace")

    def test_input(phase, index):
        x, y, w, h = CANVAS[phase]
        start_qt, start_company = log_checkpoint()
        inside = (x + 120, y + 55)
        drag = (x + 220, y + 83)
        right = (x + w + 10, y + 73)
        bottom = (x + 90, y + h + 18)
        for cmd in (f"move {inside[0]} {inside[1]}", "press",
                    f"move {drag[0]} {drag[1]}", "release", "wheel"):
            send(cmd)
        qt_delta = qt_log.read_text(errors="replace")[len(start_qt):]
        hits = [EVENT.search(l) for l in qt_delta.splitlines()]
        hits = [h for h in hits if h]
        for name, at, held_required in (
                ("press", (120, 55), False),
                ("motion", (220, 83), True),
                ("release", (220, 83), False),
                ("wheel", (220, 83), False)):
            if not any(h.group(2) == name and
                       near(int(h.group(7)), int(h.group(8)), at, 7) and
                       (not held_required or int(h.group(9)) != 0)
                       for h in hits):
                raise RuntimeError(f"M30K_REAL_QT_EVENT_MISSING_{phase}_{name} "
                                   f"observed={[h.groups() for h in hits]}")
        assert_no_unwanted_qt(qt_delta.splitlines(), (w, h))
        before_outside_qt, before_outside_company = log_checkpoint()
        for cmd in (f"move {right[0]} {right[1]}", "press", "release",
                    f"move {bottom[0]} {bottom[1]}", "press", "release"):
            send(cmd)
        after_qt, after_company = log_checkpoint()
        new_qt = after_qt[len(before_outside_qt):]
        if any(EVENT.search(l) for l in new_qt.splitlines()):
            raise RuntimeError(f"M30K_HIDDEN_NATIVE_INPUT_LEAK_{phase} " +
                               new_qt[-1600:])
        # Qt outside area is still within 840x540 source but must
        # positively reach the actual Company QML behind it.
        frame_origin = (real.EXPECTED[phase]["origin"][0] + 12,
                        real.EXPECTED[phase]["origin"][1] + 9)
        outside = frame_presses(after_company[len(before_outside_company):])
        for pt in (right, bottom):
            if not any(near(frame_origin[0] + px,
                            frame_origin[1] + py, pt, 11)
                       for px, py in outside):
                raise RuntimeError(f"M30K_COMPANY_OUTSIDE_NOT_RECEIVED_{phase} "
                                   f"expected={pt} actual={outside}")
        print(f"SD_COMPANY_M30K_POINTER phase={phase} "
              f"real_native_qt_drag_wheel=1 real_company_outside_presses=2 "
              "hidden_native_qt_events=0 PASS", flush=True)

    try:
        with company_log.open("w") as log:
            company = subprocess.Popen(
                [str(qs), "--no-color", "-p", str(shell)], stdout=log,
                stderr=subprocess.STDOUT, env=qsenv)
        if company.poll() is not None:
            raise RuntimeError("M30K_QUICKSHELL_EXIT_EARLY")
        first = wait_qml(company_log, 0, 25)
        original = screenshot("standard-before")
        with qt_log.open("w") as log:
            actor = subprocess.Popen(
                [str(qt)], stdout=log,
                stderr=subprocess.STDOUT,
                env={**env, "QT_QPA_PLATFORM": "wayland",
                     "QT_WAYLAND_DISABLE_WINDOWDECORATION": "1",
                     "QSG_RHI_BACKEND": "opengl",
                     "LIBGL_ALWAYS_SOFTWARE": "1",
                     "STAGE_DIRECTOR_CI_COMPANY_M30L": "1"})
        marker = f"SD_CI_COMPANY_QT_QPA_READY case=crop pid={actor.pid}"
        for _ in range(250):
            if actor.poll() is not None:
                raise RuntimeError("M30K_QT_FAILED " +
                                   qt_log.read_text(errors="replace")[-2500:])
            if marker in qt_log.read_text(errors="replace"):
                break
            time.sleep(0.08)
        else:
            raise RuntimeError("M30M_REAL_MAPLIBRE_QPA_NOT_READY")
        ready_deadline = time.monotonic() + 18
        while time.monotonic() < ready_deadline:
            log = qt_log.read_text(errors="replace")
            if "SD_COMPANY_M30L_MAP_READY real_maplibre=0" in log:
                raise RuntimeError("M30M_REAL_MAPLIBRE_UNAVAILABLE "+log[-4000:])
            if ("SD_COMPANY_M30L_MAP_READY real_maplibre=1" in log
                and "SD_COMPANY_M30L_FRAME_SWAPPED count=" in log
                and "MAPLIBRE_COMPAT_PROBE" in log
                and "mode offline renderer OpenGLRhi" in log):
                break
            if actor.poll() is not None:
                raise RuntimeError("M30M_MAPLIBRE_EXITED "+log[-4000:])
            time.sleep(0.12)
        else:
            raise RuntimeError("M30M_REAL_MAPLIBRE_FRAME_NOT_READY "+
                               qt_log.read_text(errors="replace")[-4000:])
        mapped = native_actor(actor.pid)
        # Kernel-held identity, not a window-title match alone.
        fd = os.pidfd_open(actor.pid, 0)
        try:
            if f"Pid:\t{actor.pid}" not in Path(
                    f"/proc/self/fdinfo/{fd}").read_text():
                raise RuntimeError("M30K_NATIVE_PIDFD_MISMATCH")
        finally:
            os.close(fd)
        wid = mapped["id"]
        for cmd in ("floating enable", "border none",
                    "resize set width 840 px height 540 px",
                    "move position 626 390"):
            sway_control(wid, cmd)
        # Real MapLibre may emit a Wayland configure/frame after Sway
        # accepts floating+resize. Do NOT weaken the compositor's exact
        # source 840x540 condition; wait boundedly for its real FOLLOW log.
        sway_log = (runtime / "sway.log")
        geometry_deadline = time.monotonic() + 14
        while time.monotonic() < geometry_deadline:
            trace = sway_log.read_text(errors="replace")
            if PROFILE_LOG["standard"] in trace:
                break
            if actor.poll() is not None:
                raise RuntimeError("M30M_NATIVE_DIED_BEFORE_EXACT_SOURCE " +
                                   qt_log.read_text(errors="replace")[-3500:])
            time.sleep(0.13)
        else:
            actual_rect = native_actor(actor.pid).get("rect", {})
            raise RuntimeError("M30M_REAL_MAPLIBRE_SOURCE_NOT_840x540 " +
                               f"sway_rect={actual_rect} " +
                               "compositor=" +
                               sway_log.read_text(errors="replace")[-6500:])
        assert_no_unwanted_qt([], CANVAS["standard"][2:])
        time.sleep(1.5)
        standard_native = screenshot("standard-native")
        check_real_geographic_pixels(standard_native,CANVAS["standard"],"standard")
        check_no_overflow(original,standard_native,CANVAS["standard"],"standard")
        reference_band=source_band(standard_native,CANVAS["standard"])
        pointer = subprocess.Popen(
            [str(pointer_bin)], stdin=subprocess.PIPE,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            text=True, bufsize=1, env=env)
        assert pointer.stdout
        read_until(pointer.stdout,
                   "SD_CI_COMPANY_POINTER_READY real_zwlr_virtual_pointer=1")
        test_input("standard", 0)
        qt_pid = actor.pid
        qt_xdg = wid

        compact = wait_qml(company_log, 1, 26)
        time.sleep(0.25)
        if PROFILE_LOG["compact"] not in sway_log.read_text(errors="replace"):
            raise RuntimeError("M30K_COMPOSITOR_COMPACT_PROFILE_NOT_MATCHED " +
                               sway_log.read_text(errors="replace")[-2500:])
        if native_actor(qt_pid)["id"] != qt_xdg:
            raise RuntimeError("M30K_NATIVE_CLIENT_REPLACED_DURING_RESIZE")
        compact_native=screenshot("compact-native")
        check_source_continuity(reference_band,compact_native,
                                CANVAS["compact"],"compact")
        test_input("compact", 1)

        full = wait_qml(company_log, 2, 26)
        time.sleep(0.3)
        if "SD_CI_COMPANY_M30K_FULLSCREEN source_smaller_than_canvas=1 " \
                not in sway_log.read_text(errors="replace"):
            raise RuntimeError("M30K_UNDERSIZED_FULLSCREEN_NOT_REFUSED")
        before_qt, before_company = log_checkpoint()
        fullscreen_img = screenshot("fullscreen-hidden")
        old_x, old_y, _, _ = CANVAS["standard"]
        # Compare only actual colored geographic source pixels, not
        # uniform dark background. The real Company fullscreen map behind
        # a hidden actor can legitimately share that background color.
        # M30k's compositor hide/input refusal remains mandatory.
        geographic = []
        for py in range(old_y+170, old_y+348, 3):
            for px in range(old_x+280, old_x+530, 3):
                r,g,b = rgb(standard_native,px,py)
                colored = (g>125 and b>115 and g>1.25*max(1,r)) or (
                    b>125 and b>1.2*max(1,g)) or (
                    r>160 and 80<g<220 and b<135)
                if colored:
                    geographic.append((px,py))
        if len(geographic)<20:
            raise RuntimeError("M30M_FULLSCREEN_NO_GEO_PIXEL_PROVENANCE")
        changed=sum(m30j.pixel_distance(
            rgb(standard_native,px,py),
            rgb(fullscreen_img,px,py))>18 for px,py in geographic)
        if changed < len(geographic)*0.9:
            raise RuntimeError(f"M30M_FULLSCREEN_NATIVE_PIXEL_LEAK "
                               f"changed={changed}/{len(geographic)}")
        print(f"SD_COMPANY_M30M_FULLSCREEN_HIDDEN_PIXELS "
              f"changed_colored_geography={changed}/{len(geographic)} "
              "real_maplibre_source_absent=1 PASS",flush=True)
        send("move 750 490")
        send("press")
        send("release")
        now_qt, now_company = log_checkpoint()
        if any(EVENT.search(l) for l in
               now_qt[len(before_qt):].splitlines()):
            raise RuntimeError("M30K_FULLSCREEN_INPUT_REACHED_HIDDEN_QT")
        full_frame_origin = (12, 9)
        received = frame_presses(now_company[len(before_company):])
        if not any(near(full_frame_origin[0] + x,
                        full_frame_origin[1] + y, (750, 490), 11)
                   for x, y in received):
            raise RuntimeError("M30K_FULLSCREEN_CLICK_NOT_RECEIVED_BY_COMPANY")
        if native_actor(qt_pid)["id"] != qt_xdg:
            raise RuntimeError("M30K_NATIVE_ACTOR_DIED_WHEN_HIDDEN")
        print("SD_COMPANY_M30K_FULLSCREEN "
              "real_canvas=1493x788 native_source=840x540 "
              "actor_hidden=1 qt_input=0 company_receives_click=1 PASS",
              flush=True)

        return_std = wait_qml(company_log, 3, 26)
        time.sleep(0.25)
        if native_actor(qt_pid)["id"] != qt_xdg:
            raise RuntimeError("M30K_NATIVE_ROOT_CHANGED_ON_RETURN")
        if sway_log.read_text(errors="replace").count(
                PROFILE_LOG["standard"]) < 2:
            raise RuntimeError(
                "M30K_STD_RETURN_NOT_RESTORED_BY_COMPOSITOR "
                "recent_compositor=" +
                sway_log.read_text(errors="replace")[-5500:])
        returned=screenshot("standard-return-native")
        check_source_continuity(reference_band,returned,
                                CANVAS["standard"],"standard-return")
        check_no_overflow(original,returned,CANVAS["standard"],"return")
        test_input("standard", 3)
        print("SD_COMPANY_M30K_SAME_ACTOR pidfd_verified=1 "
              "standard_to_compact_to_fullscreen_hidden_to_standard=PASS",
              flush=True)

        pointer.stdin.write("quit\n")
        pointer.stdin.flush()
        stop = read_until(pointer.stdout, "SD_CI_COMPANY_POINTER_STOP ", 8)
        pointer.wait(timeout=5)
        if "PASS" not in stop or pointer.returncode != 0:
            raise RuntimeError("M30K_REAL_POINTER_CLEANUP_FAILED")
        actor.terminate()
        actor.wait(timeout=8)
        native_actor(qt_pid, absent=True)
        time.sleep(0.25)
        after_close = screenshot("native-closed")
        m30j.check_restored(original, after_close,
                            CANVAS["standard"][:2])
        final = wait_qml(company_log, 4, 26)
        screenshot("company-closed")
        print("SD_COMPANY_M30M_REAL_MAP_IN_COMPANY "
              "real_original_company=1 original_maplibre_v3=1 "
              "offline_geographic_pixels=1 real_qt_pointer=1 "
              "standard_compact_fullscreen_hide_return=PASS "
              "same_native_pidfd_xdg=1 no_overflow=1 "
              "production_scene_input=OFF PASS",flush=True)
    finally:
        # Private compositor runtime is deleted after this test; retain
        # bounded, synthetic-only source logs before teardown for diagnosis.
        for label, source in (("maplibre", qt_log), ("company", company_log),
                              ("sway", runtime / "sway.log")):
            if source.is_file():
                try:
                    (temp / f"sd-company-m30m-{label}.log").write_bytes(
                        source.read_bytes()[-65536:])
                except OSError as ex:
                    print("SD_COMPANY_M30M_LOG_ARCHIVE_FAILED",
                          label, type(ex).__name__, file=sys.stderr, flush=True)
        for p in (pointer, actor, company):
            if p is not None and p.poll() is None:
                p.terminate()
        for p in (pointer, actor, company):
            if p is None:
                continue
            try:
                p.wait(timeout=7)
            except subprocess.TimeoutExpired:
                p.kill()
                p.wait(timeout=7)


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print("SD_COMPANY_M30M_BLOCKED:", exc, file=sys.stderr, flush=True)
        raise SystemExit(1)
