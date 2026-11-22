/*
 * V5F-G3D: DISPOSABLE HEADLESS CI-ONLY scene and keyboard LED observer.
 *
 * This file is NEVER part of the normal SwayPX build or installed runtime.
 * The sole caller is injected in a copied CI-runner seat-cursor SET path.
 * It does not inject input, grant physical origin, dispatch axis/frame, or
 * change actual scene, keyboard, lock state or any user-facing controls.
 *
 * The native keyboard's XKB Scroll Lock LED is a provisional diagnostic:
 * a virtual keyboard can set it. That is NOT evidence of physical HID
 * origin, authenticated Scroll Lock policy or even all-seat agreement.
 */
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_seat.h>
#include <xkbcommon/xkbcommon-names.h>
#include "log.h"
#include "sway/input/cursor.h"
#include "sway/input/seat.h"
#include "sway/output.h"
#include "sway/server.h"
#include "sway/tree/container.h"
#include "sway/tree/node.h"
#include "sway/tree/root.h"
#include "sway/tree/view.h"

enum g3d_scroll_led { G3D_LOCK_UNKNOWN, G3D_LOCK_OFF, G3D_LOCK_ON };

static enum g3d_scroll_led native_scroll_led(
        const struct sway_seat *seat) {
    if (seat == NULL || seat->wlr_seat == NULL) {
        return G3D_LOCK_UNKNOWN;
    }
    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat->wlr_seat);
    if (keyboard == NULL || keyboard->keymap == NULL
            || keyboard->xkb_state == NULL) {
        return G3D_LOCK_UNKNOWN;
    }
    xkb_led_index_t led = xkb_keymap_led_get_index(
        keyboard->keymap, XKB_LED_NAME_SCROLL);
    if (led == XKB_LED_INVALID) {
        return G3D_LOCK_UNKNOWN;
    }
    int active = xkb_state_led_index_is_active(keyboard->xkb_state, led);
    if (active < 0) {
        return G3D_LOCK_UNKNOWN;
    }
    return active ? G3D_LOCK_ON : G3D_LOCK_OFF;
}

void pa_g3d_native_sample_on_cursor_set(struct sway_cursor *cursor) {
    if (cursor == NULL || cursor->cursor == NULL ||
            cursor->seat == NULL || root == NULL ||
            root->output_layout == NULL) {
        sway_log(SWAY_INFO, "PA_G3D_NATIVE_SAMPLE "
            "scene=0 lock=UNKNOWN missing_owner=1 axis_frames=0");
        return;
    }
    struct wlr_surface *surface = NULL;
    double sx = 0, sy = 0;
    double lx = cursor->cursor->x, ly = cursor->cursor->y;
    struct sway_node *hit = node_at_coords(
        cursor->seat, lx, ly, &surface, &sx, &sy);
    struct sway_container *target = NULL;
    struct sway_output *output = NULL;
    struct wlr_output *layout = wlr_output_layout_output_at(
        root->output_layout, lx, ly);
    bool direct_view = false, layout_same = false;
    uint64_t target_epoch = 0, output_epoch = 0;

    if (hit != NULL && hit->type == N_CONTAINER &&
            !hit->destroying && !hit->dirty &&
            hit->instruction == NULL && surface != NULL &&
            hit->sway_container != NULL) {
        target = hit->sway_container;
        /* Sway's node_at_coords may resolve a popup into its VIEW
         * container. Demand the DIRECT application surface; don't
         * mistake a popup/subsurface/layer hit for the mapped view. */
        if (target->view != NULL && !target->view->destroying &&
                target->view->surface == surface) {
            direct_view = true;
            target_epoch = container_autoscroll_target_lifetime_epoch(target);
            output = node_get_output(hit);
            if (output != NULL && output->enabled &&
                    output->wlr_output != NULL &&
                    output->wlr_output->enabled &&
                    output != root->fallback_output &&
                    !output->node.destroying && !output->node.dirty &&
                    output->node.instruction == NULL) {
                layout_same = layout != NULL &&
                    layout == output->wlr_output;
                output_epoch = output_autoscroll_lifetime_epoch(output);
            }
        }
    }

    /* This is ONE synchronous native hit/layout observation, NOT a
     * compositor-signed transaction or a layout generation.
     * Preserve the false layout-generation gate in production. */
    bool scene = direct_view && layout_same &&
        target_epoch != 0 && output_epoch != 0 &&
        server.session_lock.lock == NULL &&
        cursor->active_constraint == NULL &&
        !cursor->simulating_pointer_from_touch &&
        !cursor->simulating_pointer_from_tool_tip &&
        !cursor->simulating_pointer_from_tool_button;
    enum g3d_scroll_led scroll = native_scroll_led(cursor->seat);
    const char *status = scroll == G3D_LOCK_ON ? "ON" :
        scroll == G3D_LOCK_OFF ? "OFF" : "UNKNOWN";

    sway_log(SWAY_INFO,
        "PA_G3D_NATIVE_SAMPLE "
        "scene=%d direct=%d layout_same=%d "
        "target_node=%" PRIu64 " target_epoch=%" PRIu64
        " output_epoch=%" PRIu64 " "
        "lock=%s lock_authority=DIAGNOSTIC_ONLY "
        "layout_generation=UNVERIFIED "
        "physical_origin=UNKNOWN axis_frames=0",
        scene, direct_view, layout_same,
        scene && hit != NULL ? (uint64_t)hit->id : UINT64_C(0),
        target_epoch, output_epoch, status);
}
