#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* CI disposable browser-like multiprocess fixture, not a browser.
 * The direct child and its descendant share the newly spawned process
 * group. No daemonization/setsid, no network, no arbitrary payload.
 */
int main(void) {
	const char *ci = getenv("CI");
	const char *gha = getenv("GITHUB_ACTIONS");
	const char *temp = getenv("RUNNER_TEMP");
	const char *ready = getenv("SD_CI_TREE_READY_FILE");
	if (!ci || strcmp(ci,"true") || !gha || strcmp(gha,"true") ||
			!temp || !*temp || !ready ||
			strncmp(ready,temp,strlen(temp)) ||
			ready[strlen(temp)] != '/') {
		fputs("BLOCKED_NOT_DISPOSABLE_TREE_ACTOR\n",stderr);
		return 2;
	}
	if (getpgrp() != getpid()) {
		fputs("FAIL: actor not a new group leader\n",stderr);
		return 1;
	}
	const pid_t child = fork();
	if (child < 0) return 1;
	if (child == 0) {
		if (getpgrp() != getppid()) _exit(3);
		for (;;) pause();
	}
	/* One private metadata file in the runner temp. No other path writes. */
	const int fd = open(ready,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);
	if (fd < 0) return 1;
	char message[96];
	const int len = snprintf(message,sizeof(message),"%ld\n",(long)child);
	if (len <= 0 || (size_t)len >= sizeof(message) ||
			write(fd,message,(size_t)len) != len) {
		close(fd);
		return 1;
	}
	close(fd);
	for (;;) pause();
}
