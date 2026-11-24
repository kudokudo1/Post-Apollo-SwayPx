#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_GUARD_BRIDGE_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_GUARD_BRIDGE_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/input/post_apollo_autoscroll_epoch.h"

/* Native, compositor-private glue for four owner-issued epoch sources.
 * No IPC or user-authorizable witness input. HARD OFF until independent,
 * trusted physical origin, native scroll-mode and layout witnesses exist.
 */
struct sway_cursor;
struct wlr_input_device;

struct pa_autoscroll_guard_bridge {
    struct pa_autoscroll_epoch_guard epoch;
};

/* Source snapshot is read from live compositor-owned objects. The native
 * witness is intentionally inadmissible today. Deny without consuming any
 * mouse event. Every failure erases guard state.
 */
bool pa_autoscroll_guard_bridge_begin(
    struct pa_autoscroll_guard_bridge *bridge,
    struct sway_cursor *cursor, struct wlr_input_device *device,
    uint64_t event_at_ms, uint64_t now_ms);

/* Called BEFORE any axis intent is dispatched. Timer ticks are not freshly
 * observed pointer events, so no fabricated positive freshness is allowed.
 */
bool pa_autoscroll_guard_bridge_tick(
    struct pa_autoscroll_guard_bridge *bridge,
    struct sway_cursor *cursor, struct wlr_input_device *device,
    uint64_t now_ms);

void pa_autoscroll_guard_bridge_cancel(
    struct pa_autoscroll_guard_bridge *bridge);

#endif
