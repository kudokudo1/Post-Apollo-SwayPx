/*
 * G3H native compositor-owned OS-managed input observer. CI COPY ONLY.
 *
 * An actual cursor callback supplies actual event, seat and wlroots device
 * fields to the G3F NON-ACTUATING policy model. No pointer output, no
 * native positive origin witness, no timers, no IPC and no event mutation.
 * Real application middle-click dispatch remains untouched.
 *
 * "OS-managed" is the accepted DEVELOPMENT priority, NOT a claim that
 * libinput/udev authenticates physical hardware or permits scrolling.
 */
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <linux/input-event-codes.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_pointer.h>
#include <wayland-server-core.h>

#include "log.h"
#include "sway/input/cursor.h"
#include "sway/input/input-manager.h"
#include "sway/input/post_apollo_autoscroll_origin_policy.h"
#include "sway/input/post_apollo_autoscroll_owner_snapshot.h"
#include "sway/input/post_apollo_autoscroll_source.h"
#include "sway/input/seat.h"
#include "sway/server.h"
#include "ci_v5f_g3f_trust_policy.h"

void pa_g3h_native_os_managed_diagnostic_observe(
        struct sway_cursor *cursor,
        struct wlr_pointer_button_event *event,
        bool weather_paired,
        bool company_paired) {
    if (cursor == NULL || event == NULL || event->pointer == NULL ||
            event->button != BTN_MIDDLE) {
        return;
    }

    /* A native event provides the pointer. Caller-supplied device IDs,
     * settings bits and Wayland IPC are not accepted as authoritative. */
    struct wlr_input_device *device = &event->pointer->base;
    struct sway_input_device *native = NULL;
    if (server.input != NULL) {
        struct sway_input_device *it = NULL;
        wl_list_for_each(it, &server.input->devices, link) {
            if (it->wlr_device == device) {
                native = it;
                break;
            }
        }
    }
    const bool virtual = native == NULL || native->is_virtual;
    const uint64_t device_epoch = native != NULL ?
        input_manager_autoscroll_pointer_lifetime_epoch(native) : 0;
    const uint64_t seat_epoch = cursor->seat != NULL ?
        seat_autoscroll_lifetime_epoch(cursor->seat) : 0;

    const struct pa_autoscroll_origin_facts source =
        pa_autoscroll_source_probe(device);
    const bool os_candidate = pa_autoscroll_origin_classify(&source) ==
        PA_AUTOSCROLL_ORIGIN_LIBINPUT_CANDIDATE;

    struct pa_autoscroll_owner_snapshot owners = {0};
    const bool owner_admitted =
        pa_autoscroll_owner_snapshot_read(cursor, device, &owners);
    /* A valid four-owner snapshot also performs same-seat, direct XDG
     * target and output continuity checks. Lack of such a snapshot must
     * never be repaired by fabricated numeric object IDs. */
    const bool valid_owners = owner_admitted &&
        owners.device != 0 && owners.seat != 0 &&
        owners.target != 0 && owners.output != 0 &&
        owners.device == device_epoch && owners.seat == seat_epoch;

    /* G3F input bits are STILL a diagnostic model. These are observed in
     * the compositor, but a native libinput candidate is not HID proof.
     * The G3F model cannot authorize or consume any input, even with all
     * candidate source flags set true. */
    const struct pa_g3f_source_facts facts = {
        .native_callback = true,
        .current_event_timestamp = event->time_msec != 0,
        .middle_button = event->button == BTN_MIDDLE,
        .middle_pressed =
            event->state == WL_POINTER_BUTTON_STATE_PRESSED,
        .pointer_device = device->type == WLR_INPUT_DEVICE_POINTER,
        .protocol_virtual = virtual,
        .known_kernel_synthetic = false, /* UNKNOWN, not attested false */
        .native_libinput_backend = source.libinput_backend,
        .native_libinput_handle = source.libinput_handle_present,
        .udev_tree_candidate = source.native_scroll_method_known &&
            source.libinput_backend && source.libinput_handle_present,
        .native_scroll_method_known = source.native_scroll_method_known,
        .button_down_scroll_conflict = source.native_scroll_on_button_down,
        .current_device_epoch = valid_owners,
        .current_seat_epoch = valid_owners,
        .event_device_attached_to_seat = valid_owners,
        .stage_director_first_refusal_clear =
            !weather_paired && !company_paired,
        .claimed_device_enrollment = false,
        .claimed_matching_serial = false,
        .claimed_signed_witness = false,
    };

    /* Test-only "selected" means exercise the OS lane in a disposable
     * compositor, NOT read real settings, consent, or user activation. */
    const enum pa_g3f_diagnostic status = pa_g3f_diagnose_source(
        PA_G3F_LANE_OS_MANAGED, true, &facts);
    const bool may_consume = pa_g3f_may_consume_middle(
        PA_G3F_LANE_OS_MANAGED, status);
    const bool may_axis = pa_g3f_may_emit_axis(
        PA_G3F_LANE_OS_MANAGED, status);

    /* Log only. Never intercept, replace, consume or dispatch event. */
    sway_log(SWAY_INFO,
        "PA_G3H_NATIVE_OS_DIAG phase=%d virtual=%d "
        "source_candidate=%d os_diag=%d owner_snapshot=%d "
        "device_epoch=%" PRIu64 " seat_epoch=%" PRIu64
        " stage_refusal_clear=%d os_mode=TEST_ONLY "
        "can_consume=%d can_axis=%d production=HARD_OFF",
        event->state == WL_POINTER_BUTTON_STATE_PRESSED,
        virtual, os_candidate, status, owner_admitted,
        device_epoch, seat_epoch,
        facts.stage_director_first_refusal_clear,
        may_consume, may_axis);
}
