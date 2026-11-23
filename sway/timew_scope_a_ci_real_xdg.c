#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "sway/timew_scope_a_ci_real_xdg.h"
#include "sway/timew_scope_a_lifetime_candidate.h"
#include "sway/timew_scope_a_lifecycle_signals.h"
#include "sway/input/input-manager.h"
#include "sway/input/seat.h"
#include "sway/tree/container.h"
#include "sway/tree/view.h"
#include "sway/tree/workspace.h"

/*
 * Real mapped-XDG callback harness, compiled ONLY in a disposable GitHub
 * Actions SwayPX process with CI-specific source injection into view.c.
 * The app_id is a disposable-test FILTER ONLY, not an identity or grant.
 * Fixed epochs and monotonically incremented generations here are test
 * fixtures, NOT authenticated lifetime identities.
 */
static struct timew_scope_a_lifetime_candidate ci_ledger;
static struct timew_scope_a_lifecycle_signals ci_watch;
static struct sway_view *ci_view;
static uint64_t ci_generation;
static bool ci_initialized;

static void fail(const char *why) {
	fprintf(stderr, "TIMEW_CI_REAL_XDG_FAIL reason=%s\n", why);
	fflush(stderr);
	abort(); /* Disposable runner only; no production linkage. */
}

void timew_scope_a_ci_real_xdg_after_map(struct sway_view *view) {
	if (!view || view->type != SWAY_VIEW_XDG_SHELL) {
		return;
	}
	const char *app_id = view_get_app_id(view);
	if (!app_id || strcmp(app_id, "post-apollo-weather-news") != 0) {
		return;
	}
	if (!ci_initialized) {
		timew_scope_a_lifetime_init(&ci_ledger);
		timew_scope_a_lifecycle_signals_init(&ci_watch);
		ci_initialized = true;
	}
	if (ci_view == view && ci_watch.attached) {
		/* Later valid XDG geometry updates do not reissue a lifetime. */
		return;
	}
	if (ci_watch.attached || ci_view) {
		fail("overlapping_candidate");
	}
	struct sway_container *con = view->container;
	struct sway_workspace *ws = con ? con->pending.workspace : NULL;
	struct sway_seat *seat = input_manager_current_seat();
	if (!view->surface || !view->surface->resource ||
			!view->surface->mapped ||
			!view->wlr_xdg_toplevel ||
			!view->wlr_xdg_toplevel->base ||
			view->wlr_xdg_toplevel->base->surface != view->surface ||
			!con || con->view != view || !ws || !ws->output ||
			!seat || !seat->wlr_seat ||
			!container_is_floating(con) ||
			con->pending.fullscreen_mode != FULLSCREEN_NONE ||
			con->scratchpad || con->node.destroying) {
		fail("ineligible_real_mapped_xdg");
	}
	int32_t x, y, width, height;
	if (!isfinite(con->pending.x) || !isfinite(con->pending.y) ||
			!isfinite(con->pending.width) ||
			!isfinite(con->pending.height) ||
			con->pending.width <= 0 || con->pending.height <= 0) {
		/* Do not reserve on absent or nonfinite real native geometry. */
		fprintf(stderr, "TIMEW_CI_REAL_XDG_DEFER unusable_native_geometry=1\n");
		fflush(stderr);
		return;
	}
	const bool geometry_eligible = timew_scope_a_checked_pending_rect(
		con->pending.x, con->pending.y, con->pending.width,
		con->pending.height, &x, &y, &width, &height);
	if (!geometry_eligible) {
		/* Actual SwayPX centered floating windows can have half-pixel
		 * pending x/y (e.g. y=92.5). This is a REAL geometry HOLD, not
		 * permission to truncate. This CI fixture tests only native
		 * UNMAP/DESTROY signal delivery and exact object references.
		 * A synthetic 1x1 ledger rectangle is NEVER a geometry witness.
		 * The production native adapter still rejects this geometry. */
		fprintf(stderr,
			"TIMEW_CI_REAL_XDG_GEOMETRY_HOLD pending=%g,%g %gx%g "
			"ledger_rect=synthetic_only protected_geometry=0\n",
			con->pending.x, con->pending.y,
			con->pending.width, con->pending.height);
		x = 0;
		y = 0;
		width = 1;
		height = 1;
		fflush(stderr);
	}
	/* Only the *disposable* in-process fixture creates these identities.
	 * They prove callback correlation, never input or producer provenance. */
	++ci_generation;
	const struct timew_scope_a_lifetime_observation candidate = {
		.compositor_epoch = 1,
		.lifetime_generation = ci_generation,
		.selection_nonce = ci_generation,
		.seat_epoch = 1,
		.session_epoch = 1,
		.output_epoch = 1,
		.workspace_epoch = 1,
		.native_container_ref = (uintptr_t)con,
		.native_root_surface_ref = (uintptr_t)view->surface->resource,
		.native_seat_ref = (uintptr_t)seat->wlr_seat,
		.native_output_ref = (uintptr_t)ws->output,
		.native_workspace_ref = (uintptr_t)ws,
		.x = x, .y = y, .width = width, .height = height,
		.mapped = true, .exact_xdg_toplevel = true,
		.floating = true,
	};
	if (!timew_scope_a_lifetime_reserve(&ci_ledger, &candidate)) {
		fail("ledger_reservation_refused");
	}
	if (!timew_scope_a_lifecycle_signals_bind(
			&ci_watch, &ci_ledger, &view->events.unmap,
			&con->node.events.destroy, &view->surface->events.destroy,
			(uintptr_t)con, (uintptr_t)view->surface->resource)) {
		fail("real_native_listener_bind_refused");
	}
	ci_view = view;
	fprintf(stderr,
		"TIMEW_CI_REAL_XDG_MAPPED gen=%" PRIu64
		" real_view=1 floating=1 listeners=3 authority=NONE\n",
		ci_generation);
	fflush(stderr);
}

void timew_scope_a_ci_real_xdg_after_unmap_signal(struct sway_view *view) {
	if (!ci_view || view != ci_view) {
		return;
	}
	/* Called immediately AFTER actual view->events.unmap emission in
	 * the disposable compositor and BEFORE container_begin_destroy.
	 * No native ref can be used as a future lifetime credential. */
	if (ci_watch.attached ||
			ci_ledger.state != TIMEW_SCOPE_A_REVOKED ||
			ci_ledger.selected.native_container_ref != 0 ||
			ci_ledger.selected.native_root_surface_ref != 0) {
		fail("unmap_failed_synchronous_revocation");
	}
	if (ci_ledger.last_epoch != 1 ||
			ci_ledger.last_generation != ci_generation) {
		fail("epoch_generation_floor_lost");
	}
	fprintf(stderr,
		"TIMEW_CI_REAL_XDG_UNMAPPED gen=%" PRIu64
		" revoked_before_container_destroy=1 listeners_detached=1"
		" native_resize=0 native_move=0 authority=NONE\n",
		ci_generation);
	ci_view = NULL;
	fflush(stderr);
}
