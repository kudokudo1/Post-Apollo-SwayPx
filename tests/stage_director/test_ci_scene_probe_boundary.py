#!/usr/bin/env python3
"""Reject accidental production activation of SD1 native scene probe."""
from pathlib import Path
root = Path(__file__).resolve().parents[2]
read = lambda name: (root / name).read_text(encoding="utf-8")

options = read("meson_options.txt")
top = read("meson.build")
meson = read("sway/meson.build")
observer = read("sway/stage_director.c")
xdg = read("sway/desktop/xdg_shell.c")
probe = read("sway/stage_director_ci_scene_probe.c")
test = read("tests/stage_director/ci_native_news_actor_smoke.sh")
workflow = read(".github/workflows/stage-director-headless-runtime.yml")

assert "option('sd-ci-news-scene-probe', type: 'boolean', value: false" in options
assert "if get_option('sd-ci-news-scene-probe')" in top
assert "if get_option('sd-ci-news-scene-probe')" in meson
assert "sway_sources += 'stage_director_ci_scene_probe.c'" in meson
assert "-DSD_STAGE_DIRECTOR_CI_SCENE_PROBE=1" in top
assert observer.count("#ifdef SD_STAGE_DIRECTOR_CI_SCENE_PROBE") >= 2
assert xdg.count("#ifdef SD_STAGE_DIRECTOR_CI_SCENE_PROBE") >= 2
assert "stage_director_ci_scene_probe(view);" in xdg
assert "stage_director_ci_scene_probe(container->view);" in observer
assert 'strcmp(ci, "true")' in probe
assert 'strcmp(enabled, "1")' in probe
assert 'strcmp(title, "SD1-CI-DISPOSABLE-NOT-BROWSER")' in probe
assert "wlr_scene_subsurface_tree_set_clip(" in probe
# The post-layout gate now samples both outside edges independently. Keep
# the exact count locked so added hit tests need deliberate CI review.
assert probe.count("wlr_scene_node_at(") == 8
assert "outside_right = wlr_scene_node_at(" in probe
assert "outside_bottom = wlr_scene_node_at(" in probe
assert "outside_right ? 1 : 0, outside_bottom ? 1 : 0" in probe
assert "view_center_and_clip_surface(view);" in probe
assert "normal_clip_restored=%d" in probe
assert "normal_clip_restored=1" in test
assert "normal_clip_restored=0" in test
assert "STAGE_DIRECTOR_CI_SCENE_PROBE=1" in workflow
assert "-Dsd-ci-news-scene-probe=true" in workflow
assert "after_inside=1 after_outside=0 normal_clip_restored=1 result=PASS" in test
for forbidden in ("stage_director_viewport_clip_view(", "stage_director_news_custody_capture(",
                  "stage_director_news_supervisor_start_test_child(",
                  "wlr_seat_pointer_notify_enter(", "sway_seat_set_focus(",
                  "posix_spawn(", "execve(", "fork("):
    assert forbidden not in probe, forbidden
print("Stage Director test-only native clip probe cannot activate in standard build: PASS")
