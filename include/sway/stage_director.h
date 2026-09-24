#ifndef _SWAY_STAGE_DIRECTOR_H
#define _SWAY_STAGE_DIRECTOR_H

struct sway_container;
struct sway_view;

/*
 * Stage Director SD0
 * ------------------
 * Shadow-only scene observation.
 *
 * This subsystem is deliberately non-authoritative: it may identify actors,
 * group them into scenes, observe compositor geometry, and write diagnostics,
 * but it MUST NOT mutate container geometry, focus, layout, or lifecycle.
 */

void stage_director_view_mapped(struct sway_view *view);
void stage_director_view_unmapped(struct sway_view *view);
void stage_director_view_identity_changed(struct sway_view *view);
void stage_director_observe_container(struct sway_container *container,
		const char *reason);

#endif
