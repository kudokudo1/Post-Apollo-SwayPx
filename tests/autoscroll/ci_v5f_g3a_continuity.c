/*
 * G3A: detached C continuity regression; deliberately test-only facts.
 * All-positive witness fixtures are UNTRUSTED mock data, NOT real hardware.
 * The only native live evidence remains previous G2 negative tests.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "sway/input/post_apollo_autoscroll_continuity.h"

#define CHECK(condition, label) do { \
    if (!(condition)) { \
        fprintf(stderr, "PA_G3A_CHECK_FAILED %s line=%d\n", label, __LINE__); \
        exit(1); \
    } \
} while (0)

static struct pa_autoscroll_witness fake_start(void) {
    return (struct pa_autoscroll_witness) {
        .origin = PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT,
        .compositor_event = true,
        .surface_hit_verified = true,
        .target_mapped = true,
        .output_enabled = true,
        .layout_generation_verified = true,
        .session_locked = false,
        .native_scroll_lock_active = false,
        .stage_director_pairing_consumed = false,
        .pointer_constrained = false,
        .device_id = 11,
        .seat_id = 12,
        .target_node_id = 13,
        .output_id = 14,
        .layout_generation = 15,
        .observed_at_ms = 1000,
    };
}
static struct pa_autoscroll_object_epochs fake_owners(void) {
    return (struct pa_autoscroll_object_epochs) {
        .device = 101, .seat = 102, .target = 103, .output = 104,
    };
}
static struct pa_autoscroll_continuity_scene fake_scene(void) {
    return (struct pa_autoscroll_continuity_scene) {
        .owners = fake_owners(),
        .device_id = 11, .seat_id = 12,
        .target_node_id = 13, .output_id = 14,
        .layout_generation = 15,
        .captured_at_ms = 1017,
        .surface_hit_verified = true,
        .target_mapped = true,
        .output_enabled = true,
        .layout_generation_verified = true,
        .session_locked = false,
        .native_scroll_lock_active = false,
        .stage_director_pairing_consumed = false,
        .pointer_constrained = false,
    };
}
static struct pa_autoscroll_continuity_guard new_guard(void) {
    struct pa_autoscroll_continuity_guard guard = {0};
    struct pa_autoscroll_witness start = fake_start();
    struct pa_autoscroll_object_epochs owners = fake_owners();
    CHECK(pa_autoscroll_continuity_begin(&guard, &start, &owners, 1001),
        "synthetic_start_data_is_not_hardware_attestation");
    CHECK(guard.active, "test_guard_active");
    return guard;
}

static void preserve_stationary_scrolling_without_event_forgeries(void) {
    struct pa_autoscroll_continuity_guard guard = new_guard();
    struct pa_autoscroll_continuity_scene scene = fake_scene();
    for (uint64_t i = 1; i <= 1000; i++) {
        const uint64_t tick = 1001 + 16 * i;
        /* Only scene snapshots advance. The real initial input timestamp
         * stays 1000 for every tick. No fake input event is constructed.
         */
        scene.captured_at_ms = tick;
        CHECK(pa_autoscroll_continuity_tick(&guard, &scene, tick),
            "stationary_tick_valid");
        CHECK(guard.start.started.observed_at_ms == 1000,
            "start_event_timestamp_unchanged");
        CHECK(guard.start.last_observed_at_ms == 1000,
            "no_new_physical_input_claim");
    }
    CHECK(guard.active, "stationary_continues");
    CHECK(guard.last_tick_ms == 17001, "monotonic_tick_16_seconds");
    printf("PA_G3A_STATIONARY_SCENE_CONTINUITY_PASS ticks=1000 "
           "elapsed_ms=16000 input_events=ONE_NO_FORGERY\n");

    struct pa_autoscroll_epoch_guard legacy = {0};
    struct pa_autoscroll_witness start = fake_start();
    struct pa_autoscroll_object_epochs owners = fake_owners();
    CHECK(pa_autoscroll_epoch_begin(&legacy, &start, &owners, 1001),
        "actual_legacy_begin");
    CHECK(!pa_autoscroll_epoch_tick(&legacy, &start, &owners, 1017),
        "actual_legacy_tick_rejects_stationary_replay");
    CHECK(!legacy.active, "actual_legacy_replay_cancels");
    printf("PA_G3A_ACTUAL_V5C_OLD_TICK_STATIONARY_CONFLICT_CONFIRMED_PASS\n");
}

