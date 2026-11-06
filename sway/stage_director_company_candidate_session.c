#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <wayland-server-core.h>
#include "sway/stage_director_company_candidate_session.h"

/* Candidate-only resource ledger: deliberately NO wl_global, scene or seat. */
static struct stage_director_company_session_receipt result(
		enum stage_director_company_session_status status) {
	return (struct stage_director_company_session_receipt) {
		.status = status,
	};
}

static bool same_rect(struct stage_director_rect a,
		struct stage_director_rect b) {
	return a.x == b.x && a.y == b.y &&
		a.width == b.width && a.height == b.height;
}

/* Native observations are produced by M20 inside the compositor. A public
 * application string, window title, namespace or PID cannot satisfy these
 * exact resource identities. Even satisfying them is not operator consent. */
static bool same_native_refs(
		const struct stage_director_company_candidate_session *s,
		const struct stage_director_company_native_host *h,
		const struct stage_director_company_native_actor *a) {
	return s && s->candidate && h && a && h->candidate && a->candidate &&
		h->state == SD_COMPANY_NATIVE_CANDIDATE_ONLY &&
		a->state == SD_COMPANY_NATIVE_CANDIDATE_ONLY &&
		h->native_layer_role_ref == (uintptr_t)s->host_role &&
		h->native_surface_ref == (uintptr_t)s->host_surface &&
		h->native_client_ref == (uintptr_t)s->host_client &&
		a->native_root_surface_ref == (uintptr_t)s->actor_root &&
		a->native_client_ref == (uintptr_t)s->actor_client &&
		h->native_layer_ref == s->host.native_layer_ref &&
		h->native_output_ref == s->host.native_output_ref &&
		a->native_view_ref == s->actor.native_view_ref &&
		h->pid == s->host.pid && h->uid == s->host.uid &&
		h->start_ticks == s->host.start_ticks &&
		a->pid == s->actor.pid && a->uid == s->actor.uid &&
		a->start_ticks == s->actor.start_ticks;
}

static bool acceptable_pair(struct wl_resource *role,
		struct wl_resource *surface, struct wl_resource *actor_root,
		const struct stage_director_company_native_host *host,
		const struct stage_director_company_native_actor *actor) {
	if (!host || !actor || !host->candidate || !actor->candidate ||
			host->state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
			actor->state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
			!host->native_layer_ref || !host->native_output_ref ||
			!actor->native_view_ref ||
			host->native_layer_role_ref != (uintptr_t)role ||
			host->native_surface_ref != (uintptr_t)surface ||
			actor->native_root_surface_ref != (uintptr_t)actor_root ||
			!host->pid || !actor->pid || host->pid == actor->pid ||
			host->uid != actor->uid || !host->uid ||
			!host->start_ticks || !actor->start_ticks ||
			!stage_director_rect_valid(host->host_global) ||
			!stage_director_rect_contains(
				host->output_global, host->host_global) ||
			!stage_director_rect_valid(actor->root_global)) {
		return false;
	}
	const struct stage_director_company_native_peer_result peer =
		stage_director_company_native_peer_probe(role, surface, actor_root);
	return peer.state == SD_COMPANY_PEER_DISTINCT_CANDIDATE_ONLY &&
		peer.distinct_native_clients_candidate &&
		host->native_client_ref ==
			(uintptr_t)wl_resource_get_client(role) &&
		actor->native_client_ref ==
			(uintptr_t)wl_resource_get_client(actor_root);
}

void stage_director_company_candidate_session_init(
		struct stage_director_company_candidate_session *s) {
	if (!s || s->initialized) return;
	*s = (struct stage_director_company_candidate_session){0};
	s->initialized = true;
	wl_list_init(&s->role_destroy.link);
	wl_list_init(&s->surface_destroy.link);
	wl_list_init(&s->actor_destroy.link);
}

void stage_director_company_candidate_session_close(
		struct stage_director_company_candidate_session *s) {
	if (!s || !s->initialized) return;
	if (s->listeners_active) {
		/* Unhook while resources are still tracked. The destroy callback
		 * may invoke close() synchronously during Wayland signal emit. */
		wl_list_remove(&s->role_destroy.link);
		wl_list_remove(&s->surface_destroy.link);
		wl_list_remove(&s->actor_destroy.link);
		wl_list_init(&s->role_destroy.link);
		wl_list_init(&s->surface_destroy.link);
		wl_list_init(&s->actor_destroy.link);
		s->listeners_active = false;
	}
	s->candidate = false;
	s->viewport_valid = false;
	s->fresh_observation_for_report = false;
	s->epoch = 0;
	s->revision = 0;
	s->host_role = NULL;
	s->host_surface = NULL;
	s->actor_root = NULL;
	s->host_client = NULL;
	s->actor_client = NULL;
	s->host = (struct stage_director_company_native_host){0};
	s->actor = (struct stage_director_company_native_actor){0};
	s->viewport_global = (struct stage_director_rect){0};
	s->actor_local_crop = (struct stage_director_rect){0};
}

