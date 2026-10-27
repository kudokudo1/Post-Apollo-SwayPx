#ifndef _SWAY_STAGE_DIRECTOR_CI_SCENE_PROBE_H
#define _SWAY_STAGE_DIRECTOR_CI_SCENE_PROBE_H
struct sway_view;
/* Available only to an explicitly test-enabled compositor binary. */
void stage_director_ci_scene_probe(struct sway_view *view);
/* Remove any CI screenshot hold before the disposable actor unmaps. */
void stage_director_ci_scene_probe_restore(struct sway_view *view);
/* CI-only post-arrange hook; production Meson option stays disabled. */
void stage_director_ci_scene_probe_post_layout(void);
#endif
