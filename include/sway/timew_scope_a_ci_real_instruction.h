#ifndef _SWAY_TIMEW_SCOPE_A_CI_REAL_INSTRUCTION_H
#define _SWAY_TIMEW_SCOPE_A_CI_REAL_INSTRUCTION_H

#include <stdint.h>

/*
 * CI-ONLY compositor-local diagnostic hook. Its implementation is added
 * exclusively to a disposable GitHub Actions checkout. No production
 * SwayPX unit contains the implementation or calls this interface.
 *
 * The transaction address and configure serial are process-local facts,
 * NOT replay-safe IDs, verified client ACKs, source credentials or receipts.
 */
struct sway_view;

void timew_scope_a_ci_note_real_instruction(
	struct sway_view *view, uintptr_t transaction_ref, uint32_t configure_serial);

#endif
