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
#include "sway/stage_director_company_pairing.h"

#define COMPANY_PAIR_WINDOW_MSEC 3000U
#define COMPANY_PAIR_MODIFIERS \
	(WLR_MODIFIER_CTRL | WLR_MODIFIER_LOGO | WLR_MODIFIER_SHIFT)

/* Isolated from Weather's F12 gesture/session and never callable by IPC. */
static struct {
	struct sway_seat *armed_seat;
	uint32_t armed_at;
	bool armed;
	struct sway_seat *swallow_seat;
	struct wlr_input_device *swallow_device;
	uint32_t swallow_button;
	uint32_t swallow_at;
	bool swallow_release;
} company_pairing;

static bool enabled(void) {
	const char *flag = getenv("POST_APOLLO_STAGE_DIRECTOR_COMPANY_PAIRING");
	return flag && strcmp(flag, "1") == 0;
}

static bool physical_libinput(struct wlr_input_device *device,
		enum wlr_input_device_type type) {
#if WLR_HAS_LIBINPUT_BACKEND
	return device && device->type == type &&
		wlr_input_device_is_libinput(device);
#else
	(void)device;
	(void)type;
	return false;
#endif
}

static void expire_swallowed_release(uint32_t now) {
	if (company_pairing.swallow_release &&
			(uint32_t)(now - company_pairing.swallow_at) >
				COMPANY_PAIR_WINDOW_MSEC) {
		company_pairing.swallow_release = false;
		company_pairing.swallow_seat = NULL;
		company_pairing.swallow_device = NULL;
	}
}

bool stage_director_company_pairing_key_press(
		struct sway_seat *seat, struct wlr_input_device *device,
		bool session_locked, bool shortcuts_inhibited,
		const xkb_keysym_t *raw_keysyms, size_t keysym_len,
		uint32_t raw_modifiers, uint32_t time_msec) {
	expire_swallowed_release(time_msec);
	if (!time_msec || !enabled() || session_locked ||
			shortcuts_inhibited || !seat ||
			!physical_libinput(device, WLR_INPUT_DEVICE_KEYBOARD) ||
			raw_modifiers != COMPANY_PAIR_MODIFIERS ||
			!raw_keysyms || keysym_len != 1 ||
			raw_keysyms[0] != XKB_KEY_F11 ||
			company_pairing.swallow_release) {
		return false;
	}
	/* New real-key gesture cancels previous candidate. An IPC sender
	 * cannot arm this state, select a surface or publish a global. */
	stage_director_company_pairing_clear_host();
	company_pairing.armed = true;
	company_pairing.armed_seat = seat;
	company_pairing.armed_at = time_msec;
	sway_log(SWAY_INFO, "[stage-director][COMPANY_M24][PAIR] "
		"armed via libinput keyboard; painted Company layer required");
	return true;
}

bool stage_director_company_pairing_pointer_button(
		struct sway_seat *seat, struct wlr_input_device *device,
		uint32_t time_msec, bool session_locked, uint32_t button,
		enum wl_pointer_button_state state, double lx, double ly) {
	expire_swallowed_release(time_msec);
	if (state == WL_POINTER_BUTTON_STATE_RELEASED &&
			company_pairing.swallow_release &&
			company_pairing.swallow_seat == seat &&
			company_pairing.swallow_device == device &&
			company_pairing.swallow_button == button) {
		company_pairing.swallow_release = false;
		company_pairing.swallow_seat = NULL;
		company_pairing.swallow_device = NULL;
		return true;
	}
	if (!company_pairing.armed ||
			state != WL_POINTER_BUTTON_STATE_PRESSED) {
		return false;
	}
	const bool fresh =
		(uint32_t)(time_msec - company_pairing.armed_at) <=
			COMPANY_PAIR_WINDOW_MSEC;
	const bool same_seat = seat == company_pairing.armed_seat;
	company_pairing.armed = false;
	company_pairing.armed_seat = NULL;
	if (session_locked || !time_msec || !fresh || !same_seat ||
			!enabled() ||
			!physical_libinput(device, WLR_INPUT_DEVICE_POINTER)) {
		sway_log(SWAY_DEBUG, "[stage-director][COMPANY_M24][PAIR] "
			"denied: stale, locked, wrong seat or non-libinput");
		return false;
	}
	/* Exactly one pointer press consumes the ceremony. A failed Company
	 * hit test cannot fall through and select a different namespace. */
	const bool selected = button == BTN_LEFT &&
		stage_director_company_pairing_select_at(seat, lx, ly);
	company_pairing.swallow_release = true;
	company_pairing.swallow_seat = seat;
	company_pairing.swallow_device = device;
	company_pairing.swallow_button = button;
	company_pairing.swallow_at = time_msec;
	sway_log(SWAY_INFO, "[stage-director][COMPANY_M24][PAIR] "
		"physical hit selection=%s geometry=ONLY scene=OFF input=OFF",
		selected ? "candidate" : "denied");
	return true;
}
