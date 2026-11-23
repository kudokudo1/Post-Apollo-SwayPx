#ifndef _SWAY_TIMEW_SCOPE_A_OBSERVED_LINEAGE_H
#define _SWAY_TIMEW_SCOPE_A_OBSERVED_LINEAGE_H

#include <stdbool.h>
#include <stdint.h>

#include "sway/timew_scope_a_exact_geometry.h"

/*
 * Scope A // private, process-local, DENY-ONLY native observation lineage.
 *
 * The issuer generates its OWN random boot tag and monotonic sequence.
 * A tag only disambiguates observations inside a disposable compositor
 * process: it is NOT a bearer credential, native object identity proof,
 * physical operator selection, enrolled C3e/C4b source, C5 spend, command
 * permit, signed transaction ID, or per-window receipt.
 *
 * Never expose this interface through IPC, or use caller-provided context as
 * proof. It is an isolated reference for a future compositor-owned adapter.
 * No source in this PR is registered in production sway/meson.build.
 */
struct timew_scope_a_lineage_context {
	uintptr_t native_view_ref;
	uintptr_t native_container_ref;
	uintptr_t native_root_resource_ref;
	uintptr_t native_client_ref;
	uintptr_t native_seat_ref;
	uintptr_t native_output_ref;
	uintptr_t native_workspace_ref;
	uint64_t seat_epoch;
	uint64_t session_epoch;
	uint64_t output_epoch;
	uint64_t workspace_epoch;
	uint64_t output_scale_bits;
	uint32_t output_transform;
	struct timew_scope_a_exact_geometry pending;
	bool mapped;
	bool exact_xdg_toplevel;
	bool floating;
	bool fullscreen;
	bool scratchpad;
	bool session_locked;
	bool destroying;
};

/* Opaque-to-external-consumer diagnostic correlation, not a credential. */
struct timew_scope_a_lineage_tag {
	uint64_t boot_a;
	uint64_t boot_b;
	uint64_t lifetime_sequence;
	uint64_t transaction_sequence; /* zero for lifetime-only token */
};

struct timew_scope_a_lineage_issuer {
	bool booted;
	bool shut_down;
	bool live;
	bool transaction_live;
	uint64_t boot_a;
	uint64_t boot_b;
	uint64_t lifetime_floor;
	uint64_t transaction_floor;
	struct timew_scope_a_lineage_context selected_candidate;
};

/*
 * Results can never confer privilege. A match means only that this issuer's
 * private data still correlates with observations supplied by its caller.
 */
struct timew_scope_a_lineage_assessment {
	bool candidate_correlated_only;
	bool transaction_correlated_only;
	bool physical_selection_authenticated;
	bool native_object_origin_authenticated;
	bool native_transaction_ack_authenticated;
	bool source_enrolled;
	bool c5_spent;
	bool scene_settled_verified;
	bool resize_allowed;
	bool move_allowed;
	bool protected_receipt_verified;
	bool retry_allowed;
	bool undo_allowed;
};

/* Only once on a ZERO-initialized issuer, from inside the native process.
 * getrandom(2) failure aborts creation; no clock, PID or weak fallback. */
bool timew_scope_a_lineage_boot(struct timew_scope_a_lineage_issuer *issuer);

/* Candidate-only. Issues fresh lifetime after separate closed lifetime.
 * No operator selection ceremony is performed by this component. */
bool timew_scope_a_lineage_begin(struct timew_scope_a_lineage_issuer *issuer,
	const struct timew_scope_a_lineage_context *observed,
	struct timew_scope_a_lineage_tag *out_lifetime);

/* A correctly tagged observation with any invalid/drifted context revokes
 * the active lifetime. A stale tag NEVER affects a newer lifetime. */
struct timew_scope_a_lineage_assessment
timew_scope_a_lineage_observe(struct timew_scope_a_lineage_issuer *issuer,
	const struct timew_scope_a_lineage_tag *lifetime,
	const struct timew_scope_a_lineage_context *observed);

/* Bind one PROCESS-LOCAL diagnostic transaction sequence to an already
 * live candidate. No configure ACK, settlement or command is established. */
bool timew_scope_a_lineage_begin_transaction(
	struct timew_scope_a_lineage_issuer *issuer,
	const struct timew_scope_a_lineage_tag *lifetime,
	const struct timew_scope_a_lineage_context *observed,
	struct timew_scope_a_lineage_tag *out_transaction);

struct timew_scope_a_lineage_assessment
timew_scope_a_lineage_observe_transaction(
	struct timew_scope_a_lineage_issuer *issuer,
	const struct timew_scope_a_lineage_tag *transaction,
	const struct timew_scope_a_lineage_context *observed);

/* Exact lifetime token required. Clears live pointers and geometry,
 * retains both sequence floors. No transaction can survive revocation. */
bool timew_scope_a_lineage_revoke(struct timew_scope_a_lineage_issuer *issuer,
	const struct timew_scope_a_lineage_tag *lifetime);

/* Irreversible for this process issuer; no weak reinitialization. */
void timew_scope_a_lineage_shutdown(struct timew_scope_a_lineage_issuer *issuer);

#endif
