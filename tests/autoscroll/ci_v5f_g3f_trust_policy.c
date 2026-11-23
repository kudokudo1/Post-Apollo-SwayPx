/*
 * V5F-G3F standalone, non-actuating input boundary policy MODEL.
 * NOT part of normal sway/ meson, not a credential issuer, not production.
 */
#include <stddef.h>
#include "ci_v5f_g3f_trust_policy.h"

static bool os_native_source_candidate(const struct pa_g3f_source_facts *f) {
    if (f == NULL) {
        return false;
    }
    /* Native libinput and udev are NECESSARY candidate observations, not
     * evidence of genuine physical mouse. uinput can look OS-managed. */
    return f->native_callback &&
        f->current_event_timestamp &&
        f->middle_button &&
        f->middle_pressed &&
        f->pointer_device &&
        !f->protocol_virtual &&
        !f->known_kernel_synthetic &&
        f->native_libinput_backend &&
        f->native_libinput_handle &&
        f->udev_tree_candidate &&
        f->native_scroll_method_known &&
        !f->button_down_scroll_conflict &&
        f->current_device_epoch &&
        f->current_seat_epoch &&
        f->event_device_attached_to_seat &&
        f->stage_director_first_refusal_clear;
}

enum pa_g3f_diagnostic pa_g3f_diagnose_source(
        enum pa_g3f_requested_lane lane,
        bool operator_selected,
        const struct pa_g3f_source_facts *facts) {
    /* Nothing defaults to either trust lane, including malformed values. */
    if (!operator_selected || lane == PA_G3F_LANE_OFF ||
            (lane != PA_G3F_LANE_OS_MANAGED &&
             lane != PA_G3F_LANE_ENROLLED_DEVICE) ||
            !os_native_source_candidate(facts)) {
        return PA_G3F_DIAG_DENIED;
    }
    if (lane == PA_G3F_LANE_OS_MANAGED) {
        /* This acknowledges a source CANDIDATE under the ordinary OS
         * input trust boundary. Kernel synthetic HID can spoof a normal
         * libinput/udev path, and the caller can forge all fields here.
         * Therefore this status NEVER grants permission. */
        return PA_G3F_DIAG_OS_SOURCE_CANDIDATE_ONLY;
    }

    /* Claimed enrollment, serial ID, or caller-supplied "valid signature"
     * are all ignored. Genuine enrolled-device trust requires a separate
     * privileged issuer + attestation/root-of-trust design, operator
     * approval and independent review. No such issuer exists in G3F.
     */
    return PA_G3F_DIAG_ENROLLMENT_BROKER_REQUIRED;
}

bool pa_g3f_may_consume_middle(
        enum pa_g3f_requested_lane lane,
        enum pa_g3f_diagnostic diagnostic) {
    (void)lane;
    (void)diagnostic;
    return false;
}

bool pa_g3f_may_emit_axis(
        enum pa_g3f_requested_lane lane,
        enum pa_g3f_diagnostic diagnostic) {
    (void)lane;
    (void)diagnostic;
    return false;
}