static void on_role_destroyed(struct wl_listener *listener, void *data) {
	(void)data;
	struct stage_director_company_candidate_session *s =
		wl_container_of(listener, s, role_destroy);
	stage_director_company_candidate_session_close(s);
}
static void on_surface_destroyed(struct wl_listener *listener, void *data) {
	(void)data;
	struct stage_director_company_candidate_session *s =
		wl_container_of(listener, s, surface_destroy);
	stage_director_company_candidate_session_close(s);
}
static void on_actor_destroyed(struct wl_listener *listener, void *data) {
	(void)data;
	struct stage_director_company_candidate_session *s =
		wl_container_of(listener, s, actor_destroy);
	stage_director_company_candidate_session_close(s);
}

bool stage_director_company_candidate_session_record(
		struct stage_director_company_candidate_session *s,
		struct wl_resource *role, struct wl_resource *surface,
		struct wl_resource *actor_root,
		const struct stage_director_company_native_host *host,
		const struct stage_director_company_native_actor *actor) {
	if (!s || !s->initialized || s->candidate ||
			s->last_epoch == UINT64_MAX ||
			!acceptable_pair(role, surface, actor_root, host, actor)) {
		return false;
	}
	s->epoch = s->last_epoch + 1;
	s->last_epoch = s->epoch;
	s->revision = s->last_revision < UINT64_MAX ?
		s->last_revision + 1 : 0;
	if (!s->revision) {
		stage_director_company_candidate_session_close(s);
		return false;
	}
	s->last_revision = s->revision;
	s->candidate = true;
	s->host_role = role;
	s->host_surface = surface;
	s->actor_root = actor_root;
	s->host_client = wl_resource_get_client(role);
	s->actor_client = wl_resource_get_client(actor_root);
	s->host = *host;
	s->actor = *actor;
	s->viewport_valid = false;
	s->fresh_observation_for_report = false;

	s->role_destroy.notify = on_role_destroyed;
	s->surface_destroy.notify = on_surface_destroyed;
	s->actor_destroy.notify = on_actor_destroyed;
	wl_resource_add_destroy_listener(role, &s->role_destroy);
	wl_resource_add_destroy_listener(surface, &s->surface_destroy);
	wl_resource_add_destroy_listener(actor_root, &s->actor_destroy);
	s->listeners_active = true;
	/* This does NOT issue an M19 compositor_selected lease. */
	return true;
}

bool stage_director_company_candidate_session_refresh(
		struct stage_director_company_candidate_session *s,
		const struct stage_director_company_native_host *host,
		const struct stage_director_company_native_actor *actor) {
	if (!s || !s->candidate || !s->listeners_active) return false;
	if (!same_native_refs(s, host, actor) ||
			!acceptable_pair(s->host_role, s->host_surface,
				s->actor_root, host, actor)) {
		stage_director_company_candidate_session_close(s);
		return false;
	}
	const bool moved = !same_rect(s->host.host_global, host->host_global) ||
		!same_rect(s->host.output_global, host->output_global) ||
		!same_rect(s->actor.root_global, actor->root_global);
	if (moved) {
		if (s->last_revision == UINT64_MAX) {
			stage_director_company_candidate_session_close(s);
			return false;
		}
		s->revision = ++s->last_revision;
		s->viewport_valid = false;
		s->viewport_global = (struct stage_director_rect){0};
		s->actor_local_crop = (struct stage_director_rect){0};
	}
	s->host = *host;
	s->actor = *actor;
	/* One compositor-side native refresh permits at most ONE report. */
	s->fresh_observation_for_report = true;
	return true;
}

