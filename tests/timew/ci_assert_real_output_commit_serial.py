#!/usr/bin/env python3
"""Real wlroots output commit_seq/present.commit_seq correlation, deny-only.

Matching the actual OUTPUT commit serial means an OUTPUT frame was accepted
then the referenced OUTPUT submission was presented. It does NOT prove which
XDG buffer contributed, latest configure ACK, view final frame, or a receipt.
A PRESENT before its COMMIT observation remains UNBOUND; no retroactive match.
"""
import re
import sys
from pathlib import Path

from ci_assert_real_output_render_commit import validate_output_render_commit

COMMIT = re.compile(
    r"TIMEW_CI_OUTPUT_SEQ_COMMIT lifetime=(\d+) sequence=(\d+) "
    r"output_commit_seq=(\d+) accepted_output_state=1 real_wlroots=1 "
    r"same_mapped_xdg_output=1 xdg_buffer=UNVERIFIED "
    r"source=UNENROLLED c5_spent=NONE receipt=NONE "
    r"move=OFF authority=NONE$"
)
PRESENT = re.compile(
    r"TIMEW_CI_OUTPUT_SEQ_PRESENT lifetime=(\d+) sequence=(\d+) "
    r"output_commit_seq=(\d+) output_presented=1 real_wlroots_event=1 "
    r"same_mapped_xdg_output=1 "
    r"xdg_buffer=UNVERIFIED xdg_final_frame=UNVERIFIED "
    r"ack=UNVERIFIED source=UNENROLLED c5_spent=NONE "
    r"receipt=NONE move=OFF authority=NONE$"
)
SEAL = re.compile(
    r"TIMEW_CI_OUTPUT_SEQ_SEAL lifetime=(\d+) "
    r"observed_output_commits=(\d+) observed_output_presents=(\d+) "
    r"real_unmap=1 xdg_final_frame=UNVERIFIED authority=NONE$"
)
SNAP = re.compile(
    r"TIMEW_CI_NATIVE_SCENE_SNAPSHOT lifetime=(\d+) sequence=(\d+) "
)
PARENT_COMMIT = re.compile(
    r"TIMEW_CI_RENDER_COMMIT lifetime=(\d+) sequence=(\d+) "
)
PARENT_PRESENT = re.compile(
    r"TIMEW_CI_OUTPUT_PRESENT lifetime=(\d+) sequence=(\d+) "
)
UNMAP = re.compile(r"TIMEW_CI_REAL_XDG_UNMAPPED gen=(\d+)(?:\s|$)")
DRIFT = "TIMEW_CI_OUTPUT_SEQ_DRIFT"

def checked_serial(raw):
    value = int(raw)
    if not 1 <= value < 2**32:
        raise ValueError("missing_or_wrapped_native_output_commit_seq")
    return value

