#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "sway/stage_director_process_evidence.h"

struct fixture {
	struct stage_director_process_identity items[16];
	size_t count;
	int64_t unstable_pid;
	int unstable_after_read;
	int unstable_reads;
	int64_t denied_pid;
};
static bool read_fixture(int64_t pid,
		struct stage_director_process_identity *out, void *userdata) {
	struct fixture *f = userdata;
	if (pid == f->denied_pid) return false;
	for (size_t i = 0; i < f->count; ++i) {
		if (f->items[i].pid == pid) {
			*out = f->items[i];
			if (pid == f->unstable_pid &&
				++f->unstable_reads > f->unstable_after_read) {
				out->start_time_ticks++;
			}
			return true;
		}
	}
	return false;
}

static struct fixture valid(void) {
	struct fixture f = {0};
	f.items[0] = (struct stage_director_process_identity){100, 2, 1000, 1001};
	f.items[1] = (struct stage_director_process_identity){101, 100, 1001, 1001};
	f.items[2] = (struct stage_director_process_identity){102, 101, 1002, 1001};
	f.items[3] = (struct stage_director_process_identity){200, 2, 998, 1001};
	f.count = 4;
	return f;
}
static struct stage_director_process_launch_root root(void) {
	return (struct stage_director_process_launch_root){100, 1000, 1001};
}
static void assert_no_authority(struct stage_director_process_evidence_result r,
		enum stage_director_process_evidence_state expected, bool match) {
	assert(r.state == expected);
	assert(r.lineage_candidate == match);
	assert(!r.owner_verified && !r.can_bind_viewer &&
		!r.can_launch && !r.can_clip && !r.can_accept_input);
}
static struct stage_director_process_evidence_result check(
		struct fixture *f, struct stage_director_process_launch_root *launch,
		int64_t actor, uint64_t start) {
	return stage_director_process_check(launch, actor, start,
		read_fixture, f);
}

static void test_lineage_and_unrelated(void) {
	struct fixture f = valid();
	struct stage_director_process_launch_root r = root();
	assert_no_authority(check(&f,&r,100,1000),
		SD_PROCESS_EVIDENCE_LINEAGE_CANDIDATE,true);
	assert_no_authority(check(&f,&r,101,1001),
		SD_PROCESS_EVIDENCE_LINEAGE_CANDIDATE,true);
	assert_no_authority(check(&f,&r,102,1002),
		SD_PROCESS_EVIDENCE_LINEAGE_CANDIDATE,true);
	assert_no_authority(check(&f,&r,200,998),
		SD_PROCESS_EVIDENCE_UNRELATED,false);
	assert_no_authority(check(&f,&r,102,999),
		SD_PROCESS_EVIDENCE_ACTOR_MISMATCH,false);
	r.start_time_ticks++;
	assert_no_authority(check(&f,&r,102,1002),
		SD_PROCESS_EVIDENCE_ROOT_MISMATCH,false);
	r=root();r.uid++;
	assert_no_authority(check(&f,&r,102,1002),
		SD_PROCESS_EVIDENCE_ROOT_MISMATCH,false);
	assert_no_authority(stage_director_process_check(NULL,102,1002,read_fixture,&f),
		SD_PROCESS_EVIDENCE_INVALID,false);
	assert_no_authority(stage_director_process_check(&r,102,1002,NULL,&f),
		SD_PROCESS_EVIDENCE_INVALID,false);
}

static void test_no_spoof_or_reparent(void) {
	struct fixture f = valid();
	struct stage_director_process_launch_root r = root();
	f.items[1].uid = 1002;
	assert_no_authority(check(&f,&r,102,1002),
		SD_PROCESS_EVIDENCE_UNRELATED,false);
	f=valid();
	f.items[1].parent_pid=102; /* cycle: 102 -> 101 -> 102 */
	f.items[1].start_time_ticks=1002; /* make ancestry age check neutral */
	assert_no_authority(check(&f,&r,102,1002),
		SD_PROCESS_EVIDENCE_UNRELATED,false);
	f=valid();
	f.items[1].parent_pid=200;
	assert_no_authority(check(&f,&r,102,1002),
		SD_PROCESS_EVIDENCE_UNRELATED,false);
	f=valid();
	f.items[1].start_time_ticks=1003; /* parent newer than child */
	assert_no_authority(check(&f,&r,102,1002),
		SD_PROCESS_EVIDENCE_UNSTABLE,false);
	f=valid();
	f.denied_pid=101;
	assert_no_authority(check(&f,&r,102,1002),
		SD_PROCESS_EVIDENCE_READ_FAILED,false);
	f=valid();
	f.unstable_pid=102;f.unstable_after_read=1;
	assert_no_authority(check(&f,&r,102,1002),
		SD_PROCESS_EVIDENCE_UNSTABLE,false);
	f=valid();
	f.unstable_pid=100;f.unstable_after_read=1;
	assert_no_authority(check(&f,&r,102,1002),
		SD_PROCESS_EVIDENCE_UNSTABLE,false);
	f=valid();
	f.items[2].pid=999;
	assert_no_authority(check(&f,&r,102,1002),
		SD_PROCESS_EVIDENCE_READ_FAILED,false);
}

static void test_depth_limit(void) {
	struct fixture f = {0};
	struct stage_director_process_launch_root r = {100,1000,1001};
	f.count = 16;
	for (int i=0; i<16; ++i) {
		f.items[i] = (struct stage_director_process_identity) {
			.pid=100+i, .parent_pid=101+i,
			.start_time_ticks=(i == 0 ? 1000 : (uint64_t)(2016-i)),
			.uid=1001
		};
	}
	/* Reverse link to root but don't allow arbitrarily long ancestry. */
	f.items[15].parent_pid=100;
	assert_no_authority(check(&f,&r,101,1001),
		SD_PROCESS_EVIDENCE_UNRELATED,false);
}

int main(void) {
	test_lineage_and_unrelated();
	test_no_spoof_or_reparent();
	test_depth_limit();
	assert(strcmp(stage_director_process_evidence_name(
		SD_PROCESS_EVIDENCE_LINEAGE_CANDIDATE),"LINEAGE_CANDIDATE")==0);
	puts("Stage Director NEWS Linux lineage and PID reuse evidence: PASS");
	return 0;
}
