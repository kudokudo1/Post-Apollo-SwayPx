#!/usr/bin/env python3
"""Instrument ONLY the pinned SceneFX source inside the CI Nix derivation.

Never imported by production SwayPX. Fail on any upstream source mismatch.
Logs computed scene damage/visible and GLES quad bounds for one oversized
native test surface with a deliberate 320x240 source/destination crop.
"""
from pathlib import Path


def inject(path, anchor, replacement):
    file = Path(path)
    text = file.read_text(encoding="utf-8")
    if text.count(anchor) != 1:
        raise SystemExit(f"FAIL: pinned SceneFX render hook changed: {path!s} ({text.count(anchor)})")
    file.write_text(text.replace(anchor, replacement), encoding="utf-8")


scene_anchor = "\t\tstruct fx_render_texture_options tex_options = {"
scene_trace = r'''
        if (scene_buffer->buffer &&
                scene_buffer->buffer->width > 1000 &&
                scene_buffer->src_box.width >= 319.0 &&
                scene_buffer->src_box.width <= 321.0 &&
                scene_buffer->src_box.height >= 239.0 &&
                scene_buffer->src_box.height <= 241.0) {
            const pixman_box32_t *visible =
                pixman_region32_extents(&node->visible);
            const pixman_box32_t *clip =
                pixman_region32_extents(&render_region);
            const pixman_box32_t *damage =
                pixman_region32_extents(&data->damage);
            wlr_log(WLR_INFO,
                "SD_CI_FX_SCENE_TEX src=%.0f,%.0f %.0fx%.0f "
                "dst=%d,%d %dx%d visible=%d,%d:%d,%d "
                "clip=%d,%d:%d,%d damage=%d,%d:%d,%d "
                "native=%dx%d",
                scene_buffer->src_box.x, scene_buffer->src_box.y,
                scene_buffer->src_box.width, scene_buffer->src_box.height,
                dst_box.x, dst_box.y, dst_box.width, dst_box.height,
                visible->x1, visible->y1, visible->x2, visible->y2,
                clip->x1, clip->y1, clip->x2, clip->y2,
                damage->x1, damage->y1, damage->x2, damage->y2,
                scene_buffer->buffer->width, scene_buffer->buffer->height);
        }
'''
inject(
    "types/scene/wlr_scene.c",
    scene_anchor,
    scene_trace + "\n" + scene_anchor,
)

pass_anchor = "\trender(&dst_box, &clip_region, shader->pos_attrib);"
pass_trace = r'''
    if (options->texture->width > 1000 &&
            dst_box.width == 320 && dst_box.height == 240) {
        pixman_region32_t submitted;
        pixman_region32_init_rect(&submitted, dst_box.x, dst_box.y,
            dst_box.width, dst_box.height);
        pixman_region32_intersect(&submitted, &submitted, &clip_region);
        const pixman_box32_t *clip = pixman_region32_extents(&clip_region);
        const pixman_box32_t *draw = pixman_region32_extents(&submitted);
        int rects = 0;
        (void)pixman_region32_rectangles(&submitted, &rects);
        wlr_log(WLR_INFO,
            "SD_CI_FX_GL_QUADS dst=%d,%d %dx%d "
            "clip=%d,%d:%d,%d draw=%d,%d:%d,%d "
            "rects=%d texture=%dx%d alpha=%.3f",
            dst_box.x, dst_box.y, dst_box.width, dst_box.height,
            clip->x1, clip->y1, clip->x2, clip->y2,
            draw->x1, draw->y1, draw->x2, draw->y2,
            rects, options->texture->width, options->texture->height,
            alpha);
        pixman_region32_fini(&submitted);
    }
'''
inject(
    "render/fx_renderer/fx_pass.c",
    pass_anchor,
    pass_trace + "\n" + pass_anchor,
)
print("SD_CI_FX_SOURCE_INSTRUMENTED pinned scene/GLES hooks: PASS")
