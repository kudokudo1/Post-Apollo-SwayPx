#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "sway/stage_director_news_containment.h"

#define HOST ((uintptr_t)0x1234)
#define ACTOR ((uintptr_t)0x5678)

static struct stage_director_news_containment_snapshot baseline(void) {
	return (struct stage_director_news_containment_snapshot) {
		.request = {
			.host = {100, 40, 1000, 700},
			.output = {0, 0, 2560, 1440},
			.actor = {180, 105, 950, 630},
			.viewport = {205, 120, 824, 494},
			.host_mapped = true,
			.actor_mapped = true,
			.output_active = true,
			.actor_identity_verified = true,
		},
		.epoch = 101, .native_host_ref = HOST, .native_actor_ref = ACTOR,
		.session_owner_verified = true, .actor_process_verified = true,
	};
}

static void check(struct stage_director_news_containment_result r,
		enum stage_director_news_containment_status expected,
		bool pointer, bool popup) {
	assert(r.status == expected);
	assert(r.pointer_candidate == pointer);
	assert(r.popup_candidate == popup);
	assert(!r.can_dispatch_input && !r.can_show_popup && !r.can_enable_clip);
}

static void pointer_tests(void) {
	struct stage_director_news_containment_snapshot s = baseline();
	check(stage_director_news_pointer_probe(&s,101,HOST,ACTOR,205.0,120.0),
		SD_NEWS_CONTAINMENT_CANDIDATE_ONLY,true,false);
	check(stage_director_news_pointer_probe(&s,101,HOST,ACTOR,1028.999,613.999),
		SD_NEWS_CONTAINMENT_CANDIDATE_ONLY,true,false);
	check(stage_director_news_pointer_probe(&s,101,HOST,ACTOR,1029.0,613.99),
		SD_NEWS_CONTAINMENT_OUTSIDE_VIEWPORT,false,false);
	check(stage_director_news_pointer_probe(&s,101,HOST,ACTOR,1028.9,614.0),
		SD_NEWS_CONTAINMENT_OUTSIDE_VIEWPORT,false,false);
	check(stage_director_news_pointer_probe(&s,101,HOST,ACTOR,204.999,120.0),
		SD_NEWS_CONTAINMENT_OUTSIDE_VIEWPORT,false,false);
	check(stage_director_news_pointer_probe(&s,101,HOST,ACTOR,NAN,200),
		SD_NEWS_CONTAINMENT_INVALID_COORDS,false,false);
	check(stage_director_news_pointer_probe(&s,101,HOST,ACTOR,INFINITY,200),
		SD_NEWS_CONTAINMENT_INVALID_COORDS,false,false);
	check(stage_director_news_pointer_probe(&s,101,HOST,ACTOR,205,-INFINITY),
		SD_NEWS_CONTAINMENT_INVALID_COORDS,false,false);
	check(stage_director_news_pointer_probe(&s,101,HOST,ACTOR,-1e300,120),
		SD_NEWS_CONTAINMENT_OUTSIDE_VIEWPORT,false,false);
	check(stage_director_news_pointer_probe(&s,101,HOST,0,205,120),
		SD_NEWS_CONTAINMENT_NO_SESSION,false,false);
	check(stage_director_news_pointer_probe(&s,101,HOST,0x9999,205,120),
		SD_NEWS_CONTAINMENT_NO_SESSION,false,false);
	check(stage_director_news_pointer_probe(&s,100,HOST,ACTOR,205,120),
		SD_NEWS_CONTAINMENT_NO_SESSION,false,false);
	check(stage_director_news_pointer_probe(&s,101,0x9999,ACTOR,205,120),
		SD_NEWS_CONTAINMENT_NO_SESSION,false,false);
	s.session_owner_verified = false;
	check(stage_director_news_pointer_probe(&s,101,HOST,ACTOR,205,120),
		SD_NEWS_CONTAINMENT_NO_SESSION,false,false);
	s = baseline(); s.request.host_mapped = false;
	check(stage_director_news_pointer_probe(&s,101,HOST,ACTOR,205,120),
		SD_NEWS_CONTAINMENT_STALE_GEOMETRY,false,false);
	s = baseline(); s.request.output_active = false;
	check(stage_director_news_pointer_probe(&s,101,HOST,ACTOR,205,120),
		SD_NEWS_CONTAINMENT_STALE_GEOMETRY,false,false);
	s = baseline(); s.request.actor.width = 10;
	check(stage_director_news_pointer_probe(&s,101,HOST,ACTOR,205,120),
		SD_NEWS_CONTAINMENT_STALE_GEOMETRY,false,false);
}

