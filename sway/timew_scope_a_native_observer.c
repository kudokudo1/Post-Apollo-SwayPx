#include <math.h>
#include <stdint.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "sway/timew_scope_a_native_observer.h"
#include "sway/input/cursor.h"
#include "sway/input/seat.h"
#include "sway/output.h"
#include "sway/server.h"
#include "sway/tree/container.h"
#include "sway/tree/node.h"
#include "sway/tree/view.h"
#include "sway/tree/workspace.h"

/*
 * Dormant native observer. This translation unit is intentionally NOT
 * listed in sway/meson.build or registered as a public IPC/Wayland endpoint.
 *
 * A seat's present pointer target is not proof of a physical click, an
 * armed TimeW selection, a C3e authenticated origin or C5 consent. Merely
 * returning a candidate cannot authorize a compositor mutation.
 */
struct timew_scope_a_native_candidate
timew_scope_a_observe_pointer_candidate(struct sway_seat *seat) {
	struct timew_scope_a_native_candidate result = {0};
	if (!seat || !seat->wlr_seat || !seat->cursor ||
			!seat->cursor->cursor || server.session_lock.lock) {
		return result;
	}

	struct wlr_surface *hit = NULL;
	double sx = 0, sy = 0;
	struct sway_node *node = node_at_coords(seat,
			seat->cursor->cursor->x, seat->cursor->cursor->y,
			&hit, &sx, &sy);
	if (!node || node->type != N_CONTAINER || node->destroying ||
			!node->sway_container || !hit) {
		return result;
	}
	struct sway_container *con = node->sway_container;
	struct sway_view *view = con->view;
	struct sway_workspace *ws = con->pending.workspace;
	if (con->node.destroying || !view || view->destroying ||
			view->container != con || !ws || !ws->output ||
			!ws->output->wlr_output || !ws->output->enabled ||
			!ws->output->wlr_output->enabled ||
			con->pending.parent != NULL ||
			!container_is_floating(con) ||
			con->pending.fullscreen_mode != FULLSCREEN_NONE ||
			con->scratchpad) {
		return result;
	}
	/* XWayland, popup roots, subsurfaces and synthetic app IDs excluded. */
	if (view->type != SWAY_VIEW_XDG_SHELL ||
			!view->wlr_xdg_toplevel ||
			!view->wlr_xdg_toplevel->base ||
			!view->surface || !view->surface->mapped ||
			view->wlr_xdg_toplevel->base->surface != view->surface ||
			!view->surface->resource) {
		return result;
	}
	struct wlr_surface *root = wlr_surface_get_root_surface(hit);
	if (!root || root != view->surface || !root->mapped ||
			!root->resource) {
		return result;
	}
	struct wl_client *client = wl_resource_get_client(root->resource);
	if (!client) {
		return result;
	}
	if (!isfinite(con->pending.x) || !isfinite(con->pending.y) ||
			!isfinite(con->pending.width) ||
			!isfinite(con->pending.height) ||
			con->pending.width <= 0 || con->pending.height <= 0) {
		return result;
	}

	result.native_container_ref = (uintptr_t)con;
	result.native_root_surface_ref = (uintptr_t)root->resource;
	result.native_client_ref = (uintptr_t)client;
	result.native_seat_ref = (uintptr_t)seat->wlr_seat;
	result.native_output_ref = (uintptr_t)ws->output;
	result.native_workspace_ref = (uintptr_t)ws;
	result.transient_con_id = con->node.id;
	result.x = con->pending.x;
	result.y = con->pending.y;
	result.width = con->pending.width;
	result.height = con->pending.height;
	result.candidate_observed = true;

	/*
	 * All authority flags remain zero. The native pointer and transient
	 * con_id are NOT an authenticated lifetime. A protected future caller
	 * must mint/revoke epoch/generation and validate input/seat and external
	 * C3e/C4b/C5 custody separately before any action.
	 */
	return result;
}
