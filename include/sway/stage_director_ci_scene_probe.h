#ifndef _SWAY_STAGE_DIRECTOR_CI_SCENE_PROBE_H
#define _SWAY_STAGE_DIRECTOR_CI_SCENE_PROBE_H
#include <stdbool.h>
struct sway_view;
struct wlr_scene_buffer;
/* Available only to an explicitly test-enabled compositor binary. */
void stage_director_ci_scene_probe(struct sway_view *view);
/* Remove any CI screenshot hold before the disposable actor unmaps. */
void stage_director_ci_scene_probe_restore(struct sway_view *view);
/* CI-only post-arrange hook; production Meson option stays disabled. */
void stage_director_ci_scene_probe_post_layout(void);
/* Only the disposable CI compositor may keep a tested 320x240 scene buffer
 * destination when output_configure_scene normally resets it per frame. */
bool stage_director_ci_scene_probe_preserve_dest(struct wlr_scene_buffer *buffer);
#endif
