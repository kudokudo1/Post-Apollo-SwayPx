#ifndef _SWAY_STAGE_DIRECTOR_COMPANY_VECTOR_PROTOCOL_H
#define _SWAY_STAGE_DIRECTOR_COMPANY_VECTOR_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>
#include <wayland-server-core.h>
#include "sway/stage_director_viewport.h"
#include "sway/stage_director_company_viewport_state.h"
#include "sway/stage_director_company_launch_request.h"

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

/* M32a // The private Company Wayland viewport report is now available
 * to compositor-native code as a revocable, client/epoch/revision-bound
 * geometry candidate. This is the *actual production protocol* source;
 * it has NO ability to render, clip, focus or redirect seat events.
 * The compositor must revalidate native actor PIDFD/digest/selection
 * separately before using the rectangle for any later scene operation. */
bool stage_director_company_vector_protocol_viewport_snapshot(
    struct wl_client *selected_host_client,
    struct stage_director_company_viewport_snapshot *out);

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

/* M32m // Read-only, compositor-internal admission preflight.
 *
 * Reports the current selected host, real intent/viewport, kernel-held
 * actor birth and installed executable/digest evidence. It NEVER issues
 * an operator approval, launch custody, scene lease or input permission.
 * Even exact ELF verification ends at OPERATOR_LEASE_ABSENT.
 *
 * Caller must pass a compositor-observed XDG actor and native lineage.
 * No QML, Wayland request handler, IPC or app-id can supply approval.
 */
enum stage_director_company_admission_status {
    SD_COMPANY_ADMISSION_NO_SELECTED_HOST,
    SD_COMPANY_ADMISSION_NO_LIVE_INTENT,
    SD_COMPANY_ADMISSION_NO_CURRENT_VIEWPORT,
    SD_COMPANY_ADMISSION_NO_KERNEL_ACTOR,
    SD_COMPANY_ADMISSION_NATIVE_PAIR_CHANGED,
    SD_COMPANY_ADMISSION_EXECUTABLE_UNVERIFIED,
    SD_COMPANY_ADMISSION_DIGEST_UNPINNED,
    SD_COMPANY_ADMISSION_DIGEST_UNTRUSTED,
    SD_COMPANY_ADMISSION_OPERATOR_LEASE_ABSENT,
};
struct stage_director_company_admission_preflight {
    enum stage_director_company_admission_status status;
    bool native_pair_candidate;
    bool current_executable_candidate;
    bool exact_elf_digest_candidate;
    /* No production issuer exists: these must always remain false. */
    bool operator_authorized;
    bool authenticated_launch;
    bool scene_lease_issued;
    bool can_apply_scene;
    bool can_accept_input;
    bool can_focus_keyboard;
    bool can_show_popup;
};
struct stage_director_company_admission_preflight
stage_director_company_vector_protocol_admission_preflight(
    struct wl_client *selected_host_client,
    const struct stage_director_company_native_actor *native_actor,
    const struct stage_director_company_lineage_result *native_lineage);
const char *stage_director_company_admission_status_name(
    enum stage_director_company_admission_status status);

/* M32p // Internal compositor-only bridge to M32o's inert candidate.
 * Caller supplies a LIVE sway_view object; the protocol re-observes the
 * native XDG actor, physically selected Company host, M28b kernel pidfd,
 * installed ELF and approved digest on every reserve or probe.
 *
 * Existing compiled digest is empty, so the reserve path cannot succeed
 * on the normal operator build. Even a future candidate-only match has
 * ZERO operator, launch, scene, pointer, focus or popup authority.
 * Neither function is a Wayland request handler or public IPC.
 */
struct sway_view;
struct stage_director_company_scene_lease_receipt;
bool stage_director_company_vector_protocol_candidate_reserve_current(
    struct sway_view *native_view,
    const struct stage_director_company_lineage_result *native_lineage);
struct stage_director_company_scene_lease_receipt
stage_director_company_vector_protocol_candidate_probe_current(
    struct sway_view *native_view,
    const struct stage_director_company_lineage_result *native_lineage);

/* M32r: returns a fresh compositor-only request context, never approval.
 * A generation is generated and retained by the compositor; future trusted
 * input may compare it but public Wayland/IPC cannot issue authority. */
struct stage_director_company_launch_request_receipt
stage_director_company_vector_protocol_launch_request_probe_current(
    uint64_t compositor_generation);

/* All revocation paths (explicit CLOSE, native role/host destruction,
 * replacement, shutdown) remove any scene BEFORE resetting selection,
 * intent, viewport or held kernel actor identity. */
void stage_director_company_vector_protocol_revoke(void);
void stage_director_company_vector_protocol_finish(void);

#endif
