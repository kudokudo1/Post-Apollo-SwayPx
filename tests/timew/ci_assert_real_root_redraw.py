"""Controlled disposable foot redraw: real root commit and sample revocation.

A pre-trigger checkpoint selects a still-live sample already correlated to
output PRESENT. A later committed root state and revocation must occur in
that same mapped lifetime. This is not a wl_buffer attachment or pixel proof.
"""
import re
import sys
from pathlib import Path

SAMPLE = re.compile(r"TIMEW_CI_XDG_GEN_SAMPLE lifetime=(\d+) sequence=(\d+) surface_state_seq=(\d+) root_client_object_match=1 watch_epoch=([1-9]\d*) ")
PRESENT = re.compile(r"TIMEW_CI_XDG_GEN_PRESENT lifetime=(\d+) sequence=(\d+) output_commit_seq=(\d+) current_surface_seq=(\d+) watch_epoch=([1-9]\d*) ")
REVOKE = re.compile(r"TIMEW_CI_XDG_WATCH_REVOKE lifetime=(\d+) epoch=(\d+) reason=(surface_commit|buffer_destroy|resample|unmap|scene_detach) authority=NONE$")
ROOT = re.compile(r"TIMEW_CI_XDG_REDRAW_COMMIT lifetime=(\d+) surface_state_seq=(\d+) prior_watch_epoch=(\d+) authority=NONE$")
MAP = re.compile(r"TIMEW_CI_REAL_XDG_MAPPED gen=(\d+)(?:\s|$)")
UNMAP = re.compile(r"TIMEW_CI_REAL_XDG_UNMAPPED gen=(\d+)(?:\s|$)")

def checkpoint(lines):
    """Select latest sample in most recent actual still-mapped lifetime."""
    life = None
    sample = None
    presented = False
    live = False
    for line in lines:
        m = MAP.search(line)
        if m:
            life = int(m[1])
            sample = None
            presented = live = False
        m = UNMAP.search(line)
        if m and int(m[1]) == life:
            life = None
            sample = None
            presented = live = False
        m = SAMPLE.search(line)
        if m and int(m[1]) == life:
            sample = tuple(map(int,m.groups()))
            presented = False
            live = True
        m = PRESENT.search(line)
        if m and sample and (int(m[1]),int(m[2]),int(m[4]),int(m[5])) == sample:
            presented = True
        m = REVOKE.search(line)
        if m and sample and int(m[1]) == life and int(m[2]) == sample[3]:
            live = False
    if not (life and sample and presented and live):
        raise ValueError("no_live_sample_at_present")
    return sample

def verify_after(lines, floor):
    if not 0 < floor < len(lines):
        raise ValueError("no_new_root_events")
    life, scene, seq, epoch = checkpoint(lines[:floor])
    reason = None
    for line in lines[floor:]:
        if MAP.search(line) or UNMAP.search(line):
            raise ValueError("actor_lifetime_changed")
        m = REVOKE.search(line)
        if m and int(m[1]) == life and int(m[2]) == epoch:
            if reason is not None or m[3] not in ("surface_commit","buffer_destroy"):
                raise ValueError("invalid_sample_revocation")
            reason = m[3]
        m = ROOT.search(line)
        if m and int(m[1]) == life and int(m[2]) != seq and reason:
            prior = int(m[3])
            if (reason == "surface_commit" and prior != epoch) or (reason == "buffer_destroy" and prior != 0):
                raise ValueError("wrong_revocation_callback_lineage")
            return life, epoch, reason
    raise ValueError("missing_changed_root_commit_with_revocation")

def main():
    if len(sys.argv) not in (3,4):
        return 2
    lines = Path(sys.argv[2]).read_text(errors="replace").splitlines()
    try:
        if sys.argv[1] == "ready" and len(sys.argv) == 3:
            checkpoint(lines)
            print(len(lines))
        elif sys.argv[1] == "after" and len(sys.argv) == 4:
            life, epoch, reason = verify_after(lines,int(sys.argv[3]))
            print(f"TIMEW_CI_CONTROLLED_REDRAW_PASS lifetime={life} epoch={epoch} revocation={reason} changed_root_commit=1 attachment=UNVERIFIED final_frame=UNVERIFIED authority=NONE")
        else:
            return 2
    except ValueError:
        return 1
    return 0

if __name__ == "__main__":
    sys.exit(main())
