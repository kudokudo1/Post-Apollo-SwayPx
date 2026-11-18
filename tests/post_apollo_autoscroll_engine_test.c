#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "sway/input/post_apollo_autoscroll.h"

static void assert_zero(struct pa_autoscroll_delta delta) {
    assert(!delta.active);
    assert(delta.horizontal_pixels == 0);
    assert(delta.vertical_pixels == 0);
}

static void test_approval_and_target_guard(void) {
    struct pa_autoscroll_state s;
    pa_autoscroll_init(&s);
    assert_zero(pa_autoscroll_tick(&s, 4, 8, 1000));
    assert(!pa_autoscroll_begin(&s, false, true, 4, 8, 10, 20, 1000));
    assert(!pa_autoscroll_begin(&s, true, false, 4, 8, 10, 20, 1000));
    assert(!pa_autoscroll_begin(&s, true, true, 0, 8, 10, 20, 1000));
    assert(!pa_autoscroll_begin(&s, true, true, 4, 0, 10, 20, 1000));
    assert(!pa_autoscroll_begin(&s, true, true, 4, 8, NAN, 20, 1000));
    assert(!pa_autoscroll_begin(&s, true, true, 4, 8, 10, INFINITY, 1000));
    assert(!pa_autoscroll_begin(&s, true, true, 4, 8, 10, 20, 0));
    assert(!s.active);
    assert(pa_autoscroll_begin(&s, true, true, 4, 8, 10, 20, 1000));
    assert(s.active);
    assert(s.generation == 1);
    assert(!pa_autoscroll_begin(&s, true, true, 4, 8, 10, 20, 1001));
    assert(s.target_node_id == 4 && s.layout_generation == 8);
    assert(s.anchor_x == 10 && s.anchor_y == 20);
    pa_autoscroll_cancel(&s);
    assert(!s.active && s.target_node_id == 0);
    assert(s.generation == 2);
}

static void test_sustained_stationary_scrolling_and_reversal(void) {
    struct pa_autoscroll_state s;
    pa_autoscroll_init(&s);
    assert(pa_autoscroll_begin(&s, true, true, 42, 9, -1800, 300, 1000));
    pa_autoscroll_move(&s, 0, 180);
    struct pa_autoscroll_delta a = pa_autoscroll_tick(&s, 42, 9, 1016);
    struct pa_autoscroll_delta b = pa_autoscroll_tick(&s, 42, 9, 1032);
    assert(a.active && b.active);
    assert(a.vertical_pixels > 0 && b.vertical_pixels > 0);
    assert(fabs(a.vertical_pixels - b.vertical_pixels) < 1e-8);
    assert(a.horizontal_pixels == 0);
    pa_autoscroll_move(&s, 0, -360);
    struct pa_autoscroll_delta c = pa_autoscroll_tick(&s, 42, 9, 1048);
    assert(c.active && c.vertical_pixels < 0);
    pa_autoscroll_move(&s, 320, 0);
    struct pa_autoscroll_delta d = pa_autoscroll_tick(&s, 42, 9, 1064);
    assert(d.active && d.horizontal_pixels > 0);
    pa_autoscroll_move(&s, INFINITY, 0);
    assert_zero(pa_autoscroll_tick(&s, 42, 9, 1080));
    assert(!s.active);
}

static void test_deadzone_speed_and_bounds(void) {
    struct pa_autoscroll_state s;
    pa_autoscroll_init(&s);
    assert(pa_autoscroll_begin(&s, true, true, 42, 9, 0, 0, 1000));
    pa_autoscroll_move(&s, 15, -15);
    struct pa_autoscroll_delta z = pa_autoscroll_tick(&s, 42, 9, 1016);
    assert(z.active && z.horizontal_pixels == 0 && z.vertical_pixels == 0);
    pa_autoscroll_move(&s, 2000, 2000);
    assert(s.displacement_x == 1200 && s.displacement_y == 1200);
    struct pa_autoscroll_delta m = pa_autoscroll_tick(&s, 42, 9, 1032);
    assert(m.active && m.vertical_pixels > 0 && m.horizontal_pixels > 0);
    assert(m.vertical_pixels <= 2800.0 * 0.016 + 1e-10);
    assert(m.horizontal_pixels <= 2800.0 * 0.016 + 1e-10);
}

static void test_focus_layout_and_time_fail_closed(void) {
    struct pa_autoscroll_state s;
    pa_autoscroll_init(&s);
    assert(pa_autoscroll_begin(&s, true, true, 44, 3, 0, 0, 1000));
    pa_autoscroll_move(&s, 0, 200);
    assert_zero(pa_autoscroll_tick(&s, 45, 3, 1016));
    assert(!s.active);

    assert(pa_autoscroll_begin(&s, true, true, 44, 3, 0, 0, 2000));
    pa_autoscroll_move(&s, 0, 200);
    assert_zero(pa_autoscroll_tick(&s, 44, 4, 2016));
    assert(!s.active);

    assert(pa_autoscroll_begin(&s, true, true, 44, 3, 0, 0, 3000));
    pa_autoscroll_move(&s, 0, 200);
    assert_zero(pa_autoscroll_tick(&s, 44, 3, 3101));
    assert(!s.active);

    assert(pa_autoscroll_begin(&s, true, true, 44, 3, 0, 0, 4000));
    pa_autoscroll_move(&s, 0, 200);
    assert_zero(pa_autoscroll_tick(&s, 44, 3, 4000));
    assert(!s.active);

    assert(pa_autoscroll_begin(&s, true, true, 44, 3, 0, 0, 5000));
    pa_autoscroll_move(&s, 0, 200);
    assert_zero(pa_autoscroll_tick(&s, 44, 3, 185001));
    assert(!s.active);
}

static void test_explicit_cancel_is_idempotent(void) {
    struct pa_autoscroll_state s;
    pa_autoscroll_init(&s);
    assert(pa_autoscroll_begin(&s, true, true, 13, 5, 5, 5, 50));
    pa_autoscroll_cancel(&s);
    assert(!s.active);
    assert_zero(pa_autoscroll_tick(&s, 13, 5, 66));
    pa_autoscroll_cancel(NULL);
    pa_autoscroll_move(NULL, 1, 1);
    assert_zero(pa_autoscroll_tick(NULL, 13, 5, 66));
}

int main(void) {
    test_approval_and_target_guard();
    test_sustained_stationary_scrolling_and_reversal();
    test_deadzone_speed_and_bounds();
    test_focus_layout_and_time_fail_closed();
    test_explicit_cancel_is_idempotent();
    puts("SWAYPX_AUTOSCROLL_CORE_PASS // NO_COMPOSITOR_HOOK");
    return 0;
}
