#!/usr/bin/env python3
"""Static guards for the unmerged Stage Director SD1 observation boundary."""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
read = lambda name: (root / name).read_text(encoding="utf-8")
profile = read("sway/stage_director_profile_weather_news.c")
profiles = read("sway/stage_director_profiles.c")
head = read("include/sway/stage_director_profile.h")
core = read("sway/stage_director.c")
meson = read("sway/meson.build")
geom = read("sway/stage_director_viewport.c")
types = read("include/sway/stage_director_viewport.h")

assert '"post-apollo-weather-news"' in profile
assert '"weather-news"' in profile and '"browser"' in profile
assert ".anchor = false" in profile
assert ".can_lead = false" in profile
assert "view_get_app_id" in profile and "view_get_class" in profile
assert "view_get_title" not in profile, "Titles cannot authorize NEWS actor"
assert "stage_director_profile_weather_news_classify" in profiles
assert "stage_director_profile_weather_news_classify" in head
assert "'stage_director_profile_weather_news.c'" in meson
assert "'stage_director_viewport.c'" in meson

# The compositor must remain shadow only until it has a real verified
# layer anchor, output mapping, clip implementation and input routing.
assert "authority=NONE" in core
assert "stage_director_viewport_plan(" in geom
assert "SD_VIEWPORT_GEOMETRY_ONLY" in geom
for forbidden in (
    "wlr_scene_node_set_position",
    "wlr_scene_node_reparent",
    "wlr_scene_node_set_enabled",
    "wlr_scene_subsurface_tree_set_clip",
    "wlr_scene_buffer_set_source_box",
    "sway_seat_set_focus",
    "view_close(",
    "fork(", "execv(", "system("
):
    assert forbidden not in geom and forbidden not in profile
assert ".can_launch = false" in geom
assert ".can_render = false" in geom
assert ".can_accept_input = false" in geom
assert "bool stage_director_rect_valid" in types

print("Stage Director SD1 identity + no-authority boundaries: PASS")
