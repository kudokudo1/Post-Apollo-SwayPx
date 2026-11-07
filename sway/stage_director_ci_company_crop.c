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

#define COMPANY_TEST_CROP_WIDTH 600
#define COMPANY_TEST_CROP_HEIGHT 380
#define COMPANY_TEST_SOURCE_WIDTH 840
#define COMPANY_TEST_SOURCE_HEIGHT 540

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
        COMPANY_TEST_CROP_WIDTH, COMPANY_TEST_CROP_HEIGHT);
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
        .width = COMPANY_TEST_CROP_WIDTH,
        .height = COMPANY_TEST_CROP_HEIGHT,
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
    return ci_exact_native_qt(view) &&
        view->geometry.width == COMPANY_TEST_SOURCE_WIDTH &&
        view->geometry.height == COMPANY_TEST_SOURCE_HEIGHT &&
        view->geometry.x == 0 && view->geometry.y == 0;
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
    }
}