static void popup_tests(void) {
	struct stage_director_news_containment_snapshot s = baseline();
	check(stage_director_news_popup_probe(&s,101,HOST,ACTOR,
		(struct stage_director_rect){205,120,824,494}),
		SD_NEWS_CONTAINMENT_CANDIDATE_ONLY,false,true);
	check(stage_director_news_popup_probe(&s,101,HOST,ACTOR,
		(struct stage_director_rect){1000,580,29,34}),
		SD_NEWS_CONTAINMENT_CANDIDATE_ONLY,false,true);
	check(stage_director_news_popup_probe(&s,101,HOST,ACTOR,
		(struct stage_director_rect){1028,580,30,34}),
		SD_NEWS_CONTAINMENT_POPUP_OVERFLOW,false,false);
	check(stage_director_news_popup_probe(&s,101,HOST,ACTOR,
		(struct stage_director_rect){204,121,1,12}),
		SD_NEWS_CONTAINMENT_POPUP_OVERFLOW,false,false);
	check(stage_director_news_popup_probe(&s,101,HOST,ACTOR,
		(struct stage_director_rect){205,120,0,1}),
		SD_NEWS_CONTAINMENT_INVALID_COORDS,false,false);
	check(stage_director_news_popup_probe(&s,101,HOST,ACTOR,
		(struct stage_director_rect){INT32_MAX,120,20,12}),
		SD_NEWS_CONTAINMENT_INVALID_COORDS,false,false);
	check(stage_director_news_popup_probe(&s,101,HOST,0x22,
		(struct stage_director_rect){210,130,30,30}),
		SD_NEWS_CONTAINMENT_NO_SESSION,false,false);
}

static void second_output_tests(void) {
	struct stage_director_news_containment_snapshot s = baseline();
	s.request.host = (struct stage_director_rect){-1200,10,1000,700};
	s.request.output = (struct stage_director_rect){-1360,0,1360,768};
	s.request.actor = (struct stage_director_rect){-1190,60,990,650};
	s.request.viewport = (struct stage_director_rect){-1110,80,824,494};
	check(stage_director_news_pointer_probe(&s,101,HOST,ACTOR,-1110,80),
		SD_NEWS_CONTAINMENT_CANDIDATE_ONLY,true,false);
	check(stage_director_news_pointer_probe(&s,101,HOST,ACTOR,-286,574),
		SD_NEWS_CONTAINMENT_OUTSIDE_VIEWPORT,false,false);
	check(stage_director_news_popup_probe(&s,101,HOST,ACTOR,
		(struct stage_director_rect){-1110,80,824,494}),
		SD_NEWS_CONTAINMENT_CANDIDATE_ONLY,false,true);
	s.request.output.x = 0;
	check(stage_director_news_popup_probe(&s,101,HOST,ACTOR,
		(struct stage_director_rect){-1110,80,200,100}),
		SD_NEWS_CONTAINMENT_STALE_GEOMETRY,false,false);
}

int main(void) {
	pointer_tests();
	popup_tests();
	second_output_tests();
	puts("Stage Director NEWS fractional pointer and strict popup boundary: PASS");
	return 0;
}
