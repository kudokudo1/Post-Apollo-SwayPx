#!/usr/bin/env python3
"""CI render-pixel test cannot accidentally become production NEWS cropping."""
from pathlib import Path
root = Path(__file__).resolve().parents[2]
def read(name):
    return (root / name).read_text(encoding="utf-8")

options = read("meson_options.txt")
probe = read("sway/stage_director_ci_scene_probe.c")
header = read("include/sway/stage_director_ci_scene_probe.h")
observer = read("sway/stage_director.c")
transaction = read("sway/desktop/transaction.c")
workflow = read(".github/workflows/stage-director-headless-runtime.yml")
actor = read("tests/stage_director/ci_native_news_actor_smoke.sh")
pixel = read("tests/stage_director/assert_news_render_pixels.py")
assert "option('sd-ci-news-scene-probe', type: 'boolean', value: false" in options
assert "STAGE_DIRECTOR_CI_RENDER_PROBE" in probe
assert "ci_render_actor" in probe
# CI-only held pidfd must use exact native peer tuple and existing custody
# API, without becoming part of the production SwayPX build.
pidfd = read("tests/stage_director/ci_native_news_pidfd_witness.c")
sway_meson = read("sway/meson.build")
assert "ci_native_news_pidfd_witness.c" in workflow
assert 'sway/stage_director_news_custody.c' in workflow
assert "STAGE_DIRECTOR_CI_PIDFD_WITNESS=1" in workflow
assert "STAGE_DIRECTOR_TEST_PIDFD_WITNESS=" in workflow
assert "ci_native_news_pidfd_witness.c" not in sway_meson
assert "stage_director_news_custody_reserve(" in pidfd
assert "stage_director_news_custody_capture(" in pidfd
assert "stage_director_news_custody_inspect(" in pidfd
assert "stage_director_news_custody_release(" in pidfd
assert "SD_CI_PIDFD_RELEASE process_exit_observed=1" in pidfd
assert "authority=NONE" in pidfd
assert 'strcmp(github, "true")' in pidfd
assert "if (!exited ||" in pidfd
for unsafe in ("posix_spawn(", "fork(", "execve(", "system(", "kill(",
               "wlr_scene_buffer_set_dest_size("):
    assert unsafe not in pidfd, unsafe
