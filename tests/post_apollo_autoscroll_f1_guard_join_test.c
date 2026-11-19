/*
 * V5E-F1: synthetic compositor owner lifecycle transitions into the
 * genuine V5C epoch guard. DOES NOT authenticate real hardware or mint a
 * positive production witness. No sockets, devices or Wayland events.
 */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "sway/input/post_apollo_autoscroll_epoch.h"
#include "sway/input/post_apollo_autoscroll_lifetime.h"

struct fixture {
    struct pa_autoscroll_lifetime_issuer issuer;
    struct pa_autoscroll_lifetime_slot device;
    struct pa_autoscroll_lifetime_slot seat;
    struct pa_autoscroll_lifetime_slot target;
    struct pa_autoscroll_lifetime_slot output;
};

static void init(struct fixture *f) {
    assert(pa_autoscroll_lifetime_init(&f->issuer));
    assert(pa_autoscroll_lifetime_bind(
        &f->issuer, &f->device, PA_LIFETIME_DEVICE));
    assert(pa_autoscroll_lifetime_bind(
        &f->issuer, &f->seat, PA_LIFETIME_SEAT));
    assert(pa_autoscroll_lifetime_bind(
        &f->issuer, &f->target, PA_LIFETIME_TARGET));
    assert(pa_autoscroll_lifetime_bind(
        &f->issuer, &f->output, PA_LIFETIME_OUTPUT));
}

static struct pa_autoscroll_object_epochs take(const struct fixture *f) {
    return (struct pa_autoscroll_object_epochs) {
        .device = pa_autoscroll_lifetime_epoch(&f->device, PA_LIFETIME_DEVICE),
        .seat = pa_autoscroll_lifetime_epoch(&f->seat, PA_LIFETIME_SEAT),
        .target = pa_autoscroll_lifetime_epoch(&f->target, PA_LIFETIME_TARGET),
        .output = pa_autoscroll_lifetime_epoch(&f->output, PA_LIFETIME_OUTPUT),
    };
}

static struct pa_autoscroll_witness mock(uint64_t time) {
    /* Mock values deliberately assert all positive flags. Such flags are
     * FORBIDDEN in production absent independently verified provenance. */
    return (struct pa_autoscroll_witness) {
        .origin = PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT,
        .compositor_event = true,
        .surface_hit_verified = true,
        .target_mapped = true,
        .output_enabled = true,
        .layout_generation_verified = true,
        .device_id = 11,
        .seat_id = 22,
        .target_node_id = 33,
        .output_id = 44,
        .layout_generation = 55,
        .observed_at_ms = time,
    };
}

static void test_real_issuer_feeds_guard_without_replay(void) {
    struct fixture f = {0};
    init(&f);
    struct pa_autoscroll_object_epochs before = take(&f);
    assert(before.device != 0 && before.seat != 0 &&
           before.target != 0 && before.output != 0);
    assert(before.device != before.seat && before.device != before.output);
    struct pa_autoscroll_epoch_guard guard = {0};
    struct pa_autoscroll_witness start = mock(1000);
    struct pa_autoscroll_witness new_obs = mock(1016);
    assert(pa_autoscroll_epoch_begin(&guard, &start, &before, 1000));
    assert(pa_autoscroll_epoch_tick(&guard, &new_obs, &before, 1016));
    /* Replayed observations cancel the guard, even when epochs match. */
    assert(!pa_autoscroll_epoch_tick(&guard, &new_obs, &before, 1017));
    assert(!guard.active);
}

static void test_each_lifetime_revoke_and_rebind_cancels(void) {
    struct fixture f = {0};
    init(&f);
    struct pa_autoscroll_lifetime_slot *slots[4] = {
        &f.device, &f.seat, &f.target, &f.output,
    };
    enum pa_autoscroll_lifetime_kind kinds[4] = {
        PA_LIFETIME_DEVICE, PA_LIFETIME_SEAT,
        PA_LIFETIME_TARGET, PA_LIFETIME_OUTPUT,
    };
    for (int i = 0; i < 4; ++i) {
        const uint64_t start_time = 2000u + (uint64_t)i * 100u;
        const struct pa_autoscroll_object_epochs before = take(&f);
        struct pa_autoscroll_epoch_guard revoked_guard = {0};
        struct pa_autoscroll_epoch_guard rebind_guard = {0};
        struct pa_autoscroll_witness start = mock(start_time);
        struct pa_autoscroll_witness now = mock(start_time + 16u);
        assert(pa_autoscroll_epoch_begin(
            &revoked_guard, &start, &before, start_time));
        assert(pa_autoscroll_epoch_begin(
            &rebind_guard, &start, &before, start_time));
        pa_autoscroll_lifetime_revoke(slots[i]);
        const struct pa_autoscroll_object_epochs gone = take(&f);
        assert(!pa_autoscroll_epoch_tick(
            &revoked_guard, &now, &gone, start_time + 16u));
        assert(!revoked_guard.active);
        assert(pa_autoscroll_lifetime_bind(&f.issuer, slots[i], kinds[i]));
        const struct pa_autoscroll_object_epochs after = take(&f);
        assert(!pa_autoscroll_epoch_tick(
            &rebind_guard, &now, &after, start_time + 16u));
        assert(!rebind_guard.active);
        assert(pa_autoscroll_lifetime_epoch(slots[i], kinds[i]) >
            pa_autoscroll_lifetime_epoch(slots[i], PA_LIFETIME_NONE));
        struct pa_autoscroll_epoch_guard fresh = {0};
        assert(pa_autoscroll_epoch_begin(
            &fresh, &start, &after, start_time));
        assert(pa_autoscroll_epoch_tick(
            &fresh, &now, &after, start_time + 16u));
    }
}

static void test_uninitialized_or_wrong_domain_denied(void) {
    struct fixture f = {0};
    assert(take(&f).device == 0);
    struct pa_autoscroll_epoch_guard guard = {0};
    struct pa_autoscroll_witness witness = mock(1000);
    struct pa_autoscroll_object_epochs zero = take(&f);
    assert(!pa_autoscroll_epoch_begin(&guard, &witness, &zero, 1000));
    assert(!guard.active);
    init(&f);
    assert(pa_autoscroll_lifetime_epoch(&f.device, PA_LIFETIME_SEAT) == 0);
    assert(pa_autoscroll_lifetime_epoch(&f.output, PA_LIFETIME_TARGET) == 0);
    assert(!pa_autoscroll_lifetime_init(&f.issuer));
}

int main(void) {
    test_real_issuer_feeds_guard_without_replay();
    test_each_lifetime_revoke_and_rebind_cancels();
    test_uninitialized_or_wrong_domain_denied();
    puts("PA_V5E_F1_ISSUER_TO_GUARD_CANCELLATION_PASS "
         "SYNTHETIC_WITNESS_ONLY REAL_INPUT=OFF");
    return 0;
}
