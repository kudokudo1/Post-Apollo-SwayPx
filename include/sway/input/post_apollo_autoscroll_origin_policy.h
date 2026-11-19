#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_ORIGIN_POLICY_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_ORIGIN_POLICY_H

#include <stdbool.h>

/*
 * Read-only policy for identifying a libinput-backed POINTER CANDIDATE.
 *
 * Critical: a libinput backend may also receive synthetic kernel uinput
 * devices. These facts do NOT establish genuine hardware provenance and
 * MUST NEVER be passed as "trusted_physical_pointer" to the adapter.
 *
 * Stage Director pairing has first refusal in cursor.c. The actual
 * compositor is hard-OFF until a separate owner-minted, nonreplayable
 * physical + lifecycle witness is implemented and accepted.
 */
struct pa_autoscroll_origin_facts {
    bool pointer_device;
    bool libinput_backend;
    bool libinput_handle_present;
    bool native_scroll_method_known;
    bool native_scroll_on_button_down;
};

enum pa_autoscroll_origin_result {
    PA_AUTOSCROLL_ORIGIN_DENIED = 0,
    PA_AUTOSCROLL_ORIGIN_LIBINPUT_CANDIDATE = 1,
};

enum pa_autoscroll_origin_result pa_autoscroll_origin_classify(
    const struct pa_autoscroll_origin_facts *facts);

#endif
