#define _GNU_SOURCE
#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "sway/stage_director_company_birth_pidfd.h"

/* Uses a REAL Linux forked child and REAL kernel pidfd. Synthetic objects
 * below represent compositor-observed host/actor references, which are
 * tested separately in M20. This is NOT a live Wayland launch proof. */
static int open_real_pidfd(pid_t pid) {
#if defined(SYS_pidfd_open)
	return (int)syscall(SYS_pidfd_open, pid, 0);
#else
	(void)pid;
	return -1;
#endif
}
static void no_authority(struct stage_director_company_birth_result r) {
	assert(!r.exact_executable_attested && !r.launch_authenticated &&
		!r.owner_verified && !r.can_pair_actor && !r.can_render &&
		!r.can_clip && !r.can_accept_input && !r.can_focus &&
		!r.can_show_popup);
}
static struct stage_director_company_native_host host_native(
		struct stage_director_process_identity parent) {
	return (struct stage_director_company_native_host) {
		.candidate = true, .state = SD_COMPANY_NATIVE_CANDIDATE_ONLY,
		.native_layer_ref = 0x111, .native_layer_role_ref = 0x112,
		.native_surface_ref = 0x113, .native_client_ref = 0x114,
		.native_output_ref = 0x115, .pid = parent.pid,
		.uid = parent.uid, .start_ticks = parent.start_time_ticks,
	};
}
static struct stage_director_company_native_actor actor_native(
		struct stage_director_process_identity kid) {
	return (struct stage_director_company_native_actor) {
		.candidate = true, .state = SD_COMPANY_NATIVE_CANDIDATE_ONLY,
		.native_view_ref = 0x221, .native_root_surface_ref = 0x222,
		.native_client_ref = 0x223, .pid = kid.pid,
		.uid = kid.uid, .start_ticks = kid.start_time_ticks,
	};
}
static struct stage_director_company_intent_snapshot intent(
		struct stage_director_company_native_host host,
		struct stage_director_process_identity kid) {
	return (struct stage_director_company_intent_snapshot) {
		.present = true, .selection_epoch = 9, .sequence = 12,
		.observed_boot_ticks = kid.start_time_ticks,
		.host_revision = 3, .native_host_client_ref = host.native_client_ref,
		.mode = 0,
	};
}
static struct stage_director_company_lineage_result lineage(void) {
	return (struct stage_director_company_lineage_result) {
		.status = SD_COMPANY_LINEAGE_CANDIDATE_ONLY,
		.direct_child_candidate = true,
	};
}

