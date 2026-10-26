#!/usr/bin/env python3
"""Weather host native observation is intentionally non-actuating."""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
def read(path):
    return (root / path).read_text(encoding="utf-8")

header = read("include/sway/stage_director_weather_host.h")
policy = read("sway/stage_director_weather_host.c")
adapter = read("sway/stage_director_weather_host_adapter.c")
meson = read("sway/meson.build")
registry = read("sway/stage_director.c")
layer_hooks = read("sway/desktop/layer_shell.c")
viewer = read("sway/stage_director_viewer_session.c")

assert 'STAGE_DIRECTOR_WEATHER_NAMESPACE "post-apollo-weather-station"' in header
assert "strcmp(observation->layer_namespace" in policy
assert "SD_WEATHER_HOST_CANDIDATE_ONLY" in policy
assert "stage_director_rect_contains(observation->output" in policy
assert ".owner_verified = false" in policy
assert ".can_bind_viewer = false" in policy
assert ".can_clip = false" in policy
assert "layer->namespace" in adapter
assert "output_get_box(" in adapter
assert "wlr_scene_node_coords(" in adapter
assert "layer->current.layer == ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY" in adapter
assert "surface->mapped && layer->surface->mapped" in adapter
assert "surface->output->enabled" in adapter
assert "'stage_director_weather_host.c'" in meson
assert "'stage_director_weather_host_adapter.c'" in meson

# No implicit authority is acquired by observing namespace/geometry. Only
# a future authorized compositor/launcher can bind a host and actor session.
for code in (registry, layer_hooks, viewer):
    assert "stage_director_weather_host_observe_layer(" not in code
    assert "stage_director_weather_host_probe(" not in code
for code in (policy, adapter):
    for forbidden in (
        "wlr_scene_subsurface_tree_set_clip(",
        "wlr_scene_node_set_position(",
        "stage_director_viewport_clip_view(",
        "stage_director_viewer_session_attach(",
        "stage_director_viewer_session_reserve(",
        "sway_seat_set_focus(",
        "fork(", "execv(", "system(",
    ):
        assert forbidden not in code, forbidden

print("Stage Director Weather host native no-authority boundary: PASS")
