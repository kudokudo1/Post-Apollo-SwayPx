#include <stdint.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "sway/timew_scope_a_native_lifecycle.h"
#include "sway/timew_scope_a_native_observer.h"
#include "sway/input/seat.h"
#include "sway/server.h"
#include "sway/tree/container.h"
#include "sway/tree/view.h"

/*
 * DORMANT and NOT PRODUCTION LINKED. This adapter never reserves a
 * candidate, mints an epoch, approves a click, issues a command, or performs
 * geometry writes. It attaches revoke-only Wayland listeners after a fresh
 * in-process hit-test and exact snapshot match.
 */
bool timew_scope_a_native_lifecycle_bind_candidate(
		struct timew_scope_a_lifecycle_signals *watch,
		struct timew_scope_a_lifetime_candidate *ledger,
		struct sway_seat *seat, struct sway_container *con,
		struct sway_view *view) {
	if (!watch || watch->attached || !ledger ||
			ledger->state != TIMEW_SCOPE_A_CANDIDATE) {
		return false;
	}

	/* On a bad/stale native probe, consume the existing candidate.
	 * A later positive pointer hit must NOT revive the old generation. */
	if (!seat || !seat->wlr_seat || !con || !view ||
			server.session_lock.lock || con->node.destroying ||
			view->destroying || con->view != view ||
			view->container != con || !view->surface ||
			!view->surface->mapped || !view->surface->resource ||
			view->type != SWAY_VIEW_XDG_SHELL ||
			!view->wlr_xdg_toplevel ||
			!view->wlr_xdg_toplevel->base ||
			view->wlr_xdg_toplevel->base->surface != view->surface) {
		goto deny;
	}
	struct timew_scope_a_native_candidate observed =
		timew_scope_a_observe_pointer_candidate(seat);
	if (!observed.candidate_observed ||
			observed.operator_selection_authenticated ||
			observed.c3e_producer_authenticated ||
			observed.c5_spend_verified ||
			observed.native_resize_allowed ||
			observed.native_move_allowed ||
			observed.protected_receipt_verified ||
			observed.native_container_ref != (uintptr_t)con ||
			observed.native_root_surface_ref !=
				(uintptr_t)view->surface->resource ||
			observed.native_seat_ref != (uintptr_t)seat->wlr_seat ||
			observed.native_client_ref != (uintptr_t)
				wl_resource_get_client(view->surface->resource)) {
		goto deny;
	}

	int32_t x, y, width, height;
	if (!timew_scope_a_checked_pending_rect(observed.x, observed.y,
			observed.width, observed.height,
			&x, &y, &width, &height)) {
		goto deny;
	}
	const struct timew_scope_a_lifetime_observation *selected =
		&ledger->selected;
	if (selected->native_container_ref != observed.native_container_ref ||
			selected->native_root_surface_ref !=
				observed.native_root_surface_ref ||
			selected->native_seat_ref != observed.native_seat_ref ||
			selected->native_output_ref != observed.native_output_ref ||
			selected->native_workspace_ref !=
				observed.native_workspace_ref ||
			selected->x != x || selected->y != y ||
			selected->width != width || selected->height != height ||
			!selected->mapped || !selected->exact_xdg_toplevel ||
			!selected->floating || selected->fullscreen ||
			selected->scratchpad || selected->session_locked ||
			selected->destroying) {
		goto deny;
	}

	/*
	 * These are actual native SwayPX signals. view_unmap is emitted
	 * before container_begin_destroy; the container node destroy is
	 * emitted before detach/free. Root-surface destroy is an additional
	 * exact-object loss path. All listeners self-detach on first event.
	 */
	if (!timew_scope_a_lifecycle_signals_bind(watch, ledger,
			&view->events.unmap, &con->node.events.destroy,
			&view->surface->events.destroy, (uintptr_t)con,
			(uintptr_t)view->surface->resource)) {
		goto deny;
	}
	return true;

deny:
	timew_scope_a_lifetime_revoke(ledger);
	return false;
}
