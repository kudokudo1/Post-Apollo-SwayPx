#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OWNER_SNAPSHOT_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OWNER_SNAPSHOT_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Four C11 values copied from compositor-owned, currently live objects.
 * NOT a hardware credential, authorization, V5C witness, or session ID.
 * This never accepts client-supplied numeric object identities.
 */
struct pa_autoscroll_owner_snapshot {
    uint64_t device;
    uint64_t seat;
    uint64_t target;
    uint64_t output;
};

struct sway_cursor;
struct wlr_input_device;

/*
 * Only call synchronously from the compositor event loop with a live cursor
 * and the native wlr_input_device from that event. Never retain raw pointers
 * in this result or use it to enable the production autoscroll runtime.
 *
 * Zeroes *snapshot on every failure. Rejects virtual/unattached pointers,
 * missing/revoked lifetimes, layer/popup/subsurface hits, inactive outputs,
 * locked sessions and invalidated/transaction-pending window targets.
 *
 * These checks are necessary but insufficient for physical provenance,
 * output layout authenticity, Stage Director pairing or Scroll Lock.
 */
bool pa_autoscroll_owner_snapshot_read(
    struct sway_cursor *cursor,
    struct wlr_input_device *event_device,
    struct pa_autoscroll_owner_snapshot *snapshot);

#endif
