#!/usr/bin/env python3
"""CI ONLY: map a REAL Chrome xdg_toplevel in isolated headless SwayPX.

Use the EXISTING host Chrome (never --no-sandbox), fixed about:blank,
transient systemd control-group. Compare the Sway-native mapped Wayland
client PID to the independently held Linux cgroup and kernel starttime.
No production admission, scene mutation, browser clip, focus or QML.
"""
from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import sys
import time
from assert_news_render_pixels import parse_ppm


def call(*args: str, check: bool = True, timeout: int = 20) -> str:
    out = subprocess.run(args, capture_output=True, text=True,
                         timeout=timeout, check=False)
    if check and out.returncode:
        raise RuntimeError(f"command {args[:3]} rc={out.returncode}\n"
                           + out.stderr[-2500:] + out.stdout[-1000:])
    return out.stdout.strip()


def proc_start(pid: int) -> int | None:
    try:
        return int(Path(f"/proc/{pid}/stat").read_text()
                   .rsplit(")", 1)[1].split()[19])
    except (OSError, ValueError, IndexError):
        return None


def members(root: Path) -> set[int]:
    pids: set[int] = set()
    if root.is_dir():
        for f in root.rglob("cgroup.procs"):
            try:
                pids.update(int(x) for x in f.read_text().split())
            except (OSError, ValueError):
                pass
    return pids


def service_task_pressure(root):
    """Read the kernel cgroup task count and cap, never infer from process count."""
    stats = {}
    for key in ("pids.current", "pids.max", "pids.events"):
        try:
            stats[key] = (root / key).read_text().strip()
        except OSError:
            stats[key] = "UNAVAILABLE"
    return stats


