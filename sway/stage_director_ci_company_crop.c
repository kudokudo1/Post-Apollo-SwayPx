/* M30g: disposable Company-sized real Qt surface crop experiment ONLY.
 *
 * Compiled exclusively by -Dsd-ci-company-crop-probe=true, never by normal
 * SwayPX. This is NOT selected Company actor custody, MapLibre, host layer
 * permission or production compositor functionality.
 */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>
#include <scenefx/types/fx/blur_data.h>
#include "sway/stage_director_process_evidence.h"
#include "sway/stage_director_ci_company_crop.h"
#include "sway/tree/view.h"
#include "sway/tree/container.h"
#include "sway/tree/root.h"
#include "sway/output.h"
#include "sway/layers.h"

#define COMPANY_TEST_CROP_WIDTH 600
#define COMPANY_TEST_CROP_HEIGHT 380
#define COMPANY_TEST_SOURCE_WIDTH 840
#define COMPANY_TEST_SOURCE_HEIGHT 540
/* Real M30i standard-size Canvas, independently measured from pinned QML.
 * M30j DOES NOT accept app-supplied clip values, nor run on the desktop. */
#define COMPANY_TEST_M30J_WIDTH 573
#define COMPANY_TEST_M30J_HEIGHT 369
#define COMPANY_TEST_M30J_CANVAS_X 166
#define COMPANY_TEST_M30J_CANVAS_Y 242
#define COMPANY_TEST_M30J_HOST_WIDTH 1000
#define COMPANY_TEST_M30J_HOST_HEIGHT 700
/* M30k exact additional pinned Company CI layout. Fullscreen's real
 * 1493x788 Canvas exceeds the 840x540 Qt source: ALWAYS HIDE there. */
#define COMPANY_TEST_M30K_COMPACT_HOST_W 640
#define COMPANY_TEST_M30K_COMPACT_HOST_H 490
#define COMPANY_TEST_M30K_COMPACT_CANVAS_X 166
#define COMPANY_TEST_M30K_COMPACT_CANVAS_Y 281
#define COMPANY_TEST_M30K_COMPACT_CROP_W 441
#define COMPANY_TEST_M30K_COMPACT_CROP_H 128
#define COMPANY_TEST_M30K_FULLSCREEN_HOST_W 1920
#define COMPANY_TEST_M30K_FULLSCREEN_HOST_H 1080

struct ci_company_layout {
    int x, y, width, height;
    int host_width, host_height;
    int profile;
};
static struct ci_company_layout ci_company_active_layout;
static int ci_company_m30k_logged_profile = -1;
static bool ci_company_m30k_fullscreen_logged;
static int ci_company_m30k_last_host_w = -1;
static int ci_company_m30k_last_host_h = -1;
static bool ci_company_m30k_after_fullscreen;
static unsigned ci_company_m30k_recovery_probes;


static struct sway_view *ci_company_crop_actor;
static unsigned ci_company_clip_frames;

static bool ci_company_crop_enabled(void) {
    const char *ci = getenv("CI");
    const char *github = getenv("GITHUB_ACTIONS");
    const char *flag = getenv("STAGE_DIRECTOR_CI_COMPANY_CROP");
    const char *temp = getenv("RUNNER_TEMP");
    return ci && strcmp(ci, "true") == 0 &&
        github && strcmp(github, "true") == 0 &&
        flag && strcmp(flag, "1") == 0 &&
        temp && strcmp(temp, "/home/runner/work/_temp") == 0;
}

/* Only the exact disposable CI test runner may promote the already
 * kernel-matched Qt witness above a genuinely mapped native Company
 * Quickshell *test* layer. The normal SwayPX binary omits this file. */
static bool ci_company_m30j_enabled(void) {
    const char *flag = getenv("STAGE_DIRECTOR_CI_COMPANY_M30J");
    return ci_company_crop_enabled() && flag &&
        strcmp(flag, "1") == 0;
}

