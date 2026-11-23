/*
 * G3E disposable CI compositor middle-button *negative* observer.
 *
 * The only caller is injected AFTER Weather/Company first refusal evaluation
 * in a copied GitHub-runner compositor. Production source remains unchanged.
 * This code CANNOT authorize autoscroll, set "physical verified", consume
 * a button, arm a timer or emit wheel/axis/frame output. A libinput-backed
 * kernel device, if later observed, is only a CANDIDATE (uinput exists).
 */
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <linux/input-event-codes.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_seat.h>
#include <xkbcommon/xkbcommon-names.h>
#include "log.h"
#include "sway/input/cursor.h"
#include "sway/input/input-manager.h"
#include "sway/input/post_apollo_autoscroll_origin_policy.h"
#include "sway/input/post_apollo_autoscroll_owner_snapshot.h"
#include "sway/input/post_apollo_autoscroll_source.h"
#include "sway/input/seat.h"
#include "sway/output.h"
#include "sway/server.h"
#include "sway/tree/container.h"
#include "sway/tree/node.h"
#include "sway/tree/root.h"
#include "sway/tree/view.h"

void pa_g3e_native_middle_negative_observe(
        struct sway_cursor *cursor,
        struct wlr_pointer_button_event *event,
        bool weather_paired, bool company_paired) {
    if (cursor == NULL || event == NULL || event->pointer == NULL ||
            event->button != BTN_MIDDLE) {
        return;
    }

    struct wlr_input_device *device = &event->pointer->base;
    struct sway_input_device *native = NULL;
    if (server.input != NULL) {
        struct sway_input_device *candidate;
        wl_list_for_each(candidate, &server.input->devices, link) {
            if (candidate->wlr_device == device) {
                native = candidate;
                break;
            }
        }
    }

    /* Nonzero RAW object-lifetime is possible for software virtual input.
     * input_manager_autoscroll_pointer_lifetime_epoch() must STILL be 0. */
    uint64_t raw_device = native != NULL ? pa_autoscroll_lifetime_epoch(
        &native->autoscroll_lifetime, PA_LIFETIME_DEVICE) : 0;
    uint64_t physical_eligible = native != NULL ?
        input_manager_autoscroll_pointer_lifetime_epoch(native) : 0;
    uint64_t seat_epoch = cursor->seat != NULL ?
        seat_autoscroll_lifetime_epoch(cursor->seat) : 0;
    struct pa_autoscroll_origin_facts facts =
        pa_autoscroll_source_probe(device);
    bool source_candidate = pa_autoscroll_origin_classify(&facts) ==
        PA_AUTOSCROLL_ORIGIN_LIBINPUT_CANDIDATE;
    struct pa_autoscroll_owner_snapshot snapshot = {0};
    bool owner_snapshot = pa_autoscroll_owner_snapshot_read(
        cursor, device, &snapshot);

    struct wlr_surface *surface = NULL;
    double sx = 0, sy = 0;
    struct sway_node *hit = NULL;
    struct sway_output *output = NULL;
    uint64_t target_epoch = 0, output_epoch = 0;
    bool direct_scene = false, layout_same = false;
    if (cursor->seat != NULL && cursor->cursor != NULL && root != NULL &&
            root->output_layout != NULL) {
        hit = node_at_coords(cursor->seat,
            cursor->cursor->x, cursor->cursor->y, &surface, &sx, &sy);
        if (hit != NULL && hit->type == N_CONTAINER &&
                !hit->destroying && !hit->dirty &&
                hit->instruction == NULL && surface != NULL &&
                hit->sway_container != NULL) {
            struct sway_container *target = hit->sway_container;
            direct_scene = target->view != NULL &&
                !target->view->destroying &&
                target->view->surface == surface;
            if (direct_scene) {
                target_epoch = container_autoscroll_target_lifetime_epoch(target);
                output = node_get_output(hit);
                if (output != NULL && output->enabled &&
                        output->wlr_output != NULL &&
                        output->wlr_output->enabled &&
                        output != root->fallback_output &&
                        !output->node.destroying && !output->node.dirty &&
                        output->node.instruction == NULL) {
                    struct wlr_output *layout =
                        wlr_output_layout_output_at(
                            root->output_layout, cursor->cursor->x,
                            cursor->cursor->y);
                    layout_same = layout != NULL &&
                        layout == output->wlr_output;
                    output_epoch = output_autoscroll_lifetime_epoch(output);
                }
            }
        }
    }

    /* Native LED is diagnostic only and may come from a virtual keyboard.
     * Missing seat/keymap/LED is UNKNOWN rather than assumed unlocked. */
    const char *lock = "UNKNOWN";
    if (cursor->seat != NULL && cursor->seat->wlr_seat != NULL) {
        struct wlr_keyboard *keyboard =
            wlr_seat_get_keyboard(cursor->seat->wlr_seat);
        if (keyboard != NULL && keyboard->keymap != NULL &&
                keyboard->xkb_state != NULL) {
            xkb_led_index_t led = xkb_keymap_led_get_index(
                keyboard->keymap, XKB_LED_NAME_SCROLL);
            if (led != XKB_LED_INVALID) {
                int active = xkb_state_led_index_is_active(
                    keyboard->xkb_state, led);
                lock = active < 0 ? "UNKNOWN" : active ? "ON" : "OFF";
            }
        }
    }

    /* We purposely REFUSE to derive a PA_POINTER_ORIGIN_PHYSICAL_LIBINPUT
     * witness from the native facts here, even if one day source_candidate
     * is nonzero. No positive guard begin, admission, or dispatch exists. */
    sway_log(SWAY_INFO,
        "PA_G3E_REAL_NATIVE_MIDDLE_NEGATIVE "
        "pressed=%d virtual=%d raw_device=%" PRIu64
        " eligible=%" PRIu64 " seat=%" PRIu64
        " target=%" PRIu64 " output=%" PRIu64
        " scene=%d layout_same=%d source_candidate=%d "
        "snapshot_admitted=%d weather=%d company=%d "
        "lock=%s trust=DENIED axis_frames=0",
        event->state == WL_POINTER_BUTTON_STATE_PRESSED,
        native != NULL && native->is_virtual,
        raw_device, physical_eligible, seat_epoch,
        target_epoch, output_epoch,
        direct_scene, layout_same, source_candidate,
        owner_snapshot, weather_paired, company_paired, lock);
}
