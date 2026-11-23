#!/usr/bin/env python3
"""Classify real XDG scene-buffer traversal without granting view-frame proof."""
import re
import sys
from pathlib import Path

from ci_assert_real_output_present import validate_output_present

SCAN = re.compile(
    r"TIMEW_CI_VIEW_BUFFER_SCAN lifetime=(\d+) sequence=(\d+) "
    r"ancestor_view=REAL_XDG scene_surface=1 primary_output=1 "
    r"frame_done_iterator=1 "
    r"contributed_to_present=UNVERIFIED rendered=UNVERIFIED "
    r"selected_final_frame=UNVERIFIED "
    r"client_ack=UNVERIFIED c5_spent=NONE source=UNENROLLED "
    r"receipt=NONE move=OFF retry=OFF authority=NONE$"
)
SEAL = re.compile(
    r"TIMEW_CI_VIEW_BUFFER_SCAN_SEAL lifetime=(\d+) "
    r"native_scans=(\d+) real_unmap=1 "
    r"selected_final_frame=UNVERIFIED authority=NONE$"
)
SNAP = re.compile(
    r"TIMEW_CI_NATIVE_SCENE_SNAPSHOT lifetime=(\d+) sequence=(\d+) "
)
UNMAP = re.compile(r"TIMEW_CI_REAL_XDG_UNMAPPED gen=(\d+)(?:\s|$)")
DRIFT = "TIMEW_CI_VIEW_BUFFER_DRIFT"

def validate_view_buffer_scan(lines, mode):
    # #182 / #187 / #188 validators enforce original COMMIT→BIND,
    # SCENE→snapshot and output PRESENT chronology before this stage.
    parent = validate_output_present(lines, mode)
    snaps, scans, seals, unmapped = {}, {}, {}, {}
    for idx, line in enumerate(lines):
        if DRIFT in line:
            raise ValueError("native_view_buffer_context_drift")
        m = SNAP.search(line)
        if m:
            key = (int(m[1]), int(m[2]))
            if key in snaps:
                raise ValueError("duplicate_native_scene_snapshot")
            snaps[key] = idx
        m = SCAN.search(line)
        if m:
            key = (int(m[1]), int(m[2]))
            if key in scans:
                raise ValueError("duplicate_or_replayed_scene_buffer_scan")
            scans[key] = idx
        elif "TIMEW_CI_VIEW_BUFFER_SCAN lifetime=" in line:
            raise ValueError("forged_scene_buffer_or_trust_promotion")
        m = SEAL.search(line)
        if m:
            generation = int(m[1])
            if generation in seals:
                raise ValueError("duplicate_scene_buffer_seal")
            seals[generation] = (idx, int(m[2]))
        elif "TIMEW_CI_VIEW_BUFFER_SCAN_SEAL" in line:
            raise ValueError("forged_or_bad_scene_buffer_seal")
        m = UNMAP.search(line)
        if m:
            generation = int(m[1])
            if generation in unmapped:
                raise ValueError("duplicate_native_unmap")
            unmapped[generation] = idx

    if set(seals) != {1, 2} or set(unmapped) != {1, 2}:
        raise ValueError("missing_scene_buffer_lifetime_seal")
    if len(scans) < 2 or {key[0] for key in scans} != {1, 2}:
        raise ValueError("missing_native_xdg_buffer_scan_per_lifetime")
    for generation in (1, 2):
        actual = [key for key in scans if key[0] == generation]
        seal_idx, claimed_count = seals[generation]
        if claimed_count != len(actual):
            raise ValueError("buffer_scan_seal_count_mismatch")
        if not seal_idx < unmapped[generation]:
            raise ValueError("scan_seal_after_xdg_unmap")
        for key in actual:
            if key not in snaps:
                raise ValueError("orphan_view_scan_no_actual_scene")
            if not snaps[key] < scans[key] < seal_idx:
                raise ValueError("buffer_scan_before_scene_or_after_unmap")
    return {
        "matched_view_buffer_scans": len(scans),
        "real_xdg_lifetimes": 2,
        "output_present_events": parent["output_presented_observations"],
        "ambiguous_ready_timeout_hold": parent["ambiguous_ready_timeout_hold"],
        "view_buffer_presented": False,
        "selected_final_frame": False,
        "safe_actions": [],
        "settled": False,
    }

def main():
    if len(sys.argv) != 3 or sys.argv[2] not in ("normal", "forced_timeout"):
        raise SystemExit("BLOCKED_VIEW_BUFFER_SCAN_CLI_ARGUMENTS")
    lines = Path(sys.argv[1]).read_text(errors="replace").splitlines()
    try:
        result = validate_view_buffer_scan(lines, sys.argv[2])
    except (ValueError, TypeError, KeyError, OverflowError) as exc:
        print("TIMEW_CI_VIEW_BUFFER_SCAN_FAIL reason=" + str(exc), file=sys.stderr)
        raise SystemExit(1)
    print(
        f"TIMEW_CI_VIEW_BUFFER_{sys.argv[2].upper()}_PASS "
        f"actual_xdg_scene_buffer_scans={result['matched_view_buffer_scans']} "
        f"real_xdg_lifetimes={result['real_xdg_lifetimes']} "
        f"actual_output_present_events={result['output_present_events']} "
        f"ambiguous_ready_timeout_hold={result['ambiguous_ready_timeout_hold']} "
        "same_view_ancestor=1 primary_output=1 scene_surface=1 "
        "contributed_to_present=UNVERIFIED rendered=UNVERIFIED "
        "selected_final_frame=UNVERIFIED ack=UNVERIFIED source=UNENROLLED "
        "c5_spent=NONE receipt=NONE move=OFF retry=OFF "
        "safe_actions=NONE settled=0 authority=NONE"
    )

if __name__ == "__main__":
    main()
