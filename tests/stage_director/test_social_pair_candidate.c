#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "sway/stage_director_social_pair_candidate.h"

static struct sd_social_native_member host(void) {
	return (struct sd_social_native_member) {
		.role = SD_SOCIAL_HOST_LAYER,
		.container_ref = 0x110,
		.client_ref = 0x111,
		.root_surface_ref = 0x112,
		.output_ref = 0x301,
		.workspace_ref = 0x302,
		.seat_ref = 0x303,
		.mapped_generation = 101,
		.peer_pid = 2300,
		.peer_start_ticks = 7000,
		.peer_uid = 1000,
		.mapped = true,
		.exact_role = true,
		.floating = false,
	};
}

static struct sd_social_native_member guest(void) {
	return (struct sd_social_native_member) {
		.role = SD_SOCIAL_GUEST_XDG,
		.container_ref = 0x210,
		.client_ref = 0x211,
		.root_surface_ref = 0x212,
		.output_ref = 0x301,
		.workspace_ref = 0x302,
		.seat_ref = 0x303,
		.mapped_generation = 202,
		.peer_pid = 2301,
		.peer_start_ticks = 7010,
		.peer_uid = 1000,
		.mapped = true,
		.exact_role = true,
		.floating = true,
	};
}

static void zero_authority(struct sd_social_candidate_probe p) {
	assert(!p.authenticated_pairing);
	assert(!p.can_move);
	assert(!p.can_resize);
	assert(!p.can_focus);
	assert(!p.can_clip);
	assert(!p.can_render);
	assert(!p.can_input);
	assert(!p.can_popup);
	assert(p.candidate_correlated ==
		(p.state == SD_SOCIAL_PROBE_CORRELATED_CANDIDATE));
}
static void expected(struct sd_social_candidate_probe p,
		enum sd_social_candidate_probe_state status) {
	zero_authority(p);
	assert(p.state == status);
}
static void denied_guest(struct sd_social_pair_candidate *p,
		uint64_t e, struct sd_social_native_member value) {
	assert(!sd_social_pair_candidate_guest(p, e, &value));
	assert(p->state == SD_SOCIAL_CANDIDATE_HOST);
}
static void denied_probe(struct sd_social_pair_candidate *p, uint64_t e,
		struct sd_social_native_member h,
		struct sd_social_native_member g) {
	expected(sd_social_pair_candidate_probe(p, e, &h, &g),
		SD_SOCIAL_PROBE_MISMATCH);
}

