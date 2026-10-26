#!/usr/bin/env python3
"""Source guard: NEWS input/popup geometry is NOT active compositor routing."""
from pathlib import Path
root = Path(__file__).resolve().parents[2]
def read(p):
    return (root / p).read_text(encoding="utf-8")

policy = read("sway/stage_director_news_containment.c")
header = read("include/sway/stage_director_news_containment.h")
cursor = read("sway/input/cursor.c")
seatop = read("sway/input/seatop_default.c")
popups = read("sway/desktop/xdg_shell.c")
transaction = read("sway/desktop/transaction.c")
core = read("sway/stage_director.c")
clip = read("sway/stage_director_viewport_clip.c")
meson = read("sway/meson.build")

assert "'stage_director_news_containment.c'" in meson
assert "stage_director_news_pointer_probe(" in policy
assert "stage_director_news_popup_probe(" in policy
assert "stage_director_viewport_plan(" in policy
assert "actor_process_verified" in policy
assert "session_owner_verified" in policy
assert "candidate_target_ref" in header
assert "candidate_actor_ref" in header
assert "isfinite(global_x)" in policy and "isfinite(global_y)" in policy
assert "stage_director_rect_contains(snapshot->request.viewport" in policy
assert ".can_dispatch_input = false" in policy
assert ".can_show_popup = false" in policy
assert ".can_enable_clip = false" in policy

# Sway hit testing and popups are separate from normal toplevel clip.
assert "struct sway_node *node_at_coords(" in cursor
assert "wlr_scene_node_at(" in cursor
assert "node_at_coords(seat" in seatop
assert "wlr_seat_pointer_notify_enter" in seatop
assert "root->layers.popup" in popups
assert "arrange_popups(" in transaction
assert "wlr_scene_subsurface_tree_set_clip(" in clip

# Deliberately no compositor event/scene/popup path is wired to this helper:
# enable only after a trusted scene owner can filter actual native surfaces.
for live in (cursor, seatop, popups, transaction, core, clip):
    assert "stage_director_news_pointer_probe(" not in live
    assert "stage_director_news_popup_probe(" not in live
for forbidden in ("wlr_scene_node_set_enabled(", "wlr_scene_node_reparent(",
                  "wlr_scene_subsurface_tree_set_clip(",
                  "wlr_seat_pointer_notify_enter(", "seat_set_focus(",
                  "fork(", "posix_spawn(", "system("):
    assert forbidden not in policy
print("Stage Director NEWS pointer/popup containment policy stays nonactuating: PASS")
