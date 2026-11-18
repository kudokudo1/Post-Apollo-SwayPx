#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "sway/input/post_apollo_autoscroll_adapter.h"

#define DEVICE 7u
#define SEAT 3u
#define TARGET 91u
#define LAYOUT 5u
#define START 1000u

struct sink_data {
    int calls;
    double x, y;
};
static void mock_axis_sink(void *opaque, double x, double y) {
    struct sink_data *sink = opaque;
    assert(sink != NULL);
    sink->calls++;
    sink->x += x;
    sink->y += y;
}

static bool middle(struct pa_autoscroll_adapter *a, bool pressed) {
    return pa_autoscroll_adapter_middle(a, pressed, true, DEVICE, SEAT,
        true, TARGET, LAYOUT, -1920.0, 400.0, START, false, false);
}
static bool pulse(struct pa_autoscroll_adapter *a, uint64_t ms,
                  struct sink_data *sink) {
    return pa_autoscroll_adapter_tick(a, DEVICE, SEAT, true, TARGET, LAYOUT,
        ms, false, false, mock_axis_sink, sink);
}
static void configured(struct pa_autoscroll_adapter *a) {
    pa_autoscroll_adapter_init(a);
    pa_autoscroll_adapter_configure(a, true);
}

static void test_default_off_and_real_target_requirement(void) {
    struct pa_autoscroll_adapter a;
    pa_autoscroll_adapter_init(&a);
    assert(!a.enabled);
    assert(!middle(&a, true));
    assert(!a.engine.active);
    assert(!a.middle_release_pending);
    assert(!pa_autoscroll_adapter_motion(&a, true, DEVICE, SEAT, 0, 300,
                                        false, false));
    pa_autoscroll_adapter_configure(&a, true);
    assert(!pa_autoscroll_adapter_middle(&a, true, false, DEVICE, SEAT,
        true, TARGET, LAYOUT, 10, 20, START, false, false));
    assert(!pa_autoscroll_adapter_middle(&a, true, true, DEVICE, SEAT,
        false, TARGET, LAYOUT, 10, 20, START, false, false));
    assert(!pa_autoscroll_adapter_middle(&a, true, true, DEVICE, SEAT,
        true, 0, LAYOUT, 10, 20, START, false, false));
    assert(!pa_autoscroll_adapter_middle(&a, true, true, 0, SEAT,
        true, TARGET, LAYOUT, 10, 20, START, false, false));
    assert(!pa_autoscroll_adapter_middle(&a, true, true, DEVICE, 0,
        true, TARGET, LAYOUT, 10, 20, START, false, false));
    assert(!pa_autoscroll_adapter_middle(&a, true, true, DEVICE, SEAT,
        true, TARGET, LAYOUT, 10, 20, START, true, false));
    assert(!pa_autoscroll_adapter_middle(&a, true, true, DEVICE, SEAT,
        true, TARGET, LAYOUT, 10, 20, START, false, true));
    assert(!a.engine.active);
}

static void test_middle_click_toggle_release_pairing(void) {
    struct pa_autoscroll_adapter a;
    configured(&a);
    assert(middle(&a, true));
    assert(a.engine.active && a.middle_release_pending);
    assert(middle(&a, true)); /* duplicate down is swallowed */
    assert(a.engine.active);
    assert(!pa_autoscroll_adapter_middle(&a, false, true, 19, SEAT,
        true, TARGET, LAYOUT, 0, 0, 1001, false, false));
    assert(middle(&a, false));
    assert(!a.middle_release_pending);
    assert(middle(&a, true)); /* second click cancels */
    assert(!a.engine.active);
    assert(middle(&a, false));
    assert(!a.middle_release_pending);
    pa_autoscroll_adapter_configure(&a, false);
    assert(!middle(&a, true));
    assert(!middle(&a, false));
}

static void test_stationary_motion_drives_repeated_wheel_intents(void) {
    struct pa_autoscroll_adapter a;
    struct sink_data sink = {0};
    configured(&a);
    assert(middle(&a, true));
    assert(middle(&a, false));
    assert(pa_autoscroll_adapter_motion(&a, true, DEVICE, SEAT, 0, 160,
                                       false, false));
    assert(pulse(&a, 1016, &sink));
    assert(pulse(&a, 1032, &sink));
    assert(sink.calls == 2);
    assert(sink.x == 0 && sink.y > 0);
    double down = sink.y;
    assert(pa_autoscroll_adapter_motion(&a, true, DEVICE, SEAT, 0, -320,
                                       false, false));
    assert(pulse(&a, 1048, &sink));
    assert(sink.calls == 3);
    assert(sink.y < down);
    pa_autoscroll_adapter_cancel(&a);
    assert(!pulse(&a, 1064, &sink));
    assert(sink.calls == 3);
}

