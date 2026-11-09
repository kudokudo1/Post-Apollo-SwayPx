#define _GNU_SOURCE
#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include "sway/stage_director_company_origin.h"
#include "sway/stage_director_company_birth_pidfd.h"
#include "sway/stage_director_company_executable.h"
#include "sway/stage_director_company_digest.h"
#include "sway/stage_director_company_actor_pair.h"
#include "sway/stage_director_company_scene.h"
#include "sway/stage_director_company_native.h"
#include "sway/stage_director_company_scene_lease_candidate.h"
#include <stdint.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include "sway/stage_director_company_vector_protocol.h"
#include "sway/stage_director_company_wire_policy.h"
#include "post-apollo-company-vector-v1-protocol.h"

/*
 * Dedicated Company Wayland global, invisible until native selection.
 * Never reuse Weather's private interface, tokens or session.
 *
 * The module is compiled into SwayPX but publish() has NO production caller
 * at M23. A selection issuer must be independently reviewed in M24.
 */
static struct wl_display *company_display;
static struct wl_global *company_global;
static struct wl_resource *native_role;
static struct wl_resource *native_host_surface;
static struct wl_client *selected_client;
static struct wl_resource *bound_resource;
static struct wl_listener role_destroy;
static struct wl_listener host_destroy;
static bool listeners_attached;
static uint64_t selection_epoch;
static uint64_t last_epoch;
static uint64_t last_intent_sequence;
static stage_director_company_vector_host_probe native_host_probe;
static void *native_host_context;
static stage_director_company_vector_native_host_probe birth_native_host_probe;
static void *birth_native_host_context;
static struct stage_director_company_birth_pidfd birth_held;
/* M32p: compositor-held inert candidate. No public request or scene caller. */
static struct stage_director_company_scene_lease_candidate
    native_lease_candidate;

struct company_binding {
	uint64_t epoch;
	bool launch_intent;
	bool viewport_candidate;
	/* M32a: retained, revocable candidate from the real Company wire. */
	struct stage_director_company_viewport_state viewport;
	uint64_t intent_sequence;
	uint64_t intent_boot_ticks;
	uint32_t mode;
};

static bool selection_current(struct wl_client *client) {
	return client && client == selected_client &&
		native_role && native_host_surface &&
		wl_resource_get_client(native_role) == client &&
		wl_resource_get_client(native_host_surface) == client &&
		selection_epoch != 0 && native_host_probe;
}

bool stage_director_company_vector_protocol_is_global(
		const struct wl_global *global) {
	/* wl_global_create advertises before returning the global pointer.
	 * Match generated interface identity FIRST, not stored global pointer. */
	return global && wl_global_get_interface(global) ==
		&post_apollo_company_vector_v1_interface;
}

bool stage_director_company_vector_protocol_selected_client(
		struct wl_client *client) {
	return selection_current(client);
}

/* Read the same boot-relative clock as /proc/<pid>/stat starttime.
 * Unknown ticks are fail-closed; no timing assertion from a client.
 */
static uint64_t boot_ticks_now(void) {
	struct timespec stamp = {0};
	const long frequency = sysconf(_SC_CLK_TCK);
	if (frequency <= 0 || frequency > 1000000 ||
			clock_gettime(CLOCK_BOOTTIME, &stamp) != 0 ||
			stamp.tv_sec <= 0 || stamp.tv_nsec < 0 ||
			stamp.tv_nsec >= 1000000000L ||
			(uint64_t)stamp.tv_sec > UINT64_MAX / (uint64_t)frequency) {
		return 0;
	}
	return (uint64_t)stamp.tv_sec * (uint64_t)frequency +
		((uint64_t)stamp.tv_nsec * (uint64_t)frequency) / 1000000000ULL;
}

static void send_status(struct wl_resource *resource, uint32_t status,
		uint64_t revision) {
	post_apollo_company_vector_v1_send_result(
		resource, status, (uint32_t)(selection_epoch >> 32),
		(uint32_t)selection_epoch, (uint32_t)(revision >> 32),
		(uint32_t)revision);
}

static bool bound_is_current(struct wl_client *client,
		struct wl_resource *resource) {
	if (!client || !resource || client != wl_resource_get_client(resource) ||
			resource != bound_resource || !selection_current(client)) {
		return false;
	}
	const struct company_binding *b = wl_resource_get_user_data(resource);
	return b && b->epoch == selection_epoch;
}

