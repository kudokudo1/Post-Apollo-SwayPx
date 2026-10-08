#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "log.h"
#include "sway/stage_director.h"
#include "sway/stage_director_profile.h"
#include "sway/tree/container.h"
#include "sway/tree/view.h"

#define STAGE_DIRECTOR_MAX_ACTORS 64
#define STAGE_DIRECTOR_LOG_PATH "/tmp/swayfx-stage-director-shadow.log"

struct stage_director_actor {
	struct sway_view *view;
	struct stage_director_identity identity;
	bool occupied;
	bool last_geometry_valid;
	int last_x;
	int last_y;
	int last_width;
	int last_height;
};

static struct stage_director_actor actors[STAGE_DIRECTOR_MAX_ACTORS];
static FILE *shadow_log;
static bool announced;

static void shadow_open(void) {
	if (shadow_log) {
		return;
	}
	shadow_log = fopen(STAGE_DIRECTOR_LOG_PATH, "w");
	if (!shadow_log) {
		sway_log(SWAY_ERROR,
			"[stage-director][SD0] unable to open shadow log %s",
			STAGE_DIRECTOR_LOG_PATH);
		return;
	}
	setvbuf(shadow_log, NULL, _IOLBF, 0);
}

static void shadow_write(const char *message) {
	shadow_open();
	sway_log(SWAY_INFO, "%s", message);
	if (shadow_log) {
		fprintf(shadow_log, "%s\n", message);
	}
}

static void announce_once(void) {
	if (announced) {
		return;
	}
	announced = true;
	shadow_write(
		"[stage-director][SD0] shadow online: generic scene registry; "
		"observe/classify/model/log only; NO geometry, focus, layout, or lifecycle authority");
}

static struct stage_director_actor *find_actor(struct sway_view *view) {
	for (int i = 0; i < STAGE_DIRECTOR_MAX_ACTORS; ++i) {
		if (actors[i].occupied && actors[i].view == view) {
			return &actors[i];
		}
	}
	return NULL;
}

static struct stage_director_actor *alloc_actor(struct sway_view *view) {
	struct stage_director_actor *actor = find_actor(view);
	if (actor) {
		return actor;
	}
	for (int i = 0; i < STAGE_DIRECTOR_MAX_ACTORS; ++i) {
		if (!actors[i].occupied) {
			actors[i] = (struct stage_director_actor) {
				.view = view,
				.occupied = true,
			};
			return &actors[i];
		}
	}
	shadow_write("[stage-director][SD0] actor registry full; observation dropped");
	return NULL;
}

static void log_scene_status(const char *scene, int expected) {
	int count = 0;
	int anchors = 0;
	int passive = 0;
	int leaders = 0;

	for (int i = 0; i < STAGE_DIRECTOR_MAX_ACTORS; ++i) {
		if (!actors[i].occupied || !actors[i].identity.scene ||
				strcmp(actors[i].identity.scene, scene) != 0) {
			continue;
		}
		count++;
		if (actors[i].identity.anchor) {
			anchors++;
		}
		if (actors[i].identity.passive) {
			passive++;
		}
		if (actors[i].identity.can_lead) {
			leaders++;
		}
	}

	char line[512];
	snprintf(line, sizeof(line),
		"[stage-director][SD0] scene=%s actors=%d/%d anchors=%d "
		"passive=%d leaders=%d ready=%s",
		scene, count, expected, anchors, passive, leaders,
		count == expected && anchors == 1 ? "yes" : "no");
	shadow_write(line);
}