static bool ci_company_m30k_enabled(void) {
    const char *flag = getenv("STAGE_DIRECTOR_CI_COMPANY_M30K");
    return ci_company_m30j_enabled() && flag &&
        strcmp(flag, "1") == 0;
}

static int ci_company_crop_width(void) {
    if (ci_company_m30k_enabled())
        return ci_company_active_layout.width;
    return ci_company_m30j_enabled()
        ? COMPANY_TEST_M30J_WIDTH : COMPANY_TEST_CROP_WIDTH;
}

static int ci_company_crop_height(void) {
    if (ci_company_m30k_enabled())
        return ci_company_active_layout.height;
    return ci_company_m30j_enabled()
        ? COMPANY_TEST_M30J_HEIGHT : COMPANY_TEST_CROP_HEIGHT;
}

/* Read only genuine mapped layer-shell objects *owned by the compositor*,
 * not a shell title, QML string, public IPC or a user-chosen rectangle.
 * QML Canvas geometry is separately cross-checked by the Python runner.
 * No runtime permission is granted to the Company Wayland protocol. */
static bool ci_m30j_real_company_layer(int *target_x, int *target_y) {
    if (!ci_company_m30j_enabled() || !root || !root->outputs ||
            root->outputs->length != 1 || !target_x || !target_y)
        return false;
    struct ci_company_layout candidate = {0};
    int found = 0;
    for (int index = 0; index < root->outputs->length; ++index) {
        struct sway_output *output = root->outputs->items[index];
        if (!output || !output->enabled || !output->layers.shell_overlay)
            continue;
        struct sway_layer_surface *layer;
        wl_list_for_each(layer, &output->layer_surfaces, link) {
            struct wlr_layer_surface_v1 *surface = layer->layer_surface;
            if (!layer->mapped || !surface || !surface->resource ||
                    !surface->surface || !surface->surface->mapped ||
                    surface->current.layer !=
                        ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY ||
                    !surface->namespace ||
                    strcmp(surface->namespace,
                        "post-apollo-company-network") != 0)
                continue;
            const int host_w = surface->surface->current.width;
            const int host_h = surface->surface->current.height;
            if (ci_company_m30k_enabled() &&
                    (host_w != ci_company_m30k_last_host_w ||
                     host_h != ci_company_m30k_last_host_h)) {
                ci_company_m30k_last_host_w = host_w;
                ci_company_m30k_last_host_h = host_h;
                wlr_log(WLR_INFO, "SD_CI_COMPANY_M30K_HOST "
                    "mapped_quickshell_layer=%dx%d "
                    "authority=NONE", host_w, host_h);
            }
            int canvas_x = COMPANY_TEST_M30J_CANVAS_X;
            int canvas_y = COMPANY_TEST_M30J_CANVAS_Y;
            int crop_w = COMPANY_TEST_M30J_WIDTH;
            int crop_h = COMPANY_TEST_M30J_HEIGHT;
            int profile = 1;
            if (host_w == COMPANY_TEST_M30J_HOST_WIDTH &&
                    host_h == COMPANY_TEST_M30J_HOST_HEIGHT) {
                /* M30j standard and M30k standard share the exact pin. */
            } else if (ci_company_m30k_enabled() &&
                    host_w == COMPANY_TEST_M30K_COMPACT_HOST_W &&
                    host_h == COMPANY_TEST_M30K_COMPACT_HOST_H) {
                canvas_x = COMPANY_TEST_M30K_COMPACT_CANVAS_X;
                canvas_y = COMPANY_TEST_M30K_COMPACT_CANVAS_Y;
                crop_w = COMPANY_TEST_M30K_COMPACT_CROP_W;
                crop_h = COMPANY_TEST_M30K_COMPACT_CROP_H;
                profile = 2;
            } else if (ci_company_m30k_enabled() &&
                    host_w == COMPANY_TEST_M30K_FULLSCREEN_HOST_W &&
                    host_h == COMPANY_TEST_M30K_FULLSCREEN_HOST_H) {
                /* Real 1493x788 Canvas will not fit the 840x540 source.
                 * Hiding it is mandatory; never stretch or reveal overflow. */
                if (!ci_company_m30k_fullscreen_logged) {
                    ci_company_m30k_after_fullscreen = true;
                    ci_company_m30k_recovery_probes = 0;
                    wlr_log(WLR_INFO, "SD_CI_COMPANY_M30K_FULLSCREEN "
                        "source_smaller_than_canvas=1 scene_hidden=1 "
                        "authority=NONE");
                    ci_company_m30k_fullscreen_logged = true;
                }
                return false;
            } else {
                return false;
            }
            ci_company_m30k_fullscreen_logged = false;

            struct wl_client *client =
                wl_resource_get_client(surface->resource);
            if (!client) return false;
            pid_t pid = -1;
            uid_t uid = (uid_t)-1;
            gid_t gid = (gid_t)-1;
            wl_client_get_credentials(client, &pid, &uid, &gid);
            struct stage_director_process_identity identity = {0};
            if (pid <= 1 || uid != getuid() ||
                    !stage_director_read_linux_process(
                        (int64_t)pid, &identity, NULL) ||
                    identity.pid != pid ||
                    identity.uid != (uint32_t)uid ||
                    identity.start_time_ticks == 0)
                return false;
            char path[64] = {0};
            if (snprintf(path, sizeof(path), "/proc/%ld/exe",
                    (long)pid) <= 0) return false;
            char exe[PATH_MAX] = {0};
            const ssize_t n = readlink(path, exe, sizeof(exe) - 1);
            if (n <= 0 || (size_t)n >= sizeof(exe) - 1)
                return false;
            exe[n] = '\0';
            if (strncmp(exe, "/nix/store/", strlen("/nix/store/")) != 0)
                return false;
            const char *name = strrchr(exe, '/');
            if (!name || (strcmp(name + 1, "quickshell") != 0 &&
                    strcmp(name + 1, "qs") != 0 &&
                    strcmp(name + 1, ".quickshell-wrapped") != 0))
                return false;

            if (!layer->scene || !layer->scene->tree) return false;
            int lx = 0, ly = 0;
            if (!wlr_scene_node_coords(&layer->scene->tree->node,
                    &lx, &ly)) return false;
            if (lx < output->lx || ly < output->ly ||
                    lx + host_w >
                        output->lx + output->width ||
                    ly + host_h >
                        output->ly + output->height ||
                    canvas_x < 0 || canvas_y < 0 ||
                    crop_w <= 0 || crop_h <= 0 ||
                    crop_w > COMPANY_TEST_SOURCE_WIDTH ||
                    crop_h > COMPANY_TEST_SOURCE_HEIGHT ||
                    canvas_x + crop_w > host_w ||
                    canvas_y + crop_h > host_h)
                return false;
            found++;
            candidate = (struct ci_company_layout) {
                .x = lx + canvas_x, .y = ly + canvas_y,
                .width = crop_w, .height = crop_h,
                .host_width = host_w, .host_height = host_h,
                .profile = profile,
            };
            *target_x = candidate.x;
            *target_y = candidate.y;
        }
    }
    if (found != 1) return false;
    ci_company_active_layout = candidate;
    if (ci_company_m30k_enabled() &&
            ci_company_m30k_after_fullscreen &&
            ci_company_m30k_recovery_probes++ < 7) {
        wlr_log(WLR_INFO, "SD_CI_COMPANY_M30K_HOST_RECOVERY "
            "validated_profile=%d target=%d,%d "
            "source_expected=840x540 authority=NONE",
            candidate.profile, candidate.x, candidate.y);
    }
    return true;
}