static bool fresh_native_host(struct stage_director_rect *host,
		uint64_t *revision) {
	if (!selected_client || !native_host_probe || !host || !revision) {
		return false;
	}
	*host = (struct stage_director_rect){0};
	*revision = 0;
	if (!native_host_probe(native_host_context, host, revision) ||
			!*revision || !stage_director_rect_valid(*host) ||
			host->x != 0 || host->y != 0) {
		return false;
	}
	return true;
}

bool stage_director_company_vector_protocol_intent_snapshot(
		struct wl_client *client,
		struct stage_director_company_intent_snapshot *out) {
	if (!out) return false;
	*out = (struct stage_director_company_intent_snapshot){0};
	if (!bound_is_current(client, bound_resource)) return false;
	const struct company_binding *binding =
		wl_resource_get_user_data(bound_resource);
	struct stage_director_rect host = {0};
	uint64_t revision = 0;
	if (!binding || !binding->launch_intent ||
			!binding->intent_sequence || !binding->intent_boot_ticks ||
			!fresh_native_host(&host, &revision)) return false;
	*out = (struct stage_director_company_intent_snapshot) {
		.present = true,
		.selection_epoch = selection_epoch,
		.sequence = binding->intent_sequence,
		.observed_boot_ticks = binding->intent_boot_ticks,
		.host_revision = revision,
		.native_host_client_ref = (uintptr_t)client,
		.mode = binding->mode,
	};
	return true;
}

bool stage_director_company_vector_protocol_viewport_snapshot(
        struct wl_client *client,
        struct stage_director_company_viewport_snapshot *out) {
    if (!out) return false;
    *out = (struct stage_director_company_viewport_snapshot){0};
    if (!bound_is_current(client, bound_resource)) return false;
    const struct company_binding *binding =
        wl_resource_get_user_data(bound_resource);
    struct stage_director_rect host = {0};
    uint64_t revision = 0;
    if (!binding || !binding->launch_intent ||
            !binding->viewport_candidate ||
            !binding->intent_sequence || !binding->intent_boot_ticks ||
            !fresh_native_host(&host, &revision)) return false;
    return stage_director_company_viewport_state_snapshot(
        &binding->viewport, selection_epoch, binding->intent_sequence,
        revision, (uintptr_t)client, host, out);
}

/* The QML map rectangles below are only candidates. Native scene and seat
 * operations remain prohibited until a separately authenticated production
 * lease admits the exact compositor-observed XDG actor and selected host. */
static void handle_launch_intent(struct wl_client *client,
		struct wl_resource *resource, uint32_t mode) {
	struct stage_director_rect host = {0};
	uint64_t revision = 0;
	if (!bound_is_current(client, resource) ||
			(mode != POST_APOLLO_COMPANY_VECTOR_V1_MODE_OFFLINE &&
			 mode != POST_APOLLO_COMPANY_VECTOR_V1_MODE_ONLINE) ||
			!fresh_native_host(&host, &revision)) {
		send_status(resource, POST_APOLLO_COMPANY_VECTOR_V1_STATUS_DENIED, 0);
		return;
	}
	struct company_binding *b = wl_resource_get_user_data(resource);
	if (b->launch_intent) {
		send_status(resource, POST_APOLLO_COMPANY_VECTOR_V1_STATUS_DENIED,
			revision);
		return;
	}
	/* A real selection-client request creates one M27 intent generation.
	 * No process is launched by this protocol. Reject clock failures and
	 * sequence exhaustion rather than reusing an old generation. */
	const uint64_t now = boot_ticks_now();
	if (!now || last_intent_sequence == UINT64_MAX) {
		send_status(resource, POST_APOLLO_COMPANY_VECTOR_V1_STATUS_DENIED,
			revision);
		return;
	}
	b->intent_boot_ticks = now;
	b->intent_sequence = ++last_intent_sequence;
	b->mode = mode;
	b->launch_intent = true;
	b->viewport_candidate = false;
	stage_director_company_viewport_state_reset(&b->viewport);
	/* Observed intent only. Neither online consent nor native actor birth
	 * can be established by this request. No process is started. */
	send_status(resource,
		POST_APOLLO_COMPANY_VECTOR_V1_STATUS_LAUNCH_CANDIDATE, revision);
}

