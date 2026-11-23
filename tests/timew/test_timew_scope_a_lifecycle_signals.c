#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "sway/timew_scope_a_lifecycle_signals.h"

static unsigned cases;

static struct timew_scope_a_lifetime_observation snapshot(void) {
	return (struct timew_scope_a_lifetime_observation) {
		.compositor_epoch = 7,
		.lifetime_generation = 10,
		.selection_nonce = 91,
		.seat_epoch = 2, .session_epoch = 3,
		.output_epoch = 4, .workspace_epoch = 5,
		.native_container_ref = 0x101,
		.native_root_surface_ref = 0x202,
		.native_seat_ref = 0x303,
		.native_output_ref = 0x404,
		.native_workspace_ref = 0x505,
		.x = 120, .y = -30, .width = 810, .height = 600,
		.mapped = true, .exact_xdg_toplevel = true,
		.floating = true,
	};
}

static void initialize(struct timew_scope_a_lifetime_candidate *ledger,
		struct timew_scope_a_lifecycle_signals *watch,
		struct wl_signal *unmap, struct wl_signal *destroy,
		struct wl_signal *root_destroy) {
	timew_scope_a_lifetime_init(ledger);
	timew_scope_a_lifecycle_signals_init(watch);
	wl_signal_init(unmap);
	wl_signal_init(destroy);
	wl_signal_init(root_destroy);
	const struct timew_scope_a_lifetime_observation o = snapshot();
	assert(timew_scope_a_lifetime_reserve(ledger, &o));
}

static void bind(struct timew_scope_a_lifecycle_signals *watch,
		struct timew_scope_a_lifetime_candidate *ledger,
		struct wl_signal *unmap, struct wl_signal *destroy,
		struct wl_signal *root_destroy) {
	assert(timew_scope_a_lifecycle_signals_bind(watch, ledger,
		unmap, destroy, root_destroy, 0x101, 0x202));
	assert(watch->attached);
	++cases;
}

static void test_event_revoke(int which) {
	struct timew_scope_a_lifetime_candidate ledger;
	struct timew_scope_a_lifecycle_signals watch;
	struct wl_signal unmap, destroy, root_destroy;
	initialize(&ledger, &watch, &unmap, &destroy, &root_destroy);
	bind(&watch, &ledger, &unmap, &destroy, &root_destroy);
	assert(!timew_scope_a_lifecycle_signals_bind(&watch, &ledger,
		&unmap, &destroy, &root_destroy, 0x101, 0x202));
	struct wl_signal *signal =
		which == 0 ? &unmap : which == 1 ? &destroy : &root_destroy;
	/* Matches SwayPX's mutable view_unmap/container_begin_destroy
	 * signal delivery. Detach occurs synchronously in the callback. */
	wl_signal_emit_mutable(signal, NULL);
	assert(ledger.state == TIMEW_SCOPE_A_REVOKED);
	assert(!watch.attached);
	assert(wl_list_empty(&unmap.listener_list));
	assert(wl_list_empty(&destroy.listener_list));
	assert(wl_list_empty(&root_destroy.listener_list));
	/* The second event and an explicit cleanup are idempotent. */
	wl_signal_emit_mutable(&unmap, NULL);
	wl_signal_emit_mutable(&destroy, NULL);
	wl_signal_emit_mutable(&root_destroy, NULL);
	timew_scope_a_lifecycle_signals_release(&watch);
	assert(ledger.state == TIMEW_SCOPE_A_REVOKED);
	assert(!timew_scope_a_lifetime_reserve(&ledger,
		&(struct timew_scope_a_lifetime_observation) {
			.compositor_epoch = 7, .lifetime_generation = 10,
		}));
	++cases;
}

static void test_stale_listener_does_not_revoke_new_generation(void) {
	struct timew_scope_a_lifetime_candidate ledger;
	struct timew_scope_a_lifecycle_signals watch;
	struct wl_signal unmap, destroy, root_destroy;
	initialize(&ledger, &watch, &unmap, &destroy, &root_destroy);
	bind(&watch, &ledger, &unmap, &destroy, &root_destroy);
	/* Simulated explicit replacement while old signal is still attached.
	 * Old pointer value may be reused but generation is different. */
	assert(timew_scope_a_lifetime_revoke(&ledger));
	struct timew_scope_a_lifetime_observation next = snapshot();
	next.lifetime_generation = 11;
	next.selection_nonce = 92;
	assert(timew_scope_a_lifetime_reserve(&ledger, &next));
	wl_signal_emit_mutable(&destroy, NULL);
	assert(!watch.attached);
	assert(ledger.state == TIMEW_SCOPE_A_CANDIDATE);
	assert(ledger.selected.lifetime_generation == 11);
	assert(ledger.selected.selection_nonce == 92);
	assert(wl_list_empty(&unmap.listener_list));
	assert(wl_list_empty(&root_destroy.listener_list));
	/* Old callback cannot erase the new lifetime. */
	const struct timew_scope_a_candidate_probe p =
		timew_scope_a_lifetime_probe(&ledger, &next);
	assert(p.candidate_correlated);
	assert(!p.operator_selection_authenticated);
	assert(!p.producer_verified && !p.c5_spend_verified);
	assert(!p.native_resize_allowed && !p.native_move_allowed);
	assert(!p.protected_receipt_verified);
	++cases;
}

