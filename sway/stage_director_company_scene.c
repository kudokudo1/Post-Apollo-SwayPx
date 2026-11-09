#include <limits.h>
#ifdef SD_STAGE_DIRECTOR_CI_COMPANY_SCENE_ACTUATION
#include <stdlib.h>
#include <string.h>
#include "log.h"
#endif
#include <stdint.h>
#include <scenefx/types/fx/blur_data.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wayland-util.h>
#include "sway/tree/root.h"
#include "sway/tree/container.h"
#include "sway/tree/view.h"
#include "sway/stage_director_company_native.h"
#include "sway/stage_director_company_scene.h"
#include "sway/stage_director_company_input.h"

/*
 * M32c // actual SwayPX/wlroots scene API backend.
 *
 * This file is compiled into the ordinary compositor. There is no normal
 * caller that can invoke the setters, and scene_apply is a hard refusal
 * without the dedicated, disposable CI build flag. Production may only
 * enable it once M32d owner-issued scene, seat, popup, focus and teardown
 * guarantees are independently reviewed. Never turn on with an env var.
 */
#ifdef SD_STAGE_DIRECTOR_CI_COMPANY_SCENE_ACTUATION
static struct {
    struct sway_view *view;
    struct wlr_scene_tree *original_parent;
    int original_x;
    int original_y;
    bool visible;
    uint64_t selection_epoch;
    struct stage_director_rect viewport;
} ci_scene;

static bool company_m32e_ci_refuse(const char *gate) {
    sway_log(SWAY_INFO,
        "[stage-director][COMPANY_M32E][CI_ONLY][DENIED] gate=%s",
        gate);
    return false;
}
#endif

void stage_director_company_scene_revoke(struct sway_view *view) {
#ifdef SD_STAGE_DIRECTOR_CI_COMPANY_SCENE_ACTUATION
    if (!ci_scene.view || (view && ci_scene.view != view)) return;
    /* Teardown MUST run before native scene node and parent deletion. A
     * revocation leaves the actor HIDDEN so it cannot leak a floating Qt
     * window above the desktop. Owner must close it or explicitly restore
     * it to the normal workspace in a separately authorized operation. */
    struct sway_view *actor = ci_scene.view;
    if (actor->container && !actor->container->node.destroying &&
            actor->container->scene_tree) {
        struct wlr_scene_node *node =
            &actor->container->scene_tree->node;
        wlr_scene_node_set_enabled(node, false);
        /* One-way safety: after a selected-host loss the former parent
         * may belong to an output being destroyed. Leave the actor hidden,
         * rather than dereferencing a possibly stale original parent or
         * exposing the MapLibre window as a floating desktop surface. */
        if (actor->content_tree && actor->surface) {
            view_center_and_clip_surface(actor);
        }
    }
    ci_scene.view = NULL;
    ci_scene.original_parent = NULL;
    ci_scene.visible = false;
    ci_scene.selection_epoch = 0;
    ci_scene.viewport = (struct stage_director_rect){0};
#else
    (void)view;
#endif
}

