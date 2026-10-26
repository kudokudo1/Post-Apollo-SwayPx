#!/usr/bin/env python3
"""The NEWS Linux process inspector must never become launch authority."""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
read = lambda p: (root / p).read_text(encoding="utf-8")
policy = read("sway/stage_director_process_evidence.c")
native = read("sway/stage_director_process_linux.c")
head = read("include/sway/stage_director_process_evidence.h")
core = read("sway/stage_director.c")
session = read("sway/stage_director_viewer_session.c")
adapter = read("sway/stage_director_viewport_clip.c")
meson = read("sway/meson.build")

assert "'stage_director_process_evidence.c'" in meson
assert "'stage_director_process_linux.c'" in meson
assert "stage_director_process_check(" in policy
assert "stage_director_read_linux_process(" in native
assert "stage_director_parse_linux_proc_stat(" in native
assert "strrchr(copy, ')')" in native
assert "openat(procfd, name" in native
assert "O_NOFOLLOW" in native
assert "start_time_ticks" in policy
assert "same_identity(path[i], again)" in policy
assert "SD_PROCESS_MAX_ANCESTRY" in head
assert "SD_PROCESS_EVIDENCE_LINEAGE_CANDIDATE" in head
for flag in (
    ".owner_verified = false", ".can_bind_viewer = false",
    ".can_launch = false", ".can_clip = false",
    ".can_accept_input = false"
):
    assert flag in policy, flag
for forbidden in (
    "stage_director_viewport_clip_view(",
    "stage_director_viewer_session_attach(",
    "stage_director_viewer_session_reserve(",
    "wlr_scene_node_set_position(",
    "wlr_scene_subsurface_tree_set_clip(",
    "fork(", "execv(", "system(", "Popen(",
    "sway_seat_set_focus("
):
    assert forbidden not in policy
    assert forbidden not in native
for live_source in (core, session, adapter):
    assert "stage_director_process_check(" not in live_source
    assert "stage_director_read_linux_process(" not in live_source
print("Stage Director NEWS process identity evidence remains nonactuating: PASS")
