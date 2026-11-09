#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_SCENE_LEASE_CANDIDATE_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_SCENE_LEASE_CANDIDATE_H

#include <stdbool.h>
#include <stdint.h>

/* M32o // Company native scene lease CANDIDATE, NOT a scene lease.
 *
 * This is the bounded, compositor-local lifecycle needed before an actual
 * operator-authenticated issuer can exist. No caller of this module may
 * grant rendering, native pointer events, popup, keyboard or positioning.
 *
 * No public Wayland protocol, QML, IPC or application-owned token may
 * reserve a candidate. Future producers MUST obtain all observations from
 * native compositor objects and the M32m preflight (which currently ends
 * at OPERATOR_LEASE_ABSENT), and MUST re-observe kernel pidfd/executable
 * evidence for every scene operation.
 *
 * A fixed maximum of 5 monotonic seconds is a cap, not permission. Time
 * values are from the future compositor-owned CLOCK_BOOTTIME clock; tests
 * may supply synthetic monotonic ticks. This unit never reads wall clock.
 */
#define SD_COMPANY_LEASE_CANDIDATE_MAX_NS UINT64_C(5000000000)

enum stage_director_company_scene_lease_candidate_state {
    SD_COMPANY_LEASE_CANDIDATE_EMPTY,
    SD_COMPANY_LEASE_CANDIDATE_RESERVED,
};

/* No app string, title or client-supplied identifier is accepted. Values
 * must be resolved from exact native Wayland objects in the compositor. */
struct stage_director_company_scene_lease_identity {
    uint64_t selection_epoch;
    uint64_t intent_sequence;
    uint64_t host_revision;
    uintptr_t native_host_layer_ref;
    uintptr_t native_host_surface_ref;
    uintptr_t native_host_client_ref;
    uintptr_t native_output_ref;
    uintptr_t native_actor_view_ref;
    uintptr_t native_actor_root_surface_ref;
    uintptr_t native_actor_client_ref;
    int64_t actor_pid;
    uint64_t actor_start_ticks;
};

/* Independent prerequisites, all initially unavailable in production:
 * in particular exact_digest_candidate is false until a reviewed manifest
 * is pinned. Even all-true conditions only record a CANDIDATE. */
struct stage_director_company_scene_lease_conditions {
    bool selected_native_host;
    bool live_intent_and_viewport;
    bool current_kernel_pidfd_actor;
    bool executable_inode_candidate;
    bool exact_digest_candidate;
    bool host_mapped;
    bool actor_mapped;
    bool output_active;
};

struct stage_director_company_scene_lease_candidate {
    enum stage_director_company_scene_lease_candidate_state state;
    uint64_t last_epoch; /* Persistent high-water mark across revocations. */
    uint64_t reserved_at_ns;
    uint64_t expires_at_ns;
    struct stage_director_company_scene_lease_identity identity;
};

enum stage_director_company_scene_lease_probe_state {
    SD_COMPANY_LEASE_NO_CANDIDATE,
    SD_COMPANY_LEASE_EXPIRED,
    SD_COMPANY_LEASE_NATIVE_CHANGED,
    SD_COMPANY_LEASE_EVIDENCE_LOST,
    SD_COMPANY_LEASE_MATCHED_CANDIDATE_ONLY,
};

struct stage_director_company_scene_lease_receipt {
    enum stage_director_company_scene_lease_probe_state state;
    bool native_candidate_current;
    /* All privileges are unconditionally false, including exact match. */
    bool operator_authenticated;
    bool launch_authenticated;
    bool scene_lease_issued;
    bool can_apply_scene;
    bool can_accept_input;
    bool can_focus_keyboard;
    bool can_show_popup;
};

void stage_director_company_scene_lease_candidate_init(
    struct stage_director_company_scene_lease_candidate *candidate);

/* Single reservation per fresh physical selection epoch. No overlapping
 * candidates, no epoch reuse after revoke, no unchecked TTL overflow. */
bool stage_director_company_scene_lease_candidate_reserve(
    struct stage_director_company_scene_lease_candidate *candidate,
    const struct stage_director_company_scene_lease_identity *identity,
    const struct stage_director_company_scene_lease_conditions *conditions,
    uint64_t monotonic_now_ns, uint64_t ttl_ns);

/* Fail-closed on changed native object, stale preflight, backward time or
 * expiry. Any failed check irreversibly revokes the current reservation. */
struct stage_director_company_scene_lease_receipt
stage_director_company_scene_lease_candidate_probe(
    struct stage_director_company_scene_lease_candidate *candidate,
    const struct stage_director_company_scene_lease_identity *observed,
    const struct stage_director_company_scene_lease_conditions *conditions,
    uint64_t monotonic_now_ns);

/* Explicit close or loss of the exact compositor-observed resource. A
 * nonmatching stale event cannot revoke another active candidate. */
bool stage_director_company_scene_lease_candidate_revoke(
    struct stage_director_company_scene_lease_candidate *candidate);
bool stage_director_company_scene_lease_candidate_host_lost(
    struct stage_director_company_scene_lease_candidate *candidate,
    uintptr_t native_host_layer_ref);
bool stage_director_company_scene_lease_candidate_actor_lost(
    struct stage_director_company_scene_lease_candidate *candidate,
    uintptr_t native_actor_view_ref);
bool stage_director_company_scene_lease_candidate_output_lost(
    struct stage_director_company_scene_lease_candidate *candidate,
    uintptr_t native_output_ref);
bool stage_director_company_scene_lease_candidate_process_lost(
    struct stage_director_company_scene_lease_candidate *candidate,
    int64_t actor_pid, uint64_t actor_start_ticks);

#endif