static void handle_report_viewport(struct wl_client *client,
		struct wl_resource *resource,
		int32_t fx, int32_t fy, int32_t fw, int32_t fh,
		int32_t mx, int32_t my, int32_t mw, int32_t mh) {
    /* Even an invalid current viewport report kills the previous
     * candidate; a stale/foreign reporter may not revoke a new owner. */
    if (bound_is_current(client, resource))
        stage_director_company_scene_lease_candidate_revoke(
            &native_lease_candidate);
	struct company_binding *b = wl_resource_get_user_data(resource);
	struct stage_director_rect host = {0};
	uint64_t revision = 0;
	if (!bound_is_current(client, resource) || !b || !b->launch_intent ||
			!fresh_native_host(&host, &revision)) {
		if (b) {
			b->viewport_candidate = false;
			stage_director_company_viewport_state_reset(&b->viewport);
		}
		send_status(resource, POST_APOLLO_COMPANY_VECTOR_V1_STATUS_DENIED, 0);
		return;
	}
	/* A new geometry report *replaces* the previous map rectangle, even
	 * when hidden or malformed. Never retain stale pixels or hit tests. */
	b->viewport_candidate = false;
	stage_director_company_viewport_state_reset(&b->viewport);
	const struct stage_director_company_wire_result candidate =
		stage_director_company_wire_check(
			true, true, host,
			(struct stage_director_rect){fx, fy, fw, fh},
			(struct stage_director_rect){mx, my, mw, mh});
	if (candidate.geometry_candidate) {
		b->viewport_candidate = stage_director_company_viewport_state_record(
			&b->viewport, selection_epoch, b->intent_sequence, revision,
			(uintptr_t)client, host,
			(struct stage_director_rect){fx, fy, fw, fh},
			(struct stage_director_rect){mx, my, mw, mh});
		if (!b->viewport_candidate) {
			send_status(resource,
				POST_APOLLO_COMPANY_VECTOR_V1_STATUS_DENIED, 0);
			return;
		}
	}
	send_status(resource, (uint32_t)candidate.state, revision);
}

/* A v2 Wayland fd is transferred by the selected host client. It remains
 * untrusted until the kernel proves the pidfd, process generation and
 * Quickshell direct parent. No M15 executable/scene authority is granted. */
static void handle_report_child_pidfd(struct wl_client *client,
		struct wl_resource *resource, uint32_t claimed_pid, int32_t incoming_fd) {
	struct stage_director_company_birth_result birth = {
		.status = SD_COMPANY_BIRTH_NO_INTENT,
	};
	struct stage_director_company_intent_snapshot intent = {0};
	struct stage_director_company_native_host host = {0};
	if (wl_resource_get_version(resource) >= 2 &&
			!birth_held.held &&
			bound_is_current(client, resource) &&
			birth_native_host_probe &&
			stage_director_company_vector_protocol_intent_snapshot(
				client, &intent) &&
			birth_native_host_probe(birth_native_host_context, &host) &&
			host.candidate &&
			host.native_layer_role_ref == (uintptr_t)native_role &&
			host.native_surface_ref == (uintptr_t)native_host_surface &&
			host.native_client_ref == (uintptr_t)client) {
		birth = stage_director_company_birth_pidfd_record(
			&birth_held, incoming_fd, claimed_pid, &intent, &host,
			stage_director_read_linux_process, NULL);
	}
	/* libwayland-server delivers an owned fd with the request. Always
	 * close it, even on denial. M28a holds only a separate CLOEXEC dup. */
	if (incoming_fd >= 0) close(incoming_fd);
	if (wl_resource_get_version(resource) >= 2) {
		const uint32_t state =
			birth.status == SD_COMPANY_BIRTH_HELD_CANDIDATE_ONLY
				? POST_APOLLO_COMPANY_VECTOR_V1_BIRTH_STATUS_KERNEL_CANDIDATE
				: birth.status == SD_COMPANY_BIRTH_EXHAUSTED
					? POST_APOLLO_COMPANY_VECTOR_V1_BIRTH_STATUS_EXHAUSTED
					: POST_APOLLO_COMPANY_VECTOR_V1_BIRTH_STATUS_DENIED;
		post_apollo_company_vector_v1_send_birth_result(resource, state);
	}
}

