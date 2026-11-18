#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_EPOCH_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_EPOCH_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/input/post_apollo_autoscroll_witness.h"

/*
 * V5C preparation ONLY. This portable guard neither creates a trusted
 * witness nor sends Wayland input. A privileged compositor producer must
 * mint monotonically nonrepeating object-lifetime epochs on create/rebind
 * and invalidate them on destruction. An untrusted caller can forge every
 * field in these structs; do not expose this API through IPC or QML.
 */
struct pa_autoscroll_object_epochs {
    uint64_t device;
    uint64_t seat;
    uint64_t target;
    uint64_t output;
};

struct pa_autoscroll_epoch_guard {
    bool active;
    struct pa_autoscroll_witness started;
    struct pa_autoscroll_object_epochs started_epochs;
    uint64_t last_observed_at_ms;
};

/* Arm only after actual compositor-side event-source verification. */
bool pa_autoscroll_epoch_begin(
    struct pa_autoscroll_epoch_guard *guard,
    const struct pa_autoscroll_witness *witness,
    const struct pa_autoscroll_object_epochs *epochs,
    uint64_t now_ms);

/* Must have a NEW, fresh compositor observation for every tick.
 * Any mismatch permanently cancels this guard until a new begin.
 */
bool pa_autoscroll_epoch_tick(
    struct pa_autoscroll_epoch_guard *guard,
    const struct pa_autoscroll_witness *witness,
    const struct pa_autoscroll_object_epochs *epochs,
    uint64_t now_ms);

void pa_autoscroll_epoch_cancel(struct pa_autoscroll_epoch_guard *guard);

#endif
