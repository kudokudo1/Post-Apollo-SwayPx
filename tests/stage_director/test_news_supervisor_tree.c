#define _GNU_SOURCE
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "sway/stage_director_news_supervisor.h"

/* A Linux subreaper lets CI obtain and reap the fixture's grandchild after
 * its direct parent exits; this does not grant such power to SwayPX.
 */
static bool wait_for_ready(const char *path,pid_t *descendant) {
	for (int i=0;i<400;++i) {
		FILE *f = fopen(path,"r");
		if (f) {
			long value = 0;
			const bool good = fscanf(f,"%ld",&value)==1 &&
				value > 1 && value <= INT32_MAX;
			fclose(f);
			if (!good) return false;
			*descendant = (pid_t)value;
			return true;
		}
		const struct timespec pause_time={.tv_nsec=10000000L};
		nanosleep(&pause_time,NULL);
	}
	return false;
}

int main(void) {
	const char *ci=getenv("CI"), *gha=getenv("GITHUB_ACTIONS");
	const char *temp=getenv("RUNNER_TEMP");
	if (!ci || strcmp(ci,"true") || !gha || strcmp(gha,"true") ||
			!temp || !*temp) return 2;
	alarm(20);
	assert(prctl(PR_SET_CHILD_SUBREAPER,1)==0);
	char ready[PATH_MAX];
	assert(snprintf(ready,sizeof(ready),"%s/sd-supervisor-tree-%ld.ready",
		temp,(long)getpid()) > 0);
	assert(setenv("SD_CI_TREE_READY_FILE",ready,1)==0);
	unlink(ready);
	struct stage_director_news_supervisor s;
	stage_director_news_supervisor_init(&s);
	assert(!stage_director_news_supervisor_start_test_tree(&s,0,0xA11));
	assert(!stage_director_news_supervisor_start_test_tree(&s,1,0));
	assert(stage_director_news_supervisor_start_test_tree(&s,1,0xA11));
	const pid_t root=s.child_pid;
	assert(root>1 && s.owned_pgid==root && s.pidfd>=0 &&
		s.child_start_ticks!=0 && s.child_uid==getuid());
	assert((fcntl(s.pidfd,F_GETFD)&FD_CLOEXEC)!=0);
	assert(s.state==SD_NEWS_SUPERVISOR_RUNNING);
	assert(!stage_director_news_supervisor_start_test_tree(&s,2,0xA11));
	pid_t descendant=-1;
	assert(wait_for_ready(ready,&descendant));
	assert(descendant>1 && descendant!=root);
	assert(getpgid(descendant)==root);
	assert(!stage_director_news_supervisor_request_stop(&s,2));
	assert(!stage_director_news_supervisor_finish(&s,1));
	const struct stage_director_news_supervisor_result running =
		stage_director_news_supervisor_inspect(&s);
	assert(running.child_may_be_running && !running.owner_verified &&
		!running.can_launch_browser && !running.can_clip &&
		!running.can_accept_input);
	assert(stage_director_news_supervisor_request_stop(&s,1));
	assert(s.group_stop_sent && s.state==SD_NEWS_SUPERVISOR_STOP_REQUESTED);
	bool direct_reaped=false;
	for (int i=0;i<300;++i) {
		assert(stage_director_news_supervisor_poll(&s,1));
		if (s.state==SD_NEWS_SUPERVISOR_EXITED) {
			direct_reaped=true;
			break;
		}
		const struct timespec t={.tv_nsec=10000000L};
		nanosleep(&t,NULL);
	}
	assert(direct_reaped);
	/* Grandchild is adopted by the CI-only subreaper. */
	int status=0;
	bool descendant_reaped=false;
	for (int i=0;i<300;++i) {
		pid_t got=waitpid(descendant,&status,WNOHANG);
		if (got==descendant) {
			assert(WIFSIGNALED(status) && WTERMSIG(status)==SIGTERM);
			descendant_reaped=true;
			break;
		}
		assert(got==0 || (got<0 && errno==ECHILD));
		const struct timespec t={.tv_nsec=10000000L};
		nanosleep(&t,NULL);
	}
	assert(descendant_reaped);
	assert(!stage_director_news_supervisor_request_stop(&s,1));
	assert(stage_director_news_supervisor_finish(&s,1));
	assert(s.pidfd==-1 && s.owned_pgid==0 &&
		s.state==SD_NEWS_SUPERVISOR_EMPTY && s.last_epoch==1);
	assert(!stage_director_news_supervisor_start_test_tree(&s,1,0xA11));
	unlink(ready);
	puts("SD_CI_TREE_SUPERVISOR: held_pidfd=1 exact_group_leader=1 "
		"direct_child_reaped=1 descendant_SIGTERM_reaped=1 "
		"stale_epoch_denied=1 browser=OFF scene=OFF authority=NONE PASS");
	return 0;
}