static void handle_close(struct wl_client *client,
		struct wl_resource *resource) {
	if (!bound_is_current(client, resource)) {
		send_status(resource, POST_APOLLO_COMPANY_VECTOR_V1_STATUS_DENIED, 0);
		return;
	}
    stage_director_company_scene_lease_candidate_revoke(
        &native_lease_candidate);
	/* M32n: first hide/quarantine the mapped native scene while the
	 * selected binding, intent and process provenance are still intact.
	 * Preserve the existing CLOSED response epoch by sending it before
	 * protocol_revoke() clears selection_epoch. */
	stage_director_company_scene_revoke(NULL);
	struct company_binding *b = wl_resource_get_user_data(resource);
	b->launch_intent = false;
	b->viewport_candidate = false;
	stage_director_company_viewport_state_reset(&b->viewport);
	b->intent_boot_ticks = 0;
	b->intent_sequence = 0;
	send_status(resource, POST_APOLLO_COMPANY_VECTOR_V1_STATUS_CLOSED, 0);
	stage_director_company_vector_protocol_revoke();
}

static void handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	(void)client;
	wl_resource_destroy(resource);
}

static void destroy_binding(struct wl_resource *resource) {
	/* M32n: an actual Wayland resource destroy, including implicit client
	 * disconnect, must revoke native scene/selection/pidfd BEFORE free.
	 * An old inert binding cannot revoke a newer selected session. */
	if (bound_resource == resource) {
		stage_director_company_vector_protocol_revoke();
	}
	free(wl_resource_get_user_data(resource));
}

static const struct post_apollo_company_vector_v1_interface company_impl = {
	.launch_intent = handle_launch_intent,
	.report_viewport = handle_report_viewport,
	.report_child_pidfd = handle_report_child_pidfd,
	.close = handle_close,
	.destroy = handle_destroy,
};

static void bind_company(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	(void)data;
	if ((version != 1 && version != 2) ||
			!selection_current(client) || bound_resource) {
		wl_client_post_implementation_error(client,
			"Company vector bind requires the exact selected native host");
		return;
	}
	struct wl_resource *resource = wl_resource_create(client,
		&post_apollo_company_vector_v1_interface, version, id);
	if (!resource) {
		wl_client_post_no_memory(client);
		return;
	}
	struct company_binding *b = calloc(1, sizeof(*b));
	if (!b) {
		wl_resource_destroy(resource);
		wl_client_post_no_memory(client);
		return;
	}
	b->epoch = selection_epoch;
	wl_resource_set_implementation(resource, &company_impl, b, destroy_binding);
	bound_resource = resource;
}

static void native_resource_destroyed(struct wl_listener *listener,
		void *data) {
	(void)listener;
	(void)data;
	stage_director_company_vector_protocol_revoke();
}

void stage_director_company_vector_protocol_init(struct wl_display *display) {
	if (!company_display) company_display = display;
	stage_director_company_birth_pidfd_init(&birth_held);
    stage_director_company_scene_lease_candidate_init(
        &native_lease_candidate);
}

bool stage_director_company_vector_protocol_publish(
		struct wl_resource *role, struct wl_resource *surface,
		stage_director_company_vector_host_probe probe, void *context) {
	if (!company_display || company_global || selected_client ||
			listeners_attached || !role || !surface || role == surface ||
			!probe || last_epoch == UINT64_MAX) {
		return false;
	}
	struct wl_client *client = wl_resource_get_client(role);
	if (!client || wl_resource_get_client(surface) != client) return false;
	/* The compositor selection producer must prove both resources are the
	 * exact live native Company layer/host. This API never accepts a PID or
	 * namespace token from Wayland requests. */
	native_host_probe = probe;
	native_host_context = context;
	selected_client = client;
	native_role = role;
	native_host_surface = surface;
	selection_epoch = ++last_epoch;
	struct stage_director_rect host = {0};
	uint64_t revision = 0;
	if (!fresh_native_host(&host, &revision)) {
		stage_director_company_vector_protocol_revoke();
		return false;
	}

	role_destroy.notify = native_resource_destroyed;
	host_destroy.notify = native_resource_destroyed;
	wl_resource_add_destroy_listener(role, &role_destroy);
	wl_resource_add_destroy_listener(surface, &host_destroy);
	listeners_attached = true;
	company_global = wl_global_create(company_display,
		&post_apollo_company_vector_v1_interface, 2, NULL, bind_company);
	if (!company_global) {
		stage_director_company_vector_protocol_revoke();
		return false;
	}
	return true;
}