static void classify(struct sway_view *view, const char *reason) {
	announce_once();

	/*
	 * Identity queries are only safe while the view is mapped and its
	 * container is live. Transaction application also runs for containers
	 * that are being torn down; asking profile classifiers to inspect those
	 * views can reach backend state after unmap.
	 */
	if (!view || !view->container || !view->surface ||
			view->container->node.destroying) {
		return;
	}

	struct stage_director_identity identity = {0};
	if (!stage_director_profile_classify(view, &identity)) {
		return;
	}

	struct stage_director_actor *actor = alloc_actor(view);
	if (!actor) {
		return;
	}

	bool changed = !actor->identity.profile ||
		!actor->identity.scene ||
		!actor->identity.role ||
		strcmp(actor->identity.profile, identity.profile) != 0 ||
		strcmp(actor->identity.scene, identity.scene) != 0 ||
		strcmp(actor->identity.role, identity.role) != 0 ||
		actor->identity.passive != identity.passive ||
		actor->identity.anchor != identity.anchor ||
		actor->identity.can_lead != identity.can_lead;

	actor->identity = identity;

	if (changed) {
		char line[768];
		snprintf(line, sizeof(line),
			"[stage-director][SD0] identify reason=%s profile=%s "
			"scene=%s role=%s anchor=%s passive=%s can_lead=%s "
			"app_id=%s title=%s",
			reason ? reason : "unknown",
			identity.profile ? identity.profile : "-",
			identity.scene,
			identity.role,
			identity.anchor ? "yes" : "no",
			identity.passive ? "yes" : "no",
			identity.can_lead ? "yes" : "no",
			view_get_app_id(view) ? view_get_app_id(view) : "-",
			view_get_title(view) ? view_get_title(view) : "-");
		shadow_write(line);

		if (identity.scene_geometry.declared || identity.transform.declared) {
			char model_line[768];
			snprintf(model_line, sizeof(model_line),
				"[stage-director][SD0] model profile=%s scene=%s role=%s "
				"scene_size=%dx%d transform_declared=%s "
				"transform=x(%.3f*W%+d) y(%.3f*H%+d) "
				"w(%.3f*W%+d) h(%.3f*H%+d) authority=NONE",
				identity.profile ? identity.profile : "-",
				identity.scene,
				identity.role,
				identity.scene_geometry.width,
				identity.scene_geometry.height,
				identity.transform.declared ? "yes" : "no",
				identity.transform.x_rel,
				identity.transform.x_px,
				identity.transform.y_rel,
				identity.transform.y_px,
				identity.transform.width_rel,
				identity.transform.width_px,
				identity.transform.height_rel,
				identity.transform.height_px);
			shadow_write(model_line);
		}

		log_scene_status(identity.scene, identity.expected_scene_actors);
	}
}

void stage_director_view_mapped(struct sway_view *view) {
	classify(view, "map");
	if (view && view->container) {
		stage_director_observe_container(view->container, "map");
	}
}

void stage_director_view_identity_changed(struct sway_view *view) {
	classify(view, "identity");
}

void stage_director_view_unmapped(struct sway_view *view) {
	struct stage_director_actor *actor = find_actor(view);
	if (!actor) {
		return;
	}

	char scene[128] = {0};
	char role[128] = {0};
	int expected = actor->identity.expected_scene_actors;
	if (actor->identity.scene) {
		snprintf(scene, sizeof(scene), "%s", actor->identity.scene);
	}
	if (actor->identity.role) {
		snprintf(role, sizeof(role), "%s", actor->identity.role);
	}

	char line[384];
	snprintf(line, sizeof(line),
		"[stage-director][SD0] unmap scene=%s role=%s",
		scene[0] ? scene : "-", role[0] ? role : "-");
	shadow_write(line);

	memset(actor, 0, sizeof(*actor));
	if (scene[0]) {
		log_scene_status(scene, expected);
	}
}

void stage_director_observe_container(struct sway_container *container,
		const char *reason) {
	if (!container || !container->view || container->node.destroying ||
			!container->view->surface) {
		return;
	}

	classify(container->view, reason ? reason : "geometry");

	struct stage_director_actor *actor = find_actor(container->view);
	if (!actor) {
		return;
	}

	int x = (int)container->current.x;
	int y = (int)container->current.y;
	int width = (int)container->current.width;
	int height = (int)container->current.height;

	if (actor->last_geometry_valid &&
			actor->last_x == x &&
			actor->last_y == y &&
			actor->last_width == width &&
			actor->last_height == height) {
		return;
	}

	actor->last_geometry_valid = true;
	actor->last_x = x;
	actor->last_y = y;
	actor->last_width = width;
	actor->last_height = height;

	char line[640];
	snprintf(line, sizeof(line),
		"[stage-director][SD0] observe reason=%s profile=%s scene=%s role=%s "
		"actual=%dx%d@%d,%d floating=%s authority=NONE",
		reason ? reason : "unknown",
		actor->identity.profile ? actor->identity.profile : "-",
		actor->identity.scene,
		actor->identity.role,
		width, height, x, y,
		container_is_floating(container) ? "yes" : "no");
	shadow_write(line);
}
