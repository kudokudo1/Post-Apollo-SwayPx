#include "ci_news_session_correlation.h"

/* Every result, including "correlated", is intentionally non-authorizing. */
static struct sd_ci_news_join_result result(enum sd_ci_news_join_state state) {
	return (struct sd_ci_news_join_result) {
		.state = state,
		.candidate_correlated = state == SD_CI_NEWS_CORRELATED_CANDIDATE_ONLY,
		.owner_verified = false,
		.can_launch = false,
		.can_bind_viewer = false,
		.can_render = false,
		.can_accept_input = false,
		.can_show_popup = false,
	};
}

static bool same_rect(struct stage_director_rect a,
		struct stage_director_rect b) {
	return a.x == b.x && a.y == b.y &&
		a.width == b.width && a.height == b.height;
}

struct sd_ci_news_join_result sd_ci_news_correlate(
		const struct sd_ci_news_join_observation *o) {
	if (!o || !o->session || !o->host || !o->frame ||
			!o->launch_root || !o->read_process ||
			o->session->state == SD_VIEWER_EMPTY ||
			o->session->epoch == 0 || o->session->last_epoch != o->session->epoch) {
		return result(SD_CI_NEWS_NO_SESSION);
	}

	/* A host is OBSERVED from an actual mapped native layer in any
	 * future adapter; a matching namespace alone remains untrusted. */
	const struct stage_director_weather_host_result host =
		stage_director_weather_host_probe(o->host);
	if (host.status != SD_WEATHER_HOST_CANDIDATE_ONLY ||
			!host.candidate_observed || host.owner_verified ||
			host.can_bind_viewer || host.can_clip) {
		return result(SD_CI_NEWS_UNVERIFIED_HOST_CANDIDATE);
	}
	const struct stage_director_viewer_session *s = o->session;
	const struct stage_director_viewport_frame_observation *f = o->frame;
	if (s->host_ref != o->host->layer_ref ||
			f->native_host_ref != o->host->layer_ref ||
			f->native_output_ref != o->host->output_ref ||
			f->epoch != s->epoch ||
			f->native_actor_ref == 0 ||
			f->native_actor_ref == f->native_host_ref ||
			f->native_output_ref == 0) {
		return result(SD_CI_NEWS_STALE_LINK);
	}
	if (s->state == SD_VIEWER_ACTOR_CANDIDATE &&
			(s->actor_ref != f->native_actor_ref ||
			 !same_rect(s->actor, f->geometry.actor))) {
		return result(SD_CI_NEWS_STALE_LINK);
	}
	if (!f->native_root_surface || !f->native_root_surface_ref ||
			f->native_root_surface_ref == f->native_host_ref ||
			f->native_root_surface_ref == f->native_actor_ref) {
		return result(SD_CI_NEWS_INVALID_ROOT_SURFACE);
	}
	if (!same_rect(s->host, o->host->host) ||
			!same_rect(s->output, o->host->output) ||
			!same_rect(s->host, f->geometry.host) ||
			!same_rect(s->output, f->geometry.output) ||
			!same_rect(s->viewport, f->geometry.viewport) ||
			!f->geometry.actor_identity_verified ||
			stage_director_viewport_plan(&f->geometry).result !=
				SD_VIEWPORT_GEOMETRY_ONLY) {
		return result(SD_CI_NEWS_BAD_GEOMETRY);
	}
	/* The viewer session reservation is pinned to the launch ROOT. The
	 * Wayland client may be its child (Chromium multiprocess). Do not call
	 * viewer_session_attach() with a fake equality of child and root PIDs. */
	if (o->launch_root->pid <= 0 ||
			o->launch_root->start_time_ticks == 0 ||
			o->launch_root->pid != s->expected_pid ||
			o->launch_root->start_time_ticks != s->expected_process_start) {
		return result(SD_CI_NEWS_ROOT_PROCESS_MISMATCH);
	}
	const struct stage_director_process_evidence_result lineage =
		stage_director_process_check(o->launch_root,
			o->observed_wayland_client_pid,
			o->observed_wayland_client_start,
			o->read_process, o->read_process_context);
	if (lineage.state != SD_PROCESS_EVIDENCE_LINEAGE_CANDIDATE ||
			!lineage.lineage_candidate || lineage.owner_verified ||
			lineage.can_bind_viewer || lineage.can_launch ||
			lineage.can_clip || lineage.can_accept_input) {
		return result(SD_CI_NEWS_LINEAGE_NOT_STABLE);
	}
	return result(SD_CI_NEWS_CORRELATED_CANDIDATE_ONLY);
}
