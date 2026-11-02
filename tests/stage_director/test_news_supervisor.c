#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "sway/stage_director_news_supervisor.h"

static void check_no_authority(struct stage_director_news_supervisor *s,
		enum stage_director_news_supervisor_state state) {
	const struct stage_director_news_supervisor_result r =
		stage_director_news_supervisor_inspect(s);
	assert(r.state == state);
	assert(!r.owner_verified);
	assert(!r.can_launch_browser);
	assert(!r.can_clip);
	assert(!r.can_accept_input);
}

int main(void) {
	struct stage_director_news_supervisor s;
	stage_director_news_supervisor_init(&s);
	assert(s.pidfd == -1 && s.state == SD_NEWS_SUPERVISOR_EMPTY);
	check_no_authority(&s, SD_NEWS_SUPERVISOR_EMPTY);
	assert(!stage_director_news_supervisor_start_test_child(&s, 0, 123));
	assert(!stage_director_news_supervisor_start_test_child(&s, 1, 0));
	assert(!stage_director_news_supervisor_request_stop(&s, 1));
	assert(!stage_director_news_supervisor_finish(&s, 1));
	assert(stage_director_news_supervisor_start_test_child(&s, 1, 123));
	assert(s.child_pid > 0 && s.child_start_ticks > 0);
	assert(s.pidfd >= 0);
	assert((fcntl(s.pidfd, F_GETFD) & FD_CLOEXEC) != 0);
	check_no_authority(&s, SD_NEWS_SUPERVISOR_RUNNING);
	assert(stage_director_news_supervisor_inspect(&s).child_may_be_running);
	assert(!stage_director_news_supervisor_start_test_child(&s, 2, 123));
	assert(!stage_director_news_supervisor_request_stop(&s, 2));
	assert(!stage_director_news_supervisor_poll(&s, 2));
	assert(!stage_director_news_supervisor_finish(&s, 1));

	const pid_t original = s.child_pid;
	assert(stage_director_news_supervisor_request_stop(&s, 1));
	check_no_authority(&s, SD_NEWS_SUPERVISOR_STOP_REQUESTED);
	/* A compositor must schedule nonblocking polls, never wait synchronously.
	 * Here the disposable CI test has a bounded 2-second retry loop. */
	bool reaped = false;
	for (int i = 0; i < 200; ++i) {
		assert(stage_director_news_supervisor_poll(&s, 1));
		if (s.state == SD_NEWS_SUPERVISOR_EXITED) {
			reaped = true;
			break;
		}
		const struct timespec pause = {.tv_nsec = 10000000L};
		(void)nanosleep(&pause, NULL);
	}
	assert(reaped);
	check_no_authority(&s, SD_NEWS_SUPERVISOR_EXITED);
	assert(stage_director_news_supervisor_inspect(&s).process_reaped);
	assert(WIFSIGNALED(s.wait_status) || WIFEXITED(s.wait_status));
	assert(waitpid(original, NULL, WNOHANG) == -1 && errno == ECHILD);
	assert(stage_director_news_supervisor_finish(&s, 1));
	assert(s.pidfd == -1 && s.child_pid == 0 && s.last_epoch == 1);
	assert(!stage_director_news_supervisor_finish(&s, 1));
	assert(!stage_director_news_supervisor_start_test_child(&s, 1, 123));

	assert(stage_director_news_supervisor_start_test_child(&s, 2, 123));
	assert(stage_director_news_supervisor_request_stop(&s, 2));
	reaped = false;
	for (int i = 0; i < 200; ++i) {
		assert(stage_director_news_supervisor_poll(&s, 2));
		if (s.state == SD_NEWS_SUPERVISOR_EXITED) {
			reaped = true; break;
		}
		const struct timespec pause = {.tv_nsec = 10000000L};
		(void)nanosleep(&pause, NULL);
	}
	assert(reaped && stage_director_news_supervisor_finish(&s, 2));
	check_no_authority(&s, SD_NEWS_SUPERVISOR_EMPTY);
	puts("Stage Director NEWS test-only supervised child lifecycle: PASS");
	return 0;
}
