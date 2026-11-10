#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_LAUNCH_REQUEST_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_LAUNCH_REQUEST_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_viewport.h"

/* M32r: an inert, compositor-local record of what Company requested.
 * It is NOT a physical approval, trusted launch, scene lease or privilege.
 * No Wayland/IPC/QML caller can choose a generation or authorize a scene.
 * Actual approval requires a separate trusted physical input producer. */
#define SD_COMPANY_LAUNCH_REQUEST_MAX_NS UINT64_C(10000000000)

struct stage_director_company_launch_request_identity {
    uint64_t selection_epoch;
    uint64_t intent_sequence;
    uint64_t host_revision;
    uintptr_t native_host_layer_ref;
    uintptr_t native_host_surface_ref;
    uintptr_t native_host_client_ref;
    uintptr_t native_output_ref;
    uint32_t mode; /* Wayland fixed mode: 0 offline, 1 online. */
    struct stage_director_rect map_in_host;
};

struct stage_director_company_launch_request {
    bool initialized;
    bool pending;
    uint64_t last_generation; /* High-water mark, never reset by revoke. */
    uint64_t generation;
    uint64_t started_ns;
    uint64_t expires_ns;
    struct stage_director_company_launch_request_identity identity;
};

enum stage_director_company_launch_request_state {
    SD_COMPANY_REQUEST_NONE,
    SD_COMPANY_REQUEST_STALE_GENERATION,
    SD_COMPANY_REQUEST_EXPIRED,
    SD_COMPANY_REQUEST_NATIVE_CHANGED,
    SD_COMPANY_REQUEST_CONTEXT_ONLY,
};

struct stage_director_company_launch_request_receipt {
    enum stage_director_company_launch_request_state state;
    bool current_request_context;
    /* Intentionally false for EVERY possible result. */
    bool operator_authenticated;
    bool launch_authenticated;
    bool scene_lease_issued;
    bool can_apply_scene;
    bool can_accept_input;
    bool can_focus_keyboard;
    bool can_show_popup;
};

void stage_director_company_launch_request_init(
    struct stage_director_company_launch_request *request);

/* Only a compositor re-observation of the selected host and validated
 * viewport may call reserve. Does not verify an actor, executable or input.
 * The returned generation is compositor-local, never sent to a client. */
bool stage_director_company_launch_request_reserve(
    struct stage_director_company_launch_request *request,
    const struct stage_director_company_launch_request_identity *identity,
    uint64_t now_ns, uint64_t ttl_ns, uint64_t *generation_out);

/* An old callback may NOT revoke a newer request. Current-generation
 * mismatch, clock rollback and expiry revoke this request fail-closed. */
struct stage_director_company_launch_request_receipt
stage_director_company_launch_request_probe(
    struct stage_director_company_launch_request *request,
    uint64_t expected_generation,
    const struct stage_director_company_launch_request_identity *observed,
    uint64_t now_ns);

bool stage_director_company_launch_request_revoke(
    struct stage_director_company_launch_request *request);

#endif
