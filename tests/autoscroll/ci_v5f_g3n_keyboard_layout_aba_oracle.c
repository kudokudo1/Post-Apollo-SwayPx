/*
 * V5F-G3N CI-ONLY state-transition oracle, no compositor installation.
 * All events here are TEST-SIMULATED, not authenticated compositor events.
 * This is a discriminating negative model for keyboard/layout ABA and
 * hotplug revocation, not an issuance scheme for real input authority.
 */
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct g3n_observation {
    uint64_t seat_epoch, keyboard_epoch, layout_epoch, owner_epoch;
    int keyboard_count, unexpected_keyboards, groups;
    int output_x, output_y, output_width, output_height;
    bool poisoned, owner_present;
};
struct g3n_witness {
    uint64_t seat_epoch, keyboard_epoch, layout_epoch, owner_epoch;
    int keyboard_count, output_x, output_y, output_width, output_height;
    bool observed;
};
struct g3n_test_session {
    struct g3n_witness start;
    bool hypothetical_active, revoked;
};
static bool bump(uint64_t *value, bool *poisoned) {
    if (*poisoned || *value == UINT64_MAX) {
        *poisoned = true;
        return false;
    }
    ++*value;
    return true;
}
static struct g3n_observation initial(void) {
    return (struct g3n_observation){
        .seat_epoch=1, .keyboard_epoch=1, .layout_epoch=1,
        .owner_epoch=1, .keyboard_count=4, .unexpected_keyboards=3,
        .groups=1, .output_x=0, .output_y=0,
        .output_width=1920, .output_height=1080,
        .poisoned=false, .owner_present=true,
    };
}
static struct g3n_witness capture(const struct g3n_observation *now) {
    if (!now || now->poisoned || !now->owner_present) {
        return (struct g3n_witness){0};
    }
    return (struct g3n_witness){
        .seat_epoch=now->seat_epoch,
        .keyboard_epoch=now->keyboard_epoch,
        .layout_epoch=now->layout_epoch,
        .owner_epoch=now->owner_epoch,
        .keyboard_count=now->keyboard_count,
        .output_x=now->output_x,
        .output_y=now->output_y,
        .output_width=now->output_width,
        .output_height=now->output_height,
        .observed=true,
    };
}
/* FRESHNESS ONLY: local untrusted epochs are not native protected
 * issuers. Even true here can NEVER authorize an axis or middle swallow. */
static bool same_untrusted_snapshot(const struct g3n_observation *now,
        const struct g3n_witness *old) {
    return now && old && old->observed && !now->poisoned &&
        now->owner_present &&
        old->seat_epoch==now->seat_epoch &&
        old->keyboard_epoch==now->keyboard_epoch &&
        old->layout_epoch==now->layout_epoch &&
        old->owner_epoch==now->owner_epoch &&
        old->keyboard_count==now->keyboard_count &&
        old->output_x==now->output_x &&
        old->output_y==now->output_y &&
        old->output_width==now->output_width &&
        old->output_height==now->output_height;
}
static void keyboard_topology_event(struct g3n_observation *now,
        int count,int unknown,int groups) {
    assert(now);
    if (!bump(&now->keyboard_epoch,&now->poisoned)) return;
    if (count<0 || unknown<0 || unknown>count || groups<0) {
        now->poisoned=true;
        return;
    }
    now->keyboard_count=count;
    now->unexpected_keyboards=unknown;
    now->groups=groups;
}
static void layout_event(struct g3n_observation *now,
        int x,int y,int width,int height) {
    assert(now);
    if (!bump(&now->layout_epoch,&now->poisoned)) return;
    if (width<=0 || height<=0) {now->poisoned=true;return;}
    now->output_x=x;now->output_y=y;
    now->output_width=width;now->output_height=height;
}
static void seat_event(struct g3n_observation *now) {
    assert(now);
    (void)bump(&now->seat_epoch,&now->poisoned);
}
static void owner_unmap(struct g3n_observation *now) {
    assert(now);
    (void)bump(&now->owner_epoch,&now->poisoned);
    now->owner_present=false;
}
/* Intentionally a hypothetical test marker, NOT a real autoscroll state.
 * This has no code path to checked-in cursor, libinput or pointer-axis. */
