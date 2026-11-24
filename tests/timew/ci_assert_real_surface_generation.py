#!/usr/bin/env python3
"""Real XDG root wl_surface committed-state generation across output events.

surface.current.seq is uint32 per-surface state, NOT immutable buffer content,
a client configure ACK, or source-authenticated final presented frame.
Mismatch/supersession/wrap are UNKNOWN/HOLD, never automatic MOVE/RETRY.
"""
import re
import sys
from pathlib import Path
from ci_assert_real_root_sample import validate_root_sample

SAMPLE = re.compile(
    r"TIMEW_CI_XDG_GEN_SAMPLE lifetime=(\d+) sequence=(\d+) "
    r"surface_state_seq=(\d+) root_client_object_match=([01]) "
    r"exact_native_root=1 real_scenefx_sample=1 "
    r"immutable_buffer=UNVERIFIED final_frame=UNVERIFIED "
    r"ack=UNVERIFIED source=UNENROLLED c5_spent=NONE "
    r"receipt=NONE move=OFF authority=NONE$"
)
COMMIT = re.compile(
    r"TIMEW_CI_XDG_GEN_COMMIT lifetime=(\d+) sequence=(\d+) "
    r"output_commit_seq=(\d+) sampled_this_scene=([01]) "
    r"sampled_surface_seq=(\d+) current_surface_seq=(\d+) "
    r"accepted_output_commit=1 "
    r"immutable_buffer=UNVERIFIED final_frame=UNVERIFIED "
    r"ack=UNVERIFIED source=UNENROLLED c5_spent=NONE "
    r"receipt=NONE move=OFF authority=NONE$"
)
PRESENT = re.compile(
    r"TIMEW_CI_XDG_GEN_PRESENT lifetime=(\d+) sequence=(\d+) "
    r"output_commit_seq=(\d+) current_surface_seq=(\d+) "
    r"real_output_present=1 "
    r"sampled_buffer_in_presented_frame=UNVERIFIED "
    r"immutable_buffer=UNVERIFIED final_frame=UNVERIFIED "
    r"ack=UNVERIFIED source=UNENROLLED c5_spent=NONE "
    r"receipt=NONE move=OFF authority=NONE$"
)
SEAL = re.compile(
    r"TIMEW_CI_XDG_GEN_SEAL lifetime=(\d+) "
    r"samples=(\d+) output_commits=(\d+) output_presents=(\d+) "
    r"real_unmap=1 final_frame=UNVERIFIED authority=NONE$"
)
PARENT_SAMPLE = re.compile(
    r"TIMEW_CI_ROOT_SAMPLE lifetime=(\d+) sequence=(\d+) "
)
PARENT_COMMIT = re.compile(
    r"TIMEW_CI_OUTPUT_SEQ_COMMIT lifetime=(\d+) sequence=(\d+) "
    r"output_commit_seq=(\d+) "
)
PARENT_PRESENT = re.compile(
    r"TIMEW_CI_OUTPUT_SEQ_PRESENT lifetime=(\d+) sequence=(\d+) "
    r"output_commit_seq=(\d+) "
)
UNMAP = re.compile(r"TIMEW_CI_REAL_XDG_UNMAPPED gen=(\d+)(?:\s|$)")

def uint32(raw):
    n = int(raw)
    if not 0 <= n < 2**32:
        raise ValueError("non_uint32_surface_generation_or_output_serial")
    return n

