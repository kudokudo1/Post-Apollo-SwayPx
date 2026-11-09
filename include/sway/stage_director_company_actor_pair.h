#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_ACTOR_PAIR_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_ACTOR_PAIR_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_company_native.h"
#include "sway/stage_director_company_origin.h"
#include "sway/stage_director_company_lineage.h"
#include "sway/stage_director_company_executable.h"
#include "sway/stage_director_company_digest.h"
#include "sway/stage_director_company_viewport_state.h"

/*
 * M32b // production Company viewport -> exact native XDG process pairing.
 *
 * Inputs are observations made by the compositor: selected Company host,
 * selected-host private Wayland intent + viewport, kernel-backed pidfd
 * correspondence, native XDG toplevel resources and /proc executable
 * evidence. Never populate these from app_id, titles, IPC, QML booleans,
 * client-provided pointers, process IDs or a caller's claimed digest.
 *
 * This module computes ONLY the prospective native target rectangle.
 * It cannot install/relocate a scene node, crop, focus or forward input.
 */
struct stage_director_company_actor_pair_evidence {
    bool selected_native_company_host;
    bool kernel_pidfd_actor_candidate;
    struct stage_director_company_native_host host;
    struct stage_director_company_native_actor actor;
    struct stage_director_company_intent_snapshot intent;
    struct stage_director_company_viewport_snapshot viewport;
    struct stage_director_company_lineage_result lineage;
    struct stage_director_company_executable_result executable;
    struct stage_director_company_digest_result digest;
};

enum stage_director_company_actor_pair_status {
    SD_COMPANY_PAIR_NO_SELECTED_HOST,
    SD_COMPANY_PAIR_NO_INTENT_OR_VIEWPORT,
    SD_COMPANY_PAIR_NATIVE_PEER_CHANGED,
    SD_COMPANY_PAIR_PROCESS_NOT_PROVEN,
    SD_COMPANY_PAIR_UNSAFE_TARGET,
    SD_COMPANY_PAIR_EXECUTABLE_NOT_PROVEN,
    SD_COMPANY_PAIR_DIGEST_UNPINNED,
    SD_COMPANY_PAIR_DIGEST_UNTRUSTED,
    SD_COMPANY_PAIR_EXACT_ELF_CANDIDATE_ONLY,
};

struct stage_director_company_actor_pair_result {
    enum stage_director_company_actor_pair_status status;
    /* Target geometry is never a license to move a native actor.
     * Only meaningful after matching current peer + kernel pidfd evidence. */
    bool native_process_candidate;
    bool target_geometry_candidate;
    bool executable_inode_candidate;
    bool exact_elf_digest_candidate;
    struct stage_director_rect viewport_global;
    struct stage_director_rect actor_root_observed_global;
    /* Future scene policy must separately verify output, mapping, current
     * pixel extent, exact launch approval, root crop and seat containment. */
    bool can_pair_actor;
    bool can_position;
    bool can_render;
    bool can_clip;
    bool can_accept_input;
    bool can_focus;
    bool can_show_popup;
};

/* Stateless, fail-closed source adapter. Never cache its output as authority;
 * any future render/seat operation must recheck current compositor objects. */
struct stage_director_company_actor_pair_result
stage_director_company_actor_pair_probe(
    const struct stage_director_company_actor_pair_evidence *observed);

const char *stage_director_company_actor_pair_status_name(
    enum stage_director_company_actor_pair_status status);

#endif
