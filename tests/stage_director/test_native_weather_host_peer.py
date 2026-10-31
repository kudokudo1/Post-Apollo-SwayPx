#!/usr/bin/env python3
"""CI source guard for the production read-only Weather layer Wayland peer observer.

The ordinary SwayPX build must still compile the real wlroots adapter.
This static check is NOT a real Weather/QML layer runtime authentication test.
"""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
def read(path):
    return (root / path).read_text(encoding="utf-8")

adapter = read("sway/stage_director_weather_host_adapter.c")
header = read("include/sway/stage_director_weather_host.h")
observer = read("sway/stage_director.c")
meson = read("sway/meson.build")
layer_shell = read("sway/desktop/layer_shell.c")
weather = read("sway/stage_director_weather_host.c")
lease = read("sway/stage_director_news_session_lease.c")

assert "stage_director_weather_host_observe_native_peer(" in header
assert "stage_director_weather_host_observe_native_peer(" in adapter
assert "stage_director_weather_host_observe_native_peer(surface)" in observer
assert "stage_director_weather_host_observe_layer(surface)" in observer
assert "stage_director_weather_host_probe(&current)" in adapter
assert "'stage_director_weather_host_adapter.c'," in meson

# Both the actual layer-shell role resource and native wl_surface must
# belong to the same wl_client; namespaces and IDs alone are spoofable.
assert "wl_resource_get_client(layer->resource)" in adapter
assert "wl_resource_get_client(layer->surface->resource)" in adapter
assert "layer_client != surface_client" in adapter
assert "wl_client_get_credentials(layer_client, &peer_pid, &peer_uid, NULL)" in adapter
assert adapter.count("stage_director_read_linux_process((int64_t)peer_pid,") == 2
assert "first.start_time_ticks != second.start_time_ticks" in adapter
assert "first.parent_pid != second.parent_pid" in adapter
assert "first.uid != (uint32_t)peer_uid" in adapter
assert "second.uid != (uint32_t)peer_uid" in adapter
assert "out.native_client_ref = (uintptr_t)layer_client;" in adapter
assert "out.native_layer_ref = (uintptr_t)surface;" in adapter
assert "out.native_output_ref = (uintptr_t)surface->output;" in adapter
assert "out.native_peer_candidate = true;" in adapter
assert "STAGE_DIRECTOR_WEATHER_NAMESPACE" in weather
assert "WlrLayershell.namespace" not in adapter  # QML property isn't evidence

# Entire module is observation, with no IPC, launch, input/seat, painting or
# implicit authorization. No new callback registered for NEWS.
for string in ("owner_verified = false", "can_launch = false",
               "can_bind_viewer = false", "can_clip = false",
               "can_accept_input = false"):
    assert string in adapter, string
for banned in ("posix_spawn(", "fork(", "execve(", "system(",
               "wlr_scene_buffer_set_dest_size(",
               "stage_director_news_session_lease_reserve_candidate(",
               "stage_director_news_session_lease_record_actor_candidate(",
               "seat_set_focus", "wlr_seat_", "wl_global_create("):
    assert banned not in adapter, banned

# Existing layer lifecycle events already reach the SD0 observer, including
# host loss, so adding a new monitor/poller is unnecessary.
for name in ("stage_director_layer_mapped(surface);",
             "stage_director_layer_unmapped(surface);",
             "stage_director_layer_committed(surface);",
             "stage_director_layer_destroyed(layer);"):
    assert name in layer_shell, name
assert "NEWS_HOST_PEER" in observer
assert "operator_authenticated=no can_launch=no" in observer
assert "if (!force && !changed)" in observer
assert observer.index("if (!force && !changed)") < observer.index(
    "stage_director_weather_host_observe_native_peer(surface)")
assert "stage_director_news_session_lease_" not in observer
assert "stage_director_news_session_lease_" not in layer_shell
assert "owner_verified = false" in lease
print("SD_WEATHER_HOST_PEER: native wl_client, stable /proc identity, "
      "bounded map/changed-layer observation and zero WATCH authority: PASS")
