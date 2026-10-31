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

struct wl_client;
struct wl_resource;
/* Exact selected Wayland peer and fresh mapped-host validation, private to
 * compositor protocol implementation. They grant NO browser or render rights. */
bool stage_director_news_watch_selected_client(struct wl_client *client);
bool stage_director_news_watch_probe_request(struct wl_resource *resource);
/* Prevent stale protocol objects from replaying after a new pairing epoch. */
uint64_t stage_director_news_watch_current_epoch(struct wl_client *client);

#endif
