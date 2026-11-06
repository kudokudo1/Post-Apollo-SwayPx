#define _GNU_SOURCE
#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include "sway/stage_director_company_origin.h"
#include "sway/stage_director_company_birth_pidfd.h"
#include "sway/stage_director_company_executable.h"
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

struct company_binding {
	uint64_t epoch;
	bool launch_intent;
	bool viewport_candidate;
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
	/* Observed intent only. Neither online consent nor native actor birth
	 * can be established by this request. No process is started. */
	send_status(resource,
		POST_APOLLO_COMPANY_VECTOR_V1_STATUS_LAUNCH_CANDIDATE, revision);
}

static void handle_report_viewport(struct wl_client *client,
		struct wl_resource *resource,
		int32_t fx, int32_t fy, int32_t fw, int32_t fh,
		int32_t mx, int32_t my, int32_t mw, int32_t mh) {
	struct company_binding *b = wl_resource_get_user_data(resource);
	struct stage_director_rect host = {0};
	uint64_t revision = 0;
	if (!bound_is_current(client, resource) || !b || !b->launch_intent ||
			!fresh_native_host(&host, &revision)) {
		if (b) b->viewport_candidate = false;
		send_status(resource, POST_APOLLO_COMPANY_VECTOR_V1_STATUS_DENIED, 0);
		return;
	}
	/* Geometry is evaluated and acknowledged, NEVER installed as clip. */
	b->viewport_candidate = false;
	const struct stage_director_company_wire_result candidate =
		stage_director_company_wire_check(
			true, true, host,
			(struct stage_director_rect){fx, fy, fw, fh},
			(struct stage_director_rect){mx, my, mw, mh});
	b->viewport_candidate = candidate.geometry_candidate;
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
	struct company_binding *b = wl_resource_get_user_data(resource);
	b->launch_intent = false;
	b->viewport_candidate = false;
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
	if (bound_resource == resource) bound_resource = NULL;
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

void stage_director_company_vector_protocol_revoke(void) {
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
