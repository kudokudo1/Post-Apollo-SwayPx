#ifndef _SWAY_TIMEW_SCOPE_A_CI_REAL_XDG_H
#define _SWAY_TIMEW_SCOPE_A_CI_REAL_XDG_H

/*
 * Disposable-headless CI hook, NOT a SwayPX feature or an authority API.
 * This header/source is absent from production sway/meson.build.
 *
 * The runner injects calls into its OWN temporary sway/tree/view.c checkout
 * to exercise true SwayPX view_map -> view.events.unmap ordering against
 * a real mapped XDG client. No click/gesture, C3e/C4b, C5, IPC, restore,
 * resize, move, or protected receipt is implemented.
 */
struct sway_view;
void timew_scope_a_ci_real_xdg_after_map(struct sway_view *view);
void timew_scope_a_ci_real_xdg_after_unmap_signal(struct sway_view *view);

#endif
