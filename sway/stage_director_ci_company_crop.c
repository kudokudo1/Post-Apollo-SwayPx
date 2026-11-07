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

static int ci_company_crop_width(void) {
    return ci_company_m30j_enabled()
        ? COMPANY_TEST_M30J_WIDTH : COMPANY_TEST_CROP_WIDTH;
}

static int ci_company_crop_height(void) {
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
                        "post-apollo-company-network") != 0 ||
                    surface->surface->current.width !=
                        COMPANY_TEST_M30J_HOST_WIDTH ||
                    surface->surface->current.height !=
                        COMPANY_TEST_M30J_HOST_HEIGHT)
                continue;

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
                    lx + COMPANY_TEST_M30J_HOST_WIDTH >
                        output->lx + output->width ||
                    ly + COMPANY_TEST_M30J_HOST_HEIGHT >
                        output->ly + output->height)
                return false;
            found++;
            *target_x = lx + COMPANY_TEST_M30J_CANVAS_X;
            *target_y = ly + COMPANY_TEST_M30J_CANVAS_Y;
        }
    }
    return found == 1;
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
    wlr_log(WLR_INFO,
        "SD_CI_COMPANY_CROP_NATIVE peer_pid=%ld source_expected=840x540 "
        "visible=600x380 selection=CI_ONLY authority=NONE",
        (long)view->pid);
}

void stage_director_ci_company_crop_on_unmap(struct sway_view *view) {
    if (!view || view != ci_company_crop_actor) return;
    ci_company_crop_actor = NULL;
    ci_company_clip_frames = 0;
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
    if (!ci_exact_native_qt(view) ||
            view->geometry.width != COMPANY_TEST_SOURCE_WIDTH ||
            view->geometry.height != COMPANY_TEST_SOURCE_HEIGHT ||
            view->geometry.x != 0 || view->geometry.y != 0)
        return false;
    if (!ci_company_m30j_enabled()) return true;
    if (!view->container || !view->container->scene_tree)
        return false;
    int x = 0, y = 0, target_x = 0, target_y = 0;
    if (!ci_m30j_real_company_layer(&target_x, &target_y) ||
            !wlr_scene_node_coords(&view->container->scene_tree->node,
                &x, &y))
        return false;
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
            int parent_x = 0, parent_y = 0;
            if (!wlr_scene_node_coords(&root->layers.popup->node,
                    &parent_x, &parent_y))
                return false;
            wlr_scene_node_reparent(scene, root->layers.popup);
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