bool stage_director_company_vector_protocol_attach_birth_probe(
		struct wl_resource *role, struct wl_resource *surface,
		stage_director_company_vector_native_host_probe probe,
		void *context) {
	if (!role || !surface || !probe || role != native_role ||
			surface != native_host_surface || !company_global ||
			!selected_client || birth_native_host_probe ||
			wl_resource_get_client(role) != selected_client ||
			wl_resource_get_client(surface) != selected_client) {
		return false;
	}
	struct stage_director_company_native_host observed = {0};
	if (!probe(context, &observed) || !observed.candidate ||
			observed.native_layer_role_ref != (uintptr_t)role ||
			observed.native_surface_ref != (uintptr_t)surface ||
			observed.native_client_ref != (uintptr_t)selected_client) {
		return false;
	}
	birth_native_host_probe = probe;
	birth_native_host_context = context;
	return true;
}

bool stage_director_company_vector_protocol_birth_actor_candidate(
		struct wl_client *client,
		const struct stage_director_company_native_actor *actor,
		const struct stage_director_company_lineage_result *lineage) {
	if (!selection_current(client) || !birth_held.held ||
			!birth_native_host_probe) return false;
	struct stage_director_company_intent_snapshot intent = {0};
	struct stage_director_company_native_host host = {0};
	if (!stage_director_company_vector_protocol_intent_snapshot(
				client, &intent) ||
			!birth_native_host_probe(birth_native_host_context, &host) ||
			!host.candidate ||
			intent.selection_epoch != birth_held.selection_epoch ||
			intent.sequence != birth_held.intent_sequence ||
			intent.host_revision != birth_held.host_revision ||
			intent.mode != birth_held.mode) {
		return false;
	}
	const struct stage_director_company_birth_result result =
		stage_director_company_birth_pidfd_match_actor(
			&birth_held, &host, actor, lineage,
			stage_director_read_linux_process, NULL);
	return result.status == SD_COMPANY_BIRTH_ACTOR_CANDIDATE_ONLY &&
		result.mapped_actor_candidate &&
		!result.exact_executable_attested && !result.owner_verified &&
		!result.can_pair_actor && !result.can_clip &&
		!result.can_accept_input;
}

/* M29 can run only after the exact M24 selected host, active M27
 * request, held M28b pidfd, M20 XDG peer, and M22 lineage all match.
 * The passwd home path is obtained inside the compositor, not from QML.
 * Even a successful same-inode match never creates executable approval. */
struct stage_director_company_executable_result
stage_director_company_vector_protocol_executable_evidence(
		struct wl_client *client,
		const struct stage_director_company_native_actor *actor,
		const struct stage_director_company_lineage_result *lineage) {
	if (!stage_director_company_vector_protocol_birth_actor_candidate(
			client, actor, lineage)) {
		return (struct stage_director_company_executable_result) {
			.status = SD_COMPANY_EXE_NO_PIDFD,
		};
	}
	return stage_director_company_executable_check_system_home(&birth_held);
}

/* M30a never accepts an expected digest from Wayland or QML.
 * Only after the existing native actor/pidfd gate is current can the
 * compositor compare the kernel /proc/PID/exe to its compiled manifest. */
struct stage_director_company_digest_result
stage_director_company_vector_protocol_digest_evidence(
		struct wl_client *client,
		const struct stage_director_company_native_actor *actor,
		const struct stage_director_company_lineage_result *lineage) {
	if (!stage_director_company_vector_protocol_birth_actor_candidate(
			client, actor, lineage)) {
		return (struct stage_director_company_digest_result) {
			.status = SD_COMPANY_DIGEST_INODE_UNVERIFIED,
		};
	}
	return stage_director_company_digest_check_system_home(&birth_held);
}

/* M32m: one compositor-owned READ-ONLY observation boundary.
 * All positive statuses are candidates. The actual scene authorizer must
 * independently authenticate operator intent, exact launch custody and
 * a short-lived revocable compositor lease. This function cannot do so. */
static struct stage_director_company_admission_preflight
admission_status(enum stage_director_company_admission_status state,
        bool pair, bool executable, bool digest) {
    return (struct stage_director_company_admission_preflight) {
        .status = state,
        .native_pair_candidate = pair,
        .current_executable_candidate = executable,
        .exact_elf_digest_candidate = digest,
        /* Remaining authority bits deliberately zero on EVERY path. */
    };
}

