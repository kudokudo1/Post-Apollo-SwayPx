#!/usr/bin/env python3
"""SD1 NEWS shadow connection must remain read-only and fail-closed."""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
read = lambda p: (root / p).read_text(encoding="utf-8")
core = read("sway/stage_director.c")
policy = read("sway/stage_director_news_shadow.c")
header = read("include/sway/stage_director_news_shadow.h")
meson = read("sway/meson.build")
host = read("sway/stage_director_weather_host_adapter.c")

assert "'stage_director_news_shadow.c'" in meson
assert "stage_director_news_shadow_evaluate(hosts, browser_actors)" in core
assert "stage_director_weather_host_observe_layer(surface)" in core
assert "observation->news_host_candidate = news_host.candidate_observed;" in core
assert "actor->news_actor_candidate = news_actor;" in core
assert 'strcmp(identity.scene, "weather-news") == 0' in core
assert 'strcmp(identity.role, "browser") == 0' in core
assert 'observed=%s hosts=%d actors=%d' in core
assert 'authority=NONE' in core
assert "observe_news_shadow();" in core
assert 'const bool had_news_actor = actor->news_actor_candidate;' in core
assert 'const bool had_news_host = observation->news_host_candidate;' in core
assert "tracked->news_actor_candidate = false;" in core
assert "stage_director_news_shadow_state_name(candidate.state)" in core
assert "stage_director_weather_host_probe(" in host

# Candidate counts never imply permission, even in the 1:1 case.
for required in (
    ".owner_verified = false", ".can_bind_viewer = false",
    ".can_launch = false", ".can_clip = false",
    ".can_accept_input = false",
):
    assert required in policy
assert "r.unique_candidates = true" in policy
assert "SD_NEWS_SHADOW_AMBIGUOUS_BOTH" in policy
assert "SD_NEWS_SHADOW_UNIQUE_CANDIDATES" in header
for source in (core, policy):
    for forbidden in (
        "stage_director_viewport_clip_view(",
        "stage_director_viewer_session_reserve(",
        "stage_director_viewer_session_attach(",
        "wlr_scene_subsurface_tree_set_clip(",
        "wlr_scene_node_reparent(",
        "sway_seat_set_focus(",
        "fork(", "execv(", "system(",
    ):
        assert forbidden not in source, forbidden

print("Stage Director NEWS active shadow observation without authority: PASS")