static void test_candidate_and_replay(void) {
	struct sd_social_pair_candidate p;
	struct sd_social_native_member h=host(), g=guest(), changed;
	sd_social_pair_candidate_init(&p);
	assert(p.state == SD_SOCIAL_CANDIDATE_EMPTY);
	expected(sd_social_pair_candidate_probe(&p, 1, &h, &g),
		SD_SOCIAL_PROBE_EMPTY);
	assert(!sd_social_pair_candidate_host(NULL,&h));
	assert(!sd_social_pair_candidate_host(&p,NULL));
	changed=h; changed.exact_role=false;
	assert(!sd_social_pair_candidate_host(&p,&changed));
	changed=h; changed.client_ref=0;
	assert(!sd_social_pair_candidate_host(&p,&changed));
	changed=h; changed.mapped=false;
	assert(!sd_social_pair_candidate_host(&p,&changed));
	changed=h; changed.mapped_generation=0;
	assert(!sd_social_pair_candidate_host(&p,&changed));
	const uint64_t e=sd_social_pair_candidate_host(&p,&h);
	assert(e==1 && p.generation_floor==1);
	assert(!sd_social_pair_candidate_host(&p,&h));
	expected(sd_social_pair_candidate_probe(&p,e,&h,&g),
		SD_SOCIAL_PROBE_HOST_ONLY);
	assert(!sd_social_pair_candidate_guest(&p,0,&g));
	assert(!sd_social_pair_candidate_guest(&p,e+1,&g));
	assert(!sd_social_pair_candidate_guest(&p,e,NULL));
	changed=g; changed.role=SD_SOCIAL_HOST_LAYER;
	denied_guest(&p,e,changed);
	changed=g; changed.client_ref=h.client_ref;
	denied_guest(&p,e,changed);
	changed=g; changed.container_ref=h.container_ref;
	denied_guest(&p,e,changed);
	changed=g; changed.root_surface_ref=h.root_surface_ref;
	denied_guest(&p,e,changed);
	changed=g; changed.output_ref=0x777;
	denied_guest(&p,e,changed);
	changed=g; changed.workspace_ref=0x778;
	denied_guest(&p,e,changed);
	changed=g; changed.seat_ref=0x779;
	denied_guest(&p,e,changed);
	changed=g; changed.peer_uid=1001;
	denied_guest(&p,e,changed);
	changed=g; changed.peer_pid=0;
	denied_guest(&p,e,changed);
	changed=g; changed.peer_start_ticks=0;
	denied_guest(&p,e,changed);
	changed=g; changed.floating=false;
	denied_guest(&p,e,changed);
	changed=g; changed.mapped=false;
	denied_guest(&p,e,changed);
	changed=g; changed.exact_role=false;
	denied_guest(&p,e,changed);
	assert(sd_social_pair_candidate_guest(&p,e,&g));
	assert(!sd_social_pair_candidate_guest(&p,e,&g));
	expected(sd_social_pair_candidate_probe(&p,e,&h,&g),
		SD_SOCIAL_PROBE_CORRELATED_CANDIDATE);
	denied_probe(&p,e+1,h,g);
	changed=g; changed.root_surface_ref=0x445;
	denied_probe(&p,e,h,changed);
	changed=g; changed.client_ref=0x446;
	denied_probe(&p,e,h,changed);
	changed=g; changed.container_ref=0x447;
	denied_probe(&p,e,h,changed);
	changed=g; changed.mapped_generation=203;
	denied_probe(&p,e,h,changed);
	changed=g; changed.peer_start_ticks++;
	denied_probe(&p,e,h,changed);
	changed=g; changed.peer_pid++;
	denied_probe(&p,e,h,changed);
	changed=g; changed.mapped=false;
	denied_probe(&p,e,h,changed);
	changed=g; changed.output_ref=0x778;
	denied_probe(&p,e,h,changed);
	changed=h; changed.client_ref=0x991;
	denied_probe(&p,e,changed,g);
	changed=h; changed.mapped_generation++;
	denied_probe(&p,e,changed,g);
	changed=h; changed.workspace_ref=0x664;
	denied_probe(&p,e,changed,g);
	assert(!sd_social_pair_candidate_revoke(
		&p,e+1,SD_SOCIAL_GUEST_UNMAPPED,g.root_surface_ref));
	assert(!sd_social_pair_candidate_revoke(
		&p,e,SD_SOCIAL_GUEST_UNMAPPED,0x882));
	assert(!sd_social_pair_candidate_revoke(
		&p,e,SD_SOCIAL_HOST_UNMAPPED,g.root_surface_ref));
	expected(sd_social_pair_candidate_probe(&p,e,&h,&g),
		SD_SOCIAL_PROBE_CORRELATED_CANDIDATE);
	assert(sd_social_pair_candidate_revoke(
		&p,e,SD_SOCIAL_GUEST_UNMAPPED,g.root_surface_ref));
	assert(p.state==SD_SOCIAL_CANDIDATE_EMPTY && p.generation_floor==e);
	expected(sd_social_pair_candidate_probe(&p,e,&h,&g),SD_SOCIAL_PROBE_EMPTY);

	/* Exact native pointer re-use (ABA) cannot revive the old epoch.
	 * The new mapping generation MUST also change on real remap. */
	h.mapped_generation=102;
	g.mapped_generation=203;
	const uint64_t next=sd_social_pair_candidate_host(&p,&h);
	assert(next==e+1);
	assert(sd_social_pair_candidate_guest(&p,next,&g));
	denied_probe(&p,e,h,g);
	expected(sd_social_pair_candidate_probe(&p,next,&h,&g),
		SD_SOCIAL_PROBE_CORRELATED_CANDIDATE);
	assert(!sd_social_pair_candidate_revoke(
		&p,e,SD_SOCIAL_GUEST_UNMAPPED,g.root_surface_ref));
	assert(p.state==SD_SOCIAL_CANDIDATE_PAIR);
	assert(sd_social_pair_candidate_revoke(
		&p,next,SD_SOCIAL_GUEST_CLIENT_LOST,g.client_ref));
	assert(p.state==SD_SOCIAL_CANDIDATE_EMPTY);
	printf("SD_SOCIAL_CANDIDATE_REPLAY wrong_epoch=DENIED "
		"pointer_reuse=DENIED matching_native=PASS "
		"authority=NONE\n");
}

