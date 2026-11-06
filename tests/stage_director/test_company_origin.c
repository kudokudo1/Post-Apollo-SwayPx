#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sway/stage_director_company_origin.h"

#define HZ 100ULL

static struct stage_director_company_intent_snapshot intent(void) {
	return (struct stage_director_company_intent_snapshot) {
		.present = true,
		.selection_epoch = 11,
		.sequence = 41,
		.observed_boot_ticks = 100000,
		.host_revision = 7,
		.native_host_client_ref = 0x103,
		.mode = 0,
	};
}
static struct stage_director_company_native_host host(void) {
	return (struct stage_director_company_native_host) {
		.candidate = true, .state = SD_COMPANY_NATIVE_CANDIDATE_ONLY,
		.native_layer_ref = 0x101, .native_layer_role_ref = 0x102,
		.native_surface_ref = 0x104, .native_client_ref = 0x103,
		.native_output_ref = 0x105, .pid = 3000,
		.uid = 1000, .start_ticks = 90000,
	};
}
static struct stage_director_company_native_actor actor(void) {
	return (struct stage_director_company_native_actor) {
		.candidate = true, .state = SD_COMPANY_NATIVE_CANDIDATE_ONLY,
		.native_view_ref = 0x201, .native_root_surface_ref = 0x202,
		.native_client_ref = 0x203, .pid = 3001,
		.uid = 1000, .start_ticks = 100001,
	};
}
static struct stage_director_company_lineage_result lineage(void) {
	return (struct stage_director_company_lineage_result) {
		.status = SD_COMPANY_LINEAGE_CANDIDATE_ONLY,
		.direct_child_candidate = true,
	};
}
static void check(
		const struct stage_director_company_intent_snapshot *i,
		const struct stage_director_company_native_host *h,
		const struct stage_director_company_native_actor *a,
		const struct stage_director_company_lineage_result *l,
		uint64_t hz, enum stage_director_company_origin_status wanted) {
	const struct stage_director_company_origin_result result =
		stage_director_company_origin_check(i, h, a, l, hz);
	if (result.status != wanted) {
		fprintf(stderr, "M27 expected=%s actual=%s\n",
			stage_director_company_origin_status_name(wanted),
			stage_director_company_origin_status_name(result.status));
	}
	assert(result.status == wanted);
	assert(result.intent_correlated_candidate ==
		(wanted == SD_COMPANY_ORIGIN_CANDIDATE_ONLY));
	assert(!result.launch_authenticated && !result.owner_verified &&
		!result.can_pair_actor && !result.can_render && !result.can_clip &&
		!result.can_focus && !result.can_accept_input &&
		!result.can_show_popup);
}

static void legitimate_near_time_child_is_candidate_only(void) {
	struct stage_director_company_intent_snapshot i = intent();
	struct stage_director_company_native_host h = host();
	struct stage_director_company_native_actor a = actor();
	struct stage_director_company_lineage_result l = lineage();
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_CANDIDATE_ONLY);
	i.mode = 1; /* fixed online mode remains a candidate, not consent */
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_CANDIDATE_ONLY);
	a.start_ticks = i.observed_boot_ticks - 2 * HZ;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_CANDIDATE_ONLY);
	a.start_ticks = i.observed_boot_ticks + 5 * HZ;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_CANDIDATE_ONLY);
	assert(strcmp(stage_director_company_origin_status_name(
		SD_COMPANY_ORIGIN_CANDIDATE_ONLY), "CANDIDATE_ONLY") == 0);
}

static void old_manual_and_late_qt_windows_are_not_matched(void) {
	struct stage_director_company_intent_snapshot i = intent();
	struct stage_director_company_native_host h = host();
	struct stage_director_company_native_actor a = actor();
	struct stage_director_company_lineage_result l = lineage();
	a.start_ticks = i.observed_boot_ticks - 2 * HZ - 1;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_BIRTH_OUTSIDE_WINDOW);
	a.start_ticks = i.observed_boot_ticks + 5 * HZ + 1;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_BIRTH_OUTSIDE_WINDOW);
	a.start_ticks = h.start_ticks - 1;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_STALE_PROCESS);
}

static void missing_or_replayed_private_intent_fails(void) {
	struct stage_director_company_intent_snapshot i = intent();
	struct stage_director_company_native_host h = host();
	struct stage_director_company_native_actor a = actor();
	struct stage_director_company_lineage_result l = lineage();
	check(NULL, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_NO_INTENT);
	i.present = false;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_NO_INTENT);
	i = intent(); i.selection_epoch = 0;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_NO_INTENT);
	i = intent(); i.sequence = 0;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_NO_INTENT);
	i = intent(); i.host_revision = 0;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_NO_INTENT);
	i = intent(); i.observed_boot_ticks = 0;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_NO_INTENT);
	i = intent(); i.mode = 2; /* never arbitrary URL or mode */
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_NO_INTENT);
	i = intent();
	check(&i, &h, &a, &l, 0, SD_COMPANY_ORIGIN_NO_INTENT);
	check(&i, &h, &a, &l, UINT64_MAX, SD_COMPANY_ORIGIN_NO_INTENT);
}

static void foreign_host_and_actor_are_never_promoted(void) {
	struct stage_director_company_intent_snapshot i = intent();
	struct stage_director_company_native_host h = host();
	struct stage_director_company_native_actor a = actor();
	struct stage_director_company_lineage_result l = lineage();
	i.native_host_client_ref++;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_WRONG_HOST);
	i = intent(); h.native_output_ref = 0;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_WRONG_HOST);
	h = host(); h.candidate = false;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_WRONG_HOST);
	h = host(); a.native_client_ref = h.native_client_ref;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_WRONG_ACTOR);
	a = actor(); a.uid++;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_WRONG_ACTOR);
	a = actor(); a.state = SD_COMPANY_NATIVE_UNMAPPED;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_WRONG_ACTOR);
	a = actor(); a.candidate = false;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_WRONG_ACTOR);
	a = actor();
	check(&i, &h, NULL, &l, HZ, SD_COMPANY_ORIGIN_WRONG_ACTOR);
}

static void unverified_lineage_denies_candidate(void) {
	struct stage_director_company_intent_snapshot i = intent();
	struct stage_director_company_native_host h = host();
	struct stage_director_company_native_actor a = actor();
	struct stage_director_company_lineage_result l = lineage();
	check(&i, &h, &a, NULL, HZ, SD_COMPANY_ORIGIN_NO_LINEAGE);
	l.status = SD_COMPANY_LINEAGE_NOT_DIRECT_CHILD;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_NO_LINEAGE);
	l = lineage(); l.direct_child_candidate = false;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_NO_LINEAGE);
	l = lineage(); l.can_clip = true;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_NO_LINEAGE);
	l = lineage(); l.owner_verified = true;
	check(&i, &h, &a, &l, HZ, SD_COMPANY_ORIGIN_NO_LINEAGE);
}

int main(void) {
	legitimate_near_time_child_is_candidate_only();
	old_manual_and_late_qt_windows_are_not_matched();
	missing_or_replayed_private_intent_fails();
	foreign_host_and_actor_are_never_promoted();
	unverified_lineage_denies_candidate();
	puts("STAGE_DIRECTOR_COMPANY_M27: PASS fresh private intent/Qt process birth candidate, authority=NONE");
	return 0;
}
