/*
 * G3F adversarial standalone C harness, never a real compositor verifier.
 * All input facts here are CALLER-FORGED. Even a successful OS-managed
 * diagnostic MUST NOT be an input/scroll authorization.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "ci_v5f_g3f_trust_policy.h"

#define REQUIRE(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "PA_G3F_ASSERTION_FAILED line=%d expression=%s\\n", \
            __LINE__, #condition); \
        return EXIT_FAILURE; \
    } \
} while (0)

static struct pa_g3f_source_facts complete_candidate(void) {
    return (struct pa_g3f_source_facts) {
        .native_callback = true,
        .current_event_timestamp = true,
        .middle_button = true,
        .middle_pressed = true,
        .pointer_device = true,
        .protocol_virtual = false,
        .known_kernel_synthetic = false,
        .native_libinput_backend = true,
        .native_libinput_handle = true,
        .udev_tree_candidate = true,
        .native_scroll_method_known = true,
        .button_down_scroll_conflict = false,
        .current_device_epoch = true,
        .current_seat_epoch = true,
        .event_device_attached_to_seat = true,
        .stage_director_first_refusal_clear = true,
        .claimed_device_enrollment = true,
        .claimed_matching_serial = true,
        .claimed_signed_witness = true,
    };
}

static int exhaustive_negative(void) {
    unsigned long tests = 0;
    /* 16 independent candidate facts, reversed individually or together.
     * Even the all-"perfect" forged case may only return a DIAGNOSTIC. */
    for (uint32_t mask = 0; mask < UINT32_C(65536); ++mask) {
        struct pa_g3f_source_facts f = complete_candidate();
        if (mask & (UINT32_C(1) << 0)) f.native_callback = false;
        if (mask & (UINT32_C(1) << 1)) f.current_event_timestamp = false;
        if (mask & (UINT32_C(1) << 2)) f.middle_button = false;
        if (mask & (UINT32_C(1) << 3)) f.middle_pressed = false;
        if (mask & (UINT32_C(1) << 4)) f.pointer_device = false;
        if (mask & (UINT32_C(1) << 5)) f.protocol_virtual = true;
        if (mask & (UINT32_C(1) << 6)) f.known_kernel_synthetic = true;
        if (mask & (UINT32_C(1) << 7)) f.native_libinput_backend = false;
        if (mask & (UINT32_C(1) << 8)) f.native_libinput_handle = false;
        if (mask & (UINT32_C(1) << 9)) f.udev_tree_candidate = false;
        if (mask & (UINT32_C(1) << 10)) f.native_scroll_method_known = false;
        if (mask & (UINT32_C(1) << 11)) f.button_down_scroll_conflict = true;
        if (mask & (UINT32_C(1) << 12)) f.current_device_epoch = false;
        if (mask & (UINT32_C(1) << 13)) f.current_seat_epoch = false;
        if (mask & (UINT32_C(1) << 14)) f.event_device_attached_to_seat = false;
        if (mask & (UINT32_C(1) << 15)) f.stage_director_first_refusal_clear = false;

        for (int mode = 0; mode <= 3; ++mode) {
            for (int selected = 0; selected <= 1; ++selected) {
                enum pa_g3f_requested_lane lane =
                    (enum pa_g3f_requested_lane)mode;
                enum pa_g3f_diagnostic found = pa_g3f_diagnose_source(
                    lane, selected != 0, &f);
                enum pa_g3f_diagnostic expected = PA_G3F_DIAG_DENIED;
                if (selected && mask == 0) {
                    if (lane == PA_G3F_LANE_OS_MANAGED) {
                        expected = PA_G3F_DIAG_OS_SOURCE_CANDIDATE_ONLY;
                    } else if (lane == PA_G3F_LANE_ENROLLED_DEVICE) {
                        expected = PA_G3F_DIAG_ENROLLMENT_BROKER_REQUIRED;
                    }
                }
                REQUIRE(found == expected);
                REQUIRE(!pa_g3f_may_consume_middle(lane, found));
                REQUIRE(!pa_g3f_may_emit_axis(lane, found));
                ++tests;
            }
        }
    }
    REQUIRE(tests == 524288UL);
    printf("PA_G3F_ALL_524288_INPUT_MODE_CASES_NONACTUATING_PASS "
           "no_button_consumption=1 no_scroll_axis=1\n");
    return EXIT_SUCCESS;
}

