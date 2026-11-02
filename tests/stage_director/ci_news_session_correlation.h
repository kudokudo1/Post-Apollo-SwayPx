#ifndef SD_CI_NEWS_SESSION_CORRELATION_H
#define SD_CI_NEWS_SESSION_CORRELATION_H

#include <stdbool.h>
#include <stdint.h>
#include "sway/stage_director_process_evidence.h"
#include "sway/stage_director_viewer_session.h"
#include "sway/stage_director_viewport.h"
#include "sway/stage_director_weather_host.h"

/*
 * ISOLATED CI ONLY.
 *
 * An evidence correlation is NOT a trusted browser launch, root-surface
 * attestation, compositor authority or input capability. Native refs here
 * are synthetic in tests; any future real adapter must source them inside
 * the compositor rather than accepting QML or public IPC identifiers.
 *
 * Keep this code in tests/ until a separately approved launcher and native
 * Wayland-connection authority can produce actual provenance. Do not add
 * to sway_sources, export to IPC, or call in production repaint.
 */
enum sd_ci_news_join_state {
	SD_CI_NEWS_NO_SESSION,
	SD_CI_NEWS_UNVERIFIED_HOST_CANDIDATE,
	SD_CI_NEWS_STALE_LINK,
	SD_CI_NEWS_INVALID_ROOT_SURFACE,
	SD_CI_NEWS_BAD_GEOMETRY,
	SD_CI_NEWS_ROOT_PROCESS_MISMATCH,
	SD_CI_NEWS_LINEAGE_NOT_STABLE,
	SD_CI_NEWS_CORRELATED_CANDIDATE_ONLY,
};

struct sd_ci_news_join_observation {
	const struct stage_director_viewer_session *session;
	const struct stage_director_weather_host_observation *host;
	const struct stage_director_viewport_frame_observation *frame;
	const struct stage_director_process_launch_root *launch_root;
	int64_t observed_wayland_client_pid;
	uint64_t observed_wayland_client_start;
	/* NO session authorizer; callback is read-only procfs evidence. */
	stage_director_process_reader read_process;
	void *read_process_context;
};

struct sd_ci_news_join_result {
	enum sd_ci_news_join_state state;
	bool candidate_correlated;
	bool owner_verified;
	bool can_launch;
	bool can_bind_viewer;
	bool can_render;
	bool can_accept_input;
	bool can_show_popup;
};

struct sd_ci_news_join_result sd_ci_news_correlate(
		const struct sd_ci_news_join_observation *observation);

#endif
