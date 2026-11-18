#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sway/input/post_apollo_autoscroll_axis_dispatch.h"

#define DEV 7u
#define SEAT 3u
#define TARGET 91u
#define LAYOUT 8u
#define START 1000u

struct fake_output {
    struct pa_scroll_axis_event events[32];
    size_t event_count;
    unsigned int frames;
    bool reject;
    size_t reject_after;
};

static bool record_axis(void *opaque, const struct pa_scroll_axis_event *event) {
    struct fake_output *out = opaque;
    assert(event != NULL);
    assert(event->discrete_steps == 0);
    assert(event->target_node_id == TARGET);
    assert(event->axis == PA_SCROLL_AXIS_VERTICAL ||
           event->axis == PA_SCROLL_AXIS_HORIZONTAL);
    assert(isfinite(event->delta_pixels));
    assert(event->delta_pixels != 0);
    if (out->reject && out->event_count >= out->reject_after) {
        return false;
    }
    assert(out->event_count < 32);
    out->events[out->event_count++] = *event;
    return true;
}
static void record_frame(void *opaque) {
    struct fake_output *out = opaque;
    out->frames++;
}
static bool click(struct pa_autoscroll_dispatch *d, bool pressed) {
    return pa_scroll_dispatch_middle(d, pressed, true, DEV, SEAT, true,
             TARGET, LAYOUT, -1820.0, 300.0, START, false, false);
}
static bool tick(struct pa_autoscroll_dispatch *d, uint64_t ms,
                 struct fake_output *out) {
    return pa_scroll_dispatch_tick(d, DEV, SEAT, true, TARGET, LAYOUT, ms,
                 false, false, record_axis, record_frame, out);
}
static void start(struct pa_autoscroll_dispatch *d) {
    pa_scroll_dispatch_init(d);
    pa_scroll_dispatch_enable(d, true);
    assert(click(d, true));
    assert(click(d, false));
}

static void test_default_disabled_preserves_middle_and_wheel(void) {
    struct pa_autoscroll_dispatch d;
    struct fake_output out = {0};
    pa_scroll_dispatch_init(&d);
    assert(!d.adapter.enabled);
    assert(!click(&d, true));
    assert(!click(&d, false));
    assert(!pa_scroll_dispatch_motion(&d, true, DEV, SEAT, 0, 100, false, false));
    assert(!tick(&d, START + 16, &out));
    assert(out.event_count == 0 && out.frames == 0);
}

static void test_middle_to_continuous_vertical_stream_and_stop(void) {
    struct pa_autoscroll_dispatch d;
    struct fake_output out = {0};
    start(&d);
    assert(pa_scroll_dispatch_motion(&d, true, DEV, SEAT, 0, 180,
                                     false, false));
    assert(tick(&d, START + 16, &out));
    assert(tick(&d, START + 32, &out));
    assert(out.event_count == 2);
    assert(out.frames == 2 && d.frames == 2);
    assert(out.events[0].axis == PA_SCROLL_AXIS_VERTICAL);
    assert(out.events[1].axis == PA_SCROLL_AXIS_VERTICAL);
    assert(out.events[0].delta_pixels > 0);
    assert(fabs(out.events[1].delta_pixels - out.events[0].delta_pixels) < 1e-9);
    assert(click(&d, true));
    assert(!d.adapter.engine.active);
    assert(click(&d, false));
    assert(!tick(&d, START + 48, &out));
    assert(out.event_count == 2 && out.frames == 2);
}

static void test_diagonal_axes_share_one_frame_and_reverse(void) {
    struct pa_autoscroll_dispatch d;
    struct fake_output out = {0};
    start(&d);
    assert(pa_scroll_dispatch_motion(&d, true, DEV, SEAT, 180, -170,
                                     false, false));
    assert(tick(&d, START + 16, &out));
    assert(out.event_count == 2 && out.frames == 1);
    assert(out.events[0].axis == PA_SCROLL_AXIS_VERTICAL);
    assert(out.events[0].delta_pixels < 0);
    assert(out.events[1].axis == PA_SCROLL_AXIS_HORIZONTAL);
    assert(out.events[1].delta_pixels > 0);
    assert(pa_scroll_dispatch_motion(&d, true, DEV, SEAT, -360, 340,
                                     false, false));
    assert(tick(&d, START + 32, &out));
    assert(out.event_count == 4 && out.frames == 2);
    assert(out.events[2].delta_pixels > 0);
    assert(out.events[3].delta_pixels < 0);
}

static void test_deadzone_emits_neither_axis_nor_frame(void) {
    struct pa_autoscroll_dispatch d;
    struct fake_output out = {0};
    start(&d);
    assert(pa_scroll_dispatch_motion(&d, true, DEV, SEAT, 10, -10,
                                     false, false));
    assert(tick(&d, START + 16, &out));
    assert(tick(&d, START + 32, &out));
    assert(d.adapter.engine.active);
    assert(out.event_count == 0 && out.frames == 0 && d.frames == 0);
}

