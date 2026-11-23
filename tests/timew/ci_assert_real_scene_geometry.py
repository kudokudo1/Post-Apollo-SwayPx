#!/usr/bin/env python3
"""CI-only REAL XDG post-arrange logical geometry provenance checker.

The source is the disposable in-process SwayPX native snapshot observer.
Log text is never independently signed or a trusted external receipt.
"""
import math
import re
import struct
import sys
from pathlib import Path
from ci_assert_real_instruction_outcomes import validate

SNAP = re.compile(
    r"TIMEW_CI_NATIVE_SCENE_SNAPSHOT lifetime=(\d+) sequence=(\d+) "
    r"current_x=([0-9a-f]{16}) current_y=([0-9a-f]{16}) "
    r"current_w=([0-9a-f]{16}) current_h=([0-9a-f]{16}) "
    r"pending_x=([0-9a-f]{16}) pending_y=([0-9a-f]{16}) "
    r"pending_w=([0-9a-f]{16}) pending_h=([0-9a-f]{16}) "
    r"scale=([0-9a-f]{16}) transform=(\d+) "
    r"output_lx=(-?\d+) output_ly=(-?\d+) "
    r"output_w=(\d+) output_h=(\d+) "
    r"native_source=REAL_XDG same_context=1 "
    r"ack=0 frame=0 epoch_proof=0 source=UNENROLLED "
    r"selected=0 spent=0 receipt=NONE action=NONE$"
)
SCENE = re.compile(r"TIMEW_CI_OUTCOME_SCENE lifetime=(\d+) sequence=(\d+) ")
SEAL = re.compile(r"TIMEW_CI_OUTCOME_SEAL lifetime=(\d+) ")
DEFER = re.compile(r"TIMEW_CI_NATIVE_SCENE_(?:DEFER|DRIFT) lifetime=(\d+) ")

def finite_bits(hex_value, *, positive=False):
    value = struct.unpack(">d", bytes.fromhex(hex_value))[0]
    if not math.isfinite(value) or (value == 0 and math.copysign(1, value) < 0):
        raise ValueError("invalid_or_noncanonical_binary64")
    if positive and value <= 0:
        raise ValueError("invalid_native_dimension_or_scale")
    if abs(value) > 2**31:
        raise ValueError("outside_native_logical_bound")
    return value

def validate_scene_geometry(lines, mode):
    prior = validate(lines, mode)
    stages, seals, snapshots, deferred = {}, {}, {}, 0
    for line_number, line in enumerate(lines):
        if m := SCENE.search(line):
            key = (int(m[1]), int(m[2]))
            if key in stages:
                raise ValueError("duplicate_parent_scene")
            stages[key] = line_number
        if m := SEAL.search(line):
            generation = int(m[1])
            if generation in seals:
                raise ValueError("duplicate_parent_seal")
            seals[generation] = line_number
        if m := DEFER.search(line):
            if int(m[1]) not in (1, 2):
                raise ValueError("invalid_deferred_lifetime")
            deferred += 1
        if m := SNAP.search(line):
            generation, sequence = int(m[1]), int(m[2])
            key = (generation, sequence)
            if key in snapshots:
                raise ValueError("duplicate_native_scene_snapshot")
            for n in range(3, 11):
                finite_bits(m[n], positive=n in (5, 6, 9, 10))
            finite_bits(m[11], positive=True)
            if not 0 <= int(m[12]) <= 7:
                raise ValueError("invalid_output_transform")
            if any(not -2**31 <= int(m[n]) < 2**31 for n in (13, 14)):
                raise ValueError("invalid_output_origin")
            if any(not 1 <= int(m[n]) < 2**31 for n in (15, 16)):
                raise ValueError("invalid_output_dimensions")
            snapshots[key] = line_number
        elif "TIMEW_CI_NATIVE_SCENE_SNAPSHOT" in line:
            raise ValueError("forged_or_malformed_native_scene_snapshot")

    if len(snapshots) < 2 or {x[0] for x in snapshots} != {1, 2}:
        raise ValueError("missing_real_scene_snapshot_for_a_lifetime")
    if set(seals) != {1, 2}:
        raise ValueError("missing_native_lifetime_seal")
    for key, index in snapshots.items():
        if key not in stages or not stages[key] < index < seals[key[0]]:
            raise ValueError("out_of_order_or_unbound_native_scene_snapshot")

    return {
        "snapshots": len(snapshots),
        "deferred": deferred,
        "lifetimes": 2,
        "qualified_diagnostic": prior["qualified"],
        "ambiguous_hold": prior["ambiguous_ready_timeout_hold"],
        "settled": False,
        "safe_actions": [],
    }

def main():
    if len(sys.argv) != 3 or sys.argv[2] not in ("normal", "forced_timeout"):
        raise SystemExit("BLOCKED_NATIVE_SCENE_CLI_ARGUMENTS")
    lines = Path(sys.argv[1]).read_text(errors="replace").splitlines()
    try:
        result = validate_scene_geometry(lines, sys.argv[2])
    except (ValueError, TypeError, IndexError, OverflowError) as exc:
        print("TIMEW_CI_NATIVE_SCENE_FAIL reason=" + str(exc), file=sys.stderr)
        raise SystemExit(1)
    print(
        f"TIMEW_CI_NATIVE_SCENE_{sys.argv[2].upper()}_PASS "
        f"real_post_arrange_snapshots={result['snapshots']} "
        f"real_xdg_lifetimes={result['lifetimes']} "
        f"deferred_or_drift={result['deferred']} "
        f"native_outcome_diagnostic={result['qualified_diagnostic']} "
        f"ambiguous_ready_timeout_hold={result['ambiguous_hold']} "
        "exact_binary64_current_and_pending=1 real_output_context=1 "
        "output_fit=UNVERIFIED ack=UNVERIFIED frame=UNVERIFIED "
        "selected=NONE enrolled_source=NONE c5_spent=NONE "
        "protected_receipt=NONE native_move=OFF retry=OFF "
        "safe_actions=NONE settled=0 authority=NONE"
    )

if __name__ == "__main__":
    main()