struct stage_director_company_admission_preflight
stage_director_company_vector_protocol_admission_preflight(
        struct wl_client *client,
        const struct stage_director_company_native_actor *actor,
        const struct stage_director_company_lineage_result *lineage) {
    if (!selection_current(client) || !birth_native_host_probe)
        return admission_status(SD_COMPANY_ADMISSION_NO_SELECTED_HOST,
            false,false,false);
    struct stage_director_company_intent_snapshot intent = {0};
    if (!stage_director_company_vector_protocol_intent_snapshot(
                client, &intent))
        return admission_status(SD_COMPANY_ADMISSION_NO_LIVE_INTENT,
            false,false,false);
    struct stage_director_company_viewport_snapshot viewport = {0};
    if (!stage_director_company_vector_protocol_viewport_snapshot(
                client, &viewport))
        return admission_status(SD_COMPANY_ADMISSION_NO_CURRENT_VIEWPORT,
            false,false,false);
    if (!actor || !lineage || !stage_director_company_vector_protocol_birth_actor_candidate(
                client, actor, lineage))
        return admission_status(SD_COMPANY_ADMISSION_NO_KERNEL_ACTOR,
            false,false,false);

    /* No native actor/host pointer can originate in a public Wayland
     * message. Re-observe the compositor's selected layer NOW, not from
     * an old protocol report. */
    struct stage_director_company_native_host host = {0};
    if (!birth_native_host_probe(birth_native_host_context, &host))
        return admission_status(SD_COMPANY_ADMISSION_NATIVE_PAIR_CHANGED,
            false,false,false);
    const struct stage_director_company_executable_result executable =
        stage_director_company_vector_protocol_executable_evidence(
            client, actor, lineage);
    /* The compiled manifest is intentionally empty until an independent
     * operator-approved executable is pinned in a reviewed build. */
    const struct stage_director_company_digest_result digest =
        stage_director_company_vector_protocol_digest_evidence(
            client, actor, lineage);
    const struct stage_director_company_actor_pair_evidence evidence = {
        .selected_native_company_host = true,
        .kernel_pidfd_actor_candidate = true,
        .host = host,
        .actor = *actor,
        .intent = intent,
        .viewport = viewport,
        .lineage = *lineage,
        .executable = executable,
        .digest = digest,
    };
    const struct stage_director_company_actor_pair_result pair =
        stage_director_company_actor_pair_probe(&evidence);
    if (!pair.native_process_candidate || !pair.target_geometry_candidate ||
            pair.status == SD_COMPANY_PAIR_NO_SELECTED_HOST ||
            pair.status == SD_COMPANY_PAIR_NO_INTENT_OR_VIEWPORT ||
            pair.status == SD_COMPANY_PAIR_NATIVE_PEER_CHANGED ||
            pair.status == SD_COMPANY_PAIR_PROCESS_NOT_PROVEN ||
            pair.status == SD_COMPANY_PAIR_UNSAFE_TARGET)
        return admission_status(SD_COMPANY_ADMISSION_NATIVE_PAIR_CHANGED,
            false,false,false);
    if (!pair.executable_inode_candidate)
        return admission_status(SD_COMPANY_ADMISSION_EXECUTABLE_UNVERIFIED,
            true,false,false);
    if (pair.status == SD_COMPANY_PAIR_DIGEST_UNPINNED)
        return admission_status(SD_COMPANY_ADMISSION_DIGEST_UNPINNED,
            true,true,false);
    if (!pair.exact_elf_digest_candidate ||
            pair.status != SD_COMPANY_PAIR_EXACT_ELF_CANDIDATE_ONLY)
        return admission_status(SD_COMPANY_ADMISSION_DIGEST_UNTRUSTED,
            true,true,false);
    return admission_status(SD_COMPANY_ADMISSION_OPERATOR_LEASE_ABSENT,
        true,true,true);
}

