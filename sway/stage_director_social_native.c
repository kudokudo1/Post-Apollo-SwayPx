#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>

#include "log.h"
#include "sway/commands.h"
#include "sway/config.h"
#include "sway/stage_director_social_native.h"
#include "sway/tree/arrange.h"
#include "sway/tree/container.h"
#include "sway/tree/root.h"
#include "sway/tree/view.h"

/*
 * Stage Director: opt-in native Social W floating XDG group.
 *
 * Unlike the earlier CI source overlay, this is a normal production-linked
 * module. The group remains EMPTY until the existing local Sway IPC
 * command explicitly pairs two ALREADY MAPPED and docked native windows.
 * Same-user Sway IPC is already privileged to manipulate containers;
 * this is not an independent authentication facility for foreign users.
 *
 * Never bind on a title or app_id alone: each pair stores exact live native
 * Sway pointers, wl_clients, root surfaces, workspace, and a generation.
 * Destroy/unmap callbacks invalidate it BEFORE either pointer is freed.
 */
struct social_pair {
	struct sway_container *host;
	struct sway_container *guest;
	struct sway_view *host_view;
	struct sway_view *guest_view;
	struct wlr_surface *host_surface;
	struct wlr_surface *guest_surface;
	struct wl_client *host_client;
	struct wl_client *guest_client;
	struct sway_workspace *workspace;
	uint64_t generation;
	int rail;
	int inset;
};
static struct social_pair pair;
static uint64_t last_generation;

static void revoke(const char *why) {
	if (pair.generation) {
		sway_log(SWAY_DEBUG,
			"[stage-director][social-native] revoked generation=%" PRIu64
			" reason=%s", pair.generation, why);
	}
	memset(&pair, 0, sizeof(pair));
}

static bool exact_host(struct sway_container *con) {
	if (!con || con->node.destroying || !con->view ||
			!con->view->surface || !con->view->surface->resource ||
			!container_is_floating(con) || !con->pending.workspace ||
			!con->scene_tree || !view_is_visible(con->view)) {
		return false;
	}
	const char *title = view_get_title(con->view);
	const char *shell = view_get_shell(con->view);
	return title && !strcmp(title, "QS_SOCIAL_MENU") &&
		shell && !strcmp(shell, "xdg_shell");
}

static bool exact_guest(struct sway_container *con) {
	if (!con || con->node.destroying || !con->view ||
			!con->view->surface || !con->view->surface->resource ||
			!container_is_floating(con) || !con->pending.workspace ||
			!con->scene_tree || !view_is_visible(con->view)) {
		return false;
	}
	const char *id = view_get_app_id(con->view);
	const char *shell = view_get_shell(con->view);
	return id && (!strcmp(id, "vesktop") ||
			!strcmp(id, "dev.vencord.vesktop")) &&
		shell && !strcmp(shell, "xdg_shell");
}

static bool identity_current(void) {
	if (!pair.generation || !pair.host || !pair.guest ||
			!exact_host(pair.host) || !exact_guest(pair.guest) ||
			pair.host->view != pair.host_view ||
			pair.guest->view != pair.guest_view ||
			pair.host->view->surface != pair.host_surface ||
			pair.guest->view->surface != pair.guest_surface ||
			pair.host->pending.workspace != pair.workspace ||
			pair.guest->pending.workspace != pair.workspace ||
			pair.host->pending.fullscreen_mode != FULLSCREEN_NONE ||
			pair.guest->pending.fullscreen_mode != FULLSCREEN_NONE ||
			!pair.host->current.workspace ||
			!pair.guest->current.workspace ||
			pair.host->current.workspace != pair.guest->current.workspace) {
		return false;
	}
	struct wl_client *h = wl_resource_get_client(
		pair.host_surface->resource);
	struct wl_client *g = wl_resource_get_client(
		pair.guest_surface->resource);
	return h && g && h != g &&
		h == pair.host_client && g == pair.guest_client;
}

/* Never dereference a dead pointer here. view_unmap/view_destroy call this
 * while the exact sway_view is still live, before the scene frees it. */