static bool ci_exact_native_qt(struct sway_view *view) {
    if (!ci_company_crop_enabled() || !view ||
            view->type != SWAY_VIEW_XDG_SHELL ||
            !view->surface || !view->surface->resource ||
            !view->container || view->container->node.destroying ||
            !view->content_tree) return false;

    /* Title is an intentionally CI-specific fixture selector, NOT security
     * authority. Kernel credentials, starttime and executing binary are
     * additionally checked before touching a test-only scene tree. */
    const char *title = view_get_title(view);
    if (!title ||
            strcmp(title, "SD-CI-CompanyVectorProbe-crop") != 0) {
        return false;
    }
    struct wl_client *client = wl_resource_get_client(view->surface->resource);
    if (!client) return false;
    pid_t pid = -1;
    uid_t uid = (uid_t)-1;
    gid_t gid = (gid_t)-1;
    wl_client_get_credentials(client, &pid, &uid, &gid);
    struct stage_director_process_identity id = {0};
    if (pid <= 1 || uid != getuid() || view->pid != pid ||
            !stage_director_read_linux_process((int64_t)pid, &id, NULL) ||
            id.pid != (int64_t)pid || id.uid != (uint32_t)uid ||
            id.start_time_ticks == 0) {
        return false;
    }
    char link[64] = {0};
    if (snprintf(link, sizeof(link), "/proc/%ld/exe", (long)pid) <= 0)
        return false;
    char exe[PATH_MAX] = {0};
    const ssize_t len = readlink(link, exe, sizeof(exe) - 1);
    if (len <= 0 || (size_t)len >= sizeof(exe) - 1) return false;
    exe[len] = '\0';
    if (strncmp(exe, "/nix/store/", strlen("/nix/store/")) != 0)
        return false;
    const char *base = strrchr(exe, '/');
    if (!base) return false;
    return strcmp(base + 1, "sd-company-qt-wayland-probe") == 0 ||
        strcmp(base + 1, ".sd-company-qt-wayland-probe-wrapped") == 0;
}