static void test_invalid_attachment_and_explicit_release(void) {
	struct timew_scope_a_lifetime_candidate ledger;
	struct timew_scope_a_lifecycle_signals watch;
	struct wl_signal unmap, destroy, root_destroy;
	initialize(&ledger, &watch, &unmap, &destroy, &root_destroy);
	assert(!timew_scope_a_lifecycle_signals_bind(NULL, &ledger,
		&unmap, &destroy, &root_destroy, 0x101, 0x202));
	assert(!timew_scope_a_lifecycle_signals_bind(&watch, NULL,
		&unmap, &destroy, &root_destroy, 0x101, 0x202));
	assert(!timew_scope_a_lifecycle_signals_bind(&watch, &ledger,
		&unmap, &destroy, &root_destroy, 0, 0x202));
	assert(!timew_scope_a_lifecycle_signals_bind(&watch, &ledger,
		&unmap, &destroy, &root_destroy, 0x101, 0x101));
	assert(!timew_scope_a_lifecycle_signals_bind(&watch, &ledger,
		&unmap, &destroy, &root_destroy, 0x101, 0x203));
	assert(!timew_scope_a_lifecycle_signals_bind(&watch, &ledger,
		&unmap, &destroy, &root_destroy, 0x102, 0x202));
	assert(!timew_scope_a_lifecycle_signals_bind(&watch, &ledger,
		&unmap, &unmap, &root_destroy, 0x101, 0x202));
	assert(!timew_scope_a_lifecycle_signals_bind(&watch, &ledger,
		NULL, &destroy, &root_destroy, 0x101, 0x202));
	assert(!watch.attached && ledger.state == TIMEW_SCOPE_A_CANDIDATE);
	bind(&watch, &ledger, &unmap, &destroy, &root_destroy);
	timew_scope_a_lifecycle_signals_release(&watch);
	assert(ledger.state == TIMEW_SCOPE_A_REVOKED);
	assert(!watch.attached && wl_list_empty(&unmap.listener_list));
	assert(wl_list_empty(&destroy.listener_list));
	assert(wl_list_empty(&root_destroy.listener_list));
	timew_scope_a_lifecycle_signals_release(&watch);
	++cases;
}

static void test_exact_double_conversion(void) {
	int32_t x = 1, y = 2, w = 3, h = 4;
	assert(timew_scope_a_checked_pending_rect(
		(double)INT32_MIN, (double)INT32_MAX, 1, 600,
		&x, &y, &w, &h));
	assert(x == INT32_MIN && y == INT32_MAX && w == 1 && h == 600);
	assert(timew_scope_a_checked_pending_rect(
		-400, 200, 1920, 1080, &x, &y, &w, &h));
	assert(x == -400 && y == 200 && w == 1920 && h == 1080);

#define INVALID_RECT(a,b,c,d) do { \
		assert(!timew_scope_a_checked_pending_rect( \
			(a),(b),(c),(d),&x,&y,&w,&h)); \
		assert(x == -400 && y == 200 && w == 1920 && h == 1080); \
		++cases; \
	} while (0)
	INVALID_RECT(NAN, 0, 100, 100);
	INVALID_RECT(0, INFINITY, 100, 100);
	INVALID_RECT(0, 0, 100, -INFINITY);
	INVALID_RECT(0, 0, 100, NAN);
	INVALID_RECT(0.5, 0, 100, 100);
	INVALID_RECT(0, -0.5, 100, 100);
	INVALID_RECT(0, 0, 100.5, 100);
	INVALID_RECT(0, 0, 100, 100.5);
	INVALID_RECT((double)INT32_MAX + 1, 0, 100, 100);
	INVALID_RECT((double)INT32_MIN - 1, 0, 100, 100);
	INVALID_RECT(0, 0, (double)INT32_MAX + 1, 100);
	INVALID_RECT(0, 0, 100, 0);
	INVALID_RECT(0, 0, -1, 100);
	INVALID_RECT(0, 0, 100, -1);
#undef INVALID_RECT
	assert(!timew_scope_a_checked_pending_rect(
		0,0,100,100,NULL,&y,&w,&h));
	assert(!timew_scope_a_checked_pending_rect(
		0,0,100,100,&x,NULL,&w,&h));
	++cases;
}

int main(void) {
	for (int i = 0; i < 3; ++i) {
		test_event_revoke(i);
	}
	test_stale_listener_does_not_revoke_new_generation();
	test_invalid_attachment_and_explicit_release();
	test_exact_double_conversion();
	printf("TIMEW_NATIVE_LIFECYCLE_SIGNAL PASS: %u event and exact-geometry "
		"groups; real Wayland mutable unmap/destroy listeners, "
		"old-generation-safe revocation, no native authority\n", cases);
	return 0;
}
