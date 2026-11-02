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


def containers(node):
    if isinstance(node, dict):
        if node.get("pid") and node.get("type") == "con":
            yield node
        for sub in node.get("nodes", []):
            yield from containers(sub)
        for sub in node.get("floating_nodes", []):
            yield from containers(sub)


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
            tree = json.loads(call(swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
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
        tree = json.loads(call(swaymsg, "-s", ipc, "-r", "-t", "get_tree"))
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
        os.write(fd, b"PENDING\\n")
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
             "--property=TasksMax=128",
             "--property=KillMode=control-group",
             "--property=SendSIGKILL=yes",
             "--property=TimeoutStopSec=3s",
             "--setenv=HOME=" + str(profile.parent),
             "--setenv=XDG_CONFIG_HOME=" + str(profile.parent),
             "--setenv=XDG_CACHE_HOME=" + str(profile.parent),
             "--setenv=XDG_RUNTIME_DIR=" + str(runtime),
             "--setenv=WAYLAND_DISPLAY=" + e["WAYLAND_DISPLAY"],
             "--setenv=XDG_SESSION_TYPE=wayland",
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

        actor = None
        count = 0
        stable_candidate = None
        stable_observations = 0
        for _ in range(80):
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
