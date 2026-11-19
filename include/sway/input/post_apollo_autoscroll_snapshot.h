#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_SNAPSHOT_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_SNAPSHOT_H

#include <stdbool.h>
#include "sway/input/post_apollo_autoscroll_epoch.h"

/* Internal to the compositor. NEVER expose through Wayland clients or IPC.
 * Input pointers must originate from an active compositor callback, not a
 * caller supplied integer. This returns only object-lifetime continuity,
 * NOT physical provenance, permission, a session identity, or live scroll. */
struct sway_cursor;
struct wlr_input_device;

/* Snapshot the four actual owner-issued epochs, only when all four live
 * owner relationships exist and a mapped target is under the pointer.
 * Failures zero the entire result. No reference to an old object is held. */
bool pa_autoscroll_collect_compositor_epochs(
    const struct sway_cursor *cursor, const struct wlr_input_device *device,
    struct pa_autoscroll_object_epochs *out);

#endif
