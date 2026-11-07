#ifndef _SWAY_STAGE_DIRECTOR_H
#define _SWAY_STAGE_DIRECTOR_H
#include <stdbool.h>
#include <stdint.h>

struct sway_container;
struct sway_layer_surface;
struct sway_view;

/*
 * Stage Director SD0
 * ------------------
 * Generic shadow-only scene observation.
 *
 * Profiles may identify actors and group them into named scenes. The core may
 * observe identity and compositor geometry, but SD0 MUST NOT mutate container
 * geometry, focus, layout, or lifecycle.
 */
void stage_director_view_mapped(struct sway_view *view);
void stage_director_view_unmapped(struct sway_view *view);
void stage_director_view_identity_changed(struct sway_view *view);
void stage_director_observe_container(struct sway_container *container,
		const char *reason);

/*
 * Layer-shell observation is deliberately separate from sway_view actors.
 * SD0 records namespace/layer/output/actual geometry only and never mutates
 * layer-shell state.
 */
void stage_director_layer_mapped(struct sway_layer_surface *surface);
void stage_director_layer_committed(struct sway_layer_surface *surface);
void stage_director_layer_unmapped(struct sway_layer_surface *surface);
void stage_director_layer_destroyed(struct sway_layer_surface *surface);

/*
 * Pairing gesture's internal compositor callback, never callable from IPC.
 * Exact output/client/surface verification happens on the scene-hit layer.
 * Successful selection records only a revocable native candidate: not a
 * WATCH admission, browser permission, input, popup or rendering authority.
 */
struct sway_seat;
void stage_director_news_pairing_clear_host(void);
bool stage_director_news_pairing_select_at(
	struct sway_seat *seat, double lx, double ly);

/* M24: independent opt-in physical Company host selection. Publishes only
 * private geometry/intent candidate wire and NEVER assigns XDG actor/seat. */
void stage_director_company_pairing_clear_host(void);
bool stage_director_company_pairing_select_at(
	struct sway_seat *seat, double lx, double ly);
void stage_director_company_pairing_host_changed(
	struct sway_layer_surface *host);
void stage_director_company_pairing_host_lost(
	struct sway_layer_surface *host);

struct wl_client;
struct wl_resource;
/* Exact selected Wayland peer and fresh mapped-host validation, private to
 * compositor protocol implementation. They grant NO browser or render rights. */
bool stage_director_news_watch_selected_client(struct wl_client *client);
bool stage_director_news_watch_probe_request(struct wl_resource *resource);
/* Prevent stale protocol objects from replaying after a new pairing epoch. */
uint64_t stage_director_news_watch_current_epoch(struct wl_client *client);
/* WATCH v2: report a QML rectangle relative to the paired Weather host.
 * This is NOT a trusted inner wl_surface, and confers no render/launch/input
 * rights. Every request revalidates the same native peer and host geometry. */
enum stage_director_news_watch_viewport_status {
	SD_NEWS_WATCH_VIEWPORT_DENIED,
	SD_NEWS_WATCH_VIEWPORT_CANDIDATE_ONLY,
	SD_NEWS_WATCH_VIEWPORT_HIDDEN,
};
struct stage_director_news_watch_viewport_ack {
	enum stage_director_news_watch_viewport_status status;
	uint64_t host_revision;
};
struct stage_director_news_watch_viewport_ack
stage_director_news_watch_report_viewport(
	struct wl_resource *resource, int32_t x, int32_t y,
	int32_t width, int32_t height);
/* Exact protocol resource's destructor revokes its last candidate. */
void stage_director_news_watch_viewport_resource_destroyed(
	struct wl_resource *resource);

/* M30c: internal native callback when Company v2 kernel birth receipt is
 * accepted. Read-only scan of a bounded, compositor-owned mapped XDG list.
 * On selection revoke, clear all opaque view pointers and destroy listeners. */
void stage_director_company_m30c_birth_ready(void);
void stage_director_company_m30c_selection_revoked(void);

/* Production compositor-side WATCH candidate bookkeeping.
 * Called only after actual paired-client + request-epoch checks in native
 * protocol handler. First accepted WATCH reserves one native candidate.
 * No browser launch, scene, focus, input or popup authority is granted.
 */
bool stage_director_news_watch_record_candidate(
	struct wl_resource *request_resource, const char *source_id);


#endif