int main(void) {
    struct pa_g3f_source_facts f = complete_candidate();
    enum pa_g3f_diagnostic os = pa_g3f_diagnose_source(
        PA_G3F_LANE_OS_MANAGED, true, &f);
    enum pa_g3f_diagnostic strong = pa_g3f_diagnose_source(
        PA_G3F_LANE_ENROLLED_DEVICE, true, &f);
    REQUIRE(os == PA_G3F_DIAG_OS_SOURCE_CANDIDATE_ONLY);
    REQUIRE(strong == PA_G3F_DIAG_ENROLLMENT_BROKER_REQUIRED);
    REQUIRE(!pa_g3f_may_consume_middle(PA_G3F_LANE_OS_MANAGED, os));
    REQUIRE(!pa_g3f_may_emit_axis(PA_G3F_LANE_ENROLLED_DEVICE, strong));
    puts("PA_G3F_OS_LANE_IS_ONLY_SOURCE_CANDIDATE_PASS "
         "os_boundary_does_not_prove_hardware=1");
    puts("PA_G3F_ENROLLED_LANE_REQUIRES_INDEPENDENT_TRUST_BROKER_PASS "
         "serial_and_claimed_signature_ignored=1");

    REQUIRE(pa_g3f_diagnose_source(PA_G3F_LANE_OFF, true, &f) ==
        PA_G3F_DIAG_DENIED);
    REQUIRE(pa_g3f_diagnose_source(PA_G3F_LANE_OS_MANAGED, false, &f) ==
        PA_G3F_DIAG_DENIED);
    REQUIRE(pa_g3f_diagnose_source(PA_G3F_LANE_ENROLLED_DEVICE, false, &f) ==
        PA_G3F_DIAG_DENIED);
    REQUIRE(pa_g3f_diagnose_source(
        (enum pa_g3f_requested_lane)99, true, &f) == PA_G3F_DIAG_DENIED);
    REQUIRE(pa_g3f_diagnose_source(PA_G3F_LANE_OS_MANAGED, true, NULL) ==
        PA_G3F_DIAG_DENIED);
    puts("PA_G3F_OFF_UNSELECTED_INVALID_OR_MISSING_SOURCE_FAIL_CLOSED_PASS");

    /* A real Wayland virtual pointer has native epochs, potentially scene
     * and event times. It must be rejected on its virtual flag. */
    f.protocol_virtual = true;
    REQUIRE(pa_g3f_diagnose_source(PA_G3F_LANE_OS_MANAGED, true, &f) ==
        PA_G3F_DIAG_DENIED);
    REQUIRE(pa_g3f_diagnose_source(PA_G3F_LANE_ENROLLED_DEVICE, true, &f) ==
        PA_G3F_DIAG_DENIED);
    f.protocol_virtual = false;

    /* Even kernel input that appears libinput-backed may be uinput. Known
     * synthetic inputs are refused. Unknown synthetic inputs may still
     * masquerade as OS candidates: this is the EXPLICIT OS trust risk. */
    f.known_kernel_synthetic = true;
    REQUIRE(pa_g3f_diagnose_source(PA_G3F_LANE_OS_MANAGED, true, &f) ==
        PA_G3F_DIAG_DENIED);
    f.known_kernel_synthetic = false;
    REQUIRE(pa_g3f_diagnose_source(PA_G3F_LANE_OS_MANAGED, true, &f) ==
        PA_G3F_DIAG_OS_SOURCE_CANDIDATE_ONLY);
    REQUIRE(!pa_g3f_may_consume_middle(
        PA_G3F_LANE_OS_MANAGED, PA_G3F_DIAG_OS_SOURCE_CANDIDATE_ONLY));
    puts("PA_G3F_VIRTUAL_AND_KNOWN_UINPUT_DENIED_UNKNOWN_SPOOF_RISK_PASS");

    /* A forged enrolled device record/serial/claimed signed witness does
     * not create an independent trusted issuer, nor permission to scroll. */
    f.claimed_device_enrollment = false;
    f.claimed_matching_serial = false;
    f.claimed_signed_witness = false;
    REQUIRE(pa_g3f_diagnose_source(PA_G3F_LANE_ENROLLED_DEVICE, true, &f) ==
        PA_G3F_DIAG_ENROLLMENT_BROKER_REQUIRED);
    f.claimed_device_enrollment = true;
    f.claimed_matching_serial = true;
    f.claimed_signed_witness = true;
    REQUIRE(pa_g3f_diagnose_source(PA_G3F_LANE_ENROLLED_DEVICE, true, &f) ==
        PA_G3F_DIAG_ENROLLMENT_BROKER_REQUIRED);
    puts("PA_G3F_FORGED_ENROLLMENT_SERIAL_OR_SIGNATURE_CANNOT_PROMOTE_PASS");

    return exhaustive_negative();
}