static struct g3n_test_session hypothetical_test_session(
        const struct g3n_observation *now) {
    return (struct g3n_test_session){
        .start=capture(now),.hypothetical_active=true,.revoked=false,
    };
}
static void test_only_revalidate(struct g3n_test_session *s,
        const struct g3n_observation *now) {
    assert(s);
    if (s->hypothetical_active &&
            !same_untrusted_snapshot(now,&s->start)) {
        s->revoked=true;
        s->hypothetical_active=false;
    }
}
static bool may_consume_middle_or_axis(
        const struct g3n_observation *now,
        const struct g3n_test_session *s) {
    (void)now;(void)s;
    return false; /* HARD OFF. No positive path even with fresh epochs. */
}
static void show(const char *scenario) {
    printf("PA_G3N_UNTRUSTED_EPOCH_REVOKED_PASS scenario=%s "
           "may_consume=0 axis=0 native_issuer=ABSENT\n",scenario);
}
int main(void) {
    struct g3n_observation now=initial();
    struct g3n_test_session s=hypothetical_test_session(&now);
    assert(same_untrusted_snapshot(&now,&s.start));
    assert(!may_consume_middle_or_axis(&now,&s));
    printf("PA_G3N_INITIAL_FRESH_BUT_UNAUTHORIZED_PASS "
           "keyboard_total=%d unknown=%d groups=%d "
           "native_issuer=ABSENT axis=0\n",
           now.keyboard_count,now.unexpected_keyboards,now.groups);

    /* Actual G3M native run observed 4 -> 5 -> 4; this separate test
     * simulates those counts. ABA must not resurrect an earlier witness. */
    keyboard_topology_event(&now,5,3,1);
    test_only_revalidate(&s,&now);
    assert(s.revoked && !s.hypothetical_active);
    assert(!same_untrusted_snapshot(&now,&s.start));
    keyboard_topology_event(&now,4,3,1);
    assert(now.keyboard_count==s.start.keyboard_count);
    assert(!same_untrusted_snapshot(&now,&s.start));
    assert(!may_consume_middle_or_axis(&now,&s));
    show("KEYBOARD_HOTPLUG_ABA_4_5_4");

    now=initial();s=hypothetical_test_session(&now);
    layout_event(&now,137,0,1920,1080);
    test_only_revalidate(&s,&now);
    assert(s.revoked);
    layout_event(&now,0,0,1920,1080);
    assert(now.output_x==s.start.output_x);
    assert(!same_untrusted_snapshot(&now,&s.start));
    assert(!may_consume_middle_or_axis(&now,&s));
    show("OUTPUT_POSITION_ABA_0_137_0");

    now=initial();s=hypothetical_test_session(&now);
    keyboard_topology_event(&now,4,2,1);
    test_only_revalidate(&s,&now);
    assert(s.revoked && !same_untrusted_snapshot(&now,&s.start));
    show("SAME_COUNT_DIFFERENT_KEYBOARD_MEMBERSHIP");

    now=initial();s=hypothetical_test_session(&now);
    keyboard_topology_event(&now,4,3,2);
    test_only_revalidate(&s,&now);
    assert(s.revoked && !same_untrusted_snapshot(&now,&s.start));
    show("SAME_COUNT_GROUP_CHANGED");

    now=initial();s=hypothetical_test_session(&now);
    seat_event(&now);test_only_revalidate(&s,&now);
    assert(s.revoked && !same_untrusted_snapshot(&now,&s.start));
    show("SEAT_CHANGE");

    now=initial();s=hypothetical_test_session(&now);
    owner_unmap(&now);test_only_revalidate(&s,&now);
    assert(s.revoked && !same_untrusted_snapshot(&now,&s.start));
    show("TARGET_UNMAP");

    now=initial();s=hypothetical_test_session(&now);
    now.keyboard_epoch=UINT64_MAX;
    keyboard_topology_event(&now,5,3,1);
    test_only_revalidate(&s,&now);
    assert(now.poisoned && s.revoked && !may_consume_middle_or_axis(&now,&s));
    show("EPOCH_OVERFLOW_FAIL_CLOSED");

    now=initial();s=hypothetical_test_session(&now);
    layout_event(&now,0,0,0,1080);
    test_only_revalidate(&s,&now);
    assert(now.poisoned && s.revoked);
    show("INVALID_LAYOUT_FAIL_CLOSED");

    puts("PA_G3N_ALL_EIGHT_ADVERSARIAL_FRESHNESS_TESTS_PASS "
         "simulation=TEST_ONLY native_authorization=NONE "
         "no_input_consumption=1 no_axis=1 production=HARD_OFF");
    return 0;
}
