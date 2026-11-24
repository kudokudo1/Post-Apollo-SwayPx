#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_LIFETIME_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_LIFETIME_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Portable object-lifetime epoch ISSUER, independent of pointer addresses.
 * Only the compositor owning a real object lifecycle may bind/revoke slots.
 * Do not accept slot identity, kind, issuer state or tokens from clients/IPC.
 * This is a detached preparation module, not production input activation.
 */
enum pa_autoscroll_lifetime_kind {
    PA_LIFETIME_NONE = 0,
    PA_LIFETIME_DEVICE = 1,
    PA_LIFETIME_SEAT = 2,
    PA_LIFETIME_TARGET = 3,
    PA_LIFETIME_OUTPUT = 4,
};

struct pa_autoscroll_lifetime_issuer {
    uint64_t latest_epoch;
    bool exhausted;
    bool initialized;
};

struct pa_autoscroll_lifetime_slot {
    enum pa_autoscroll_lifetime_kind kind;
    uint64_t epoch;
    bool live;
};

/* The issuer must be zero-initialized. Reject repeated initialization. */
bool pa_autoscroll_lifetime_init(struct pa_autoscroll_lifetime_issuer *issuer);

/* Refuse double bind, invalid kind, zero wraparound and exhaustion. */
bool pa_autoscroll_lifetime_bind(
    struct pa_autoscroll_lifetime_issuer *issuer,
    struct pa_autoscroll_lifetime_slot *slot,
    enum pa_autoscroll_lifetime_kind kind);

/* Revocation invalidates current epoch, including after address reuse. */
void pa_autoscroll_lifetime_revoke(struct pa_autoscroll_lifetime_slot *slot);

/* Return zero unless the slot is live and bound to the expected kind. */
uint64_t pa_autoscroll_lifetime_epoch(
    const struct pa_autoscroll_lifetime_slot *slot,
    enum pa_autoscroll_lifetime_kind kind);

#endif
