#include <stdint.h>
#include <string.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include "log.h"
#include "sway/layers.h"
#include "sway/server.h"
#include "sway/stage_director.h"
#include "sway/stage_director_company_native.h"
#include "sway/stage_director_company_vector_protocol.h"
#include "sway/stage_director_company_scene.h"
#include "sway/stage_director_company_scene.h"
#include "sway/input/cursor.h"
#include "sway/input/seat.h"

/* Native source selected by a compositor-owned physical hit test, NOT a
 * Quickshell PID, namespace string, focus or Sway command. No actor/XDG
 * surface is selected or authorized by this host-only gate. */
static struct {
	struct sway_layer_surface *host;
	uintptr_t layer_ref, role_ref, surface_ref, client_ref, output_ref;
	int64_t pid;
	uint32_t uid;
	uint64_t start_ticks;
	struct stage_director_rect host_global;
	struct stage_director_rect output_global;
	uint64_t revision;
} company_selection;

static bool same_rect(struct stage_director_rect a,
		struct stage_director_rect b) {
	return a.x == b.x && a.y == b.y &&
		a.width == b.width && a.height == b.height;
}

/* This callback runs on every M23 intent/report, not just at bind time.
 * It MUST NOT infer identity from the public namespace or screen coords. */
static bool probe_paired_native_host(void *context,
		struct stage_director_rect *host, uint64_t *revision) {
	(void)context;
	if (!company_selection.host || !host || !revision ||
			server.session_lock.lock) {
		return false;
	}
	const struct stage_director_company_native_host observed =
		stage_director_company_native_observe_host(
			company_selection.host);
	if (!observed.candidate ||
			observed.native_layer_ref != company_selection.layer_ref ||
			observed.native_layer_role_ref != company_selection.role_ref ||
			observed.native_surface_ref != company_selection.surface_ref ||
			observed.native_client_ref != company_selection.client_ref ||
			observed.native_output_ref != company_selection.output_ref ||
			observed.pid != company_selection.pid ||
			observed.uid != company_selection.uid ||
			observed.start_ticks != company_selection.start_ticks) {
		return false;
	}
	/* Host/output resizing and movement issue a new opaque revision.
	 * Output transfer or native root replacement fails the identity check. */
	if (!same_rect(company_selection.host_global,
				observed.host_global) ||
			!same_rect(company_selection.output_global,
				observed.output_global)) {
		/* The previously installed clip/scene is stale the instant the
		 * host moves or the selected output changes. Hide before creating
		 * a fresh generation; never wait for another pointer event. */
		stage_director_company_scene_revoke(NULL);
		if (company_selection.revision == UINT64_MAX) return false;
		company_selection.revision++;
		company_selection.host_global = observed.host_global;
		company_selection.output_global = observed.output_global;
	}
	*host = (struct stage_director_rect) {
		.x = 0,
		.y = 0,
		.width = observed.host_global.width,
		.height = observed.host_global.height,
	};
	*revision = company_selection.revision;
	return *revision != 0;
}

/* Private M28b callback runs on every pidfd receipt and XDG candidate
 * query. The physically selected native Company role/client/process must
 * still be the exact same object at that moment. */
static bool probe_paired_birth_host(void *context,
		struct stage_director_company_native_host *out) {
	if (!out) return false;
	struct stage_director_rect local = {0};
	uint64_t revision = 0;
	if (!probe_paired_native_host(context, &local, &revision)) return false;
	*out = stage_director_company_native_observe_host(
		company_selection.host);
	return out->candidate &&
		out->native_layer_ref == company_selection.layer_ref &&
		out->native_layer_role_ref == company_selection.role_ref &&
		out->native_surface_ref == company_selection.surface_ref &&
		out->native_client_ref == company_selection.client_ref &&
		out->native_output_ref == company_selection.output_ref &&
		out->pid == company_selection.pid &&
		out->uid == company_selection.uid &&
		out->start_ticks == company_selection.start_ticks;
}

void stage_director_company_pairing_clear_host(void) {
	/* Any previous native map actor must be hidden before revoking the
	 * selected Company Wayland host. Never leave an overlay after loss. */
	stage_director_company_scene_revoke(NULL);
	/* Clear local native references before destroying the published global.
	 * Any remaining bound resource is unusable until the next native epoch. */
	memset(&company_selection, 0, sizeof(company_selection));
	stage_director_company_vector_protocol_revoke();
}

bool stage_director_company_pairing_select_at(
		struct sway_seat *seat, double lx, double ly) {
	if (!seat || !seat->cursor || !seat->cursor->cursor ||
			server.session_lock.lock) {
		return false;
	}
	struct wlr_surface *hit = NULL;
	double sx = 0, sy = 0;
	(void)node_at_coords(seat, lx, ly, &hit, &sx, &sy);
	if (!hit) return false;
	struct wlr_surface *root = wlr_surface_get_root_surface(hit);
	if (!root || !root->mapped) return false;
	struct wlr_layer_surface_v1 *role =
		wlr_layer_surface_v1_try_from_wlr_surface(root);
	if (!role || role->surface != root || !role->initialized ||
			!role->resource || !root->resource ||
			!role->namespace ||
			strcmp(role->namespace, SD_COMPANY_NATIVE_NAMESPACE) ||
			role->current.layer != ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY) {
		return false;
	}
	struct sway_layer_surface *host = role->data;
	if (!host || host->layer_surface != role || !host->mapped ||
			!host->output) return false;
	const struct stage_director_company_native_host observed =
		stage_director_company_native_observe_host(host);
	if (!observed.candidate ||
			observed.native_layer_ref != (uintptr_t)host ||
			observed.native_layer_role_ref != (uintptr_t)role->resource ||
			observed.native_surface_ref != (uintptr_t)root->resource ||
			observed.native_client_ref !=
				(uintptr_t)wl_resource_get_client(root->resource)) {
		return false;
	}
	stage_director_company_pairing_clear_host();
	company_selection.host = host;
	company_selection.layer_ref = observed.native_layer_ref;
	company_selection.role_ref = observed.native_layer_role_ref;
	company_selection.surface_ref = observed.native_surface_ref;
	company_selection.client_ref = observed.native_client_ref;
	company_selection.output_ref = observed.native_output_ref;
	company_selection.pid = observed.pid;
	company_selection.uid = observed.uid;
	company_selection.start_ticks = observed.start_ticks;
	company_selection.host_global = observed.host_global;
	company_selection.output_global = observed.output_global;
	company_selection.revision = 1;
	if (!stage_director_company_vector_protocol_publish(
			role->resource, root->resource,
			probe_paired_native_host, NULL) ||
			!stage_director_company_vector_protocol_attach_birth_probe(
				role->resource, root->resource,
				probe_paired_birth_host, NULL)) {
		stage_director_company_pairing_clear_host();
		return false;
	}
	sway_log(SWAY_INFO, "[stage-director][COMPANY_M24][HOST] "
		"native Company layer physically selected; private wire=%s "
		"actor=NONE scene=OFF input=OFF",
		"candidate");
	return true;
}

void stage_director_company_pairing_host_changed(
		struct sway_layer_surface *host) {
	if (!host || company_selection.host != host) return;
	struct stage_director_rect ignored = {0};
	uint64_t revision = 0;
	if (!probe_paired_native_host(NULL, &ignored, &revision)) {
		stage_director_company_pairing_clear_host();
	}
}

void stage_director_company_pairing_host_lost(
		struct sway_layer_surface *host) {
	if (host && company_selection.host == host) {
		stage_director_company_pairing_clear_host();
	}
}
