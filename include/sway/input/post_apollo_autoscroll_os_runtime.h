#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OS_RUNTIME_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OS_RUNTIME_H
#include "sway/input/post_apollo_autoscroll_os_ingress.h"
#include "sway/input/post_apollo_autoscroll_os_scene.h"
#include "sway/input/post_apollo_autoscroll_os_keyboard.h"
struct sway_seat;
struct wlr_pointer_motion_event;
/* Unlinked compositor-private seat runtime. Owns one controller, one
 * native input/read sequence and one lazy timer. No client IPC, no public
 * enable switch, no source injection. Layout owner outlives seat runtime.
 * Clearance must come from a reviewed synchronous compositor owner:
 * no provider means no admission. The provider must not mutate scene,
 * dispatch input or re-enter runtime. Its lifetime is the seat lifetime.
 * No approved production clearance provider exists at this milestone. */
typedef bool (*pa_os_scene_clearance_reader)(struct sway_seat *,
    const struct pa_os_scroll_scene *,struct pa_os_ingress_clearance *);
struct pa_os_runtime {
    struct pa_os_controller controller;
    struct pa_os_native_slot native;
    struct pa_os_key_owner keyboard_owner;
    const struct pa_os_layout_owner *layout;
    pa_os_scene_clearance_reader clearance;
    struct wl_event_source *timer;
    bool retired;
};
bool pa_os_runtime_init(struct pa_os_runtime *,struct sway_seat *,
    const struct pa_os_layout_owner *,pa_os_scene_clearance_reader);
bool pa_os_runtime_select(struct pa_os_runtime *,enum pa_os_scroll_mode);
bool pa_os_runtime_bind_key(struct pa_os_runtime *,struct pa_os_key_binding *,struct sway_keyboard *);
void pa_os_runtime_key(struct pa_os_runtime *,struct pa_os_key_binding *,
    struct sway_keyboard *,const struct wlr_keyboard_key_event *);
void pa_os_runtime_retire_key(struct pa_os_runtime *,struct pa_os_key_binding *);
bool pa_os_runtime_middle(struct pa_os_runtime *,struct sway_cursor *,struct wlr_pointer_button_event *);
bool pa_os_runtime_motion(struct pa_os_runtime *,struct sway_cursor *,struct wlr_pointer_motion_event *);
void pa_os_runtime_cancel(struct pa_os_runtime *);
void pa_os_runtime_invalidate(struct pa_os_runtime *);
void pa_os_runtime_retire_device(struct pa_os_runtime *,uint64_t);
void pa_os_runtime_finish(struct pa_os_runtime *);
#endif
