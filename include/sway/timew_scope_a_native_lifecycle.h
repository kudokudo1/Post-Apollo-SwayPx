#ifndef _SWAY_TIMEW_SCOPE_A_NATIVE_LIFECYCLE_H
#define _SWAY_TIMEW_SCOPE_A_NATIVE_LIFECYCLE_H

#include <stdbool.h>

#include "sway/timew_scope_a_lifecycle_signals.h"

struct sway_seat;
struct sway_view;
struct sway_container;

/*
 * Dormant native adapter. Its ONLY permitted effect is installing real
 * in-process destroy/unmap listeners which revoke a candidate.
 *
 * It MUST remain unlinked from the production compositor, and must never
 * be exposed to IPC or called by an untrusted packet handler. A current
 * pointer hit, even with matching native refs, is NOT authenticated TimeW
 * selection or physical-input provenance.
 */
bool timew_scope_a_native_lifecycle_bind_candidate(
	struct timew_scope_a_lifecycle_signals *watch,
	struct timew_scope_a_lifetime_candidate *ledger,
	struct sway_seat *seat, struct sway_container *con,
	struct sway_view *view);

#endif
