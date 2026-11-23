#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "sway/timew_scope_a_txn_stage.h"

static unsigned cases;

static void no_authority(struct timew_scope_a_txn_assessment a) {
	assert(!a.selected_surface_authenticated);
	assert(!a.configure_ack_authenticated);
	assert(!a.compositor_receipt_authenticated);
	assert(!a.geometry_settled_verified);
	assert(!a.native_resize_allowed);
	assert(!a.native_move_allowed);
	assert(!a.retry_allowed);
	assert(!a.undo_allowed);
	++cases;
}

static bool send(struct timew_scope_a_txn_trace *t,
		enum timew_scope_a_txn_event event) {
	return timew_scope_a_txn_trace_event(t, 7, 10, 22, 8, event);
}

static struct timew_scope_a_txn_trace begin(void) {
	struct timew_scope_a_txn_trace t = {0};
	assert(timew_scope_a_txn_trace_begin(&t, 7, 10, 22, 8));
	assert(t.stage == TIMEW_SCOPE_A_TXN_QUEUED);
	assert(!timew_scope_a_txn_trace_begin(&t, 7, 10, 22, 8));
	no_authority(timew_scope_a_txn_trace_assess(&t));
	return t;
}

static void test_normal_is_still_not_settled(void) {
	struct timew_scope_a_txn_trace t = begin();
	assert(send(&t, TIMEW_SCOPE_A_TXN_EVENT_READY));
	assert(t.stage == TIMEW_SCOPE_A_TXN_READY_REPORTED);
	assert(!timew_scope_a_txn_trace_assess(&t).current_value_observed_only);
	no_authority(timew_scope_a_txn_trace_assess(&t));
	assert(send(&t, TIMEW_SCOPE_A_TXN_EVENT_APPLY));
	assert(t.stage == TIMEW_SCOPE_A_TXN_CURRENT_COPIED);
	assert(timew_scope_a_txn_trace_assess(&t).current_value_observed_only);
	no_authority(timew_scope_a_txn_trace_assess(&t));
	assert(send(&t, TIMEW_SCOPE_A_TXN_EVENT_ARRANGE));
	assert(t.stage == TIMEW_SCOPE_A_TXN_SCENE_ARRANGED);
	no_authority(timew_scope_a_txn_trace_assess(&t));
	assert(send(&t, TIMEW_SCOPE_A_TXN_EVENT_PRESENT));
	assert(t.stage == TIMEW_SCOPE_A_TXN_PRESENT_REPORTED);
	/* Even a caller-supplied PRESENT label is not a trusted settled frame. */
	assert(!timew_scope_a_txn_trace_assess(&t).timeout_or_revocation_hold);
	no_authority(timew_scope_a_txn_trace_assess(&t));
	assert(!send(&t, TIMEW_SCOPE_A_TXN_EVENT_PRESENT));
	assert(t.stage == TIMEW_SCOPE_A_TXN_HOLD);
	no_authority(timew_scope_a_txn_trace_assess(&t));
}

static void test_timeout_applies_but_never_acknowledges(void) {
	struct timew_scope_a_txn_trace t = begin();
	assert(send(&t, TIMEW_SCOPE_A_TXN_EVENT_TIMEOUT));
	assert(t.timeout_seen);
	assert(t.stage == TIMEW_SCOPE_A_TXN_TIMEOUT_FORCED);
	assert(!timew_scope_a_txn_trace_assess(&t).current_value_observed_only);
	assert(send(&t, TIMEW_SCOPE_A_TXN_EVENT_APPLY));
	assert(t.stage == TIMEW_SCOPE_A_TXN_CURRENT_COPIED);
	assert(send(&t, TIMEW_SCOPE_A_TXN_EVENT_ARRANGE));
	assert(send(&t, TIMEW_SCOPE_A_TXN_EVENT_PRESENT));
	struct timew_scope_a_txn_assessment result =
		timew_scope_a_txn_trace_assess(&t);
	assert(result.current_value_observed_only);
	assert(result.timeout_or_revocation_hold);
	no_authority(result);
	/* A late configure ACK cannot turn a timeout into normal readiness. */
	assert(!send(&t, TIMEW_SCOPE_A_TXN_EVENT_READY));
	assert(t.stage == TIMEW_SCOPE_A_TXN_HOLD);
	no_authority(timew_scope_a_txn_trace_assess(&t));
}