def validate_surface_generation(lines, mode):
    parent = validate_root_sample(lines, mode)
    samples, commits, presents, seals, unmaps = {}, {}, {}, {}, {}
    parent_sample, parent_commit, parent_present = {}, {}, {}
    for i, line in enumerate(lines):
        for regexp, target, label in (
            (PARENT_SAMPLE, parent_sample, "native_sample"),
            (PARENT_COMMIT, parent_commit, "native_output_commit"),
            (PARENT_PRESENT, parent_present, "native_output_present"),
        ):
            m = regexp.search(line)
            if m:
                key = (int(m[1]), int(m[2]))
                if key in target:
                    raise ValueError("duplicate_" + label)
                target[key] = (i, int(m[3]) if len(m.groups()) == 3 else None)

        for regexp, target, label in (
            (SAMPLE, samples, "surface_generation_sample"),
            (COMMIT, commits, "surface_generation_output_commit"),
            (PRESENT, presents, "surface_generation_output_present"),
        ):
            m = regexp.search(line)
            if m:
                key = (int(m[1]), int(m[2]))
                if key in target:
                    raise ValueError("duplicate_" + label)
                vals = tuple(uint32(x) for x in m.groups()[2:])
                target[key] = (i, vals)
            elif "TIMEW_CI_XDG_GEN_" + {
                    "surface_generation_sample":"SAMPLE",
                    "surface_generation_output_commit":"COMMIT",
                    "surface_generation_output_present":"PRESENT"}[label] \
                    + " lifetime=" in line:
                raise ValueError("malformed_or_trust_promoted_" + label)
        m = SEAL.search(line)
        if m:
            gen = int(m[1])
            if gen in seals:
                raise ValueError("duplicate_xdg_generation_seal")
            seals[gen] = (i, int(m[2]), int(m[3]), int(m[4]))
        elif "TIMEW_CI_XDG_GEN_SEAL" in line:
            raise ValueError("forged_xdg_generation_seal")
        m = UNMAP.search(line)
        if m:
            gen = int(m[1])
            if gen in unmaps:
                raise ValueError("duplicate_native_unmap")
            unmaps[gen] = i

    if set(seals) != {1, 2} or set(unmaps) != {1, 2} or \
            {key[0] for key in samples} != {1, 2}:
        raise ValueError("missing_real_xdg_surface_generation_by_lifetime")
    if set(samples) != set(parent_sample) or set(commits) != set(parent_commit) or \
            set(presents) != set(parent_present):
        raise ValueError("surface_generation_missing_or_orphan_parent_event")

    # At the instant of sampling, the compositor's root committed-state seq
    # is captured. It may be a no-buffer state or may wrap: neither is a
    # durable content identifier; 0 remains entirely unqualified.
    client_object_matches = 0
    for key, (i, (seq, object_match)) in samples.items():
        if not parent_sample[key][0] < i < seals[key[0]][0]:
            raise ValueError("root_surface_state_before_native_sample_or_after_seal")
        client_object_matches += object_match
    sampled_unchanged_at_commit = 0
    sampled_superseded_at_commit = 0
    unsampled_output_commits = 0
    by_serial = {}
    for key, (i, vals) in commits.items():
        serial, sampled_flag, sample_seq, now = vals
        actual_i, actual_serial = parent_commit[key]
        if serial != actual_serial or not actual_i < i < seals[key[0]][0]:
            raise ValueError("wrong_actual_wlroots_output_serial_or_commit_order")
        if serial == 0 or serial in by_serial:
            raise ValueError("reused_or_wrapped_wlroots_output_serial")
        sample = samples.get(key)
        if bool(sampled_flag) != bool(sample):
            raise ValueError("sampled_flag_does_not_match_actual_root_sample")
        if sample:
            if sample_seq != sample[1][0] or not sample[0] < i:
                raise ValueError("sampled_surface_seq_not_bound_to_real_scene")
            if sample_seq != 0 and sample_seq == now:
                sampled_unchanged_at_commit += 1
            else:
                sampled_superseded_at_commit += 1
        elif sample_seq != 0:
            raise ValueError("unsampled_output_commit_fabricated_root_generation")
        else:
            unsampled_output_commits += 1
        by_serial[serial] = (key, i, sample_seq if sample else None,
                             now, bool(sample))
    sampled_unchanged_at_present = 0
    sampled_superseded_at_present = 0
    unbound_output_present_hold = 0
    for key, (i, vals) in presents.items():
        serial, now = vals
        actual_i, actual_serial = parent_present[key]
        if serial != actual_serial or not actual_i < i < seals[key[0]][0]:
            raise ValueError("wrong_actual_output_present_serial_or_order")
        source = by_serial.get(serial)
        # A PRESENT serial can be from an earlier scene within this XDG lifetime.
        # Never attribute it to the current scene's sampled root generation.
        if not source or source[0] != key or not source[1] < i:
            unbound_output_present_hold += 1
            continue
        _, _, sampled_seq, at_commit, sampled = source
        if not sampled:
            continue
        if sampled_seq and sampled_seq == at_commit == now:
            sampled_unchanged_at_present += 1
        else:
            sampled_superseded_at_present += 1

    for gen in (1, 2):
        si, ns, nc, np = seals[gen]
        if (not si < unmaps[gen] or
                ns != sum(k[0] == gen for k in samples) or
                nc != sum(k[0] == gen for k in commits) or
                np != sum(k[0] == gen for k in presents)):
            raise ValueError("xdg_generation_lifetime_seal_count_or_order")
    # Generation replacement (including wrap) is a recorded UNKNOWN/HOLD,
    # not an excuse to fail the diagnostic or manufacture a positive frame.
    # The inherited SceneFX validator already requires actual root samples
    # for both XDG lifetimes. No unchanged generation is an authority grant.

    return {
        "actual_root_state_samples": len(samples),
        "root_client_object_matches_at_sample_diagnostic": client_object_matches,
        "root_client_object_unmatched_at_sample_hold": len(samples) - client_object_matches,
        "client_buffer_attachment_continuity": False,
        "accepted_output_commits": len(commits),
        "actual_output_present_events": len(presents),
        "unchanged_at_output_commit_diagnostic": sampled_unchanged_at_commit,
        "superseded_or_wrapped_at_commit_hold": sampled_superseded_at_commit,
        "unchanged_at_output_present_diagnostic": sampled_unchanged_at_present,
        "superseded_or_wrapped_at_present_hold": sampled_superseded_at_present,
        "unsampled_output_commits_hold": unsampled_output_commits,
        "unbound_output_present_hold": unbound_output_present_hold,
        "parent_output_serial_matches": parent["native_output_serial_matches"],
        "ambiguous_ready_timeout_hold": parent["ambiguous_ready_timeout_hold"],
        "immutable_buffer_identified": False,
        "client_configure_ack": False,
        "selected_xdg_final_frame": False,
        "safe_actions": [],
        "settled": False,
    }

