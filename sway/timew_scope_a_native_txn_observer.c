#include <math.h>
#include <stdint.h>
#include <string.h>

#include <wlr/types/wlr_output.h>

#include "sway/timew_scope_a_native_txn_observer.h"
#include "sway/output.h"
#include "sway/server.h"
#include "sway/tree/container.h"
#include "sway/tree/view.h"
#include "sway/tree/workspace.h"

/*
 * No production link, no callback registration and no privileged IPC.
 * Observation of native `current` is NOT a compositor configure ACK,
 * transaction completion proof or presentation/settlement receipt. SwayPX
 * can copy current state on a transaction TIMEOUT in transaction_progress.
 */
bool timew_scope_a_native_txn_snapshot_read(
		struct sway_view *view,
		struct timew_scope_a_native_txn_snapshot *out) {
	if (!view || !out || view->destroying ||
			!view->surface || !view->surface->mapped ||
			!view->surface->resource ||
			!view->container ||
			view->container->view != view ||
			view->type != SWAY_VIEW_XDG_SHELL ||
			!view->wlr_xdg_toplevel ||
			!view->wlr_xdg_toplevel->base ||
			view->wlr_xdg_toplevel->base->surface != view->surface ||
			server.session_lock.lock) {
		return false;
	}
	struct sway_container *con = view->container;
	struct sway_workspace *pending_ws = con->pending.workspace;
	struct sway_workspace *current_ws = con->current.workspace;
	if (con->node.destroying || !pending_ws || !current_ws ||
			pending_ws != current_ws ||
			!pending_ws->output ||
			!current_ws->output ||
			pending_ws->output != current_ws->output ||
			con->pending.parent || con->current.parent ||
			!container_is_floating(con) ||
			con->scratchpad ||
			con->pending.fullscreen_mode != FULLSCREEN_NONE ||
			con->current.fullscreen_mode != FULLSCREEN_NONE) {
		return false;
	}
	struct sway_output *output = pending_ws->output;
	if (!output->enabled || !output->wlr_output ||
			!output->wlr_output->enabled ||
			output->width <= 0 || output->height <= 0) {
		return false;
	}
	const double scale = output->wlr_output->scale;
	if (!isfinite(scale) || scale <= 0.0) {
		return false;
	}
	struct timew_scope_a_native_txn_snapshot next = {0};
	if (!timew_scope_a_exact_geometry_capture(&next.pending,
			TIMEW_SCOPE_A_GEOMETRY_PENDING,
			con->pending.x, con->pending.y,
			con->pending.width, con->pending.height) ||
			!timew_scope_a_exact_geometry_capture(&next.current,
			TIMEW_SCOPE_A_GEOMETRY_CURRENT,
			con->current.x, con->current.y,
			con->current.width, con->current.height)) {
		return false;
	}
	next.native_container_ref = (uintptr_t)con;
	next.native_root_surface_ref = (uintptr_t)view->surface->resource;
	next.pending_workspace_ref = (uintptr_t)pending_ws;
	next.current_workspace_ref = (uintptr_t)current_ws;
	next.native_output_ref = (uintptr_t)output;
	next.output_lx = output->lx;
	next.output_ly = output->ly;
	next.output_transformed_width = output->width;
	next.output_transformed_height = output->height;
	next.output_transform = (uint32_t)output->wlr_output->transform;
	memcpy(&next.output_scale_binary64_bits, &scale, sizeof(scale));
	next.pending_transaction_present = server.pending_transaction != NULL;
	next.queued_transaction_present = server.queued_transaction != NULL;
	next.current_layout_value_observed = true;
	/* All authority/proof flags remain false by zero initialization.
	 * Neither native ref nor transform/scale is independently enrolled.
	 * No assumption that current is painted or visible on the output. */
	*out = next;
	return true;
}
