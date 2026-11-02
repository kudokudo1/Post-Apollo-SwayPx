#include <scenefx/types/fx/blur_data.h>
#include <stdint.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_scene.h>
#include "sway/stage_director_process_evidence.h"
#include "sway/layers.h"
#include "sway/output.h"
#include "sway/stage_director_weather_host.h"

/*
 * Native READ-ONLY bridge: derive candidate host bounds and layer identity
 * from the compositor's real mapped surface, not caller-supplied coordinates.
 * It does not bind any browser or grant the SD1 clip authority booleans.
 * The SD0 layer map/commit observer already calls this adapter. A
 * dedicated namespace now exists, but still does NOT authenticate WATCH.
 */
struct stage_director_weather_host_result stage_director_weather_host_observe_layer(
		const struct sway_layer_surface *surface) {
	if (!surface || !surface->layer_surface ||
			!surface->layer_surface->surface || !surface->scene ||
			!surface->scene->tree || !surface->output ||
			!surface->output->wlr_output) {
		return stage_director_weather_host_probe(NULL);
	}

	const struct wlr_layer_surface_v1 *layer = surface->layer_surface;
	struct wlr_box output_box = {0};
	output_get_box(surface->output, &output_box);
	int global_x = 0;
	int global_y = 0;
	wlr_scene_node_coords(&surface->scene->tree->node,
		&global_x, &global_y);

	const struct stage_director_weather_host_observation current = {
		.layer_namespace = layer->namespace,
		.layer_ref = (uintptr_t)surface,
		.output_ref = (uintptr_t)surface->output,
		.initialized = layer->initialized,
		.mapped = surface->mapped && layer->surface->mapped,
		.overlay_layer = layer->initialized &&
			layer->current.layer == ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY,
		.output_active = surface->output->enabled,
		.host = {
			.x = global_x,
			.y = global_y,
			.width = layer->surface->current.width,
			.height = layer->surface->current.height,
		},
		.output = {
			.x = output_box.x,
			.y = output_box.y,
			.width = output_box.width,
			.height = output_box.height,
		},
	};
	return stage_director_weather_host_probe(&current);
}

/*
 * Refresh real Wayland layer-shell host peer evidence from native wl_resource
 * objects plus kernel /proc identity. Both the layer-shell role resource and
 * the underlying wl_surface must belong to the same exact wl_client.
 *
 * This function records no process custody, calls no browser launcher, and
 * deliberately leaves every authority bit FALSE. Namespace similarity alone
 * cannot make native_peer_candidate true.
 */
struct stage_director_weather_host_native_peer
stage_director_weather_host_observe_native_peer(
		const struct sway_layer_surface *surface) {
	struct stage_director_weather_host_native_peer out = {
		.host = stage_director_weather_host_observe_layer(surface),
		.owner_verified = false,
		.can_launch = false,
		.can_bind_viewer = false,
		.can_clip = false,
		.can_accept_input = false,
	};
	if (!out.host.candidate_observed || !surface ||
			!surface->layer_surface || !surface->output) {
		return out;
	}
	const struct wlr_layer_surface_v1 *layer = surface->layer_surface;
	if (!layer->resource || !layer->surface ||
			!layer->surface->resource) {
		return out;
	}

	struct wl_client *layer_client =
		wl_resource_get_client(layer->resource);
	struct wl_client *surface_client =
		wl_resource_get_client(layer->surface->resource);
	if (!layer_client || !surface_client ||
			layer_client != surface_client) {
		return out;
	}
	pid_t peer_pid = -1;
	uid_t peer_uid = (uid_t)-1;
	wl_client_get_credentials(layer_client, &peer_pid, &peer_uid, NULL);
	if (peer_pid <= 0 || peer_uid == (uid_t)-1) {
		return out;
	}
	struct stage_director_process_identity first = {0};
	struct stage_director_process_identity second = {0};
	if (!stage_director_read_linux_process((int64_t)peer_pid,
			&first, NULL) ||
			!stage_director_read_linux_process((int64_t)peer_pid,
				&second, NULL) ||
			first.pid != peer_pid || second.pid != peer_pid ||
			first.uid != (uint32_t)peer_uid ||
			second.uid != (uint32_t)peer_uid ||
			first.start_time_ticks == 0 ||
			first.start_time_ticks != second.start_time_ticks ||
			first.parent_pid != second.parent_pid) {
		return out;
	}
	out.native_layer_ref = (uintptr_t)surface;
	out.native_client_ref = (uintptr_t)layer_client;
	out.native_output_ref = (uintptr_t)surface->output;
	out.client_pid = (int64_t)peer_pid;
	out.client_uid = (uint32_t)peer_uid;
	out.client_start_time_ticks = second.start_time_ticks;
	out.native_peer_candidate = true;
	return out;
}
