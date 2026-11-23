#!/usr/bin/env python3
"""Strict OUTPUT-level present chronology; never a selected XDG frame receipt."""
import re
import sys
from pathlib import Path
from ci_assert_real_scene_geometry import validate_scene_geometry

SNAP = re.compile(r"TIMEW_CI_NATIVE_SCENE_SNAPSHOT lifetime=(\d+) sequence=(\d+) ")
PRESENT = re.compile(
    r"TIMEW_CI_OUTPUT_PRESENT lifetime=(\d+) sequence=(\d+) "
    r"output_presented=1 real_callback=1 same_output_context=1 "
    r"selected_xdg_frame=UNVERIFIED final_view_frame=UNVERIFIED "
    r"source=UNENROLLED ack=UNVERIFIED c5_spent=NONE "
    r"receipt=NONE move=OFF retry=OFF authority=NONE$"
)
SEAL = re.compile(
    r"TIMEW_CI_OUTPUT_PRESENT_SEAL lifetime=(\d+) "
    r"observed_output_presentations=(\d+) real_unmap=1 "
    r"selected_xdg_frame=UNVERIFIED authority=NONE$"
)
UNMAP = re.compile(r"TIMEW_CI_REAL_XDG_UNMAPPED gen=(\d+)(?:\s|$)")

def validate_output_present(lines, mode):
    prior = validate_scene_geometry(lines, mode)
    native_scene = {}
    presents = {}
    seals = {}
    unmaps = {}
    for n, line in enumerate(lines):
        m = SNAP.search(line)
        if m:
            key = (int(m[1]), int(m[2]))
            if key in native_scene:
                raise ValueError("duplicate_native_scene")
            native_scene[key] = n
        m = PRESENT.search(line)
        if m:
            key = (int(m[1]), int(m[2]))
            if key in presents:
                raise ValueError("duplicate_or_replayed_output_present")
            presents[key] = n
        elif "TIMEW_CI_OUTPUT_PRESENT lifetime=" in line:
            raise ValueError("malformed_or_positive_output_present")
        m = SEAL.search(line)
        if m:
            gen = int(m[1])
            if gen in seals:
                raise ValueError("duplicate_output_present_seal")
            seals[gen] = (n, int(m[2]))
        elif "TIMEW_CI_OUTPUT_PRESENT_SEAL" in line:
            raise ValueError("malformed_or_positive_output_seal")
        m = UNMAP.search(line)
        if m:
            gen = int(m[1])
            if gen in unmaps:
                raise ValueError("duplicate_xdg_unmap")
            unmaps[gen] = n
    if set(seals) != {1, 2} or set(unmaps) != {1, 2}:
        raise ValueError("missing_real_output_lifetime_seals")
    if len(presents) < 2 or {k[0] for k in presents} != {1, 2}:
        raise ValueError("no_real_output_present_for_each_lifetime")
    for generation in (1, 2):
        per_life = [k for k in presents if k[0] == generation]
        if seals[generation][1] != len(per_life):
            raise ValueError("output_present_seal_count_mismatch")
        if not seals[generation][0] < unmaps[generation]:
            raise ValueError("output_present_seal_after_native_unmap")
        for key in per_life:
            if key not in native_scene:
                raise ValueError("orphan_output_present_no_scene_snapshot")
            if not native_scene[key] < presents[key] < seals[generation][0]:
                raise ValueError("output_present_before_scene_or_after_seal")
    return {
        "output_presented_observations": len(presents),
        "native_scene_observations": prior["snapshots"],
        "ambiguous_ready_timeout_hold": prior["ambiguous_hold"],
        "safe_actions": [],
        "selected_xdg_frame_verified": False,
        "settled": False,
    }

def main():
    if len(sys.argv) != 3 or sys.argv[2] not in ("normal", "forced_timeout"):
        raise SystemExit("BLOCKED_OUTPUT_PRESENT_CI_CLI_ARGUMENTS")
    lines = Path(sys.argv[1]).read_text(errors="replace").splitlines()
    try:
        result = validate_output_present(lines, sys.argv[2])
    except (ValueError, TypeError, OverflowError) as exc:
        print("TIMEW_CI_OUTPUT_PRESENT_FAIL reason=" + str(exc), file=sys.stderr)
        raise SystemExit(1)
    print(
        f"TIMEW_CI_OUTPUT_PRESENT_{sys.argv[2].upper()}_PASS "
        f"real_presented_output_events={result['output_presented_observations']} "
        f"native_scene_snapshots={result['native_scene_observations']} "
        f"ambiguous_ready_timeout_hold={result['ambiguous_ready_timeout_hold']} "
        "same_output=DIAGNOSTIC_ONLY selected_xdg_final_frame=UNVERIFIED "
        "selected_view_contributed=UNVERIFIED output_fit=UNVERIFIED "
        "operator_selection=NONE source=UNENROLLED c5_spent=NONE "
        "protected_receipt=NONE move=OFF retry=OFF safe_actions=NONE "
        "settled=0 authority=NONE"
    )

if __name__ == "__main__":
    main()
