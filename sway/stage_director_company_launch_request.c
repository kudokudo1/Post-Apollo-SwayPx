#include <limits.h>
#include <string.h>
#include "sway/stage_director_company_launch_request.h"

static bool valid_rect(struct stage_director_rect r) {
    return r.x >= 0 && r.y >= 0 && r.width > 0 && r.height > 0 &&
        (int64_t)r.x + r.width <= INT32_MAX &&
        (int64_t)r.y + r.height <= INT32_MAX;
}

static bool complete_identity(
        const struct stage_director_company_launch_request_identity *i) {
    return i && i->selection_epoch && i->intent_sequence &&
        i->host_revision && i->native_host_layer_ref &&
        i->native_host_surface_ref && i->native_host_client_ref &&
        i->native_output_ref && i->mode <= 1 &&
        valid_rect(i->map_in_host);
}

static bool same_identity(
        const struct stage_director_company_launch_request_identity *a,
        const struct stage_director_company_launch_request_identity *b) {
    return complete_identity(a) && complete_identity(b) &&
        a->selection_epoch == b->selection_epoch &&
        a->intent_sequence == b->intent_sequence &&
        a->host_revision == b->host_revision &&
        a->native_host_layer_ref == b->native_host_layer_ref &&
        a->native_host_surface_ref == b->native_host_surface_ref &&
        a->native_host_client_ref == b->native_host_client_ref &&
        a->native_output_ref == b->native_output_ref &&
        a->mode == b->mode &&
        a->map_in_host.x == b->map_in_host.x &&
        a->map_in_host.y == b->map_in_host.y &&
        a->map_in_host.width == b->map_in_host.width &&
        a->map_in_host.height == b->map_in_host.height;
}

void stage_director_company_launch_request_init(
        struct stage_director_company_launch_request *r) {
    if (!r || r->initialized) return;
    memset(r, 0, sizeof(*r));
    r->initialized = true;
}

bool stage_director_company_launch_request_revoke(
        struct stage_director_company_launch_request *r) {
    if (!r || !r->initialized || !r->pending) return false;
    r->pending = false;
    r->generation = 0;
    r->started_ns = 0;
    r->expires_ns = 0;
    memset(&r->identity, 0, sizeof(r->identity));
    /* last_generation is deliberately retained: a stale approval cannot
     * be replayed after a CLOSE, viewport change or new physical pairing. */
    return true;
}

bool stage_director_company_launch_request_reserve(
        struct stage_director_company_launch_request *r,
        const struct stage_director_company_launch_request_identity *id,
        uint64_t now_ns, uint64_t ttl_ns, uint64_t *generation_out) {
    if (generation_out) *generation_out = 0;
    if (!r || !r->initialized || !generation_out || r->pending ||
            !complete_identity(id) ||
            !now_ns || !ttl_ns ||
            ttl_ns > SD_COMPANY_LAUNCH_REQUEST_MAX_NS ||
            now_ns > UINT64_MAX - ttl_ns ||
            r->last_generation == UINT64_MAX) return false;
    r->last_generation++;
    r->generation = r->last_generation;
    r->started_ns = now_ns;
    r->expires_ns = now_ns + ttl_ns;
    r->identity = *id;
    r->pending = true;
    *generation_out = r->generation;
    return true;
}

static struct stage_director_company_launch_request_receipt receipt(
        enum stage_director_company_launch_request_state state) {
    return (struct stage_director_company_launch_request_receipt) {
        .state = state,
        .current_request_context =
            state == SD_COMPANY_REQUEST_CONTEXT_ONLY,
        /* Even valid exact match does not verify operator consent. */
    };
}

struct stage_director_company_launch_request_receipt
stage_director_company_launch_request_probe(
        struct stage_director_company_launch_request *r,
        uint64_t generation,
        const struct stage_director_company_launch_request_identity *observed,
        uint64_t now_ns) {
    if (!r || !r->initialized || !r->pending)
        return receipt(SD_COMPANY_REQUEST_NONE);
    /* Old physical events must never revoke a later request. */
    if (!generation || generation != r->generation)
        return receipt(SD_COMPANY_REQUEST_STALE_GENERATION);
    if (!now_ns || now_ns < r->started_ns ||
            now_ns >= r->expires_ns) {
        stage_director_company_launch_request_revoke(r);
        return receipt(SD_COMPANY_REQUEST_EXPIRED);
    }
    if (!same_identity(&r->identity, observed)) {
        stage_director_company_launch_request_revoke(r);
        return receipt(SD_COMPANY_REQUEST_NATIVE_CHANGED);
    }
    return receipt(SD_COMPANY_REQUEST_CONTEXT_ONLY);
}