bool stage_director_company_scene_apply_ci_only(
        struct sway_view *view,
        const struct stage_director_company_actor_pair_evidence *current,
        const struct stage_director_company_scene_guard *guard) {
#ifndef SD_STAGE_DIRECTOR_CI_COMPANY_SCENE_ACTUATION
    /* Deliberate compile-time refusal, including if future buggy code
     * accidentally calls this with fabricated "approved" booleans. */
    (void)view;
    (void)current;
    (void)guard;
    return false;
#else
    if (!view || !view->surface || !view->content_tree ||
            !view->container || view->container->node.destroying ||
            !view->container->scene_tree || !current ||
            !root || !root->layers.popup ||
            !view->wlr_xdg_toplevel || !view->wlr_xdg_toplevel->base)
        return company_m32e_ci_refuse("scene-object-not-ready");
    /* Popups created BEFORE scene admission would already be independent
     * unmasked trees. Refuse the actor if its native XDG root has ANY
     * existing popup descendants. New top-level/nested popups are denied
     * by the M32d XDG hook once the actor is embedded. */
    if (!wl_list_empty(&view->wlr_xdg_toplevel->base->popups))
        return company_m32e_ci_refuse("preexisting-xdg-popup");

    /* Do not use a client-supplied view/ID, desktop title or app-id.
     * Re-observe the actual mapped native XDG root and match every field
     * to the M32b actor evidence obtained from compositor-side Wayland. */
    const struct stage_director_company_native_actor now =
        stage_director_company_native_observe_actor(view);
    if (!now.candidate ||
            now.native_view_ref != current->actor.native_view_ref ||
            now.native_root_surface_ref !=
                current->actor.native_root_surface_ref ||
            now.native_client_ref != current->actor.native_client_ref ||
            now.pid != current->actor.pid ||
            now.uid != current->actor.uid ||
            now.start_ticks != current->actor.start_ticks ||
            now.root_global.width != current->actor.root_global.width ||
            now.root_global.height != current->actor.root_global.height)
        return company_m32e_ci_refuse("native-root-reobservation-changed");

    const struct stage_director_rect source = {
        .x = view->geometry.x, .y = view->geometry.y,
        .width = view->geometry.width,
        .height = view->geometry.height,
    };
    const struct stage_director_company_scene_plan plan =
        stage_director_company_scene_prepare(current, guard, source);
    if (plan.state != SD_COMPANY_SCENE_READY_FOR_PRIVATE_CI_ONLY ||
            !plan.geometry_ready || plan.can_apply_in_production ||
            view->geometry.x != 0 || view->geometry.y != 0) {
        sway_log(SWAY_INFO,
            "[stage-director][COMPANY_M32E][CI_ONLY][SCENE_PLAN] "
            "state=%d exact_source=%d,%d %dx%d actor_root=%dx%d",
            (int)plan.state, source.x, source.y, source.width,
            source.height, current->actor.root_global.width,
            current->actor.root_global.height);
        return company_m32e_ci_refuse("scene-plan-or-geometry");
    }
    struct wlr_scene_node *node = &view->container->scene_tree->node;
    if (!node->parent || (ci_scene.view && ci_scene.view != view) ||
            !node->enabled)
        return company_m32e_ci_refuse("scene-node-disabled-or-occupied");

    int scene_x=0,scene_y=0,content_x=0,content_y=0,parent_x=0,parent_y=0;
    if (!wlr_scene_node_coords(node,&scene_x,&scene_y) ||
            !wlr_scene_node_coords(&view->content_tree->node,
                &content_x,&content_y) ||
            !wlr_scene_node_coords(&root->layers.popup->node,
                &parent_x,&parent_y))
        return company_m32e_ci_refuse("scene-coordinate-query");
    const int64_t x=(int64_t)plan.target_global.x -
        ((int64_t)content_x-scene_x)-parent_x;
    const int64_t y=(int64_t)plan.target_global.y -
        ((int64_t)content_y-scene_y)-parent_y;
    if (x<INT_MIN || x>INT_MAX || y<INT_MIN || y>INT_MAX)
        return company_m32e_ci_refuse("scene-coordinate-overflow");

    if (!ci_scene.view) {
        ci_scene.view=view;
        ci_scene.original_parent=node->parent;
        ci_scene.original_x=node->x;
        ci_scene.original_y=node->y;
    }
    /* Hide until clipping is installed. Do not resize the imported buffer;
     * M30t/M31a showed forced destination remapping can shear native RGB565.
     * The pure plan requires the exact Qt source size to match the canvas. */
    wlr_scene_node_set_enabled(node,false);
    if (node->parent != root->layers.popup)
        wlr_scene_node_reparent(node,root->layers.popup);
    wlr_scene_node_set_position(node,(int)x,(int)y);
    const struct wlr_box clip = {
        .x = plan.root_surface_clip.x,
        .y = plan.root_surface_clip.y,
        .width = plan.root_surface_clip.width,
        .height = plan.root_surface_clip.height,
    };
    wlr_scene_subsurface_tree_set_clip(
        &view->content_tree->node,&clip);
    wlr_scene_node_raise_to_top(node);
    wlr_scene_node_set_enabled(node,true);
    ci_scene.viewport = plan.target_global;
    ci_scene.selection_epoch = guard->selection_epoch;
    ci_scene.visible = true;
    return true;
#endif
}

