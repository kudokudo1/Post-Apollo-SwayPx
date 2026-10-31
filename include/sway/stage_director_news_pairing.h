#ifndef _SWAY_STAGE_DIRECTOR_NEWS_PAIRING_H
#define _SWAY_STAGE_DIRECTOR_NEWS_PAIRING_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <wayland-server-protocol.h>
#include <wlr/types/wlr_input_device.h>
#include <xkbcommon/xkbcommon.h>

struct sway_seat;

/*
 * Disabled unless SwayPX is explicitly started with
 * POST_APOLLO_STAGE_DIRECTOR_PAIRING=1. This is a compositor-native input
 * gesture, not a Sway command, IPC action, public Wayland protocol or QML API.
 *
 * Ctrl+Super+Shift+F12 on an ungrouped libinput keyboard arms a one-shot,
 * three-second selection. A libinput left press then selects the *exact*
 * scene-hit Weather layer shell surface. Consumes the pairing click.
 *
 * libinput rejects ordinary Wayland virtual input, but Linux uinput can
 * emulate a libinput device. This does not promise a cryptographic
 * hardware-presence trust guarantee against a compromised local UID.
 */
bool stage_director_news_pairing_key_press(struct sway_seat *seat,
	struct wlr_input_device *device, bool session_locked,
	const xkb_keysym_t *raw_keysyms, size_t keysyms_len,
	uint32_t raw_modifiers, uint32_t time_msec);

/* true => consume the pointer press/release; ordinary input otherwise. */
bool stage_director_news_pairing_pointer_button(struct sway_seat *seat,
	struct wlr_input_device *device, uint32_t time_msec,
	uint32_t button, enum wl_pointer_button_state state,
	double lx, double ly);

#endif
