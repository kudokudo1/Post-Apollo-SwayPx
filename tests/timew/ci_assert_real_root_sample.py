#!/usr/bin/env python3
"""Exact native XDG ROOT scene-buffer output_sample, deny-only.

Pinned SceneFX emits this event after adding a texture to the render pass or
preparing direct scanout. Output commit and PRESENT remain separate evidence.
This is not proof of a final presented XDG buffer, configure ACK or authority.
"""
import re
import sys
from pathlib import Path

from ci_assert_real_output_commit_serial import validate_output_commit_serial

SAMPLE = re.compile(
    r"TIMEW_CI_ROOT_SAMPLE lifetime=(\d+) sequence=(\d+) "
    r"direct_scanout=([01]) exact_root_surface=1 sampled_scene_buffer=1 "
    r"real_scenefx_event=1 same_primary_output=1 "
    r"render_pass_or_scanout=1 accepted_output_commit=UNVERIFIED "
    r"selected_xdg_final_presented=UNVERIFIED "
    r"configure_ack=UNVERIFIED source=UNENROLLED c5_spent=NONE "
    r"receipt=NONE move=OFF retry=OFF authority=NONE$"
)
BIND = re.compile(
    r"TIMEW_CI_ROOT_SAMPLE_BIND lifetime=(\d+) "
    r"exact_native_xdg_root=1 primary_output=1 "
    r"receipt=NONE authority=NONE$"
)
SEAL = re.compile(
    r"TIMEW_CI_ROOT_SAMPLE_SEAL lifetime=(\d+) "
    r"root_samples=(\d+) subscriptions=(\d+) "
    r"real_unmap=1 settled=0 authority=NONE$"
)
SCENE = re.compile(
    r"TIMEW_CI_NATIVE_SCENE_SNAPSHOT lifetime=(\d+) sequence=(\d+) "
)
OUTPUT_COMMIT = re.compile(
    r"TIMEW_CI_OUTPUT_SEQ_COMMIT lifetime=(\d+) sequence=(\d+) "
)
UNMAP = re.compile(r"TIMEW_CI_REAL_XDG_UNMAPPED gen=(\d+)(?:\s|$)")

def validate_root_sample(lines, mode):
    parent = validate_output_commit_serial(lines, mode)
    samples, binds, seals, scenes, commits, unmaps = {}, {}, {}, {}, {}, {}
    for i, line in enumerate(lines):
        if "TIMEW_CI_ROOT_SAMPLE_DRIFT" in line or \
                "TIMEW_CI_ROOT_SAMPLE_REVOKED" in line:
            raise ValueError("root_scene_buffer_lifetime_or_output_revoked")
        m = SAMPLE.search(line)
        if m:
            key = (int(m[1]), int(m[2]))
            if key in samples:
                raise ValueError("duplicate_or_replayed_root_sample")
            samples[key] = (i, int(m[3]))
        elif "TIMEW_CI_ROOT_SAMPLE lifetime=" in line:
            raise ValueError("forged_root_sample_or_positive_authority")
        m = BIND.search(line)
        if m:
            binds.setdefault(int(m[1]), []).append(i)
        elif "TIMEW_CI_ROOT_SAMPLE_BIND" in line:
            raise ValueError("forged_root_sample_bind")
        m = SEAL.search(line)
        if m:
            gen = int(m[1])
            if gen in seals:
                raise ValueError("duplicate_root_sample_lifetime_seal")
            seals[gen] = (i, int(m[2]), int(m[3]))
        elif "TIMEW_CI_ROOT_SAMPLE_SEAL" in line:
            raise ValueError("forged_root_sample_lifetime_seal")
        for pattern, table, label in (
            (SCENE, scenes, "scene"),
            (OUTPUT_COMMIT, commits, "commit"),
        ):
            m = pattern.search(line)
            if m:
                key = (int(m[1]), int(m[2]))
                if key in table:
                    raise ValueError("duplicate_parent_" + label)
                table[key] = i
        m = UNMAP.search(line)
        if m:
            gen = int(m[1])
            if gen in unmaps:
                raise ValueError("duplicate_native_unmap")
            unmaps[gen] = i

    if set(seals) != {1, 2} or set(unmaps) != {1, 2} or \
            set(binds) != {1, 2} or \
            {key[0] for key in samples} != {1, 2}:
        raise ValueError("missing_real_root_sample_for_xdg_lifetime")
    if any(len(x) != 1 for x in binds.values()):
        raise ValueError("duplicated_root_subscription")
    for key, (i, direct_scanout) in samples.items():
        gen = key[0]
        if key not in scenes or key not in commits or \
                not binds[gen][0] < i or not scenes[key] < i < commits[key] or \
                not i < seals[gen][0] < unmaps[gen]:
            raise ValueError("root_sample_not_bound_to_prior_scene_and_commit")
    for gen in (1, 2):
        count = sum(key[0] == gen for key in samples)
        idx, claimed_samples, claimed_subscriptions = seals[gen]
        if (count != claimed_samples or
                claimed_subscriptions != len(binds[gen]) or
                not binds[gen][0] < idx < unmaps[gen]):
            raise ValueError("root_sample_seal_count_or_order_mismatch")
    return {
        "native_root_samples": len(samples),
        "composited_root_samples": sum(not value[1] for value in samples.values()),
        "direct_scanout_root_samples": sum(value[1] for value in samples.values()),
        "native_output_serial_matches": parent["same_output_serials_proven_in_log"],
        "ambiguous_ready_timeout_hold": parent["ambiguous_ready_timeout_hold"],
        "selected_xdg_render_completed": False,
        "selected_xdg_final_presented": False,
        "safe_actions": [],
        "settled": False,
    }

def main():
    if len(sys.argv) != 3 or sys.argv[2] not in ("normal", "forced_timeout"):
        raise SystemExit("BLOCKED_ROOT_SAMPLE_CLI_ARGUMENTS")
    lines = Path(sys.argv[1]).read_text(errors="replace").splitlines()
    try:
        r = validate_root_sample(lines, sys.argv[2])
    except (ValueError, TypeError, KeyError, OverflowError) as e:
        print(f"TIMEW_CI_ROOT_SAMPLE_FAIL reason={e}", file=sys.stderr)
        raise SystemExit(1)
    print(
        f"TIMEW_CI_ROOT_SAMPLE_{sys.argv[2].upper()}_PASS "
        f"real_xdg_root_samples={r['native_root_samples']} "
        f"composited={r['composited_root_samples']} "
        f"direct_scanout={r['direct_scanout_root_samples']} "
        f"native_output_serial_matches={r['native_output_serial_matches']} "
        f"ambiguous_ready_timeout_hold={r['ambiguous_ready_timeout_hold']} "
        "root_scene_buffer=REAL_NATIVE_SCENEFX_SAMPLE "
        "gpu_completion=UNVERIFIED xdg_final_frame=UNVERIFIED "
        "configure_ack=UNVERIFIED enrolled_source=NONE c5_spent=NONE "
        "receipt=NONE move=OFF retry=OFF safe_actions=NONE "
        "settled=0 authority=NONE"
    )

if __name__ == "__main__":
    main()