static void real_kernel_pidfd_lifecycle(void) {
	struct stage_director_process_identity parent = {0};
	assert(stage_director_read_linux_process(getpid(), &parent, NULL));
	assert(parent.uid == (uint32_t)getuid());
	assert(parent.uid != 0); // normal user-scoped desktop; no root compositor
	int pipefd[2] = {-1, -1};
	assert(pipe(pipefd) == 0);
	pid_t child = fork();
	assert(child >= 0);
	if (child == 0) {
		close(pipefd[1]);
		char byte = 0;
		if (read(pipefd[0], &byte, 1) < 0) _exit(2);
		close(pipefd[0]);
		_exit(0);
	}
	close(pipefd[0]);
	const int pidfd = open_real_pidfd(child);
	assert(pidfd >= 0); // Linux 5.3+ required for M28a
	struct stage_director_process_identity kid = {0};
	assert(stage_director_read_linux_process(child, &kid, NULL));
	assert(kid.parent_pid == parent.pid);
	const struct stage_director_company_native_host host =
		host_native(parent);
	const struct stage_director_company_native_actor actor =
		actor_native(kid);
	const struct stage_director_company_lineage_result l = lineage();
	struct stage_director_company_intent_snapshot i = intent(host, kid);
	struct stage_director_company_birth_pidfd held = {0};
	stage_director_company_birth_pidfd_init(&held);
	assert(held.initialized && held.fd == -1 && !held.held);

	/* Plain pipe is not a pidfd. A real kernel pidfd for another PID is
	 * not sufficient either. */
	struct stage_director_company_birth_result r =
		stage_director_company_birth_pidfd_record(&held, pipefd[1],
			child, &i, &host, stage_director_read_linux_process, NULL);
	assert(r.status == SD_COMPANY_BIRTH_INVALID_PIDFD);
	no_authority(r);
	r = stage_director_company_birth_pidfd_record(&held, pidfd,
		child + 500, &i, &host, stage_director_read_linux_process, NULL);
	assert(r.status == SD_COMPANY_BIRTH_WRONG_CHILD);
	no_authority(r);

	i.native_host_client_ref++;
	r = stage_director_company_birth_pidfd_record(&held, pidfd,
		child, &i, &host, stage_director_read_linux_process, NULL);
	assert(r.status == SD_COMPANY_BIRTH_NO_INTENT);
	i.native_host_client_ref--;
	i.selection_epoch = 0;
	r = stage_director_company_birth_pidfd_record(&held, pidfd,
		child, &i, &host, stage_director_read_linux_process, NULL);
	assert(r.status == SD_COMPANY_BIRTH_NO_INTENT);
	i.selection_epoch = 9;
	i.observed_boot_ticks = kid.start_time_ticks - 1000;
	r = stage_director_company_birth_pidfd_record(&held, pidfd,
		child, &i, &host, stage_director_read_linux_process, NULL);
	assert(r.status == SD_COMPANY_BIRTH_OUTSIDE_WINDOW);
	i.observed_boot_ticks = kid.start_time_ticks;

	r = stage_director_company_birth_pidfd_record(&held, pidfd,
		child, &i, &host, stage_director_read_linux_process, NULL);
	assert(r.status == SD_COMPANY_BIRTH_HELD_CANDIDATE_ONLY);
	assert(r.kernel_child_candidate && !r.mapped_actor_candidate);
	no_authority(r);
	assert(held.held && held.fd >= 3 && held.fd != pidfd);
	assert(held.pid == child && held.child_start_ticks == kid.start_time_ticks);

	/* The caller may close the *received* fd. The held CLOEXEC dup remains
	 * a genuine kernel handle to the same process generation. */
	close(pidfd);
	r = stage_director_company_birth_pidfd_match_actor(&held,
		&host, &actor, &l, stage_director_read_linux_process, NULL);
	assert(r.status == SD_COMPANY_BIRTH_ACTOR_CANDIDATE_ONLY);
	assert(r.kernel_child_candidate && r.mapped_actor_candidate);
	no_authority(r);
	struct stage_director_company_native_actor wrong = actor;
	wrong.native_client_ref = host.native_client_ref;
	r = stage_director_company_birth_pidfd_match_actor(&held,
		&host, &wrong, &l, stage_director_read_linux_process, NULL);
	assert(r.status == SD_COMPANY_BIRTH_WRONG_CHILD);
	no_authority(r);
	wrong = actor;
	wrong.start_ticks++;
	r = stage_director_company_birth_pidfd_match_actor(&held,
		&host, &wrong, &l, stage_director_read_linux_process, NULL);
	assert(r.status == SD_COMPANY_BIRTH_WRONG_CHILD);
	no_authority(r);
	struct stage_director_company_native_host stale = host;
	stale.start_ticks++;
	r = stage_director_company_birth_pidfd_match_actor(&held,
		&stale, &actor, &l, stage_director_read_linux_process, NULL);
	assert(r.status == SD_COMPANY_BIRTH_INVALID_NATIVE_HOST);
	no_authority(r);

	/* Child exits and is reaped. Process cannot be matched again. */
	close(pipefd[1]);
	int status = 0;
	assert(waitpid(child, &status, 0) == child);
	assert(WIFEXITED(status));
	r = stage_director_company_birth_pidfd_match_actor(&held,
		&host, &actor, &l, stage_director_read_linux_process, NULL);
	assert(r.status == SD_COMPANY_BIRTH_INVALID_PIDFD ||
		r.status == SD_COMPANY_BIRTH_PROCESS_CHANGED);
	no_authority(r);
	stage_director_company_birth_pidfd_revoke(&held);
	assert(held.initialized && !held.held && held.fd == -1);
	stage_director_company_birth_pidfd_revoke(&held);
}

int main(void) {
	real_kernel_pidfd_lifecycle();
	puts("STAGE_DIRECTOR_COMPANY_M28A: PASS real kernel pidfd, native child, exit and no-authority");
	return 0;
}