void stage_director_social_native_view_lost(struct sway_view *view) {
	if (view && pair.generation &&
			(view == pair.host_view || view == pair.guest_view)) {
		revoke("native-view-lost");
	}
}

static bool initial_slot(struct sway_container *host,
		struct sway_container *guest, int rail, int inset) {
	if (rail < 40 || rail > 500 || inset < 0 || inset > 20 ||
			host->pending.width < rail + 260 ||
			host->pending.height < 260 ||
			guest->pending.width < 200 || guest->pending.height < 200 ||
			host->pending.workspace != guest->pending.workspace ||
			host->pending.fullscreen_mode != FULLSCREEN_NONE ||
			guest->pending.fullscreen_mode != FULLSCREEN_NONE)
		return false;
	return fabs(guest->pending.x - host->pending.x - rail - inset) <= 1 &&
		fabs(guest->pending.y - host->pending.y - inset) <= 1 &&
		fabs(guest->pending.width - host->pending.width + rail +
			2 * inset) <= 1 &&
		fabs(guest->pending.height - host->pending.height +
			2 * inset) <= 1;
}

static bool find_by_id(struct sway_container *con, void *user) {
	const size_t target = *(const size_t *)user;
	return con && con->node.id == target && !con->node.destroying;
}

static struct sway_container *exact_container(size_t id) {
	return root_find_container(find_by_id, &id);
}

static bool positive_id(const char *source, size_t *out) {
	if (!source || !*source || *source == '-' || *source == '+')
		return false;
	errno = 0;
	char *end = NULL;
	const unsigned long long value = strtoull(source, &end, 10);
	if (errno || !end || *end || !value ||
			value > SIZE_MAX) return false;
	*out = (size_t)value;
	return true;
}

static bool bounded_int(const char *source, int *out, int min, int max) {
	if (!source || !*source) return false;
	errno = 0;
	char *end = NULL;
	const long value = strtol(source, &end, 10);
	if (errno || !end || *end || value < min || value > max) return false;
	*out = (int)value;
	return true;
}

struct cmd_results *cmd_stage_director_social(int argc, char **argv) {
	if (!config->active || config->reading)
		return cmd_results_new(CMD_FAILURE,
			"Stage Director Social is a runtime-only local Sway operation");
	if (argc == 1 && !strcmp(argv[0], "clear")) {
		revoke("explicit-clear");
		return cmd_results_new(CMD_SUCCESS, NULL);
	}
	if (argc == 1 && !strcmp(argv[0], "status")) {
		return cmd_results_new(pair.generation && identity_current() ?
			CMD_SUCCESS : CMD_FAILURE,
			pair.generation && identity_current() ? NULL :
			"Stage Director Social has no current native pair");
	}
	if (argc != 5 || strcmp(argv[0], "pair"))
		return cmd_results_new(CMD_INVALID,
			"Expected: stage_director_social pair <host-id> <guest-id> "
			"<rail-width> <inset> | status | clear");

	size_t host_id=0, guest_id=0;
	int rail=0, inset=0;
	if (!positive_id(argv[1], &host_id) ||
			!positive_id(argv[2], &guest_id) ||
			host_id == guest_id ||
			!bounded_int(argv[3], &rail, 40, 500) ||
			!bounded_int(argv[4], &inset, 0, 20))
		return cmd_results_new(CMD_INVALID, "Invalid Social pair arguments");

	struct sway_container *host = exact_container(host_id);
	struct sway_container *guest = exact_container(guest_id);
	if (!exact_host(host) || !exact_guest(guest) ||
			!initial_slot(host, guest, rail, inset))
		return cmd_results_new(CMD_FAILURE,
			"Social host/guest not exact mapped and docked floating XDG views");

	struct wl_client *hc = wl_resource_get_client(
		host->view->surface->resource);
	struct wl_client *gc = wl_resource_get_client(
		guest->view->surface->resource);
	if (!hc || !gc || hc == gc || host->view->pid <= 0 ||
			guest->view->pid <= 0 || last_generation == UINT64_MAX)
		return cmd_results_new(CMD_FAILURE, "Untrusted Social native pair");

