#ifndef _SWAY_STAGE_DIRECTOR_VIEWPORT_CLIP_H
#define _SWAY_STAGE_DIRECTOR_VIEWPORT_CLIP_H

#include <stdbool.h>
#include "sway/stage_director_viewport.h"

struct sway_view;

/*
 * Dormant SD1 V1 compositor-native clip backend.
 * NOT wired to SD0 map, observe, transaction, commands, IPC or NEWS buttons.
 * A later authorized owner must install a trusted session, including input
 * / popup / teardown protection, and reapply after normal sway layout.
 */
bool stage_director_viewport_clip_view(struct sway_view *view,
		const struct stage_director_viewport_request *request,
		const struct stage_director_viewport_authority *authority);

/*
 * Restore SwayFX's normal centering/content clip while the view is mapped.
 * Caller owns unmap/destroy invalidation and must never retain a stale view.
 */
bool stage_director_viewport_restore_view(struct sway_view *view);

#endif
