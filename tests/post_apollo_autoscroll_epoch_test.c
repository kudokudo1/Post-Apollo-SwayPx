/* V5C isolation proof. Synthetic events only, never a physical source. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "sway/input/post_apollo_autoscroll_epoch.h"

#define START 1000u
#define DEVICE 7u
#define SEAT 3u
#define TARGET 91u
#define OUTPUT 21u
#define LAYOUT 8u

static struct pa_autoscroll_witness evidence(uint64_t when) {
    struct pa_autoscroll_witness witness = {
        .origin = PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT,
        .compositor_event = true,
        .surface_hit_verified = true,
        .target_mapped = true,
        .output_enabled = true,
        .layout_generation_verified = true,
        .device_id = DEVICE,
        .seat_id = SEAT,
        .target_node_id = TARGET,
        .output_id = OUTPUT,
        .layout_generation = LAYOUT,
        .observed_at_ms = when,
    };
    /* All claims above are mock inputs, not evidence of a real device. */
    return witness;
}

static struct pa_autoscroll_object_epochs epochs(void) {
    struct pa_autoscroll_object_epochs out = {
        .device = 1001u, .seat = 1002u,
        .target = 1003u, .output = 1004u,
    };
    return out;
}

static void test_missing_or_virtual_origin_rejected(void) {
    struct pa_autoscroll_epoch_guard guard = {0};
    struct pa_autoscroll_witness valid = evidence(START);
    struct pa_autoscroll_object_epochs owner = epochs();
    assert(!pa_autoscroll_epoch_begin(NULL, &valid, &owner, START));
    assert(!pa_autoscroll_epoch_begin(&guard, NULL, &owner, START));
    assert(!guard.active);
    assert(!pa_autoscroll_epoch_begin(&guard, &valid, NULL, START));
    owner.device = 0;
    assert(!pa_autoscroll_epoch_begin(&guard, &valid, &owner, START));
    owner = epochs();
    valid.origin = PA_POINTER_ORIGIN_VIRTUAL;
    assert(!pa_autoscroll_epoch_begin(&guard, &valid, &owner, START));
    assert(!guard.active);
    valid = evidence(START);
    assert(pa_autoscroll_epoch_begin(&guard, &valid, &owner, START));
    assert(guard.active);
    /* A bad restart must not inherit an older authenticated state. */
    assert(!pa_autoscroll_epoch_begin(&guard, NULL, &owner, START));
    assert(!guard.active);
}

static void test_new_timed_evidence_and_cancel(void) {
    struct pa_autoscroll_epoch_guard guard = {0};
    struct pa_autoscroll_witness start = evidence(START);
    struct pa_autoscroll_object_epochs owner = epochs();
    assert(pa_autoscroll_epoch_begin(&guard, &start, &owner, START));
    assert(!pa_autoscroll_epoch_tick(NULL, &start, &owner, START));
    struct pa_autoscroll_witness now = evidence(START + 16);
    assert(pa_autoscroll_epoch_tick(&guard, &now, &owner, START + 16));
    now = evidence(START + 32);
    assert(pa_autoscroll_epoch_tick(&guard, &now, &owner, START + 32));
    /* Reusing the last event is not a fresh sample. */
    assert(!pa_autoscroll_epoch_tick(&guard, &now, &owner, START + 33));
    assert(!guard.active);
    now = evidence(START + 48);
    assert(!pa_autoscroll_epoch_tick(&guard, &now, &owner, START + 48));
    assert(pa_autoscroll_epoch_begin(&guard, &start, &owner, START));
    /* A long-running scroll may outlive the initial 80ms witness window,
     * provided current evidence is newly observed and fresh. */
    now = evidence(START + 5000);
    assert(pa_autoscroll_epoch_tick(&guard, &now, &owner, START + 5000));
    now = evidence(START + 5016);
    assert(!pa_autoscroll_epoch_tick(&guard, &now, &owner, START + 5100));
    assert(!guard.active);
}

static void test_pointer_address_reuse_not_session_reuse(void) {
    const struct pa_autoscroll_object_epochs owner = epochs();
    const struct pa_autoscroll_witness start = evidence(START);
    struct pa_autoscroll_witness now = evidence(START + 16);
    struct pa_autoscroll_epoch_guard guard = {0};
    assert(pa_autoscroll_epoch_begin(&guard, &start, &owner, START));
    struct pa_autoscroll_object_epochs new_owner = owner;
    /* All numerical device/seat/target/output IDs remain identical. Only
     * the object lifetime epoch changes, simulating pointer/address reuse.
     */
    new_owner.device++;
    assert(!pa_autoscroll_epoch_tick(&guard, &now, &new_owner, START + 16));
    assert(!guard.active);
    assert(pa_autoscroll_epoch_begin(&guard, &start, &owner, START));
    new_owner = owner; new_owner.seat++;
    assert(!pa_autoscroll_epoch_tick(&guard, &now, &new_owner, START + 16));
    assert(pa_autoscroll_epoch_begin(&guard, &start, &owner, START));
    new_owner = owner; new_owner.target++;
    assert(!pa_autoscroll_epoch_tick(&guard, &now, &new_owner, START + 16));
    assert(pa_autoscroll_epoch_begin(&guard, &start, &owner, START));
    new_owner = owner; new_owner.output++;
    assert(!pa_autoscroll_epoch_tick(&guard, &now, &new_owner, START + 16));
}

static void test_scene_virtual_lock_and_time_refusals(void) {
    struct pa_autoscroll_epoch_guard guard = {0};
    const struct pa_autoscroll_witness start = evidence(START);
    const struct pa_autoscroll_object_epochs owner = epochs();
    struct pa_autoscroll_witness now = evidence(START + 16);
#define REJECT(field, value) do { \
    assert(pa_autoscroll_epoch_begin(&guard, &start, &owner, START)); \
    now = evidence(START + 16); \
    now.field = (value); \
    assert(!pa_autoscroll_epoch_tick(&guard, &now, &owner, START + 16)); \
    assert(!guard.active); \
} while (0)
    REJECT(origin, PA_POINTER_ORIGIN_VIRTUAL);
    REJECT(compositor_event, false);
    REJECT(target_node_id, TARGET + 1);
    REJECT(output_id, OUTPUT + 1);
    REJECT(layout_generation, LAYOUT + 1);
    REJECT(session_locked, true);
    REJECT(native_scroll_lock_active, true);
    REJECT(stage_director_pairing_consumed, true);
    REJECT(pointer_constrained, true);
    REJECT(surface_hit_verified, false);
    REJECT(output_enabled, false);
    REJECT(observed_at_ms, START - 1);
#undef REJECT
    assert(pa_autoscroll_epoch_begin(&guard, &start, &owner, START));
    now = evidence(START + 16);
    assert(!pa_autoscroll_epoch_tick(&guard, &now, &owner, START + 100));
    assert(!guard.active);
    pa_autoscroll_epoch_cancel(&guard);
    assert(!guard.active);
}

int main(void) {
    test_missing_or_virtual_origin_rejected();
    test_new_timed_evidence_and_cancel();
    test_pointer_address_reuse_not_session_reuse();
    test_scene_virtual_lock_and_time_refusals();
    puts("SWAYPX_V5C_EPOCH_PASS // SYNTHETIC_ONLY // NO_REAL_INPUT");
    return 0;
}
