#ifndef _SWAY_STAGE_DIRECTOR_WEATHER_HOST_H
#define _SWAY_STAGE_DIRECTOR_WEATHER_HOST_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_viewport.h"

/*
 * Weather Station host detection, not authentication or compositor authority.
 * The Weather UI declares the dedicated namespace in WeatherStationW.qml,
 * but any Wayland client can claim that string. The native peer reader must
 * verify resources and kernel process identity separately, still WITHOUT
 * interpreting a matching namespace/PID as WATCH authorization.
 */
#define STAGE_DIRECTOR_WEATHER_NAMESPACE "post-apollo-weather-station"

struct sway_layer_surface;

/* Populate exclusively from native compositor observations, never public IPC. */
struct stage_director_weather_host_observation {
	const char *layer_namespace;
	uintptr_t layer_ref;
	uintptr_t output_ref;
	bool initialized;
	bool mapped;
	bool overlay_layer;
	bool output_active;
	struct stage_director_rect host;
	struct stage_director_rect output;
};

enum stage_director_weather_host_status {
	SD_WEATHER_HOST_MISSING,
	SD_WEATHER_HOST_WRONG_NAMESPACE,
	SD_WEATHER_HOST_UNMAPPED,
	SD_WEATHER_HOST_WRONG_LAYER,
	SD_WEATHER_HOST_OUTPUT_INACTIVE,
	SD_WEATHER_HOST_GEOMETRY_INVALID,
	SD_WEATHER_HOST_OUTSIDE_OUTPUT,
	SD_WEATHER_HOST_CANDIDATE_ONLY,
};

struct stage_director_weather_host_result {
	enum stage_director_weather_host_status status;
	bool candidate_observed;
	/* Classifying a namespace NEVER verifies a trusted launch/session. */
	bool owner_verified;
	bool can_bind_viewer;
	bool can_clip;
};

struct stage_director_weather_host_result stage_director_weather_host_probe(
		const struct stage_director_weather_host_observation *observation);

/*
 * Native adapter for a live sway layer-shell surface. READ-ONLY.
 * Already called by the existing SD0 map/commit/unmap observer. It only
 * classifies a mapped layer, never grants process or render authority.
 */
struct stage_director_weather_host_native_peer {
	struct stage_director_weather_host_result host;
	/* In-process native compositor refs, never serialize to QML/IPC. */
	uintptr_t native_layer_ref;
	uintptr_t native_client_ref;
	uintptr_t native_output_ref;
	int64_t client_pid;
	uint32_t client_uid;
	uint64_t client_start_time_ticks;
	/* Means only same native wl_client + stable Linux PID/UID/starttime. */
	bool native_peer_candidate;
	bool owner_verified;
	bool can_launch;
	bool can_bind_viewer;
	bool can_clip;
	bool can_accept_input;
};

/* A fresh *read-only* view of a live layer-shell host's actual wl_client. */
struct stage_director_weather_host_native_peer
stage_director_weather_host_observe_native_peer(
		const struct sway_layer_surface *surface);

struct stage_director_weather_host_result stage_director_weather_host_observe_layer(
		const struct sway_layer_surface *surface);

#endif