struct stage_director_company_session_receipt
stage_director_company_candidate_session_report(
		struct stage_director_company_candidate_session *s,
		struct wl_resource *reporter, uint64_t epoch, uint64_t revision,
		const struct stage_director_company_map_report *r) {
	if (!s || !s->candidate || !s->listeners_active ||
			!s->fresh_observation_for_report || !r || !reporter) {
		return result(SD_COMPANY_SESSION_NO_CANDIDATE);
	}
	s->fresh_observation_for_report = false;
	if (reporter == s->host_role || reporter == s->host_surface ||
			reporter == s->actor_root ||
			wl_resource_get_client(reporter) != s->host_client) {
		return result(SD_COMPANY_SESSION_FOREIGN_REPORTER);
	}
	/* An invalid/stale request from the paired reporting client must
	 * revoke its previous geometry. Never retain an older crop after a
	 * failed replacement, even if the host/actor themselves still exist. */
	s->viewport_valid = false;
	s->viewport_global = (struct stage_director_rect){0};
	s->actor_local_crop = (struct stage_director_rect){0};
	if (epoch != s->epoch || revision != s->revision ||
			r->epoch != epoch ||
			r->host_geometry_revision != revision) {
		return result(SD_COMPANY_SESSION_STALE_NATIVE);
	}
	if (!r->company_open || !r->coverage_active || !r->native_map_running ||
			(!r->map_in_frame.x && !r->map_in_frame.y &&
			 !r->map_in_frame.width && !r->map_in_frame.height)) {
		return (struct stage_director_company_session_receipt) {
			.status = SD_COMPANY_SESSION_HIDDEN,
			.epoch = epoch,
			.revision = revision,
		};
	}
	const struct stage_director_rect outer = {
		0, 0, s->host.host_global.width, s->host.host_global.height,
	};
	const struct stage_director_rect frame = {
		0, 0, r->frame_in_host.width, r->frame_in_host.height,
	};
	if (!stage_director_rect_contains(outer, r->frame_in_host) ||
			!stage_director_rect_contains(frame, r->map_in_frame)) {
		return result(SD_COMPANY_SESSION_INVALID_GEOMETRY);
	}
	const int64_t x =
		(int64_t)r->frame_in_host.x + r->map_in_frame.x;
	const int64_t y =
		(int64_t)r->frame_in_host.y + r->map_in_frame.y;
	if (x < INT_MIN || x > INT_MAX || y < INT_MIN || y > INT_MAX) {
		return result(SD_COMPANY_SESSION_INVALID_GEOMETRY);
	}
	struct stage_director_rect global = {0};
	if (!stage_director_viewport_from_host_local(s->host.host_global,
			(struct stage_director_rect) {
				(int)x, (int)y, r->map_in_frame.width,
				r->map_in_frame.height,
			}, &global) ||
			!stage_director_rect_contains(
				s->host.output_global, global)) {
		return result(SD_COMPANY_SESSION_INVALID_GEOMETRY);
	}
	const struct stage_director_viewport_plan plan =
		stage_director_viewport_plan(
			&(struct stage_director_viewport_request) {
				.host = s->host.host_global,
				.output = s->host.output_global,
				.actor = s->actor.root_global,
				.viewport = global,
				.host_mapped = true,
				.actor_mapped = true,
				.output_active = true,
				.actor_identity_verified = true,
			});
	if (plan.result != SD_VIEWPORT_GEOMETRY_ONLY ||
			!plan.geometry_valid || plan.can_launch ||
			plan.can_render || plan.can_accept_input) {
		return result(SD_COMPANY_SESSION_ACTOR_NOT_COVERING);
	}
	s->viewport_valid = true;
	s->viewport_global = global;
	s->actor_local_crop = plan.actor_local_crop;
	return (struct stage_director_company_session_receipt) {
		.status = SD_COMPANY_SESSION_GEOMETRY_CANDIDATE_ONLY,
		.epoch = epoch,
		.revision = revision,
		.viewport_global = global,
		.actor_local_crop = plan.actor_local_crop,
		.geometry_candidate = true,
		/* owner/launch/clip/seat/renderer authority remains FALSE */
	};
}

bool stage_director_company_candidate_session_receipt_current(
		const struct stage_director_company_candidate_session *s,
		const struct stage_director_company_session_receipt *r) {
	return s && r && s->candidate && s->viewport_valid &&
		r->status == SD_COMPANY_SESSION_GEOMETRY_CANDIDATE_ONLY &&
		r->geometry_candidate && !r->owner_verified &&
		!r->can_launch && !r->can_render && !r->can_clip &&
		!r->can_accept_input && !r->can_show_popup && !r->can_focus &&
		r->epoch == s->epoch && r->revision == s->revision &&
		same_rect(r->viewport_global, s->viewport_global) &&
		same_rect(r->actor_local_crop, s->actor_local_crop);
}