static void ci_company_crop_buffer(struct wlr_scene_buffer *buffer,
        int sx, int sy, void *data) {
    (void)sx;
    (void)sy;
    (void)data;
    if (!wlr_scene_surface_try_from_buffer(buffer)) return;
    /* wlr_scene_subsurface_tree_set_clip sets the *source* crop; Sway's
     * output_configure_scene separately sets buffer *destination* size,
     * so normalize both after its normal frame pass. */
    wlr_scene_buffer_set_dest_size(buffer,
        ci_company_crop_width(), ci_company_crop_height());
}

static void ci_company_uncrop_buffer(struct wlr_scene_buffer *buffer,
        int sx, int sy, void *data) {
    (void)sx;
    (void)sy;
    (void)data;
    if (wlr_scene_surface_try_from_buffer(buffer))
        wlr_scene_buffer_set_dest_size(buffer, 0, 0);
}

static void ci_company_apply_clip(struct sway_view *view) {
    if (!view || !view->content_tree) return;
    const struct wlr_box crop = {
        .x = 0, .y = 0,
        .width = ci_company_crop_width(),
        .height = ci_company_crop_height(),
    };
    wlr_scene_subsurface_tree_set_clip(&view->content_tree->node, &crop);
    wlr_scene_node_for_each_buffer(&view->content_tree->node,
        ci_company_crop_buffer, NULL);
}

void stage_director_ci_company_crop_on_map(struct sway_view *view) {
    if (!ci_exact_native_qt(view)) return;
    if (ci_company_crop_actor && ci_company_crop_actor != view) {
        wlr_log(WLR_ERROR, "SD_CI_COMPANY_CROP ambiguous_native_root=1 FAIL");
        return;
    }
    ci_company_crop_actor = view;
    ci_company_clip_frames = 0;
    ci_company_m30k_logged_profile = -1;
    ci_company_m30k_fullscreen_logged = false;
    ci_company_m30k_last_host_w = -1;
    ci_company_m30k_last_host_h = -1;
    ci_company_m30k_after_fullscreen = false;
    ci_company_m30k_recovery_probes = 0;
    wlr_log(WLR_INFO,
        "SD_CI_COMPANY_CROP_NATIVE peer_pid=%ld source_expected=840x540 "
        "visible=600x380 selection=CI_ONLY authority=NONE",
        (long)view->pid);
}

