#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sway/stage_director_viewer_session.h"

#define HOST ((uintptr_t)0x1234u)
#define ACTOR ((uintptr_t)0x5678u)
#define OTHER ((uintptr_t)0x99aau)
#define PID 5432
#define START 1654321u

static struct stage_director_rect host(void) {
	return (struct stage_director_rect){100, 40, 1000, 700};
}
static struct stage_director_rect output(void) {
	return (struct stage_director_rect){0, 0, 2560, 1440};
}
static struct stage_director_rect bay(void) {
	return (struct stage_director_rect){205, 120, 824, 494};
}
static struct stage_director_rect actor(void) {
	return (struct stage_director_rect){180, 105, 950, 630};
}
static struct stage_director_viewport_request snapshot(void) {
	return (struct stage_director_viewport_request) {
		.host = host(), .output = output(), .viewport = bay(),
		.actor = actor(), .host_mapped = true, .actor_mapped = true,
		.output_active = true, .actor_identity_verified = true,
	};
}
static bool reserve(struct stage_director_viewer_session *s, uint64_t epoch) {
	return stage_director_viewer_session_reserve(s, epoch, HOST,
		PID, START, host(), output(), bay());
}
static bool attach(struct stage_director_viewer_session *s, uint64_t epoch) {
	return stage_director_viewer_session_attach(s, epoch, HOST, ACTOR,
		PID, START, actor());
}
static void confirm_no_authority(struct stage_director_viewer_session *s,
		uint64_t epoch, uintptr_t host_ref, uintptr_t actor_ref,
		const struct stage_director_viewport_request *geometry,
		bool expected_candidate) {
	const struct stage_director_viewer_probe p =
		stage_director_viewer_session_probe(
			s, epoch, host_ref, actor_ref, geometry);
	assert(p.candidate_matched == expected_candidate);
	assert(!p.can_launch && !p.can_clip && !p.can_accept_input);
}

static void test_normal_and_replay(void) {
	struct stage_director_viewer_session s;
	stage_director_viewer_session_init(&s);
	assert(s.state == SD_VIEWER_EMPTY);
	assert(reserve(&s, 101));
	assert(s.state == SD_VIEWER_HOST_RESERVED);
	assert(!reserve(&s, 102)); /* no competing session while active */
	assert(!attach(&s, 100)); /* stale epoch */
	assert(!stage_director_viewer_session_attach(
		&s, 101, OTHER, ACTOR, PID, START, actor()));
	assert(!stage_director_viewer_session_attach(
		&s, 101, HOST, ACTOR, PID+1, START, actor()));
	assert(!stage_director_viewer_session_attach(
		&s, 101, HOST, ACTOR, PID, START+1, actor()));
	assert(!stage_director_viewer_session_attach(
		&s, 101, HOST, HOST, PID, START, actor()));
	assert(attach(&s, 101));
	assert(!attach(&s, 101)); /* can't hijack an attached actor */
	struct stage_director_viewport_request view = snapshot();
	confirm_no_authority(&s, 101, HOST, ACTOR, &view, true);
	confirm_no_authority(&s, 101, HOST, OTHER, &view, false);
	confirm_no_authority(&s, 100, HOST, ACTOR, &view, false);
	confirm_no_authority(&s, 101, OTHER, ACTOR, &view, false);
	assert(!stage_director_viewer_session_close(&s, 100));
	assert(stage_director_viewer_session_close(&s, 101));
	assert(s.state == SD_VIEWER_EMPTY);
	assert(s.host_ref == 0 && s.actor_ref == 0 && s.expected_pid == 0);
	assert(s.last_epoch == 101);
	assert(!stage_director_viewer_session_close(&s, 101));
	assert(!reserve(&s, 101));
	assert(!reserve(&s, 100));
	assert(reserve(&s, 102));
	assert(attach(&s, 102));
	confirm_no_authority(&s, 102, HOST, ACTOR, &view, true);
}

