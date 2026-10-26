#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "sway/stage_director_news_custody.h"

static void non_authority(struct stage_director_news_custody *lease,
		bool candidate, bool alive) {
	const struct stage_director_news_custody_result r =
		stage_director_news_custody_inspect(lease);
	assert(r.process_candidate == candidate);
	assert(r.process_alive == alive);
	assert(!r.owner_verified && !r.can_launch && !r.can_bind_viewer);
	assert(!r.can_clip && !r.can_accept_input);
}

int main(void) {
	struct stage_director_news_custody lease;
	stage_director_news_custody_init(&lease);
	assert(lease.pidfd == -1 && lease.state == SD_NEWS_CUSTODY_EMPTY);
	assert(!stage_director_news_custody_reserve(&lease, 1, 0x1234, false));
	assert(!stage_director_news_custody_reserve(&lease, 0, 0x1234, true));
	assert(!stage_director_news_custody_reserve(&lease, 1, 0, true));
	assert(stage_director_news_custody_reserve(&lease, 1, 0x1234, true));
	assert(!stage_director_news_custody_reserve(&lease, 2, 0x1234, true));
	assert(!stage_director_news_custody_capture(&lease, 0, NULL));
	non_authority(&lease, false, false);

	/* Only a harmless controlled test child, NEVER a browser process. */
	int fds[2];
	assert(pipe(fds) == 0);
	pid_t child = fork();
	assert(child >= 0);
	if (child == 0) {
		close(fds[1]);
		char byte;
		(void)read(fds[0], &byte, 1);
		close(fds[0]);
		_exit(0);
	}
	close(fds[0]);
	struct stage_director_process_identity observed = {0};
	assert(stage_director_read_linux_process(child, &observed, NULL));
	assert(observed.start_time_ticks != 0);
	const struct stage_director_process_launch_root expected = {
		.pid = child,
		.start_time_ticks = observed.start_time_ticks,
		.uid = observed.uid,
	};
	struct stage_director_process_launch_root bad = expected;
	bad.start_time_ticks++;
	assert(!stage_director_news_custody_capture(&lease, 1, &bad));
	assert(lease.pidfd == -1);
	bad = expected;
	bad.uid++;
	assert(!stage_director_news_custody_capture(&lease, 1, &bad));
	assert(!stage_director_news_custody_capture(&lease, 2, &expected));
	assert(stage_director_news_custody_capture(&lease, 1, &expected));
	assert(lease.pidfd >= 0);
	assert(fcntl(lease.pidfd, F_GETFD) & FD_CLOEXEC);
	non_authority(&lease, true, true);
	assert(!stage_director_news_custody_capture(&lease, 1, &expected));
	assert(!stage_director_news_custody_release(&lease, 2));
	non_authority(&lease, true, true);

	close(fds[1]);
	int status = 0;
	assert(waitpid(child, &status, 0) == child);
	assert(WIFEXITED(status));
	assert(WEXITSTATUS(status) == 0);
	non_authority(&lease, true, false);
	assert(stage_director_news_custody_release(&lease, 1));
	assert(lease.pidfd == -1 && lease.root.pid == 0);
	assert(lease.last_epoch == 1);
	assert(!stage_director_news_custody_release(&lease, 1));
	assert(!stage_director_news_custody_reserve(&lease, 1, 0x1234, true));
	assert(stage_director_news_custody_reserve(&lease, 2, 0x1234, true));
	assert(stage_director_news_custody_release(&lease, 2));
	non_authority(&lease, false, false);
	puts("Stage Director NEWS held pidfd lifecycle and fail-closed custody: PASS");
	return 0;
}
