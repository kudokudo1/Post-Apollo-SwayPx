#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sway/stage_director_company_lineage.h"

struct fixture {
	struct stage_director_process_identity list[8];
	int count;
	int64_t denied_pid;
	int64_t unstable_pid;
	int unstable_after;
	int reads_of_unstable;
};

static bool read_fixture(int64_t pid,
		struct stage_director_process_identity *out, void *context) {
	struct fixture *f = context;
	if (!f || !out || pid == f->denied_pid) return false;
	for (int i = 0; i < f->count; i++) {
		if (f->list[i].pid != pid) continue;
		*out = f->list[i];
		if (pid == f->unstable_pid &&
				++f->reads_of_unstable > f->unstable_after) {
			out->parent_pid++;
		}
		return true;
	}
	return false;
}

static struct fixture fixture(void) {
	struct fixture f = {0};
	f.list[0] = (struct stage_director_process_identity) {
		.pid=1000, .parent_pid=1, .start_time_ticks=20000, .uid=1001
	};
	f.list[1] = (struct stage_director_process_identity) {
		.pid=1001, .parent_pid=1000, .start_time_ticks=20010, .uid=1001
	};
	f.list[2] = (struct stage_director_process_identity) {
		.pid=2001, .parent_pid=20, .start_time_ticks=20011, .uid=1001
	};
	f.count=3;
	return f;
}

static struct stage_director_company_native_host host(void) {
	return (struct stage_director_company_native_host) {
		.state = SD_COMPANY_NATIVE_CANDIDATE_ONLY,
		.candidate = true,
		.native_layer_ref = 0x11,
		.native_layer_role_ref = 0x12,
		.native_surface_ref = 0x13,
		.native_client_ref = 0x14,
		.native_output_ref = 0x15,
		.pid = 1000, .uid = 1001, .start_ticks = 20000,
	};
}
static struct stage_director_company_native_actor actor(void) {
	return (struct stage_director_company_native_actor) {
		.state = SD_COMPANY_NATIVE_CANDIDATE_ONLY,
		.candidate = true,
		.native_view_ref = 0x21,
		.native_root_surface_ref = 0x22,
		.native_client_ref = 0x23,
		.pid = 1001, .uid = 1001, .start_ticks = 20010,
	};
}

static struct stage_director_company_lineage_result check(
		struct fixture *f,
		struct stage_director_company_native_host *h,
		struct stage_director_company_native_actor *a,
		enum stage_director_company_lineage_status expected) {
	struct stage_director_company_lineage_result r =
		stage_director_company_lineage_probe(
			h, a, read_fixture, f);
	if (r.status != expected) {
		fprintf(stderr,"M22 expected=%s actual=%s\n",
			stage_director_company_lineage_status_name(expected),
			stage_director_company_lineage_status_name(r.status));
	}
	assert(r.status == expected);
	assert(r.direct_child_candidate ==
		(expected == SD_COMPANY_LINEAGE_CANDIDATE_ONLY));
	assert(!r.owner_verified && !r.can_launch &&
		!r.can_pair_actor && !r.can_render && !r.can_clip &&
		!r.can_accept_input && !r.can_focus);
	return r;
}

static void direct_exec_child_candidate_no_privileges(void) {
	struct fixture f = fixture();
	struct stage_director_company_native_host h = host();
	struct stage_director_company_native_actor a = actor();
	check(&f, &h, &a, SD_COMPANY_LINEAGE_CANDIDATE_ONLY);
	assert(strcmp(stage_director_company_lineage_status_name(
		SD_COMPANY_LINEAGE_CANDIDATE_ONLY), "CANDIDATE_ONLY") == 0);
	/* The Python execv chain does not change PID or start ticks. Tests
	 * here represent observed kernel tuples, not fake birth authority. */
}

