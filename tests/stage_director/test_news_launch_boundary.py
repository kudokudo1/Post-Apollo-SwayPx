#!/usr/bin/env python3
"""Guard NEWS launch planning from becoming an uncontained browser launcher."""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
read = lambda p: (root / p).read_text(encoding="utf-8")
policy = read("sway/stage_director_news_launch_plan.c")
header = read("include/sway/stage_director_news_launch_plan.h")
meson = read("sway/meson.build")
core = read("sway/stage_director.c")
clip = read("sway/stage_director_viewport_clip.c")
custody = read("sway/stage_director_news_custody.c")

assert "'stage_director_news_launch_plan.c'" in meson
assert "stage_director_news_source_url(" in policy
assert "SD_NEWS_LAUNCH_UNKNOWN_SOURCE" in policy
assert "SD_NEWS_LAUNCH_AWAITING_CONTAINMENT" in policy
assert "SD_NEWS_LAUNCH_CANDIDATE_ONLY" in policy
assert ".can_spawn = false" in policy
assert "request->compositor_clip_verified" in policy
assert "request->compositor_input_guard_verified" in policy
assert "request->compositor_popup_guard_verified" in policy
assert "request->compositor_teardown_guard_verified" in policy
assert "request->operator_intent_authenticated" in policy
assert "request->native_host_verified" in policy
assert "--class=post-apollo-weather-news" in policy
assert "--user-data-dir=/run/user/" in policy
assert "https://live.house.gov/" in policy
assert "stage_director_news_plan_argv(" in policy

# No arbitrary URL, shell, process, focus or compositor mutation in plan.
for forbidden in (
    "fork(", "vfork(", "execv(", "execve(", "posix_spawn(",
    "system(", "popen(", "kill(", "waitpid(",
    "wlr_scene_subsurface_tree_set_clip(", "sway_seat_set_focus(",
    "stage_director_news_custody_capture(",
):
    assert forbidden not in policy, forbidden
for live in (core, clip, custody):
    assert "stage_director_news_plan(" not in live
    assert "stage_director_news_plan_argv(" not in live
assert "bool can_spawn" in header
print("Stage Director NEWS browser argv plan is disconnected and nonlaunching: PASS")
