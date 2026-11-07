#!/usr/bin/env python3
"""CI-only native Sway seatop guest-unmap and replacement-custody witness.

This deliberately destroys a real mapped Wayland guest during an active
native move and later an active native resize. The disposable compositor
MUST revoke its raw peer pointer on unref. A new guest reusing the same
app_id MUST remain independent until the exact fresh container ID and PID
are supplied to a new test-only lease and a new seatop begins.

This is a lifecycle proof, NOT a trusted production lease implementation.
"""
from __future__ import annotations

from pathlib import Path

import ci_social_real_wayland_pair as b


def verify_unmap_lifecycle(*, swaymsg, ipc, env, foot, grim, runtime, lease,
                           host, guest, other, actors, ids, host_box,
                           guest_box, unrelated_box, log,
                           move_fn, resize_fn):
    if (env.get("CI") != "true" or env.get("GITHUB_ACTIONS") != "true"
            or env.get("STAGE_DIRECTOR_CI_SOCIAL_NATIVE_MOVE") != "1"
            or env.get("STAGE_DIRECTOR_CI_SOCIAL_FRAME_TRACE") != "1"
            or env.get("RUNNER_TEMP") != "/home/runner/work/_temp"):
        raise RuntimeError("BLOCKED_SOCIAL_UNMAP_NOT_CI")
    if not Path(runtime).is_relative_to(Path(env["RUNNER_TEMP"])):
        raise RuntimeError("BLOCKED_SOCIAL_UNMAP_NOT_PRIVATE_RUNTIME")

    def command(text):
        b.sway_command(swaymsg, ipc, env, text)

    def exact(app, proc, rect):
        return b.wait_for(swaymsg, ipc, env, app, proc.pid, rect)

    def start_host(kind, rect):
        criteria = f'[con_id="{ids[b.APP_HOST]}"]'
        command(f"{criteria} focus")
        command(f'seat - cursor set {rect[0]+45} {rect[1]+135}')
        command(f"{criteria} focus")
        command("seat - cursor ci-social-start-"+kind+" 1")

    def advance(dx, dy):
        command(f"seat - cursor ci-social-step {dx} {dy}")

    def end():
        command("seat - cursor ci-social-end 1")

    def set_lease(guest_id, guest_pid):
        # The lease is only the old CI fixture's authority model; the
        # negative test requires exact ID/PID, not just reused app_id.
        lease.write_text(
            f"{ids[b.APP_HOST]} {guest_id} {host.pid} {guest_pid}\n",
            encoding="utf-8")
        lease.chmod(0o600)

    def read_sway_log():
        log.flush()
        return (Path(runtime) / "sway.log").read_text(
            encoding="utf-8", errors="replace")

    old_id = ids[b.APP_GUEST]
    old_pid = guest.pid

    # Existing deliberately invalid PID test left the host and old guest
    # separated. Re-align with normal Sway geometry before native testing.
    guest_box = b.guest_from_chassis(host_box)
    b.locate(swaymsg, ipc, env, old_id, guest_box)
    exact(b.APP_HOST, host, host_box)
    exact(b.APP_GUEST, guest, guest_box)
    exact(b.APP_OTHER, other, unrelated_box)
    set_lease(old_id, old_pid)
    pre = read_sway_log()
    prev_move_revokes = pre.count(
        "SD_CI_SOCIAL_NATIVE_REVOKE peer_unmapped=1")
    prev_resize_revokes = pre.count(
        "SD_CI_SOCIAL_NATIVE_RESIZE_REVOKED peer_unmapped=1")
    prev_denials = pre.count(
        "SD_CI_SOCIAL_NATIVE_ATTACH denied=1")

    # 1. Move while genuinely paired. Kill the guest BEFORE releasing the
    # pointer, then issue two more native pointer motions on the still-active
    # host seatop. Neither may access a destroyed guest.
    start_host("move", host_box)
    try:
        advance(10, 5)
        host_box = (host_box[0]+10, host_box[1]+5, *host_box[2:])
        guest_box = b.guest_from_chassis(host_box)
        exact(b.APP_HOST, host, host_box)
        exact(b.APP_GUEST, guest, guest_box)

        b.quiet_stop(guest)
        b.wait_for(swaymsg, ipc, env, b.APP_GUEST, old_pid,
                   present=False)
        if read_sway_log().count(
                "SD_CI_SOCIAL_NATIVE_REVOKE peer_unmapped=1") != (
                prev_move_revokes+1):
            raise RuntimeError("SD_CI_SOCIAL_UNMAP_MOVE_PEER_NOT_REVOKED")

        advance(14, 8)
        host_box = (host_box[0]+14, host_box[1]+8, *host_box[2:])
        exact(b.APP_HOST, host, host_box)
        exact(b.APP_OTHER, other, unrelated_box)

        guest2 = b.foot_process(
            foot, b.APP_GUEST, "SOCIAL-CI-GUEST-REPLACEMENT-1",
            b.GUEST_COLOR, runtime, env)
        actors.append(guest2)
        replacement = b.wait_for(
            swaymsg, ipc, env, b.APP_GUEST, guest2.pid)
        new_id = replacement["id"]
        if new_id == old_id or guest2.pid == old_pid:
            raise RuntimeError("SD_CI_SOCIAL_UNMAP_IDENTITY_REUSED")
        sentinel = (1340, 220, 320, 240)
        b.locate(swaymsg, ipc, env, new_id, sentinel)
        exact(b.APP_GUEST, guest2, sentinel)
        advance(12, 5)
        host_box = (host_box[0]+12, host_box[1]+5, *host_box[2:])
        exact(b.APP_HOST, host, host_box)
        exact(b.APP_GUEST, guest2, sentinel)
        exact(b.APP_OTHER, other, unrelated_box)
    finally:
        end()
    print("SD_CI_SOCIAL_ACTIVE_MOVE_UNMAP "
          "native_peer_unref=PASS host_continued=PASS "
          "replacement_during_same_seatop_independent=PASS",
          flush=True)

    # A new seatop using the obsolete lease MUST NOT pick up the
    # same-app-id replacement. Then explicitly dock and re-authorize
    # that exact replacement and require it to follow the next motion.
    stale_dx, stale_dy = move_fn(
        swaymsg, ipc, env, ids[b.APP_HOST], b.APP_HOST, host,
        b.APP_GUEST, guest2, [(11, 4)])
    host_box = (host_box[0]+stale_dx, host_box[1]+stale_dy,
                *host_box[2:])
    exact(b.APP_HOST, host, host_box)
    exact(b.APP_GUEST, guest2, sentinel)
    if read_sway_log().count(
            "SD_CI_SOCIAL_NATIVE_ATTACH denied=1") <= prev_denials:
        raise RuntimeError("SD_CI_SOCIAL_REPLACEMENT_STALE_LEASE_ACCEPTED")

    ids[b.APP_GUEST] = new_id
    guest_box = b.guest_from_chassis(host_box)
    b.locate(swaymsg, ipc, env, new_id, guest_box)
    exact(b.APP_GUEST, guest2, guest_box)
    set_lease(new_id, guest2.pid)
    dx, dy = move_fn(swaymsg, ipc, env,
                     ids[b.APP_HOST], b.APP_HOST, host,
                     b.APP_GUEST, guest2, [(13, 6)])
    host_box = (host_box[0]+dx, host_box[1]+dy, *host_box[2:])
    guest_box = b.guest_from_chassis(host_box)
    exact(b.APP_HOST, host, host_box)
    exact(b.APP_GUEST, guest2, guest_box)
    print("SD_CI_SOCIAL_REPLACEMENT_REBIND "
          "old_lease=DENIED new_exact_id_pid=ACCEPTED "
          "reused_app_id_no_authority=1", flush=True)

    # 2. Independently prove the resize seatop's peer-unref behavior.
    # Begin a genuine native resize and require a fresh configure/ACK.
    guest2_id = new_id
    guest2_pid = guest2.pid
    start_host("resize", host_box)
    try:
        advance(18, 10)
        host_box = (
            host_box[0], host_box[1],
            host_box[2]+18, host_box[3]+10)
        guest_box = b.guest_from_chassis(host_box)
        exact(b.APP_HOST, host, host_box)
        exact(b.APP_GUEST, guest2, guest_box)

        b.quiet_stop(guest2)
        b.wait_for(swaymsg, ipc, env, b.APP_GUEST,
                   guest2_pid, present=False)
        if read_sway_log().count(
                "SD_CI_SOCIAL_NATIVE_RESIZE_REVOKED peer_unmapped=1") != (
                prev_resize_revokes+1):
            raise RuntimeError("SD_CI_SOCIAL_UNMAP_RESIZE_PEER_NOT_REVOKED")

        advance(11, 6)
        host_box = (
            host_box[0], host_box[1],
            host_box[2]+11, host_box[3]+6)
        exact(b.APP_HOST, host, host_box)
        exact(b.APP_OTHER, other, unrelated_box)
    finally:
        end()
    print("SD_CI_SOCIAL_ACTIVE_RESIZE_UNMAP "
          "native_peer_unref=PASS host_continued=PASS "
          "unrelated_unchanged=PASS", flush=True)

    guest3 = b.foot_process(
        foot, b.APP_GUEST, "SOCIAL-CI-GUEST-REPLACEMENT-2",
        b.GUEST_COLOR, runtime, env)
    actors.append(guest3)
    third = b.wait_for(
        swaymsg, ipc, env, b.APP_GUEST, guest3.pid)
    third_id = third["id"]
    if third_id in (old_id, guest2_id) or guest3.pid in (
            old_pid, guest2_pid):
        raise RuntimeError("SD_CI_SOCIAL_RESIZE_UNMAP_STALE_ID_REUSED")
    ids[b.APP_GUEST] = third_id
    guest_box = b.guest_from_chassis(host_box)
    b.locate(swaymsg, ipc, env, third_id, guest_box)
    exact(b.APP_GUEST, guest3, guest_box)
    set_lease(third_id, guest3.pid)
    dw, dh = resize_fn(
        swaymsg, ipc, env, ids[b.APP_HOST], b.APP_HOST, host,
        b.APP_GUEST, guest3, [(14, 8)])
    host_box = (
        host_box[0], host_box[1], host_box[2]+dw, host_box[3]+dh)
    guest_box = b.guest_from_chassis(host_box)
    exact(b.APP_HOST, host, host_box)
    exact(b.APP_GUEST, guest3, guest_box)
    exact(b.APP_OTHER, other, unrelated_box)

    command(f'[con_id="{third_id}"] focus')
    b.capture(grim, Path(runtime) / "rebind-native-after-unmap.ppm", env, [
        ("rail", (host_box[0]+25,host_box[1]+155), b.HOST_COLOR),
        ("guest", (guest_box[0]+170,guest_box[1]+155), b.GUEST_COLOR),
        ("unrelated", (unrelated_box[0]+130,unrelated_box[1]+110),
         b.OTHER_COLOR),
        ("background", (25,25), (0,0,0)),
    ])
    output_log = read_sway_log()
    for tag in ("SD_CI_SOCIAL_NATIVE_REVOKE peer_unmapped=1",
                "SD_CI_SOCIAL_NATIVE_RESIZE_REVOKED peer_unmapped=1"):
        if tag not in output_log:
            raise RuntimeError("SD_CI_SOCIAL_UNMAP_MISSING_REVOCATION "+tag)
    if (f"guest_id={third_id} rect=PASS scene=PASS"
            not in output_log):
        raise RuntimeError(
            "SD_CI_SOCIAL_REBOUND_OUTPUT_COMMIT_NOT_VERIFIED")
    print("SD_CI_SOCIAL_NATIVE_LIFECYCLE final=PASS "
          "active_move_unmap=1 active_resize_unmap=1 "
          "same_app_replacements=2 fresh_id_pid_required=1 "
          "rebound_output_commits=PASS pixels=PASS "
          "production_identity=NOT_IMPLEMENTED "
          "live_desktop=UNCHANGED", flush=True)
