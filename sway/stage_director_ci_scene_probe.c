#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "sway/server.h"
#include <string.h>
#include <sys/types.h>
#include <wayland-server-core.h>
#include "sway/stage_director_process_evidence.h"
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
/* The timer and request file are available exclusively in the independently
 * compiled CI scene-probe binary, not the installed production compositor. */
static struct wl_event_source *ci_mask_request_timer;
static bool ci_mask_request_applied;
static bool ci_mask_third_applied;
static bool ci_mask_pending_observed;


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
	if (!probe.found ||
			!((ci_render_clip.width == 320 && ci_render_clip.height == 240) ||
				(ci_mask_request_applied && ci_render_clip.width == 400 &&
				 ci_render_clip.height == 300) ||
				(ci_mask_third_applied && ci_render_clip.width == 280 &&
				 ci_render_clip.height == 220))) {
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
	if (ci_mask_request_timer) {
		wl_event_source_remove(ci_mask_request_timer);
		ci_mask_request_timer = NULL;
	}
	if (ci_mask_pending_observed) {
		wlr_log(WLR_INFO, "SD_CI_CHROME_MASK_PENDING_REVOKED "
			"actor_unmapped=1 timer_removed=1 authority=NONE result=PASS");
	}
	ci_mask_request_applied = false;
	ci_mask_third_applied = false;
	ci_mask_pending_observed = false;
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


/*
 * SD1 // Real Chrome pixel experiment, compiled ONLY in CI probe variant.
 *
 * IMPORTANT: the production SwayPX build never links this translation unit.
 * This is a disposable test selector, not WATCH authority or an actor broker.
 * The compositor independently resolves wl_client credentials and the
 * systemd-owned exact cgroup path, rather than trusting app_id/window title.
 */
static bool ci_digits(const char *value) {
	if (!value || !*value) return false;
	for (const unsigned char *p = (const unsigned char *)value; *p; p++) {
		if (!isdigit(*p)) return false;
	}
	return true;
}

static bool ci_real_chrome_process(struct sway_view *view, pid_t *peer_out) {
	const char *github = getenv("GITHUB_ACTIONS");
	const char *ci = getenv("CI");
	const char *flag = getenv("STAGE_DIRECTOR_CI_REAL_CHROME_PIXEL");
	const char *run = getenv("GITHUB_RUN_ID");
	const char *attempt = getenv("GITHUB_RUN_ATTEMPT");
	const char *runner_temp = getenv("RUNNER_TEMP");
	if (!github || strcmp(github, "true") != 0 ||
			!ci || strcmp(ci, "true") != 0 ||
			!flag || strcmp(flag, "1") != 0 ||
			!runner_temp || !*runner_temp ||
			!ci_digits(run) || strlen(run) > 22 ||
			(attempt && (!ci_digits(attempt) || strlen(attempt) > 8)) ||
			!view || view->type != SWAY_VIEW_XDG_SHELL ||
			!view->surface || !view->surface->resource ||
			!view->container || view->container->node.destroying ||
			!view->content_tree || !view->wlr_xdg_toplevel) {
		return false;
	}
	struct wl_client *client =
		wl_resource_get_client(view->surface->resource);
	pid_t peer = -1;
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;
	if (!client) return false;
	wl_client_get_credentials(client, &peer, &uid, &gid);
	struct stage_director_process_identity observed = {0};
	if (peer <= 1 || uid != getuid() || view->pid != peer ||
			!stage_director_read_linux_process((int64_t)peer,
				&observed, NULL) ||
			observed.pid != peer || observed.uid != (uint32_t)uid ||
			observed.start_time_ticks == 0) {
		return false;
	}
	/* The cgroup is created by the *separate* CI systemd service.
	 * Match an exact final path component, no prefix/substring spoof.
	 */
	char expected[100];
	const int size = snprintf(expected, sizeof(expected),
		"/sd-ci-chrome-wayland-%s-%s.service",
		run, attempt ? attempt : "1");
	if (size <= 0 || size >= (int)sizeof(expected)) return false;
	char file[64];
	if (snprintf(file, sizeof(file), "/proc/%ld/cgroup",
			(long)peer) <= 0) return false;
	FILE *fp = fopen(file, "r");
	if (!fp) return false;
	char line[512];
	bool match = false;
	while (fgets(line, sizeof(line), fp)) {
		size_t n = strcspn(line, "\r\n");
		line[n] = '\0';
		const size_t suffix = strlen(expected);
		if (strncmp(line, "0::/", 4) == 0 && n >= suffix &&
				strcmp(line + n - suffix, expected) == 0) {
			match = true;
			break;
		}
	}
	fclose(fp);
	if (match && peer_out) *peer_out = peer;
	return match;
}

/* Any actual XDG popup is placed under root->layers.popup by Sway's
 * xdg_shell.c, not under the CI-clipped browser content. In the isolated
 * Chrome pixel experiment NO popup has an authenticated paint/seat owner.
 * Refuse creation before the independent global scene tree is allocated.
 * This intentionally does not attempt to transform popup coordinates.
 *
 * The exact native Chrome wl_client, Linux PID/starttime and service cgroup
 * are re-observed for every call. Unrelated applications are unchanged.
 */
bool stage_director_ci_scene_probe_reject_chrome_popup(
		struct sway_view *view) {
	if (!view) return false;
	/* A popup can be requested BEFORE the screenshot selector chooses
	 * the main Chrome window. Exact native wl_client/cgroup membership is
	 * sufficient to deny it. Once selected, losing credentials on SIGKILL
	 * must also never grant a global popup scene escape. */
	pid_t peer = -1;
	const bool current = ci_real_chrome_process(view, &peer);
	const bool selected = view == ci_render_actor;
	if (!selected && !current) return false;
	wlr_log(WLR_INFO, "SD_CI_CHROME_POPUP_DENIED "
		"selected_native_actor=%d current_cgroup=%d pid=%ld "
		"global_popup_tree=BLOCKED authority=NONE result=PASS",
		selected ? 1 : 0, current ? 1 : 0, (long)peer);
	return true;
}

/* Applied once at XDG view_map, before the first container arrange.
 * Verify the source via kernel wl_client credentials and the exact private
 * CI service cgroup; never by app_id/title, request source ID, or IPC.
 * No global border config changes, and no other native view is modified.
 */
bool stage_director_ci_scene_probe_borderless_verified_actor(
		struct sway_view *view) {
	if (!view || !view->container || !view->surface) return false;
	pid_t native_peer = -1;
	if (!ci_real_chrome_process(view, &native_peer)) return false;
	view->container->pending.border = B_NONE;
	view->container->pending.border_thickness = 0;
	wlr_log(WLR_INFO,
		"SD_CI_CHROME_PER_VIEW_BORDER native_cgroup=1 "
		"exact_native_peer=1 actor_only=1 border=none "
		"unrelated_window_policy=UNCHANGED authority=NONE pid=%ld",
		(long)native_peer);
	return true;
}


/*
 * CI-only "host mask" transaction. A disposable runner agent creates a
 * 0600, exact-name PHASE2 then PHASE3 requests, each after a real pixel
 * screenshot. The compositor polls on its own Wayland thread, rechecks the
 * exact browser and refuses replays. It NEVER moves/resizes the
 * Chrome XDG container or takes keyboard/pointer/focus authority.
 */
static int ci_browser_mask_request(void *data) {
	struct sway_view *view = data;
	if (!view || view != ci_render_actor || !view->surface ||
			!view->content_tree || !view->container ||
			view->container->node.destroying) {
		ci_mask_request_timer = NULL;
		return 0;
	}
	const char *runner = getenv("RUNNER_TEMP");
	const char *run = getenv("GITHUB_RUN_ID");
	const char *attempt = getenv("GITHUB_RUN_ATTEMPT");
	const char *enabled = getenv("STAGE_DIRECTOR_CI_BROWSER_MASK");
	pid_t peer = -1;
	if (!enabled || strcmp(enabled, "1") != 0 ||
			!runner || strcmp(runner, "/home/runner/work/_temp") != 0 ||
			!ci_digits(run) || strlen(run) > 22 ||
			(attempt && (!ci_digits(attempt) || strlen(attempt) > 8)) ||
			!ci_real_chrome_process(view, &peer) ||
			view->geometry.x != ci_render_geometry.x ||
			view->geometry.y != ci_render_geometry.y ||
			view->geometry.width != ci_render_geometry.width ||
			view->geometry.height != ci_render_geometry.height) {
		/* Fail closed: missing source ownership or unexpected window motion.
		 * Restore the ordinary scene rather than retaining stale custody. */
		stage_director_ci_scene_probe_restore(view);
		wlr_log(WLR_ERROR, "SD_CI_CHROME_MASK_SOURCE_LOST result=FAIL");
		return 0;
	}
	char request[320];
	int length = snprintf(request, sizeof(request),
		"%s/sd-chrome-mask-%s-%s.request",
		runner, run, attempt ? attempt : "1");
	if (length <= 0 || length >= (int)sizeof(request)) {
		stage_director_ci_scene_probe_restore(view);
		return 0;
	}
	int fd = open(request, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
	if (fd < 0) {
		if (ci_mask_request_timer) {
			wl_event_source_timer_update(ci_mask_request_timer, 100);
		}
		return 0;
	}
	struct stat st = {0};
	char bytes[16] = {0};
	ssize_t n = read(fd, bytes, sizeof(bytes));
	const bool safe = fstat(fd, &st) == 0 && S_ISREG(st.st_mode) &&
		st.st_uid == getuid() && st.st_nlink == 1 &&
		(st.st_mode & 077) == 0;
	close(fd);
	if (!safe) {
		stage_director_ci_scene_probe_restore(view);
		wlr_log(WLR_ERROR, "SD_CI_CHROME_MASK_REQUEST_INVALID result=FAIL");
		return 0;
	}
	/* Explicit CI-only held request. The compositor acknowledges that a
	 * request has reached its own event loop but does NOT change its clip.
	 * A forced native actor unmap must remove this timer and revoke the
	 * held request. This is NOT an interrupted in-progress scene mutation. */
	const char *pending_mode = getenv("STAGE_DIRECTOR_CI_CHROME_PENDING_EXIT");
	if (pending_mode && strcmp(pending_mode, "1") == 0) {
		if (n != 8 || memcmp(bytes, "PENDING\n", 8) != 0 ||
				ci_mask_request_applied || ci_mask_third_applied ||
				ci_render_clip.width != 320 || ci_render_clip.height != 240) {
			stage_director_ci_scene_probe_restore(view);
			wlr_log(WLR_ERROR,
				"SD_CI_CHROME_MASK_PENDING_INVALID result=FAIL");
			return 0;
		}
		if (!ci_mask_pending_observed) {
			ci_mask_pending_observed = true;
			wlr_log(WLR_INFO, "SD_CI_CHROME_MASK_PENDING_HELD "
				"native_actor=1 cgroup=1 pid=%ld "
				"clip_unchanged=320x240 authority=NONE result=PASS",
				(long)peer);
		}
		if (ci_mask_request_timer) {
			wl_event_source_timer_update(ci_mask_request_timer, 100);
		}
		return 0;
	}
	if (n != 7) {
		stage_director_ci_scene_probe_restore(view);
		wlr_log(WLR_ERROR, "SD_CI_CHROME_MASK_REQUEST_INVALID result=FAIL");
		return 0;
	}
	/* The PHASE2 request is intentionally still on disk while the runner
	 * observes the compositor, then deletes it before creating PHASE3.
	 * Holding PHASE2 must never cause a spurious replay or restoration. */
	if (ci_mask_request_applied && !ci_mask_third_applied &&
			memcmp(bytes, "PHASE2\n", 7) == 0) {
		if (ci_mask_request_timer) {
			wl_event_source_timer_update(ci_mask_request_timer, 100);
		}
		return 0;
	}
	const bool phase2 = !ci_mask_request_applied &&
		!ci_mask_third_applied &&
		memcmp(bytes, "PHASE2\n", 7) == 0 &&
		ci_render_clip.width == 320 && ci_render_clip.height == 240;
	const bool phase3 = ci_mask_request_applied &&
		!ci_mask_third_applied &&
		memcmp(bytes, "PHASE3\n", 7) == 0 &&
		ci_render_clip.width == 400 && ci_render_clip.height == 300;
	if (!phase2 && !phase3) {
		stage_director_ci_scene_probe_restore(view);
		wlr_log(WLR_ERROR, "SD_CI_CHROME_MASK_REQUEST_INVALID result=FAIL");
		return 0;
	}
	/* Fixed, nonoverlapping test viewports, NEVER actual Weather geometry.
	 * Chrome's mapped XDG container, PID and cgroup remain stationary. */
	const struct wlr_box phase2_rect = {
		.x = view->geometry.x + 520,
		.y = view->geometry.y + 240,
		.width = 400,
		.height = 300,
	};
	const struct wlr_box phase3_rect = {
		.x = view->geometry.x + 1040,
		.y = view->geometry.y + 560,
		.width = 280,
		.height = 220,
	};
	const struct wlr_box next = phase2 ? phase2_rect : phase3_rect;
	if (view->geometry.width < 1400 || view->geometry.height < 820 ||
			next.x < view->geometry.x || next.y < view->geometry.y ||
			next.x + next.width > view->geometry.x + view->geometry.width ||
			next.y + next.height > view->geometry.y + view->geometry.height) {
		stage_director_ci_scene_probe_restore(view);
		wlr_log(WLR_ERROR, "SD_CI_CHROME_MASK_OUT_OF_BOUNDS result=FAIL");
		return 0;
	}
	ci_render_clip = next;
	if (phase2) {
		ci_mask_request_applied = true;
	} else {
		ci_mask_third_applied = true;
	}
	wlr_scene_subsurface_tree_set_clip(
		&view->content_tree->node, &ci_render_clip);
	wlr_scene_node_for_each_buffer(&view->content_tree->node,
		ci_normalize_root_buffer, NULL);
	if (phase2) {
		wlr_log(WLR_INFO, "SD_CI_CHROME_MASK_REBIND "
			"same_native_actor=1 cgroup=1 pid=%ld "
			"source_unchanged=1 window_geometry_unchanged=1 "
			"clip=520,240 400x300 authority=NONE result=PASS",
			(long)peer);
		/* Keep one timer while the runner captures phase2 screenshot. */
		if (ci_mask_request_timer) {
			wl_event_source_timer_update(ci_mask_request_timer, 100);
		}
	} else {
		wlr_log(WLR_INFO, "SD_CI_CHROME_MASK_REBIND_PHASE3 "
			"same_native_actor=1 cgroup=1 pid=%ld "
			"source_unchanged=1 window_geometry_unchanged=1 "
			"clip=1040,560 280x220 authority=NONE result=PASS",
			(long)peer);
		if (ci_mask_request_timer) {
			wl_event_source_remove(ci_mask_request_timer);
			ci_mask_request_timer = NULL;
		}
	}
	return 0;
}

static bool ci_real_chrome_pixel_probe(struct sway_view *view) {
	static bool selected;
	if (selected || ci_render_actor || !view || !view->content_tree ||
			wl_list_empty(&view->content_tree->children) ||
			view->geometry.width < 600 || view->geometry.height < 440) {
		return false;
	}
	pid_t peer = -1;
	if (!ci_real_chrome_process(view, &peer)) return false;
	int x = 0, y = 0;
	wlr_scene_node_coords(&view->content_tree->node, &x, &y);
	const struct wlr_box crop = {
		.x = view->geometry.x + 96,
		.y = view->geometry.y + 96,
		.width = 320,
		.height = 240,
	};
	double sx = 0, sy = 0;
	const struct wlr_scene_node *before_inside = wlr_scene_node_at(
		&view->content_tree->node,
		(double)x + view->geometry.x + 160,
		(double)y + view->geometry.y + 160, &sx, &sy);
	const struct wlr_scene_node *before_outside = wlr_scene_node_at(
		&view->content_tree->node,
		(double)x + view->geometry.x + 24,
		(double)y + view->geometry.y + 24, &sx, &sy);
	if (!before_inside || !before_outside) return false;

	wlr_scene_subsurface_tree_set_clip(&view->content_tree->node, &crop);
	const struct wlr_scene_node *after_inside = wlr_scene_node_at(
		&view->content_tree->node,
		(double)x + view->geometry.x + 160,
		(double)y + view->geometry.y + 160, &sx, &sy);
	const struct wlr_scene_node *after_outside = wlr_scene_node_at(
		&view->content_tree->node,
		(double)x + view->geometry.x + 24,
		(double)y + view->geometry.y + 24, &sx, &sy);
	if (!after_inside || after_outside) {
		view_center_and_clip_surface(view);
		wlr_log(WLR_ERROR, "SD_CI_CHROME_PIXEL_SCENE "
			"native=1 cgroup=1 hit_test_crop=FAIL authority=NONE");
		return false;
	}
	selected = true;
	ci_render_actor = view;
	ci_render_clip = crop;
	ci_render_geometry = view->geometry;
	const char *mask_mode = getenv("STAGE_DIRECTOR_CI_BROWSER_MASK");
	if (mask_mode && strcmp(mask_mode, "1") == 0) {
		ci_mask_request_applied = false;
		ci_mask_third_applied = false;
		ci_mask_pending_observed = false;
		if (!server.wl_event_loop || ci_mask_request_timer) {
			stage_director_ci_scene_probe_restore(view);
			return false;
		}
		ci_mask_request_timer = wl_event_loop_add_timer(
			server.wl_event_loop, ci_browser_mask_request, view);
		if (!ci_mask_request_timer ||
				wl_event_source_timer_update(ci_mask_request_timer, 100) < 0) {
			stage_director_ci_scene_probe_restore(view);
			return false;
		}
	}
	wlr_log(WLR_INFO, "SD_CI_CHROME_PIXEL_SCENE "
		"native=1 cgroup=1 pid=%ld starttime_checked=1 "
		"hit_test_crop=PASS viewport=320x240 "
		"render_pixels=UNVERIFIED authority=NONE",
		(long)peer);
	return true;
}

void stage_director_ci_scene_probe(struct sway_view *view) {
	/* Separate isolated CI Chrome candidate, verified by native process
	 * cgroup and kernel identity, never window title or app_id. */
	(void)ci_real_chrome_pixel_probe(view);
	static bool measured = false;
	static bool child_reported = false;
	if (!view || !view->surface || !view->content_tree ||
			!view->container || view->container->node.destroying) {
		return;
	}
	/*
	 * A SECOND, independent CI Wayland window is spawned as the child of a
	 * fixed bash launch root. It has no NEWS clip and no privileges. Observe
	 * its actual wl_client credentials once so the isolated lineage witness
	 * can check the real kernel process tree. This is never used as a scene
	 * selector, launch ticket, IPC credential or browser session authority.
	 */
	const char *ci_child = getenv("GITHUB_ACTIONS");
	const char *child_gate = getenv("STAGE_DIRECTOR_CI_CHILD_LINEAGE");
	const char *child_title = view_get_title(view);
	const char *child_app = view_get_app_id(view);
	if (!child_reported && ci_child && strcmp(ci_child, "true") == 0 &&
			child_gate && strcmp(child_gate, "1") == 0 &&
			child_title && strcmp(child_title, "SD1-CI-LINEAGE-CHILD-NOT-BROWSER") == 0 &&
			child_app && strcmp(child_app, "post-apollo-weather-news-child") == 0 &&
			view->type == SWAY_VIEW_XDG_SHELL && view->surface->resource) {
		child_reported = true;
		struct wl_client *client = wl_resource_get_client(view->surface->resource);
		pid_t peer_pid = -1;
		uid_t peer_uid = (uid_t)-1;
		gid_t peer_gid = (gid_t)-1;
		if (client) {
			wl_client_get_credentials(client, &peer_pid, &peer_uid, &peer_gid);
		}
		struct stage_director_process_identity observed = {0};
		const bool peer_ok = peer_pid > 0 &&
			stage_director_read_linux_process((int64_t)peer_pid, &observed, NULL) &&
			observed.pid == peer_pid && observed.uid == (uint32_t)peer_uid &&
			observed.start_time_ticks > 0 && view->pid == peer_pid;
		wlr_log(peer_ok ? WLR_INFO : WLR_ERROR,
			"SD_CI_CHILD_WL_PEER pid=%ld view_pid=%ld uid=%lu "
			"proc_uid=%lu proc_start=%llu evidence=%s authority=NONE",
			(long)peer_pid, (long)view->pid,
			(unsigned long)peer_uid, (unsigned long)observed.uid,
			(unsigned long long)observed.start_time_ticks,
			peer_ok ? "PASS" : "FAIL");
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
	/*
	 * CI-only real Wayland peer evidence. SwayPX view_map already populates
	 * view->pid from this same wl_client, but that stored PID is not a
	 * browser-launch authorization. Re-query kernel-supplied peer credentials
	 * and compare with a fresh Linux /proc identity for the disposable actor.
	 *
	 * The check does not start a broker, bind a NEWS viewer, grant input or
	 * alter the rendering path; evidence=FAIL only fails the test harness.
	 */
	pid_t peer_pid = -1;
	uid_t peer_uid = (uid_t)-1;
	gid_t peer_gid = (gid_t)-1;
	struct stage_director_process_identity observed = {0};
	const bool native_xdg =
		view->type == SWAY_VIEW_XDG_SHELL && view->surface->resource != NULL;
	if (native_xdg) {
		struct wl_client *client =
			wl_resource_get_client(view->surface->resource);
		if (client) {
			wl_client_get_credentials(client, &peer_pid, &peer_uid, &peer_gid);
		}
	}
	const bool proc_ok = peer_pid > 0 &&
		stage_director_read_linux_process((int64_t)peer_pid,
			&observed, NULL);
	const bool verified = native_xdg && proc_ok &&
		peer_pid == view->pid && observed.pid == peer_pid &&
		observed.start_time_ticks > 0 &&
		observed.uid == (uint32_t)peer_uid;
	wlr_log(verified ? WLR_INFO : WLR_ERROR,
		"SD_CI_WL_PEER pid=%ld view_pid=%ld uid=%lu proc_uid=%lu "
		"proc_start=%llu native_xdg=%d evidence=%s",
		(long)peer_pid, (long)view->pid,
		(unsigned long)peer_uid, (unsigned long)observed.uid,
		(unsigned long long)observed.start_time_ticks,
		native_xdg ? 1 : 0, verified ? "PASS" : "FAIL");
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
