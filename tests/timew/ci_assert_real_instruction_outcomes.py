#!/usr/bin/env python3
"""Strict CI-only outcome chronology checker; never a TimeW receipt verifier.

Consumes actual instrumented native callback logs, not trusted wire packets.
Even complete ordinary READY/CURRENT/SCENE remains NO AUTHORITY; timeout
remains UNKNOWN/HOLD regardless of later matching CURRENT or SCENE.
"""
import re
import sys
from pathlib import Path

EVENT = re.compile(
    r"TIMEW_CI_OUTCOME_(COMMIT|READY|TIMEOUT|CURRENT|SCENE) "
    r"lifetime=(\d+) sequence=(\d+) configured=([01]) "
    r"real_callback=1 receipt=NONE authority=NONE"
)
SEAL = re.compile(
    r"TIMEW_CI_OUTCOME_SEAL lifetime=(\d+) "
    r"committed_instructions=(\d+) real_unmap=1 settled=0 authority=NONE"
)
OPEN = re.compile(r"TIMEW_CI_LINEAGE_OPEN lifetime=(\d+) transaction=(\d+)")
REVOKE = re.compile(r"TIMEW_CI_LINEAGE_REVOKED lifetime=(\d+) transaction=(\d+)")
BIND = re.compile(
    r"TIMEW_CI_REAL_INSTRUCTION_BIND lifetime=(\d+) commit_sequence=(\d+)"
)
UNMAP = re.compile(r"TIMEW_CI_REAL_XDG_UNMAPPED gen=(\d+)")
EXPECTED = {1, 2}

def validate(lines, mode):
    if mode not in ("normal", "forced_timeout"):
        raise ValueError("invalid_mode")
    starts, revokes, seals, unmaps = {}, {}, {}, {}
    seen = {}
    commits = []
    binds = {}
    for line_number, line in enumerate(lines):
        if ("TIMEW_CI_REAL_INSTRUCTION_FAIL" in line or
                "TIMEW_CI_REAL_XDG_FAIL" in line or
                "TIMEW_CI_LINEAGE_FAIL" in line):
            raise ValueError("upstream_native_failure")
        for regex, mapping, label in (
            (OPEN, starts, "open"), (REVOKE, revokes, "revoke"),
            (SEAL, seals, "seal"), (UNMAP, unmaps, "unmap")
        ):
            m = regex.search(line)
            if m:
                generation = int(m[1])
                if generation in mapping:
                    raise ValueError("duplicate_" + label)
                mapping[generation] = (
                    line_number, int(m[2]) if m.lastindex and m.lastindex > 1
                    else None
                )
        m = BIND.search(line)
        if m:
            key = (int(m[1]), int(m[2]))
            if key in binds:
                raise ValueError("duplicate_parent_commit")
            binds[key] = line_number
        m = EVENT.search(line)
        if m:
            stage, lifetime, sequence, configured = (
                m[1], int(m[2]), int(m[3]), int(m[4])
            )
            if lifetime not in EXPECTED or sequence <= 0:
                raise ValueError("unknown_lifetime_or_sequence")
            key = (lifetime, sequence)
            if stage == "COMMIT":
                if key in seen:
                    raise ValueError("duplicate_commit")
                seen[key] = {"configured": configured, "events": [(stage, line_number)]}
                commits.append(key)
            else:
                if key not in seen:
                    raise ValueError("orphan_event")
                r = seen[key]
                if r["configured"] != configured:
                    raise ValueError("configured_fork")
                if any(e == stage for e, _ in r["events"]):
                    raise ValueError("duplicate_stage")
                r["events"].append((stage, line_number))
        elif "TIMEW_CI_OUTCOME_" in line and not (
                "TIMEW_CI_OUTCOME_OVERLAY_PASS" in line or
                SEAL.search(line)):
            raise ValueError("malformed_or_unauthorized_outcome_line")

    if any(set(x) != EXPECTED for x in (starts, revokes, seals, unmaps)):
        raise ValueError("missing_or_extra_lifetime_boundary")
    if len(commits) < 2 or not all(s > 0 for _, s in commits):
        raise ValueError("missing_real_instructions")
    if [s for _, s in commits] != sorted(set(s for _, s in commits)):
        raise ValueError("nonmonotonic_or_reused_sequence")
    if set(commits) != set(binds):
        raise ValueError("parent_native_commit_mismatch")
    if not starts[1][0] < unmaps[1][0] < starts[2][0] < unmaps[2][0]:
        raise ValueError("overlapping_lifetimes")

    qualified = {1: 0, 2: 0}
    for generation in EXPECTED:
        life_commits = [key for key in commits if key[0] == generation]
        if not life_commits or seals[generation][1] != len(life_commits):
            raise ValueError("seal_count_mismatch")
        if not (
                starts[generation][0] < seals[generation][0] <
                revokes[generation][0] < unmaps[generation][0]
        ):
            raise ValueError("seal_revoke_unmap_order")
        if starts[generation][1] != revokes[generation][1]:
            raise ValueError("parent_lineage_fork")
        for key in life_commits:
            r = seen[key]
            events = r["events"]
            names = [e for e, _ in events]
            if names[0] != "COMMIT":
                raise ValueError("missing_commit_anchor")
            if any(not (starts[generation][0] < idx < seals[generation][0])
                   for _, idx in events):
                raise ValueError("event_outside_lifetime")
            if "SCENE" in names and (
                    "CURRENT" not in names or
                    names.index("CURRENT") > names.index("SCENE")):
                raise ValueError("scene_without_current")
            if r["configured"] and "CURRENT" in names and not (
                    "READY" in names or "TIMEOUT" in names):
                raise ValueError("configured_current_without_ready_or_timeout")
            if "READY" in names and (
                    not r["configured"] or
                    ("CURRENT" in names and
                     names.index("READY") > names.index("CURRENT"))):
                raise ValueError("invalid_ready_order")
            if "TIMEOUT" in names and "CURRENT" in names and (
                    names.index("TIMEOUT") > names.index("CURRENT")
            ):
                raise ValueError("late_timeout")
            if r["configured"] and all(x in names for x in ("CURRENT", "SCENE")):
                if mode == "normal":
                    if "READY" in names and "TIMEOUT" not in names:
                        qualified[generation] += 1
                else:
                    if "TIMEOUT" in names:
                        qualified[generation] += 1
        if not qualified[generation]:
            raise ValueError("missing_requested_real_outcome_path_" + mode)

    # This integer describes diagnostic callbacks, NOT eligible actions.
    return {"lifetimes": 2, "native_instructions": len(commits),
            "qualified": sum(qualified.values()), "mode": mode,
            "safe_actions": [], "settled": False}

def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: ci_assert_real_instruction_outcomes.py LOG MODE")
    lines = Path(sys.argv[1]).read_text(errors="replace").splitlines()
    try:
        r = validate(lines, sys.argv[2])
    except ValueError as exc:
        print("TIMEW_CI_OUTCOME_FAIL reason=" + str(exc), file=sys.stderr)
        raise SystemExit(1)
    print("TIMEW_CI_OUTCOME_" + sys.argv[2].upper() +
          "_PASS actual_xdg_lifetimes=2 native_instructions=" +
          str(r["native_instructions"]) +
          " qualified_correlations=" + str(r["qualified"]) +
          " configured_outcome=DIAGNOSTIC_ONLY" +
          " present=UNVERIFIED settled=0" +
          " selected_operator=NONE enrolled_source=NONE c5_spent=NONE" +
          " receipt=NONE native_restore=OFF native_move=OFF retry=OFF" +
          " safe_actions=NONE authority=NONE")

if __name__ == "__main__":
    main()
