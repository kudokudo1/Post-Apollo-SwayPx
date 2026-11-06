#!/usr/bin/env python3
"""Company M20 source boundary: read-only native observation, no admission."""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
adapter = (root / "sway/stage_director_company_native_adapter.c").read_text()
peers = (root / "sway/stage_director_company_native_peers.c").read_text()
header = (root / "include/sway/stage_director_company_native.h").read_text()
stage = (root / "sway/stage_director.c").read_text()
meson = (root / "sway/meson.build").read_text()

for required in (
    "wl_client_get_credentials(", "wl_resource_get_client(",
    "stage_director_read_linux_process(", "first.start_time_ticks != second.start_time_ticks",
    "wlr_surface_get_root_surface(", "view->type != SWAY_VIEW_XDG_SHELL",
    "view->wlr_xdg_toplevel->base->surface != view->surface",
    "wlr_scene_node_coords(", "output_get_box(",
    "stage_director_rect_contains(out.output_global",
    "SD_COMPANY_NATIVE_NAMESPACE",
):
    assert required in adapter, f"Missing native observation check: {required}"

for required in (
    'SD_COMPANY_NATIVE_NAMESPACE "post-apollo-company-network"',
    "owner_verified", "can_clip", "can_accept_input",
):
    assert required in header, required

for required in (
    "role_client != surface_client", "actor_client == role_client",
    "SD_COMPANY_PEER_DISTINCT_CANDIDATE_ONLY",
):
    assert required in peers, required

for required in (
    '"sway/stage_director_company_native.h"',
    "stage_director_company_native_observe_host(surface)",
    "stage_director_company_native_observe_actor(view)",
    'strstr(app_id, "CompanyVectorProbe")',
    '"[stage-director][COMPANY_M20][LAYER]',
    '"[stage-director][COMPANY_M20][XDG]',
):
    assert required in stage, required

for name in (
    "'stage_director_company_map.c'",
    "'stage_director_company_native_peers.c'",
    "'stage_director_company_native_adapter.c'",
):
    assert name in meson, name

# Nothing can move, resize, clip, reveal, focus, launch or authorize a window.
for source in (adapter, peers):
    for bad in (
        "wlr_scene_node_set_", "wlr_scene_buffer_set_",
        "wlr_seat_", "wl_global_create(", "wl_display_connect(",
        "posix_spawn(", "execve(", "fork(", "system(",
        "swaymsg", "stage_director_company_map_probe(",
        "stage_director_news_pairing_select_at(",
        "stage_director_news_watch_record_candidate(",
    ):
        assert bad not in source, f"Unauthorized symbol {bad}"
    assert "owner_verified = true" not in source
    assert "can_clip = true" not in source
    assert "can_accept_input = true" not in source

# Existing Weather pairing must remain a separate native selector.
assert 'strcmp(role->namespace, STAGE_DIRECTOR_WEATHER_NAMESPACE)' in stage
assert "stage_director_news_pairing_select_at(" in stage

print("STAGE_DIRECTOR_COMPANY_M20: native observers only, no authority PASS")
