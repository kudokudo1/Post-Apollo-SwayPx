#ifndef _SWAY_STAGE_DIRECTOR_CI_SCENE_PROBE_H
#define _SWAY_STAGE_DIRECTOR_CI_SCENE_PROBE_H
#include <stdbool.h>
struct sway_view;
struct wlr_surface;
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
/* CI only: early native cgroup test before the new Chrome sibling's
 * workspace layout. No untrusted title, app_id or shell assertion. */
bool stage_director_ci_scene_probe_is_chrome_sibling(
	struct sway_view *view);
/* CI only: suppress the *entire* sibling container's scene and any
 * separate output/decoration paint before renderer presentation. */
void stage_director_ci_scene_probe_quarantine_chrome_sibling(
	struct sway_view *view);
/* CI-only seat gate: reject a kernel/cgroup-verified hidden native Chrome
 * sibling BEFORE keyboard/activation/focus stack changes. */
bool stage_director_ci_scene_probe_reject_chrome_focus(
	struct sway_view *view);
/* CI-only direct-surface gate for tracked native sibling root surfaces.
 * Nested surface focus is separately unverified. */
bool stage_director_ci_scene_probe_reject_chrome_surface_focus(
	struct wlr_surface *surface);
/* CI-only instrumentation at wlroots event delivery boundaries.
 * Reports actual focused/target surface; never grants input authority. */
void stage_director_ci_scene_probe_record_input_delivery(
	struct wlr_surface *surface, bool is_keyboard);
/* Observe actual wlroots button delivery target and press/release state.
 * This does not synthesize input or grant production authority. */
void stage_director_ci_scene_probe_record_pointer_button_delivery(
	struct wlr_surface *surface, bool pressed);
void stage_director_ci_scene_probe(struct sway_view *view);
/* Remove any CI screenshot hold before the disposable actor unmaps. */
void stage_director_ci_scene_probe_restore(struct sway_view *view);
/* CI-only post-arrange hook; production Meson option stays disabled. */
void stage_director_ci_scene_probe_post_layout(void);
/* CI-only pre-output-build containment, never a production authority. */
void stage_director_ci_scene_probe_pre_frame_guard(void);
/* Discard guarded native-view pointer before unmap destroys the view. */
void stage_director_ci_scene_probe_early_unmap(struct sway_view *view);
/* Only the disposable CI compositor may keep a tested 320x240 scene buffer
 * destination when output_configure_scene normally resets it per frame. */
bool stage_director_ci_scene_probe_preserve_dest(struct wlr_scene_buffer *buffer);
#endif
