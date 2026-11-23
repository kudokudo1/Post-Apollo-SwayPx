#ifndef _SWAY_TIMEW_SCOPE_A_LIFETIME_CANDIDATE_H
#define _SWAY_TIMEW_SCOPE_A_LIFETIME_CANDIDATE_H

#include <stdbool.h>
#include <stdint.h>

/*
 * TimeW Scope A // source-only, candidate-only lifetime ledger.
 *
 * NOT LINKED INTO SWAYPX. This API can correlate synthetic snapshots in CI;
 * it does not authenticate an operator selection, physical input, native
 * Wayland object, compositor release, C3e/C4b issuer, C5 consent/spend, or
 * protected receipt. It cannot execute resize, move, IPC, or any native
 * action. Never expose it as an IPC command or feed it caller-supplied refs
 * as proof of an installed compositor/selection origin.
 *
 * A later, separately reviewed compositor adapter must source observations
 * from actual private live objects and independently approved input/seat
 * handling. The API intentionally provides NO positive authorization result.
 */

enum timew_scope_a_candidate_state {
	TIMEW_SCOPE_A_EMPTY = 0,
	TIMEW_SCOPE_A_CANDIDATE,
	TIMEW_SCOPE_A_REVOKED,
};

struct timew_scope_a_lifetime_observation {
	uint64_t compositor_epoch;
	uint64_t lifetime_generation;
	uint64_t selection_nonce;
	uint64_t seat_epoch;
	uint64_t session_epoch;
	uint64_t output_epoch;
	uint64_t workspace_epoch;

	/* CI-only opaque values, not dereferenced or authenticated pointers. */
	uintptr_t native_container_ref;
	uintptr_t native_root_surface_ref;
	uintptr_t native_seat_ref;
	uintptr_t native_output_ref;
	uintptr_t native_workspace_ref;

	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
	bool mapped;
	bool exact_xdg_toplevel;
	bool floating;
	bool fullscreen;
	bool scratchpad;
	bool session_locked;
	bool destroying;
};

struct timew_scope_a_lifetime_candidate {
	enum timew_scope_a_candidate_state state;
	/* Retained on revocation. Never accept a reused identity in one epoch. */
	uint64_t last_epoch;
	uint64_t last_generation;
	struct timew_scope_a_lifetime_observation selected;
};

enum timew_scope_a_probe_state {
	TIMEW_SCOPE_A_PROBE_NO_CANDIDATE = 0,
	TIMEW_SCOPE_A_PROBE_MATCHED_CANDIDATE_ONLY,
	TIMEW_SCOPE_A_PROBE_REVOKED,
};

struct timew_scope_a_candidate_probe {
	enum timew_scope_a_probe_state state;
	bool candidate_correlated; /* Not physical selection or trusted origin. */
	bool operator_selection_authenticated;
	bool producer_verified;
	bool c5_spend_verified;
	bool native_resize_allowed;
	bool native_move_allowed;
	bool protected_receipt_verified;
};

void timew_scope_a_lifetime_init(struct timew_scope_a_lifetime_candidate *candidate);

/*
 * Reserve at most one synthetic candidate. Enforces monotonically fresh
 * (compositor_epoch, lifetime_generation) within this ledger lifetime.
 * No call here marks selection authenticated or C5 spend confirmed.
 */
bool timew_scope_a_lifetime_reserve(
	struct timew_scope_a_lifetime_candidate *candidate,
	const struct timew_scope_a_lifetime_observation *observation);

/*
 * Every probe compares the complete scoped snapshot. Any missing or changed
 * observation permanently revokes this candidate, including a geometry
 * change that is later reversed. A subsequent attempt needs a fresh identity.
 * Probes can correlate a candidate, but NEVER grant native action authority.
 */
struct timew_scope_a_candidate_probe timew_scope_a_lifetime_probe(
	struct timew_scope_a_lifetime_candidate *candidate,
	const struct timew_scope_a_lifetime_observation *observation);

/* Synchronous fail-closed cancellation; no native action is performed. */
bool timew_scope_a_lifetime_revoke(
	struct timew_scope_a_lifetime_candidate *candidate);

/*
 * Event-driven loss from a compositor-owned callback. Match only this epoch
 * and one of its exact native ref values. No pointer dereference. This must
 * be wired to actual destroy/unmap/seat/output callbacks only after review.
 */
bool timew_scope_a_lifetime_native_ref_lost(
	struct timew_scope_a_lifetime_candidate *candidate,
	uint64_t compositor_epoch, uintptr_t native_ref);

#endif
