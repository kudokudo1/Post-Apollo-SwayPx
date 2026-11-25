#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OS_CONTROLLER_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OS_CONTROLLER_H
#include "sway/input/post_apollo_autoscroll.h"
#include "sway/input/post_apollo_autoscroll_os_controls.h"

/* Private unlinked seat controller. All observations originate at the
 * compositor owner. No native input hooks, physical trust conversion,
 * external IPC or Wayland delivery. A batch is an intent, not a receipt.
 * The native owner must validate and emit the whole batch synchronously,
 * with no intervening callbacks, and cancel on any delivery failure. */
struct pa_os_controller {
    struct pa_os_controls controls;
    struct pa_os_scroll_session session;
    struct pa_autoscroll_state engine;
    uint64_t input_sequence, pending_middle_device;
    bool exhausted;
};
struct pa_os_scroll_batch {
    uint64_t seat_epoch,device_epoch,target_epoch,output_epoch;
    uint64_t layout_epoch,policy_epoch,read_sequence;
    double horizontal_pixels,vertical_pixels;
};
bool pa_os_controller_init(struct pa_os_controller *,uint64_t seat_epoch);
bool pa_os_controller_select(struct pa_os_controller *,enum pa_os_scroll_mode);
void pa_os_controller_cancel(struct pa_os_controller *);
void pa_os_controller_invalidate(struct pa_os_controller *);
bool pa_os_controller_key(struct pa_os_controller *,struct pa_os_keyboard_controls *,
    const struct pa_os_control_event *);
/* True = swallow this middle button ONLY, after original owner identity
 * and sequence are resolved. A consumed press retains its release debt
 * across cancellation/disable. Other buttons are not handled here. */
bool pa_os_controller_middle(struct pa_os_controller *,const struct pa_os_scroll_event *,
    const struct pa_os_scroll_scene *,double x,double y,uint64_t now_ms);
/* Use same native input sequence domain for motion and middle events.
 * Motion is consumed only for the active source and current policy.
 * source_candidate is a fresh OS-source observation, not physical proof. */
bool pa_os_controller_motion(struct pa_os_controller *,uint64_t seat_epoch,
    uint64_t device_epoch,uint64_t sequence,bool source_candidate,double dx,double dy);
/* Timer reads do not increment input_sequence. The caller must re-probe
 * the real original source, obtain a fresh scene and cancel on failure. */
bool pa_os_controller_tick(struct pa_os_controller *,const struct pa_os_scroll_scene *,
    bool source_candidate,uint64_t now_ms,struct pa_os_scroll_batch *);
void pa_os_controller_retire_device(struct pa_os_controller *,uint64_t device_epoch);
#endif
