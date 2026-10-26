#include <stdlib.h>
#include <string.h>
#include <scenefx/types/fx/blur_data.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>
#include "sway/tree/container.h"
#include "sway/tree/view.h"
#include "sway/stage_director_ci_scene_probe.h"

/*
 * This translation unit exists only in the -Dsd-ci-news-scene-probe=true
 * CI build. It never authenticates operator intent or enables NEWS playback.
 * No browser, host layer, IPC permission or persistent scene mutation.
 */
/*
 * Pixel screenshot mode exists ONLY in the opt-in CI compiler variant and
 * only under GITHUB_ACTIONS=true + exact disposable test identity.
 * It holds one native clip until the test actor unmaps, when it restores the
 * standard Sway clipping. No process, seat, popup, or WATCH authority.
 */
static struct sway_view *ci_render_actor;
static struct wlr_box ci_render_clip;
static struct wlr_box ci_render_geometry;

void stage_director_ci_scene_probe_restore(struct sway_view *view) {
	if (!view || ci_render_actor != view) return;
	ci_render_actor = NULL;
	if (view->surface && view->content_tree && view->container &&
			!view->container->node.destroying) {
		view_center_and_clip_surface(view);
		wlr_log(WLR_INFO, "SD_CI_RENDER_PIXEL clip_restored_on_unmap=1");
	}
}

void stage_director_ci_scene_probe(struct sway_view *view) {
	static bool measured = false;
	if (!view || !view->surface || !view->content_tree ||
			!view->container || view->container->node.destroying) {
		return;
	}
	const char *pixel = getenv("STAGE_DIRECTOR_CI_RENDER_PROBE");
	const bool pixel_mode = pixel && strcmp(pixel, "1") == 0;
	if (ci_render_actor == view) {
		if (view->geometry.x != ci_render_geometry.x ||
				view->geometry.y != ci_render_geometry.y ||
				view->geometry.width != ci_render_geometry.width ||
				view->geometry.height != ci_render_geometry.height) {
			stage_director_ci_scene_probe_restore(view);
			wlr_log(WLR_ERROR, "SD_CI_RENDER_PIXEL stale_geometry=1 result=FAIL");
		} else {
			/* Ordinary XDG commits/transactions can reset the clip. */
			wlr_scene_subsurface_tree_set_clip(
				&view->content_tree->node, &ci_render_clip);
		}
		return;
	}
	if (measured) return;
	const char *ci = getenv("GITHUB_ACTIONS");
	const char *enabled = getenv("STAGE_DIRECTOR_CI_SCENE_PROBE");
	const char *title = view_get_title(view);
	const char *app_id = view_get_app_id(view);
	if (!ci || strcmp(ci, "true") != 0 ||
			!enabled || strcmp(enabled, "1") != 0 ||
			!title || strcmp(title, "SD1-CI-DISPOSABLE-NOT-BROWSER") != 0 ||
			!app_id || strcmp(app_id, "post-apollo-weather-news") != 0 ||
			view->geometry.width < 600 || view->geometry.height < 440 ||
			wl_list_empty(&view->content_tree->children)) {
		return;
	}

	int x = 0;
	int y = 0;
	wlr_scene_node_coords(&view->content_tree->node, &x, &y);
	const double outside_x = (double)x + view->geometry.x + 24;
	const double outside_y = (double)y + view->geometry.y + 24;
	const double inside_x = (double)x + view->geometry.x + 160;
	const double inside_y = (double)y + view->geometry.y + 160;
	const struct wlr_box local_clip = {
		.x = view->geometry.x + 96,
		.y = view->geometry.y + 96,
		.width = 320,
		.height = 240,
	};
	double sx = 0, sy = 0;
	struct wlr_scene_node *before_inside = wlr_scene_node_at(
		&view->content_tree->node, inside_x, inside_y, &sx, &sy);
	struct wlr_scene_node *before_outside = wlr_scene_node_at(
		&view->content_tree->node, outside_x, outside_y, &sx, &sy);
	/* If the first transaction ran before a client buffer existed, defer. */
	if (!before_inside || !before_outside) {
		return;
	}
	measured = true;
	wlr_scene_subsurface_tree_set_clip(&view->content_tree->node, &local_clip);
	struct wlr_scene_node *after_inside = wlr_scene_node_at(
		&view->content_tree->node, inside_x, inside_y, &sx, &sy);
	struct wlr_scene_node *after_outside = wlr_scene_node_at(
		&view->content_tree->node, outside_x, outside_y, &sx, &sy);

	const bool inside_preserved = after_inside != NULL;
	const bool outside_rejected = after_outside == NULL;
	const bool held = pixel_mode && inside_preserved && outside_rejected;
	if (held) {
		ci_render_actor = view;
		ci_render_clip = local_clip;
		ci_render_geometry = view->geometry;
	} else {
		/* Standard hit-test mode always restores in this call. */
		view_center_and_clip_surface(view);
	}
	wlr_log(WLR_INFO,
		"SD_CI_SCENE_CLIP measured=1 before_inside=1 before_outside=1 "
		"after_inside=%d after_outside=%d "
		"normal_clip_restored=%d result=%s",
		inside_preserved ? 1 : 0, after_outside ? 1 : 0,
		held ? 0 : 1,
		inside_preserved && outside_rejected ? "PASS" : "FAIL");
	if (held) {
		wlr_log(WLR_INFO,
			"SD_CI_RENDER_PIXEL inside=%d,%d outside=%d,%d "
			"clip_held=1",
			(int)inside_x, (int)inside_y, (int)outside_x, (int)outside_y);
	}
}
