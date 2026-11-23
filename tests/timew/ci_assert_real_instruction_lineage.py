#!/usr/bin/env python3
"""Fail-closed evidence checker for actual XDG transaction instruction callbacks.

This verifies diagnostic callback chronology only. In-compositor pointers,
serials, lifetime numbers and the test app_id are NOT credentials or receipts.
"""
import re
import sys
from pathlib import Path

if len(sys.argv) != 2:
    raise SystemExit("BLOCKED_CI_INSTRUCTION_LOG_ARGS")

lines = Path(sys.argv[1]).read_text(errors="replace").splitlines()
opens = {}
maps = {}
seals = {}
losses = {}
unmaps = {}
bindings = []
patterns = {
    "open": re.compile(r"TIMEW_CI_LINEAGE_OPEN lifetime=(\d+) transaction=(\d+)"),
    "map": re.compile(r"TIMEW_CI_REAL_XDG_MAPPED gen=(\d+)"),
    "bind": re.compile(
        r"TIMEW_CI_REAL_INSTRUCTION_BIND lifetime=(\d+) commit_sequence=(\d+) "
        r"native_transaction_instruction=1 mapped_xdg=1 "
        r"configure_serial_nonzero=([01]) transaction_pointer=diagnostic_only "
        r"physical_selection=NONE source=UNENROLLED c5_spent=NONE "
        r"protected_receipt=0 native_move=OFF authority=NONE"
    ),
    "seal": re.compile(
        r"TIMEW_CI_REAL_INSTRUCTION_SEAL lifetime=(\d+) "
        r"real_commit_count=(\d+) real_unmap=1 generation_closed=1 "
        r"protected_receipt=0 authority=NONE"
    ),
    "lost": re.compile(r"TIMEW_CI_LINEAGE_REVOKED lifetime=(\d+) transaction=(\d+)"),
    "unmap": re.compile(r"TIMEW_CI_REAL_XDG_UNMAPPED gen=(\d+)"),
}

def unique(d, k, value, label):
    if k in d:
        raise SystemExit(f"TIMEW_CI_REAL_INSTRUCTION_FAIL duplicate_{label}_{k}")
    d[k] = value

for idx, line in enumerate(lines):
    if "TIMEW_CI_REAL_INSTRUCTION_FAIL" in line or "TIMEW_CI_LINEAGE_FAIL" in line:
        raise SystemExit("TIMEW_CI_REAL_INSTRUCTION_FAIL upstream_native_diagnostic_failed")
    if m := patterns["open"].search(line):
        unique(opens, int(m[1]), (idx, int(m[2])), "open")
    if m := patterns["map"].search(line):
        unique(maps, int(m[1]), idx, "mapped")
    if m := patterns["bind"].search(line):
        bindings.append((idx, int(m[1]), int(m[2]), int(m[3])))
    if m := patterns["seal"].search(line):
        unique(seals, int(m[1]), (idx, int(m[2])), "seal")
    if m := patterns["lost"].search(line):
        unique(losses, int(m[1]), (idx, int(m[2])), "revoke")
    if m := patterns["unmap"].search(line):
        unique(unmaps, int(m[1]), idx, "unmap")

expected = {1, 2}
if any(set(d) != expected for d in (opens, maps, seals, losses, unmaps)):
    raise SystemExit("TIMEW_CI_REAL_INSTRUCTION_FAIL missing_or_extra_native_lifetime")
if len(bindings) < 2 or len(set(x[2] for x in bindings)) != len(bindings):
    raise SystemExit("TIMEW_CI_REAL_INSTRUCTION_FAIL reused_or_missing_commit_number")
if [x[2] for x in bindings] != sorted(x[2] for x in bindings):
    raise SystemExit("TIMEW_CI_REAL_INSTRUCTION_FAIL commit_sequence_not_monotonic")

for generation in (1, 2):
    observed = [x for x in bindings if x[1] == generation]
    if not observed:
        raise SystemExit(f"TIMEW_CI_REAL_INSTRUCTION_FAIL no_commit_for_lifetime_{generation}")
    if not any(x[3] == 1 for x in observed):
        raise SystemExit(f"TIMEW_CI_REAL_INSTRUCTION_FAIL no_actual_xdg_configure_for_{generation}")
    if not all(
        opens[generation][0] < maps[generation] < x[0] <
        seals[generation][0] < losses[generation][0] < unmaps[generation]
        for x in observed
    ):
        raise SystemExit(f"TIMEW_CI_REAL_INSTRUCTION_FAIL event_order_{generation}")
    if seals[generation][1] != len(observed) or losses[generation][1] != generation:
        raise SystemExit(f"TIMEW_CI_REAL_INSTRUCTION_FAIL incorrect_generation_count_{generation}")
    if generation == 1 and not (unmaps[1] < opens[2][0]):
        raise SystemExit("TIMEW_CI_REAL_INSTRUCTION_FAIL overlapping_candidate_lifetimes")

print(
    "TIMEW_CI_REAL_INSTRUCTION_LINEAGE PASS: "
    f"actual_native_commit_instructions={len(bindings)} "
    "two_distinct_real_XDG_lifetimes=1 same_view_root_membership=1 "
    "commit_sequence_minted_at_actual_transaction_commit=1 "
    "configured_instruction_seen_per_lifetime=1 "
    "unmap_generation_sealed_before_destroy=1 "
    "protected_selection=NONE enrolled_source=NONE "
    "per_step_ack=UNVERIFIED settled=0 receipt=NONE "
    "native_resize=OFF native_move=OFF retry=OFF authority=NONE"
)
