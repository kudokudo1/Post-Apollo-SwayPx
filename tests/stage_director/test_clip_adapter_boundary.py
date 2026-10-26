#!/usr/bin/env python3
"""Source contracts for dormant SD1 V1 real wlroots crop adapter."""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
read = lambda name: (root / name).read_text(encoding="utf-8")
adapter = read("sway/stage_director_viewport_clip.c")
header = read("include/sway/stage_director_viewport_clip.h")
policy = read("sway/stage_director_viewport.c")
registry = read("sway/stage_director.c")
meson = read("sway/meson.build")
view = read("sway/tree/view.c")
transaction = read("sway/desktop/transaction.c")
profiles = read("sway/stage_director_profiles.c")

assert "wlr_scene_subsurface_tree_set_clip(" in adapter
assert "wlr_scene_subsurface_tree_set_clip(&view->content_tree->node, &box)" in adapter
assert "stage_director_viewport_prepare_clip" in adapter
assert "wlr_scene_node_coords(&view->content_tree->node" in adapter
assert "view->geometry.x, view->geometry.y" in adapter
assert "request->actor.width != view->geometry.width" in adapter
assert "request->actor.y != actual_y" in adapter
assert "view->container->node.destroying" in adapter
assert '"post-apollo-weather-news"' in adapter
assert "view_get_title" not in adapter
assert "stage_director_viewport_restore_view" in header
assert "view_center_and_clip_surface(view)" in adapter
assert "'stage_director_viewport_clip.c'" in meson
for gate in [
    "operator_enabled", "host_session_verified", "actor_process_verified",
    "input_hit_test_guard_installed", "popup_guard_installed",
    "teardown_guard_installed",
]:
    assert "!" + "authority->" + gate in policy, gate

# No caller yet. SD0 remains observation-only. V1 must not accidentally be
# activated just because a matching browser app_id appeared.
for file in (registry, view, transaction, profiles):
    assert "stage_director_viewport_clip_view(" not in file
    assert "stage_director_viewport_restore_view(" not in file
assert "authority=NONE" in registry
assert "wlr_scene_subsurface_tree_set_clip(" in view
assert "wlr_scene_subsurface_tree_set_clip(" in transaction
for text in (adapter, policy):
    for forbidden in ("fork(", "exec(", "system(", "startDetached(",
                      "sway_seat_set_focus(", "sway_cursor_warp("):
        assert forbidden not in text, forbidden

print("Stage Director V1 dormant clip backend and authority guards: PASS")
