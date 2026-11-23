#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/random.h>

#include "sway/timew_scope_a_observed_lineage.h"

/*
 * CI-only private issuer, not a cryptographic authorization mechanism.
 * Event callbacks must be compositor-owned before any future runtime use.
 */
static bool boot_entropy(uint64_t *a, uint64_t *b) {
	unsigned char bytes[sizeof(uint64_t) * 2];
	size_t done = 0;
	while (done < sizeof(bytes)) {
		ssize_t n = getrandom(bytes + done, sizeof(bytes) - done, 0);
		if (n < 0 && errno == EINTR) {
			continue;
		}
		if (n <= 0) {
			memset(bytes, 0, sizeof(bytes));
			return false;
		}
		done += (size_t)n;
	}
	memcpy(a, bytes, sizeof(*a));
	memcpy(b, bytes + sizeof(*a), sizeof(*b));
	memset(bytes, 0, sizeof(bytes));
	return (*a != 0 || *b != 0);
}

static bool valid_scale_bits(uint64_t bits) {
	double scale;
	memcpy(&scale, &bits, sizeof(scale));
	return isfinite(scale) && scale > 0.0 &&
		!(scale == 0.0 && signbit(scale));
}

static bool valid_context(const struct timew_scope_a_lineage_context *c) {
	double x, y, width, height;
	return c &&
		c->native_view_ref && c->native_container_ref &&
		c->native_root_resource_ref && c->native_client_ref &&
		c->native_seat_ref && c->native_output_ref &&
		c->native_workspace_ref &&
		c->native_container_ref != c->native_root_resource_ref &&
		c->seat_epoch && c->session_epoch &&
		c->output_epoch && c->workspace_epoch &&
		valid_scale_bits(c->output_scale_bits) &&
		c->output_transform <= 7 &&
		c->mapped && c->exact_xdg_toplevel && c->floating &&
		!c->fullscreen && !c->scratchpad &&
		!c->session_locked && !c->destroying &&
		c->pending.phase == TIMEW_SCOPE_A_GEOMETRY_PENDING &&
		timew_scope_a_exact_geometry_read(
			&c->pending, &x, &y, &width, &height);
}

static bool same_context(const struct timew_scope_a_lineage_context *a,
		const struct timew_scope_a_lineage_context *b) {
	return valid_context(a) && valid_context(b) &&
		a->native_view_ref == b->native_view_ref &&
		a->native_container_ref == b->native_container_ref &&
		a->native_root_resource_ref == b->native_root_resource_ref &&
		a->native_client_ref == b->native_client_ref &&
		a->native_seat_ref == b->native_seat_ref &&
		a->native_output_ref == b->native_output_ref &&
		a->native_workspace_ref == b->native_workspace_ref &&
		a->seat_epoch == b->seat_epoch &&
		a->session_epoch == b->session_epoch &&
		a->output_epoch == b->output_epoch &&
		a->workspace_epoch == b->workspace_epoch &&
		a->output_scale_bits == b->output_scale_bits &&
		a->output_transform == b->output_transform &&
		timew_scope_a_exact_geometry_equal(&a->pending, &b->pending) &&
		a->mapped == b->mapped &&
		a->exact_xdg_toplevel == b->exact_xdg_toplevel &&
		a->floating == b->floating &&
		a->fullscreen == b->fullscreen &&
		a->scratchpad == b->scratchpad &&
		a->session_locked == b->session_locked &&
		a->destroying == b->destroying;
}

static bool same_lifetime(const struct timew_scope_a_lineage_issuer *i,
		const struct timew_scope_a_lineage_tag *t) {
	return i && t && i->booted && !i->shut_down &&
		i->live && i->boot_a == t->boot_a &&
		i->boot_b == t->boot_b &&
		i->lifetime_floor == t->lifetime_sequence &&
		t->lifetime_sequence != 0;
}

static struct timew_scope_a_lineage_assessment deny(bool lifetime,
		bool transaction) {
	return (struct timew_scope_a_lineage_assessment) {
		.candidate_correlated_only = lifetime,
		.transaction_correlated_only = transaction,
		.physical_selection_authenticated = false,
		.native_object_origin_authenticated = false,
		.native_transaction_ack_authenticated = false,
		.source_enrolled = false,
		.c5_spent = false,
		.scene_settled_verified = false,
		.resize_allowed = false,
		.move_allowed = false,
		.protected_receipt_verified = false,
		.retry_allowed = false,
		.undo_allowed = false,
	};
}

