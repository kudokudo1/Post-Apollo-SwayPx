#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sway/timew_scope_a_observed_lineage.h"

static unsigned checks;

#define CHECK(x) do { assert(x); ++checks; } while (0)

static uint64_t bits(double n) {
	uint64_t raw;
	memcpy(&raw, &n, sizeof(raw));
	return raw;
}

static struct timew_scope_a_lineage_context fixture(void) {
	struct timew_scope_a_lineage_context c = {
		.native_view_ref = 0x10,
		.native_container_ref = 0x20,
		.native_root_resource_ref = 0x30,
		.native_client_ref = 0x40,
		.native_seat_ref = 0x50,
		.native_output_ref = 0x60,
		.native_workspace_ref = 0x70,
		.seat_epoch = 1,
		.session_epoch = 2,
		.output_epoch = 3,
		.workspace_epoch = 4,
		.output_scale_bits = bits(1.25),
		.output_transform = 1,
		.mapped = true,
		.exact_xdg_toplevel = true,
		.floating = true,
	};
	CHECK(timew_scope_a_exact_geometry_capture(&c.pending,
		TIMEW_SCOPE_A_GEOMETRY_PENDING, 259, 92.5, 1402, 872));
	return c;
}

static void no_authority(struct timew_scope_a_lineage_assessment a) {
	CHECK(!a.physical_selection_authenticated);
	CHECK(!a.native_object_origin_authenticated);
	CHECK(!a.native_transaction_ack_authenticated);
	CHECK(!a.source_enrolled && !a.c5_spent);
	CHECK(!a.scene_settled_verified);
	CHECK(!a.resize_allowed && !a.move_allowed);
	CHECK(!a.protected_receipt_verified);
	CHECK(!a.retry_allowed && !a.undo_allowed);
}

static struct timew_scope_a_lineage_issuer boot(void) {
	struct timew_scope_a_lineage_issuer i = {0};
	CHECK(timew_scope_a_lineage_boot(&i));
	CHECK(!timew_scope_a_lineage_boot(&i));
	CHECK(i.booted && !i.shut_down);
	CHECK(i.boot_a || i.boot_b);
	return i;
}

static void test_boot_and_stale_reuse(void) {
	struct timew_scope_a_lineage_issuer i = boot();
	struct timew_scope_a_lineage_issuer j = boot();
	CHECK(i.boot_a != j.boot_a || i.boot_b != j.boot_b);
	struct timew_scope_a_lineage_context c = fixture();
	struct timew_scope_a_lineage_tag old = {0};
	CHECK(timew_scope_a_lineage_begin(&i, &c, &old));
	CHECK(old.boot_a == i.boot_a && old.boot_b == i.boot_b);
	CHECK(old.lifetime_sequence == 1 && old.transaction_sequence == 0);
	CHECK(!timew_scope_a_lineage_begin(&i, &c, &old));
	struct timew_scope_a_lineage_assessment a =
		timew_scope_a_lineage_observe(&i, &old, &c);
	CHECK(a.candidate_correlated_only);
	CHECK(!a.transaction_correlated_only);
	no_authority(a);
	CHECK(timew_scope_a_lineage_revoke(&i, &old));
	CHECK(!i.live && !i.transaction_live);
	CHECK(i.lifetime_floor == 1);
	struct timew_scope_a_lineage_tag newer = {0};
	/* Same exact native refs and values are allowed only as a NEW lifetime.
	 * There is no claim of a physical gesture or a protected selected XDG. */
	CHECK(timew_scope_a_lineage_begin(&i, &c, &newer));
	CHECK(newer.lifetime_sequence == 2);
	CHECK(!timew_scope_a_lineage_revoke(&i, &old));
	CHECK(!timew_scope_a_lineage_observe(&i, &old, &c).candidate_correlated_only);
	CHECK(timew_scope_a_lineage_observe(&i, &newer, &c).candidate_correlated_only);
	struct timew_scope_a_lineage_tag from_other = newer;
	from_other.boot_a = j.boot_a;
	from_other.boot_b = j.boot_b;
	CHECK(!timew_scope_a_lineage_revoke(&i, &from_other));
	CHECK(i.live);
	timew_scope_a_lineage_shutdown(&i);
	CHECK(i.shut_down && !i.live && i.boot_a == 0 && i.boot_b == 0);
	CHECK(!timew_scope_a_lineage_boot(&i));
	CHECK(!timew_scope_a_lineage_begin(&i, &c, &old));
	timew_scope_a_lineage_shutdown(&i);
	timew_scope_a_lineage_shutdown(&j);
}

