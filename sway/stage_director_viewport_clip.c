#include <string.h>
#include <scenefx/types/fx/blur_data.h>
#include <wayland-util.h>
#include <wlr/types/wlr_scene.h>
#include "sway/stage_director_viewport_clip.h"
#include "sway/tree/container.h"
#include "sway/tree/view.h"

/*
 * A real wlroots scene-subtree cropping primitive, but intentionally NOT
 * connected to runtime events. Calling it with fabricated identity alone
 * is insufficient: every independent authority gate must already be true.
 *
 * Standard Sway view_center_and_clip_surface()/transaction code rewrites
 * these clips. A future compositor session owner must reapply at the proper
 * post-layout boundary and reset on invalidation before live activation.
 * This function makes no claim about independently positioned XDG popups,
 * XWayland override-redirect surfaces, or pointer hit-test containment.
 */
static bool news_actor_identity(struct sway_view *view) {
	const char *app_id = view_get_app_id(view);
	const char *window_class = view_get_class(view);
	return (app_id && strcmp(app_id, "post-apollo-weather-news") == 0) ||
		(window_class &&
		 strcmp(window_class, "post-apollo-weather-news") == 0);
}

bool stage_director_viewport_clip_view(struct sway_view *view,
		const struct stage_director_viewport_request *request,
		const struct stage_director_viewport_authority *authority) {
	if (!view || !request || !authority ||
			!view->surface || !view->content_tree || !view->container ||
			view->container->node.destroying ||
			!news_actor_identity(view)) {
		return false;
	}
	if (wl_list_empty(&view->content_tree->children)) {
		return false;
	}
	/*
	 * The actor envelope MUST come from actual scene coordinates and the
	 * active wl_surface geometry. Refuse a request containing a geometry
	 * reported by some other display or stale compositor frame.
	 */
	int actual_x = 0;
	int actual_y = 0;
	wlr_scene_node_coords(&view->content_tree->node, &actual_x, &actual_y);
	if (request->actor.x != actual_x ||
			request->actor.y != actual_y ||
			request->actor.width != view->geometry.width ||
			request->actor.height != view->geometry.height ||
			view->geometry.width <= 0 || view->geometry.height <= 0) {
		return false;
	}
	const struct stage_director_viewport_clip_plan clip =
		stage_director_viewport_prepare_clip(request, authority,
			view->geometry.x, view->geometry.y);
	if (!clip.allowed) {
		return false;
	}
	const struct wlr_box box = {
		.x = clip.root_surface_clip.x,
		.y = clip.root_surface_clip.y,
		.width = clip.root_surface_clip.width,
		.height = clip.root_surface_clip.height,
	};
	wlr_scene_subsurface_tree_set_clip(&view->content_tree->node, &box);
	return true;
}

bool stage_director_viewport_restore_view(struct sway_view *view) {
	if (!view || !view->surface || !view->container ||
			view->container->node.destroying || !view->content_tree) {
		return false;
	}
	/* Do NOT restore with NULL: the normal compositor clip may be nonempty. */
	view_center_and_clip_surface(view);
	return true;
}
