#ifndef SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OS_SESSION_H
#define SWAY_INPUT_POST_APOLLO_AUTOSCROLL_OS_SESSION_H
#include <stdbool.h>
#include <stdint.h>

/* Detached G4C1 admission/continuity contract. NOT linked by Meson.
 * Only a future private compositor event owner may populate these
 * records. They are not credentials, IPC input or physical attestation.
 * Positive result means OS-policy session eligibility, not axis delivery.
 * Allocate one zero-initialized slot per live seat; never reuse across
 * seat lifetimes. The caller must resolve owners before reading them. */
enum pa_os_scroll_mode {
    PA_OS_SCROLL_OFF=0, PA_OS_SCROLL_MANAGED=1, PA_OS_SCROLL_ENROLLED=2
};
enum pa_os_scroll_lock {
    PA_OS_SCROLL_LOCK_UNKNOWN=0, PA_OS_SCROLL_LOCK_DISARMED=1,
    PA_OS_SCROLL_LOCK_ARMED=2
};
struct pa_os_scroll_scene {
    uint64_t device_epoch, seat_epoch, target_epoch, output_epoch;
    uint64_t layout_epoch, policy_epoch, read_sequence, read_ms;
    enum pa_os_scroll_mode mode;
    enum pa_os_scroll_lock lock;
    bool selected, owners_live, device_attached, direct_mapped_hit;
    bool focused_root_matches, layout_verified, no_pending_transaction;
    bool no_constraint, session_unlocked, no_conflicting_grab;
    bool company_weather_clear, no_emulation;
};
struct pa_os_scroll_event {
    uint64_t sequence, observed_ms;
    bool native_callback, middle_press, libinput_pointer, live_handle;
    bool coherent_kernel_tree, known_synthetic, protocol_virtual;
    bool scroll_method_known, native_button_scroll;
};
struct pa_os_scroll_session {
    uint64_t seat_epoch, last_event_sequence, last_read_sequence;
    uint64_t begin_ms, last_tick_ms;
    struct pa_os_scroll_scene bound;
    bool active;
};
/* Seat epoch never changes on a live slot. No global state or positive
 * enrolled mode. Cancel preserves anti-replay high-water marks. */
bool pa_os_scroll_init(struct pa_os_scroll_session *,uint64_t seat_epoch);
void pa_os_scroll_cancel(struct pa_os_scroll_session *);
bool pa_os_scroll_begin(struct pa_os_scroll_session *,
    const struct pa_os_scroll_event *,const struct pa_os_scroll_scene *,
    uint64_t now_ms);
bool pa_os_scroll_tick(struct pa_os_scroll_session *,
    const struct pa_os_scroll_scene *,uint64_t now_ms);
#endif
