#include <stdlib.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <wlr/config.h>
#if WLR_HAS_LIBINPUT_BACKEND
#include <wlr/backend/libinput.h>
#endif
#include <wlr/types/wlr_keyboard.h>
#include "log.h"
#include "sway/stage_director.h"
#include "sway/stage_director_news_pairing.h"

#define PAIR_WINDOW_MSEC 3000U
#define PAIR_MODIFIERS (WLR_MODIFIER_CTRL | WLR_MODIFIER_LOGO | WLR_MODIFIER_SHIFT)

/* Compositor-only, one seat at a time. Never passed through public IPC. */
static struct {
	struct sway_seat *armed_seat;
	uint32_t armed_at;
	bool armed;
	struct sway_seat *swallow_seat;
	struct wlr_input_device *swallow_device;
	uint32_t swallow_button;
	bool swallow_release;
} pairing;

static bool enabled(void) {
	const char *value = getenv("POST_APOLLO_STAGE_DIRECTOR_PAIRING");
	return value && strcmp(value, "1") == 0;
}

static bool libinput_device(struct wlr_input_device *device,
		enum wlr_input_device_type type) {
#if WLR_HAS_LIBINPUT_BACKEND
	return device && device->type == type && wlr_input_device_is_libinput(device);
#else
	(void)device;
	(void)type;
	return false;
#endif
}

bool stage_director_news_pairing_key_press(struct sway_seat *seat,
		struct wlr_input_device *device, bool session_locked,
		const xkb_keysym_t *raw_keysyms, size_t keysyms_len,
		uint32_t raw_modifiers, uint32_t time_msec) {
	if (!enabled() || session_locked || !seat ||
			!libinput_device(device, WLR_INPUT_DEVICE_KEYBOARD) ||
			raw_modifiers != PAIR_MODIFIERS ||
			!raw_keysyms || keysyms_len != 1 ||
			raw_keysyms[0] != XKB_KEY_F12 ||
			pairing.swallow_release) {
		return false;
	}
	/* A fresh physical-key gesture revokes the previously selected host.
	 * The following click MUST be hit-tested, never inferred by namespace. */
	stage_director_news_pairing_clear_host();
	pairing.armed_seat = seat;
	pairing.armed_at = time_msec;
	pairing.armed = true;
	sway_log(SWAY_INFO, "[stage-director][SD1][PAIR] armed via libinput "
			"keyboard; exact visible Weather layer required");
	return true;
}

bool stage_director_news_pairing_pointer_button(struct sway_seat *seat,
		struct wlr_input_device *device, uint32_t time_msec,
		uint32_t button, enum wl_pointer_button_state state,
		double lx, double ly) {
	/* A swallowed press also swallows exactly its matching release. */
	if (state == WL_POINTER_BUTTON_STATE_RELEASED &&
			pairing.swallow_release && pairing.swallow_seat == seat &&
			pairing.swallow_device == device &&
			pairing.swallow_button == button) {
		pairing.swallow_release = false;
		pairing.swallow_seat = NULL;
		pairing.swallow_device = NULL;
		return true;
	}
	if (!pairing.armed || state != WL_POINTER_BUTTON_STATE_PRESSED) {
		return false;
	}
	const bool fresh = (uint32_t)(time_msec - pairing.armed_at) <=
		PAIR_WINDOW_MSEC;
	const bool same_seat = seat == pairing.armed_seat;
	/* One pointer press, successful or not, consumes the ceremony. */
	pairing.armed = false;
	pairing.armed_seat = NULL;
	if (!fresh || !same_seat || !enabled() ||
			!libinput_device(device, WLR_INPUT_DEVICE_POINTER)) {
		sway_log(SWAY_DEBUG, "[stage-director][SD1][PAIR] denied: "
				"expired, seat mismatch or non-libinput pointer");
		return false;
	}
	/* Require left click and exact compositor scene hit; no transparent
	 * rectangle fallback, namespace scan, or focused-layer inference. */
	const bool selected = button == BTN_LEFT &&
		stage_director_news_pairing_select_at(seat, lx, ly);
	pairing.swallow_release = true;
	pairing.swallow_seat = seat;
	pairing.swallow_device = device;
	pairing.swallow_button = button;
	sway_log(SWAY_INFO, "[stage-director][SD1][PAIR] physical click "
			"selection %s; WATCH and all compositor actuation denied",
			selected ? "native-candidate" : "denied");
	return true;
}
