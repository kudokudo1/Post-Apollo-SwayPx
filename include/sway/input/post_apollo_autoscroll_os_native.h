#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OS_NATIVE_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OS_NATIVE_H
#include "sway/input/post_apollo_autoscroll_os_session.h"
struct sway_cursor;
struct wlr_pointer_button_event;
/* Private candidate, unlinked in shipping build. A zeroed slot belongs
 * to exactly one live seat; no IPC, credential or positive authority.
 * Called synchronously from the real cursor callback AFTER first refusal.
 * No stored native pointer, no writable mode or lock input. */
struct pa_os_native_slot {
    uint64_t seat_epoch,event_sequence,read_sequence;
    bool exhausted;
};
struct pa_os_native_observation {
    struct pa_os_scroll_event event;
    struct pa_os_scroll_scene scene;
    uint64_t now_ms;
    bool registered,attached;
};
bool pa_os_native_observe(struct pa_os_native_slot *,
    struct sway_cursor *,struct wlr_pointer_button_event *,
    struct pa_os_native_observation *);
#endif
