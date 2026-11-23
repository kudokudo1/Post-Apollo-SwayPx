#ifndef _SWAY_TIMEW_SCOPE_A_NATIVE_OBSERVER_H
#define _SWAY_TIMEW_SCOPE_A_NATIVE_OBSERVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * TimeW Scope A // compositor-native, READ-ONLY candidate observation.
 *
 * This interface is not registered on an input handler and is NOT built into
 * production SwayPX. It does NOT arm an explicit selection gesture, mint a
 * compositor epoch/generation, create a protected credential, issue C5
 * consent, accept a protected IPC command, resize, move or send a receipt.
 *
 * Returned pointers/IDs are private diagnostics for a future independently
 * reviewed compositor adapter; no client may use them as a trust anchor.
 * A candidate may only be inspected inside a live compositor callback.
 */
struct sway_seat;

struct timew_scope_a_native_candidate {
	bool candidate_observed;
	bool operator_selection_authenticated;
	bool c3e_producer_authenticated;
	bool c5_spend_verified;
	bool native_resize_allowed;
	bool native_move_allowed;
	bool protected_receipt_verified;

	uintptr_t native_container_ref;
	uintptr_t native_root_surface_ref;
	uintptr_t native_client_ref;
	uintptr_t native_seat_ref;
	uintptr_t native_output_ref;
	uintptr_t native_workspace_ref;
	size_t transient_con_id;

	/* Exact current compositor pending rectangle: no x/y assumption. */
	double x;
	double y;
	double width;
	double height;
};

/*
 * Inspect the actual current compositor pointer hit-test. Only an already
 * mapped, exact XDG toplevel at the seat's cursor, with a floating,
 * non-fullscreen and non-scratchpad top-level container, is a candidate.
 *
 * Do NOT call this function merely because a window gained focus. It
 * authenticates neither the physical origin of input nor the operator's
 * TimeW selection intent. It never passes itself into the candidate ledger.
 */
struct timew_scope_a_native_candidate
timew_scope_a_observe_pointer_candidate(struct sway_seat *seat);

#endif
