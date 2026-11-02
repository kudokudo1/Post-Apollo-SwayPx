#define _GNU_SOURCE
#include <errno.h>
#include <stdlib.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#if defined(SD_STAGE_DIRECTOR_TEST_HELPER) || defined(SD_STAGE_DIRECTOR_TEST_TREE)
#include <spawn.h>
extern char **environ;
#endif
#include "sway/stage_director_process_evidence.h"
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
	/* A browser-like child tree must be stopped as its OWN process group,
	 * not merely by a pidfd signal to the leader. The direct launch root
	 * remains unreaped, preventing process group ID reuse at this point.
	 * Recheck kernel start/uid and process-group leader before signaling.
	 */
	if (s->owned_pgid > 0) {
		struct stage_director_process_identity now = {0};
		if (s->owned_pgid != s->child_pid ||
				!pidfd_alive(s->pidfd) ||
				!stage_director_read_linux_process(
					s->child_pid, &now, NULL) ||
				now.pid != s->child_pid ||
				now.start_time_ticks != s->child_start_ticks ||
				now.uid != s->child_uid ||
				getpgid(s->child_pid) != s->owned_pgid) {
			return false;
		}
		if (kill(-s->owned_pgid, SIGTERM) != 0) {
			return false;
		}
		s->group_stop_sent = true;
		s->state = SD_NEWS_SUPERVISOR_STOP_REQUESTED;
		return true;
	}
	/* Existing single-child method stays strictly pidfd targeted. */
	if (syscall(SYS_pidfd_send_signal, s->pidfd, SIGTERM, NULL, 0) != 0) {
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
			s->pidfd < 0 ||
			(s->owned_pgid > 0 && !s->group_stop_sent)) return false;
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
	s->child_uid = identity.uid;
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


#ifdef SD_STAGE_DIRECTOR_TEST_TREE
/*
 * CI-only multi-process tree launcher. This executable is compiled into the
 * test harness, not accepted from IPC, QML, a runtime source ID or an
 * environment-specified argv. The real-browser publisher is NOT wired.
 */
#ifndef SD_CI_TREE_ACTOR_PATH
#error SD_STAGE_DIRECTOR_TEST_TREE requires a compiled fixed actor pathname
#endif
static bool tree_test_ci_only(void) {
	const char *ci = getenv("CI");
	const char *gha = getenv("GITHUB_ACTIONS");
	const char *runner = getenv("RUNNER_TEMP");
	const char *path = SD_CI_TREE_ACTOR_PATH;
	return ci && strcmp(ci, "true") == 0 &&
		gha && strcmp(gha, "true") == 0 &&
		runner && *runner &&
		strlen(path) > strlen(runner) + 1 &&
		strncmp(path, runner, strlen(runner)) == 0 &&
		path[strlen(runner)] == '/' &&
		path[0] == '/';
}

bool stage_director_news_supervisor_start_test_tree(
		struct stage_director_news_supervisor *s,
		uint64_t epoch, uintptr_t host_ref) {
	if (!s || !tree_test_ci_only() ||
			s->state != SD_NEWS_SUPERVISOR_EMPTY ||
			s->pidfd != -1 || epoch == 0 || epoch <= s->last_epoch ||
			host_ref == 0) return false;
#if !defined(SYS_pidfd_open) || !defined(SYS_pidfd_send_signal)
	return false;
#else
	posix_spawnattr_t attr;
	if (posix_spawnattr_init(&attr) != 0) return false;
	/* pgid=0 means the spawned process itself becomes group leader. */
	bool attrs = posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETPGROUP) == 0 &&
		posix_spawnattr_setpgroup(&attr, 0) == 0;
	if (!attrs) {
		posix_spawnattr_destroy(&attr);
		return false;
	}
	char *const argv[] = {(char *)SD_CI_TREE_ACTOR_PATH, NULL};
	pid_t pid = -1;
	const int result = posix_spawn(&pid, SD_CI_TREE_ACTOR_PATH,
		NULL, &attr, argv, environ);
	posix_spawnattr_destroy(&attr);
	if (result != 0 || pid <= 0) return false;
	const int descriptor = (int)syscall(SYS_pidfd_open, pid, 0);
	struct stage_director_process_identity identity = {0};
	const bool good = descriptor >= 0 &&
		stage_director_read_linux_process(pid, &identity, NULL) &&
		identity.pid == pid && identity.uid == getuid() &&
		identity.start_time_ticks != 0 &&
		getpgid(pid) == pid &&
		pidfd_alive(descriptor);
	if (!good) {
		/* The direct child is still unreaped. Group ID cannot be recycled. */
		(void)kill(-pid, SIGKILL);
		(void)kill(pid, SIGKILL);
		(void)waitpid(pid, NULL, 0);
		if (descriptor >= 0) close(descriptor);
		return false;
	}
	s->state = SD_NEWS_SUPERVISOR_RUNNING;
	s->epoch = epoch;
	s->last_epoch = epoch;
	s->host_ref = host_ref;
	s->child_pid = pid;
	s->owned_pgid = pid;
	s->child_start_ticks = identity.start_time_ticks;
	s->child_uid = identity.uid;
	s->pidfd = descriptor;
	return true;
#endif
}
#endif
