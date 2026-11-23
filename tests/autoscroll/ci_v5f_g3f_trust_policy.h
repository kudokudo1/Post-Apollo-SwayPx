#ifndef PA_G3F_CI_TWO_TIER_TRUST_POLICY_H
#define PA_G3F_CI_TWO_TIER_TRUST_POLICY_H

/*
 * V5F-G3F SOURCE-ONLY / TEST-ONLY reference policy.
 *
 * This header is NEVER included in sway/ and is never linked into the
 * compositor or user desktop. Caller-supplied booleans CANNOT establish
 * physical hardware trust, login-session authorization, enrollment, or
 * permission to consume a middle-click.
 *
 * All outputs are non-actuating DIAGNOSTIC labels, not credentials.
 */
#include <stdbool.h>
#include <stdint.h>

enum pa_g3f_requested_lane {
    PA_G3F_LANE_OFF = 0,
    PA_G3F_LANE_OS_MANAGED = 1,
    PA_G3F_LANE_ENROLLED_DEVICE = 2,
};

enum pa_g3f_diagnostic {
    PA_G3F_DIAG_DENIED = 0,
    PA_G3F_DIAG_OS_SOURCE_CANDIDATE_ONLY = 1,
    PA_G3F_DIAG_ENROLLMENT_BROKER_REQUIRED = 2,
};

struct pa_g3f_source_facts {
    /* ONLY an actual compositor event owner may eventually fill these.
     * In this standalone model, all are UNTRUSTED test-input fields. */
    bool native_callback;
    bool current_event_timestamp;
    bool middle_button;
    bool middle_pressed;
    bool pointer_device;
    bool protocol_virtual;
    bool known_kernel_synthetic;
    bool native_libinput_backend;
    bool native_libinput_handle;
    bool udev_tree_candidate;
    bool native_scroll_method_known;
    bool button_down_scroll_conflict;
    bool current_device_epoch;
    bool current_seat_epoch;
    bool event_device_attached_to_seat;
    bool stage_director_first_refusal_clear;

    /* Deliberately untrusted and IGNORED by this classification.
     * These claims must never make an enrolled HID appear authenticated. */
    bool claimed_device_enrollment;
    bool claimed_matching_serial;
    bool claimed_signed_witness;
};

/* operator_selected is a proposal input only. This source-only library
 * DOES NOT read a real settings file or prove who selected the lane. */
enum pa_g3f_diagnostic pa_g3f_diagnose_source(
    enum pa_g3f_requested_lane lane,
    bool operator_selected,
    const struct pa_g3f_source_facts *facts);

/* The only authorization result G3F can produce is false. */
bool pa_g3f_may_consume_middle(
    enum pa_g3f_requested_lane lane,
    enum pa_g3f_diagnostic diagnostic);

bool pa_g3f_may_emit_axis(
    enum pa_g3f_requested_lane lane,
    enum pa_g3f_diagnostic diagnostic);

#endif
