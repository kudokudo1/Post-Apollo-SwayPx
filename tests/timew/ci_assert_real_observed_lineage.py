#!/usr/bin/env python3
"""Real SwayPX XDG private lineage assertions, NEVER a protected receipt.

The ephemeral native callback must mint increasing internal observation IDs
for two distinct real mapped XDG lifetimes and revoke each synchronously
when the actual Wayland unmap emits. Pointer/native resource reuse is NOT
treated as identity or physical operator provenance.
"""
from pathlib import Path
import re
import sys

if len(sys.argv) != 2:
    raise SystemExit("BLOCKED_LINEAGE_LOG_ARGS")
lines = Path(sys.argv[1]).read_text(errors="replace").splitlines()
opens, losses = [], []
real_map, real_unmap = [], []
open_pat = re.compile(
    r"TIMEW_CI_LINEAGE_OPEN lifetime=(\d+) transaction=(\d+) "
    r"native_xdg=1 boot_tag=local_only geometry=exact "
    r"operator_selection=NONE source=UNENROLLED "
    r"protected_receipt=0 native_move=OFF authority=NONE"
)
loss_pat = re.compile(
    r"TIMEW_CI_LINEAGE_REVOKED lifetime=(\d+) transaction=(\d+) "
    r"real_unmap=1 old_generation_invalid=1 privileged_action=0"
)
for index, line in enumerate(lines):
    if m := open_pat.search(line):
        opens.append((index, int(m[1]), int(m[2])))
    if m := loss_pat.search(line):
        losses.append((index, int(m[1]), int(m[2])))
    if "TIMEW_CI_REAL_XDG_MAPPED gen=" in line:
        real_map.append(index)
    if "TIMEW_CI_REAL_XDG_UNMAPPED gen=" in line:
        real_unmap.append(index)

if len(opens) != 2 or len(losses) != 2 or len(real_map) != 2 or len(real_unmap) != 2:
    raise SystemExit(f"TIMEW_CI_LINEAGE_FAIL missing native callbacks "
                     f"open={opens} revoked={losses} "
                     f"mapped={len(real_map)} unmapped={len(real_unmap)}")
if [(x[1], x[2]) for x in opens] != [(1,1),(2,2)] or [
    (x[1],x[2]) for x in losses] != [(1,1),(2,2)]:
    raise SystemExit(f"TIMEW_CI_LINEAGE_FAIL reused generation {opens} {losses}")
for i in range(2):
    # The private correlation happens in the real mapped callback, the
    # exact-generation revocation happens in the real unmap callback.
    if not (opens[i][0] < real_map[i] < losses[i][0] < real_unmap[i]):
        raise SystemExit(f"TIMEW_CI_LINEAGE_FAIL callback order generation={i+1}")
    if i == 0 and not (real_unmap[i] < opens[i+1][0]):
        raise SystemExit("TIMEW_CI_LINEAGE_FAIL old lifetime was not closed")
if "TIMEW_CI_REAL_XDG_FAIL" in "\n".join(lines):
    raise SystemExit("TIMEW_CI_LINEAGE_FAIL previous real XDG event failed")
print("TIMEW_CI_REAL_NATIVE_LINEAGE PASS: two distinct real mapped XDG "
      "lifetimes and process-minted txn counters; synchronous loss "
      "before container destroy; no selected-surface authentication, "
      "no source custody, no settled receipt or native authority")
