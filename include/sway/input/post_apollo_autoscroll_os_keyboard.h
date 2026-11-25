#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OS_KEYBOARD_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OS_KEYBOARD_H
#include "sway/input/post_apollo_autoscroll_os_controls.h"
struct sway_keyboard;
struct wlr_keyboard_key_event;
/* Unlinked private keyboard observation. Seat-local namespace, no
 * pointer identities retained. Caller owns native callback and lifecycle.
 * A managed source is OS-boundary eligibility only, not physical proof,
 * selection, Company clearance or authorization to consume any key. */
struct pa_os_key_owner {
    uint64_t seat_epoch, next_keyboard, sequence;
    bool exhausted;
};
struct pa_os_key_binding {
    struct pa_os_keyboard_controls control;
    uint64_t last_scroll_ms,last_escape_ms;
};
struct pa_os_key_observation {
    struct pa_os_control_event event;
    bool registered,attached,native_libinput,live_handle;
    bool coherent_tree,known_synthetic,protocol_virtual,fresh,unlocked;
};
bool pa_os_key_bind(struct pa_os_key_owner *,struct pa_os_key_binding *,
    struct sway_keyboard *);
bool pa_os_key_observe(struct pa_os_key_owner *,struct pa_os_key_binding *,
    struct sway_keyboard *,const struct wlr_keyboard_key_event *,
    struct pa_os_key_observation *);
void pa_os_key_retire(struct pa_os_key_binding *);
#endif
