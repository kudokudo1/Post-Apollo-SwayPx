#!/usr/bin/env python3
"""Guard first production native Weather input pairing slice.

This is static integration coverage, not an actual physical input, live
Weather UI or uinput-compromise test. Full SwayPX build runs in a separate CI.
"""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
def read(path):
    return (root / path).read_text(encoding="utf-8")

meson = read("sway/meson.build")
stage = read("sway/stage_director.c")
pair = read("sway/stage_director_news_pairing.c")
key = read("sway/input/keyboard.c")
mouse = read("sway/input/cursor.c")
session = read("sway/stage_director_news_host_session.c")
header = read("include/sway/stage_director_news_pairing.h")

assert "'stage_director_news_pairing.c'," in meson
assert "'stage_director_news_host_session.c'," in meson
assert "'stage_director_news_request_peer.c'," in meson

# No public IPC command, private socket, QML, arbitrary namespace search or
# automatic first/sole-host pairing. No browser, compositor or seat authority.
assert 'getenv("POST_APOLLO_STAGE_DIRECTOR_PAIRING")' in pair
assert 'strcmp(value, "1") == 0' in pair
assert 'WLR_HAS_LIBINPUT_BACKEND' in pair
assert 'wlr_input_device_is_libinput(device)' in pair
assert 'WLR_INPUT_DEVICE_KEYBOARD' in pair
assert 'WLR_INPUT_DEVICE_POINTER' in pair
assert 'raw_modifiers != PAIR_MODIFIERS' in pair
assert 'raw_keysyms[0] != XKB_KEY_F12' in pair
assert 'keysyms_len != 1' in pair
assert 'session_locked' in pair
assert 'shortcuts_inhibited' in pair
assert 'PAIR_WINDOW_MSEC 3000U' in pair
assert 'same_seat' in pair
assert 'button == BTN_LEFT' in pair
assert 'pairing.swallow_release = true' in pair
assert 'expire_swallowed_release(time_msec)' in pair
assert 'stage_director_news_pairing_clear_host();' in pair
assert 'stage_director_news_pairing_select_at(seat, lx, ly)' in pair
assert 'stage_director_news_pairing_key_press(' in key
assert 'stage_director_news_pairing_pointer_button(' in mouse
assert 'server.session_lock.lock != NULL' in mouse
assert 'if (!handled && event->state == WL_KEYBOARD_KEY_STATE_PRESSED)' in key
assert 'wlr_input_device_get_virtual_keyboard' in key

# Scene hit selection must derive actual native root role and reject
# transparent holes where a different surface is hit.
for required in (
    'node_at_coords(seat, lx, ly, &hit, &sx, &sy)',
    'wlr_surface_get_root_surface(hit)',
    'wlr_layer_surface_v1_try_from_wlr_surface(root_surface)',
    'strcmp(role->namespace, STAGE_DIRECTOR_WEATHER_NAMESPACE)',
    'role->current.layer != ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY',
    'stage_director_weather_host_observe_native_peer(host)',
    'stage_director_news_host_session_record_native_candidate(',
    'weather_host_session.last_epoch + 1',
    'UINT64_MAX',
):
    assert required in stage, required

for path in (
    "sway/ipc-server.c", "sway/commands.c", "sway/server.c",
    "sway/desktop/layer_shell.c", "sway/input/seatop_default.c",
):
    source = read(path)
    assert 'stage_director_news_pairing_key_press(' not in source, path
    assert 'stage_director_news_pairing_pointer_button(' not in source, path
    assert 'stage_director_news_pairing_select_at(' not in source, path

for path in (pair, stage, session):
    for banned in (
        'wl_global_create(', 'posix_spawn(', 'fork(', 'execve(',
        'wlr_scene_buffer_set_dest_size(', 'wlr_seat_pointer_notify_button(',
    ):
        assert banned not in path, banned

assert 'operator_watch_authenticated' not in pair
assert 'can_launch' not in pair
assert 'POST_APOLLO_STAGE_DIRECTOR_PAIRING' in header

print("SD_NATIVE_PAIRING_SOURCE: disabled-by-default libinput chord + "
      "exact root layer pointer selection, native revocation and "
      "no IPC/scene/seat/launch authority PASS")
