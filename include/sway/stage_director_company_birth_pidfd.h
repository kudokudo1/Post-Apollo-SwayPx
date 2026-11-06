#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_BIRTH_PIDFD_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_BIRTH_PIDFD_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_company_origin.h"
#include "sway/stage_director_process_evidence.h"

/*
 * M28a // hold a real Linux pidfd delivered by a future private M23 v2
 * selected-Company Wayland request. The fd is a kernel process lifetime
 * handle, not a string/PID token. Capturing a pidfd does NOT prove that the
 * process was started from the approved M15 executable, or authorize scene.
 *
 * The server-side Wayland handler must revalidate exact selected wl_client,
 * host selection, one outstanding intent generation, and single fd ownership.
 * This module never publishes a global, sends a Wayland request, or launches.
 */
enum stage_director_company_birth_status {
	SD_COMPANY_BIRTH_NO_INTENT,
	SD_COMPANY_BIRTH_INVALID_NATIVE_HOST,
	SD_COMPANY_BIRTH_INVALID_PIDFD,
	SD_COMPANY_BIRTH_WRONG_CHILD,
	SD_COMPANY_BIRTH_PROCESS_CHANGED,
	SD_COMPANY_BIRTH_OUTSIDE_WINDOW,
	SD_COMPANY_BIRTH_EXHAUSTED,
	SD_COMPANY_BIRTH_HELD_CANDIDATE_ONLY,
	SD_COMPANY_BIRTH_ACTOR_CANDIDATE_ONLY,
};

struct stage_director_company_birth_result {
	enum stage_director_company_birth_status status;
	bool kernel_child_candidate;
	bool mapped_actor_candidate;
	bool exact_executable_attested;
	bool launch_authenticated;
	bool owner_verified;
	bool can_pair_actor;
	bool can_render;
	bool can_clip;
	bool can_accept_input;
	bool can_focus;
	bool can_show_popup;
};

/* Initialize once; cannot memcpy/move while holding a file descriptor. */
struct stage_director_company_birth_pidfd {
	bool initialized;
	bool held;
	int fd;
	int64_t pid;
	uint32_t uid;
	uint64_t child_start_ticks;
	int64_t host_pid;
	uint64_t host_start_ticks;
	uintptr_t host_native_client_ref;
	uint64_t selection_epoch;
	uint64_t intent_sequence;
	uint64_t host_revision;
	uint32_t mode;
};

void stage_director_company_birth_pidfd_init(
	struct stage_director_company_birth_pidfd *held);
void stage_director_company_birth_pidfd_revoke(
	struct stage_director_company_birth_pidfd *held);

/* The incoming fd is BORROWED. On success a CLOEXEC duplicate is retained.
 * On failure, caller still owns and must close the incoming fd. */
struct stage_director_company_birth_result
stage_director_company_birth_pidfd_record(
	struct stage_director_company_birth_pidfd *held,
	int borrowed_fd, int64_t claimed_pid,
	const struct stage_director_company_intent_snapshot *intent,
	const struct stage_director_company_native_host *host,
	stage_director_process_reader read_process, void *reader_context);

/* Recheck original native Wayland peer and Linux process against the held
 * kernel pidfd. A positive result is still candidate-only, not a lease. */
struct stage_director_company_birth_result
stage_director_company_birth_pidfd_match_actor(
	struct stage_director_company_birth_pidfd *held,
	const struct stage_director_company_native_host *host,
	const struct stage_director_company_native_actor *actor,
	const struct stage_director_company_lineage_result *lineage,
	stage_director_process_reader read_process, void *reader_context);

const char *stage_director_company_birth_status_name(
	enum stage_director_company_birth_status status);
#endif
