#include <assert.h>
#include <stdio.h>
#include "sway/input/post_apollo_autoscroll_origin_policy.h"

static void test_candidate_is_never_physical_authority(void) {
    const struct pa_autoscroll_origin_facts good = {
        .pointer_device = true,
        .libinput_backend = true,
        .libinput_handle_present = true,
        .native_scroll_method_known = true,
        .native_scroll_on_button_down = false,
    };
    assert(pa_autoscroll_origin_classify(NULL) ==
        PA_AUTOSCROLL_ORIGIN_DENIED);
    assert(pa_autoscroll_origin_classify(&good) ==
        PA_AUTOSCROLL_ORIGIN_LIBINPUT_CANDIDATE);

#define DENY(field, value) do { \
    struct pa_autoscroll_origin_facts candidate = good; \
    candidate.field = value; \
    assert(pa_autoscroll_origin_classify(&candidate) == \
        PA_AUTOSCROLL_ORIGIN_DENIED); \
} while (0)

    DENY(pointer_device, false);
    DENY(libinput_backend, false);
    DENY(libinput_handle_present, false);
    DENY(native_scroll_method_known, false);
    DENY(native_scroll_on_button_down, true);
#undef DENY
}

static void test_empty_is_denied(void) {
    struct pa_autoscroll_origin_facts empty = {0};
    assert(pa_autoscroll_origin_classify(&empty) ==
        PA_AUTOSCROLL_ORIGIN_DENIED);
}

int main(void) {
    test_empty_is_denied();
    test_candidate_is_never_physical_authority();
    puts("PA_AUTOSCROLL_V5E_A_CANDIDATE_ONLY_PASS physical_authority=NONE");
    return 0;
}
