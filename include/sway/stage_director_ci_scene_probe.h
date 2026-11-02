#ifndef _SWAY_STAGE_DIRECTOR_CI_SCENE_PROBE_H
#define _SWAY_STAGE_DIRECTOR_CI_SCENE_PROBE_H
#include <stdbool.h>
struct sway_view;
struct wlr_scene_buffer;
/* Available only to an explicitly test-enabled compositor binary. */
/* CI-only: override *one verified actor* to a borderless Sway container,
 * before the first layout transaction. All other views keep their border.
 * Does not grant production WATCH, scene, popup, focus or input authority.
 */
bool stage_director_ci_scene_probe_borderless_verified_actor(
	struct sway_view *view);
/* CI-only exact native Chrome actor: deny ALL XDG popup creation before
 * it enters root->layers.popup. Bounds alone cannot protect that tree.
 * Returns false for unrelated native views; never a production permission.
 */
bool stage_director_ci_scene_probe_reject_chrome_popup(
	struct sway_view *view);
void stage_director_ci_scene_probe(struct sway_view *view);
/* Remove any CI screenshot hold before the disposable actor unmaps. */
void stage_director_ci_scene_probe_restore(struct sway_view *view);
/* CI-only post-arrange hook; production Meson option stays disabled. */
void stage_director_ci_scene_probe_post_layout(void);
/* Only the disposable CI compositor may keep a tested 320x240 scene buffer
 * destination when output_configure_scene normally resets it per frame. */
bool stage_director_ci_scene_probe_preserve_dest(struct wlr_scene_buffer *buffer);
#endif
