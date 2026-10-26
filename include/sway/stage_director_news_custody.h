#ifndef _SWAY_STAGE_DIRECTOR_NEWS_CUSTODY_H
#define _SWAY_STAGE_DIRECTOR_NEWS_CUSTODY_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_process_evidence.h"

/*
 * Candidate-only Linux process custody for future authorized WATCH sessions.
 * This component does not launch, stop, signal or control a browser.
 * It must NOT be reachable from untrusted IPC or QML. A PID and a namespace
 * are not sufficient provenance for an operator-requested child.
 */
enum stage_director_news_custody_state {
	SD_NEWS_CUSTODY_EMPTY,
	SD_NEWS_CUSTODY_RESERVED,
	SD_NEWS_CUSTODY_PROCESS_CANDIDATE,
};

struct stage_director_news_custody {
	enum stage_director_news_custody_state state;
	uint64_t last_epoch;
	uint64_t epoch;
	uintptr_t host_ref;
	struct stage_director_process_launch_root root;
	int pidfd; /* owned; -1 until captured; CLOEXEC; never serialized */
};

struct stage_director_news_custody_result {
	bool process_candidate;
	bool process_alive;
	bool owner_verified;
	bool can_launch;
	bool can_bind_viewer;
	bool can_clip;
	bool can_accept_input;
};

void stage_director_news_custody_init(struct stage_director_news_custody *lease);
bool stage_director_news_custody_reserve(
		struct stage_director_news_custody *lease, uint64_t epoch,
		uintptr_t host_ref, bool unique_native_host_candidate);

/* Inspect and hold a pidfd for the exact expected PID+start-time+UID. */
bool stage_director_news_custody_capture(
		struct stage_director_news_custody *lease, uint64_t epoch,
		const struct stage_director_process_launch_root *expected);

struct stage_director_news_custody_result stage_director_news_custody_inspect(
		const struct stage_director_news_custody *lease);

/* CLOSE / host loss / child disappearance: only close our owned pidfd. */
bool stage_director_news_custody_release(
		struct stage_director_news_custody *lease, uint64_t epoch);

#endif