def main():
    if len(sys.argv) != 3 or sys.argv[2] not in ("normal", "forced_timeout"):
        raise SystemExit("BLOCKED_XDG_GENERATION_CLI_ARGUMENTS")
    lines = Path(sys.argv[1]).read_text(errors="replace").splitlines()
    try:
        r = validate_surface_generation(lines, sys.argv[2])
    except (TypeError, ValueError, KeyError, OverflowError) as exc:
        print("TIMEW_CI_XDG_GEN_FAIL reason=" + str(exc), file=sys.stderr)
        raise SystemExit(1)
    print(
        f"TIMEW_CI_XDG_GEN_{sys.argv[2].upper()}_PASS "
        f"actual_root_state_samples={r['actual_root_state_samples']} "
        f"root_client_object_matches_at_sample={r['root_client_object_matches_at_sample_diagnostic']} "
        f"root_client_object_unmatched_at_sample_hold={r['root_client_object_unmatched_at_sample_hold']} "
        "client_buffer_attachment_continuity=UNVERIFIED "
        f"accepted_output_commits={r['accepted_output_commits']} "
        f"output_presents={r['actual_output_present_events']} "
        f"root_generation_unchanged_at_commit={r['unchanged_at_output_commit_diagnostic']} "
        f"root_generation_unchanged_at_present={r['unchanged_at_output_present_diagnostic']} "
        f"superseded_commit_hold={r['superseded_or_wrapped_at_commit_hold']} "
        f"superseded_present_hold={r['superseded_or_wrapped_at_present_hold']} "
        f"unsampled_output_commits_hold={r['unsampled_output_commits_hold']} "
        f"unbound_output_present_hold={r['unbound_output_present_hold']} "
        f"ambiguous_ready_timeout_hold={r['ambiguous_ready_timeout_hold']} "
        "immutable_buffer=UNVERIFIED xdg_final_frame=UNVERIFIED "
        "client_ack=UNVERIFIED enrolled_source=NONE c5_spent=NONE "
        "protected_receipt=NONE move=OFF retry=OFF safe_actions=NONE "
        "settled=0 authority=NONE"
    )

if __name__ == "__main__":
    main()
