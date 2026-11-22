#ifndef _SWAY_TIMEW_SCOPE_A_LIFECYCLE_SIGNALS_H
#define _SWAY_TIMEW_SCOPE_A_LIFECYCLE_SIGNALS_H

#include <stdbool.h>
#include <stdint.h>
#include <wayland-server-core.h>

#include "sway/timew_scope_a_lifetime_candidate.h"

/*
 * TimeW Scope A // unlinked, candidate-only real Wayland signal watchers.
 *
 * NO operator selection, input hook, signing, C5 spend, IPC, window resize,
 * window move, protected receipt or production linkage. Caller MUST own the
 * event-source object lifetimes and this watcher until detach/first event.
 * Attach only from in-process compositor code, never caller-provided IPC.
 *
 * A positive correlation is NEVER an authorization. The only state change
 * triggered by signals is revocation of the exact captured generation.
 */
struct timew_scope_a_lifecycle_signals {
	struct wl_listener view_unmap;
	struct wl_listener container_destroy;
	struct wl_listener root_surface_destroy;
	struct timew_scope_a_lifetime_candidate *ledger;
	uint64_t compositor_epoch;
	uint64_t lifetime_generation;
	uint64_t selection_nonce;
	uintptr_t native_container_ref;
	uintptr_t native_root_surface_ref;
	bool attached;
};

/* Initialize once before attaching. A live watcher cannot be reinitialized. */
void timew_scope_a_lifecycle_signals_init(
	struct timew_scope_a_lifecycle_signals *signals);

/*
 * Register on REAL Wayland signal objects supplied by an exact native
 * compositor adapter. Reject invalid signals, duplicate refs, missing
 * ledger, or already attached watcher. The epoch/generation/nonce are copied
 * from a CANDIDATE ONLY ledger and do not authenticate their issuer.
 */
bool timew_scope_a_lifecycle_signals_bind(
	struct timew_scope_a_lifecycle_signals *signals,
	struct timew_scope_a_lifetime_candidate *ledger,
	struct wl_signal *view_unmap, struct wl_signal *container_destroy,
	struct wl_signal *root_surface_destroy,
	uintptr_t native_container_ref, uintptr_t native_root_surface_ref);

/*
 * Cleanly detach. If this watcher still owns the exact candidate generation,
 * revoke it; a new candidate generation must NEVER be revoked by an old
 * callback. Safe to call again after the event-driven detach.
 */
void timew_scope_a_lifecycle_signals_release(
	struct timew_scope_a_lifecycle_signals *signals);

/*
 * Checked bridge between #169's pending double x/y/w/h and #168's int32.
 * Only finite EXACT integers in int32 range are accepted. No truncation,
 * rounding, unit conversion, or assumption that pending means settled.
 */
bool timew_scope_a_checked_pending_rect(
	double x, double y, double width, double height,
	int32_t *ix, int32_t *iy, int32_t *iwidth, int32_t *iheight);

#endif
