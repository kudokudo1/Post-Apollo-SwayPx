/*
 * Synthetic-only witness and axis-intent tests.
 * No physical device, wl_pointer client, input injection or compositor runs.
 */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sway/input/post_apollo_autoscroll_witness.h"
#include "sway/input/post_apollo_autoscroll_axis_dispatch.h"

#define DEVICE 7u
#define SEAT 3u
#define TARGET 91u
#define OUTPUT 21u
#define LAYOUT 8u
#define START 1000u

struct fake_axis_receiver {
    unsigned int axes;
    unsigned int frames;
    bool reject;
};

static bool fake_axis(void *opaque, const struct pa_scroll_axis_event *event) {
    struct fake_axis_receiver *receiver = opaque;
    assert(receiver != NULL && event != NULL);
    assert(event->target_node_id == TARGET);
    assert(event->discrete_steps == 0);
    if (receiver->reject) {
        return false;
    }
    receiver->axes++;
    return true;
}

static void fake_frame(void *opaque) {
    struct fake_axis_receiver *receiver = opaque;
    assert(receiver != NULL);
    receiver->frames++;
}

static struct pa_autoscroll_witness valid_witness(uint64_t when) {
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
    return witness;
}

static void test_no_evidence_is_authority(void) {
    struct pa_autoscroll_witness empty = {0};
    assert(!pa_autoscroll_witness_admissible(NULL, START));
    assert(!pa_autoscroll_witness_admissible(&empty, START));
    assert(!pa_autoscroll_witness_same_session(NULL, &empty, START));
    assert(!pa_autoscroll_witness_same_session(&empty, NULL, START));

    const struct pa_autoscroll_witness good = valid_witness(START);
    assert(pa_autoscroll_witness_admissible(&good, START));
    assert(pa_autoscroll_witness_admissible(
        &good, START + PA_AUTOSCROLL_WITNESS_MAX_AGE_MS));
    assert(!pa_autoscroll_witness_admissible(
        &good, START + PA_AUTOSCROLL_WITNESS_MAX_AGE_MS + 1));
    assert(!pa_autoscroll_witness_admissible(&good, START - 1));
    struct pa_autoscroll_witness changed = good;
    changed.observed_at_ms = 0;
    assert(!pa_autoscroll_witness_admissible(&changed, START));
}

#define REFUSE(field, value) do { \
    struct pa_autoscroll_witness changed = good; \
    changed.field = (value); \
    assert(!pa_autoscroll_witness_admissible(&changed, START)); \
} while (0)

static void test_provenance_lock_scene_and_ids_refuse(void) {
    const struct pa_autoscroll_witness good = valid_witness(START);
    REFUSE(origin, PA_POINTER_ORIGIN_UNKNOWN);
    REFUSE(origin, PA_POINTER_ORIGIN_VIRTUAL);
    REFUSE(compositor_event, false);
    REFUSE(surface_hit_verified, false);
    REFUSE(target_mapped, false);
    REFUSE(output_enabled, false);
    REFUSE(layout_generation_verified, false);
    REFUSE(session_locked, true);
    REFUSE(native_scroll_lock_active, true);
    REFUSE(stage_director_pairing_consumed, true);
    REFUSE(pointer_constrained, true);
    REFUSE(device_id, 0);
    REFUSE(seat_id, 0);
    REFUSE(target_node_id, 0);
    REFUSE(output_id, 0);
    REFUSE(layout_generation, 0);
}

#undef REFUSE

static void test_source_target_output_and_epoch_stay_paired(void) {
    const struct pa_autoscroll_witness start = valid_witness(START);
    struct pa_autoscroll_witness now = valid_witness(START + 5000);
    assert(pa_autoscroll_witness_same_session(&start, &now, START + 5000));

#define DIFFERENT(field, value) do { \
    struct pa_autoscroll_witness changed = now; \
    changed.field = (value); \
    assert(!pa_autoscroll_witness_same_session( \
        &start, &changed, START + 5000)); \
} while (0)

    DIFFERENT(device_id, DEVICE + 1);
    DIFFERENT(seat_id, SEAT + 1);
    DIFFERENT(target_node_id, TARGET + 1);
    DIFFERENT(output_id, OUTPUT + 1);
    DIFFERENT(layout_generation, LAYOUT + 1);
    DIFFERENT(origin, PA_POINTER_ORIGIN_VIRTUAL);
    DIFFERENT(native_scroll_lock_active, true);
    DIFFERENT(session_locked, true);
    DIFFERENT(stage_director_pairing_consumed, true);
    DIFFERENT(pointer_constrained, true);
    DIFFERENT(surface_hit_verified, false);
    DIFFERENT(target_mapped, false);
    DIFFERENT(output_enabled, false);
    DIFFERENT(layout_generation_verified, false);
    DIFFERENT(observed_at_ms, START - 1);
    DIFFERENT(observed_at_ms, START + 4900);