void stage_director_ci_company_crop_on_unmap(struct sway_view *view) {
    if (!view || view != ci_company_crop_actor) return;
    ci_company_crop_actor = NULL;
    ci_company_clip_frames = 0;
    ci_company_active_layout = (struct ci_company_layout) {0};
    ci_company_m30k_logged_profile = -1;
    if (view->content_tree && view->container &&
            !view->container->node.destroying) {
        view_center_and_clip_surface(view);
        wlr_scene_node_for_each_buffer(&view->content_tree->node,
            ci_company_uncrop_buffer, NULL);
    }
    wlr_log(WLR_INFO, "SD_CI_COMPANY_CROP_NATIVE unmapped=1 "
        "crop_revoked=1 authority=NONE");
}

static bool ci_company_crop_ready(void) {
    struct sway_view *view = ci_company_crop_actor;
    const bool native_ok = ci_exact_native_qt(view);
    if (!native_ok ||
            view->geometry.width != COMPANY_TEST_SOURCE_WIDTH ||
            view->geometry.height != COMPANY_TEST_SOURCE_HEIGHT ||
            view->geometry.x != 0 || view->geometry.y != 0) {
        if (ci_company_m30k_enabled() &&
                ci_company_m30k_after_fullscreen &&
                ci_company_m30k_recovery_probes++ < 7) {
            wlr_log(WLR_INFO, "SD_CI_COMPANY_M30K_RECOVERY_REFUSED "
                "source_native_identity=%d geometry=%d,%d %dx%d",
                native_ok,
                view ? view->geometry.x : -1,
                view ? view->geometry.y : -1,
                view ? view->geometry.width : -1,
                view ? view->geometry.height : -1);
        }
        return false;
    }
    if (!ci_company_m30j_enabled()) return true;
    if (!view->container || !view->container->scene_tree)
        return false;
    int x = 0, y = 0, target_x = 0, target_y = 0;
    const bool host_ok =
        ci_m30j_real_company_layer(&target_x, &target_y);
    if (!host_ok) {
        if (ci_company_m30k_enabled() &&
                ci_company_m30k_after_fullscreen &&
                ci_company_m30k_recovery_probes++ < 7) {
            wlr_log(WLR_INFO, "SD_CI_COMPANY_M30K_RECOVERY_REFUSED "
                "host_identity=0 source_identity=1 authority=NONE");
        }
        return false;
    }
    /* M30k may be recovering a node deliberately HIDDEN while the
     * fullscreen host was unsupported. wlroots refuses coordinates for
     * disabled scene nodes, by design. Do not demand the unavailable
     * *old* coordinates: host+source+PID are already verified above.
     * The CI-only caller repositions the hidden node to the trusted
     * mapped host target BEFORE revealing it. No scene/input grants
     * or identity checks are relaxed, and ordinary M30j is unchanged. */
    if (ci_company_m30k_enabled()) return true;
    if (!wlr_scene_node_coords(&view->container->scene_tree->node,
            &x, &y)) return false;
    return abs(x - target_x) <= 4 && abs(y - target_y) <= 4;
}

/* Layout and Qt surface commits are asynchronous. Temporarily unknown
 * dimensions must NOT expose an uncropped orange backing buffer. Hide the
 * isolated actor until the native source returns to its exact 840x540
 * expected geometry; the output screenshot itself is the final verdict. */
static unsigned ci_company_pending_frames;

