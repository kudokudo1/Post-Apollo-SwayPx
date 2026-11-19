/* V5F-A: synthetic object lifecycle compatibility with pinned V5C guard.
 * Uses only the actual monotonic lifetime issuer from D1-D3 and the V5C
 * epoch/witness portable code from the exact master pin. The positive
 * witness values below are MOCKS, NOT real physical device proof.
 */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sway/input/post_apollo_autoscroll_lifetime.h"
#include "sway/input/post_apollo_autoscroll_owner_snapshot.h"
#include "sway/input/post_apollo_autoscroll_epoch.h"

enum { DEVICE_SLOT, SEAT_SLOT, TARGET_SLOT, OUTPUT_SLOT, SLOT_COUNT };

static const enum pa_autoscroll_lifetime_kind kinds[SLOT_COUNT] = {
    PA_LIFETIME_DEVICE, PA_LIFETIME_SEAT, PA_LIFETIME_TARGET,
    PA_LIFETIME_OUTPUT,
};

static struct pa_autoscroll_owner_snapshot read_slots(
        struct pa_autoscroll_lifetime_slot slots[SLOT_COUNT]) {
    return (struct pa_autoscroll_owner_snapshot) {
        .device = pa_autoscroll_lifetime_epoch(&slots[DEVICE_SLOT], PA_LIFETIME_DEVICE),
        .seat = pa_autoscroll_lifetime_epoch(&slots[SEAT_SLOT], PA_LIFETIME_SEAT),
        .target = pa_autoscroll_lifetime_epoch(&slots[TARGET_SLOT], PA_LIFETIME_TARGET),
        .output = pa_autoscroll_lifetime_epoch(&slots[OUTPUT_SLOT], PA_LIFETIME_OUTPUT),
    };
}

static struct pa_autoscroll_object_epochs into_guard(
        struct pa_autoscroll_owner_snapshot owner) {
    return (struct pa_autoscroll_object_epochs) {
        .device = owner.device, .seat = owner.seat,
        .target = owner.target, .output = owner.output,
    };
}

static struct pa_autoscroll_witness mocked_evidence(uint64_t at) {
    return (struct pa_autoscroll_witness) {
        .origin = PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT, /* MOCK ONLY */
        .compositor_event = true, /* MOCK ONLY */
        .surface_hit_verified = true,
        .target_mapped = true,
        .output_enabled = true,
        .layout_generation_verified = true, /* MOCK ONLY */
        .device_id = 101,
        .seat_id = 202,
        .target_node_id = 303,
        .output_id = 404,
        .layout_generation = 505,
        .observed_at_ms = at,
    };
}

static void test_four_domain_revocation_and_address_reuse(void) {
    struct pa_autoscroll_lifetime_issuer issuer = {0};
    struct pa_autoscroll_lifetime_slot slots[SLOT_COUNT] = {0};
    assert(pa_autoscroll_lifetime_init(&issuer));
    for (unsigned i = 0; i < SLOT_COUNT; i++) {
        assert(pa_autoscroll_lifetime_bind(&issuer, &slots[i], kinds[i]));
    }
    struct pa_autoscroll_owner_snapshot first = read_slots(slots);
    assert(first.device && first.seat && first.target && first.output);
    assert(first.device != first.seat && first.seat != first.target &&
           first.target != first.output);

    struct pa_autoscroll_witness started = mocked_evidence(1000);
    struct pa_autoscroll_witness later = mocked_evidence(1016);
    struct pa_autoscroll_object_epochs original = into_guard(first);
    struct pa_autoscroll_epoch_guard guard = {0};

    assert(pa_autoscroll_epoch_begin(&guard, &started, &original, 1000));
    assert(pa_autoscroll_epoch_tick(&guard, &later, &original, 1016));

    for (unsigned i = 0; i < SLOT_COUNT; i++) {
        /* Each domain starts from its OWN current four-token baseline.
         * Otherwise an earlier domain's already-changed epoch could make
         * all later cancellation cases pass for the wrong reason. */
        struct pa_autoscroll_object_epochs before =
            into_guard(read_slots(slots));
        assert(pa_autoscroll_epoch_begin(&guard, &started, &before, 1000));
        pa_autoscroll_lifetime_revoke(&slots[i]);
        struct pa_autoscroll_object_epochs missing = into_guard(read_slots(slots));
        assert(!pa_autoscroll_epoch_tick(&guard, &later, &missing, 1016));
        assert(!guard.active);

        /* Reusing the very same C slot address cannot restore identity. */
        assert(pa_autoscroll_lifetime_bind(&issuer, &slots[i], kinds[i]));
        struct pa_autoscroll_object_epochs replacement =
            into_guard(read_slots(slots));
        assert(!pa_autoscroll_epoch_tick(&guard, &later, &replacement, 1016));
        assert(pa_autoscroll_epoch_begin(&guard, &started, &before, 1000));
        assert(!pa_autoscroll_epoch_tick(&guard, &later, &replacement, 1016));
        assert(!guard.active);
    }
}

