#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OS_INGRESS_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OS_INGRESS_H
#include "sway/input/post_apollo_autoscroll_os_native.h"
#include "sway/input/post_apollo_autoscroll_os_controller.h"

/* Private synchronous boundary; not linked in production. All arguments
 * are compositor-owned observations, not user/IPC-provided credentials.
 * Pairing debt is resolved before authorization checks. Clearance must
 * originate from complete scene and shared-owner policy, not from mere
 * presence of a layout revision. No test grant flag exists here. */
struct pa_os_ingress_clearance {
    uint64_t layout_epoch;
    bool layout_verified,no_conflicting_grab,company_weather_clear;
};
bool pa_os_ingress_middle(struct pa_os_controller *,
    const struct pa_os_native_observation *,const struct pa_os_ingress_clearance *,
    double x,double y);
#endif
