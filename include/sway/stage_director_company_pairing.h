#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_PAIRING_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_PAIRING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <wayland-server-protocol.h>
#include <wlr/types/wlr_input_device.h>
#include <xkbcommon/xkbcommon.h>

struct sway_seat;

/*
 * M24 // Company-only physical native selection.
 *
 * Disabled without POST_APOLLO_STAGE_DIRECTOR_COMPANY_PAIRING=1.
 * Ctrl+Super+Shift+F11 on libinput keyboard arms the next 3-second
 * libinput left click to hit-test an exact painted Company layer.
 * Weather's separate F12 ceremony remains unchanged.
 *
 * This is a user gesture gate, NOT a cryptographic hardware-presence
 * proof against compromised local UID or malicious Linux uinput.
 */
bool stage_director_company_pairing_key_press(
	struct sway_seat *seat, struct wlr_input_device *device,
	bool session_locked, bool shortcuts_inhibited,
	const xkb_keysym_t *raw_keysyms, size_t keysyms_len,
	uint32_t raw_modifiers, uint32_t time_msec);

bool stage_director_company_pairing_pointer_button(
	struct sway_seat *seat, struct wlr_input_device *device,
	uint32_t time_msec, bool session_locked, uint32_t button,
	enum wl_pointer_button_state state, double lx, double ly);

#endif