const char *stage_director_company_admission_status_name(
        enum stage_director_company_admission_status state) {
    switch (state) {
    case SD_COMPANY_ADMISSION_NO_SELECTED_HOST: return "NO_SELECTED_HOST";
    case SD_COMPANY_ADMISSION_NO_LIVE_INTENT: return "NO_LIVE_INTENT";
    case SD_COMPANY_ADMISSION_NO_CURRENT_VIEWPORT: return "NO_CURRENT_VIEWPORT";
    case SD_COMPANY_ADMISSION_NO_KERNEL_ACTOR: return "NO_KERNEL_ACTOR";
    case SD_COMPANY_ADMISSION_NATIVE_PAIR_CHANGED: return "NATIVE_PAIR_CHANGED";
    case SD_COMPANY_ADMISSION_EXECUTABLE_UNVERIFIED: return "EXECUTABLE_UNVERIFIED";
    case SD_COMPANY_ADMISSION_DIGEST_UNPINNED: return "DIGEST_UNPINNED";
    case SD_COMPANY_ADMISSION_DIGEST_UNTRUSTED: return "DIGEST_UNTRUSTED";
    case SD_COMPANY_ADMISSION_OPERATOR_LEASE_ABSENT: return "OPERATOR_LEASE_ABSENT";
    }
    return "INVALID";
}

/* M32p // The candidate is produced from CURRENT compositor-native source
 * observations, not a Wayland command, app_id, PID string or cached QML
 * rectangle. The existing M32m preflight must reach its final blocker,
 * OPERATOR_LEASE_ABSENT. The caller can only create an inert M32o
 * CANDIDATE, which has no renderer or seat permission.
 */
static uint64_t candidate_boot_ns(void) {
    struct timespec stamp = {0};
    if (clock_gettime(CLOCK_BOOTTIME, &stamp) != 0 ||
            stamp.tv_sec <= 0 || stamp.tv_nsec < 0 ||
            stamp.tv_nsec >= 1000000000L ||
            (uint64_t)stamp.tv_sec >
                (UINT64_MAX - (uint64_t)stamp.tv_nsec) /
                    UINT64_C(1000000000)) return 0;
    return (uint64_t)stamp.tv_sec * UINT64_C(1000000000) +
        (uint64_t)stamp.tv_nsec;
}

static bool current_native_lease_observation(
        struct sway_view *view,
        const struct stage_director_company_lineage_result *lineage,
        struct stage_director_company_scene_lease_identity *identity,
        struct stage_director_company_scene_lease_conditions *conditions) {
    if (!view || !lineage || !identity || !conditions ||
            !selected_client || !birth_native_host_probe) return false;
    *identity = (struct stage_director_company_scene_lease_identity){0};
    *conditions = (struct stage_director_company_scene_lease_conditions){0};

    /* Re-observe the exact mapped XDG root from the compositor's sway_view.
     * The passed view is not a Wayland argument or user-provided address. */
    const struct stage_director_company_native_actor actor =
        stage_director_company_native_observe_actor(view);
    if (!actor.candidate || actor.state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
            actor.native_view_ref != (uintptr_t)view) return false;

    const struct stage_director_company_admission_preflight preflight =
        stage_director_company_vector_protocol_admission_preflight(
            selected_client, &actor, lineage);
    if (preflight.status != SD_COMPANY_ADMISSION_OPERATOR_LEASE_ABSENT ||
            !preflight.native_pair_candidate ||
            !preflight.current_executable_candidate ||
            !preflight.exact_elf_digest_candidate ||
            preflight.operator_authorized ||
            preflight.authenticated_launch ||
            preflight.scene_lease_issued || preflight.can_apply_scene ||
            preflight.can_accept_input || preflight.can_focus_keyboard ||
            preflight.can_show_popup) return false;

    struct stage_director_company_intent_snapshot intent = {0};
    struct stage_director_company_viewport_snapshot viewport = {0};
    struct stage_director_company_native_host host = {0};
    if (!stage_director_company_vector_protocol_intent_snapshot(
                selected_client, &intent) ||
            !stage_director_company_vector_protocol_viewport_snapshot(
                selected_client, &viewport) ||
            !birth_native_host_probe(birth_native_host_context, &host) ||
            !host.candidate ||
            host.state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
            !viewport.candidate_only ||
            intent.selection_epoch != viewport.selection_epoch ||
            intent.sequence != viewport.intent_sequence ||
            intent.host_revision != viewport.host_revision ||
            intent.native_host_client_ref != viewport.native_host_client_ref ||
            host.native_client_ref != intent.native_host_client_ref)
        return false;

    *identity = (struct stage_director_company_scene_lease_identity) {
        .selection_epoch = intent.selection_epoch,
        .intent_sequence = intent.sequence,
        .host_revision = intent.host_revision,
        .native_host_layer_ref = host.native_layer_ref,
        .native_host_surface_ref = host.native_surface_ref,
        .native_host_client_ref = host.native_client_ref,
        .native_output_ref = host.native_output_ref,
        .native_actor_view_ref = actor.native_view_ref,
        .native_actor_root_surface_ref = actor.native_root_surface_ref,
        .native_actor_client_ref = actor.native_client_ref,
        .actor_pid = actor.pid,
        .actor_start_ticks = actor.start_ticks,
    };
    *conditions = (struct stage_director_company_scene_lease_conditions) {
        .selected_native_host = true,
        .live_intent_and_viewport = true,
        .current_kernel_pidfd_actor = true,
        .executable_inode_candidate =
            preflight.current_executable_candidate,
        .exact_digest_candidate = preflight.exact_elf_digest_candidate,
        .host_mapped = host.candidate,
        .actor_mapped = actor.candidate,
        .output_active = host.candidate,
    };
    return true;
}

