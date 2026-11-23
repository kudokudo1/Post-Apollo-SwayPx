#ifndef _SWAY_TIMEW_SCOPE_A_TXN_STAGE_H
#define _SWAY_TIMEW_SCOPE_A_TXN_STAGE_H

#include <stdbool.h>
#include <stdint.h>

/*
 * CI-only TimeW Scope A transaction-stage correlation.
 *
 * This source does NOT integrate with production SwayPX, authenticate real
 * Wayland configure ACKs, emit a native receipt, select a surface, spend C5,
 * or authorize a resize/move. Stage events are caller-supplied test inputs.
 * The most advanced result is diagnostic state, NEVER a verified placement.
 */
enum timew_scope_a_txn_stage {
	TIMEW_SCOPE_A_TXN_EMPTY = 0,
	TIMEW_SCOPE_A_TXN_QUEUED,
	TIMEW_SCOPE_A_TXN_READY_REPORTED,
	TIMEW_SCOPE_A_TXN_TIMEOUT_FORCED,
	TIMEW_SCOPE_A_TXN_CURRENT_COPIED,
	TIMEW_SCOPE_A_TXN_SCENE_ARRANGED,
	TIMEW_SCOPE_A_TXN_PRESENT_REPORTED,
	TIMEW_SCOPE_A_TXN_HOLD,
};

enum timew_scope_a_txn_event {
	TIMEW_SCOPE_A_TXN_EVENT_READY = 1,
	TIMEW_SCOPE_A_TXN_EVENT_TIMEOUT,
	TIMEW_SCOPE_A_TXN_EVENT_APPLY,
	TIMEW_SCOPE_A_TXN_EVENT_ARRANGE,
	TIMEW_SCOPE_A_TXN_EVENT_PRESENT,
	TIMEW_SCOPE_A_TXN_EVENT_REVOKE,
};

struct timew_scope_a_txn_trace {
	uint64_t compositor_epoch;
	uint64_t selected_generation;
	uint64_t transaction_generation;
	uint64_t source_output_epoch;
	enum timew_scope_a_txn_stage stage;
	bool timeout_seen;
	bool revoked;
};

/* No positive placement, source, operation, retry, or receipt flags exist. */
struct timew_scope_a_txn_assessment {
	enum timew_scope_a_txn_stage stage;
	bool timeout_or_revocation_hold;
	bool current_value_observed_only;
	bool selected_surface_authenticated;
	bool configure_ack_authenticated;
	bool compositor_receipt_authenticated;
	bool geometry_settled_verified;
	bool native_resize_allowed;
	bool native_move_allowed;
	bool retry_allowed;
	bool undo_allowed;
};

/* The values here are synthetic in CI; NOT compositor-authenticated IDs. */
bool timew_scope_a_txn_trace_begin(struct timew_scope_a_txn_trace *trace,
	uint64_t compositor_epoch, uint64_t selected_generation,
	uint64_t transaction_generation, uint64_t output_epoch);

/* Exact epoch/generation/output correlation required on every event. */
bool timew_scope_a_txn_trace_event(struct timew_scope_a_txn_trace *trace,
	uint64_t compositor_epoch, uint64_t selected_generation,
	uint64_t transaction_generation, uint64_t output_epoch,
	enum timew_scope_a_txn_event event);

struct timew_scope_a_txn_assessment timew_scope_a_txn_trace_assess(
	const struct timew_scope_a_txn_trace *trace);

#endif
