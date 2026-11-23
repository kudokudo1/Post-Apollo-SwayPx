#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "sway/timew_scope_a_lifetime_candidate.h"

static int checks;

static struct timew_scope_a_lifetime_observation live_fixture(void) {
	return (struct timew_scope_a_lifetime_observation) {
		.compositor_epoch = 7,
		.lifetime_generation = 10,
		.selection_nonce = 91,
		.seat_epoch = 6,
		.session_epoch = 12,
		.output_epoch = 8,
		.workspace_epoch = 4,
		.native_container_ref = 0x111,
		.native_root_surface_ref = 0x222,
		.native_seat_ref = 0x333,
		.native_output_ref = 0x444,
		.native_workspace_ref = 0x555,
		.x = 110,
		.y = 200,
		.width = 720,
		.height = 480,
		.mapped = true,
		.exact_xdg_toplevel = true,
		.floating = true,
		.fullscreen = false,
		.scratchpad = false,
		.session_locked = false,
		.destroying = false,
	};
}

static void no_authority(struct timew_scope_a_candidate_probe p) {
	assert(!p.operator_selection_authenticated);
	assert(!p.producer_verified);
	assert(!p.c5_spend_verified);
	assert(!p.native_resize_allowed);
	assert(!p.native_move_allowed);
	assert(!p.protected_receipt_verified);
}

static void assert_revoked(struct timew_scope_a_lifetime_candidate *s,
		const struct timew_scope_a_lifetime_observation *original) {
	struct timew_scope_a_candidate_probe p =
		timew_scope_a_lifetime_probe(s, original);
	no_authority(p);
	assert(!p.candidate_correlated);
	assert(p.state == TIMEW_SCOPE_A_PROBE_REVOKED);
	assert(s->state == TIMEW_SCOPE_A_REVOKED);
	assert(!timew_scope_a_lifetime_reserve(s, original));
	++checks;
}

static void test_invalid_candidates(void) {
	struct timew_scope_a_lifetime_candidate s;
	struct timew_scope_a_lifetime_observation o = live_fixture();
	timew_scope_a_lifetime_init(&s);
	assert(!timew_scope_a_lifetime_reserve(NULL, &o));
	assert(!timew_scope_a_lifetime_reserve(&s, NULL));
	assert(!timew_scope_a_lifetime_revoke(&s));
	no_authority(timew_scope_a_lifetime_probe(NULL, &o));
	no_authority(timew_scope_a_lifetime_probe(&s, &o));
	assert(timew_scope_a_lifetime_probe(&s, &o).state ==
		TIMEW_SCOPE_A_PROBE_NO_CANDIDATE);

#define INVALID(change) do { \
		timew_scope_a_lifetime_init(&s); \
		o = live_fixture(); \
		change; \
		assert(!timew_scope_a_lifetime_reserve(&s, &o)); \
		assert(s.state == TIMEW_SCOPE_A_EMPTY); \
		++checks; \
	} while (0)
	INVALID(o.compositor_epoch = 0);
	INVALID(o.lifetime_generation = 0);
	INVALID(o.selection_nonce = 0);
	INVALID(o.seat_epoch = 0);
	INVALID(o.session_epoch = 0);
	INVALID(o.output_epoch = 0);
	INVALID(o.workspace_epoch = 0);
	INVALID(o.native_container_ref = 0);
	INVALID(o.native_root_surface_ref = 0);
	INVALID(o.native_root_surface_ref = o.native_container_ref);
	INVALID(o.native_seat_ref = 0);
	INVALID(o.native_output_ref = 0);
	INVALID(o.native_workspace_ref = 0);
	INVALID(o.width = 0);
	INVALID(o.height = -1);
	INVALID(o.mapped = false);
	INVALID(o.exact_xdg_toplevel = false);
	INVALID(o.floating = false);
	INVALID(o.fullscreen = true);
	INVALID(o.scratchpad = true);
	INVALID(o.session_locked = true);
	INVALID(o.destroying = true);
#undef INVALID
}

static void test_correlated_but_never_authorized(void) {
	struct timew_scope_a_lifetime_candidate s;
	struct timew_scope_a_lifetime_observation o = live_fixture();
	timew_scope_a_lifetime_init(&s);
	assert(timew_scope_a_lifetime_reserve(&s, &o));
	assert(!timew_scope_a_lifetime_reserve(&s, &o));
	for (int i = 0; i < 5; ++i) {
		struct timew_scope_a_candidate_probe p =
			timew_scope_a_lifetime_probe(&s, &o);
		assert(p.state == TIMEW_SCOPE_A_PROBE_MATCHED_CANDIDATE_ONLY);
		assert(p.candidate_correlated);
		no_authority(p);
		++checks;
	}

	/* Even a perfect fixture match cannot move a surface. */
	assert(s.state == TIMEW_SCOPE_A_CANDIDATE);
	assert(timew_scope_a_lifetime_revoke(&s));
	assert_revoked(&s, &o);
	assert(!timew_scope_a_lifetime_revoke(&s));
	/* An old selection/identity cannot be resurrected after cancellation. */
	assert(!timew_scope_a_lifetime_reserve(&s, &o));
	o.lifetime_generation = 11;
	o.selection_nonce = 92;
	assert(timew_scope_a_lifetime_reserve(&s, &o));
	assert(timew_scope_a_lifetime_revoke(&s));
	o.lifetime_generation = 10;
	assert(!timew_scope_a_lifetime_reserve(&s, &o));
	o.lifetime_generation = 1;
	o.compositor_epoch = 8;
	assert(timew_scope_a_lifetime_reserve(&s, &o));
	assert(timew_scope_a_lifetime_revoke(&s));
	o.compositor_epoch = 7;
	o.lifetime_generation = 50;
	assert(!timew_scope_a_lifetime_reserve(&s, &o));
	++checks;
}