#define REJECT(label, mutate) do { \
    struct pa_autoscroll_continuity_guard g = new_guard(); \
    struct pa_autoscroll_continuity_scene s = fake_scene(); \
    mutate; \
    CHECK(!pa_autoscroll_continuity_tick(&g, &s, 1017), label); \
    CHECK(!g.active && !g.start.active, "refusal_must_cancel"); \
    CHECK(!pa_autoscroll_continuity_tick(&g, &s, 1033), \
        "no_reactivation_without_new_begin"); \
} while (0)

static void every_native_owner_or_scene_drift_cancels(void) {
    REJECT("device_epoch", s.owners.device++);
    REJECT("seat_epoch", s.owners.seat++);
    REJECT("target_epoch", s.owners.target++);
    REJECT("output_epoch", s.owners.output++);
    REJECT("zero_device_epoch", s.owners.device = 0);
    REJECT("zero_output_epoch", s.owners.output = 0);
    REJECT("device_id", s.device_id++);
    REJECT("seat_id", s.seat_id++);
    REJECT("target_id", s.target_node_id++);
    REJECT("output_id", s.output_id++);
    REJECT("layout_rotation", s.layout_generation++);
    REJECT("session_lock", s.session_locked = true);
    REJECT("native_scroll_lock", s.native_scroll_lock_active = true);
    REJECT("weather_or_company_consumption", s.stage_director_pairing_consumed = true);
    REJECT("pointer_constraint", s.pointer_constrained = true);
    REJECT("target_unmapped", s.target_mapped = false);
    REJECT("output_disabled", s.output_enabled = false);
    REJECT("hit_test_not_verified", s.surface_hit_verified = false);
    REJECT("layout_unverified", s.layout_generation_verified = false);
    REJECT("stale_scene", s.captured_at_ms = 900);
    REJECT("future_scene", s.captured_at_ms = 1018);
    REJECT("scene_without_capture", s.captured_at_ms = 0);
    printf("PA_G3A_OWNER_SCENE_FAIL_CLOSED_MATRIX_PASS cases=22\n");
}

static void time_and_rearm_fail_closed(void) {
    struct pa_autoscroll_continuity_guard g = new_guard();
    struct pa_autoscroll_continuity_scene scene = fake_scene();
    CHECK(!pa_autoscroll_continuity_tick(&g, &scene, 1001),
        "duplicate_timer_instant");
    CHECK(!g.active, "duplicate_timer_cancel");
    g = new_guard();
    scene.captured_at_ms = 1300;
    CHECK(!pa_autoscroll_continuity_tick(&g, &scene, 1300),
        "long_schedule_gap_no_catchup_burst");
    CHECK(!g.active, "long_gap_cancel");
    g = new_guard();
    CHECK(!pa_autoscroll_continuity_tick(&g, NULL, 1017),
        "no_real_scene_sample");
    CHECK(!g.active, "missing_scene_cancel");
    g = new_guard();
    pa_autoscroll_continuity_cancel(&g);
    CHECK(!g.active && !g.start.active && g.last_tick_ms == 0,
        "explicit_cancel_zeroized");

    struct pa_autoscroll_witness start = fake_start();
    struct pa_autoscroll_object_epochs owners = fake_owners();
    start.origin = PA_POINTER_ORIGIN_VIRTUAL;
    CHECK(!pa_autoscroll_continuity_begin(&g, &start, &owners, 1001),
        "virtual_start_denied");
    start.origin = PA_POINTER_ORIGIN_UNKNOWN;
    CHECK(!pa_autoscroll_continuity_begin(&g, &start, &owners, 1001),
        "unknown_start_denied");
    start = fake_start();
    start.observed_at_ms = 2000;
    CHECK(!pa_autoscroll_continuity_begin(&g, &start, &owners, 1001),
        "future_start_denied");
    start = fake_start();
    start.stage_director_pairing_consumed = true;
    CHECK(!pa_autoscroll_continuity_begin(&g, &start, &owners, 1001),
        "prior_first_refusal_denied");
    start = fake_start();
    CHECK(pa_autoscroll_continuity_begin(&g, &start, &owners, 1001),
        "new_start_only_after_rearm");
    CHECK(g.active, "test_only_rearm");
    printf("PA_G3A_TIMER_GAP_REARM_AND_ORIGIN_DENIAL_PASS\n");
}

int main(void) {
    preserve_stationary_scrolling_without_event_forgeries();
    every_native_owner_or_scene_drift_cancels();
    time_and_rearm_fail_closed();
    puts("PA_G3A_DETACHED_POLICY_ONLY_PASS "
         "native_producer=ABSENT production_axis_frames=ZERO");
    return 0;
}
