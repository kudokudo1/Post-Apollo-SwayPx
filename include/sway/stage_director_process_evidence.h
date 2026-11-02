#ifndef _SWAY_STAGE_DIRECTOR_PROCESS_EVIDENCE_H
#define _SWAY_STAGE_DIRECTOR_PROCESS_EVIDENCE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * NEWS SD1 // kernel process provenance EVIDENCE only.
 *
 * A compositor-observed wl_client PID is still not proof that a browser
 * belongs to a particular operator WATCH request. Only a future trusted
 * launcher can record the root process identity at birth. This module
 * checks lineage against that identity and NEVER grants render/input/launch.
 */
#define SD_PROCESS_MAX_ANCESTRY 12

struct stage_director_process_identity {
	int64_t pid;
	int64_t parent_pid;
	uint64_t start_time_ticks;
	uint32_t uid;
};

struct stage_director_process_launch_root {
	int64_t pid;                 /* from future private trusted launcher */
	uint64_t start_time_ticks;  /* kernel /proc/<pid>/stat field 22 */
	uint32_t uid;               /* kernel observed identity */
};

typedef bool (*stage_director_process_reader)(
		int64_t pid, struct stage_director_process_identity *out,
		void *context);

enum stage_director_process_evidence_state {
	SD_PROCESS_EVIDENCE_INVALID,
	SD_PROCESS_EVIDENCE_READ_FAILED,
	SD_PROCESS_EVIDENCE_ROOT_MISMATCH,
	SD_PROCESS_EVIDENCE_ACTOR_MISMATCH,
	SD_PROCESS_EVIDENCE_UNRELATED,
	SD_PROCESS_EVIDENCE_UNSTABLE,
	SD_PROCESS_EVIDENCE_LINEAGE_CANDIDATE,
};

struct stage_director_process_evidence_result {
	enum stage_director_process_evidence_state state;
	bool lineage_candidate;
	/* CRITICAL: no provenance proof or activation with read-only evidence. */
	bool owner_verified;
	bool can_bind_viewer;
	bool can_launch;
	bool can_clip;
	bool can_accept_input;
};

struct stage_director_process_evidence_result stage_director_process_check(
		const struct stage_director_process_launch_root *launch_root,
		int64_t actor_pid, uint64_t observed_actor_start,
		stage_director_process_reader read_process, void *context);

const char *stage_director_process_evidence_name(
		enum stage_director_process_evidence_state state);

/*
 * Pure Linux /proc/<pid>/stat parser and native read-only adapter.
 * Process names may contain spaces and ')' so tokenize AFTER final ')'.
 */
bool stage_director_parse_linux_proc_stat(const char *stat,
		int64_t expected_pid, int64_t *out_parent_pid,
		uint64_t *out_start_time_ticks);
bool stage_director_read_linux_process(
		int64_t pid, struct stage_director_process_identity *out,
		void *context);

#endif
