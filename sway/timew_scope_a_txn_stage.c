#include <stddef.h>

#include "sway/timew_scope_a_txn_stage.h"

static bool valid_id(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
	return a && b && c && d;
}

bool timew_scope_a_txn_trace_begin(struct timew_scope_a_txn_trace *t,
		uint64_t compositor_epoch, uint64_t selected_generation,
		uint64_t transaction_generation, uint64_t output_epoch) {
	if (!t || t->stage != TIMEW_SCOPE_A_TXN_EMPTY ||
			!valid_id(compositor_epoch, selected_generation,
				transaction_generation, output_epoch)) {
		return false;
	}
	*t = (struct timew_scope_a_txn_trace) {
		.compositor_epoch = compositor_epoch,
		.selected_generation = selected_generation,
		.transaction_generation = transaction_generation,
		.source_output_epoch = output_epoch,
		.stage = TIMEW_SCOPE_A_TXN_QUEUED,
	};
	return true;
}

bool timew_scope_a_txn_trace_event(struct timew_scope_a_txn_trace *t,
		uint64_t compositor_epoch, uint64_t selected_generation,
		uint64_t transaction_generation, uint64_t output_epoch,
		enum timew_scope_a_txn_event event) {
	if (!t || t->stage == TIMEW_SCOPE_A_TXN_EMPTY ||
			t->stage == TIMEW_SCOPE_A_TXN_HOLD ||
			!valid_id(compositor_epoch, selected_generation,
				transaction_generation, output_epoch)) {
		return false;
	}
	/* An old event for another transaction/generation must NEVER advance
	 * or cancel a newer trace; the adapter must separately revoke on real
	 * output/seat/lifetime drift. */
	if (t->compositor_epoch != compositor_epoch ||
			t->selected_generation != selected_generation ||
			t->transaction_generation != transaction_generation ||
			t->source_output_epoch != output_epoch) {
		return false;
	}
	if (event == TIMEW_SCOPE_A_TXN_EVENT_REVOKE) {
		t->revoked = true;
		t->stage = TIMEW_SCOPE_A_TXN_HOLD;
		return true;
	}
	switch (event) {
	case TIMEW_SCOPE_A_TXN_EVENT_READY:
		if (t->stage == TIMEW_SCOPE_A_TXN_QUEUED) {
			t->stage = TIMEW_SCOPE_A_TXN_READY_REPORTED;
			return true;
		}
		break;
	case TIMEW_SCOPE_A_TXN_EVENT_TIMEOUT:
		if (t->stage == TIMEW_SCOPE_A_TXN_QUEUED) {
			t->timeout_seen = true;
			t->stage = TIMEW_SCOPE_A_TXN_TIMEOUT_FORCED;
			return true;
		}
		break;
	case TIMEW_SCOPE_A_TXN_EVENT_APPLY:
		if (t->stage == TIMEW_SCOPE_A_TXN_READY_REPORTED ||
				t->stage == TIMEW_SCOPE_A_TXN_TIMEOUT_FORCED) {
			t->stage = TIMEW_SCOPE_A_TXN_CURRENT_COPIED;
			return true;
		}
		break;
	case TIMEW_SCOPE_A_TXN_EVENT_ARRANGE:
		if (t->stage == TIMEW_SCOPE_A_TXN_CURRENT_COPIED) {
			t->stage = TIMEW_SCOPE_A_TXN_SCENE_ARRANGED;
			return true;
		}
		break;
	case TIMEW_SCOPE_A_TXN_EVENT_PRESENT:
		if (t->stage == TIMEW_SCOPE_A_TXN_SCENE_ARRANGED) {
			t->stage = TIMEW_SCOPE_A_TXN_PRESENT_REPORTED;
			return true;
		}
		break;
	case TIMEW_SCOPE_A_TXN_EVENT_REVOKE:
		break;
	}
	/* Wrong ordering creates an ambiguity, NEVER an implicit ACK. */
	t->stage = TIMEW_SCOPE_A_TXN_HOLD;
	return false;
}

struct timew_scope_a_txn_assessment timew_scope_a_txn_trace_assess(
		const struct timew_scope_a_txn_trace *t) {
	enum timew_scope_a_txn_stage stage =
		t ? t->stage : TIMEW_SCOPE_A_TXN_EMPTY;
	return (struct timew_scope_a_txn_assessment) {
		.stage = stage,
		.timeout_or_revocation_hold =
			!t || t->timeout_seen || t->revoked ||
			stage == TIMEW_SCOPE_A_TXN_EMPTY ||
			stage == TIMEW_SCOPE_A_TXN_HOLD,
		.current_value_observed_only =
			t && (stage == TIMEW_SCOPE_A_TXN_CURRENT_COPIED ||
				stage == TIMEW_SCOPE_A_TXN_SCENE_ARRANGED ||
				stage == TIMEW_SCOPE_A_TXN_PRESENT_REPORTED),
		.selected_surface_authenticated = false,
		.configure_ack_authenticated = false,
		.compositor_receipt_authenticated = false,
		.geometry_settled_verified = false,
		.native_resize_allowed = false,
		.native_move_allowed = false,
		.retry_allowed = false,
		.undo_allowed = false,
	};
}
