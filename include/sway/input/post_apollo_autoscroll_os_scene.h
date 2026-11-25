#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OS_SCENE_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OS_SCENE_H
#include <wayland-server-core.h>
#include "sway/input/post_apollo_autoscroll_os_native.h"
struct wlr_output_layout;
/* Private root-lifetime layout revision. Zero initialized, one attachment;
 * destruction or exhaustion terminally invalidates it. Only the live
 * layout owner may attach; this is not a complete scene revision. */
struct pa_os_layout_owner {
    struct wl_listener change,destroy;
    struct wlr_output_layout *layout;
    uint64_t revision;
    bool retired;
};
bool pa_os_layout_attach(struct pa_os_layout_owner *,struct wlr_output_layout *);
uint64_t pa_os_layout_revision(const struct pa_os_layout_owner *);
struct pa_os_scene_read {
    struct pa_os_scroll_scene scene;
    bool source_candidate;
    bool device_resolved,seat_resolved,layout_matches;
};
/* Timer path: resolve original owner epochs from LIVE lists, never retain
 * cursor/device/surface pointers from a previous callback. Increment only
 * read_sequence in the seat's existing native slot; event_sequence remains
 * unchanged. Output is observation only: mode OFF and unresolved gates deny.
 * Caller must cancel on false or any mismatch, never infer authority. */
bool pa_os_scene_read(struct pa_os_native_slot *,const struct pa_os_layout_owner *,
    uint64_t device_epoch,uint64_t expected_layout,struct pa_os_scene_read *);
#endif
