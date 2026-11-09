#include "sway/stage_director_company_actor_pair.h"
#include "sway/stage_director_company_wire_policy.h"

static struct stage_director_company_actor_pair_result denied(
        enum stage_director_company_actor_pair_status status) {
    return (struct stage_director_company_actor_pair_result) {
        .status = status,
    };
}

/*
 * M32b is not an actor issuer. These are instantaneous native observations.
 * Even an approved digest does not certify the loaded Qt libraries,
 * authorized launch origin, popup ownership or continuing scene custody.
 */
struct stage_director_company_actor_pair_result
stage_director_company_actor_pair_probe(
        const struct stage_director_company_actor_pair_evidence *e) {
    if (!e || !e->selected_native_company_host || !e->host.candidate ||
            e->host.state != SD_COMPANY_NATIVE_CANDIDATE_ONLY) {
        return denied(SD_COMPANY_PAIR_NO_SELECTED_HOST);
    }
    if (!e->intent.present || !e->viewport.candidate_only ||
            !e->intent.selection_epoch || !e->intent.sequence ||
            !e->intent.host_revision || !e->viewport.selection_epoch ||
            !e->viewport.intent_sequence || !e->viewport.host_revision ||
            e->intent.selection_epoch != e->viewport.selection_epoch ||
            e->intent.sequence != e->viewport.intent_sequence ||
            e->intent.host_revision != e->viewport.host_revision ||
            e->intent.native_host_client_ref !=
                    e->viewport.native_host_client_ref ||
            e->intent.mode > 1) {
        return denied(SD_COMPANY_PAIR_NO_INTENT_OR_VIEWPORT);
    }
    /* The selected host and native XDG root come from the compositor,
     * not an app-id/name match or the untrusted viewport report. */
    if (!e->host.native_layer_ref || !e->host.native_layer_role_ref ||
            !e->host.native_surface_ref || !e->host.native_client_ref ||
            !e->host.native_output_ref || !e->actor.candidate ||
            e->actor.state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
            !e->actor.native_view_ref ||
            !e->actor.native_root_surface_ref ||
            !e->actor.native_client_ref ||
            e->host.native_client_ref !=
                    e->viewport.native_host_client_ref ||
            e->host.native_client_ref ==
                    e->actor.native_client_ref ||
            e->host.native_surface_ref ==
                    e->actor.native_root_surface_ref) {
        return denied(SD_COMPANY_PAIR_NATIVE_PEER_CHANGED);
    }
    /* Only the trusted selected-client protocol may assert this predicate,
     * and only after exact held Linux pidfd matches the actor's native XDG
     * process generation and its direct Company parent. */
    if (!e->kernel_pidfd_actor_candidate ||
            e->lineage.status != SD_COMPANY_LINEAGE_CANDIDATE_ONLY ||
            !e->lineage.direct_child_candidate ||
            e->host.pid <= 0 || e->host.uid == 0 ||
            !e->host.start_ticks || e->actor.pid <= 0 ||
            e->actor.uid != e->host.uid || !e->actor.start_ticks ||
            e->host.pid == e->actor.pid ||
            e->host.start_ticks == e->actor.start_ticks ||
            e->intent.native_host_client_ref !=
                    e->host.native_client_ref) {
        return denied(SD_COMPANY_PAIR_PROCESS_NOT_PROVEN);
    }

    /* The M32a client rectangle is revalidated against the *current*
     * compositor host surface. It is an exact desired target even when
     * the current separate XDG surface is not yet placed over the panel. */
    const struct stage_director_rect host_local = {
        0, 0, e->host.host_global.width, e->host.host_global.height
    };
    const struct stage_director_company_wire_result wire =
        stage_director_company_wire_check(
            true, true, host_local,
            e->viewport.frame_in_host, e->viewport.map_in_frame);
    struct stage_director_rect target = {0};
    if (wire.state != SD_COMPANY_WIRE_VIEWPORT_CANDIDATE ||
            !wire.geometry_candidate ||
            wire.can_launch || wire.can_render || wire.can_clip ||
            wire.can_focus || wire.can_accept_input || wire.can_show_popup ||
            wire.map_in_host.x != e->viewport.map_in_host.x ||
            wire.map_in_host.y != e->viewport.map_in_host.y ||
            wire.map_in_host.width != e->viewport.map_in_host.width ||
            wire.map_in_host.height != e->viewport.map_in_host.height ||
            !stage_director_rect_valid(e->host.host_global) ||
            !stage_director_rect_contains(
                e->host.output_global, e->host.host_global) ||
            !stage_director_rect_valid(e->actor.root_global) ||
            !stage_director_viewport_from_host_local(
                e->host.host_global, wire.map_in_host, &target) ||
            !stage_director_rect_contains(e->host.output_global, target)) {
        return denied(SD_COMPANY_PAIR_UNSAFE_TARGET);
    }

    struct stage_director_company_actor_pair_result result = {
        .status = SD_COMPANY_PAIR_EXECUTABLE_NOT_PROVEN,
        .native_process_candidate = true,
        .target_geometry_candidate = true,
        .viewport_global = target,
        .actor_root_observed_global = e->actor.root_global,
        /* Never grant render/clip/seat, even with a matching hash. */
    };
    if (e->executable.status !=
                    SD_COMPANY_EXE_CURRENT_INSTALL_INODE_CANDIDATE ||
            !e->executable.running_current_install_inode_candidate) {
        return result;
    }
    result.executable_inode_candidate = true;
    if (e->digest.status == SD_COMPANY_DIGEST_UNPINNED) {
        result.status = SD_COMPANY_PAIR_DIGEST_UNPINNED;
        return result;
    }
    if (e->digest.status != SD_COMPANY_DIGEST_PIN_MATCH_CANDIDATE ||
            !e->digest.pinned_elf_hash_candidate) {
        result.status = SD_COMPANY_PAIR_DIGEST_UNTRUSTED;
        return result;
    }
    result.exact_elf_digest_candidate = true;
    result.status = SD_COMPANY_PAIR_EXACT_ELF_CANDIDATE_ONLY;
    return result;
}

const char *stage_director_company_actor_pair_status_name(
        enum stage_director_company_actor_pair_status status) {
    switch (status) {
    case SD_COMPANY_PAIR_NO_SELECTED_HOST: return "NO_SELECTED_HOST";
    case SD_COMPANY_PAIR_NO_INTENT_OR_VIEWPORT: return "NO_INTENT_OR_VIEWPORT";
    case SD_COMPANY_PAIR_NATIVE_PEER_CHANGED: return "NATIVE_PEER_CHANGED";
    case SD_COMPANY_PAIR_PROCESS_NOT_PROVEN: return "PROCESS_NOT_PROVEN";
    case SD_COMPANY_PAIR_UNSAFE_TARGET: return "UNSAFE_TARGET";
    case SD_COMPANY_PAIR_EXECUTABLE_NOT_PROVEN: return "EXECUTABLE_NOT_PROVEN";
    case SD_COMPANY_PAIR_DIGEST_UNPINNED: return "DIGEST_UNPINNED";
    case SD_COMPANY_PAIR_DIGEST_UNTRUSTED: return "DIGEST_UNTRUSTED";
    case SD_COMPANY_PAIR_EXACT_ELF_CANDIDATE_ONLY:
        return "EXACT_ELF_CANDIDATE_ONLY";
    }
    return "INVALID";
}