assert 'STAGE_DIRECTOR_CI_PIDFD_WITNESS' in actor
assert 'read -r peer_pid peer_uid peer_start' in actor
assert 'kill -TERM "$actor"' in actor
assert 'wait "$pidfd_witness"' in actor
# CI-only real XDG peer evidence must come from the native wl_client, and
# fail if it disagrees with Sway's stored PID or the disposable actor PID.
assert "wl_resource_get_client(view->surface->resource)" in probe
assert "wl_client_get_credentials(client, &peer_pid, &peer_uid, &peer_gid)" in probe
assert "stage_director_read_linux_process(" in probe
assert "peer_pid == view->pid" in probe
assert "SD_CI_WL_PEER pid=" in probe
assert "SD_CI_WL_PEER_CHECK source=actual_wl_client" in actor
assert 'expected = int(sys.argv[1])' in actor
assert 'int(pid) == expected == int(view_pid)' in actor
assert "ci_render_clip" in probe
assert "stage_director_ci_scene_probe_restore(" in header
assert "stage_director_ci_scene_probe_restore(view);" in observer
assert "stage_director_ci_scene_probe_post_layout(" in header
assert "stage_director_ci_scene_probe_post_layout();" in transaction
assert "arrange_root(root);" in transaction
assert "SD_CI_POST_LAYOUT clip_reapplied=1" in probe
assert "STAGE_DIRECTOR_CI_RENDER_NORMALIZE" in probe
assert "wlr_scene_buffer_set_dest_size(" in probe
assert "SD_CI_BUFFER_METRICS at=" in probe
output_source = read("sway/desktop/output.c")
probe_header = read("include/sway/stage_director_ci_scene_probe.h")
assert "STAGE_DIRECTOR_CI_FRAME_DEST_OVERRIDE=1" in workflow
assert "SD_CI_FRAME_DEST_PRESERVED dst=320x240" in actor
assert "stage_director_ci_scene_probe_preserve_dest(" in output_source
assert "stage_director_ci_scene_probe_preserve_dest(" in probe
assert "stage_director_ci_scene_probe_preserve_dest(" in probe_header
assert "if (!stage_director_ci_scene_probe_preserve_dest(buffer))" in output_source
assert output_source.count("#ifdef SD_STAGE_DIRECTOR_CI_SCENE_PROBE") >= 2
assert 'strcmp(ci, "true")' in probe
assert 'strcmp(enabled, "1")' in probe
assert 'probe.found || ci_render_clip.width != 320' in probe
assert "ci_render_actor->content_tree" in probe
# Renderer telemetry is produced only by a separate, pinned CI derivation.
trace_nix = read("tests/stage_director/ci_scenefx_render_trace.nix")
trace_patch = read("tests/stage_director/ci_scenefx_render_trace.py")
assert "ci_scenefx_render_trace.nix" in workflow
assert "STAGE_DIRECTOR_CI_FX_PASS_TRACE=1" in workflow
assert "SD_CI_FX_SCENE_TEX" in trace_patch
assert "SD_CI_FX_GL_QUADS" in trace_patch
assert "scene_anchor" in trace_patch and "pass_anchor" in trace_patch
assert "pinned SceneFX render hook changed" in trace_patch
assert "ordinarySceneFX.overrideAttrs" in trace_nix
assert "postPatch" in trace_nix and "ci_scenefx_render_trace.py" in trace_nix
assert "STAGE_DIRECTOR_CI_FX_PASS_TRACE" in actor
assert "FAIL: pinned SceneFX actual scene draw region not measured" in actor
assert "FAIL: pinned SceneFX submitted GLES texture region not measured" in actor
assert "struct ci_paint_audit" in probe
assert "struct ci_draw_audit" in probe
assert "SD_CI_DRAW_NODE kind=rect" in probe
assert "SD_CI_DRAW_NODE kind=buffer" in probe
assert "SD_CI_DRAW_AUDIT nodes=" in probe
assert "node == audit->output_handler" in probe
assert "SD_CI_DRAW_AUDIT nodes=[1-9]" in actor
assert "SD_CI_PAINT_BUFFER scope=" in probe
assert "SD_CI_PAINT_AUDIT content=" in probe
assert 'scope = "container"' in probe
assert 'scope = "content"' in probe
assert "ci_audit_paint_buffer" in probe
assert "SD_CI_PAINT_AUDIT content=[1-9]" in actor
assert "after_dst=%dx%d" in probe
assert "SD_CI_BUFFER_METRICS .*after_dst=320x240" in actor
assert "ci_restore_root_buffer" in probe
assert "STAGE_DIRECTOR_CI_RENDER_NORMALIZE=1" in workflow
assert "wlr_scene_subsurface_tree_set_clip(" in probe
assert "view_center_and_clip_surface(view);" in probe
assert 'strcmp(ci, "true")' in probe
assert 'strcmp(title, "SD1-CI-DISPOSABLE-NOT-BROWSER")' in probe
assert "STAGE_DIRECTOR_CI_PIXEL_TEST=1" in workflow
assert "STAGE_DIRECTOR_TEST_GRIM=" in workflow
assert "STAGE_DIRECTOR_CI_RENDER_PROBE=1" in workflow
assert "actions/upload-artifact@v4" in workflow
assert "[colors-dark]" in actor and "background=ffffff" in actor
assert "-t ppm" in actor and "assert_news_render_pixels.py" in actor
assert "inside_luma" in pixel and "outside_luma" in pixel
assert "max(abs(v - desktop) for v in outside_values)" in pixel
# Four-sided real screenshot regression proof; no simple top-left-only proxy.
for edge in ("(x + 24, y + 160)", "(x + 160, y + 24)",
             "(x + 450, y + 150)", "(x + 160, y + 370)"):
    assert edge in pixel, edge
