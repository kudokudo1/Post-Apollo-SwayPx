/*
 * Compositor-owned read-only snapshot of actual pointer/seat/target/output
 * lifetimes. HARD OFF: not an admission or an event source. No timer, events,
 * IPC, positive physical witness or Wayland axis/frame dispatch.
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_output_layout.h>
#include "sway/input/cursor.h"
#include "sway/input/input-manager.h"
#include "sway/input/post_apollo_autoscroll_snapshot.h"
#include "sway/input/seat.h"
#include "sway/output.h"
#include "sway/server.h"
#include "sway/tree/container.h"
#include "sway/tree/node.h"
#include "sway/tree/root.h"
#include "list.h"

bool pa_autoscroll_collect_compositor_epochs(
        const struct sway_cursor *cursor,
        const struct wlr_input_device *device,
        struct pa_autoscroll_object_epochs *out) {
    if (out == NULL) {
        return false;
    }
    memset(out, 0, sizeof(*out));
    if (cursor == NULL || cursor->cursor == NULL || device == NULL ||
            cursor->active_constraint != NULL || server.input == NULL ||
            server.session_lock.lock != NULL || root == NULL ||
            root->output_layout == NULL || root->outputs == NULL) {
        return false;
    }

    /* Find the actual live seat from the manager list BEFORE touching the
     * requested seat object. Do not dereference a stale seat pointer. */
    struct sway_seat *seat = NULL;
    struct sway_seat *candidate_seat;
    wl_list_for_each(candidate_seat, &server.input->seats, link) {
        if (candidate_seat == cursor->seat) {
            seat = candidate_seat;
            break;
        }
    }
    if (seat == NULL || seat->cursor != cursor) {
        return false;
    }

    /* Compare supplied device pointer against existing compositor-owned
     * wlroots wrappers, never dereference an unknown/stale event pointer. */
    struct sway_input_device *input_device = NULL;
    struct sway_input_device *candidate;
    wl_list_for_each(candidate, &server.input->devices, link) {
        if (candidate->wlr_device == device) {
            input_device = candidate;
            break;
        }
    }
    if (input_device == NULL || input_device->is_virtual) {
        return false;
    }

    /* A registered device is not sufficient if it belongs to another seat
     * or was detached. Never synthesize a device-to-seat relationship. */
    bool seat_attached = false;
    struct sway_seat_device *seat_device;
    wl_list_for_each(seat_device, &seat->devices, link) {
        if (seat_device->input_device == input_device) {
            seat_attached = true;
            break;
        }
    }
    if (!seat_attached) {
        return false;
    }

    struct wlr_surface *surface = NULL;
    double sx = 0.0, sy = 0.0;
    struct sway_node *node = node_at_coords(seat, cursor->cursor->x,
        cursor->cursor->y, &surface, &sx, &sy);
    if (node == NULL || node->type != N_CONTAINER ||
            node->destroying || surface == NULL) {
        return false;
    }

    /* Only trust a wlroots output that the compositor currently has in the
     * enabled root output list. Never dereference an unknown layout result.
     * Conservatively deny windows that extend beyond their owning output. */
    struct wlr_output *wlr_output = wlr_output_layout_output_at(
        root->output_layout, cursor->cursor->x, cursor->cursor->y);
    if (wlr_output == NULL) {
        return false;
    }
    struct sway_output *output = NULL;
    for (int i = 0; i < root->outputs->length; ++i) {
        struct sway_output *candidate_output = root->outputs->items[i];
        if (candidate_output != NULL &&
                candidate_output->wlr_output == wlr_output) {
            output = candidate_output;
            break;
        }
    }
    if (output == NULL || node_get_output(node) != output) {
        return false;
    }

    /* Read the four actual object owner accessors only after live list and
     * scene membership is verified. All fields are from this compositor's
     * single input-manager issuer, never from an IPC-provided epoch. */
    struct pa_autoscroll_object_epochs snapshot = {
        .device = input_manager_autoscroll_pointer_lifetime_epoch(input_device),
        .seat = seat_autoscroll_lifetime_epoch(seat),
        .target = container_autoscroll_target_lifetime_epoch(node->sway_container),
        .output = output_autoscroll_lifetime_epoch(output),
    };
    if (snapshot.device == 0 || snapshot.seat == 0 ||
            snapshot.target == 0 || snapshot.output == 0) {
        return false;
    }
    *out = snapshot;
    return true;
}
