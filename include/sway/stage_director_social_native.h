#ifndef _SWAY_STAGE_DIRECTOR_SOCIAL_NATIVE_H
#define _SWAY_STAGE_DIRECTOR_SOCIAL_NATIVE_H

#include <stdbool.h>
#include <stdint.h>

struct sway_container;
struct sway_view;

/* Production Social W floating XDG pair.
 * Only Sway IPC's existing local-user authority may request admission;
 * the compositor independently checks both exact mapped native surfaces,
 * identity, workspace and already-docked geometry. Never accept app_id alone.
 * This is a local desktop feature, NOT a cross-UID security privilege.
 */
bool stage_director_social_native_begin(struct sway_container *leader,
		uint64_t *generation);
bool stage_director_social_native_move(struct sway_container *leader,
		uint64_t generation);
bool stage_director_social_native_resize(struct sway_container *leader,
		uint64_t generation);
void stage_director_social_native_guest_above(struct sway_container *leader,
		uint64_t generation);
void stage_director_social_native_view_lost(struct sway_view *view);

#endif