assert "Stage Director actual compositor render PIXELS confined" in pixel
assert "STAGE_DIRECTOR_CI_OUTPUT_REALLOC=0" in workflow
assert "compare_news_reallocated_pixels.py" in workflow
assert "compare_news_reallocated_pixels.py" in actor
assert "news-clip-reallocated.ppm" in actor
assert "stage-director-render-evidence*.png" in workflow
assert 'output * resolution 1918x1078' in actor
assert 'original_result' in actor and 'fresh_result' in actor
compare = read("tests/stage_director/compare_news_reallocated_pixels.py")
assert 'original["size"] != (1920, 1080)' in compare
assert 'resized["size"] != (1918, 1078)' in compare
assert "OVERFLOW_SURVIVES_FRESH_OUTPUT" in compare
assert "STAGE_DIRECTOR_CI_COLD_STAGE=initial" in workflow
assert "STAGE_DIRECTOR_CI_COLD_STAGE=fresh" in workflow
assert "STAGE_DIRECTOR_CI_COLD_SIZE=1888x1056" in workflow
assert "compare_news_cold_pixels.py" in workflow
assert "stage-director-cold-$stage.ppm" in actor
assert 'STAGE_DIRECTOR_CI_COLD_STAGE' in actor
cold = read("tests/stage_director/compare_news_cold_pixels.py")
assert "BOTH_COLD_OUTPUTS_LEAK" in cold
assert "STAGE_DIRECTOR_CI_MOVE_PIXEL=1" in workflow
assert "compare_news_moved_pixels.py" in workflow
assert "stage-director-moved.ppm" in workflow
assert "moved_pixel_result" in workflow
assert "SD_CI_MOVE_START target=520,180" in actor
assert "STAGE_DIRECTOR_CI_OPACITY_PIXEL=1" in workflow
assert "STAGE_DIRECTOR_CI_BACKGROUND_REFRESH=1" in workflow
assert "compare_news_background_pixels.py" in workflow
assert "background_pixel_result" in workflow
assert "SD_CI_BACKGROUND_REFRESH mapped_actor=1 desktop_restored=1" in actor
assert "stage-director-render-evidence-background-refreshed.png" in actor
background = read("tests/stage_director/compare_news_background_pixels.py")
assert "LEAK_PERSISTS_AFTER_BACKGROUND_REFRESH" in background
assert "LEAK_CLEARED_AFTER_BACKGROUND_REFRESH" in background
assert "first_id != second_id" in background
assert "compare_news_opacity_pixels.py" in workflow
assert "restored_pixel_result" in workflow
assert "SD_CI_OPACITY_START alpha=0 actor_still_mapped=1" in actor
assert "SD_CI_OPACITY_CAPTURE alpha=0 actor_still_mapped=1" in actor
assert "SD_CI_OPACITY_CAPTURE alpha=1 actor_still_mapped=1" in actor
assert "stage-director-render-evidence-opacity-zero.png" in actor
assert "stage-director-render-evidence-opacity-one.png" in actor
opacity = read("tests/stage_director/compare_news_opacity_pixels.py")
assert "ACTOR_PIXELS_CLEAR_ON_ALPHA_ZERO" in opacity
assert "ACTOR_PIXELS_PERSIST_AT_ALPHA_ZERO" in opacity
assert "len(set(ids)) != 1" in opacity
assert "SD_CI_MOVE_CAPTURE actor_still_mapped=1" in actor
assert "stage-director-render-evidence-moved.png" in actor
moved = read("tests/stage_director/compare_news_moved_pixels.py")
assert "LEAK_FOLLOWS_MAPPED_ACTOR" in moved
assert "MOVED_LEAK_WITH_OLD_RESIDUAL" in moved
assert "before_id != after_id" in moved
assert "old_x + 8 >= x1" in moved
assert 'first["size"] != (1920, 1080)' in cold
assert 'fresh["size"] != (1888, 1056)' in cold
headless = read("tests/stage_director/headless_sway_smoke.sh")
assert 'SD_CI_COLD_BOOT virtual_output=' in headless
assert 'BLOCKED_COLD_OUTPUT_OUTSIDE_CI' in headless
for forbidden in ("stage_director_viewport_clip_view(",
                  "stage_director_news_custody_capture(",
                  "posix_spawn(", "fork(", "execve(",
                  "wlr_seat_pointer_notify_enter(",
                  "sway_seat_set_focus("):
    assert forbidden not in probe
print("Stage Director CI-only render-pixel test stays disconnected from production: PASS")
