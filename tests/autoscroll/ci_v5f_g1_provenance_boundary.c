/*
 * V5F-G1: adversarial provenance boundary for real, pinned C policy code.
 *
 * These unit vectors are SYNTHETIC DATA, not input hardware. This program
 * must never call compositor, Wayland IPC or native scroll actuation.
 * Deliberately demonstrates that the portable guard trusts its C caller:
 * a forged all-positive witness can be accepted by the detached policy,
 * whereas the native bridge currently NEVER supplies that forged witness.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sway/input/post_apollo_autoscroll_origin_policy.h"
#include "sway/input/post_apollo_autoscroll_topology.h"
#include "sway/input/post_apollo_autoscroll_epoch.h"

#define CHECK(cond, tag) do { \
    if (!(cond)) { \
        fprintf(stderr, "PA_G1_FAILED %s at line %d\n", tag, __LINE__); \
        exit(1); \
    } \
} while (0)

static struct pa_autoscroll_witness specimen(void) {
    return (struct pa_autoscroll_witness) {
        .origin = PA_POINTER_ORIGIN_UNKNOWN,
        .compositor_event = true,
        .surface_hit_verified = true,
        .target_mapped = true,
        .output_enabled = true,
        .layout_generation_verified = true,
        .session_locked = false,
        .native_scroll_lock_active = false,
        .stage_director_pairing_consumed = false,
        .pointer_constrained = false,
        .device_id = 41,
        .seat_id = 42,
        .target_node_id = 43,
        .output_id = 44,
        .layout_generation = 45,
        .observed_at_ms = 1000,
    };
}

static struct pa_autoscroll_object_epochs issued(void) {
    return (struct pa_autoscroll_object_epochs) {
        .device = 91, .seat = 92, .target = 93, .output = 94,
    };
}

static void candidate_is_not_hardware(void) {
    /* This bus-looking topology may also be software-produced HID.
     * Passing this NEGATIVE FILTER is categorically not an attestation. */
    const char *syspath = "/sys/devices/pci0000:00/0000:00:14.0/"
        "usb1/1-2/1-2:1.0/0003:046D:C077.0001/input/input12/event34";
    CHECK(pa_autoscroll_kernel_tree_candidate(
            syspath, "/dev/input/event34"), "bus_looking_candidate");
    CHECK(!pa_autoscroll_kernel_tree_candidate(
            "/sys/devices/virtual/input/input12/event34",
            "/dev/input/event34"), "virtual_tree_denied");
    CHECK(!pa_autoscroll_kernel_tree_candidate(
            "/sys/devices/pci0000:00/input/input12/event35",
            "/dev/input/event34"), "mismatched_event_node_denied");
    struct pa_autoscroll_origin_facts facts = {
        .pointer_device = true,
        .libinput_backend = true,
        .libinput_handle_present = true,
        .native_scroll_method_known = true,
        .native_scroll_on_button_down = false,
    };
    CHECK(pa_autoscroll_origin_classify(&facts)
        == PA_AUTOSCROLL_ORIGIN_LIBINPUT_CANDIDATE, "candidate_only");
    facts.native_scroll_on_button_down = true;
    CHECK(pa_autoscroll_origin_classify(&facts)
        == PA_AUTOSCROLL_ORIGIN_DENIED, "button_scroll_refusal");
    facts.native_scroll_on_button_down = false;
    facts.native_scroll_method_known = false;
    CHECK(pa_autoscroll_origin_classify(&facts)
        == PA_AUTOSCROLL_ORIGIN_DENIED, "unknown_scroll_mode_refusal");
    printf("PA_G1_REAL_C_ORIGIN_CANDIDATE_NOT_CREDENTIAL_PASS\n");
}