static void invalidate_live(struct timew_scope_a_lineage_issuer *i) {
	memset(&i->selected_candidate, 0, sizeof(i->selected_candidate));
	i->live = false;
	i->transaction_live = false;
}

bool timew_scope_a_lineage_boot(struct timew_scope_a_lineage_issuer *i) {
	if (!i || i->booted || i->shut_down ||
			i->live || i->transaction_live ||
			i->boot_a || i->boot_b ||
			i->lifetime_floor || i->transaction_floor) {
		return false;
	}
	uint64_t a = 0, b = 0;
	if (!boot_entropy(&a, &b)) {
		return false;
	}
	i->boot_a = a;
	i->boot_b = b;
	i->booted = true;
	return true;
}

bool timew_scope_a_lineage_begin(struct timew_scope_a_lineage_issuer *i,
		const struct timew_scope_a_lineage_context *observed,
		struct timew_scope_a_lineage_tag *out) {
	if (!i || !out || !i->booted || i->shut_down ||
			i->live || i->lifetime_floor == UINT64_MAX ||
			!valid_context(observed)) {
		return false;
	}
	struct timew_scope_a_lineage_tag next = {
		.boot_a = i->boot_a,
		.boot_b = i->boot_b,
		.lifetime_sequence = i->lifetime_floor + 1,
		.transaction_sequence = 0,
	};
	i->lifetime_floor++;
	i->selected_candidate = *observed;
	i->live = true;
	i->transaction_live = false;
	*out = next;
	return true;
}

struct timew_scope_a_lineage_assessment
timew_scope_a_lineage_observe(struct timew_scope_a_lineage_issuer *i,
		const struct timew_scope_a_lineage_tag *lifetime,
		const struct timew_scope_a_lineage_context *observed) {
	if (!same_lifetime(i, lifetime) ||
			lifetime->transaction_sequence != 0) {
		return deny(false, false);
	}
	if (!same_context(&i->selected_candidate, observed)) {
		invalidate_live(i);
		return deny(false, false);
	}
	return deny(true, false);
}

bool timew_scope_a_lineage_begin_transaction(
		struct timew_scope_a_lineage_issuer *i,
		const struct timew_scope_a_lineage_tag *lifetime,
		const struct timew_scope_a_lineage_context *observed,
		struct timew_scope_a_lineage_tag *out) {
	if (!same_lifetime(i, lifetime) ||
			lifetime->transaction_sequence != 0 ||
			!out || i->transaction_live ||
			i->transaction_floor == UINT64_MAX) {
		return false;
	}
	if (!same_context(&i->selected_candidate, observed)) {
		invalidate_live(i);
		return false;
	}
	struct timew_scope_a_lineage_tag next = {
		.boot_a = i->boot_a,
		.boot_b = i->boot_b,
		.lifetime_sequence = i->lifetime_floor,
		.transaction_sequence = i->transaction_floor + 1,
	};
	i->transaction_floor++;
	i->transaction_live = true;
	*out = next;
	return true;
}

struct timew_scope_a_lineage_assessment
timew_scope_a_lineage_observe_transaction(
		struct timew_scope_a_lineage_issuer *i,
		const struct timew_scope_a_lineage_tag *transaction,
		const struct timew_scope_a_lineage_context *observed) {
	if (!same_lifetime(i, transaction) ||
			!i->transaction_live ||
			transaction->transaction_sequence == 0 ||
			i->transaction_floor != transaction->transaction_sequence) {
		return deny(false, false);
	}
	if (!same_context(&i->selected_candidate, observed)) {
		invalidate_live(i);
		return deny(false, false);
	}
	return deny(true, true);
}

bool timew_scope_a_lineage_revoke(struct timew_scope_a_lineage_issuer *i,
		const struct timew_scope_a_lineage_tag *lifetime) {
	if (!same_lifetime(i, lifetime) ||
			lifetime->transaction_sequence != 0) {
		return false;
	}
	invalidate_live(i);
	return true;
}

void timew_scope_a_lineage_shutdown(struct timew_scope_a_lineage_issuer *i) {
	if (!i || !i->booted || i->shut_down) {
		return;
	}
	invalidate_live(i);
	i->boot_a = 0;
	i->boot_b = 0;
	i->shut_down = true;
}
