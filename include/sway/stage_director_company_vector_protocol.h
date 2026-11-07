#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_VECTOR_PROTOCOL_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_VECTOR_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>
#include <wayland-server-core.h>
#include "sway/stage_director_viewport.h"

/*
 * M23: dedicated nonauthorizing Company geometry/intent Wayland interface.
 * Initialized at compositor startup, but NEVER published automatically.
 * Only a future trusted compositor physical selection can call publish().
 * The production server has no caller to publish() in M23.
 *
 * The selection producer must validate a real mapped Company layer role,
 * wl_surface, native Wayland client, and operator intent before calling.
 * The callback MUST re-observe this host's native geometry on every request
 * and return false if native role, client, output or session changed.
 */
typedef bool (*stage_director_company_vector_host_probe)(
	void *context, struct stage_director_rect *host_local,
	uint64_t *host_revision);

void stage_director_company_vector_protocol_init(struct wl_display *display);
bool stage_director_company_vector_protocol_is_global(
	const struct wl_global *global);
bool stage_director_company_vector_protocol_selected_client(
	struct wl_client *client);

/* M27: compositor-internal native-client query after an actual private M23
 * launch_intent request. Never export this snapshot to public IPC or QML. */
struct stage_director_company_intent_snapshot;
bool stage_director_company_vector_protocol_intent_snapshot(
	struct wl_client *client,
	struct stage_director_company_intent_snapshot *out);

/* Bounded future compositor-only activation seam. Not called by M23. */
bool stage_director_company_vector_protocol_publish(
	struct wl_resource *native_layer_role,
	struct wl_resource *native_host_surface,
	stage_director_company_vector_host_probe probe,
	void *probe_context);
/* M28b: this callback is provided only by a physically selected M24
 * compositor-side Company layer. No user/client sends native pointers. */
struct stage_director_company_native_host;
struct stage_director_company_native_actor;
struct stage_director_company_lineage_result;
typedef bool (*stage_director_company_vector_native_host_probe)(
	void *context, struct stage_director_company_native_host *out);

/* After M24 publishes its exact real layer resources, attach the native
 * host snapshot observer. Legacy v1 publication stays compatible but cannot
 * admit v2 birth receipts without this additional compositor proof. */
bool stage_director_company_vector_protocol_attach_birth_probe(
	struct wl_resource *native_layer_role,
	struct wl_resource *native_host_surface,
	stage_director_company_vector_native_host_probe probe,
	void *probe_context);

/* Returns ONLY kernel child candidate evidence; never actor/scene authority. */
bool stage_director_company_vector_protocol_birth_actor_candidate(
	struct wl_client *selected_host_client,
	const struct stage_director_company_native_actor *actor,
	const struct stage_director_company_lineage_result *lineage);

/* M29 read-only, compositor-side executable-inode evidence. No path or
 * command from QML/Wayland is accepted. The result cannot authorize scene. */
struct stage_director_company_executable_result;
struct stage_director_company_executable_result
stage_director_company_vector_protocol_executable_evidence(
	struct wl_client *selected_host_client,
	const struct stage_director_company_native_actor *actor,
	const struct stage_director_company_lineage_result *lineage);

/* M30a compiled, source-pinned manifest match: diagnostic only.
 * No expected digest is supplied by a Wayland/QML client. */
struct stage_director_company_digest_result;
struct stage_director_company_digest_result
stage_director_company_vector_protocol_digest_evidence(
	struct wl_client *selected_host_client,
	const struct stage_director_company_native_actor *actor,
	const struct stage_director_company_lineage_result *lineage);

void stage_director_company_vector_protocol_revoke(void);
void stage_director_company_vector_protocol_finish(void);

#endif
