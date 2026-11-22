#include <string.h>

#include "sway/timew_scope_a_lifetime_candidate.h"

/*
 * Pure, unlinked CI candidate-only state logic. No production compositor
 * callback, selection origin, seat enrollment, native credential, signer,
 * grant spend, protected command or receipt is present in this file.
 */
static bool valid_observation(
		const struct timew_scope_a_lifetime_observation *o) {
	return o &&
		o->compositor_epoch != 0 &&
		o->lifetime_generation != 0 &&
		o->selection_nonce != 0 &&
		o->seat_epoch != 0 &&
		o->session_epoch != 0 &&
		o->output_epoch != 0 &&
		o->workspace_epoch != 0 &&
		o->native_container_ref != 0 &&
		o->native_root_surface_ref != 0 &&
		o->native_container_ref != o->native_root_surface_ref &&
		o->native_seat_ref != 0 &&
		o->native_output_ref != 0 &&
		o->native_workspace_ref != 0 &&
		o->width > 0 && o->height > 0 &&
		o->mapped && o->exact_xdg_toplevel &&
		o->floating && !o->fullscreen &&
		!o->scratchpad && !o->session_locked &&
		!o->destroying;
}

static bool same_observation(
		const struct timew_scope_a_lifetime_observation *a,
		const struct timew_scope_a_lifetime_observation *b) {
	return valid_observation(a) && valid_observation(b) &&
		a->compositor_epoch == b->compositor_epoch &&
		a->lifetime_generation == b->lifetime_generation &&
		a->selection_nonce == b->selection_nonce &&
		a->seat_epoch == b->seat_epoch &&
		a->session_epoch == b->session_epoch &&
		a->output_epoch == b->output_epoch &&
		a->workspace_epoch == b->workspace_epoch &&
		a->native_container_ref == b->native_container_ref &&
		a->native_root_surface_ref == b->native_root_surface_ref &&
		a->native_seat_ref == b->native_seat_ref &&
		a->native_output_ref == b->native_output_ref &&
		a->native_workspace_ref == b->native_workspace_ref &&
		a->x == b->x && a->y == b->y &&
		a->width == b->width && a->height == b->height &&
		a->mapped == b->mapped &&
		a->exact_xdg_toplevel == b->exact_xdg_toplevel &&
		a->floating == b->floating &&
		a->fullscreen == b->fullscreen &&
		a->scratchpad == b->scratchpad &&
		a->session_locked == b->session_locked &&
		a->destroying == b->destroying;
}

static struct timew_scope_a_candidate_probe result(
		enum timew_scope_a_probe_state state) {
	return (struct timew_scope_a_candidate_probe) {
		.state = state,
		.candidate_correlated =
			state == TIMEW_SCOPE_A_PROBE_MATCHED_CANDIDATE_ONLY,
		.operator_selection_authenticated = false,
		.producer_verified = false,
		.c5_spend_verified = false,
		.native_resize_allowed = false,
		.native_move_allowed = false,
		.protected_receipt_verified = false,
	};
}

void timew_scope_a_lifetime_init(
		struct timew_scope_a_lifetime_candidate *candidate) {
	if (!candidate) {
		return;
	}
	memset(candidate, 0, sizeof(*candidate));
	candidate->state = TIMEW_SCOPE_A_EMPTY;
}

bool timew_scope_a_lifetime_revoke(
		struct timew_scope_a_lifetime_candidate *candidate) {
	if (!candidate || candidate->state != TIMEW_SCOPE_A_CANDIDATE) {
		return false;
	}
	/* Retain the epoch/generation floor across cancellation. */
	memset(&candidate->selected, 0, sizeof(candidate->selected));
	candidate->state = TIMEW_SCOPE_A_REVOKED;
	return true;
}

bool timew_scope_a_lifetime_reserve(
		struct timew_scope_a_lifetime_candidate *candidate,
		const struct timew_scope_a_lifetime_observation *observation) {
	if (!candidate || !valid_observation(observation) ||
			candidate->state == TIMEW_SCOPE_A_CANDIDATE ||
			(candidate->state != TIMEW_SCOPE_A_EMPTY &&
			 candidate->state != TIMEW_SCOPE_A_REVOKED) ||
			observation->compositor_epoch < candidate->last_epoch ||
			(observation->compositor_epoch == candidate->last_epoch &&
			 observation->lifetime_generation <= candidate->last_generation)) {
		return false;
	}

	candidate->selected = *observation;
	candidate->last_epoch = observation->compositor_epoch;
	candidate->last_generation = observation->lifetime_generation;
	candidate->state = TIMEW_SCOPE_A_CANDIDATE;
	return true;
}

struct timew_scope_a_candidate_probe timew_scope_a_lifetime_probe(
		struct timew_scope_a_lifetime_candidate *candidate,
		const struct timew_scope_a_lifetime_observation *observation) {
	if (!candidate || candidate->state == TIMEW_SCOPE_A_EMPTY) {
		return result(TIMEW_SCOPE_A_PROBE_NO_CANDIDATE);
	}
	if (candidate->state != TIMEW_SCOPE_A_CANDIDATE) {
		return result(TIMEW_SCOPE_A_PROBE_REVOKED);
	}

	if (!same_observation(&candidate->selected, observation)) {
		timew_scope_a_lifetime_revoke(candidate);
		return result(TIMEW_SCOPE_A_PROBE_REVOKED);
	}

	/*
	 * A matching pointer/epoch/rectangle is CI correlation only. A future
	 * protected compositor adapter must independently verify live native
	 * identity and authorizations at the actual mutation critical section.
	 */
	return result(TIMEW_SCOPE_A_PROBE_MATCHED_CANDIDATE_ONLY);
}

bool timew_scope_a_lifetime_native_ref_lost(
		struct timew_scope_a_lifetime_candidate *candidate,
		uint64_t compositor_epoch, uintptr_t native_ref) {
	if (!candidate || candidate->state != TIMEW_SCOPE_A_CANDIDATE ||
			!compositor_epoch || !native_ref ||
			candidate->selected.compositor_epoch != compositor_epoch) {
		return false;
	}
	const struct timew_scope_a_lifetime_observation *o =
		&candidate->selected;
	if (native_ref != o->native_container_ref &&
			native_ref != o->native_root_surface_ref &&
			native_ref != o->native_seat_ref &&
			native_ref != o->native_output_ref &&
			native_ref != o->native_workspace_ref) {
		return false;
	}
	return timew_scope_a_lifetime_revoke(candidate);
}