static void test_other_button_consumes_press_and_release(void) {
    struct pa_autoscroll_adapter a;
    configured(&a);
    assert(middle(&a, true));
    assert(middle(&a, false));
    assert(pa_autoscroll_adapter_other_button(&a, PA_AUTOSCROLL_BTN_LEFT,
                                               true, true, DEVICE, SEAT));
    assert(!a.engine.active && a.other_release_pending);
    assert(pa_autoscroll_adapter_other_button(&a, PA_AUTOSCROLL_BTN_LEFT,
                                               true, true, DEVICE, SEAT));
    assert(!pa_autoscroll_adapter_other_button(&a, PA_AUTOSCROLL_BTN_RIGHT,
                                                false, true, DEVICE, SEAT));
    assert(pa_autoscroll_adapter_other_button(&a, PA_AUTOSCROLL_BTN_LEFT,
                                               false, true, DEVICE, SEAT));
    assert(!a.other_release_pending);
    assert(!pa_autoscroll_adapter_other_button(&a, PA_AUTOSCROLL_BTN_LEFT,
                                                true, true, DEVICE, SEAT));
}

static void test_escape_and_disable_do_not_leak_button_releases(void) {
    struct pa_autoscroll_adapter a;
    configured(&a);
    assert(middle(&a, true));
    assert(!pa_autoscroll_adapter_escape(&a, true, 99));
    assert(!pa_autoscroll_adapter_escape(&a, false, SEAT));
    assert(pa_autoscroll_adapter_escape(&a, true, SEAT));
    assert(!a.engine.active);
    assert(middle(&a, false)); /* press consumed before Escape */
    assert(middle(&a, true));
    pa_autoscroll_adapter_configure(&a, false);
    assert(!a.engine.active);
    assert(middle(&a, false)); /* paired release still consumed */
    assert(!middle(&a, true));
    assert(!pa_autoscroll_adapter_escape(&a, true, SEAT));
}

static void test_wrong_device_seat_layout_or_target_fails_closed(void) {
    struct pa_autoscroll_adapter a;
    struct sink_data sink = {0};
    configured(&a);
    assert(middle(&a, true));
    assert(middle(&a, false));
    assert(!pa_autoscroll_adapter_motion(&a, true, 999, SEAT, 0, 100,
                                         false, false));
    assert(!a.engine.active);
    assert(middle(&a, true));
    assert(middle(&a, false));
    assert(pa_autoscroll_adapter_motion(&a, true, DEVICE, SEAT, 0, 120,
                                       false, false));
    assert(!pa_autoscroll_adapter_tick(&a, DEVICE, SEAT, true, TARGET + 1,
                LAYOUT, 1016, false, false, mock_axis_sink, &sink));
    assert(!a.engine.active && sink.calls == 0);
    assert(middle(&a, true));
    assert(middle(&a, false));
    assert(!pa_autoscroll_adapter_tick(&a, DEVICE, SEAT, true, TARGET,
                LAYOUT + 1, 1016, false, false, mock_axis_sink, &sink));
    assert(!a.engine.active);
    assert(middle(&a, true));
    assert(middle(&a, false));
    assert(!pa_autoscroll_adapter_tick(&a, DEVICE, SEAT, false, TARGET,
                LAYOUT, 1016, false, false, mock_axis_sink, &sink));
    assert(!a.engine.active);
}

static void test_null_sink_lock_and_focus_refuse_scroll(void) {
    struct pa_autoscroll_adapter a;
    configured(&a);
    assert(middle(&a, true));
    assert(middle(&a, false));
    assert(!pa_autoscroll_adapter_tick(&a, DEVICE, SEAT, true, TARGET,
                                       LAYOUT, 1016, false, false, NULL, NULL));
    assert(!a.engine.active);
    assert(middle(&a, true));
    assert(middle(&a, false));
    assert(!pa_autoscroll_adapter_motion(&a, true, DEVICE, SEAT, 0, 100,
                                         false, true));
    assert(!a.engine.active);
    assert(middle(&a, true));
    assert(middle(&a, false));
    struct sink_data sink = {0};
    assert(!pa_autoscroll_adapter_tick(&a, DEVICE, SEAT, true, TARGET,
                LAYOUT, 1016, true, false, mock_axis_sink, &sink));
    assert(!a.engine.active);
    assert(sink.calls == 0);
}

static void test_secondary_or_virtual_cannot_steal_session(void) {
    struct pa_autoscroll_adapter a;
    configured(&a);
    assert(middle(&a, true));
    assert(middle(&a, false));
    assert(!pa_autoscroll_adapter_middle(&a, true, true, 333, SEAT,
        true, TARGET, LAYOUT, 0, 0, 1016, false, false));
    assert(!a.engine.active);
    assert(middle(&a, true));
    assert(middle(&a, false));
    assert(!pa_autoscroll_adapter_middle(&a, true, false, DEVICE, SEAT,
        true, TARGET, LAYOUT, 0, 0, 1016, false, false));
    assert(!a.engine.active);
}

int main(void) {
    test_default_off_and_real_target_requirement();
    test_middle_click_toggle_release_pairing();
    test_stationary_motion_drives_repeated_wheel_intents();
    test_other_button_consumes_press_and_release();
    test_escape_and_disable_do_not_leak_button_releases();
    test_wrong_device_seat_layout_or_target_fails_closed();
    test_null_sink_lock_and_focus_refuse_scroll();
    test_secondary_or_virtual_cannot_steal_session();
    puts("SWAYPX_AUTOSCROLL_ADAPTER_PASS // NO_ACTIVE_COMPOSITOR_INPUT");
    return 0;
}
