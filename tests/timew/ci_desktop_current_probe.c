/* Disposable CI-only, included into ipc-server.c by a guarded overlay.
 * No installed API, lifetime UUID, revision, source authority or Restore.
 * This is a bounded CURRENT topology witness, not a M12a capture batch. */
#include <inttypes.h>
#include <math.h>

#define CI_CAPTURE_LIMIT 1024
#define CI_CAPTURE_DEPTH 64

struct ci_capture_walk {
	size_t ids[CI_CAPTURE_LIMIT];
	size_t count;
	json_object *rows;
};

static bool ci_capture_node(struct ci_capture_walk *walk,
		struct sway_node *node, enum sway_node_type type) {
	if (!node || node->type != type || !node->id || node->dirty ||
			node->destroying || node->instruction || node->ntxnrefs ||
			walk->count == CI_CAPTURE_LIMIT) {
		return false;
	}
	for (size_t i = 0; i < walk->count; i++) {
		if (walk->ids[i] == node->id) {
			return false;
		}
	}
	walk->ids[walk->count++] = node->id;
	return true;
}

static bool ci_capture_list(list_t *list) {
	return list && list->length >= 0 && list->length <= CI_CAPTURE_LIMIT;
}

static bool ci_capture_container(struct ci_capture_walk *walk,
		struct sway_container *con, struct sway_container *parent,
		struct sway_workspace *ws, struct sway_output *out,
		bool floating, unsigned depth) {
	if (!con || depth >= CI_CAPTURE_DEPTH ||
			!ci_capture_node(walk, &con->node, N_CONTAINER) ||
			con->current.parent != parent || con->current.workspace != ws ||
			con->scratchpad || con->current.fullscreen_mode != FULLSCREEN_NONE) {
		return false;
	}
	if (!con->view) {
		if (!ci_capture_list(con->current.children) ||
				con->current.children->length == 0) {
			return false;
		}
		for (int i = 0; i < con->current.children->length; i++) {
			if (!ci_capture_container(walk, con->current.children->items[i],
					con, ws, out, floating, depth + 1)) {
				return false;
			}
		}
		return true;
	}
	struct sway_view *view = con->view;
	if (view->type != SWAY_VIEW_XDG_SHELL || view->destroying ||
			view->container != con || !view->surface || !view->surface->mapped ||
			(con->current.children && con->current.children->length)) {
		return false;
	}
	double values[] = {con->current.x, con->current.y,
		con->current.width, con->current.height};
	_Static_assert(sizeof(double) == sizeof(uint64_t), "binary64 required");
	json_object *bits = json_object_new_array();
	if (!bits) {
		return false;
	}
	for (unsigned i = 0; i < 4; i++) {
		if (!isfinite(values[i]) || (i >= 2 && values[i] <= 0)) {
			json_object_put(bits);
			return false;
		}
		uint64_t raw;
		memcpy(&raw, &values[i], sizeof(raw));
		char hex[17];
		snprintf(hex, sizeof(hex), "%016" PRIx64, raw);
		json_object_array_add(bits, json_object_new_string(hex));
	}
	json_object *row = json_object_new_object();
	if (!row) {
		json_object_put(bits);
		return false;
	}
	json_object_object_add(row, "con_id", json_object_new_uint64(con->node.id));
	json_object_object_add(row, "workspace_node", json_object_new_uint64(ws->node.id));
	json_object_object_add(row, "output_node", json_object_new_uint64(out->node.id));
	json_object_object_add(row, "floating", json_object_new_boolean(floating));
	json_object_object_add(row, "current_bits", bits);
	json_object_array_add(walk->rows, row);
	return true;
}

static json_object *ci_capture_current(void) {
	json_object *result = json_object_new_object();
	struct ci_capture_walk walk = {.rows = json_object_new_array()};
	const char *reason = "unsupported_or_inconsistent_topology";
	if (server.pending_transaction || server.queued_transaction ||
			!server.dirty_nodes || server.dirty_nodes->length) {
		reason = "transaction_busy";
		goto refused;
	}
	if (!root || server.session_lock.lock || root->fullscreen_global ||
			!ci_capture_list(root->scratchpad) || root->scratchpad->length ||
			!ci_capture_list(root->outputs) || !root->outputs->length ||
			!walk.rows) {
		goto refused;
	}
	for (int i = 0; i < root->outputs->length; i++) {
		struct sway_output *out = root->outputs->items[i];
		if (!out || !out->enabled || !out->wlr_output ||
				!ci_capture_node(&walk, &out->node, N_OUTPUT) ||
				!ci_capture_list(out->current.workspaces)) {
			goto refused;
		}
		for (int j = 0; j < out->current.workspaces->length; j++) {
			struct sway_workspace *ws = out->current.workspaces->items[j];
			if (!ws || !ci_capture_node(&walk, &ws->node, N_WORKSPACE) ||
					ws->current.output != out || ws->current.fullscreen ||
					!ci_capture_list(ws->current.tiling) ||
					!ci_capture_list(ws->current.floating)) {
				goto refused;
			}
			for (int k = 0; k < ws->current.tiling->length; k++) {
				if (!ci_capture_container(&walk, ws->current.tiling->items[k],
						NULL, ws, out, false, 0)) {
					goto refused;
				}
			}
			for (int k = 0; k < ws->current.floating->length; k++) {
				if (!ci_capture_container(&walk, ws->current.floating->items[k],
						NULL, ws, out, true, 0)) {
					goto refused;
				}
			}
		}
	}
	json_object_object_add(result, "status", json_object_new_string("CURRENT_DIAGNOSTIC"));
	json_object_object_add(result, "rows", walk.rows);
	json_object_object_add(result, "authority", json_object_new_string("NONE"));
	return result;
refused:
	json_object_put(walk.rows);
	json_object_object_add(result, "status", json_object_new_string("REFUSED"));
	json_object_object_add(result, "reason", json_object_new_string(reason));
	json_object_object_add(result, "authority", json_object_new_string("NONE"));
	return result;
}
