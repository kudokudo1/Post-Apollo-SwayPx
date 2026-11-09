#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sway/stage_director_company_viewport_state.h"

static const struct stage_director_rect host = {0, 0, 1000, 700};
static const struct stage_director_rect frame = {12, 9, 976, 679};
static const struct stage_director_rect map = {200, 110, 650, 450};

static bool record(struct stage_director_company_viewport_state *s,
        struct stage_director_rect f, struct stage_director_rect m) {
    return stage_director_company_viewport_state_record(
        s, 7, 3, 12, 0x1234, host, f, m);
}

static bool snapshot(const struct stage_director_company_viewport_state *s,
        struct stage_director_company_viewport_snapshot *out) {
    return stage_director_company_viewport_state_snapshot(
        s, 7, 3, 12, 0x1234, host, out);
}

static void empty(const struct stage_director_company_viewport_snapshot *s) {
    assert(!s->candidate_only);
    assert(!s->selection_epoch);
    assert(!s->intent_sequence);
    assert(!s->host_revision);
    assert(!s->native_host_client_ref);
    assert(!s->map_in_host.width);
    assert(!s->map_in_host.height);
}

static void real_report_is_stored_and_can_be_revalidated(void) {
    struct stage_director_company_viewport_state s = {0};
    struct stage_director_company_viewport_snapshot out = {0};
    assert(!snapshot(&s, &out));
    empty(&out);
    assert(record(&s, frame, map));
    assert(s.candidate);
    assert(snapshot(&s, &out));
    assert(out.candidate_only);
    assert(out.selection_epoch == 7 && out.intent_sequence == 3);
    assert(out.host_revision == 12 && out.native_host_client_ref == 0x1234);
    assert(out.map_in_host.x == 212 && out.map_in_host.y == 119);
    assert(out.map_in_host.width == 650 && out.map_in_host.height == 450);
    assert(out.frame_in_host.x == 12 && out.map_in_frame.x == 200);
    stage_director_company_viewport_state_reset(&s);
    assert(!s.candidate);
    assert(!snapshot(&s, &out));
    empty(&out);
}

static void no_implicit_authority_or_fabricated_report(void) {
    struct stage_director_company_viewport_state s = {0};
    struct stage_director_company_viewport_snapshot out = {0};
    assert(!stage_director_company_viewport_state_record(
        NULL, 7, 3, 12, 0x1234, host, frame, map));
    assert(!stage_director_company_viewport_state_record(
        &s, 0, 3, 12, 0x1234, host, frame, map));
    assert(!stage_director_company_viewport_state_record(
        &s, 7, 0, 12, 0x1234, host, frame, map));
    assert(!stage_director_company_viewport_state_record(
        &s, 7, 3, 0, 0x1234, host, frame, map));
    assert(!stage_director_company_viewport_state_record(
        &s, 7, 3, 12, 0, host, frame, map));
    assert(!snapshot(&s, &out));
    empty(&out);
    assert(record(&s, frame, map));
    assert(!stage_director_company_viewport_state_snapshot(
        &s, 7, 3, 12, 0x1234, host, NULL));
    assert(snapshot(&s, &out));
    /* Candidate snapshots have no can_render/can_focus/can_accept_input
     * fields. There is intentionally no scene/seat API in this component. */
    assert(out.candidate_only);
}

static void stale_client_launch_epoch_revision_and_host_are_refused(void) {
    struct stage_director_company_viewport_state s = {0};
    struct stage_director_company_viewport_snapshot out = {0};
    assert(record(&s, frame, map));
    assert(!stage_director_company_viewport_state_snapshot(
        &s, 8, 3, 12, 0x1234, host, &out));
    empty(&out);
    assert(!stage_director_company_viewport_state_snapshot(
        &s, 7, 4, 12, 0x1234, host, &out));
    empty(&out);
    assert(!stage_director_company_viewport_state_snapshot(
        &s, 7, 3, 13, 0x1234, host, &out));
    empty(&out);
    assert(!stage_director_company_viewport_state_snapshot(
        &s, 7, 3, 12, 0x9999, host, &out));
    empty(&out);
    assert(!stage_director_company_viewport_state_snapshot(
        &s, 7, 3, 12, 0x1234,
        (struct stage_director_rect){0, 0, 500, 700}, &out));
    empty(&out);
    assert(!stage_director_company_viewport_state_snapshot(
        &s, 7, 3, 12, 0x1234,
        (struct stage_director_rect){2, 0, 1000, 700}, &out));
    empty(&out);
    assert(snapshot(&s, &out));
}

static void malformed_or_hidden_updates_revoke_the_previous_rectangle(void) {
    struct stage_director_company_viewport_state s = {0};
    struct stage_director_company_viewport_snapshot out = {0};
    assert(record(&s, frame, map));
    assert(!record(&s, (struct stage_director_rect){0},
                   (struct stage_director_rect){0}));
    assert(!snapshot(&s, &out)); empty(&out);
    assert(record(&s, frame, map));
    assert(!record(&s, frame,
        (struct stage_director_rect){650, 110, 650, 450}));
    assert(!snapshot(&s, &out)); empty(&out);
    assert(record(&s, frame, map));
    assert(!record(&s, (struct stage_director_rect){INT_MAX, 0, 976, 679},
                  map));
    assert(!snapshot(&s, &out)); empty(&out);
    assert(record(&s, frame, map));
    assert(!record(&s, frame,
                  (struct stage_director_rect){0, 0, 0, 0}));
    assert(!snapshot(&s, &out)); empty(&out);
    assert(record(&s, frame, map));
    assert(!stage_director_company_viewport_state_record(
        &s, 7, 3, 12, 0x1234,
        (struct stage_director_rect){0, 0, 400, 200}, frame, map));
    assert(!snapshot(&s, &out)); empty(&out);
}

static void resized_canvas_requires_fresh_valid_report(void) {
    struct stage_director_company_viewport_state s = {0};
    struct stage_director_company_viewport_snapshot out = {0};
    assert(record(&s, frame, map));
    assert(!stage_director_company_viewport_state_snapshot(
        &s, 7, 3, 13, 0x1234,
        (struct stage_director_rect){0, 0, 640, 490}, &out));
    empty(&out);
    assert(stage_director_company_viewport_state_record(
        &s, 7, 3, 13, 0x1234,
        (struct stage_director_rect){0, 0, 640, 490},
        (struct stage_director_rect){12, 9, 616, 469},
        (struct stage_director_rect){80, 65, 380, 250}));
    assert(stage_director_company_viewport_state_snapshot(
        &s, 7, 3, 13, 0x1234,
        (struct stage_director_rect){0, 0, 640, 490}, &out));
    assert(out.map_in_host.x == 92 && out.map_in_host.y == 74);
    assert(out.map_in_host.width == 380 && out.map_in_host.height == 250);
    assert(!snapshot(&s, &out));
    empty(&out);
}

int main(void) {
    real_report_is_stored_and_can_be_revalidated();
    no_implicit_authority_or_fabricated_report();
    stale_client_launch_epoch_revision_and_host_are_refused();
    malformed_or_hidden_updates_revoke_the_previous_rectangle();
    resized_canvas_requires_fresh_valid_report();
    puts("SD_COMPANY_M32A_VIEWPORT_STATE PASS "
         "production_path=COMPOSITOR_PRIVATE "
         "stale_epoch_intent_revision_client=REFUSED "
         "hidden_malformed_revoke=PASS "
         "scene=OFF input=OFF");
    return 0;
}