	if (pair.generation && pair.host == host && pair.guest == guest &&
			pair.rail == rail && pair.inset == inset && identity_current())
		return cmd_results_new(CMD_SUCCESS, NULL);

	revoke("replace-current");
	pair = (struct social_pair) {
		.host = host, .guest = guest,
		.host_view = host->view, .guest_view = guest->view,
		.host_surface = host->view->surface,
		.guest_surface = guest->view->surface,
		.host_client = hc, .guest_client = gc,
		.workspace = host->pending.workspace,
		.generation = ++last_generation,
		.rail = rail, .inset = inset,
	};
	/* The guest must paint above an opaque Quickshell chassis. */
	container_raise_floating(guest);
	sway_log(SWAY_INFO, "[stage-director][social-native] paired "
		"generation=%" PRIu64 " host=%zu guest=%zu",
		pair.generation, host_id, guest_id);
	return cmd_results_new(CMD_SUCCESS, NULL);
}

bool stage_director_social_native_begin(struct sway_container *leader,
		uint64_t *generation) {
	if (generation) *generation = 0;
	if (!leader || !pair.generation ||
			(leader != pair.host && leader != pair.guest))
		return false;
	if (!identity_current()) {
		revoke("gesture-identity-lost");
		return false;
	}
	if (generation) *generation = pair.generation;
	return true;
}

void stage_director_social_native_guest_above(struct sway_container *leader,
		uint64_t generation) {
	if (generation && generation == pair.generation &&
			leader == pair.host && identity_current())
		container_raise_floating(pair.guest);
}

bool stage_director_social_native_move(struct sway_container *leader,
		uint64_t generation) {
	if (!generation || generation != pair.generation ||
			(leader != pair.host && leader != pair.guest))
		return false;
	if (!identity_current()) {
		revoke("move-identity-lost");
		return false;
	}
	struct sway_container *other =
		leader == pair.host ? pair.guest : pair.host;
	const double x = leader == pair.host
		? leader->pending.x + pair.rail + pair.inset
		: leader->pending.x - pair.rail - pair.inset;
	const double y = leader == pair.host
		? leader->pending.y + pair.inset
		: leader->pending.y - pair.inset;
	container_floating_move_to(other, x, y);
	return true;
}

bool stage_director_social_native_resize(struct sway_container *leader,
		uint64_t generation) {
	if (!generation || generation != pair.generation ||
			(leader != pair.host && leader != pair.guest))
		return false;
	if (!identity_current()) {
		revoke("resize-identity-lost");
		return false;
	}
	struct sway_container *other =
		leader == pair.host ? pair.guest : pair.host;
	double x, y, width, height;
	if (leader == pair.host) {
		x = leader->pending.x + pair.rail + pair.inset;
		y = leader->pending.y + pair.inset;
		width = leader->pending.width - pair.rail - 2 * pair.inset;
		height = leader->pending.height - 2 * pair.inset;
	} else {
		x = leader->pending.x - pair.rail - pair.inset;
		y = leader->pending.y - pair.inset;
		width = leader->pending.width + pair.rail + 2 * pair.inset;
		height = leader->pending.height + 2 * pair.inset;
	}
	if (width < 200 || height < 200 || !isfinite(x) || !isfinite(y) ||
			!isfinite(width) || !isfinite(height)) {
		revoke("resize-constraints");
		return false;
	}
	/* Stage the other member's entire pending view + content box before
	 * the existing seatop's SINGLE transaction_commit_dirty(). */
	const double dx = x - other->pending.x;
	const double dy = y - other->pending.y;
	const double dw = width - other->pending.width;
	const double dh = height - other->pending.height;
	other->pending.x += dx;
	other->pending.y += dy;
	other->pending.width += dw;
	other->pending.height += dh;
	other->pending.content_x += dx;
	other->pending.content_y += dy;
	other->pending.content_width += dw;
	other->pending.content_height += dh;
	arrange_container(other);
	return true;
}
