#include <scenefx/types/fx/blur_data.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include "sway/layers.h"
#include "sway/output.h"
#include "sway/stage_director_company_native.h"
#include "sway/stage_director_process_evidence.h"
#include "sway/tree/container.h"
#include "sway/tree/view.h"

/* Kernel data is observed twice to reject torn/reused process snapshots.
 * It establishes only Linux Wayland peer evidence, not parentage or custody
 * of the Company M15 process or operator intent. */
static bool observe_peer_process(struct wl_client *client,
		int64_t *pid_out, uint32_t *uid_out, uint64_t *start_out) {
	if (!client || !pid_out || !uid_out || !start_out) return false;
	pid_t pid = -1;
	uid_t uid = (uid_t)-1;
	wl_client_get_credentials(client, &pid, &uid, NULL);
	if (pid <= 0 || uid == (uid_t)-1) return false;

	struct stage_director_process_identity first = {0};
	struct stage_director_process_identity second = {0};
	if (!stage_director_read_linux_process((int64_t)pid, &first, NULL) ||
			!stage_director_read_linux_process((int64_t)pid, &second, NULL) ||
			first.pid != pid || second.pid != pid ||
			first.uid != (uint32_t)uid || second.uid != (uint32_t)uid ||
			!first.start_time_ticks ||
			first.start_time_ticks != second.start_time_ticks ||
			first.parent_pid != second.parent_pid) {
		return false;
	}
	*pid_out = (int64_t)pid;
	*uid_out = (uint32_t)uid;
	*start_out = second.start_time_ticks;
	return true;
}

struct stage_director_company_native_host
stage_director_company_native_observe_host(
		const struct sway_layer_surface *surface) {
	struct stage_director_company_native_host out = {
		.state = SD_COMPANY_NATIVE_MISSING,
	};
	if (!surface || !surface->layer_surface ||
			!surface->layer_surface->surface || !surface->scene ||
			!surface->scene->tree || !surface->output ||
			!surface->output->wlr_output) return out;

	const struct wlr_layer_surface_v1 *role = surface->layer_surface;
	if (!role->namespace ||
			strcmp(role->namespace, SD_COMPANY_NATIVE_NAMESPACE)) {
		out.state = SD_COMPANY_NATIVE_WRONG_NAMESPACE;
		return out;
	}
	if (!role->initialized || !surface->mapped ||
			!role->surface->mapped) {
		out.state = SD_COMPANY_NATIVE_UNMAPPED;
		return out;
	}
	if (role->current.layer != ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY ||
			!role->resource || !role->surface->resource ||
			wlr_surface_get_root_surface(role->surface) != role->surface) {
		out.state = SD_COMPANY_NATIVE_WRONG_ROLE;
		return out;
	}
	struct wl_client *role_client = wl_resource_get_client(role->resource);
	struct wl_client *surface_client =
		wl_resource_get_client(role->surface->resource);
	if (!role_client || role_client != surface_client) {
		out.state = SD_COMPANY_NATIVE_WRONG_CLIENT;
		return out;
	}
	int64_t peer_pid = 0;
	uint32_t peer_uid = 0;
	uint64_t peer_start = 0;
	if (!observe_peer_process(role_client, &peer_pid,
			&peer_uid, &peer_start)) {
		out.state = SD_COMPANY_NATIVE_PROCESS_UNVERIFIED;
		return out;
	}
	int global_x = 0, global_y = 0;
	wlr_scene_node_coords(&surface->scene->tree->node, &global_x, &global_y);
	struct wlr_box output_box = {0};
	output_get_box(surface->output, &output_box);
	out.host_global = (struct stage_director_rect) {
		global_x, global_y,
		role->surface->current.width, role->surface->current.height,
	};
	out.output_global = (struct stage_director_rect) {
		output_box.x, output_box.y, output_box.width, output_box.height,
	};
	if (!surface->output->enabled ||
			!stage_director_rect_contains(out.output_global,
				out.host_global)) {
		out.state = SD_COMPANY_NATIVE_OUTPUT_INVALID;
		return out;
	}
	out.native_layer_ref = (uintptr_t)surface;
	out.native_layer_role_ref = (uintptr_t)role->resource;
	out.native_surface_ref = (uintptr_t)role->surface->resource;
	out.native_client_ref = (uintptr_t)role_client;
	out.native_output_ref = (uintptr_t)surface->output;
	out.pid = peer_pid;
	out.uid = peer_uid;
	out.start_ticks = peer_start;
	out.candidate = true;
	out.state = SD_COMPANY_NATIVE_CANDIDATE_ONLY;
	return out;
}

struct stage_director_company_native_actor
stage_director_company_native_observe_actor(const struct sway_view *view) {
	struct stage_director_company_native_actor out = {
		.state = SD_COMPANY_NATIVE_MISSING,
	};
	/* Reject Xwayland, popups, destroyed/unmapped views and anything without
	 * a native XDG toplevel root. app_id/title are not ownership evidence. */
	if (!view || view->type != SWAY_VIEW_XDG_SHELL ||
			!view->wlr_xdg_toplevel || !view->wlr_xdg_toplevel->base ||
			!view->surface || !view->container ||
			view->container->node.destroying || !view->content_tree) {
		return out;
	}
	if (!view->surface->mapped) {
		out.state = SD_COMPANY_NATIVE_UNMAPPED;
		return out;
	}
	if (view->wlr_xdg_toplevel->base->surface != view->surface ||
			wlr_surface_get_root_surface(view->surface) != view->surface ||
			!view->surface->resource) {
		out.state = SD_COMPANY_NATIVE_WRONG_ROLE;
		return out;
	}
	struct wl_client *client =
		wl_resource_get_client(view->surface->resource);
	if (!client) {
		out.state = SD_COMPANY_NATIVE_WRONG_CLIENT;
		return out;
	}
	int64_t peer_pid = 0;
	uint32_t peer_uid = 0;
	uint64_t peer_start = 0;
	if (!observe_peer_process(client, &peer_pid, &peer_uid, &peer_start)) {
		out.state = SD_COMPANY_NATIVE_PROCESS_UNVERIFIED;
		return out;
	}
	int global_x = 0, global_y = 0;
	wlr_scene_node_coords(&view->content_tree->node,
			&global_x, &global_y);
	out.root_global = (struct stage_director_rect) {
		global_x, global_y,
		view->surface->current.width, view->surface->current.height,
	};
	if (!stage_director_rect_valid(out.root_global)) {
		out.state = SD_COMPANY_NATIVE_OUTPUT_INVALID;
		return out;
	}
	out.native_view_ref = (uintptr_t)view;
	out.native_root_surface_ref = (uintptr_t)view->surface->resource;
	out.native_client_ref = (uintptr_t)client;
	out.pid = peer_pid;
	out.uid = peer_uid;
	out.start_ticks = peer_start;
	out.state = SD_COMPANY_NATIVE_CANDIDATE_ONLY;
	out.candidate = true;
	return out;
}
