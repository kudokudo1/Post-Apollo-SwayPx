#!/usr/bin/env python3
"""Static authority boundary for M24. Live libinput/Wayland acceptance separate."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
read = lambda p: (ROOT / p).read_text()

company = read("sway/stage_director_company_pairing.c")
selection = read("sway/stage_director_company_selection.c")
keyboard = read("sway/input/keyboard.c")
cursor = read("sway/input/cursor.c")
sd = read("sway/stage_director.c")
weather = read("sway/stage_director_news_pairing.c")
meson = read("sway/meson.build")
protocol = read("sway/stage_director_company_vector_protocol.c")

# Disabled by default and independent from Weather's F12.
assert 'getenv("POST_APOLLO_STAGE_DIRECTOR_COMPANY_PAIRING")' in company
assert "raw_keysyms[0] != XKB_KEY_F11" in company
assert "raw_keysyms[0] != XKB_KEY_F12" not in company
assert 'getenv("POST_APOLLO_STAGE_DIRECTOR_PAIRING")' in weather
assert "raw_keysyms[0] != XKB_KEY_F12" in weather
assert "PAIR_WINDOW_MSEC 3000U" in company
assert "raw_modifiers != COMPANY_PAIR_MODIFIERS" in company
assert "WLR_MODIFIER_CTRL | WLR_MODIFIER_LOGO | WLR_MODIFIER_SHIFT" in company
assert "libinput_device_is_libinput(device)" in company
assert "shortcuts_inhibited" in company and "session_locked" in company
assert "seat == company_pairing.armed_seat" in company
assert "button == BTN_LEFT" in company
assert "state == WL_POINTER_BUTTON_STATE_RELEASED" in company
assert "company_pairing.swallow_release = true;" in company

# Actual compositor scene hit, no client-provided PID, namespace shortcut
# or focused-window-only fallback. Root must be the live layer role surface.
for fragment in (
    "node_at_coords(seat, lx, ly, &hit, &sx, &sy)",
    "wlr_surface_get_root_surface(hit)",
    "wlr_layer_surface_v1_try_from_wlr_surface(root)",
    "role->surface != root",
    "strcmp(role->namespace, SD_COMPANY_NATIVE_NAMESPACE)",
    "role->current.layer != ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY",
    "role->data",
    "host->layer_surface != role",
    "stage_director_company_native_observe_host(host)",
    "observed.native_layer_ref != (uintptr_t)host",
    "observed.native_surface_ref != (uintptr_t)root->resource",
    "stage_director_company_vector_protocol_publish(",
    "probe_paired_native_host",
    "server.session_lock.lock",
    "observed.start_ticks != company_selection.start_ticks",
    "observed.native_output_ref != company_selection.output_ref",
    "company_selection.revision++",
    "stage_director_company_vector_protocol_revoke()",
):
    assert fragment in selection, f"missing native selection gate: {fragment}"

# Only real physical input can arm/click. Sway IPC command and public
# Wayland handlers do not have a Company pairing issuer.
assert "stage_director_company_pairing_key_press(" in keyboard
assert "stage_director_news_pairing_key_press(" in keyboard
assert "stage_director_company_pairing_pointer_button(" in cursor
assert "stage_director_news_pairing_pointer_button(" in cursor
assert "const bool weather_paired" in cursor
assert "const bool company_paired" in cursor
assert "weather_paired || company_paired" in cursor

# Native layer changes proactively revoke, not only kernel process exit.
assert "stage_director_company_pairing_host_changed(surface)" in sd
assert sd.count("stage_director_company_pairing_host_lost(surface)") == 2
assert "'stage_director_company_selection.c'," in meson
assert "'stage_director_company_pairing.c'," in meson

# The only production call to publish is inside the physically gated
# Company scene-hit selection. Never activate on startup or namespace match.
assert "stage_director_company_vector_protocol_publish(" not in sd
assert "stage_director_company_vector_protocol_publish(" not in keyboard
assert "stage_director_company_vector_protocol_publish(" not in cursor
assert "stage_director_company_vector_protocol_publish(" not in company
assert 'strcmp(role->namespace, SD_COMPANY_NATIVE_NAMESPACE)' in selection

for text in (company, selection):
    for forbidden in (
        "wlr_scene_node_set_", "wlr_scene_buffer_set_",
        "wlr_seat_", "wl_display_connect(", "posix_spawn(",
        "execve(", "fork(", "system(", "swaymsg",
        "stage_director_company_candidate_session_record(",
        "stage_director_company_map_probe(",
        "owner_verified = true", "can_clip = true",
        "can_launch = true", "can_accept_input = true",
    ):
        assert forbidden not in text, forbidden

assert "stage_director_company_vector_protocol_is_global" in protocol
print("STAGE_DIRECTOR_COMPANY_M24: physical-only F11, private host and Weather isolation PASS")