static void test_lost_and_wrong_order_events_hold(void) {
	const enum timew_scope_a_txn_event invalid[] = {
		TIMEW_SCOPE_A_TXN_EVENT_APPLY,
		TIMEW_SCOPE_A_TXN_EVENT_ARRANGE,
		TIMEW_SCOPE_A_TXN_EVENT_PRESENT,
		(enum timew_scope_a_txn_event)99,
	};
	for (unsigned i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
		struct timew_scope_a_txn_trace t = begin();
		assert(!send(&t, invalid[i]));
		assert(t.stage == TIMEW_SCOPE_A_TXN_HOLD);
		assert(timew_scope_a_txn_trace_assess(&t).timeout_or_revocation_hold);
		no_authority(timew_scope_a_txn_trace_assess(&t));
	}
}

static void test_stale_generation_and_output_epoch(void) {
	struct timew_scope_a_txn_trace t = begin();
	assert(!timew_scope_a_txn_trace_event(&t,
		7, 9, 22, 8, TIMEW_SCOPE_A_TXN_EVENT_TIMEOUT));
	assert(!timew_scope_a_txn_trace_event(&t,
		8, 10, 22, 8, TIMEW_SCOPE_A_TXN_EVENT_READY));
	assert(!timew_scope_a_txn_trace_event(&t,
		7, 10, 23, 8, TIMEW_SCOPE_A_TXN_EVENT_READY));
	assert(!timew_scope_a_txn_trace_event(&t,
		7, 10, 22, 9, TIMEW_SCOPE_A_TXN_EVENT_PRESENT));
	assert(t.stage == TIMEW_SCOPE_A_TXN_QUEUED);
	/* A stale callback cannot revoke a newer identity. */
	assert(!timew_scope_a_txn_trace_event(&t,
		7, 9, 22, 8, TIMEW_SCOPE_A_TXN_EVENT_REVOKE));
	assert(send(&t, TIMEW_SCOPE_A_TXN_EVENT_REVOKE));
	assert(t.revoked && t.stage == TIMEW_SCOPE_A_TXN_HOLD);
	assert(!send(&t, TIMEW_SCOPE_A_TXN_EVENT_READY));
	no_authority(timew_scope_a_txn_trace_assess(&t));
}

static void test_invalid_inputs(void) {
	struct timew_scope_a_txn_trace t = {0};
	assert(!timew_scope_a_txn_trace_begin(NULL, 7, 10, 22, 8));
	assert(!timew_scope_a_txn_trace_begin(&t, 0, 10, 22, 8));
	assert(!timew_scope_a_txn_trace_begin(&t, 7, 0, 22, 8));
	assert(!timew_scope_a_txn_trace_begin(&t, 7, 10, 0, 8));
	assert(!timew_scope_a_txn_trace_begin(&t, 7, 10, 22, 0));
	assert(!send(&t, TIMEW_SCOPE_A_TXN_EVENT_READY));
	no_authority(timew_scope_a_txn_trace_assess(NULL));
	no_authority(timew_scope_a_txn_trace_assess(&t));
}

int main(void) {
	test_normal_is_still_not_settled();
	test_timeout_applies_but_never_acknowledges();
	test_lost_and_wrong_order_events_hold();
	test_stale_generation_and_output_epoch();
	test_invalid_inputs();
	printf("TIMEW_SCOPE_A_TRANSACTION_STAGE PASS: %u fail-closed "
		"assessments; timed-out applied layout and all reported stages "
		"grant NO source/ack/settled/geometry/retry authority\n", cases);
	return 0;
}