static void test_sink_refusal_stops_no_frame(void) {
    struct pa_autoscroll_dispatch d;
    struct fake_output out = {.reject = true, .reject_after = 0};
    start(&d);
    assert(pa_scroll_dispatch_motion(&d, true, DEV, SEAT, 180, 180,
                                     false, false));
    assert(!tick(&d, START + 16, &out));
    assert(!d.adapter.engine.active);
    assert(out.event_count == 0 && out.frames == 0);
    assert(!tick(&d, START + 32, &out));
    /* A partial horizontal rejection must never invent a completed frame. */
    struct pa_autoscroll_dispatch second;
    struct fake_output partial = {.reject = true, .reject_after = 1};
    start(&second);
    assert(pa_scroll_dispatch_motion(&second, true, DEV, SEAT, 180, 180,
                                     false, false));
    assert(!tick(&second, START + 16, &partial));
    assert(partial.event_count == 1 && partial.frames == 0);
    assert(!second.adapter.engine.active);
}

static void test_missing_sink_fails_closed(void) {
    struct pa_autoscroll_dispatch d;
    struct fake_output out = {0};
    start(&d);
    assert(pa_scroll_dispatch_motion(&d, true, DEV, SEAT, 0, 180,
                                     false, false));
    assert(!pa_scroll_dispatch_tick(&d, DEV, SEAT, true, TARGET, LAYOUT,
              START + 16, false, false, NULL, record_frame, &out));
    assert(!d.adapter.engine.active);
    assert(out.frames == 0);
    start(&d);
    assert(!pa_scroll_dispatch_tick(&d, DEV, SEAT, true, TARGET, LAYOUT,
              START + 16, false, false, record_axis, NULL, &out));
    assert(!d.adapter.engine.active);
    assert(out.event_count == 0);
}

static void test_wrong_target_layout_and_device_no_delivery(void) {
    struct pa_autoscroll_dispatch d;
    struct fake_output out = {0};
    start(&d);
    assert(pa_scroll_dispatch_motion(&d, true, DEV, SEAT, 0, 200,
                                     false, false));
    assert(!pa_scroll_dispatch_tick(&d, DEV, SEAT, true, TARGET + 1,
               LAYOUT, START + 16, false, false,
               record_axis, record_frame, &out));
    assert(!d.adapter.engine.active && out.event_count == 0);
    start(&d);
    assert(!pa_scroll_dispatch_tick(&d, DEV, SEAT, true, TARGET,
               LAYOUT + 1, START + 16, false, false,
               record_axis, record_frame, &out));
    assert(!d.adapter.engine.active);
    start(&d);
    assert(!pa_scroll_dispatch_tick(&d, 999, SEAT, true, TARGET,
               LAYOUT, START + 16, false, false,
               record_axis, record_frame, &out));
    assert(!d.adapter.engine.active);
    assert(out.frames == 0);
}

static void test_escape_other_click_session_lock_and_scroll_lock(void) {
    struct pa_autoscroll_dispatch d;
    struct fake_output out = {0};
    start(&d);
    assert(!pa_scroll_dispatch_escape(&d, true, 999));
    assert(pa_scroll_dispatch_escape(&d, true, SEAT));
    assert(!tick(&d, START + 16, &out));
    start(&d);
    assert(pa_scroll_dispatch_other_button(&d, PA_AUTOSCROLL_BTN_RIGHT,
               true, true, DEV, SEAT));
    assert(pa_scroll_dispatch_other_button(&d, PA_AUTOSCROLL_BTN_RIGHT,
               false, true, DEV, SEAT));
    assert(!tick(&d, START + 16, &out));
    start(&d);
    assert(!pa_scroll_dispatch_tick(&d, DEV, SEAT, true, TARGET,
               LAYOUT, START + 16, true, false,
               record_axis, record_frame, &out));
    assert(!d.adapter.engine.active);
    start(&d);
    assert(!pa_scroll_dispatch_tick(&d, DEV, SEAT, true, TARGET,
               LAYOUT, START + 16, false, true,
               record_axis, record_frame, &out));
    assert(!d.adapter.engine.active && out.frames == 0);
}

int main(void) {
    test_default_disabled_preserves_middle_and_wheel();
    test_middle_to_continuous_vertical_stream_and_stop();
    test_diagonal_axes_share_one_frame_and_reverse();
    test_deadzone_emits_neither_axis_nor_frame();
    test_sink_refusal_stops_no_frame();
    test_missing_sink_fails_closed();
    test_wrong_target_layout_and_device_no_delivery();
    test_escape_other_click_session_lock_and_scroll_lock();
    puts("SWAYPX_AUTOSCROLL_AXIS_DISPATCH_PASS // SOURCE_ONLY");
    return 0;
}
