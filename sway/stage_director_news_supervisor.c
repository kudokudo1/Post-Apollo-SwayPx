#define _GNU_SOURCE
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#ifdef SD_STAGE_DIRECTOR_TEST_HELPER
#include <spawn.h>
#include "sway/stage_director_process_evidence.h"
extern char **environ;
#endif
#include "sway/stage_director_news_supervisor.h"

static bool pidfd_alive(int pidfd) {
	if (pidfd < 0) return false;
	struct pollfd p = {.fd = pidfd, .events = POLLIN};
	return poll(&p, 1, 0) == 0;
}

void stage_director_news_supervisor_init(
		struct stage_director_news_supervisor *s) {
	if (!s) return;
	memset(s, 0, sizeof(*s));
	s->pidfd = -1;
}

struct stage_director_news_supervisor_result stage_director_news_supervisor_inspect(
		const struct stage_director_news_supervisor *s) {
	struct stage_director_news_supervisor_result out = {
		.state = s ? s->state : SD_NEWS_SUPERVISOR_EMPTY,
		.owner_verified = false,
		.can_launch_browser = false,
		.can_clip = false,
		.can_accept_input = false,
	};
	if (!s) return out;
	out.process_reaped = s->state == SD_NEWS_SUPERVISOR_EXITED;
	if ((s->state == SD_NEWS_SUPERVISOR_RUNNING ||
			s->state == SD_NEWS_SUPERVISOR_STOP_REQUESTED) &&
			s->pidfd >= 0) {
		out.child_may_be_running = pidfd_alive(s->pidfd);
	}
	return out;
}

bool stage_director_news_supervisor_request_stop(
		struct stage_director_news_supervisor *s, uint64_t epoch) {
	if (!s || epoch == 0 || s->epoch != epoch ||
			(s->state != SD_NEWS_SUPERVISOR_RUNNING &&
			 s->state != SD_NEWS_SUPERVISOR_STOP_REQUESTED) ||
			s->pidfd < 0) return false;
#ifdef SYS_pidfd_send_signal
	/* Kernel-pinned descriptor avoids ever signaling a reused PID. */
	if (syscall(SYS_pidfd_send_signal, s->pidfd, SIGTERM, NULL, 0) != 0) {
		/* ESRCH means the child already exited. Caller must still reap. */
		if (errno != ESRCH) return false;
	}
	s->state = SD_NEWS_SUPERVISOR_STOP_REQUESTED;
	return true;
#else
	return false;
#endif
}

bool stage_director_news_supervisor_poll(
		struct stage_director_news_supervisor *s, uint64_t epoch) {
	if (!s || epoch == 0 || s->epoch != epoch ||
			(s->state != SD_NEWS_SUPERVISOR_RUNNING &&
			 s->state != SD_NEWS_SUPERVISOR_STOP_REQUESTED) ||
			s->child_pid <= 0) return false;
	int status = 0;
	const pid_t got = waitpid(s->child_pid, &status, WNOHANG);
	if (got == 0) return true;
	if (got == s->child_pid) {
		s->wait_status = status;
		s->state = SD_NEWS_SUPERVISOR_EXITED;
		return true;
	}
	/*
	 * ECHILD or any unexpected result is never 'cleanly reaped' by us.
	 * Keep descriptor/ownership record and refuse reuse for investigation.
	 */
	s->state = SD_NEWS_SUPERVISOR_LOST_REAPER;
	return false;
}

bool stage_director_news_supervisor_finish(
		struct stage_director_news_supervisor *s, uint64_t epoch) {
	if (!s || epoch == 0 || s->epoch != epoch ||
			s->state != SD_NEWS_SUPERVISOR_EXITED ||
			s->pidfd < 0) return false;
	close(s->pidfd);
	const uint64_t floor = s->last_epoch;
	memset(s, 0, sizeof(*s));
	s->pidfd = -1;
	s->last_epoch = floor;
	return true;
}

#ifdef SD_STAGE_DIRECTOR_TEST_HELPER
/*
 * The only execution path is build-gated for CI, hardcoded /usr/bin/sleep.
 * No user-controlled executable, URL, environment or browser invocation.
 * Production compilation excludes this function entirely.
 */
bool stage_director_news_supervisor_start_test_child(
		struct stage_director_news_supervisor *s,
		uint64_t epoch, uintptr_t host_ref) {
	if (!s || s->state != SD_NEWS_SUPERVISOR_EMPTY ||
			s->pidfd != -1 || epoch == 0 || epoch <= s->last_epoch ||
			host_ref == 0) return false;
#if !defined(SYS_pidfd_open) || !defined(SYS_pidfd_send_signal)
	return false;
#else
	char *const argv[] = {"/usr/bin/sleep", "30", NULL};
	pid_t pid = -1;
	const int rc = posix_spawn(&pid, "/usr/bin/sleep",
		NULL, NULL, argv, environ);
	if (rc != 0 || pid <= 0) return false;
	const int descriptor = (int)syscall(SYS_pidfd_open, pid, 0);
	if (descriptor < 0) {
		/* Unreaped direct child cannot have its PID recycled. */
		(void)kill(pid, SIGKILL);
		(void)waitpid(pid, NULL, 0);
		return false;
	}
	struct stage_director_process_identity identity = {0};
	const bool verified = stage_director_read_linux_process(
		pid, &identity, NULL) && identity.pid == pid &&
		identity.start_time_ticks != 0 && identity.uid == getuid() &&
		pidfd_alive(descriptor);
	if (!verified) {
		(void)syscall(SYS_pidfd_send_signal, descriptor, SIGKILL, NULL, 0);
		(void)waitpid(pid, NULL, 0);
		close(descriptor);
		return false;
	}
	s->child_pid = pid;
	s->child_start_ticks = identity.start_time_ticks;
	s->pidfd = descriptor;
	s->host_ref = host_ref;
	s->epoch = epoch;
	s->last_epoch = epoch;
	s->state = SD_NEWS_SUPERVISOR_RUNNING;
	return true;
#endif
}
#endif
