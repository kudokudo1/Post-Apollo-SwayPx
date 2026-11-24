#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "sway/input/post_apollo_autoscroll_lifetime.h"

static void test_unique_domains_and_address_reuse(void) {
    struct pa_autoscroll_lifetime_issuer issuer={0};
    struct pa_autoscroll_lifetime_slot device={0}, seat={0};
    struct pa_autoscroll_lifetime_slot target={0}, output={0};
    assert(pa_autoscroll_lifetime_init(&issuer));
    assert(!pa_autoscroll_lifetime_init(NULL));
    struct pa_autoscroll_lifetime_issuer uninitialized={0};
    assert(!pa_autoscroll_lifetime_bind(
        &uninitialized,&device,PA_LIFETIME_DEVICE));
    assert(!pa_autoscroll_lifetime_bind(NULL,&device,PA_LIFETIME_DEVICE));
    assert(!pa_autoscroll_lifetime_bind(&issuer,NULL,PA_LIFETIME_DEVICE));
    assert(!pa_autoscroll_lifetime_bind(&issuer,&device,PA_LIFETIME_NONE));
    assert(pa_autoscroll_lifetime_bind(&issuer,&device,PA_LIFETIME_DEVICE));
    /* A mistaken repeated initialization must not reset the sequence. */
    assert(!pa_autoscroll_lifetime_init(&issuer));
    assert(issuer.latest_epoch == 1);
    assert(pa_autoscroll_lifetime_bind(&issuer,&seat,PA_LIFETIME_SEAT));
    assert(pa_autoscroll_lifetime_bind(&issuer,&target,PA_LIFETIME_TARGET));
    assert(pa_autoscroll_lifetime_bind(&issuer,&output,PA_LIFETIME_OUTPUT));
    assert(pa_autoscroll_lifetime_epoch(&device,PA_LIFETIME_DEVICE)==1);
    assert(pa_autoscroll_lifetime_epoch(&seat,PA_LIFETIME_SEAT)==2);
    assert(pa_autoscroll_lifetime_epoch(&target,PA_LIFETIME_TARGET)==3);
    assert(pa_autoscroll_lifetime_epoch(&output,PA_LIFETIME_OUTPUT)==4);
    assert(pa_autoscroll_lifetime_epoch(&device,PA_LIFETIME_SEAT)==0);
    assert(pa_autoscroll_lifetime_epoch(NULL,PA_LIFETIME_DEVICE)==0);
    assert(!pa_autoscroll_lifetime_bind(&issuer,&device,PA_LIFETIME_DEVICE));
    pa_autoscroll_lifetime_revoke(&device);
    assert(pa_autoscroll_lifetime_epoch(&device,PA_LIFETIME_DEVICE)==0);
    /* Reuse the exact same slot address, but never the old epoch. */
    assert(pa_autoscroll_lifetime_bind(&issuer,&device,PA_LIFETIME_DEVICE));
    assert(pa_autoscroll_lifetime_epoch(&device,PA_LIFETIME_DEVICE)==5);
    pa_autoscroll_lifetime_revoke(&seat);
    assert(pa_autoscroll_lifetime_epoch(&seat,PA_LIFETIME_SEAT)==0);
    assert(pa_autoscroll_lifetime_bind(&issuer,&seat,PA_LIFETIME_SEAT));
    assert(pa_autoscroll_lifetime_epoch(&seat,PA_LIFETIME_SEAT)==6);
}

static void test_no_wraparound_and_fail_closed(void) {
    struct pa_autoscroll_lifetime_issuer issuer={0};
    struct pa_autoscroll_lifetime_slot a={0},b={0};
    assert(pa_autoscroll_lifetime_init(&issuer));
    issuer.latest_epoch=UINT64_MAX-1u;
    assert(pa_autoscroll_lifetime_bind(&issuer,&a,PA_LIFETIME_OUTPUT));
    assert(pa_autoscroll_lifetime_epoch(&a,PA_LIFETIME_OUTPUT)==UINT64_MAX);
    assert(!pa_autoscroll_lifetime_bind(&issuer,&b,PA_LIFETIME_DEVICE));
    assert(issuer.exhausted);
    assert(pa_autoscroll_lifetime_epoch(&b,PA_LIFETIME_DEVICE)==0);
    pa_autoscroll_lifetime_revoke(&a);
    assert(!pa_autoscroll_lifetime_bind(&issuer,&a,PA_LIFETIME_DEVICE));
}
int main(void) {
    test_unique_domains_and_address_reuse();
    test_no_wraparound_and_fail_closed();
    puts("PA_AUTOSCROLL_LIFETIME_EPOCH_ISSUER_PASS "
         "compositor_lifecycle_hooks=NOT_YET_CONNECTED");
    return 0;
}
