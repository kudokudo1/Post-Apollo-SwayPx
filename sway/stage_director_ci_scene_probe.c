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
		buffer->buffer ? buffer->buffer->width : 0,
		buffer->buffer ? buffer->buffer->height : 0);
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

/*
 * Prevent Sway's output_repaint_timer_handler from undoing the held clip's
 * destination AFTER post_layout() has already normalized the native buffer.
 * This is linked only with the opt-in CI Meson target and exact disposable
 * actor. The normal package builds output_configure_scene unchanged.
 */
struct ci_dest_candidate {
	struct wlr_scene_buffer *candidate;
	bool found;
};

static void ci_match_render_dest(struct wlr_scene_buffer *buffer,
		int sx, int sy, void *data) {
	(void)sx;
	(void)sy;
	struct ci_dest_candidate *candidate = data;
	if (buffer == candidate->candidate) {
		candidate->found = true;
	}
}

bool stage_director_ci_scene_probe_preserve_dest(struct wlr_scene_buffer *buffer) {
	if (!buffer || !ci_render_actor || !ci_render_actor->content_tree ||
			!ci_render_actor->surface || !ci_render_actor->container ||
			ci_render_actor->container->node.destroying) {
		return false;
	}
	const char *ci = getenv("GITHUB_ACTIONS");
	const char *enabled = getenv("STAGE_DIRECTOR_CI_FRAME_DEST_OVERRIDE");
	if (!ci || strcmp(ci, "true") != 0 ||
			!enabled || strcmp(enabled, "1") != 0 ||
			!wlr_scene_surface_try_from_buffer(buffer)) {
		return false;
	}
	const struct ci_dest_candidate initial = { .candidate = buffer };
	struct ci_dest_candidate probe = initial;
	wlr_scene_node_for_each_buffer(&ci_render_actor->content_tree->node,
		ci_match_render_dest, &probe);
	if (!probe.found || ci_render_clip.width != 320 ||
			ci_render_clip.height != 240) {
		return false;
	}
	wlr_scene_buffer_set_dest_size(buffer,
		ci_render_clip.width, ci_render_clip.height);
	static int reports;
	if (reports++ < 4) {
		wlr_log(WLR_INFO, "SD_CI_FRAME_DEST_PRESERVED dst=%dx%d "
			"source=CI_only_after_output_configure_scene",
			buffer->dst_width, buffer->dst_height);
	}
	return true;
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
/*
 * Read-only scene provenance: distinguish a second paint-bearing buffer in
 * the surrounding Sway container from the single cropped NEWS content buffer.
 * This callback never changes geometry, allocation, focus or visibility.
 */
struct ci_paint_audit {
	const char *scope;
	int count;
	int surfaces;
};

static void ci_audit_paint_buffer(struct wlr_scene_buffer *buffer,
		int sx, int sy, void *data) {
	struct ci_paint_audit *audit = data;
	bool is_surface = wlr_scene_surface_try_from_buffer(buffer) != NULL;
	audit->count++;
	audit->surfaces += is_surface ? 1 : 0;
	wlr_log(WLR_INFO,
		"SD_CI_PAINT_BUFFER scope=%s index=%d at=%d,%d "
		"surface=%d src=%.1f,%.1f %.1fx%.1f dst=%dx%d native=%dx%d",
		audit->scope, audit->count, sx, sy, is_surface ? 1 : 0,
		buffer->src_box.x, buffer->src_box.y,
		buffer->src_box.width, buffer->src_box.height,
		buffer->dst_width, buffer->dst_height,
		buffer->buffer ? buffer->buffer->width : 0,
		buffer->buffer ? buffer->buffer->height : 0);
}

/*
 * Inspect every draw-capable node near the actor. The previous buffer-only
 * audit included a large NULL-buffer output handler, which is not by itself
 * an explanation for white pixels. This traversal is read-only, bounded,
 * and compiled only in the distinct CI probe compositor.
 */
struct ci_draw_audit {
	const struct wlr_scene_node *output_handler;
	int nodes;
	int trees;
	int buffers;
	int drawable_buffers;
	int rects;
	int visible_rects;
	int effects;
};

static void ci_draw_node(struct wlr_scene_node *node, int depth,
		bool ancestor_enabled, struct ci_draw_audit *audit) {
	if (!node || audit->nodes >= 96 || depth > 16) return;
	audit->nodes++;
	bool active = ancestor_enabled && node->enabled;
	int x = 0, y = 0;
	wlr_scene_node_coords(node, &x, &y);

	switch (node->type) {
	case WLR_SCENE_NODE_TREE:;
		audit->trees++;
		struct wlr_scene_tree *tree = wlr_scene_tree_from_node(node);
		struct wlr_scene_node *child;
		wl_list_for_each(child, &tree->children, link) {
			if (audit->nodes >= 96) break;
			ci_draw_node(child, depth + 1, active, audit);
		}
		break;
	case WLR_SCENE_NODE_RECT:;
		audit->rects++;
		struct wlr_scene_rect *rect = wlr_scene_rect_from_node(node);
		if (active && rect->color[3] > 0.01f) audit->visible_rects++;
		wlr_log(WLR_INFO, "SD_CI_DRAW_NODE kind=rect active=%d depth=%d "
			"at=%d,%d size=%dx%d rgba=%.3f,%.3f,%.3f,%.3f",
			active ? 1 : 0, depth, x, y, rect->width, rect->height,
			rect->color[0], rect->color[1], rect->color[2], rect->color[3]);
		break;
	case WLR_SCENE_NODE_BUFFER:;
		audit->buffers++;
		struct wlr_scene_buffer *buffer = wlr_scene_buffer_from_node(node);
		if (active && buffer->buffer && buffer->opacity > 0.01f) {
			audit->drawable_buffers++;
		}
		wlr_log(WLR_INFO, "SD_CI_DRAW_NODE kind=buffer active=%d depth=%d "
			"at=%d,%d dst=%dx%d native=%dx%d opaque=%d output_handler=%d",
			active ? 1 : 0, depth, x, y,
			buffer->dst_width, buffer->dst_height,
			buffer->buffer ? buffer->buffer->width : 0,
			buffer->buffer ? buffer->buffer->height : 0,
			buffer->opacity > 0.99f ? 1 : 0,
			node == audit->output_handler ? 1 : 0);
		break;
	default:
		/* The compilation API exposes TREE/RECT/BUFFER here; future
		 * SceneFX-private effects remain distinct unknown node types.
		 * Do not guess private struct fields or their draw behavior. */
		audit->effects++;
		wlr_log(WLR_INFO,
			"SD_CI_DRAW_NODE kind=other type=%d active=%d at=%d,%d",
			(int)node->type, active ? 1 : 0, x, y);
		break;
	}
}

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
	struct ci_paint_audit content = { .scope = "content" };
	struct ci_paint_audit container = { .scope = "container" };
	wlr_scene_node_for_each_buffer(&view->content_tree->node,
		ci_audit_paint_buffer, &content);
	if (view->container->scene_tree) {
		wlr_scene_node_for_each_buffer(&view->container->scene_tree->node,
			ci_audit_paint_buffer, &container);
	}
	wlr_log(WLR_INFO, "SD_CI_PAINT_AUDIT content=%d content_surfaces=%d "
		"container=%d container_surfaces=%d",
		content.count, content.surfaces, container.count, container.surfaces);
	struct ci_draw_audit draw = {
		.output_handler = &view->output_handler->node,
	};
	ci_draw_node(&view->container->scene_tree->node, 0, true, &draw);
	wlr_log(WLR_INFO, "SD_CI_DRAW_AUDIT nodes=%d trees=%d buffers=%d "
		"drawable_buffers=%d rects=%d visible_rects=%d effects=%d",
		draw.nodes, draw.trees, draw.buffers, draw.drawable_buffers,
		draw.rects, draw.visible_rects, draw.effects);
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
