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
workflow = read(".github/workflows/stage-director-headless-runtime.yml")
actor = read("tests/stage_director/ci_native_news_actor_smoke.sh")
pixel = read("tests/stage_director/assert_news_render_pixels.py")
assert "option('sd-ci-news-scene-probe', type: 'boolean', value: false" in options
assert "STAGE_DIRECTOR_CI_RENDER_PROBE" in probe
assert "ci_render_actor" in probe
assert "ci_render_clip" in probe
assert "stage_director_ci_scene_probe_restore(" in header
assert "stage_director_ci_scene_probe_restore(view);" in observer
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
assert "Stage Director actual compositor render PIXELS confined" in pixel
for forbidden in ("stage_director_viewport_clip_view(",
                  "stage_director_news_custody_capture(",
                  "posix_spawn(", "fork(", "execve(",
                  "wlr_seat_pointer_notify_enter(",
                  "sway_seat_set_focus("):
    assert forbidden not in probe
print("Stage Director CI-only render-pixel test stays disconnected from production: PASS")