static void test_host_unmap_and_actor_exit(void) {
	struct stage_director_viewer_session s = {0};
	assert(reserve(&s, 1));
	assert(!stage_director_viewer_session_host_lost(&s, OTHER));
	assert(stage_director_viewer_session_host_lost(&s, HOST));
	assert(s.state == SD_VIEWER_EMPTY && s.last_epoch == 1);
	assert(!attach(&s, 1));
	assert(reserve(&s, 2));
	assert(attach(&s, 2));
	assert(!stage_director_viewer_session_actor_lost(&s, OTHER));
	assert(stage_director_viewer_session_actor_lost(&s, ACTOR));
	assert(s.state == SD_VIEWER_EMPTY && s.last_epoch == 2);
	assert(!reserve(&s, 2));
	assert(reserve(&s, 3));
	assert(attach(&s, 3));
	assert(stage_director_viewer_session_host_lost(&s, HOST));
	assert(!stage_director_viewer_session_actor_lost(&s, ACTOR));
	assert(s.actor_ref == 0);
}

static void test_resize_monitor_and_guard_loss(void) {
	struct stage_director_viewer_session s = {0};
	assert(reserve(&s, 300));
	assert(attach(&s, 300));
	struct stage_director_viewport_request view = snapshot();
	view.host.x += 2; /* no stale crop on host movement */
	confirm_no_authority(&s, 300, HOST, ACTOR, &view, false);
	view = snapshot();
	view.output.x = -1360;
	confirm_no_authority(&s, 300, HOST, ACTOR, &view, false);
	view = snapshot();
	view.actor.x += 1;
	confirm_no_authority(&s, 300, HOST, ACTOR, &view, false);
	view = snapshot();
	view.viewport.width--;
	confirm_no_authority(&s, 300, HOST, ACTOR, &view, false);
	view = snapshot();
	view.host_mapped = false;
	confirm_no_authority(&s, 300, HOST, ACTOR, &view, false);
	view = snapshot();
	view.actor_mapped = false;
	confirm_no_authority(&s, 300, HOST, ACTOR, &view, false);
	view = snapshot();
	view.output_active = false;
	confirm_no_authority(&s, 300, HOST, ACTOR, &view, false);
	view = snapshot();
	view.actor_identity_verified = false;
	confirm_no_authority(&s, 300, HOST, ACTOR, &view, false);
}

static void test_invalid_ticket_inputs(void) {
	struct stage_director_viewer_session s = {0};
	assert(!reserve(&s, 0));
	assert(!stage_director_viewer_session_reserve(
		&s, 1, 0, PID, START, host(), output(), bay()));
	assert(!stage_director_viewer_session_reserve(
		&s, 1, HOST, 0, START, host(), output(), bay()));
	assert(!stage_director_viewer_session_reserve(
		&s, 1, HOST, PID, 0, host(), output(), bay()));
	struct stage_director_rect bad = bay();
	bad.x = INT_MAX - 5;
	assert(!stage_director_viewer_session_reserve(
		&s, 1, HOST, PID, START, host(), output(), bad));
	bad = bay();
	bad.width = 0;
	assert(!stage_director_viewer_session_reserve(
		&s, 1, HOST, PID, START, host(), output(), bad));
	assert(s.state == SD_VIEWER_EMPTY && s.last_epoch == 0);
	assert(!stage_director_viewer_session_host_lost(&s, HOST));
	assert(!stage_director_viewer_session_actor_lost(&s, ACTOR));
	assert(!stage_director_viewer_session_close(&s, 1));
	assert(!stage_director_viewer_session_reserve(
		NULL, 1, HOST, PID, START, host(), output(), bay()));
	struct stage_director_viewport_request view = snapshot();
	confirm_no_authority(NULL, 1, HOST, ACTOR, &view, false);
}

int main(void) {
	test_normal_and_replay();
	test_host_unmap_and_actor_exit();
	test_resize_monitor_and_guard_loss();
	test_invalid_ticket_inputs();
	puts("Stage Director viewer session lifecycle and replay prevention: PASS");
	return 0;
}