#undef DIFFERENT
}

static bool guarded_middle(struct pa_autoscroll_dispatch *dispatch,
    const struct pa_autoscroll_witness *witness, bool pressed) {
    if (!pa_autoscroll_witness_admissible(
            witness, witness == NULL ? 0 : witness->observed_at_ms)) {
        return false;
    }
    return pa_scroll_dispatch_middle(dispatch, pressed, true,
        witness->device_id, witness->seat_id, true,
        witness->target_node_id, witness->layout_generation, 300.0, 400.0,
        witness->observed_at_ms, false, false);
}

static bool guarded_tick(struct pa_autoscroll_dispatch *dispatch,
    const struct pa_autoscroll_witness *started,
    const struct pa_autoscroll_witness *current,
    struct fake_axis_receiver *receiver) {
    if (!pa_autoscroll_witness_same_session(
            started, current, current == NULL ? 0 : current->observed_at_ms)) {
        pa_scroll_dispatch_cancel(dispatch);
        return false;
    }
    return pa_scroll_dispatch_tick(dispatch, current->device_id,
        current->seat_id, true, current->target_node_id,
        current->layout_generation, current->observed_at_ms,
        false, false, fake_axis, fake_frame, receiver);
}

static void test_guarded_mock_axis_delivery_and_abort(void) {
    /* Configure is test-only; there is no production call to enable this. */
    struct pa_autoscroll_dispatch dispatch;
    struct fake_axis_receiver receiver = {0};
    struct pa_autoscroll_witness start = valid_witness(START);
    pa_scroll_dispatch_init(&dispatch);
    assert(!dispatch.adapter.enabled);
    pa_scroll_dispatch_enable(&dispatch, true);

    struct pa_autoscroll_witness virtual = start;
    virtual.origin = PA_POINTER_ORIGIN_VIRTUAL;
    assert(!guarded_middle(&dispatch, &virtual, true));
    assert(!dispatch.adapter.engine.active);

    assert(guarded_middle(&dispatch, &start, true));
    assert(guarded_middle(&dispatch, &start, false));
    assert(pa_scroll_dispatch_motion(&dispatch, true, DEVICE, SEAT,
        0.0, 170.0, false, false));

    struct pa_autoscroll_witness tick = valid_witness(START + 16);
    assert(guarded_tick(&dispatch, &start, &tick, &receiver));
    tick = valid_witness(START + 32);
    assert(guarded_tick(&dispatch, &start, &tick, &receiver));
    assert(receiver.axes == 2 && receiver.frames == 2);

    /* Simulated target turnover must cancel, not deliver another axis. */
    tick.target_node_id = TARGET + 1;
    assert(!guarded_tick(&dispatch, &start, &tick, &receiver));
    assert(!dispatch.adapter.engine.active);
    assert(receiver.axes == 2 && receiver.frames == 2);

    /* A newly admitted session still refuses when the output moves. */
    assert(guarded_middle(&dispatch, &start, true));
    assert(guarded_middle(&dispatch, &start, false));
    assert(pa_scroll_dispatch_motion(&dispatch, true, DEVICE, SEAT,
        180.0, 180.0, false, false));
    tick = valid_witness(START + 16);
    tick.layout_generation = LAYOUT + 1;
    assert(!guarded_tick(&dispatch, &start, &tick, &receiver));
    assert(receiver.axes == 2 && receiver.frames == 2);
    assert(!dispatch.adapter.engine.active);
}

static void test_no_frame_after_sink_rejection(void) {
    struct pa_autoscroll_dispatch dispatch;
    struct fake_axis_receiver receiver = {.reject = true};
    struct pa_autoscroll_witness start = valid_witness(START);
    pa_scroll_dispatch_init(&dispatch);
    pa_scroll_dispatch_enable(&dispatch, true);
    assert(guarded_middle(&dispatch, &start, true));
    assert(guarded_middle(&dispatch, &start, false));
    assert(pa_scroll_dispatch_motion(&dispatch, true, DEVICE, SEAT,
        0.0, 200.0, false, false));
    struct pa_autoscroll_witness tick = valid_witness(START + 16);
    assert(!guarded_tick(&dispatch, &start, &tick, &receiver));
    assert(!dispatch.adapter.engine.active);
    assert(receiver.axes == 0 && receiver.frames == 0);
}

int main(void) {
    test_no_evidence_is_authority();
    test_provenance_lock_scene_and_ids_refuse();
    test_source_target_output_and_epoch_stay_paired();
    test_guarded_mock_axis_delivery_and_abort();
    test_no_frame_after_sink_rejection();
    puts("SWAYPX_AUTOSCROLL_WITNESS_PASS // MOCK_ONLY_NO_WAYLAND_DELIVERY");
    return 0;
}