#ifdef SD_STAGE_DIRECTOR_CI_COMPANY_SCENE_ACTUATION
/*
 * M32e // positive native-scene probe for the previously observed M30s
 * fullscreen fixture only. The inputs below model test evidence; they do
 * NOT authenticate a launched process or approve an installed executable.
 * This function does not exist in an ordinary compositor build. It is
 * additionally closed outside the exact GitHub disposable session.
 *
 * The fixed Company rectangle (166,203,1493,788) comes from the earlier
 * real Company Quickshell M30s source, not client-provided IPC or app-id.
 * A true runtime PASS requires native MapLibre and captured compositor
 * pixels: successfully compiling this function does not establish that.
 */
bool stage_director_company_scene_apply_m32e_fixture_only(
        struct sway_view *view,
        const struct stage_director_company_native_host *host) {
    const char *ci = getenv("CI");
    const char *gha = getenv("GITHUB_ACTIONS");
    const char *m32e = getenv("STAGE_DIRECTOR_CI_COMPANY_M32E");
    const char *temp = getenv("RUNNER_TEMP");
    if (!ci || strcmp(ci, "true") || !gha || strcmp(gha, "true") ||
            !m32e || strcmp(m32e, "1") || !temp ||
            strcmp(temp, "/home/runner/work/_temp"))
        return company_m32e_ci_refuse("disposable-environment");
    if (!view || !host || !host->candidate ||
            host->state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
            host->host_global.x != 0 || host->host_global.y != 0 ||
            host->host_global.width != 1920 ||
            host->host_global.height != 1080 ||
            host->output_global.x != 0 || host->output_global.y != 0 ||
            host->output_global.width != 1920 ||
            host->output_global.height != 1080 ||
            !host->native_client_ref || !host->native_surface_ref ||
            !host->native_output_ref || !host->native_layer_role_ref ||
            !host->native_layer_ref || host->pid <= 0 || !host->uid ||
            !host->start_ticks)
        return company_m32e_ci_refuse("current-native-company-host");
    const struct stage_director_company_native_actor actor =
        stage_director_company_native_observe_actor(view);
    if (!actor.candidate ||
            actor.state != SD_COMPANY_NATIVE_CANDIDATE_ONLY ||
            !actor.native_client_ref || !actor.native_view_ref ||
            !actor.native_root_surface_ref ||
            actor.native_client_ref == host->native_client_ref ||
            actor.pid <= 0 || actor.pid == host->pid ||
            actor.uid != host->uid || !actor.start_ticks ||
            actor.root_global.width != 1493 ||
            actor.root_global.height != 788 ||
            view->geometry.x != 0 || view->geometry.y != 0 ||
            view->geometry.width != 1493 ||
            view->geometry.height != 788) {
        sway_log(SWAY_INFO,
            "[stage-director][COMPANY_M32E][CI_ONLY][ACTOR_DIAG] "
            "root=%d,%d %dx%d view_geo=%d,%d %dx%d actor_state=%d",
            actor.root_global.x, actor.root_global.y,
            actor.root_global.width, actor.root_global.height,
            view->geometry.x, view->geometry.y,
            view->geometry.width, view->geometry.height,
            (int)actor.state);
        return company_m32e_ci_refuse("current-native-qt-actor");
    }
    /* Explicitly FAKE lineage, digest, binary and lease attestations.
     * These can only flow through this private-CI function. Production has
     * no caller and no approved executable digest. Native refs, geometry,
     * actual wlroots node operations and screenshots must be real. */
    const struct stage_director_company_actor_pair_evidence e = {
        .selected_native_company_host = true,
        .kernel_pidfd_actor_candidate = true,
        .host = *host,
        .actor = actor,
        .intent = {
            .present = true, .selection_epoch = 1, .sequence = 1,
            .host_revision = 1,
            .native_host_client_ref = host->native_client_ref,
            .mode = 0,
        },
        .viewport = {
            .candidate_only = true, .selection_epoch = 1,
            .intent_sequence = 1, .host_revision = 1,
            .native_host_client_ref = host->native_client_ref,
            .frame_in_host = {0, 0, 1920, 1080},
            .map_in_frame = {166, 203, 1493, 788},
            .map_in_host = {166, 203, 1493, 788},
        },
        .lineage = {
            .status = SD_COMPANY_LINEAGE_CANDIDATE_ONLY,
            .direct_child_candidate = true,
        },
        .executable = {
            .status = SD_COMPANY_EXE_CURRENT_INSTALL_INODE_CANDIDATE,
            .running_current_install_inode_candidate = true,
        },
        .digest = {
            .status = SD_COMPANY_DIGEST_PIN_MATCH_CANDIDATE,
            .pinned_elf_hash_candidate = true,
        },
    };
    const struct stage_director_company_scene_guard guard = {
        .operator_authorized_native_actor = true,
        .approved_executable_and_launch = true,
        .compositor_scene_lease_current = true,
        .input_hit_test_guard_installed = true,
        .keyboard_focus_guard_installed = true,
        .popup_guard_installed = true,
        .teardown_guard_installed = true,
        .host_mapped = true, .actor_mapped = true,
        .output_active = true,
        .selection_epoch = 1, .intent_sequence = 1,
        .host_revision = 1,
        .native_host_client_ref = host->native_client_ref,
        .native_actor_root_surface_ref = actor.native_root_surface_ref,
    };
    const struct stage_director_rect exact_source = {0, 0, 1493, 788};
    const struct stage_director_company_scene_plan plan =
        stage_director_company_scene_prepare(&e, &guard, exact_source);
    if (plan.state != SD_COMPANY_SCENE_READY_FOR_PRIVATE_CI_ONLY ||
            !plan.geometry_ready || plan.can_apply_in_production) {
        const struct stage_director_company_actor_pair_result pair =
            stage_director_company_actor_pair_probe(&e);
        sway_log(SWAY_INFO,
            "[stage-director][COMPANY_M32E][CI_ONLY][FIXTURE_PLAN] "
            "state=%d pair_status=%d pair_target=%d,%d %dx%d",
            (int)plan.state, (int)pair.status,
            pair.viewport_global.x, pair.viewport_global.y,
            pair.viewport_global.width, pair.viewport_global.height);
        return company_m32e_ci_refuse("fixture-pair-or-scene-plan");
    }
    return stage_director_company_scene_apply_ci_only(view, &e, &guard);
}
#endif