static void test_every_drift_revokes_without_resurrection(void) {
	struct timew_scope_a_lifetime_candidate s;
	struct timew_scope_a_lifetime_observation original, changed;

#define DRIFT(change) do { \
		timew_scope_a_lifetime_init(&s); \
		original = live_fixture(); \
		assert(timew_scope_a_lifetime_reserve(&s, &original)); \
		changed = original; \
		change; \
		struct timew_scope_a_candidate_probe p = \
			timew_scope_a_lifetime_probe(&s, &changed); \
		no_authority(p); \
		assert(!p.candidate_correlated); \
		assert(p.state == TIMEW_SCOPE_A_PROBE_REVOKED); \
		assert_revoked(&s, &original); \
	} while (0)
	DRIFT(changed.compositor_epoch = 8);
	DRIFT(changed.lifetime_generation = 11);
	DRIFT(changed.selection_nonce = 92);
	DRIFT(changed.seat_epoch = 7);
	DRIFT(changed.session_epoch = 13);
	DRIFT(changed.output_epoch = 9);
	DRIFT(changed.workspace_epoch = 5);
	DRIFT(changed.native_container_ref = 0x991);
	DRIFT(changed.native_root_surface_ref = 0x992);
	DRIFT(changed.native_seat_ref = 0x993);
	DRIFT(changed.native_output_ref = 0x994);
	DRIFT(changed.native_workspace_ref = 0x995);
	DRIFT(changed.x = 111);
	DRIFT(changed.y = 201);
	DRIFT(changed.width = 721);
	DRIFT(changed.height = 481);
	DRIFT(changed.mapped = false);
	DRIFT(changed.exact_xdg_toplevel = false);
	DRIFT(changed.floating = false);
	DRIFT(changed.fullscreen = true);
	DRIFT(changed.scratchpad = true);
	DRIFT(changed.session_locked = true);
	DRIFT(changed.destroying = true);
#undef DRIFT

	/* SwayPX floating resize set centers the new rectangle, shifting x/y.
	 * TimeW may not act on a synthetic original-x/y post-resize assumption. */
	timew_scope_a_lifetime_init(&s);
	original = live_fixture();
	assert(timew_scope_a_lifetime_reserve(&s, &original));
	changed = original;
	changed.width += 100;
	changed.x -= 50;
	changed.height += 60;
	changed.y -= 30;
	struct timew_scope_a_candidate_probe p =
		timew_scope_a_lifetime_probe(&s, &changed);
	no_authority(p);
	assert(p.state == TIMEW_SCOPE_A_PROBE_REVOKED);
	assert_revoked(&s, &original);

	/* Missing observation is not permission to retry. */
	timew_scope_a_lifetime_init(&s);
	original = live_fixture();
	assert(timew_scope_a_lifetime_reserve(&s, &original));
	p = timew_scope_a_lifetime_probe(&s, NULL);
	assert(p.state == TIMEW_SCOPE_A_PROBE_REVOKED);
	no_authority(p);
	assert_revoked(&s, &original);
}

static void test_native_event_revocation(void) {
	const uintptr_t refs[] = { 0x111, 0x222, 0x333, 0x444, 0x555 };
	for (unsigned i = 0; i < sizeof(refs) / sizeof(refs[0]); ++i) {
		struct timew_scope_a_lifetime_candidate s;
		struct timew_scope_a_lifetime_observation o = live_fixture();
		timew_scope_a_lifetime_init(&s);
		assert(timew_scope_a_lifetime_reserve(&s, &o));
		assert(!timew_scope_a_lifetime_native_ref_lost(&s, 6, refs[i]));
		assert(!timew_scope_a_lifetime_native_ref_lost(&s, 7, 0));
		assert(!timew_scope_a_lifetime_native_ref_lost(&s, 7, 0x999));
		assert(timew_scope_a_lifetime_native_ref_lost(&s, 7, refs[i]));
		assert_revoked(&s, &o);
		assert(!timew_scope_a_lifetime_native_ref_lost(&s, 7, refs[i]));
	}
}

int main(void) {
	test_invalid_candidates();
	test_correlated_but_never_authorized();
	test_every_drift_revokes_without_resurrection();
	test_native_event_revocation();
	printf("TIMEW_SCOPE_A_LIFETIME_CI PASS: %d candidate, drift, "
		"native-loss, anti-reuse and no-authority assertions; "
		"operator/C3e/C5/resize/move/receipt authority NONE\n", checks);
	return 0;
}
