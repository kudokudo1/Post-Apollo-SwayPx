#ifndef _SWAY_TIMEW_SCOPE_A_CI_REAL_OUTCOME_H
#define _SWAY_TIMEW_SCOPE_A_CI_REAL_OUTCOME_H

#include <stdint.h>

/*
 * Disposable CI-only native callback observations.
 * These are NOT installed into production SwayPX or exposed over IPC.
 * Raw transaction refs are local matching hints, never identities, grants
 * or receipts. Actual source/selection/C5/final presentation remain UNKNOWN.
 */
struct sway_view;
void timew_scope_a_ci_outcome_ready(struct sway_view *view,
	uintptr_t transaction_ref, uint32_t serial);
void timew_scope_a_ci_outcome_timeout(uintptr_t transaction_ref);
void timew_scope_a_ci_outcome_current(uintptr_t transaction_ref);
void timew_scope_a_ci_outcome_scene(uintptr_t transaction_ref);

#endif