static void negative_evidence_refused(void) {
    struct pa_autoscroll_epoch_guard guard = {0};
    struct pa_autoscroll_object_epochs epochs = issued();
    struct pa_autoscroll_witness w = specimen();
    CHECK(!pa_autoscroll_epoch_begin(&guard, &w, &epochs, 1001),
        "unknown_origin");
    CHECK(!guard.active, "unknown_origin_not_armed");
    w.origin = PA_POINTER_ORIGIN_VIRTUAL;
    CHECK(!pa_autoscroll_epoch_begin(&guard, &w, &epochs, 1001),
        "virtual_origin");

    /* The rest are independent invalidations even if a test forges
     * the privileged physical-origin enum. This is NOT a real attestation. */
    w.origin = PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT;
    w.native_scroll_lock_active = true;
    CHECK(!pa_autoscroll_epoch_begin(&guard, &w, &epochs, 1001),
        "native_middle_button_scroll_active");
    w.native_scroll_lock_active = false;
    w.layout_generation_verified = false;
    CHECK(!pa_autoscroll_epoch_begin(&guard, &w, &epochs, 1001),
        "unverified_layout");
    w.layout_generation_verified = true;
    w.layout_generation = 0;
    CHECK(!pa_autoscroll_epoch_begin(&guard, &w, &epochs, 1001),
        "zero_layout_generation");
    w.layout_generation = 45;
    w.stage_director_pairing_consumed = true;
    CHECK(!pa_autoscroll_epoch_begin(&guard, &w, &epochs, 1001),
        "stage_director_first_refusal");
    w.stage_director_pairing_consumed = false;
    w.pointer_constrained = true;
    CHECK(!pa_autoscroll_epoch_begin(&guard, &w, &epochs, 1001),
        "pointer_constraint_refusal");
    w.pointer_constrained = false;
    w.session_locked = true;
    CHECK(!pa_autoscroll_epoch_begin(&guard, &w, &epochs, 1001),
        "session_lock_refusal");
    w.session_locked = false;
    w.compositor_event = false;
    CHECK(!pa_autoscroll_epoch_begin(&guard, &w, &epochs, 1001),
        "client_supplied_not_compositor_event");
    w.compositor_event = true;
    epochs.output = 0;
    CHECK(!pa_autoscroll_epoch_begin(&guard, &w, &epochs, 1001),
        "revoked_output");
    epochs = issued();
    CHECK(!pa_autoscroll_epoch_begin(&guard, &w, &epochs, 1081),
        "stale_callback_age");
    CHECK(!pa_autoscroll_epoch_begin(&guard, &w, &epochs, 999),
        "future_callback_time");
    CHECK(!guard.active, "refusal_never_arms");
    printf("PA_G1_REAL_C_POLICY_NEGATIVE_MATRIX_PASS\n");
}

static void adversarial_caller_limit_is_explicit(void) {
    struct pa_autoscroll_epoch_guard guard = {0};
    struct pa_autoscroll_witness forged = specimen();
    struct pa_autoscroll_object_epochs epochs = issued();

    /* IMPORTANT: the detached policy *cannot* authenticate how a caller
     * obtained each field. This all-positive test-only object arms it.
     * Native F-B bridge prevents such values from being supplied.
     * This is a REQUIRED pre-activation integration gate, not a pass
     * for physical identity. */
    forged.origin = PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT;
    CHECK(pa_autoscroll_epoch_begin(&guard, &forged, &epochs, 1001),
        "portable_guard_accepts_untrusted_forged_fields");
    CHECK(guard.active, "portable_guard_is_data_policy_only");

    /* A timer does not mint a new physical event. Unchanged observation
     * must cancel, even when the original owner tuple still looks live. */
    CHECK(!pa_autoscroll_epoch_tick(&guard, &forged, &epochs, 1002),
        "same_event_replay_denied");
    CHECK(!guard.active, "replay_clears_guard");
    CHECK(pa_autoscroll_epoch_begin(&guard, &forged, &epochs, 1001),
        "rearm_test_only");
    struct pa_autoscroll_witness new_event = forged;
    new_event.observed_at_ms = 1005;
    struct pa_autoscroll_object_epochs replaced = epochs;
    replaced.device++;
    CHECK(!pa_autoscroll_epoch_tick(&guard, &new_event, &replaced, 1006),
        "owner_device_change_denied");
    CHECK(!guard.active, "owner_replacement_clears_guard");

    CHECK(pa_autoscroll_epoch_begin(&guard, &forged, &epochs, 1001),
        "rearm_layout_test_only");
    new_event = forged;
    new_event.observed_at_ms = 1006;
    new_event.layout_generation++;
    CHECK(!pa_autoscroll_epoch_tick(&guard, &new_event, &epochs, 1007),
        "scene_layout_generation_change_denied");
    CHECK(!guard.active, "layout_change_clears_guard");

    CHECK(pa_autoscroll_epoch_begin(&guard, &forged, &epochs, 1001),
        "rearm_freshness_test_only");
    new_event = forged;
    new_event.observed_at_ms = 0;
    CHECK(!pa_autoscroll_epoch_tick(&guard, &new_event, &epochs, 1007),
        "timer_must_not_fabricate_fresh_event");
    CHECK(!guard.active, "timer_refusal_clears_guard");

    printf("PA_G1_FORGED_POSITIVE_ACCEPTED_BY_PORTABLE_POLICY_ONLY"
           "_NO_REAL_HARDWARE_ATTESTATION_PASS\n");
    printf("PA_G1_REPLAY_OWNER_LAYOUT_TIMER_CANCELLATION_PASS\n");
}

int main(void) {
    candidate_is_not_hardware();
    negative_evidence_refused();
    adversarial_caller_limit_is_explicit();
    printf("PA_G1_PROVENANCE_PREFLIGHT_PASS "
           "native_positive_witness=ABSENT "
           "production_axis_frames=ZERO\n");
    return 0;
}
