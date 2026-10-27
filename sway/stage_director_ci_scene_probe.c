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

/*
 * Diagnostic: wlroots' subsurface clip narrows the SOURCE rectangle, while
 * scene buffer DESTINATION dimensions are independently specified.
 * The CI-only foot root buffer originally retained full-window destination
 * size, so a narrow source rectangle could still paint far beyond the crop.
 * Test one native root scene surface with an explicit 1:1 cropped destination.
 */
static int ci_dest_normalized_count;

static void ci_normalize_root_buffer(struct wlr_scene_buffer *buffer,
		int sx, int sy, void *data) {
	(void)sx;
	(void)sy;
	(void)data;
	if (!wlr_scene_surface_try_from_buffer(buffer)) return;
	const int previous_width = buffer->dst_width;
	const int previous_height = buffer->dst_height;
	wlr_scene_buffer_set_dest_size(buffer,
		ci_render_clip.width, ci_render_clip.height);
	/* CI-only evidence: source crop, destination size and native buffer
	 * are distinct renderer inputs. This does not assert visible clipping. */
	wlr_log(WLR_INFO,
		"SD_CI_BUFFER_METRICS at=%d,%d before_dst=%dx%d after_dst=%dx%d "
		"src=%.1f,%.1f %.1fx%.1f buffer=%dx%d",
		sx, sy, previous_width, previous_height,
		buffer->dst_width, buffer->dst_height,
		buffer->src_box.x, buffer->src_box.y,
		buffer->src_box.width, buffer->src_box.height,
		buffer->buffer_width, buffer->buffer_height);
	ci_dest_normalized_count++;
}

static void ci_restore_root_buffer(struct wlr_scene_buffer *buffer,
		int sx, int sy, void *data) {
	(void)sx;
	(void)sy;
	(void)data;
	if (!wlr_scene_surface_try_from_buffer(buffer)) return;
	/* The fixed CI foot actor had no previous destination-size override. */
	wlr_scene_buffer_set_dest_size(buffer, 0, 0);
}

void stage_director_ci_scene_probe_restore(struct sway_view *view) {
	if (!view || ci_render_actor != view) return;
	ci_render_actor = NULL;
	if (view->surface && view->content_tree && view->container &&
			!view->container->node.destroying) {
		view_center_and_clip_surface(view);
		wlr_scene_node_for_each_buffer(&view->content_tree->node,
			ci_restore_root_buffer, NULL);
		wlr_log(WLR_INFO, "SD_CI_RENDER_PIXEL clip_restored_on_unmap=1");
	}
}

/*
 * Sway transaction_progress calls arrange_root() AFTER applying container
 * state. arrange_root() writes the ordinary wlroots clip, overwriting an
 * earlier CI probe. Reapply only in this test-only, last-after-arrange hook.
 * Never expose this as authenticated session or production rendering power.
 */
void stage_director_ci_scene_probe_post_layout(void) {
	struct sway_view *view = ci_render_actor;
	if (!view || !view->surface || !view->content_tree || !view->container ||
			view->container->node.destroying) {
		return;
	}
	if (view->geometry.x != ci_render_geometry.x ||
			view->geometry.y != ci_render_geometry.y ||
			view->geometry.width != ci_render_geometry.width ||
			view->geometry.height != ci_render_geometry.height) {
		stage_director_ci_scene_probe_restore(view);
		wlr_log(WLR_ERROR, "SD_CI_POST_LAYOUT stale_geometry=1 result=FAIL");
		return;
	}
	wlr_scene_subsurface_tree_set_clip(
		&view->content_tree->node, &ci_render_clip);
	ci_dest_normalized_count = 0;
	const char *normalize = getenv("STAGE_DIRECTOR_CI_RENDER_NORMALIZE");
	if (normalize && strcmp(normalize, "1") == 0) {
		wlr_scene_node_for_each_buffer(&view->content_tree->node,
			ci_normalize_root_buffer, NULL);
		wlr_log(WLR_INFO, "SD_CI_BUFFER_DEST normalized=%d",
			ci_dest_normalized_count);
	}
	int x = 0, y = 0;
	wlr_scene_node_coords(&view->content_tree->node, &x, &y);
	double sx = 0, sy = 0;
	struct wlr_scene_node *inside = wlr_scene_node_at(
		&view->content_tree->node,
		(double)x + view->geometry.x + 160,
		(double)y + view->geometry.y + 160, &sx, &sy);
	struct wlr_scene_node *outside = wlr_scene_node_at(
		&view->content_tree->node,
		(double)x + view->geometry.x + 24,
		(double)y + view->geometry.y + 24, &sx, &sy);
	/* The old probe only exercised top-left; the real screenshot leaks to
	 * the right and below. Measure those edges in the SAME post-layout state. */
	struct wlr_scene_node *outside_right = wlr_scene_node_at(
		&view->content_tree->node,
		(double)x + view->geometry.x + 450,
		(double)y + view->geometry.y + 150, &sx, &sy);
	struct wlr_scene_node *outside_bottom = wlr_scene_node_at(
		&view->content_tree->node,
		(double)x + view->geometry.x + 160,
		(double)y + view->geometry.y + 370, &sx, &sy);
	wlr_log(WLR_INFO,
		"SD_CI_POST_LAYOUT clip_reapplied=1 origin=%d,%d "
		"inside=%d outside=%d result=%s outside_right=%d outside_bottom=%d",
		x, y, inside ? 1 : 0, outside ? 1 : 0,
		inside && !outside && !outside_right && !outside_bottom
			? "PASS" : "FAIL",
		outside_right ? 1 : 0, outside_bottom ? 1 : 0);
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
