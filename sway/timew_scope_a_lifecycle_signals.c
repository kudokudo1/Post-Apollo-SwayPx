#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <string.h>

#include "sway/timew_scope_a_lifecycle_signals.h"

/*
 * This object is NOT in production sway/meson.build. It listens only when
 * an in-process native test/adapter has explicitly attached its signals.
 * It does NOT establish a trusted selection/epoch or enable native actions.
 */
static bool still_our_generation(
		const struct timew_scope_a_lifecycle_signals *watch) {
	if (!watch || !watch->ledger ||
			watch->ledger->state != TIMEW_SCOPE_A_CANDIDATE) {
		return false;
	}
	const struct timew_scope_a_lifetime_observation *s =
		&watch->ledger->selected;
	return s->compositor_epoch == watch->compositor_epoch &&
		s->lifetime_generation == watch->lifetime_generation &&
		s->selection_nonce == watch->selection_nonce &&
		s->native_container_ref == watch->native_container_ref &&
		s->native_root_surface_ref == watch->native_root_surface_ref;
}

static void detach(struct timew_scope_a_lifecycle_signals *watch) {
	if (!watch || !watch->attached) {
		return;
	}
	/* Real SwayPX view/container destroy signals use mutable emission,
	 * permitting their listeners to unregister during the event. */
	watch->attached = false;
	wl_list_remove(&watch->view_unmap.link);
	wl_list_remove(&watch->container_destroy.link);
	wl_list_remove(&watch->root_surface_destroy.link);
	wl_list_init(&watch->view_unmap.link);
	wl_list_init(&watch->container_destroy.link);
	wl_list_init(&watch->root_surface_destroy.link);
	watch->ledger = NULL;
}

static void revoke_and_detach(struct timew_scope_a_lifecycle_signals *watch) {
	if (!watch || !watch->attached) {
		return;
	}
	/* A delayed event belonging to an older generation NEVER cancels
	 * a newer candidate that happens to reuse the same native address. */
	if (still_our_generation(watch)) {
		timew_scope_a_lifetime_revoke(watch->ledger);
	}
	detach(watch);
}

static void on_view_unmap(struct wl_listener *listener, void *data) {
	(void)data;
	struct timew_scope_a_lifecycle_signals *watch =
		wl_container_of(listener, watch, view_unmap);
	revoke_and_detach(watch);
}

static void on_container_destroy(struct wl_listener *listener, void *data) {
	(void)data;
	struct timew_scope_a_lifecycle_signals *watch =
		wl_container_of(listener, watch, container_destroy);
	revoke_and_detach(watch);
}

static void on_root_surface_destroy(struct wl_listener *listener, void *data) {
	(void)data;
	struct timew_scope_a_lifecycle_signals *watch =
		wl_container_of(listener, watch, root_surface_destroy);
	revoke_and_detach(watch);
}

void timew_scope_a_lifecycle_signals_init(
		struct timew_scope_a_lifecycle_signals *watch) {
	if (!watch) {
		return;
	}
	/* Only valid on an unbound or already detached watcher. */
	memset(watch, 0, sizeof(*watch));
	wl_list_init(&watch->view_unmap.link);
	wl_list_init(&watch->container_destroy.link);
	wl_list_init(&watch->root_surface_destroy.link);
}

bool timew_scope_a_lifecycle_signals_bind(
		struct timew_scope_a_lifecycle_signals *watch,
		struct timew_scope_a_lifetime_candidate *ledger,
		struct wl_signal *view_unmap, struct wl_signal *container_destroy,
		struct wl_signal *root_surface_destroy,
		uintptr_t native_container_ref, uintptr_t native_root_surface_ref) {
	if (!watch || watch->attached || !ledger ||
			ledger->state != TIMEW_SCOPE_A_CANDIDATE ||
			!view_unmap || !container_destroy || !root_surface_destroy ||
			view_unmap == container_destroy ||
			view_unmap == root_surface_destroy ||
			container_destroy == root_surface_destroy ||
			!native_container_ref || !native_root_surface_ref ||
			native_container_ref == native_root_surface_ref ||
			ledger->selected.native_container_ref != native_container_ref ||
			ledger->selected.native_root_surface_ref != native_root_surface_ref ||
			ledger->selected.compositor_epoch == 0 ||
			ledger->selected.lifetime_generation == 0 ||
			ledger->selected.selection_nonce == 0) {
		return false;
	}
	watch->ledger = ledger;
	watch->compositor_epoch = ledger->selected.compositor_epoch;
	watch->lifetime_generation = ledger->selected.lifetime_generation;
	watch->selection_nonce = ledger->selected.selection_nonce;
	watch->native_container_ref = native_container_ref;
	watch->native_root_surface_ref = native_root_surface_ref;
	watch->view_unmap.notify = on_view_unmap;
	watch->container_destroy.notify = on_container_destroy;
	watch->root_surface_destroy.notify = on_root_surface_destroy;
	wl_signal_add(view_unmap, &watch->view_unmap);
	wl_signal_add(container_destroy, &watch->container_destroy);
	wl_signal_add(root_surface_destroy, &watch->root_surface_destroy);
	watch->attached = true;
	return true;
}

void timew_scope_a_lifecycle_signals_release(
		struct timew_scope_a_lifecycle_signals *watch) {
	revoke_and_detach(watch);
}

static bool exact_i32(double value, int32_t *out) {
	if (!out || !isfinite(value) ||
			value < (double)INT32_MIN ||
			value > (double)INT32_MAX || trunc(value) != value) {
		return false;
	}
	*out = (int32_t)value;
	return true;
}

bool timew_scope_a_checked_pending_rect(
		double x, double y, double width, double height,
		int32_t *ix, int32_t *iy, int32_t *iwidth, int32_t *iheight) {
	int32_t cx, cy, cw, ch;
	if (!ix || !iy || !iwidth || !iheight ||
			!exact_i32(x, &cx) || !exact_i32(y, &cy) ||
			!exact_i32(width, &cw) || !exact_i32(height, &ch) ||
			cw <= 0 || ch <= 0) {
		return false;
	}
	*ix = cx;
	*iy = cy;
	*iwidth = cw;
	*iheight = ch;
	return true;
}
