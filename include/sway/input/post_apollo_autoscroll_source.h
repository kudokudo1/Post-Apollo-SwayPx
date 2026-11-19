#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_SOURCE_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_SOURCE_H

#include "sway/input/post_apollo_autoscroll_origin_policy.h"

/*
 * Called exclusively from the compositor-owned cursor callback, after
 * Weather and Company Stage Director pairing have first refusal.
 *
 * Inspect the current wlroots pointer BACKEND and read-only libinput scroll
 * configuration. Return policy facts only, never "trusted physical" status.
 * A libinput-backed kernel uinput device may still be synthetic.
 */
struct wlr_input_device;

struct pa_autoscroll_origin_facts pa_autoscroll_source_probe(
    struct wlr_input_device *device);

#endif