struct revoke_fixture {
	enum sd_social_candidate_lost event;
	uintptr_t native_ref;
};
static void test_revocations(void) {
	const struct sd_social_native_member h=host(),g=guest();
	const struct revoke_fixture cases[]={
		{SD_SOCIAL_HOST_UNMAPPED,h.root_surface_ref},
		{SD_SOCIAL_GUEST_UNMAPPED,g.root_surface_ref},
		{SD_SOCIAL_HOST_CLIENT_LOST,h.client_ref},
		{SD_SOCIAL_GUEST_CLIENT_LOST,g.client_ref},
		{SD_SOCIAL_OUTPUT_LOST,h.output_ref},
		{SD_SOCIAL_WORKSPACE_LOST,h.workspace_ref},
		{SD_SOCIAL_SEAT_LOST,h.seat_ref},
		{SD_SOCIAL_OPERATOR_CLOSED,h.client_ref},
	};
	for(size_t i=0;i<sizeof cases/sizeof cases[0];i++){
		struct sd_social_pair_candidate p;
		sd_social_pair_candidate_init(&p);
		const uint64_t e=sd_social_pair_candidate_host(&p,&h);
		assert(e && sd_social_pair_candidate_guest(&p,e,&g));
		assert(!sd_social_pair_candidate_revoke(
			&p,e,cases[i].event,0xabcddcba));
		assert(!sd_social_pair_candidate_revoke(
			&p,e+1,cases[i].event,cases[i].native_ref));
		expected(sd_social_pair_candidate_probe(&p,e,&h,&g),
			SD_SOCIAL_PROBE_CORRELATED_CANDIDATE);
		assert(sd_social_pair_candidate_revoke(
			&p,e,cases[i].event,cases[i].native_ref));
		assert(p.state==SD_SOCIAL_CANDIDATE_EMPTY);
		assert(!sd_social_pair_candidate_revoke(
			&p,e,cases[i].event,cases[i].native_ref));
	}
	printf("SD_SOCIAL_CANDIDATE_REVOKE events=8 "
		"stale_event=DENIED wrong_ref=DENIED result=PASS\n");
}

static void test_host_only_and_overflow(void) {
	struct sd_social_pair_candidate p;
	const struct sd_social_native_member h=host(),g=guest();
	sd_social_pair_candidate_init(&p);
	const uint64_t e=sd_social_pair_candidate_host(&p,&h);
	assert(e);
	assert(!sd_social_pair_candidate_revoke(
		&p,e,SD_SOCIAL_GUEST_UNMAPPED,g.root_surface_ref));
	assert(sd_social_pair_candidate_revoke(
		&p,e,SD_SOCIAL_HOST_CLIENT_LOST,h.client_ref));
	assert(p.generation_floor==1);
	/* This is a denial, not an epoch wrap to a reusable credential. */
	p.generation_floor=UINT64_MAX;
	assert(sd_social_pair_candidate_host(&p,&h)==0);
	assert(p.state==SD_SOCIAL_CANDIDATE_EMPTY);
	assert(!sd_social_pair_candidate_guest(&p,1,&g));
	printf("SD_SOCIAL_CANDIDATE_OVERFLOW generation_wrap=DENIED "
		"host_only_revoke=PASS\n");
}

int main(void) {
	test_candidate_and_replay();
	test_revocations();
	test_host_only_and_overflow();
	puts("SD_SOCIAL_CANDIDATE final=PASS "
		"native_observation=CALLER_REQUIRED "
		"private_compositor_admission=NOT_WIRED "
		"production_actuation=OFF");
	return 0;
}
