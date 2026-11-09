#include <string.h>
#include "sway/stage_director_company_scene_lease_candidate.h"

/* M32o: inert compositor-local bounded candidate ledger. It does not
 * publish a Wayland global, create a scene, issue authority, or read QML.
 * All observations must be supplied by a FUTURE trusted compositor caller,
 * after revalidating the actual M32m native/process/digest evidence. */
static bool complete_identity(
        const struct stage_director_company_scene_lease_identity *i) {
    return i && i->selection_epoch && i->intent_sequence &&
        i->host_revision && i->native_host_layer_ref &&
        i->native_host_surface_ref && i->native_host_client_ref &&
        i->native_output_ref && i->native_actor_view_ref &&
        i->native_actor_root_surface_ref && i->native_actor_client_ref &&
        i->actor_pid > 0 && i->actor_start_ticks &&
        i->native_host_client_ref != i->native_actor_client_ref &&
        i->native_host_surface_ref != i->native_actor_root_surface_ref &&
        i->native_host_layer_ref != i->native_actor_view_ref;
}

static bool current_conditions(
        const struct stage_director_company_scene_lease_conditions *c) {
    return c && c->selected_native_host && c->live_intent_and_viewport &&
        c->current_kernel_pidfd_actor && c->executable_inode_candidate &&
        c->exact_digest_candidate && c->host_mapped && c->actor_mapped &&
        c->output_active;
}

static bool same_identity(
        const struct stage_director_company_scene_lease_identity *a,
        const struct stage_director_company_scene_lease_identity *b) {
    return complete_identity(a) && complete_identity(b) &&
        a->selection_epoch == b->selection_epoch &&
        a->intent_sequence == b->intent_sequence &&
        a->host_revision == b->host_revision &&
        a->native_host_layer_ref == b->native_host_layer_ref &&
        a->native_host_surface_ref == b->native_host_surface_ref &&
        a->native_host_client_ref == b->native_host_client_ref &&
        a->native_output_ref == b->native_output_ref &&
        a->native_actor_view_ref == b->native_actor_view_ref &&
        a->native_actor_root_surface_ref == b->native_actor_root_surface_ref &&
        a->native_actor_client_ref == b->native_actor_client_ref &&
        a->actor_pid == b->actor_pid &&
        a->actor_start_ticks == b->actor_start_ticks;
}

static void empty_preserving_floor(
        struct stage_director_company_scene_lease_candidate *c) {
    const uint64_t last_epoch = c->last_epoch;
    memset(c, 0, sizeof(*c));
    c->initialized = true;
    c->state = SD_COMPANY_LEASE_CANDIDATE_EMPTY;
    c->last_epoch = last_epoch;
}

void stage_director_company_scene_lease_candidate_init(
        struct stage_director_company_scene_lease_candidate *c) {
    if (!c || c->initialized) return;
    memset(c, 0, sizeof(*c));
    c->initialized = true;
    c->state = SD_COMPANY_LEASE_CANDIDATE_EMPTY;
}

bool stage_director_company_scene_lease_candidate_reserve(
        struct stage_director_company_scene_lease_candidate *c,
        const struct stage_director_company_scene_lease_identity *identity,
        const struct stage_director_company_scene_lease_conditions *conditions,
        uint64_t monotonic_now_ns, uint64_t ttl_ns) {
    if (!c || !c->initialized ||
            c->state != SD_COMPANY_LEASE_CANDIDATE_EMPTY ||
            !complete_identity(identity) || !current_conditions(conditions) ||
            identity->selection_epoch <= c->last_epoch ||
            !monotonic_now_ns || !ttl_ns ||
            ttl_ns > SD_COMPANY_LEASE_CANDIDATE_MAX_NS ||
            monotonic_now_ns > UINT64_MAX - ttl_ns) {
        return false;
    }
    /* This is a candidate only. Reserve cannot return any scene privileges
     * and has no public or production issuer. Persist the high-water mark
     * BEFORE future expiry/revoke so old physical selection epochs fail. */
    c->identity = *identity;
    c->last_epoch = identity->selection_epoch;
    c->reserved_at_ns = monotonic_now_ns;
    c->expires_at_ns = monotonic_now_ns + ttl_ns;
    c->state = SD_COMPANY_LEASE_CANDIDATE_RESERVED;
    return true;
}

