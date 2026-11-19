/* V5C epoch guard, compiled into SwayPX but not connected to live input. */
#include <stddef.h>
#include <string.h>
#include "sway/input/post_apollo_autoscroll_epoch.h"

static bool epochs_valid(const struct pa_autoscroll_object_epochs *epochs) {
    return epochs != NULL && epochs->device != 0 && epochs->seat != 0
        && epochs->target != 0 && epochs->output != 0;
}

static bool epochs_equal(const struct pa_autoscroll_object_epochs *left,
        const struct pa_autoscroll_object_epochs *right) {
    return epochs_valid(left) && epochs_valid(right)
        && left->device == right->device
        && left->seat == right->seat
        && left->target == right->target
        && left->output == right->output;
}

void pa_autoscroll_epoch_cancel(struct pa_autoscroll_epoch_guard *guard) {
    if (guard != NULL) {
        memset(guard, 0, sizeof(*guard));
    }
}

bool pa_autoscroll_epoch_begin(
        struct pa_autoscroll_epoch_guard *guard,
        const struct pa_autoscroll_witness *witness,
        const struct pa_autoscroll_object_epochs *epochs,
        uint64_t now_ms) {
    if (guard == NULL) {
        return false;
    }
    pa_autoscroll_epoch_cancel(guard);
    if (!epochs_valid(epochs) ||
            !pa_autoscroll_witness_admissible(witness, now_ms)) {
        return false;
    }
    guard->started = *witness;
    guard->started_epochs = *epochs;
    guard->last_observed_at_ms = witness->observed_at_ms;
    guard->active = true;
    return true;
}

bool pa_autoscroll_epoch_tick(
        struct pa_autoscroll_epoch_guard *guard,
        const struct pa_autoscroll_witness *witness,
        const struct pa_autoscroll_object_epochs *epochs,
        uint64_t now_ms) {
    if (guard == NULL || !guard->active) {
        return false;
    }
    /* Witness IDs might be reused after object teardown. Epochs must be
     * minted by the owner, never inferred from pointer addresses alone.
     * Require new evidence on every tick, not a recycled last event.
     */
    if (witness == NULL || !epochs_equal(&guard->started_epochs, epochs) ||
            witness->observed_at_ms <= guard->last_observed_at_ms ||
            !pa_autoscroll_witness_same_session(
                &guard->started, witness, now_ms)) {
        pa_autoscroll_epoch_cancel(guard);
        return false;
    }
    guard->last_observed_at_ms = witness->observed_at_ms;
    return true;
}
