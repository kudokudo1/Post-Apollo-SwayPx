#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OS_CONTROLS_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OS_CONTROLS_H
#include "sway/input/post_apollo_autoscroll_os_session.h"

/* Private detached state machine. Zeroed objects, owned by live compositor
 * seat and keyboard lifetimes in a future native bridge. No hardware proof,
 * IPC, key capture, keyboard LED writes, or Wayland output. */
enum pa_os_control_key { PA_OS_KEY_SCROLL_LOCK=1, PA_OS_KEY_ESCAPE=2 };
enum pa_os_control_edge { PA_OS_KEY_RELEASE=0, PA_OS_KEY_PRESS=1, PA_OS_KEY_REPEAT=2 };
enum pa_os_control_source { PA_OS_KEY_UNKNOWN=0, PA_OS_KEY_MANAGED=1 };
struct pa_os_controls {
    uint64_t seat_epoch, policy_epoch, last_sequence;
    enum pa_os_scroll_mode mode;
    enum pa_os_scroll_lock lock;
    bool exhausted;
};
struct pa_os_keyboard_controls {
    uint64_t seat_epoch, device_epoch;
    bool scroll_held, escape_held, retired;
};
struct pa_os_control_event {
    uint64_t seat_epoch, device_epoch, sequence;
    enum pa_os_control_key key;
    enum pa_os_control_edge edge;
    enum pa_os_control_source source;
};
bool pa_os_controls_init(struct pa_os_controls *,uint64_t seat_epoch);
bool pa_os_keyboard_controls_init(struct pa_os_keyboard_controls *,
    uint64_t seat_epoch,uint64_t device_epoch);
/* Mode selection is separate from arming; every configuration change
 * disarms and cancels. Only the trusted owner can call these APIs. */
bool pa_os_controls_select(struct pa_os_controls *,struct pa_os_scroll_session *,
    enum pa_os_scroll_mode);
void pa_os_controls_invalidate(struct pa_os_controls *,struct pa_os_scroll_session *);
void pa_os_keyboard_controls_retire(struct pa_os_controls *,
    struct pa_os_keyboard_controls *,struct pa_os_scroll_session *);
/* Returns true only if an accepted edge changed control state or
 * requested cancellation. Never means consume the native key. */
bool pa_os_controls_key(struct pa_os_controls *,struct pa_os_keyboard_controls *,
    struct pa_os_scroll_session *,const struct pa_os_control_event *);
#endif