static bool ci_company_crop_revalidate_or_hide(const char *phase) {
    struct sway_view *view = ci_company_crop_actor;
    if (!view || !view->container || !view->container->scene_tree)
        return false;
    if (!ci_company_crop_ready()) {
        wlr_scene_node_set_enabled(&view->container->scene_tree->node, false);
        if (ci_company_pending_frames++ < 5) {
            wlr_log(WLR_INFO,
                "SD_CI_COMPANY_CROP_PENDING phase=%s "
                "geometry=%d,%d %dx%d clip_held=0 scene_hidden=1 "
                "authority=NONE",
                phase, view->geometry.x, view->geometry.y,
                view->geometry.width, view->geometry.height);
        }
        return false;
    }
    if (ci_company_m30j_enabled()) {
        int target_x = 0, target_y = 0;
        if (!ci_m30j_real_company_layer(&target_x, &target_y))
            return false;
        /* M30j TEST ONLY: put a single kernel-matched Qt test actor above
         * the real Quickshell overlay. Only its 573x369 crop may receive
         * pixels/hit-tests. This does NOT authorize a real Company child.
         * Never reparent the layer itself or relax unrelated input. */
        struct wlr_scene_node *scene =
            &view->container->scene_tree->node;
        if (scene->parent != root->layers.popup) {
            wlr_scene_node_reparent(scene, root->layers.popup);
        }
        if (ci_company_m30k_enabled()) {
            int parent_x = 0, parent_y = 0;
            if (!wlr_scene_node_coords(&root->layers.popup->node,
                    &parent_x, &parent_y))
                return false;
            wlr_scene_node_set_position(scene,
                target_x - parent_x, target_y - parent_y);
            if (ci_company_m30k_logged_profile !=
                    ci_company_active_layout.profile) {
                ci_company_m30k_logged_profile =
                    ci_company_active_layout.profile;
                if (ci_company_active_layout.profile == 1)
                    ci_company_m30k_after_fullscreen = false;
                wlr_log(WLR_INFO, "SD_CI_COMPANY_M30K_FOLLOW "
                    "profile=%d crop=%dx%d target=%d,%d "
                    "same_native_root=1 scene=CI_ONLY authority=NONE",
                    ci_company_active_layout.profile,
                    ci_company_active_layout.width,
                    ci_company_active_layout.height,
                    target_x, target_y);
            }
        } else if (scene->parent == root->layers.popup) {
            /* Preserve the original M30j single-placement behavior. */
            int parent_x = 0, parent_y = 0;
            if (!wlr_scene_node_coords(&root->layers.popup->node,
                    &parent_x, &parent_y))
                return false;
            wlr_scene_node_set_position(scene,
                target_x - parent_x, target_y - parent_y);
        }
        wlr_scene_node_raise_to_top(scene);
    }
    ci_company_apply_clip(view);
    wlr_scene_node_set_enabled(&view->container->scene_tree->node, true);
    return true;
}

void stage_director_ci_company_crop_post_layout(void) {
    if (ci_company_crop_actor)
        (void)ci_company_crop_revalidate_or_hide("layout");
}

void stage_director_ci_company_crop_pre_frame(void) {
    if (!ci_company_crop_actor ||
            !ci_company_crop_revalidate_or_hide("pre_frame")) return;
    /* Must run AFTER output_configure_scene and BEFORE output scene-build. */
    if (ci_company_clip_frames++ < 3) {
        wlr_log(WLR_INFO, "SD_CI_COMPANY_CROP_FRAME "
            "real_native_qt=1 source=840x540 visible=600x380 "
            "buffer_dest_normalized=1 pixels=PENDING authority=NONE");
        if (ci_company_m30j_enabled()) {
            wlr_log(WLR_INFO, "SD_CI_COMPANY_M30J_REAL_PANEL "
                "real_quickshell_layer=1 native_qt=1 "
                "source=840x540 clip=573x369 "
                "above_overlay=CI_ONLY authority=NONE");
        }
    }
}