/* M32d // real cursor, popup and focus hooks consult the scene owned by
 * this compositor, not a client title/app-id. These do not enable the
 * scene. When the test-only actor is absent all hooks pass through. */
static struct stage_director_company_input_snapshot
current_company_input_scene(void) {
    struct stage_director_company_input_snapshot out = {0};
#ifdef SD_STAGE_DIRECTOR_CI_COMPANY_SCENE_ACTUATION
    if (ci_scene.visible && ci_scene.view && ci_scene.view->surface) {
        out.scene_active = true;
        out.selection_epoch = ci_scene.selection_epoch;
        out.native_actor_view_ref = (uintptr_t)ci_scene.view;
        out.native_actor_root_surface_ref =
            (uintptr_t)ci_scene.view->surface;
        out.map_global = ci_scene.viewport;
    }
#endif
    return out;
}

bool stage_director_company_scene_reject_popup(struct sway_view *view) {
    const struct stage_director_company_input_snapshot scene =
        current_company_input_scene();
    return stage_director_company_input_reject_popup(
        &scene, (uintptr_t)view);
}

bool stage_director_company_scene_reject_keyboard_view(
        struct sway_view *view) {
    const struct stage_director_company_input_snapshot scene =
        current_company_input_scene();
    return view && scene.scene_active &&
        scene.native_actor_view_ref == (uintptr_t)view;
}

bool stage_director_company_scene_is_embedded_root(
        struct wlr_surface *surface) {
    const struct stage_director_company_input_snapshot scene =
        current_company_input_scene();
    struct wlr_surface *root_surface = surface ?
        wlr_surface_get_root_surface(surface) : NULL;
    return scene.scene_active && root_surface &&
        scene.native_actor_root_surface_ref == (uintptr_t)root_surface;
}

bool stage_director_company_scene_reject_keyboard_surface(
        struct wlr_surface *surface) {
    return stage_director_company_scene_is_embedded_root(surface);
}

bool stage_director_company_scene_reject_pointer(
        struct wlr_surface *hit_surface,
        struct wlr_surface *focused_surface,
        double global_x, double global_y, bool session_locked) {
    const struct stage_director_company_input_snapshot scene =
        current_company_input_scene();
    struct wlr_surface *hit_root = hit_surface ?
        wlr_surface_get_root_surface(hit_surface) : NULL;
    struct wlr_surface *focused_root = focused_surface ?
        wlr_surface_get_root_surface(focused_surface) : NULL;
    return stage_director_company_input_pointer(
        &scene, (uintptr_t)hit_root, (uintptr_t)focused_root,
        global_x, global_y, session_locked).deny_event;
}