static void test_transaction_and_revocation(void) {
	struct timew_scope_a_lineage_issuer i = boot();
	struct timew_scope_a_lineage_context c = fixture();
	struct timew_scope_a_lineage_tag lifetime = {0};
	struct timew_scope_a_lineage_tag transaction = {0};
	CHECK(timew_scope_a_lineage_begin(&i, &c, &lifetime));
	CHECK(timew_scope_a_lineage_begin_transaction(
		&i, &lifetime, &c, &transaction));
	CHECK(transaction.lifetime_sequence == lifetime.lifetime_sequence);
	CHECK(transaction.transaction_sequence == 1);
	CHECK(!timew_scope_a_lineage_begin_transaction(
		&i, &lifetime, &c, &transaction));
	struct timew_scope_a_lineage_assessment a =
		timew_scope_a_lineage_observe_transaction(&i, &transaction, &c);
	CHECK(a.candidate_correlated_only && a.transaction_correlated_only);
	no_authority(a);
	CHECK(!timew_scope_a_lineage_revoke(&i, &transaction));
	CHECK(i.live && i.transaction_live);
	/* One-bit geometry drift permanently revokes this candidate and
	 * transaction, even when the original value is restored. */
	struct timew_scope_a_lineage_context modified = c;
	CHECK(timew_scope_a_exact_geometry_capture(&modified.pending,
		TIMEW_SCOPE_A_GEOMETRY_PENDING,
		259, nextafter(92.5, INFINITY), 1402, 872));
	a = timew_scope_a_lineage_observe_transaction(
		&i, &transaction, &modified);
	CHECK(!a.candidate_correlated_only && !a.transaction_correlated_only);
	CHECK(!i.live && !i.transaction_live);
	CHECK(!timew_scope_a_lineage_observe_transaction(
		&i, &transaction, &c).candidate_correlated_only);
	CHECK(!timew_scope_a_lineage_revoke(&i, &lifetime));
	CHECK(i.lifetime_floor == 1 && i.transaction_floor == 1);
	struct timew_scope_a_lineage_tag fresh = {0};
	struct timew_scope_a_lineage_tag new_txn = {0};
	CHECK(timew_scope_a_lineage_begin(&i, &c, &fresh));
	CHECK(timew_scope_a_lineage_begin_transaction(
		&i, &fresh, &c, &new_txn));
	CHECK(fresh.lifetime_sequence == 2 &&
		new_txn.transaction_sequence == 2);
	CHECK(!timew_scope_a_lineage_observe_transaction(
		&i, &transaction, &c).candidate_correlated_only);
	CHECK(timew_scope_a_lineage_observe_transaction(
		&i, &new_txn, &c).transaction_correlated_only);
	CHECK(timew_scope_a_lineage_revoke(&i, &fresh));
	CHECK(!i.live && !i.transaction_live);
	timew_scope_a_lineage_shutdown(&i);
}