def validate_output_commit_serial(lines, mode):
    parent = validate_output_render_commit(lines, mode)
    snapshots, old_commits, old_presents = {}, {}, {}
    commits, presents, seals, unmaps = {}, {}, {}, {}
    for i, line in enumerate(lines):
        if DRIFT in line:
            raise ValueError("actual_output_commit_seq_context_drift")
        for pattern, table, name in (
            (SNAP, snapshots, "native_scene"),
            (PARENT_COMMIT, old_commits, "parent_output_commit"),
            (PARENT_PRESENT, old_presents, "parent_output_present"),
        ):
            m = pattern.search(line)
            if m:
                key = (int(m[1]), int(m[2]))
                if key in table:
                    raise ValueError("duplicate_" + name)
                table[key] = i
        m = COMMIT.search(line)
        if m:
            key = (int(m[1]), int(m[2]))
            if key in commits:
                raise ValueError("duplicate_wlroots_output_seq_commit_record")
            commits[key] = (i, checked_serial(m[3]))
        elif "TIMEW_CI_OUTPUT_SEQ_COMMIT lifetime=" in line:
            raise ValueError("malformed_or_forged_wlroots_commit_seq")
        m = PRESENT.search(line)
        if m:
            key = (int(m[1]), int(m[2]))
            if key in presents:
                raise ValueError("duplicate_wlroots_output_seq_present_record")
            presents[key] = (i, checked_serial(m[3]))
        elif "TIMEW_CI_OUTPUT_SEQ_PRESENT lifetime=" in line:
            raise ValueError("malformed_or_forged_wlroots_present_seq")
        m = SEAL.search(line)
        if m:
            gen = int(m[1])
            if gen in seals:
                raise ValueError("duplicate_serial_seal")
            seals[gen] = (i, int(m[2]), int(m[3]))
        elif "TIMEW_CI_OUTPUT_SEQ_SEAL" in line:
            raise ValueError("malformed_serial_seal")
        m = UNMAP.search(line)
        if m:
            gen = int(m[1])
            if gen in unmaps:
                raise ValueError("duplicate_native_xdg_unmap")
            unmaps[gen] = i

    if set(seals) != {1, 2} or set(unmaps) != {1, 2}:
        raise ValueError("missing_true_native_xdg_lifetime_seal")
    if set(k[0] for k in commits) != {1, 2} or \
            set(k[0] for k in presents) != {1, 2}:
        raise ValueError("missing_actual_commit_or_present_serial_by_lifetime")
    if len(commits) != parent["accepted_output_state_commits"]:
        raise ValueError("native_commit_serial_count_mismatch")
    if len(presents) != parent["output_present_events"]:
        raise ValueError("native_present_serial_count_mismatch")

    used_output_commit_serials = {}
    for key, (index, serial) in commits.items():
        generation = key[0]
        if serial in used_output_commit_serials:
            raise ValueError("replayed_or_wrapped_output_commit_serial")
        used_output_commit_serials[serial] = (key, index)
        if key not in snapshots or key not in old_commits or \
                not snapshots[key] < old_commits[key] < index:
            raise ValueError("serial_commit_not_bound_to_actual_scene_and_commit")
        if not index < seals[generation][0] < unmaps[generation]:
            raise ValueError("serial_commit_after_seal_or_native_unmap")

    seen_present_serials = set()
    matched = unbound = 0
    for key, (index, serial) in presents.items():
        generation = key[0]
        if serial in seen_present_serials:
            raise ValueError("duplicate_or_replayed_present_commit_serial")
        seen_present_serials.add(serial)
        if key not in snapshots or key not in old_presents or \
                not snapshots[key] < old_presents[key] < index:
            raise ValueError("serial_present_not_bound_to_actual_output_present")
        if not index < seals[generation][0] < unmaps[generation]:
            raise ValueError("serial_present_after_seal_or_native_unmap")

        prior = used_output_commit_serials.get(serial)
        if prior is None:
            unbound += 1
            continue
        prior_key, commit_line = prior
        if prior_key[0] == generation and commit_line < index:
            matched += 1
        else:
            # An early PRESENT, a stale prior-lifetime serial or an event
            # from another output is not retroactively authenticated.
            unbound += 1

    for generation in (1, 2):
        actual_commits = sum(key[0] == generation for key in commits)
        actual_presents = sum(key[0] == generation for key in presents)
        seal_line, count_commits, count_presents = seals[generation]
        if (count_commits != actual_commits or
                count_presents != actual_presents or
                not seal_line < unmaps[generation]):
            raise ValueError("serial_seal_count_or_unmap_order_mismatch")

    return {
        "real_output_commits": len(commits),
        "real_output_presents": len(presents),
        "same_output_serials_proven_in_log": matched,
        "unbound_or_early_presentation_hold": unbound,
        "real_xdg_lifetimes": 2,
        "ambiguous_ready_timeout_hold": parent["ambiguous_ready_timeout_hold"],
        "selected_xdg_buffer_rendered": False,
        "selected_xdg_final_frame": False,
        "safe_actions": [],
        "settled": False,
    }

def main():
    if len(sys.argv) != 3 or sys.argv[2] not in ("normal", "forced_timeout"):
        raise SystemExit("BLOCKED_WLROOTS_OUTPUT_SEQ_CLI_ARGUMENTS")
    lines = Path(sys.argv[1]).read_text(errors="replace").splitlines()
    try:
        result = validate_output_commit_serial(lines, sys.argv[2])
    except (ValueError, TypeError, KeyError, OverflowError) as exc:
        print("TIMEW_CI_OUTPUT_SEQ_FAIL reason=" + str(exc), file=sys.stderr)
        raise SystemExit(1)
    print(
        f"TIMEW_CI_OUTPUT_SEQ_{sys.argv[2].upper()}_PASS "
        f"actual_output_commits={result['real_output_commits']} "
        f"actual_output_presentations={result['real_output_presents']} "
        f"causally_matched_output_serials={result['same_output_serials_proven_in_log']} "
        f"unbound_early_or_stale_hold={result['unbound_or_early_presentation_hold']} "
        f"real_xdg_lifetimes={result['real_xdg_lifetimes']} "
        f"ambiguous_ready_timeout_hold={result['ambiguous_ready_timeout_hold']} "
        "buffer_contribution=UNVERIFIED selected_final_frame=UNVERIFIED "
        "client_ack=UNVERIFIED enrolled_source=NONE c5_spent=NONE "
        "protected_receipt=NONE move=OFF retry=OFF "
        "safe_actions=NONE settled=0 authority=NONE"
    )

if __name__ == "__main__":
    main()
