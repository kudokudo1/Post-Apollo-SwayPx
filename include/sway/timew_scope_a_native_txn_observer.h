#ifndef _SWAY_TIMEW_SCOPE_A_NATIVE_TXN_OBSERVER_H
#define _SWAY_TIMEW_SCOPE_A_NATIVE_TXN_OBSERVER_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/timew_scope_a_exact_geometry.h"

/*
 * PRIVATE, DORMANT read-only compositor snapshot (not Sway IPC).
 * Reads pending/current value fields and output context from real in-process
 * objects. No trust is placed in pointers, stale fields, or a caller-supplied
 * stage. No registered hook, signed receipt, presentation witness or action.
 */
struct sway_view;

struct timew_scope_a_native_txn_snapshot {
	struct timew_scope_a_exact_geometry pending;
	struct timew_scope_a_exact_geometry current;
	uintptr_t native_container_ref;
	uintptr_t native_root_surface_ref;
	uintptr_t pending_workspace_ref;
	uintptr_t current_workspace_ref;
	uintptr_t native_output_ref;
	int32_t output_lx, output_ly;
	int32_t output_transformed_width, output_transformed_height;
	uint32_t output_transform;
	uint64_t output_scale_binary64_bits;
	bool pending_transaction_present;
	bool queued_transaction_present;
	bool current_layout_value_observed;
	bool native_client_ack_proven;
	bool native_transaction_identity_proven;
	bool native_scene_presented_proven;
	bool output_epoch_proven;
	bool physical_selection_proven;
	bool source_proven;
	bool c5_spend_proven;
	bool protected_geometry_ready;
	bool native_resize_allowed;
	bool native_move_allowed;
};

/* False on missing/invalid state and does not modify *out. */
bool timew_scope_a_native_txn_snapshot_read(
	struct sway_view *view, struct timew_scope_a_native_txn_snapshot *out);

#endif