static void test_context_drift(void) {
	for (int kind = 0; kind < 13; ++kind) {
		struct timew_scope_a_lineage_issuer i = boot();
		struct timew_scope_a_lineage_context c = fixture();
		struct timew_scope_a_lineage_tag t = {0};
		CHECK(timew_scope_a_lineage_begin(&i, &c, &t));
		struct timew_scope_a_lineage_context altered = c;
		switch (kind) {
		case 0: altered.native_view_ref++; break;
		case 1: altered.native_root_resource_ref++; break;
		case 2: altered.native_seat_ref++; break;
		case 3: altered.native_output_ref++; break;
		case 4: altered.native_workspace_ref++; break;
		case 5: altered.seat_epoch++; break;
		case 6: altered.session_epoch++; break;
		case 7: altered.output_epoch++; break;
		case 8: altered.workspace_epoch++; break;
		case 9: altered.output_scale_bits = bits(1.5); break;
		case 10: altered.output_transform = 2; break;
		case 11: altered.session_locked = true; break;
		case 12: altered.mapped = false; break;
		}
		struct timew_scope_a_lineage_assessment a =
			timew_scope_a_lineage_observe(&i, &t, &altered);
		CHECK(!a.candidate_correlated_only);
		CHECK(!i.live);
		CHECK(!timew_scope_a_lineage_observe(&i, &t, &c)
			.candidate_correlated_only);
		no_authority(a);
		timew_scope_a_lineage_shutdown(&i);
	}
}

static void test_invalid_and_overflow(void) {
	struct timew_scope_a_lineage_issuer i = {0};
	struct timew_scope_a_lineage_context c = fixture();
	struct timew_scope_a_lineage_tag token = {
		.boot_a = UINT64_MAX,
		.lifetime_sequence = 99,
	};
	struct timew_scope_a_lineage_tag original = token;
	CHECK(!timew_scope_a_lineage_begin(&i, &c, &token));
	CHECK(memcmp(&token, &original, sizeof(token)) == 0);
	CHECK(!timew_scope_a_lineage_boot(NULL));
	CHECK(timew_scope_a_lineage_boot(&i));
	struct timew_scope_a_lineage_context bad = c;
	bad.native_root_resource_ref = bad.native_container_ref;
	CHECK(!timew_scope_a_lineage_begin(&i, &bad, &token));
	CHECK(memcmp(&token, &original, sizeof(token)) == 0);
	bad = c; bad.output_scale_bits = bits(NAN);
	CHECK(!timew_scope_a_lineage_begin(&i, &bad, &token));
	bad = c; bad.output_scale_bits = bits(-1.0);
	CHECK(!timew_scope_a_lineage_begin(&i, &bad, &token));
	bad = c; bad.output_transform = 9;
	CHECK(!timew_scope_a_lineage_begin(&i, &bad, &token));
	bad = c; bad.pending.phase = TIMEW_SCOPE_A_GEOMETRY_CURRENT;
	CHECK(!timew_scope_a_lineage_begin(&i, &bad, &token));
	CHECK(i.lifetime_floor == 0 && !i.live);
	i.lifetime_floor = UINT64_MAX;
	CHECK(!timew_scope_a_lineage_begin(&i, &c, &token));
	CHECK(i.lifetime_floor == UINT64_MAX && !i.live);
	i.lifetime_floor = 0;
	CHECK(timew_scope_a_lineage_begin(&i, &c, &token));
	struct timew_scope_a_lineage_tag txn = {0};
	i.transaction_floor = UINT64_MAX;
	CHECK(!timew_scope_a_lineage_begin_transaction(
		&i, &token, &c, &txn));
	CHECK(!i.transaction_live);
	CHECK(timew_scope_a_lineage_revoke(&i, &token));
	timew_scope_a_lineage_shutdown(&i);
}

int main(void) {
	test_boot_and_stale_reuse();
	test_transaction_and_revocation();
	test_context_drift();
	test_invalid_and_overflow();
	printf("TIMEW_OBSERVED_LINEAGE PASS: %u assertions; "
		"random process boot tag + monotonic lifetime/txn floors; "
		"stale callback/geometry/output/seat refusal; "
		"authority NONE, selection NONE, receipt NONE\n", checks);
	return 0;
}