static void wrong_peer_and_spoofed_names_fail(void) {
	struct fixture f = fixture();
	struct stage_director_company_native_host h = host();
	struct stage_director_company_native_actor a = actor();
	a.native_client_ref=h.native_client_ref;
	check(&f, &h, &a, SD_COMPANY_LINEAGE_WRONG_NATIVE_PEER);
	a=actor(); a.uid++;
	check(&f, &h, &a, SD_COMPANY_LINEAGE_WRONG_NATIVE_PEER);
	a=actor(); a.candidate=false;
	check(&f, &h, &a, SD_COMPANY_LINEAGE_WRONG_NATIVE_PEER);
	a=actor(); a.state=SD_COMPANY_NATIVE_WRONG_ROLE;
	check(&f, &h, &a, SD_COMPANY_LINEAGE_WRONG_NATIVE_PEER);
	a=actor(); a.native_root_surface_ref=0;
	check(&f, &h, &a, SD_COMPANY_LINEAGE_WRONG_NATIVE_PEER);
	a=actor(); h.state=SD_COMPANY_NATIVE_UNMAPPED;
	check(&f, &h, &a, SD_COMPANY_LINEAGE_WRONG_NATIVE_PEER);
	h=host();
	check(&f, &h, &a, SD_COMPANY_LINEAGE_CANDIDATE_ONLY);
	assert(stage_director_company_lineage_probe(
		NULL, &a, read_fixture, &f).status==SD_COMPANY_LINEAGE_INVALID);
	assert(stage_director_company_lineage_probe(
		&h, &a, NULL, &f).status==SD_COMPANY_LINEAGE_INVALID);
}

static void unrelated_and_grandchildren_fail(void) {
	struct fixture f = fixture();
	struct stage_director_company_native_host h = host();
	struct stage_director_company_native_actor a = actor();
	/* A manual Kitty-launched Qt process has the right UID but not QS
	 * parentage. Exact PID and advertised app_id are insufficient. */
	a.pid=2001; a.start_ticks=20011;
	check(&f, &h, &a, SD_COMPANY_LINEAGE_NOT_DIRECT_CHILD);
	/* A random grandchild of Quickshell is also not the exact Process
	 * started from Company, even though ancestry could reach QS. */
	f.list[1].parent_pid=1000;
	f.list[2].parent_pid=1001;
	check(&f, &h, &a, SD_COMPANY_LINEAGE_NOT_DIRECT_CHILD);
}

static void reused_and_terminated_processes_fail(void) {
	struct fixture f = fixture();
	struct stage_director_company_native_host h = host();
	struct stage_director_company_native_actor a = actor();
	f.list[1].start_time_ticks++;
	check(&f, &h, &a, SD_COMPANY_LINEAGE_PROCESS_CHANGED);
	f=fixture();
	f.list[0].start_time_ticks++;
	check(&f, &h, &a, SD_COMPANY_LINEAGE_PROCESS_CHANGED);
	f=fixture();
	f.list[1].uid++;
	check(&f, &h, &a, SD_COMPANY_LINEAGE_PROCESS_CHANGED);
	f=fixture();
	f.denied_pid=1001;
	check(&f, &h, &a, SD_COMPANY_LINEAGE_READ_FAILED);
	f=fixture();
	f.unstable_pid=1001;
	f.unstable_after=1;
	check(&f, &h, &a, SD_COMPANY_LINEAGE_PROCESS_CHANGED);
	f=fixture();
	f.unstable_pid=1000;
	f.unstable_after=1;
	check(&f, &h, &a, SD_COMPANY_LINEAGE_PROCESS_CHANGED);
	f=fixture();
	f.list[1].start_time_ticks=19999;
	a.start_ticks=19999;
	check(&f, &h, &a, SD_COMPANY_LINEAGE_NOT_DIRECT_CHILD);
}

int main(void) {
	direct_exec_child_candidate_no_privileges();
	wrong_peer_and_spoofed_names_fail();
	unrelated_and_grandchildren_fail();
	reused_and_terminated_processes_fail();
	puts("STAGE_DIRECTOR_COMPANY_M22: PASS direct child evidence, authority=NONE");
	return 0;
}
