/* Detached compositor-owned epoch allocator. No devices or Wayland dispatch. */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "sway/input/post_apollo_autoscroll_lifetime.h"

static bool kind_valid(enum pa_autoscroll_lifetime_kind kind) {
    return kind >= PA_LIFETIME_DEVICE && kind <= PA_LIFETIME_OUTPUT;
}

void pa_autoscroll_lifetime_init(struct pa_autoscroll_lifetime_issuer *issuer) {
    if (issuer != NULL) {
        memset(issuer, 0, sizeof(*issuer));
    }
}

bool pa_autoscroll_lifetime_bind(
        struct pa_autoscroll_lifetime_issuer *issuer,
        struct pa_autoscroll_lifetime_slot *slot,
        enum pa_autoscroll_lifetime_kind kind) {
    if (issuer == NULL || slot == NULL || !kind_valid(kind)
            || slot->live || issuer->exhausted) {
        return false;
    }
    if (issuer->latest_epoch == UINT64_MAX) {
        issuer->exhausted = true;
        return false;
    }

    /* A single session-local monotonic sequence spans all four domains,
     * so tokens never collide even if an object address is recycled. */
    uint64_t issued = ++issuer->latest_epoch;
    slot->epoch = issued;
    slot->kind = kind;
    slot->live = true;
    return true;
}

void pa_autoscroll_lifetime_revoke(struct pa_autoscroll_lifetime_slot *slot) {
    if (slot != NULL) {
        slot->live = false;
        /* Do not erase the last value: a subsequent bind uses a new epoch. */
    }
}

uint64_t pa_autoscroll_lifetime_epoch(
        const struct pa_autoscroll_lifetime_slot *slot,
        enum pa_autoscroll_lifetime_kind kind) {
    return slot != NULL && slot->live && kind_valid(kind) &&
            slot->kind == kind ? slot->epoch : 0;
}
