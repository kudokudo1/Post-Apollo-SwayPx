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
void stage_director_ci_scene_probe(struct sway_view *view) {
	static bool measured = false;
	if (measured || !view || !view->surface || !view->content_tree ||
			!view->container || view->container->node.destroying) {
		return;
	}
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

	/* Always restore normal Sway view clipping BEFORE logging or returning.
	 * Input, focus, other windows, and the running browser remain untouched. */
	view_center_and_clip_surface(view);
	const bool inside_preserved = after_inside != NULL;
	const bool outside_rejected = after_outside == NULL;
	wlr_log(WLR_INFO,
		"SD_CI_SCENE_CLIP measured=1 before_inside=1 before_outside=1 "
		"after_inside=%d after_outside=%d "
		"normal_clip_restored=1 result=%s",
		inside_preserved ? 1 : 0, after_outside ? 1 : 0,
		inside_preserved && outside_rejected ? "PASS" : "FAIL");
}