bool stage_director_company_vector_protocol_candidate_reserve_current(
        struct sway_view *view,
        const struct stage_director_company_lineage_result *lineage) {
    struct stage_director_company_scene_lease_identity identity = {0};
    struct stage_director_company_scene_lease_conditions conditions = {0};
    if (!current_native_lease_observation(
            view, lineage, &identity, &conditions)) return false;
    const uint64_t now = candidate_boot_ns();
    return stage_director_company_scene_lease_candidate_reserve(
        &native_lease_candidate, &identity, &conditions, now,
        SD_COMPANY_LEASE_CANDIDATE_MAX_NS);
}

struct stage_director_company_scene_lease_receipt
stage_director_company_vector_protocol_candidate_probe_current(
        struct sway_view *view,
        const struct stage_director_company_lineage_result *lineage) {
    struct stage_director_company_scene_lease_identity identity = {0};
    struct stage_director_company_scene_lease_conditions conditions = {0};
    /* Failed reobservation MUST revoke an existing reservation. */
    const bool current = current_native_lease_observation(
        view, lineage, &identity, &conditions);
    return stage_director_company_scene_lease_candidate_probe(
        &native_lease_candidate,
        current ? &identity : NULL,
        current ? &conditions : NULL, candidate_boot_ns());
}

/* Revoking the selected Company protocol MUST hide and quarantine any
 * already-admitted native Qt actor first. The private compositor positive
 * test can have a scene; the normal compositor cannot. In both cases,
 * this operation is safe, one-way and grants no new authority. */
void stage_director_company_vector_protocol_revoke(void) {
    /* Drop every private candidate before scene and kernel provenance. */
    stage_director_company_scene_lease_candidate_revoke(
        &native_lease_candidate);
    stage_director_company_scene_revoke(NULL);
	/* Wipe the last admitted geometry before releasing native host refs.
	 * Already-read snapshots are not leases: consumers must re-query. */
	if (bound_resource) {
		struct company_binding *binding =
			wl_resource_get_user_data(bound_resource);
		if (binding) {
			binding->viewport_candidate = false;
			stage_director_company_viewport_state_reset(&binding->viewport);
		}
	}
	if (company_global) {
		wl_global_destroy(company_global);
		company_global = NULL;
	}
	/* On destruction callbacks, remove native listeners before resetting
	 * borrowed wl_resource references. Do not access destroyed resources. */
	if (listeners_attached) {
		wl_list_remove(&role_destroy.link);
		wl_list_remove(&host_destroy.link);
		wl_list_init(&role_destroy.link);
		wl_list_init(&host_destroy.link);
		listeners_attached = false;
	}
	native_role = NULL;
	native_host_surface = NULL;
	selected_client = NULL;
	selection_epoch = 0;
	native_host_probe = NULL;
	native_host_context = NULL;
	birth_native_host_probe = NULL;
	birth_native_host_context = NULL;
	stage_director_company_birth_pidfd_revoke(&birth_held);
	bound_resource = NULL;
	/* Previously bound resource remains inert: every request checks the
	 * epoch/client again; on its destruction the binding is freed. */
}

void stage_director_company_vector_protocol_finish(void) {
	stage_director_company_vector_protocol_revoke();
	company_display = NULL;
}