static struct stage_director_company_scene_lease_receipt receipt(
        enum stage_director_company_scene_lease_probe_state state) {
    return (struct stage_director_company_scene_lease_receipt) {
        .state = state,
        .native_candidate_current =
            state == SD_COMPANY_LEASE_MATCHED_CANDIDATE_ONLY,
        /* All authority fields are zero, including synthetic exact-match. */
    };
}

struct stage_director_company_scene_lease_receipt
stage_director_company_scene_lease_candidate_probe(
        struct stage_director_company_scene_lease_candidate *c,
        const struct stage_director_company_scene_lease_identity *observed,
        const struct stage_director_company_scene_lease_conditions *conditions,
        uint64_t monotonic_now_ns) {
    if (!c || !c->initialized ||
            c->state != SD_COMPANY_LEASE_CANDIDATE_RESERVED) {
        return receipt(SD_COMPANY_LEASE_NO_CANDIDATE);
    }
    /* Monotonic clock rollback, zero time and expiry all fail CLOSED. */
    if (!monotonic_now_ns || monotonic_now_ns < c->reserved_at_ns ||
            monotonic_now_ns >= c->expires_at_ns) {
        empty_preserving_floor(c);
        return receipt(SD_COMPANY_LEASE_EXPIRED);
    }
    if (!same_identity(&c->identity, observed)) {
        empty_preserving_floor(c);
        return receipt(SD_COMPANY_LEASE_NATIVE_CHANGED);
    }
    if (!current_conditions(conditions)) {
        empty_preserving_floor(c);
        return receipt(SD_COMPANY_LEASE_EVIDENCE_LOST);
    }
    return receipt(SD_COMPANY_LEASE_MATCHED_CANDIDATE_ONLY);
}

bool stage_director_company_scene_lease_candidate_revoke(
        struct stage_director_company_scene_lease_candidate *c) {
    if (!c || !c->initialized ||
            c->state != SD_COMPANY_LEASE_CANDIDATE_RESERVED) return false;
    empty_preserving_floor(c);
    return true;
}

bool stage_director_company_scene_lease_candidate_host_lost(
        struct stage_director_company_scene_lease_candidate *c,
        uintptr_t native_host_layer_ref) {
    return c && c->initialized &&
        c->state == SD_COMPANY_LEASE_CANDIDATE_RESERVED &&
        native_host_layer_ref &&
        c->identity.native_host_layer_ref == native_host_layer_ref &&
        stage_director_company_scene_lease_candidate_revoke(c);
}

bool stage_director_company_scene_lease_candidate_actor_lost(
        struct stage_director_company_scene_lease_candidate *c,
        uintptr_t native_actor_view_ref) {
    return c && c->initialized &&
        c->state == SD_COMPANY_LEASE_CANDIDATE_RESERVED &&
        native_actor_view_ref &&
        c->identity.native_actor_view_ref == native_actor_view_ref &&
        stage_director_company_scene_lease_candidate_revoke(c);
}

bool stage_director_company_scene_lease_candidate_output_lost(
        struct stage_director_company_scene_lease_candidate *c,
        uintptr_t native_output_ref) {
    return c && c->initialized &&
        c->state == SD_COMPANY_LEASE_CANDIDATE_RESERVED &&
        native_output_ref &&
        c->identity.native_output_ref == native_output_ref &&
        stage_director_company_scene_lease_candidate_revoke(c);
}

bool stage_director_company_scene_lease_candidate_process_lost(
        struct stage_director_company_scene_lease_candidate *c,
        int64_t actor_pid, uint64_t actor_start_ticks) {
    return c && c->initialized &&
        c->state == SD_COMPANY_LEASE_CANDIDATE_RESERVED &&
        actor_pid > 0 && actor_start_ticks &&
        c->identity.actor_pid == actor_pid &&
        c->identity.actor_start_ticks == actor_start_ticks &&
        stage_director_company_scene_lease_candidate_revoke(c);
}
