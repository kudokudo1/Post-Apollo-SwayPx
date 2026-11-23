#!/usr/bin/env python3
"""Strict native OUTPUT render-commit witness, never selected XDG presentation.

A real accepted output state and a real presented output event may coexist.
Their coexistence does not prove which XDG buffer was drawn, was current,
was included in that presentation, or was final and settled.
"""
import re
import sys
from pathlib import Path
from ci_assert_real_view_buffer_scan import validate_view_buffer_scan

COMMIT = re.compile(
    r"TIMEW_CI_RENDER_COMMIT lifetime=(\d+) sequence=(\d+) "
    r"scene_state_built=1 output_commit_accepted=1 real_callback=1 "
    r"same_mapped_output=1 "
    r"xdg_rendered=UNVERIFIED xdg_contributed=UNVERIFIED "
    r"xdg_final_presented=UNVERIFIED "
    r"client_ack=UNVERIFIED source=UNENROLLED c5_spent=NONE "
    r"receipt=NONE move=OFF retry=OFF authority=NONE$"
)
SEAL = re.compile(
    r"TIMEW_CI_RENDER_COMMIT_SEAL lifetime=(\d+) "
    r"accepted_output_commits=(\d+) real_unmap=1 "
    r"xdg_final_presented=UNVERIFIED authority=NONE$"
)
SNAP = re.compile(
    r"TIMEW_CI_NATIVE_SCENE_SNAPSHOT lifetime=(\d+) sequence=(\d+) "
)
UNMAP = re.compile(r"TIMEW_CI_REAL_XDG_UNMAPPED gen=(\d+)(?:\s|$)")
DRIFT = "TIMEW_CI_RENDER_COMMIT_DRIFT"

def validate_output_render_commit(lines, mode):
    parent = validate_view_buffer_scan(lines, mode)
    native_snapshots, commits, seals, unmaps = {}, {}, {}, {}
    for idx, line in enumerate(lines):
        if DRIFT in line:
            raise ValueError("real_output_commit_context_drift")
        m = SNAP.search(line)
        if m:
            key = (int(m[1]), int(m[2]))
            if key in native_snapshots:
                raise ValueError("duplicate_native_scene_snapshot")
            native_snapshots[key] = idx
        m = COMMIT.search(line)
        if m:
            key = (int(m[1]), int(m[2]))
            if key in commits:
                raise ValueError("duplicate_or_replayed_render_commit")
            commits[key] = idx
        elif "TIMEW_CI_RENDER_COMMIT lifetime=" in line:
            raise ValueError("forged_or_promoted_render_commit")
        m = SEAL.search(line)
        if m:
            generation = int(m[1])
            if generation in seals:
                raise ValueError("duplicate_output_commit_seal")
            seals[generation] = (idx, int(m[2]))
        elif "TIMEW_CI_RENDER_COMMIT_SEAL" in line:
            raise ValueError("forged_or_malformed_commit_seal")
        m = UNMAP.search(line)
        if m:
            generation = int(m[1])
            if generation in unmaps:
                raise ValueError("duplicate_native_unmap")
            unmaps[generation] = idx

    if set(seals) != {1, 2} or set(unmaps) != {1, 2}:
        raise ValueError("missing_render_commit_lifetime_seal")
    if len(commits) < 2 or {key[0] for key in commits} != {1, 2}:
        raise ValueError("missing_real_output_commit_per_xdg_lifetime")
    for generation in (1, 2):
        per_life = [key for key in commits if key[0] == generation]
        seal_idx, count = seals[generation]
        if count != len(per_life):
            raise ValueError("render_commit_seal_count_mismatch")
        if not seal_idx < unmaps[generation]:
            raise ValueError("render_commit_seal_after_unmap")
        for key in per_life:
            if key not in native_snapshots:
                raise ValueError("orphan_render_commit_no_real_scene_snapshot")
            if not native_snapshots[key] < commits[key] < seal_idx:
                raise ValueError("render_commit_before_scene_or_after_seal")

    return {
        "accepted_output_state_commits": len(commits),
        "real_xdg_lifetimes": 2,
        "view_buffer_scans": parent["matched_view_buffer_scans"],
        "output_present_events": parent["output_present_events"],
        "ambiguous_ready_timeout_hold": parent["ambiguous_ready_timeout_hold"],
        "selected_xdg_rendered": False,
        "xdg_contributed_to_present": False,
        "selected_xdg_final_frame": False,
        "settled": False,
        "safe_actions": [],
    }

def main():
    if len(sys.argv) != 3 or sys.argv[2] not in ("normal", "forced_timeout"):
        raise SystemExit("BLOCKED_OUTPUT_RENDER_COMMIT_CLI_ARGUMENTS")
    lines = Path(sys.argv[1]).read_text(errors="replace").splitlines()
    try:
        result = validate_output_render_commit(lines, sys.argv[2])
    except (ValueError, TypeError, KeyError, OverflowError) as exc:
        print("TIMEW_CI_RENDER_COMMIT_FAIL reason=" + str(exc), file=sys.stderr)
        raise SystemExit(1)
    print(
        f"TIMEW_CI_RENDER_COMMIT_{sys.argv[2].upper()}_PASS "
        f"accepted_output_state_commits={result['accepted_output_state_commits']} "
        f"real_xdg_lifetimes={result['real_xdg_lifetimes']} "
        f"view_buffer_scans={result['view_buffer_scans']} "
        f"output_present_events={result['output_present_events']} "
        f"ambiguous_ready_timeout_hold={result['ambiguous_ready_timeout_hold']} "
        "render_state=OUTPUT_ONLY accepted=OUTPUT_ONLY "
        "xdg_buffer_contributed=UNVERIFIED xdg_final_frame=UNVERIFIED "
        "client_ack=UNVERIFIED source=UNENROLLED c5_spent=NONE "
        "receipt=NONE native_move=OFF retry=OFF safe_actions=NONE "
        "settled=0 authority=NONE"
    )

if __name__ == "__main__":
    main()