static void test_new_session_not_inherited_from_replayed_witness(void) {
    struct pa_autoscroll_lifetime_issuer issuer = {0};
    struct pa_autoscroll_lifetime_slot slots[SLOT_COUNT] = {0};
    assert(pa_autoscroll_lifetime_init(&issuer));
    for (unsigned i = 0; i < SLOT_COUNT; i++) {
        assert(pa_autoscroll_lifetime_bind(&issuer, &slots[i], kinds[i]));
    }
    struct pa_autoscroll_object_epochs owner = into_guard(read_slots(slots));
    struct pa_autoscroll_witness started = mocked_evidence(1000);
    struct pa_autoscroll_witness later = mocked_evidence(1016);
    struct pa_autoscroll_epoch_guard guard = {0};

    assert(pa_autoscroll_epoch_begin(&guard, &started, &owner, 1000));
    /* A repeated observation has no right to keep a running session alive. */
    assert(!pa_autoscroll_epoch_tick(&guard, &started, &owner, 1001));
    assert(!guard.active);

    assert(pa_autoscroll_epoch_begin(&guard, &started, &owner, 1000));
    later.native_scroll_lock_active = true;
    assert(!pa_autoscroll_epoch_tick(&guard, &later, &owner, 1016));
    assert(!guard.active);

    assert(pa_autoscroll_epoch_begin(&guard, &started, &owner, 1000));
    later = mocked_evidence(1016);
    later.origin = PA_POINTER_ORIGIN_VIRTUAL;
    assert(!pa_autoscroll_epoch_tick(&guard, &later, &owner, 1016));
    assert(!guard.active);

    assert(pa_autoscroll_epoch_begin(&guard, &started, &owner, 1000));
    later = mocked_evidence(1016);
    later.layout_generation = 506;
    assert(!pa_autoscroll_epoch_tick(&guard, &later, &owner, 1016));
    assert(!guard.active);
}

static void test_wrong_kind_and_uninitialized_issuer(void) {
    struct pa_autoscroll_lifetime_issuer issuer = {0};
    struct pa_autoscroll_lifetime_slot slot = {0};
    assert(!pa_autoscroll_lifetime_bind(&issuer, &slot, PA_LIFETIME_DEVICE));
    assert(pa_autoscroll_lifetime_init(&issuer));
    assert(pa_autoscroll_lifetime_bind(&issuer, &slot, PA_LIFETIME_DEVICE));
    assert(pa_autoscroll_lifetime_epoch(&slot, PA_LIFETIME_SEAT) == 0);
    assert(pa_autoscroll_lifetime_epoch(&slot, PA_LIFETIME_DEVICE) != 0);
    pa_autoscroll_lifetime_revoke(&slot);
    assert(pa_autoscroll_lifetime_epoch(&slot, PA_LIFETIME_DEVICE) == 0);
}

int main(void) {
    test_four_domain_revocation_and_address_reuse();
    test_new_session_not_inherited_from_replayed_witness();
    test_wrong_kind_and_uninitialized_issuer();
    puts("PA_V5F_A_V5C_PINNED_CONTRACT_PASS // SYNTHETIC_NOT_PHYSICAL");
    return 0;
}