def assert_chrome_main_rect_stable(e, temp, swaymsg, ipc, root,
                                   actor, snapshot):
    """Bounded post-startup stabilization before touching the scene mask.

    Exact XDG/PID/rect remains required; never silently reselect a window.
    """
    initial_id, initial_pid = actor["id"], actor["pid"]
    initial_rect = actor.get("rect")
    initial_start = snapshot[initial_pid]
    for sample in range(16):
        tree = json.loads(call(swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
        matches = [n for n in containers(tree)
                   if n.get("id") == initial_id and
                   n.get("pid") == initial_pid]
        if (len(matches) != 1 or initial_pid not in members(root) or
                proc_start(initial_pid) != initial_start):
            raise RuntimeError(
                "FAIL_CHROME_ACTOR_DISAPPEARED_WHILE_SETTLING "
                "tasks=" + json.dumps(service_task_pressure(root)))
        current_rect = matches[0].get("rect")
        if current_rect != initial_rect:
            (temp / "sd-ci-chrome-geometry-changed-tree.json").write_text(
                json.dumps(tree), encoding="utf-8")
            raise RuntimeError(
                "FAIL_CHROME_NATIVE_RECT_CHANGED_BEFORE_MASK "
                f"sample={sample} initial={initial_rect} "
                f"current={current_rect} "
                "tasks=" + json.dumps(service_task_pressure(root)))
        time.sleep(0.15)
    print("SD_CI_CHROME_NATIVE_RECT_SETTLED "
          f"native_id={initial_id} pid={initial_pid} "
          "samples=16 same_geometry=1 same_starttime=1 "
          "authority=NONE result=PASS", flush=True)


def containers(node):
    if isinstance(node, dict):
        if node.get("pid") and node.get("type") == "con":
            yield node
        for sub in node.get("nodes", []):
            yield from containers(sub)
        for sub in node.get("floating_nodes", []):
            yield from containers(sub)


def native_xdg_toplevels(node):
    """Include Sway's floating_con roots, not just tiled 'con' nodes."""
    if isinstance(node, dict):
        if (node.get("type") in ("con", "floating_con") and
                node.get("shell") == "xdg_shell" and node.get("pid")):
            yield node
        for kind in ("nodes", "floating_nodes"):
            for child in node.get(kind, []):
                yield from native_xdg_toplevels(child)


def read_unrelated_startup_tree(swaymsg, ipc):
    """Bounded startup-only IPC recovery, before Chrome or mask mutation.

    Do not tolerate failures in any later Chrome focus/scene verification.
    A missed response is recorded, not silently converted into a PASS.
    """
    for attempt in range(1, 5):
        out = subprocess.run(
            [swaymsg, "-s", ipc, "-r", "-t", "get_tree"],
            text=True, capture_output=True, timeout=20, check=False)
        if out.returncode == 0:
            try:
                tree = json.loads(out.stdout)
                if isinstance(tree, dict):
                    if attempt > 1:
                        print("SD_CI_EARLY_IPC_RECOVERED "
                              f"attempt={attempt} bounded=4 "
                              "pre_chrome=1 authority=NONE", flush=True)
                    return tree
            except json.JSONDecodeError:
                pass
            raise RuntimeError("FAIL_EARLY_IPC_INVALID_TREE_RESPONSE")
        if "Unable to receive IPC response" not in out.stderr:
            raise RuntimeError("FAIL_EARLY_IPC_UNEXPECTED_RESPONSE " +
                               out.stderr[-1400:])
        print("SD_CI_EARLY_IPC_RESPONSE_MISSED "
              f"attempt={attempt} bounded=4 pre_chrome=1",
              flush=True)
        if attempt < 4:
            time.sleep(0.3)
    raise RuntimeError("FAIL_EARLY_IPC_RECOVERY_EXHAUSTED attempts=4")


def verify_unrelated_titlebar(swaymsg: str, ipc: str, foot: str) -> None:
    """One ordinary native Wayland control proves borders aren't disabled globally."""
    if not foot.startswith("/nix/store/") or not Path(foot).is_file():
        raise RuntimeError("BLOCKED_MISSING_PINNED_UNRELATED_WAYLAND_CONTROL")
    actor = subprocess.Popen(
        [foot, "--app-id=sd-ci-unrelated-titlebar-control",
         "--title=SD1-CI-UNRELATED-TITLEBAR-KEPT",
         "--window-size-pixels=720x480", "/usr/bin/sleep", "15"],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
        start_new_session=False)
    try:
        control = None
        for _ in range(100):
            if actor.poll() is not None:
                raise RuntimeError("FAIL_UNRELATED_CONTROL_EXITED_BEFORE_MAP")
            tree = read_unrelated_startup_tree(swaymsg, ipc)
            matches = [n for n in containers(tree)
                       if n.get("pid") == actor.pid and
                       n.get("shell") == "xdg_shell" and
                       n.get("name") == "SD1-CI-UNRELATED-TITLEBAR-KEPT"]
            if len(matches) == 1:
                control = matches[0]
                break
            time.sleep(0.1)
        if control is None:
            raise RuntimeError("FAIL_UNRELATED_NATIVE_WINDOW_NOT_MAPPED")
        if control.get("border") != "normal":
            raise RuntimeError("FAIL_UNRELATED_WINDOW_DECORATION_CHANGED: "
                               + str(control.get("border")))
        print("SD_CI_UNRELATED_DECORATION: native_xdg=1 "
              "normal_border=1 titlebar_policy_preserved=1 "
              "not_browser_cgroup=1 result=PASS", flush=True)
    finally:
        actor.terminate()
        try:
            actor.wait(timeout=3)
        except subprocess.TimeoutExpired:
            actor.kill()
            actor.wait(timeout=3)
    for _ in range(100):
        tree = read_unrelated_startup_tree(swaymsg, ipc)
        if not any(n.get("pid") == actor.pid for n in containers(tree)):
            return
        time.sleep(0.05)
    raise RuntimeError("FAIL_UNRELATED_CONTROL_STILL_MAPPED_AFTER_STOP")



def verify_pending_mask_exit(e, temp, runtime, swaymsg, ipc, grim,
                             unit, root, snapshot, client_pid, profile):
    """CI-only held viewport request; SIGKILL before any mask mutation.

    The compositor acknowledges receipt without applying a transition.
    The exact owned Chrome unit is the *only* process this harness kills.
    """
    if (e.get("GITHUB_ACTIONS") != "true" or e.get("CI") != "true" or
            e.get("STAGE_DIRECTOR_CI_BROWSER_MASK") != "1" or
            e.get("STAGE_DIRECTOR_CI_CHROME_PENDING_EXIT") != "1" or
            e.get("STAGE_DIRECTOR_CI_CHROME_ABRUPT_EXIT") == "1"):
        raise RuntimeError("BLOCKED_PENDING_EXIT_OUTSIDE_EXACT_CI_MODE")
    request = temp / (
        "sd-chrome-mask-" + e["GITHUB_RUN_ID"] + "-" +
        e.get("GITHUB_RUN_ATTEMPT", "1") + ".request")
    log_path = runtime / "sway.log"
    marker_start = len(log_path.read_text(encoding="utf-8", errors="replace"))
    fd = os.open(request,
                 os.O_CREAT | os.O_EXCL | os.O_WRONLY |
                 getattr(os, "O_NOFOLLOW", 0), 0o600)
    try:
        os.write(fd, b"PENDING\n")
    finally:
        os.close(fd)
    try:
        acknowledged = False
        for _ in range(100):
            new_log = log_path.read_text(
                encoding="utf-8", errors="replace")[marker_start:]
            if "SD_CI_CHROME_MASK_PENDING_HELD" in new_log:
                acknowledged = True
                break
            if ("SD_CI_CHROME_MASK_PENDING_INVALID" in new_log or
                    "SD_CI_CHROME_MASK_SOURCE_LOST" in new_log):
                break
            time.sleep(0.1)
        if not acknowledged:
            raise RuntimeError("FAIL_PENDING_NOT_ACKNOWLEDGED " +
                               new_log[-3000:])
        if "SD_CI_CHROME_MASK_REBIND" in new_log:
            raise RuntimeError("FAIL_PENDING_MASK_ALREADY_APPLIED")
        if (proc_start(client_pid) != snapshot[client_pid] or
                client_pid not in members(root)):
            raise RuntimeError("FAIL_PENDING_BROWSER_IDENTITY_LOST_EARLY")
        # Sample while the compositor event loop is demonstrably holding
        # the queued request. Its first crop must still be visible.
        time.sleep(0.35)
        held_ppm = temp / "sd-chrome-pending-held.ppm"
        held_tree = temp / "sd-chrome-pending-held-tree.json"
        call(grim, "-t", "ppm", str(held_ppm), timeout=25)
        held_tree.write_text(json.dumps(json.loads(call(
            swaymsg, "-s", ipc, "-r", "-t", "get_tree"))), encoding="utf-8")

        call("sudo", "-n", "systemctl", "kill",
             "--signal=SIGKILL", "--kill-whom=all", unit, timeout=20)
        print("SD_CI_CHROME_PENDING_SIGNAL: exact_test_service=1 "
              "signal=SIGKILL held_unapplied=1 source=CI_only", flush=True)
        revoked = False
        for _ in range(100):
            last_log = log_path.read_text(
                encoding="utf-8", errors="replace")[marker_start:]
            clean_tree = json.loads(call(
                swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
            still_mapped = any(
                node.get("pid") == client_pid for node in containers(clean_tree))
            still_alive = any(
                proc_start(pid) == ticks for pid, ticks in snapshot.items())
            still_group = bool(members(root))
            revoked = "SD_CI_CHROME_MASK_PENDING_REVOKED" in last_log
            restored = ("SD_CI_RENDER_PIXEL clip_restored_on_unmap=1"
                        in last_log)
            if (revoked and restored and not still_mapped and
                    not still_alive and not still_group):
                break
            time.sleep(0.1)
        else:
            raise RuntimeError(
                "FAIL_PENDING_CHROME_CUSTODY_NOT_REVOKED "
                f"unmap={not still_mapped} alive={still_alive} "
                f"cgroup={still_group} revoked={revoked} restored={restored} "
                + last_log[-3000:])
        # Deliberately leave the original pending request on disk as the
        # timer would have polled again; it must now be removed on unmap.
        time.sleep(0.35)
        last_log = log_path.read_text(
            encoding="utf-8", errors="replace")[marker_start:]
        if "SD_CI_CHROME_MASK_REBIND" in last_log:
            raise RuntimeError("FAIL_PENDING_MASK_APPLIED_AFTER_NATIVE_UNMAP")
        after_ppm = temp / "sd-chrome-pending-after-kill.ppm"
        after_png = temp / "sd-chrome-pending-after-kill.png"
        after_tree = temp / "sd-chrome-pending-after-tree.json"
        call(grim, "-t", "ppm", str(after_ppm), timeout=25)
        call(grim, "-t", "png", str(after_png), timeout=25)
        after_tree.write_text(json.dumps(clean_tree), encoding="utf-8")
        result = call(
            sys.executable,
            "tests/stage_director/assert_real_chrome_pending_exit_pixels.py",
            str(temp / "sd-chrome-before.ppm"), str(held_ppm),
            str(after_ppm), str(held_tree), str(after_tree),
            str(client_pid), timeout=30)
        print(result, flush=True)
        if "SD_CI_CHROME_PENDING_EXIT_PIXELS: result=PASS" not in result:
            raise RuntimeError("FAIL_PENDING_SCREENSHOT_MARKER_MISSING")
        call("sudo", "-n", "systemctl", "stop", unit, check=False)
        for _ in range(100):
            if (not profile.parent.exists() and not members(root) and
                    all(proc_start(pid) != ticks
                        for pid, ticks in snapshot.items())):
                break
            time.sleep(0.1)
        else:
            raise RuntimeError("FAIL_PENDING_PRIVATE_BROWSER_PROFILE_NOT_REMOVED")
        print("SD_CI_CHROME_PENDING_REQUEST_ABORT: result=PASS "
              "request_held_not_applied=1 native_unmap=1 "
              "timer_revoked=1 process_cgroup_cleared=1 "
              "post_kill_pixels_clear=1 authority=NONE", flush=True)
    finally:
        request.unlink(missing_ok=True)



def native_popup_pointer_target(ppm):
    """Select a real rendered popup pixel instead of guessing its geometry."""
    width, height, rgb = parse_ppm(ppm)
    if (width, height) != (1920, 1080):
        raise RuntimeError("FAIL_NATIVE_POPUP_POINTER_SCREENSHOT_DIMENSIONS")
    hits = []
    for i in range(0, len(rgb), 3):
        red, green, blue = rgb[i], rgb[i + 1], rgb[i + 2]
        if (red >= 150 and blue >= 150 and green <= 125 and
                min(red, blue) - green >= 70):
            pixel = i // 3
            hits.append((pixel % width, pixel // width))
    if len(hits) < 800:
        raise RuntimeError("FAIL_NATIVE_POPUP_POINTER_NO_PAINTED_TARGET")
    return hits[len(hits) // 2]


def verify_native_popup_witness(e, temp, runtime, grim, unit, snapshot,
                                root, crop_ppm):
    """Real xdg_popup positive/negative control in one CI compositor.

    CONTROL: unrelated runner cgroup can paint magenta through global popup.
    DENIAL: same witness in exact Chrome service cgroup cannot paint magenta.
    This does NOT mean the actual Chrome executable emitted an XDG popup.
    """
    witness = e.get("STAGE_DIRECTOR_TEST_XDG_POPUP_WITNESS", "")
    if (e.get("CI") != "true" or e.get("GITHUB_ACTIONS") != "true" or
            e.get("STAGE_DIRECTOR_CI_CHROME_POPUP_WITNESS") != "1" or
            not witness.startswith("/nix/store/") or
            not Path(witness).is_file()):
        raise RuntimeError("BLOCKED_NATIVE_POPUP_WITNESS_OUTSIDE_CI")
    injector = e.get("STAGE_DIRECTOR_TEST_VIRTUAL_INPUT", "")
    if (e.get("STAGE_DIRECTOR_CI_CHROME_REAL_INPUT") != "1" or
            not injector.startswith("/nix/store/") or
            not Path(injector).is_file()):
        raise RuntimeError("BLOCKED_NATIVE_POPUP_INPUT_WITNESS_UNAVAILABLE")
    control_log = temp / "sd-ci-native-popup-control.log"
    pointer_xy = None
    with control_log.open("w", encoding="utf-8") as output:
        control = subprocess.Popen(
            [witness, "emit"], stdout=output, stderr=subprocess.STDOUT)
        try:
            for _ in range(100):
                report = control_log.read_text(encoding="utf-8")
                if "SD_CI_NATIVE_XDG_POPUP_BUFFER_ATTACHED" in report:
                    break
                if control.poll() is not None:
                    raise RuntimeError(
                        "FAIL_NATIVE_POPUP_POSITIVE_CONTROL_EXITED " + report)
                time.sleep(0.1)
            else:
                raise RuntimeError(
                    "FAIL_NATIVE_POPUP_POSITIVE_CONTROL_UNMAPPED " +
                    control_log.read_text(encoding="utf-8"))
            time.sleep(0.3)
            positive = temp / "sd-ci-native-popup-positive.ppm"
            call(grim, "-t", "ppm", str(positive), timeout=25)
            call(grim, "-t", "png",
                 str(temp / "sd-ci-native-popup-positive.png"), timeout=25)
            pointer_xy = native_popup_pointer_target(positive)
            compositor_before = len((runtime / "sway.log").read_text(
                encoding="utf-8", errors="replace"))
            call(injector, "button", str(pointer_xy[0]), str(pointer_xy[1]),
                 timeout=20)
            expected = (
                "SD_CI_NATIVE_XDG_POPUP_INPUT kind=ENTER owner=POPUP",
                "SD_CI_NATIVE_XDG_POPUP_INPUT kind=BUTTON state=PRESS owner=POPUP",
                "SD_CI_NATIVE_XDG_POPUP_INPUT kind=BUTTON state=RELEASE owner=POPUP",
            )
            for _ in range(65):
                receipt = control_log.read_text(encoding="utf-8")
                if all(marker in receipt for marker in expected):
                    break
                if control.poll() is not None:
                    raise RuntimeError(
                        "FAIL_NATIVE_POPUP_POSITIVE_INPUT_CLIENT_EXITED " + receipt)
                time.sleep(0.1)
            else:
                raise RuntimeError("FAIL_NATIVE_POPUP_POSITIVE_INPUT_RECEIPTS " +
                                   control_log.read_text(encoding="utf-8"))
            event_log = (runtime / "sway.log").read_text(
                encoding="utf-8", errors="replace")[compositor_before:]
            if ("SD_CI_NATIVE_INPUT_DELIVERED kind=POINTER target=OTHER" not in
                    event_log or
                    "SD_CI_NATIVE_BUTTON_DELIVERED state=PRESS target=OTHER" not in
                    event_log or
                    "SD_CI_NATIVE_BUTTON_DELIVERED state=RELEASE target=OTHER" not in
                    event_log or "target=HIDDEN_SIBLING" in event_log):
                raise RuntimeError("FAIL_NATIVE_POPUP_COMPOSITOR_INPUT_ROOT " +
                                   event_log[-3800:])
            print("SD_CI_NATIVE_POPUP_POSITIVE_INPUT: result=PASS "
                  "native_popup_pointer_enter=1 click_press_release=1 "
                  "compositor_root=OTHER authority=NONE", flush=True)
            print("SD_CI_NATIVE_POPUP_CONTROL_PROTOCOL_LOG:\n" +
                  control_log.read_text(encoding="utf-8"), flush=True)
        finally:
            if control.poll() is None:
                control.terminate()
            try:
                control.wait(timeout=3)
            except subprocess.TimeoutExpired:
                control.kill()
                control.wait(timeout=3)
    time.sleep(0.35)
    request = temp / ("sd-ci-xdg-popup-" + e["GITHUB_RUN_ID"] + "-" +
                      e.get("GITHUB_RUN_ATTEMPT", "1") + ".request")
    marker_start = len((runtime / "sway.log").read_text(
        encoding="utf-8", errors="replace"))
    fd = os.open(request, os.O_CREAT | os.O_EXCL | os.O_WRONLY |
                 getattr(os, "O_NOFOLLOW", 0), 0o600)
    try:
        os.write(fd, b"POPUP\n")
    finally:
        os.close(fd)
    try:
        observed = False
        for _ in range(120):
            new_log = (runtime / "sway.log").read_text(
                encoding="utf-8", errors="replace")[marker_start:]
            if ("SD_CI_CHROME_POPUP_DENIED" in new_log and
                    "current_cgroup=1" in new_log):
                observed = True
                break
            if not members(root):
                break
            time.sleep(0.1)
        if not observed:
            raise RuntimeError("FAIL_NATIVE_POPUP_GLOBAL_DENIAL_NOT_OBSERVED "
                               + new_log[-4400:])
        journal = call("sudo", "-n", "journalctl", "-u", unit,
                       "-n", "100", "--no-pager", check=False, timeout=20)
        if ("SD_CI_XDG_POPUP_SAME_CGROUP_WITNESS_STARTED" not in journal or
                "SD_CI_NATIVE_XDG_POPUP_REQUEST_SENT" not in journal):
            raise RuntimeError(
                "FAIL_NATIVE_POPUP_REAL_PROTOCOL_REQUEST_NOT_PROVEN " +
                journal[-4300:])
        # No visually pink popup may appear outside the browser content.
        time.sleep(0.35)
        denied = temp / "sd-ci-native-popup-denied.ppm"
        call(grim, "-t", "ppm", str(denied), timeout=25)
        call(grim, "-t", "png",
             str(temp / "sd-ci-native-popup-denied.png"), timeout=25)
        report = call(sys.executable,
                      "tests/stage_director/assert_real_xdg_popup_witness_pixels.py",
                      str(crop_ppm), str(positive), str(denied), timeout=30)
        print(report, flush=True)
        if "SD_CI_NATIVE_POPUP_DENIAL_PIXELS: result=PASS" not in report:
            raise RuntimeError("FAIL_NATIVE_POPUP_SCREENSHOT_PROOF_MISSING")
        # Click the same location after the exact Chrome-cgroup popup is
        # rejected; its service must have no native popup input receipt.
        if pointer_xy is None:
            raise RuntimeError("FAIL_NATIVE_POPUP_POSITIVE_COORDINATE_MISSING")
        denied_event_start = len((runtime / "sway.log").read_text(
            encoding="utf-8", errors="replace"))
        call(injector, "button", str(pointer_xy[0]), str(pointer_xy[1]),
             timeout=20)
        denied_event_log = (runtime / "sway.log").read_text(
            encoding="utf-8", errors="replace")[denied_event_start:]
        if "target=HIDDEN_SIBLING" in denied_event_log:
            raise RuntimeError("FAIL_DENIED_POPUP_INPUT_HIDDEN_CHROME_TARGET " +
                               denied_event_log[-2500:])
        denied_journal = call("sudo", "-n", "journalctl", "-u", unit,
                              "-n", "160", "--no-pager",
                              check=False, timeout=20)
        if "SD_CI_NATIVE_XDG_POPUP_INPUT kind=ENTER owner=POPUP" in denied_journal or (
                "SD_CI_NATIVE_XDG_POPUP_INPUT kind=BUTTON" in denied_journal and
                "owner=POPUP" in denied_journal):
            raise RuntimeError("FAIL_DENIED_POPUP_RECEIVED_NATIVE_INPUT " +
                               denied_journal[-3500:])
        print("SD_CI_XDG_POPUP_INPUT_CUSTODY: result=PASS "
              "unrelated_popup_native_receipt=1 denied_same_cgroup_popup_receipt=0 "
              "same_pixels_clicked=1 protected_popup_global_scene=BLOCKED "
              "browser_executable_popup=UNTESTED authority=NONE", flush=True)
        print("SD_CI_XDG_POPUP_NATIVE_EVENT: result=PASS "
              "separate_unrelated_popup_rendered=1 "
              "same_chrome_service_cgroup_emitter=1 "
              "root_popup_creation_denied=1 real_pixels=1 "
              "browser_executable_popup=UNTESTED authority=NONE", flush=True)
    finally:
        request.unlink(missing_ok=True)


def verify_multi_chrome_sibling_cycles(e, temp, swaymsg, ipc, grim,
                                       unit, root, snapshot, original_id,
                                       crop_ppm, before_tree, initial_siblings):
    """Two simultaneous real Chrome roots, close one, reopen a fresh root.

    Only the exact disposable Chrome service may emit the requests. This
    evaluates steady-state compositor pixels, not unseen transient frames.
    """
    if (e.get("CI") != "true" or e.get("GITHUB_ACTIONS") != "true" or
            e.get("STAGE_DIRECTOR_CI_CHROME_MULTI_SIBLING") != "1" or
            e.get("STAGE_DIRECTOR_CI_CHROME_SIBLING_CONTAINMENT") != "1"):
        raise RuntimeError("BLOCKED_MULTI_CHROME_OUTSIDE_CI")
    prefix = (temp / ("sd-ci-chrome-sibling-" + e["GITHUB_RUN_ID"] +
                       "-" + e.get("GITHUB_RUN_ATTEMPT", "1") + ".request"))
    initial_id = initial_siblings[0]["id"]
    identities = {initial_id}
    expected_current = {initial_id}
    first_rect = None

    def observed_native():
        tree = json.loads(call(swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
        pids = members(root)
        candidates = {}
        for n in native_xdg_toplevels(tree):
            pid = n.get("pid")
            if (n.get("shell") == "xdg_shell" and pid in pids and
                    proc_start(pid) is not None):
                candidates[n["id"]] = n
                snapshot[pid] = proc_start(pid)
        if original_id not in candidates:
            raise RuntimeError("FAIL_MULTI_CHROME_ORIGINAL_LOST")
        return tree, {k: n for k, n in candidates.items()
                      if k != original_id}

    def request(stage):
        path = Path(str(prefix) + "." + str(stage))
        fd = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY |
                     getattr(os, "O_NOFOLLOW", 0), 0o600)
        try:
            value = ("SIBLING" + str(stage) + "\n").encode("ascii")
            if os.write(fd, value) != len(value):
                raise RuntimeError("FAIL_MULTI_CHROME_SHORT_REQUEST")
        finally:
            os.close(fd)
        return path

    def wait_for(expected, retired=None):
        for _ in range(150):
            tree, siblings = observed_native()
            keys = set(siblings)
            if keys == expected and (retired is None or retired not in keys):
                return tree, siblings
            time.sleep(0.10)
        raise RuntimeError(
            "FAIL_MULTI_CHROME_NATIVE_ROOT_COUNT "
            f"expected={sorted(expected)} actual={sorted(keys)}")

    def photo(stage, tree):
        tree_path = temp / ("sd-ci-chrome-multi-" + stage + "-tree.json")
        ppm = temp / ("sd-ci-chrome-multi-" + stage + ".ppm")
        tree_path.write_text(json.dumps(tree), encoding="utf-8")
        call(grim, "-t", "ppm", str(ppm), timeout=25)
        call(grim, "-t", "png",
             str(temp / ("sd-ci-chrome-multi-" + stage + ".png")), timeout=25)
        report = call(
            sys.executable,
            "tests/stage_director/assert_real_chrome_sibling_discovery.py",
            str(crop_ppm), str(ppm), str(before_tree),
            str(tree_path), str(original_id), "--require-contained",
            "--require-extras", "2" if stage != "after-close" else "1",
            timeout=30)
        print(report, flush=True)
        if "SD_CI_CHROME_SECOND_TOPLEVEL_CONTAINMENT: result=PASS" not in report:
            raise RuntimeError("FAIL_MULTI_CHROME_PIXELS_NOT_CONTAINED")
        print(f"SD_CI_CHROME_MULTI_FRAME stage={stage} "
              "sampled_outside_bright=0 main_geometry_stable=1 "
              "authority=NONE result=PASS", flush=True)

    paths = []
    try:
        paths.append(request(2))
        both_tree = None
        for _ in range(150):
            tree, siblings = observed_native()
            if len(siblings) == 2 and initial_id in siblings:
                both_tree = tree
                break
            time.sleep(0.1)
        if both_tree is None:
            raise RuntimeError("FAIL_MULTI_CHROME_SECOND_SIBLING_NOT_MAPPED")
        second_id = next(k for k in siblings if k != initial_id)
        identities.add(second_id)
        expected_current = {initial_id, second_id}
        time.sleep(0.35)
        both_tree, siblings = wait_for(expected_current)
        photo("two-open", both_tree)
        log = (Path(e["XDG_RUNTIME_DIR"]) / "sway.log").read_text(
            encoding="utf-8", errors="replace")
        if log.count("SD_CI_CHROME_SIBLING_QUARANTINED") < 2:
            raise RuntimeError("FAIL_MULTI_CHROME_TWO_QUARANTINE_EVENTS")

        # Close one and require the surviving hidden sibling remains mapped.
        command = f"[con_id={initial_id}] kill"
        result = json.loads(call(
            swaymsg, "-s", ipc, "-r", "-t", "command", command, timeout=20))
        if not (isinstance(result, list) and len(result) == 1 and
                result[0].get("success") is True):
            raise RuntimeError("FAIL_MULTI_CHROME_EXACT_CLOSE_REJECTED")
        for _ in range(120):
            tree, siblings = observed_native()
            if set(siblings) == {second_id}:
                break
            time.sleep(0.10)
        else:
            raise RuntimeError("FAIL_MULTI_CHROME_CLOSED_ROOT_STILL_MAPPED")
        expected_current = {second_id}
        time.sleep(0.25)
        tree, siblings = wait_for(expected_current, retired=initial_id)
        # after-close uses one surviving sibling, never a source title.
        photo("after-close", tree)
        log = (Path(e["XDG_RUNTIME_DIR"]) / "sway.log").read_text(
            encoding="utf-8", errors="replace")
        if "SD_CI_CHROME_SIBLING_UNMAPPED" not in log:
            raise RuntimeError("FAIL_MULTI_CHROME_SLOT_NOT_REVOKED")

        paths.append(request(3))
        for _ in range(150):
            tree, siblings = observed_native()
            if (len(siblings) == 2 and second_id in siblings and
                    initial_id not in siblings):
                break
            time.sleep(0.1)
        else:
            raise RuntimeError("FAIL_MULTI_CHROME_REOPEN_ROOT_NOT_MAPPED")
        reopened_id = next(k for k in siblings if k != second_id)
        if reopened_id in identities:
            raise RuntimeError("FAIL_MULTI_CHROME_REUSED_NATIVE_ID")
        identities.add(reopened_id)
        expected_current = {second_id, reopened_id}
        time.sleep(0.35)
        tree, siblings = wait_for(expected_current, retired=initial_id)
        photo("reopened", tree)
        log = (Path(e["XDG_RUNTIME_DIR"]) / "sway.log").read_text(
            encoding="utf-8", errors="replace")
        if (log.count("SD_CI_CHROME_SIBLING_QUARANTINED") < 3 or
                log.count("SD_CI_CHROME_SIBLING_UNMAPPED") < 1 or
                "SD_CI_CHROME_SIBLING_CAPACITY_EXCEEDED" in log):
            raise RuntimeError("FAIL_MULTI_CHROME_SCENE_SLOT_LIFECYCLE")
        journal = call("sudo", "-n", "journalctl", "-u", unit,
                       "-n", "150", "--no-pager", check=False)
        for stage in (1, 2, 3):
            if f"SD_CI_CHROME_SIBLING_EXECUTABLE_REQUESTED stage={stage}" not in journal:
                raise RuntimeError(
                    "FAIL_MULTI_CHROME_REAL_EXECUTABLE_STAGE_" + str(stage))
        print("SD_CI_CHROME_MULTI_SIBLING_LIFECYCLE: result=PASS "
              "native_chrome_extra_roots=2 simultaneous=1 "
              "closed_root_unmapped=1 new_native_root=1 "
              "scene_slots_reused=1 three_real_snapshots=1 "
              "zero_sampled_bright_escape=1 original_geometry_stable=1 "
              "same_exact_service=1 authority=NONE", flush=True)
        return identities
    finally:
        for path in paths:
            path.unlink(missing_ok=True)


def verify_real_native_subsurface_ownership(e, temp, swaymsg, ipc, grim,
                                            original_id, sibling_id, root,
                                            send, focus, view_state, log_path):
    """Actual Wayland wl_subsurface event from unrelated control, not Chrome.

    The observable unrelated descendant confirms the native wlroots root
    resolver is exercised, while the Chrome hidden sibling stays excluded.
    An XDG popup is NOT a wl_subsurface ancestor and remains unproven.
    """
    exe = e.get("STAGE_DIRECTOR_TEST_SUBSURFACE_WITNESS", "")
    if (e.get("CI") != "true" or e.get("GITHUB_ACTIONS") != "true" or
            e.get("STAGE_DIRECTOR_CI_CHROME_SUBSURFACE") != "1" or
            e.get("STAGE_DIRECTOR_CI_CHROME_REAL_BUTTON") != "1" or
            not exe.startswith("/nix/store/") or not Path(exe).is_file()):
        raise RuntimeError("BLOCKED_NATIVE_SUBSURFACE_OUTSIDE_EXACT_CI")
    outpath = temp / "sd-ci-native-subsurface-witness.log"
    outfile = outpath.open("w", encoding="utf-8")
    child = None
    try:
        child = subprocess.Popen([exe, "run"], stdout=outfile,
                                 stderr=subprocess.STDOUT)
        native = None
        for _ in range(120):
            if child.poll() is not None:
                raise RuntimeError("FAIL_SUBSURFACE_WITNESS_EXITED_EARLY")
            tree = json.loads(call(swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
            matches = [n for n in native_xdg_toplevels(tree)
                       if n.get("pid") == child.pid and
                       n.get("app_id") == "sd-ci-subsurface-witness" and
                       n.get("shell") == "xdg_shell"]
            if len(matches) == 1:
                native = matches[0]
                break
            time.sleep(0.1)
        if native is None:
            raise RuntimeError("FAIL_SUBSURFACE_XDG_PARENT_NOT_MAPPED")
        rect = native.get("rect") or {}
        x, y = int(rect["x"]) + 49, int(rect["y"]) + 47
        if not (0 <= x < 1920 and 0 <= y < 1080):
            raise RuntimeError("BLOCKED_SUBSURFACE_COORDS_OUTSIDE_OUTPUT")
        for _ in range(30):
            outfile.flush()
            if "SD_CI_NATIVE_SUBSURFACE_MAPPED" in outpath.read_text(
                    encoding="utf-8", errors="replace"):
                break
            time.sleep(0.10)
        else:
            raise RuntimeError("FAIL_REAL_SUBSURFACE_BUFFER_NOT_ATTACHED")
        pos = len(log_path.read_text(encoding="utf-8", errors="replace"))
        send("pointer", str(x), str(y))
        send("button", str(x), str(y))
        # Independent parent-surface click: if both parent and child miss
        # button receipts, a pre-notify target log is not receipt proof.
        px, py = int(rect["x"]) + 130, int(rect["y"]) + 103
        if not (0 <= px < 1920 and 0 <= py < 1080):
            raise RuntimeError("BLOCKED_SUBSURFACE_PARENT_COORDS_OUTSIDE_OUTPUT")
        send("button", str(px), str(py))
        events = log_path.read_text(
            encoding="utf-8", errors="replace")[pos:]
        if ("kind=POINTER target=OTHER root_only=0 descendant=1" not in
                events or
                "state=PRESS target=OTHER root_only=0 descendant=1" not in
                events or
                "state=RELEASE target=OTHER root_only=0 descendant=1" not in
                events):
            print("SD_CI_SUBSURFACE_INPUT_DIAGNOSTIC "
                  "child_root_resolution=FAIL " + events[-2600:],
                  flush=True)
        if ("SD_CI_NATIVE_BUTTON_RESULT state=PRESS serial_nonzero=1"
                not in events or
                "SD_CI_NATIVE_BUTTON_RESULT state=RELEASE serial_nonzero=1"
                not in events):
            raise RuntimeError("FAIL_SUBSURFACE_WLROOTS_BUTTON_NO_SERIAL " +
                               events[-3500:])
        if ("kind=POINTER target=OTHER root_only=0 descendant=1" not in
                events or
                "state=PRESS target=OTHER root_only=0 descendant=1" not in
                events or
                "state=RELEASE target=OTHER root_only=0 descendant=1" not in
                events):
            raise RuntimeError("FAIL_NATIVE_SUBSURFACE_ROOT_RESOLUTION " +
                               events[-3500:])
        if "target=HIDDEN_SIBLING" in events:
            raise RuntimeError("FAIL_SUBSURFACE_EVENT_TO_CHROME_HIDDEN_ROOT")
        for _ in range(30):
            outfile.flush()
            clientlog = outpath.read_text(
                encoding="utf-8", errors="replace")
            if ("kind=ENTER owner=CHILD" in clientlog and
                    "kind=BUTTON state=PRESS owner=CHILD" in clientlog and
                    "kind=BUTTON state=RELEASE owner=CHILD" in clientlog and
                    "kind=BUTTON state=PRESS owner=PARENT" in clientlog and
                    "kind=BUTTON state=RELEASE owner=PARENT" in clientlog):
                break
            time.sleep(0.10)
        else:
            raise RuntimeError("FAIL_REAL_SUBSURFACE_CLIENT_NO_RECEIPT " +
                               "actual_client_log=" + clientlog[-2500:] +
                               " native_wlroots=" + events[-2500:])
        call(grim, "-t", "png",
             str(temp / "sd-ci-subsurface-positive.png"), timeout=20)
        print("SD_CI_NATIVE_SUBSURFACE_INPUT_OWNER: result=PASS "
              "real_wl_subsurface=1 real_xdg_parent=1 "
              "wlroots_root_resolved=1 descendant=1 "
              "child_pointer_receipt=1 child_button_press_release=1 "
              "other_root_positive=1 hidden_chrome_root_receipts=0 "
              "xdg_popup_parentage=UNTESTED authority=NONE", flush=True)
    finally:
        if child is not None:
            if child.poll() is None:
                child.terminate()
            try:
                child.wait(timeout=4)
            except subprocess.TimeoutExpired:
                child.kill()
                child.wait(timeout=3)
        outfile.close()
        if child is not None:
            for _ in range(100):
                tree = json.loads(call(
                    swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
                if not any(n.get("pid") == child.pid
                           for n in containers(tree)):
                    break
                time.sleep(0.05)
            else:
                raise RuntimeError("FAIL_SUBSURFACE_XDG_PARENT_STILL_MAPPED")
        focus(original_id)
        _, nodes = view_state()
        if nodes[sibling_id].get("focused") or not nodes[original_id].get("focused"):
            raise RuntimeError("FAIL_SUBSURFACE_TEST_CHANGED_CHROME_FOCUS")


def verify_chrome_native_focus_custody(e, temp, swaymsg, ipc, grim,
                                       root, original_id, sibling_id,
                                       baseline_ppm):
    """CI native seat focus: allow original, deny exact hidden Chrome sibling.

    This is Sway seat/IPC focus proof, NOT actual key presses, pointer
    events or Wayland subsurface focus. The renderer is still CI-only.
    """
    if (e.get("CI") != "true" or e.get("GITHUB_ACTIONS") != "true" or
            e.get("STAGE_DIRECTOR_CI_CHROME_FOCUS_CUSTODY") != "1" or
            e.get("STAGE_DIRECTOR_CI_CHROME_SIBLING_CONTAINMENT") != "1" or
            not original_id or not sibling_id or original_id == sibling_id):
        raise RuntimeError("BLOCKED_CHROME_FOCUS_OUTSIDE_EXACT_CI")
    log_path = Path(e["XDG_RUNTIME_DIR"]) / "sway.log"
    log_before = log_path.read_text(encoding="utf-8", errors="replace")
    marker_start = len(log_before)
    active = members(root)

    def view_state():
        tree = json.loads(call(swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
        native = {n["id"]: n for n in native_xdg_toplevels(tree)
                  if n.get("shell") == "xdg_shell" and
                  n.get("pid") in active and proc_start(n["pid"]) is not None}
        if original_id not in native or sibling_id not in native:
            raise RuntimeError("FAIL_FOCUS_NATIVE_XDG_IDENTITY_DISAPPEARED")
        return tree, native

    def focus(command_id):
        cmd = f"[con_id={command_id}] focus"
        result = json.loads(call(
            swaymsg, "-s", ipc, "-r", "-t", "command", cmd, timeout=20))
        if not (isinstance(result, list) and len(result) == 1 and
                result[0].get("success") is True):
            raise RuntimeError("FAIL_FOCUS_NATIVE_IPC_COMMAND_REJECTED " +
                               repr(result))

    # Explicit positive control: the real visible Chrome actor must be
    # focusable through the very same exact-id IPC command.
    focus(original_id)
    tree, nodes = view_state()
    if not nodes[original_id].get("focused"):
        raise RuntimeError("FAIL_FOCUS_VISIBLE_CHROME_POSITIVE_CONTROL")
    if nodes[sibling_id].get("focused"):
        raise RuntimeError("FAIL_FOCUS_HIDDEN_SIBLING_ALREADY_FOCUSED")
    initial_rect = nodes[original_id].get("rect")

    # Negative attempt: Sway command may succeed syntactically, but
    # native seat/keyboard focus MUST remain with the original actor.
    focus(sibling_id)
    tree, nodes = view_state()
    if (not nodes[original_id].get("focused") or
            nodes[sibling_id].get("focused") or
            nodes[original_id].get("rect") != initial_rect):
        raise RuntimeError("FAIL_FOCUS_HIDDEN_CHROME_STOLE_SEAT")
    log = log_path.read_text(encoding="utf-8", errors="replace")[marker_start:]
    if ("SD_CI_CHROME_SIBLING_FOCUS_DENIED" not in log or
            "source=seat_container" not in log):
        raise RuntimeError("FAIL_FOCUS_COMPOSITOR_DENIAL_NOT_OBSERVED " +
                           log[-3000:])
    if e.get("STAGE_DIRECTOR_CI_CHROME_REAL_INPUT") == "1":
        exe = e.get("STAGE_DIRECTOR_TEST_VIRTUAL_INPUT", "")
        if (not exe.startswith("/nix/store/") or
                not Path(exe).is_file() or
                e.get("CI") != "true" or
                e.get("GITHUB_ACTIONS") != "true"):
            raise RuntimeError("BLOCKED_NATIVE_VIRTUAL_INPUT_NOT_PINNED")
        if "SD_CI_CHROME_SIBLING_QUARANTINED" not in log_before:
            raise RuntimeError("FAIL_VIRTUAL_INPUT_NO_HIDDEN_CHROME_SCENE")
        original_rect = nodes[original_id].get("rect") or {}
        sibling_rect = nodes[sibling_id].get("rect") or {}
        # Existing SceneFX clip = local geometry x+96..416, y+96..336.
        # Choose real compositor coordinates clearly inside that crop.
        main_x = int(original_rect["x"]) + 160
        main_y = int(original_rect["y"]) + 160
        sibling_x = int(sibling_rect["x"]) + int(sibling_rect["width"]) // 2
        sibling_y = int(sibling_rect["y"]) + int(sibling_rect["height"]) // 2
        if not (0 <= main_x < 1920 and 0 <= main_y < 1080 and
                0 <= sibling_x < 1920 and 0 <= sibling_y < 1080):
            raise RuntimeError("BLOCKED_NATIVE_INPUT_COORDINATES_OUT_OF_OUTPUT")
        def send(*args):
            print(call(exe, *args, timeout=15), flush=True)
        pos = len(log_path.read_text(encoding="utf-8", errors="replace"))
        send("keyboard")
        key_log = log_path.read_text(
            encoding="utf-8", errors="replace")[pos:]
        if ("SD_CI_NATIVE_INPUT_DELIVERED kind=KEYBOARD target=MAIN" not in
                key_log or
                "kind=KEYBOARD target=HIDDEN_SIBLING" in key_log):
            raise RuntimeError(
                "FAIL_NATIVE_KEY_DELIVERY_TARGET " + key_log[-2700:])
        pos = len(log_path.read_text(encoding="utf-8", errors="replace"))
        send("pointer", str(main_x), str(main_y))
        main_pointer_log = log_path.read_text(
            encoding="utf-8", errors="replace")[pos:]
        if ("SD_CI_NATIVE_INPUT_DELIVERED kind=POINTER target=MAIN"
                not in main_pointer_log):
            raise RuntimeError("FAIL_NATIVE_POINTER_VISIBLE_POSITIVE " +
                               main_pointer_log[-2700:])
        pos = len(log_path.read_text(encoding="utf-8", errors="replace"))
        send("pointer", str(sibling_x), str(sibling_y))
        sibling_pointer_log = log_path.read_text(
            encoding="utf-8", errors="replace")[pos:]
        if "kind=POINTER target=HIDDEN_SIBLING" in sibling_pointer_log:
            raise RuntimeError("FAIL_NATIVE_POINTER_REACHED_HIDDEN_SIBLING")
        if e.get("STAGE_DIRECTOR_CI_CHROME_REAL_BUTTON") == "1":
            def button_signals(label, *args):
                cursor_start = len(log_path.read_text(
                    encoding="utf-8", errors="replace"))
                send(*args)
                signals = log_path.read_text(
                    encoding="utf-8", errors="replace")[cursor_start:]
                if "SD_CI_NATIVE_BUTTON_DELIVERED state=PRESS" not in signals:
                    raise RuntimeError(
                        "FAIL_NATIVE_BUTTON_PRESS_NOT_DELIVERED " +
                        label + " " + signals[-2500:])
                if "SD_CI_NATIVE_BUTTON_DELIVERED state=RELEASE" not in signals:
                    raise RuntimeError(
                        "FAIL_NATIVE_BUTTON_RELEASE_NOT_DELIVERED " +
                        label + " " + signals[-2500:])
                if ("SD_CI_NATIVE_BUTTON_DELIVERED state=PRESS "
                        "target=HIDDEN_SIBLING" in signals or
                        "SD_CI_NATIVE_BUTTON_DELIVERED state=RELEASE "
                        "target=HIDDEN_SIBLING" in signals or
                        "SD_CI_NATIVE_INPUT_DELIVERED kind=POINTER "
                        "target=HIDDEN_SIBLING" in signals):
                    raise RuntimeError(
                        "FAIL_NATIVE_BUTTON_GRAB_REACHED_HIDDEN_SIBLING " +
                        label + " " + signals[-3500:])
                return signals
            visible = button_signals(
                "visible_click", "button", str(main_x), str(main_y))
            if (("SD_CI_NATIVE_BUTTON_DELIVERED state=PRESS target=MAIN"
                    not in visible) or
                    ("SD_CI_NATIVE_BUTTON_DELIVERED state=RELEASE target=MAIN"
                     not in visible)):
                raise RuntimeError("FAIL_NATIVE_BUTTON_VISIBLE_POSITIVE " +
                                   visible[-2500:])
            hidden = button_signals(
                "hidden_click", "button", str(sibling_x), str(sibling_y))
            _, hidden_click_nodes = view_state()
            if (hidden_click_nodes[sibling_id].get("focused") or
                    hidden_click_nodes[original_id].get("rect") != initial_rect):
                raise RuntimeError(
                    "FAIL_NATIVE_BUTTON_HIDDEN_FOCUS_OR_GEOMETRY")
            # Sway may legitimately focus an empty workspace after clicking
            # hidden coordinates. Restore the visible actor before the
            # independent same-pointer grab positive test.
            focus(original_id)
            grab = button_signals(
                "cross_hidden_grab", "grab", str(main_x), str(main_y),
                str(sibling_x), str(sibling_y))
            if "SD_CI_NATIVE_BUTTON_DELIVERED state=PRESS target=MAIN" not in grab:
                raise RuntimeError("FAIL_NATIVE_GRAB_MAIN_PRESS_MISSING " +
                                   grab[-2500:])
            _, after_grab_nodes = view_state()
            if (after_grab_nodes[sibling_id].get("focused") or
                    after_grab_nodes[original_id].get("rect") != initial_rect):
                raise RuntimeError(
                    "FAIL_NATIVE_GRAB_HIDDEN_FOCUS_OR_GEOMETRY")
            focus(original_id)
            print("SD_CI_CHROME_BUTTON_GRAB: result=PASS "
                  "native_btn_left_press_release=1 "
                  "visible_button_positive=1 hidden_button_negative=1 "
                  "same_pointer_grab_cross_hidden=1 "
                  "hidden_root_button_receipts=0 "
                  "hidden_root_pointer_receipts=0 authority=NONE",
                  flush=True)
        if e.get("STAGE_DIRECTOR_CI_CHROME_SUBSURFACE") == "1":
            verify_real_native_subsurface_ownership(
                e, temp, swaymsg, ipc, grim, original_id, sibling_id, root,
                send, focus, view_state, log_path)
        # Return actual cursor to the visible, clipped Chrome region
        # before the independent scene screenshot pixel check.
        send("pointer", str(main_x), str(main_y))
        post_tree, post_nodes = view_state()
        if (not post_nodes[original_id].get("focused") or
                post_nodes[sibling_id].get("focused") or
                post_nodes[original_id].get("rect") != initial_rect):
            raise RuntimeError("FAIL_VIRTUAL_INPUT_CHANGED_NATIVE_SEAT_OR_GEOMETRY")
        tree = post_tree
        print("SD_CI_CHROME_REAL_INPUT: result=PASS "
              "native_virtual_keyboard=1 native_virtual_pointer=1 "
              "visible_keyboard_target=MAIN hidden_keyboard_target=NONE "
              "visible_pointer_target=MAIN hidden_pointer_target=NOT_SIBLING "
              "exact_native_xdg=1 authority=NONE", flush=True)
    photo = temp / "sd-ci-chrome-hidden-focus-denied.ppm"
    tree_file = temp / "sd-ci-chrome-hidden-focus-denied-tree.json"
    tree_file.write_text(json.dumps(tree), encoding="utf-8")
    call(grim, "-t", "ppm", str(photo), timeout=25)
    call(grim, "-t", "png",
         str(temp / "sd-ci-chrome-hidden-focus-denied.png"), timeout=25)
    # The same real screenshot confinement check must remain valid after
    # attempting to raise/focus the invisible floating Chrome sibling.
    baseline_tree = temp / "sd-ci-chrome-sibling-before-tree.json"
    report = call(
        sys.executable,
        "tests/stage_director/assert_real_chrome_sibling_discovery.py",
        str(baseline_ppm), str(photo), str(baseline_tree),
        str(tree_file), str(original_id), "--require-contained", timeout=30)
    print(report, flush=True)
    if "SD_CI_CHROME_SECOND_TOPLEVEL_CONTAINMENT: result=PASS" not in report:
        raise RuntimeError("FAIL_FOCUS_SIBLING_REENABLED_VISIBLE_SCENE")
    print("SD_CI_CHROME_FOCUS_CUSTODY: result=PASS "
          "native_ipc_focus_attempt=1 hidden_sibling_denied=1 "
          "visible_actor_positive=1 original_keyboard_focus_retained=1 "
          "original_geometry_stable=1 sampled_scene_clear=1 "
          "pointer_events=UNTESTED real_keypress=UNTESTED "
          "authority=NONE", flush=True)


def discover_chrome_second_toplevel(e, temp, swaymsg, ipc, grim,
                                     unit, root, snapshot, actor,
                                     crop_ppm, profile):
    """Real Chrome second XDG root; diagnostic classification, not custody."""
    if (e.get("CI") != "true" or e.get("GITHUB_ACTIONS") != "true" or
            e.get("STAGE_DIRECTOR_CI_CHROME_SIBLING_DISCOVERY") != "1"):
        raise RuntimeError("BLOCKED_CHROME_SIBLING_OUTSIDE_CI")
    request = temp / ("sd-ci-chrome-sibling-" + e["GITHUB_RUN_ID"] +
                      "-" + e.get("GITHUB_RUN_ATTEMPT", "1") + ".request")
    previous = json.loads(call(swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
    before_tree = temp / "sd-ci-chrome-sibling-before-tree.json"
    before_tree.write_text(json.dumps(previous), encoding="utf-8")
    original_id, original_pid = actor["id"], actor["pid"]
    fd = os.open(request, os.O_CREAT | os.O_EXCL | os.O_WRONLY |
                 getattr(os, "O_NOFOLLOW", 0), 0o600)
    try:
        os.write(fd, b"SIBLING\n")
    finally:
        os.close(fd)
    try:
        siblings = []
        for _ in range(120):
            tree = json.loads(call(swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
            active = members(root)
            originals = [n for n in native_xdg_toplevels(tree)
                         if n.get("id") == original_id and
                         n.get("pid") == original_pid]
            siblings = [n for n in native_xdg_toplevels(tree)
                        if n.get("id") != original_id and
                        n.get("shell") == "xdg_shell" and
                        n.get("pid") in active and
                        proc_start(n["pid"]) is not None]
            if originals and siblings:
                break
            if not active:
                break
            time.sleep(0.1)
        else:
            # Preserve the real IPC representation at the precise failure
            # boundary; never infer an invisible native view from a log.
            tree = json.loads(call(
                swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
            (temp / "sd-ci-chrome-sibling-failed-tree.json").write_text(
                json.dumps(tree), encoding="utf-8")
            flat = []
            def inspect_node(node):
                if isinstance(node, dict):
                    if (node.get("pid") is not None or
                            node.get("type") in ("con", "floating_con")):
                        pid = node.get("pid")
                        flat.append({
                            "type": node.get("type"), "id": node.get("id"),
                            "pid": pid, "shell": node.get("shell"),
                            "cgroup": pid in members(root) if pid else False,
                            "rect": node.get("rect")})
                    for key in ("nodes", "floating_nodes"):
                        for child in node.get(key, []):
                            inspect_node(child)
            inspect_node(tree)
            log = (Path(e["XDG_RUNTIME_DIR"]) / "sway.log").read_text(
                encoding="utf-8", errors="replace")
            raise RuntimeError(
                "FAIL_CHROME_EXECUTABLE_NO_SECOND_NATIVE_XDG "
                "observed_tree=" + json.dumps(flat) +
                " original_id=" + str(original_id) +
                " sibling_quarantined=" +
                str("SD_CI_CHROME_SIBLING_QUARANTINED" in log))
        if not originals or not siblings:
            raise RuntimeError("FAIL_CHROME_SIBLING_LOST_ORIGINAL_NATIVE_XDG")
        journal = call("sudo", "-n", "journalctl", "-u", unit,
                       "-n", "100", "--no-pager", check=False)
        if "SD_CI_CHROME_SIBLING_EXECUTABLE_REQUESTED" not in journal:
            raise RuntimeError("FAIL_CHROME_SECOND_EXECUTABLE_NOT_STARTED " +
                               journal[-2500:])
        for node in siblings:
            snapshot[node["pid"]] = proc_start(node["pid"])
        containment_mode = (
            e.get("STAGE_DIRECTOR_CI_CHROME_SIBLING_CONTAINMENT") == "1")
        if containment_mode:
            log = (Path(e["XDG_RUNTIME_DIR"]) / "sway.log").read_text(
                encoding="utf-8", errors="replace")
            if ("SD_CI_CHROME_SIBLING_QUARANTINED" not in log or
                    "whole_container_scene_disabled=1" not in log):
                raise RuntimeError(
                    "FAIL_CHROME_SIBLING_SCENE_QUARANTINE_NOT_OBSERVED "
                    + log[-3600:])
        time.sleep(0.4)
        after_tree = temp / "sd-ci-chrome-sibling-after-tree.json"
        tree = json.loads(call(swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
        active = members(root)
        latest = [n for n in native_xdg_toplevels(tree)
                  if n.get("shell") == "xdg_shell" and
                  n.get("id") != original_id and n.get("pid") in active]
        if not latest:
            raise RuntimeError("FAIL_CHROME_SECOND_ROOT_DID_NOT_REMAIN")
        after_tree.write_text(json.dumps(tree), encoding="utf-8")
        shot = temp / "sd-ci-chrome-second-toplevel.ppm"
        call(grim, "-t", "ppm", str(shot), timeout=25)
        call(grim, "-t", "png",
             str(temp / "sd-ci-chrome-second-toplevel.png"), timeout=25)
        measured = call(
            sys.executable,
            "tests/stage_director/assert_real_chrome_sibling_discovery.py",
            str(crop_ppm), str(shot), str(before_tree),
            str(after_tree), str(original_id),
            *(["--require-contained"] if containment_mode else []),
            timeout=30)
        print(measured, flush=True)
        expected = ("SD_CI_CHROME_SECOND_TOPLEVEL_CONTAINMENT: result=PASS"
                    if containment_mode else
                    "SD_CI_CHROME_SECOND_TOPLEVEL_DIAGNOSTIC: result=OBSERVED")
        if expected not in measured:
            raise RuntimeError("FAIL_CHROME_SIBLING_PIXEL_PROOF_MISSING")
        print("SD_CI_CHROME_SECOND_NATIVE_XDG_DISCOVERED "
              f"initial_native_id={original_id} second_count={len(latest)} "
              "real_chrome_executable=1 native_cgroup=1 "
              "kernel_starttime=1 confinement=UNVERIFIED authority=NONE",
              flush=True)
        if e.get("STAGE_DIRECTOR_CI_CHROME_FOCUS_CUSTODY") == "1":
            if len(latest) != 1:
                raise RuntimeError("FAIL_FOCUS_EXACT_ONE_HIDDEN_SIBLING")
            verify_chrome_native_focus_custody(
                e, temp, swaymsg, ipc, grim, root, original_id,
                latest[0]["id"], crop_ppm)
        all_extra_ids = {n["id"] for n in latest}
        if e.get("STAGE_DIRECTOR_CI_CHROME_MULTI_SIBLING") == "1":
            all_extra_ids |= verify_multi_chrome_sibling_cycles(
                e, temp, swaymsg, ipc, grim, unit, root, snapshot,
                original_id, crop_ppm, before_tree, latest)
        call("sudo", "-n", "systemctl", "stop", unit)
        ids = {original_id} | all_extra_ids
        for _ in range(100):
            left = json.loads(call(
                swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
            if (not members(root) and not profile.parent.exists() and
                    all(proc_start(pid) != st
                        for pid, st in snapshot.items()) and
                    not any(n.get("id") in ids for n in native_xdg_toplevels(left))):
                break
            time.sleep(0.1)
        else:
            raise RuntimeError("FAIL_CHROME_SIBLING_TEARDOWN")
        print("SD_CI_CHROME_SECOND_TOPLEVEL_TEARDOWN "
              "all_native_roots_unmapped=1 cgroup_empty=1 "
              "private_profile_removed=1 authority=NONE result=PASS",
              flush=True)
    finally:
        request.unlink(missing_ok=True)


def main() -> None:
    e = os.environ
    runtime = Path(e.get("XDG_RUNTIME_DIR", "/missing"))
    temp = Path(e.get("RUNNER_TEMP", "/not-runner"))
    swaymsg = e.get("STAGE_DIRECTOR_TEST_SWAYMSG", "")
    if (e.get("GITHUB_ACTIONS") != "true" or e.get("CI") != "true" or
            not str(runtime).startswith(str(temp) + "/") or
            not runtime.is_dir() or not swaymsg.startswith("/nix/store/") or
            not Path(swaymsg).is_file() or
            not Path("/run/systemd/system").is_dir() or
            not Path("/opt/google/chrome/chrome").is_file() or
            not Path("/sys/fs/cgroup/cgroup.controllers").is_file() or
            not e.get("WAYLAND_DISPLAY", "").startswith("wayland-")):
        raise RuntimeError("BLOCKED_OUTSIDE_ISOLATED_WAYLAND_GITHUB_CI")
    # Existing headless harness created the socket under this exact runtime.
    sock = runtime / e["WAYLAND_DISPLAY"]
    if not sock.is_socket():
        raise RuntimeError("BLOCKED_NO_ISOLATED_WAYLAND_SOCKET")
    if call("id", "-un") != "runner":
        raise RuntimeError("BLOCKED_NOT_RUNNER_USER")
    ipc_sockets = [p for p in runtime.glob("sway-ipc.*.sock")
                   if p.is_socket()]
    if len(ipc_sockets) != 1:
        raise RuntimeError("BLOCKED_NO_UNIQUE_PRIVATE_SWAY_IPC")
    ipc = str(ipc_sockets[0])
    pixel_mode = e.get("STAGE_DIRECTOR_CI_REAL_CHROME_PIXEL") == "1"
    grim = e.get("STAGE_DIRECTOR_TEST_GRIM", "")
    if pixel_mode:
        if not grim.startswith("/nix/store/") or not Path(grim).is_file():
            raise RuntimeError("BLOCKED_MISSING_PINNED_GRIM")
        # This unrelated app uses the SAME compositor and normal titlebar
        # config. The Chrome candidate has not yet been spawned.
        verify_unrelated_titlebar(
            swaymsg, ipc, e.get("STAGE_DIRECTOR_TEST_DECOR_CONTROL_FOOT", ""))
        # A clean image from exactly this compositor before Chrome maps.
        call(grim, "-t", "ppm",
             str(temp / "sd-chrome-before.ppm"), timeout=20)
    run_id = e["GITHUB_RUN_ID"] + "-" + e.get("GITHUB_RUN_ATTEMPT", "1")
    unit = "sd-ci-chrome-wayland-" + run_id + ".service"
    rt_name = "sd-ci-chrome-wayland-" + run_id
    profile = Path("/run") / rt_name / "profile"
    uid = os.getuid()
    started = False
    root: Path | None = None
    snapshot: dict[int, int] = {}
    popup_mode = e.get("STAGE_DIRECTOR_CI_CHROME_POPUP_WITNESS") == "1"
    sibling_mode = e.get("STAGE_DIRECTOR_CI_CHROME_SIBLING_DISCOVERY") == "1"
    if popup_mode and sibling_mode:
        raise RuntimeError("BLOCKED_CONFLICTING_CHROME_CI_MODES")
    sibling_request = temp / (
        "sd-ci-chrome-sibling-" + e["GITHUB_RUN_ID"] + "-" +
        e.get("GITHUB_RUN_ATTEMPT", "1") + ".request")
    popup_exe = e.get("STAGE_DIRECTOR_TEST_XDG_POPUP_WITNESS", "")
    popup_request = temp / (
        "sd-ci-xdg-popup-" + e["GITHUB_RUN_ID"] + "-" +
        e.get("GITHUB_RUN_ATTEMPT", "1") + ".request")
    if popup_mode and (not popup_exe.startswith("/nix/store/") or
                       not Path(popup_exe).is_file()):
        raise RuntimeError("BLOCKED_NATIVE_POPUP_WITNESS_UNBUILT")
    try:
        # The runner's Chrome has its own distro/AppArmor policy (whether
        # the internal sandbox is ACTIVE remains unverified). The service
        # never requests a browser-sandbox bypass or outside network.
        call("sudo", "-n", "systemd-run", "--unit=" + unit,
             "--property=User=runner", "--property=Group=runner",
             "--property=RuntimeDirectory=" + rt_name,
             "--property=RuntimeDirectoryMode=0700",
             "--property=PrivateNetwork=yes",
             "--property=ProtectSystem=strict",
             # The headless socket lives in runner HOME. Read-only allows
             # traversal/socket connection but not writing inside HOME.
             "--property=ProtectHome=read-only",
             "--property=PrivateTmp=yes",
             "--property=NoNewPrivileges=yes",
             "--property=IPAddressDeny=any",
             "--property=MemoryMax=1536M",
             "--property=TasksMax=256",
             "--property=KillMode=control-group",
             "--property=SendSIGKILL=yes",
             "--property=TimeoutStopSec=3s",
             "--setenv=HOME=" + str(profile.parent),
             "--setenv=XDG_CONFIG_HOME=" + str(profile.parent),
             "--setenv=XDG_CACHE_HOME=" + str(profile.parent),
             "--setenv=XDG_RUNTIME_DIR=" + str(runtime),
             "--setenv=WAYLAND_DISPLAY=" + e["WAYLAND_DISPLAY"],
             "--setenv=XDG_SESSION_TYPE=wayland",
             *([
                 "--setenv=CI=true",
                 "--setenv=GITHUB_ACTIONS=true",
                 "--setenv=GITHUB_RUN_ID=" + e["GITHUB_RUN_ID"],
                 "--setenv=GITHUB_RUN_ATTEMPT=" +
                     e.get("GITHUB_RUN_ATTEMPT", "1"),
                 "--setenv=RUNNER_TEMP=" + str(temp),
                 "--setenv=STAGE_DIRECTOR_CI_CHROME_POPUP_WITNESS=1",
                 "/usr/bin/bash",
                 str(Path("tests/stage_director/ci_chrome_popup_service.sh").resolve()),
                 popup_exe, str(popup_request),
             ] if popup_mode else [
                 "--setenv=CI=true",
                 "--setenv=GITHUB_ACTIONS=true",
                 "--setenv=GITHUB_RUN_ID=" + e["GITHUB_RUN_ID"],
                 "--setenv=GITHUB_RUN_ATTEMPT=" +
                     e.get("GITHUB_RUN_ATTEMPT", "1"),
                 "--setenv=RUNNER_TEMP=" + str(temp),
                 "--setenv=STAGE_DIRECTOR_CI_CHROME_SIBLING_DISCOVERY=1",
                 *([
                     "--setenv=STAGE_DIRECTOR_CI_CHROME_MULTI_SIBLING=1",
                 ] if e.get("STAGE_DIRECTOR_CI_CHROME_MULTI_SIBLING") == "1" else []),
                 "/usr/bin/bash",
                 str(Path("tests/stage_director/ci_chrome_sibling_service.sh")
                     .resolve()),
                 str(sibling_request),
             ] if sibling_mode else []),
             "/opt/google/chrome/chrome",
             "--ozone-platform=wayland",
             "--disable-gpu",
             "--disable-background-networking",
             "--disable-component-update",
             "--disable-extensions", "--disable-sync",
             "--no-first-run", "--no-default-browser-check",
             "--user-data-dir=" + str(profile),
             "--window-size=1120,720",
             "--app=about:blank", timeout=30)
        started = True
        relative = call("systemctl", "show", unit,
                        "--property=ControlGroup", "--value")
        if (not relative.startswith("/") or ".." in relative or
                rt_name not in relative or relative == "/"):
            raise RuntimeError("BLOCKED_MISSING_EXACT_SERVICE_CGROUP")
        root = Path("/sys/fs/cgroup" + relative)
        if not (root / "cgroup.procs").is_file():
            raise RuntimeError("BLOCKED_CGROUP_NOT_VISIBLE")
        print("SD_CI_CHROME_SERVICE_TASK_PRESSURE at=start "
              + json.dumps(service_task_pressure(root)), flush=True)

        actor = None
        count = 0
        stable_candidate = None
        stable_observations = 0
        # The popup input lane starts an additional pinned native client and
        # runs alongside nine Chrome CI lanes. Keep the exact native identity,
        # unique-window, cgroup and three stable geometry checks unchanged;
        # allow bounded extra cold-start observations only for this CI mode.
        # Parallel Chrome CI can start slowly under runner pressure. These
        # modes run extra native clients/windows but still require one exact
        # source/cgroup-matched native XDG + three stable geometry reads.
        extra_startup = (popup_mode or
                         e.get("STAGE_DIRECTOR_CI_CHROME_SUBSURFACE") == "1" or
                         e.get("STAGE_DIRECTOR_CI_CHROME_MULTI_SIBLING") == "1")
        startup_samples = 160 if extra_startup else 80
        for _ in range(startup_samples):
            if call("systemctl", "is-active", unit, check=False) != "active":
                logs = call("sudo", "-n", "journalctl", "-u", unit,
                            "-n", "75", "--no-pager", check=False)
                raise RuntimeError("BLOCKED_CHROME_SERVICE_EXITED\n"
                                   + logs[-3700:])
            pids = members(root)
            count = sum(1 for p in pids if (
                Path(f"/proc/{p}/comm").exists() and
                Path(f"/proc/{p}/comm").read_text().strip() == "chrome"))
            snapshot.update((p, st) for p in pids
                            if (st := proc_start(p)) is not None)
            tree = json.loads(call(swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
            matches = []
            for node in containers(tree):
                pid = node["pid"]
                rect = node.get("rect") or {}
                if (pid in pids and proc_start(pid) == snapshot.get(pid) and
                        node.get("shell") == "xdg_shell" and
                        rect.get("width", 0) >= 300 and
                        rect.get("height", 0) >= 200):
                    matches.append(node)
            # Avoid accepting a momentary sole toplevel while Chrome is
            # still adding/removing surfaces. In the pixel mode, require
            # THREE consecutive observations of the same unique native
            # PID, border and settled rectangle. Do not relax uniqueness.
            if count >= 3 and len(matches) == 1:
                candidate = matches[0]
                rect = candidate.get("rect") or {}
                key = (
                    candidate.get("pid"),
                    candidate.get("border"),
                    rect.get("x"), rect.get("y"),
                    rect.get("width"), rect.get("height"))
                if key == stable_candidate:
                    stable_observations += 1
                else:
                    stable_candidate = key
                    stable_observations = 1
                if stable_observations >= (3 if pixel_mode else 1):
                    actor = candidate
                    break
            else:
                stable_candidate = None
                stable_observations = 0
            time.sleep(0.15)
        if actor is None:
            # Read-only diagnostic: preserve actual mapped window dimensions
            # and border state instead of guessing why a unique XDG candidate
            # was not found. No untrusted title/app_id authorizes a candidate.
            try:
                final_tree = json.loads(call(swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
                (temp / "sd-chrome-failed-tree.json").write_text(
                    json.dumps(final_tree), encoding="utf-8")
                observed = [
                    {"pid": n.get("pid"), "shell": n.get("shell"),
                     "border": n.get("border"), "rect": n.get("rect"),
                     "cgroup_member": n.get("pid") in members(root),
                     "kernel_start_matches":
                         proc_start(n.get("pid")) == snapshot.get(n.get("pid"))}
                    for n in containers(final_tree)]
            except Exception as exc:
                observed = [{"read_error": str(exc)}]
            logs = call("sudo", "-n", "journalctl", "-u", unit,
                        "-n", "65", "--no-pager", check=False)
            raise RuntimeError("BLOCKED_NO_UNIQUE_LIVE_NATIVE_CHROME_TOPLEVEL "
                               f"chrome={count} group_pids={len(snapshot)} "
                               "observed=" + json.dumps(observed) + "\n" +
                               logs[-3500:])
        if pixel_mode and actor.get("border") != "none":
            raise RuntimeError("FAIL_NATIVE_CHROME_BORDER_NOT_SCOPED: "
                               + str(actor.get("border")))
        if pixel_mode:
            print("SD_CI_CHROME_PER_VIEW_DECORATION: "
                  "browser_border=none unrelated_border=normal "
                  "native_cgroup_verified=1 PASS", flush=True)
        client_pid = actor["pid"]
        # A mapped XDG root is actual native Sway provenance, not title or
        # advertised app_id; matching kernel starttime and exact service cgroup
        # gives a READ-ONLY candidate, not authorized browser/viewer custody.
        print(f"SD_CI_REAL_CHROME_WAYLAND_WINDOW: native_xdg=1 "
              f"mapped=1 actor_in_exact_service_cgroup=1 "
              f"kernel_starttime=1 browser_processes={count} "
              f"stable_native_observations={stable_observations} "
              "authority=NONE PASS", flush=True)
        if pixel_mode:
            # Never infer pixel containment from clip state or Sway hit tests.
            # The compositor test-only probe has independently checked the
            # exact browser's wl_client PID, kernel starttime and cgroup.
            verified_crop = False
            for _ in range(60):
                log = (runtime / "sway.log").read_text(
                    encoding="utf-8", errors="replace")
                if ("SD_CI_CHROME_PIXEL_SCENE native=1 cgroup=1"
                        in log and "hit_test_crop=PASS viewport=320x240" in log):
                    verified_crop = True
                    break
                time.sleep(0.1)
            if not verified_crop:
                raise RuntimeError("FAIL_NO_CGROUP_VERIFIED_CHROME_SCENE_CROP "
                                   + log[-3800:])
            # Read compositor screenshot AFTER the actual browser maps.
            # Keep evidence if pixel assertion fails, not just after PASS.
            time.sleep(0.4)
            crop_ppm = temp / "sd-chrome-crop.ppm"
            crop_png = temp / "sd-chrome-crop.png"
            tree_path = temp / "sd-chrome-mapped-tree.json"
            tree_path.write_text(json.dumps(tree), encoding="utf-8")
            call(grim, "-t", "ppm", str(crop_ppm), timeout=25)
            call(grim, "-t", "png", str(crop_png), timeout=25)
            pixel_report = call(
                 sys.executable,
                 "tests/stage_director/assert_real_chrome_clip_pixels.py",
                 str(temp / "sd-chrome-before.ppm"), str(crop_ppm),
                 str(tree_path), str(client_pid), timeout=30)
            # Preserve actual measured luma in GitHub logs, not only a green
            # child exit code; no operator screenshot or private URL involved.
            print(pixel_report, flush=True)
            if ("SD_CI_REAL_CHROME_RENDER_PIXELS:" not in pixel_report or
                    "no_left_top_right_bottom_leak=1" not in pixel_report):
                raise RuntimeError("FAIL_CHROME_PIXEL_EVIDENCE_MARKER_MISSING")

            if e.get("STAGE_DIRECTOR_CI_CHROME_FIRST_FRAME") == "1":
                # The actual output-build hook must release the source-verified
                # crop before the screenshot, never merely accept a scene hit.
                early_log = (runtime / "sway.log").read_text(
                    encoding="utf-8", errors="replace")
                hidden = early_log.find("SD_CI_FIRST_FRAME_MAP_HIDDEN ")
                released = early_log.find("SD_CI_FIRST_FRAME_RELEASE ")
                if (hidden < 0 or released < 0 or released <= hidden or
                        "SD_CI_FIRST_FRAME_CUSTODY_LOST" in early_log):
                    raise RuntimeError("FAIL_FIRST_FRAME_NATIVE_OUTPUT_GUARD " +
                                       early_log[-6000:])
                print("SD_CI_FIRST_FRAME_NATIVE_GUARD: result=PASS "
                      "map_hidden_before_layout=1 "
                      "output_build_crop_released=1 "
                      "real_screenshot_four_edges=1 "
                      "intermediate_pixels=UNMEASURED authority=NONE", flush=True)
            # Chrome can asynchronously alter its initial native geometry
            # after the first screenshot. Require a later stable rectangle
            # before sending PHASE2 or a second-window request. No relaxation.
            assert_chrome_main_rect_stable(
                e, temp, swaymsg, ipc, root, actor, snapshot)
            if sibling_mode:
                discover_chrome_second_toplevel(
                    e, temp, swaymsg, ipc, grim, unit, root, snapshot,
                    actor, crop_ppm, profile)
                return
            if popup_mode:
                verify_native_popup_witness(
                    e, temp, runtime, grim, unit, snapshot, root, crop_ppm)
            if e.get("STAGE_DIRECTOR_CI_CHROME_PENDING_EXIT") == "1":
                verify_pending_mask_exit(
                    e, temp, runtime, swaymsg, ipc, grim, unit, root,
                    snapshot, client_pid, profile)
                return
            if e.get("STAGE_DIRECTOR_CI_BROWSER_MASK") == "1":
                # CI host request (not Sway IPC, not Chrome external window
                # movement). The compositor only accepts this exact one-time
                # 0600 request from the disposable runner and revalidates
                # its native Chrome wl_client and Linux cgroup independently.
                request = temp / (
                    "sd-chrome-mask-" + e["GITHUB_RUN_ID"] + "-" +
                    e.get("GITHUB_RUN_ATTEMPT", "1") + ".request")
                fd = os.open(request,
                             os.O_CREAT | os.O_EXCL | os.O_WRONLY |
                             getattr(os, "O_NOFOLLOW", 0), 0o600)
                try:
                    os.write(fd, b"PHASE2\n")
                finally:
                    os.close(fd)
                evidence = False
                try:
                    for _ in range(100):
                        lines = (runtime / "sway.log").read_text(
                            encoding="utf-8", errors="replace")
                        if ("SD_CI_CHROME_MASK_REBIND" in lines and
                                "clip=520,240 400x300" in lines):
                            evidence = True
                            break
                        if ("SD_CI_CHROME_MASK_SOURCE_LOST" in lines or
                                "SD_CI_CHROME_MASK_REQUEST_INVALID" in lines):
                            break
                        time.sleep(0.1)
                finally:
                    request.unlink(missing_ok=True)
                if not evidence:
                    raise RuntimeError(
                        "FAIL_MASK_SCENE_REQUEST_NOT_ACCEPTED " + lines[-4500:])
                if proc_start(client_pid) != snapshot[client_pid] or (
                        client_pid not in members(root)):
                    raise RuntimeError("FAIL_BROWSER_PROCESS_IDENTITY_CHANGED")
                # A second screenshot in the SAME compositor must show the
                # new larger mask, old location black, and no outside pixels.
                time.sleep(0.35)
                second = temp / "sd-chrome-mask-shift.ppm"
                call(grim, "-t", "ppm", str(second), timeout=25)
                call(grim, "-t", "png",
                     str(temp / "sd-chrome-mask-shift.png"), timeout=25)
                shifted_tree = json.loads(call(
                    swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
                shifted_tree_file = temp / "sd-chrome-mask-shift-tree.json"
                shifted_tree_file.write_text(
                    json.dumps(shifted_tree), encoding="utf-8")
                mask_report = call(
                    sys.executable,
                    "tests/stage_director/assert_real_chrome_mask_shift_pixels.py",
                    str(temp / "sd-chrome-before.ppm"), str(crop_ppm),
                    str(second), str(tree_path), str(shifted_tree_file),
                    str(client_pid), timeout=30)
                print(mask_report, flush=True)
                if "SD_CI_CHROME_MASK_SHIFT_PIXELS: result=PASS" not in mask_report:
                    raise RuntimeError("FAIL_MASK_SCREENSHOT_MARKER_MISSING")
                # A second sequential request in the SAME isolated compositor.
                # The previous PHASE2 request has been unlinked before this
                # O_EXCL 0600 file is created, so an old file cannot replay.
                fd = os.open(request,
                             os.O_CREAT | os.O_EXCL | os.O_WRONLY |
                             getattr(os, "O_NOFOLLOW", 0), 0o600)
                try:
                    os.write(fd, b"PHASE3\n")
                finally:
                    os.close(fd)
                third_evidence = False
                try:
                    for _ in range(100):
                        lines = (runtime / "sway.log").read_text(
                            encoding="utf-8", errors="replace")
                        if ("SD_CI_CHROME_MASK_REBIND_PHASE3" in lines and
                                "clip=1040,560 280x220" in lines):
                            third_evidence = True
                            break
                        if ("SD_CI_CHROME_MASK_SOURCE_LOST" in lines or
                                "SD_CI_CHROME_MASK_REQUEST_INVALID" in lines):
                            break
                        time.sleep(0.1)
                finally:
                    request.unlink(missing_ok=True)
                if not third_evidence:
                    raise RuntimeError(
                        "FAIL_THIRD_MASK_REQUEST_NOT_ACCEPTED " + lines[-4500:])
                if proc_start(client_pid) != snapshot[client_pid] or (
                        client_pid not in members(root)):
                    raise RuntimeError("FAIL_BROWSER_PROCESS_IDENTITY_CHANGED_PHASE3")
                time.sleep(0.35)
                third = temp / "sd-chrome-mask-third.ppm"
                call(grim, "-t", "ppm", str(third), timeout=25)
                call(grim, "-t", "png",
                     str(temp / "sd-chrome-mask-third.png"), timeout=25)
                third_tree = json.loads(call(
                    swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
                third_tree_path = temp / "sd-chrome-mask-third-tree.json"
                third_tree_path.write_text(
                    json.dumps(third_tree), encoding="utf-8")
                multiple_report = call(
                    sys.executable,
                    "tests/stage_director/assert_real_chrome_multi_mask_pixels.py",
                    str(temp / "sd-chrome-before.ppm"), str(crop_ppm),
                    str(second), str(third), str(tree_path),
                    str(shifted_tree_file), str(third_tree_path),
                    str(client_pid), timeout=30)
                print(multiple_report, flush=True)
                if "SD_CI_CHROME_MULTI_MASK_PIXELS: result=PASS" not in multiple_report:
                    raise RuntimeError("FAIL_MULTI_MASK_SCREENSHOT_MARKER_MISSING")
                if e.get("STAGE_DIRECTOR_CI_CHROME_ABRUPT_EXIT") == "1":
                    # The exact browser systemd service is disposable and
                    # network-isolated, created by this harness, not a user's
                    # Chrome process. Kill its whole cgroup *without* asking
                    # Chrome to close gracefully. The compositor must unmap
                    # the native XDG surface and release its CI scene state.
                    marker_start = len((runtime / "sway.log").read_text(
                        encoding="utf-8", errors="replace"))
                    call("sudo", "-n", "systemctl", "kill",
                         "--signal=SIGKILL", "--kill-whom=all", unit,
                         timeout=20)
                    print("SD_CI_CHROME_ABRUPT_SIGNAL: "
                          "exact_test_service=1 signal=SIGKILL "
                          "control_group=all source=CI_only", flush=True)
                    clean_tree = None
                    for _ in range(100):
                        clean_tree = json.loads(call(
                            swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
                        still_mapped = any(
                            n.get("pid") == client_pid
                            for n in containers(clean_tree))
                        still_alive = any(
                            proc_start(p) == start
                            for p, start in snapshot.items())
                        still_in_group = bool(members(root))
                        new_log = (runtime / "sway.log").read_text(
                            encoding="utf-8", errors="replace")[marker_start:]
                        restored = (
                            "SD_CI_RENDER_PIXEL clip_restored_on_unmap=1"
                            in new_log)
                        if (not still_mapped and not still_alive and
                                not still_in_group and restored):
                            break
                        time.sleep(0.1)
                    else:
                        raise RuntimeError(
                            "FAIL_CHROME_ABORT_DID_NOT_REVOKE_SCENE "
                            f"still_mapped={still_mapped} "
                            f"still_alive={still_alive} "
                            f"still_in_group={still_in_group} "
                            f"restore_marker={restored} "
                            + new_log[-3200:])
                    # Screenshot is taken only AFTER the kernel-side SIGKILL,
                    # the exact native Wayland unmap, and compositor restore.
                    # Do not confuse process death with clearing stale pixels.
                    time.sleep(0.35)
                    after_kill_ppm = temp / "sd-chrome-abrupt-exit.ppm"
                    after_kill_png = temp / "sd-chrome-abrupt-exit.png"
                    after_kill_tree = temp / "sd-chrome-abrupt-exit-tree.json"
                    call(grim, "-t", "ppm", str(after_kill_ppm), timeout=25)
                    call(grim, "-t", "png", str(after_kill_png), timeout=25)
                    after_kill_tree.write_text(
                        json.dumps(clean_tree), encoding="utf-8")
                    abort_report = call(
                        sys.executable,
                        "tests/stage_director/assert_real_chrome_abrupt_exit_pixels.py",
                        str(temp / "sd-chrome-before.ppm"), str(third),
                        str(after_kill_ppm), str(third_tree_path),
                        str(after_kill_tree), str(client_pid), timeout=30)
                    print(abort_report, flush=True)
                    if ("SD_CI_CHROME_ABRUPT_EXIT_PIXELS: result=PASS"
                            not in abort_report):
                        raise RuntimeError(
                            "FAIL_CHROME_ABORT_PIXEL_PROOF_MISSING")
        call("sudo", "-n", "systemctl", "stop", unit)
        for _ in range(100):
            survivors = [p for p, st in snapshot.items() if proc_start(p) == st]
            current = members(root)
            if not survivors and not current and not profile.parent.exists():
                break
            time.sleep(0.1)
        else:
            raise RuntimeError(f"FAIL_BROWSER_WAYLAND_CGROUP_TEARDOWN "
                               f"survivors={survivors} members={current} "
                               f"profile_runtime_exists={profile.parent.exists()}")
        after = json.loads(call(swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
        if any(n.get("pid") == client_pid for n in containers(after)):
            raise RuntimeError("FAIL_STALE_BROWSER_XDG_AFTER_STOP")
        print("SD_CI_REAL_CHROME_WAYLAND_TEARDOWN: original_pids_gone=1 "
              "native_xdg_unmapped=1 cgroup_members=0 private_profile_gone=1 "
              "watch=OFF scene_clip=OFF input=OFF PASS", flush=True)
    finally:
        if started and root and root.exists():
            print("SD_CI_CHROME_SERVICE_TASK_PRESSURE at=cleanup "
                  + json.dumps(service_task_pressure(root)), flush=True)
        if started:
            call("sudo", "-n", "systemctl", "stop", unit, check=False)
            call("sudo", "-n", "systemctl", "reset-failed",
                 unit, check=False)


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"SD_CI_REAL_CHROME_WAYLAND_BLOCKED: {exc}",
              file=sys.stderr)
        raise SystemExit(1)
