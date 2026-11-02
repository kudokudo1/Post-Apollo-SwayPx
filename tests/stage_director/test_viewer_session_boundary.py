#!/usr/bin/env python3
"""Ensure the SD1 viewer session is a candidate association, never authority."""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
def read(path):
    return (root / path).read_text(encoding="utf-8")

core = read("sway/stage_director.c")
session = read("sway/stage_director_viewer_session.c")
header = read("include/sway/stage_director_viewer_session.h")
clip = read("sway/stage_director_viewport_clip.c")
meson = read("sway/meson.build")
workflow = read(".github/workflows/stage-director-viewport-contracts.yml")

assert "'stage_director_viewer_session.c'" in meson
assert "test_viewer_session.c" in workflow
assert "stage_director_viewer_session_reserve(" in session
assert "stage_director_viewer_session_attach(" in session
assert "stage_director_viewer_session_host_lost(" in session
assert "stage_director_viewer_session_actor_lost(" in session
assert "stage_director_viewer_session_close(" in session
assert "stage_director_viewer_session_probe(" in session
assert "observed_process_start != session->expected_process_start" in session
assert "observed_pid != session->expected_pid" in session
assert "epoch <= session->last_epoch" in session
assert "session->state != SD_VIEWER_EMPTY" in session
assert "same_rect(session->viewport, current->viewport)" in session
assert "SD_VIEWER_ACTOR_CANDIDATE" in header

# No live hook or IPC entry point may adopt caller-supplied observations as
# trusted compositor / launcher facts. This requires a separate approved task.
for code in (session, header):
    for forbidden in (
        "wlr_scene_subsurface_tree_set_clip",
        "stage_director_viewport_clip_view(",
        "sway_seat_set_focus",
        "wlr_scene_node_reparent",
        "wlr_scene_node_set_position",
        "swaymsg ",
        "fork(", "execv(", "system(", "Popen(",
        "QtWebEngine",
    ):
        assert forbidden not in code, forbidden
for other in (core, clip):
    assert "stage_director_viewer_session_reserve(" not in other
    assert "stage_director_viewer_session_attach(" not in other
    assert "stage_director_viewer_session_probe(" not in other
assert ".can_launch = false" in session
assert ".can_clip = false" in session
assert ".can_accept_input = false" in session
assert "authority=NONE" in core
print("Stage Director viewer session no-activation and lifecycle source contract: PASS")
