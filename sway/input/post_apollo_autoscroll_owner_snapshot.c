/*
 * V5F-A compositor-owned object epoch observation.
 *
 * Only a synchronous, nonactuating source check. Every returned epoch comes
 * from an owner-minted slot, never a pointer address or Wayland IPC value.
 * The true physical/libinput provenance and trusted V5C witness remain
 * separate, unproven gates. Production stays compile-time HARD OFF.
 */
#include <stddef.h>
#include <string.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_output_layout.h>

#include "sway/input/cursor.h"
#include "sway/input/input-manager.h"
#include "sway/input/post_apollo_autoscroll_owner_snapshot.h"
#include "sway/input/seat.h"
#include "sway/output.h"
#include "sway/server.h"
#include "sway/tree/container.h"
#include "sway/tree/node.h"
#include "sway/tree/root.h"
#include "sway/tree/view.h"

static struct sway_seat *find_live_seat(struct sway_cursor *cursor) {
    struct sway_seat *candidate = NULL;
    wl_list_for_each(candidate, &server.input->seats, link) {
        /* Pointer equality before dereference prevents stale-seat reads
         * when a caller accidentally holds a destroyed seat address. */
        if (candidate == cursor->seat) {
            return candidate;
        }
    }
    return NULL;
}

static struct sway_input_device *find_live_native_pointer(
        struct wlr_input_device *event_device) {
    struct sway_input_device *candidate = NULL;
    wl_list_for_each(candidate, &server.input->devices, link) {
        if (candidate->wlr_device == event_device) {
            return candidate->is_virtual ? NULL : candidate;
        }
    }
    return NULL;
}

static bool device_attached_to_seat(
        struct sway_seat *seat, struct sway_input_device *device) {
    struct sway_seat_device *seat_device = NULL;
    wl_list_for_each(seat_device, &seat->devices, link) {
        if (seat_device->input_device == device &&
                seat_device->sway_seat == seat) {
            return true;
        }
    }
    return false;
}

static bool epochs_unique_and_nonzero(
        const struct pa_autoscroll_owner_snapshot *epochs) {
    return epochs->device != 0 && epochs->seat != 0 &&
        epochs->target != 0 && epochs->output != 0 &&
        epochs->device != epochs->seat &&
        epochs->device != epochs->target &&
        epochs->device != epochs->output &&
        epochs->seat != epochs->target &&
        epochs->seat != epochs->output &&
        epochs->target != epochs->output;
}

bool pa_autoscroll_owner_snapshot_read(
        struct sway_cursor *cursor,
        struct wlr_input_device *event_device,
        struct pa_autoscroll_owner_snapshot *snapshot) {
    if (snapshot == NULL) {
        return false;
    }
    /* Refusal never leaks a partially populated object-lifetime record. */
    memset(snapshot, 0, sizeof(*snapshot));
    if (server.input == NULL || root == NULL ||
            root->output_layout == NULL || cursor == NULL ||
            cursor->cursor == NULL || event_device == NULL ||
            cursor->seat == NULL || server.session_lock.lock != NULL ||
            cursor->active_constraint != NULL ||
            cursor->simulating_pointer_from_touch ||
            cursor->simulating_pointer_from_tool_tip ||
            cursor->simulating_pointer_from_tool_button) {
        return false;
    }

    struct sway_seat *seat = find_live_seat(cursor);
    if (seat == NULL || seat->cursor != cursor || seat->wlr_seat == NULL) {
        return false;
    }
    struct sway_input_device *native = find_live_native_pointer(event_device);
    if (native == NULL || !device_attached_to_seat(seat, native)) {
        return false;
    }

    struct pa_autoscroll_owner_snapshot candidate = {
        .device = input_manager_autoscroll_pointer_lifetime_epoch(native),
        .seat = seat_autoscroll_lifetime_epoch(seat),
    };
    if (candidate.device == 0 || candidate.seat == 0) {
        return false;
    }

    struct wlr_surface *hit_surface = NULL;
    double sx = 0, sy = 0;
    struct sway_node *node = node_at_coords(
        seat, cursor->cursor->x, cursor->cursor->y,
        &hit_surface, &sx, &sy);
    if (node == NULL || node->type != N_CONTAINER || node->destroying ||
            node->dirty || node->instruction != NULL ||
            hit_surface == NULL || node->sway_container == NULL) {
        return false;
    }
    struct sway_container *target = node->sway_container;
    /* Direct mapped view only. A popup, layer, XWayland unmanaged surface,
     * subsurface or transaction-pending container is NOT silently trusted. */
    if (target->view == NULL || target->view->destroying ||
            target->view->surface != hit_surface) {
        return false;
    }
    candidate.target = container_autoscroll_target_lifetime_epoch(target);
    if (candidate.target == 0) {
        return false;
    }

    struct sway_output *output = node_get_output(node);
    if (output == NULL || !output->enabled || output->wlr_output == NULL ||
            !output->wlr_output->enabled || root->fallback_output == output ||
            output->node.destroying || output->node.dirty ||
            output->node.instruction != NULL) {
        return false;
    }
    /* Bind hit target to the actual layout output at cursor coordinates.
     * Do not infer a monitor solely from the target's workspace pointer. */
    struct wlr_output *layout_output = wlr_output_layout_output_at(
        root->output_layout, cursor->cursor->x, cursor->cursor->y);
    if (layout_output == NULL || layout_output != output->wlr_output) {
        return false;
    }
    candidate.output = output_autoscroll_lifetime_epoch(output);
    if (!epochs_unique_and_nonzero(&candidate)) {
        return false;
    }
    *snapshot = candidate;
    return true;
}
