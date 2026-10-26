#include <scenefx/types/fx/blur_data.h>
#include <stdint.h>
#include <wlr/types/wlr_scene.h>
#include "sway/layers.h"
#include "sway/output.h"
#include "sway/stage_director_weather_host.h"

/*
 * Native READ-ONLY bridge: derive candidate host bounds and layer identity
 * from the compositor's real mapped surface, not caller-supplied coordinates.
 * It does not bind any browser or grant the SD1 clip authority booleans.
 * No callback currently calls this adapter. Do not wire it until QS sets a
 * dedicated namespace and the compositor authenticates launcher provenance.
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
