#ifndef _SWAY_STAGE_DIRECTOR_NEWS_SUPERVISOR_H
#define _SWAY_STAGE_DIRECTOR_NEWS_SUPERVISOR_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

/*
 * SD1 // isolated one-child process supervision.
 *
 * Production builds expose NO spawn method. The only executable test
 * launcher is compiled with SD_STAGE_DIRECTOR_TEST_HELPER and runs a
 * harmless fixed /usr/bin/sleep child in a disposable CI environment.
 * Nothing connects this component to NEWS QML, Sway IPC or compositor hooks.
 */
enum stage_director_news_supervisor_state {
	SD_NEWS_SUPERVISOR_EMPTY,
	SD_NEWS_SUPERVISOR_RUNNING,
	SD_NEWS_SUPERVISOR_STOP_REQUESTED,
	SD_NEWS_SUPERVISOR_EXITED,
	SD_NEWS_SUPERVISOR_LOST_REAPER,
};

struct stage_director_news_supervisor {
	enum stage_director_news_supervisor_state state;
	uint64_t last_epoch;
	uint64_t epoch;
	uintptr_t host_ref;
	pid_t child_pid;
	uint64_t child_start_ticks;
	int pidfd;
	int wait_status;
};

struct stage_director_news_supervisor_result {
	enum stage_director_news_supervisor_state state;
	bool child_may_be_running;
	bool process_reaped;
	bool owner_verified;
	bool can_launch_browser;
	bool can_clip;
	bool can_accept_input;
};

void stage_director_news_supervisor_init(
		struct stage_director_news_supervisor *supervisor);

struct stage_director_news_supervisor_result stage_director_news_supervisor_inspect(
		const struct stage_director_news_supervisor *supervisor);

/* Signal ONLY the held child by pidfd; no PID-based arbitrary signaling. */
bool stage_director_news_supervisor_request_stop(
		struct stage_director_news_supervisor *supervisor, uint64_t epoch);

/* Nonblocking direct-child wait; never wait on unrelated processes. */
bool stage_director_news_supervisor_poll(
		struct stage_director_news_supervisor *supervisor, uint64_t epoch);

/* Close pidfd and allow a newer epoch ONLY after child has been reaped. */
bool stage_director_news_supervisor_finish(
		struct stage_director_news_supervisor *supervisor, uint64_t epoch);

#ifdef SD_STAGE_DIRECTOR_TEST_HELPER
/* TEST-ONLY: spawn fixed /usr/bin/sleep 30, no browser or arbitrary argv. */
bool stage_director_news_supervisor_start_test_child(
		struct stage_director_news_supervisor *supervisor,
		uint64_t epoch, uintptr_t host_ref);
#endif

#endif
