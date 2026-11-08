#!/usr/bin/env python3
"""M31a: fail-closed patch of one Nix-pinned SceneFX SOURCE COPY for CI only.

Production SceneFX, flake.lock, SwayPX Stage Director and Social are unchanged.
The two original OpenGL pixel upload paths use GL_UNPACK_ROW_LENGTH_EXT
but inherit GL_UNPACK_ALIGNMENT (normally 4). For packed RGB565 width 1493
the physical 2986-byte row then advances as though it were 2988 bytes,
matching M30z's exactly -1 pixel-per-row slant. This test patch saves the
prior pixel-store alignment, sets 1 for actual CPU uploads, then restores it.

This *does not* make a production change or claim universal driver behavior.
"""
from __future__ import annotations

import sys
from pathlib import Path

MARKER = "M31A_SCENEFX_GL_UNPACK_ALIGNMENT_CI_ONLY"


def replace_once(src: str, old: str, new: str, label: str) -> str:
    if src.count(old) != 1:
        raise RuntimeError("M31A_PINNED_SCENEFX_SOURCE_DRIFT_" + label)
    return src.replace(old, new, 1)


def patch(src: str) -> str:
    if MARKER in src:
        raise RuntimeError("M31A_PATCH_ALREADY_APPLIED")
    # Make the two function scopes explicit; don't silently patch an
    # unrelated glTexImage2D or texture-update path after upstream changes.
    a = "static bool fx_texture_update_from_buffer("
    b = "static struct wlr_texture *fx_texture_from_pixels("
    c = "static struct wlr_texture *fx_texture_from_dmabuf("
    if src.count(a) != 1 or src.count(b) != 1 or src.count(c) != 1:
        raise RuntimeError("M31A_PINNED_SCENEFX_FUNCTION_LAYOUT_CHANGED")
    start_update = src.index(a)
    start_create = src.index(b)
    start_dma = src.index(c)
    if not start_update < start_create < start_dma:
        raise RuntimeError("M31A_PINNED_SCENEFX_FUNCTION_ORDER_CHANGED")
    update_start = src[:start_update]
    update = src[start_update:start_create]
    create = src[start_create:start_dma]
    tail = src[start_dma:]

    update = replace_once(
        update,
        "\tglBindTexture(GL_TEXTURE_2D, texture->tex);\n\n\tint rects_len = 0;",
        "\tglBindTexture(GL_TEXTURE_2D, texture->tex);\n\n"
        "\tGLint m31a_prior_unpack_alignment = 0;\n"
        "\tglGetIntegerv(GL_UNPACK_ALIGNMENT, &m31a_prior_unpack_alignment);\n"
        "\tglPixelStorei(GL_UNPACK_ALIGNMENT, 1);\n\n"
        "\tint rects_len = 0;",
        "UPDATE_BEGIN")
    update = replace_once(
        update,
        "\tglPixelStorei(GL_UNPACK_SKIP_ROWS_EXT, 0);\n\n"
        "\tglBindTexture(GL_TEXTURE_2D, 0);",
        "\tglPixelStorei(GL_UNPACK_SKIP_ROWS_EXT, 0);\n"
        "\tglPixelStorei(GL_UNPACK_ALIGNMENT, m31a_prior_unpack_alignment);\n\n"
        "\tglBindTexture(GL_TEXTURE_2D, 0);",
        "UPDATE_RESTORE")
    create = replace_once(
        create,
        "\tglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);\n"
        "\tglPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, stride / drm_fmt->bytes_per_block);",
        "\tglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);\n"
        "\tGLint m31a_prior_unpack_alignment = 0;\n"
        "\tglGetIntegerv(GL_UNPACK_ALIGNMENT, &m31a_prior_unpack_alignment);\n"
        "\tglPixelStorei(GL_UNPACK_ALIGNMENT, 1);\n"
        "\tif (drm_format == DRM_FORMAT_RGB565 && width == 1493) {\n"
        "\t\tstatic bool m31a_logged = false;\n"
        "\t\tif (!m31a_logged) {\n"
        f'\t\t\twlr_log(WLR_INFO, "{MARKER} width=1493 packed_pitch=2986 "
'
        '\t\t\t\t"production_authority=OFF");\n'
        "\t\t\tm31a_logged = true;\n"
        "\t\t}\n"
        "\t}\n"
        "\tglPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, stride / drm_fmt->bytes_per_block);",
        "CREATE_BEGIN")
    create = replace_once(
        create,
        "\tglPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, 0);\n\n"
        "\tglBindTexture(GL_TEXTURE_2D, 0);",
        "\tglPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, 0);\n"
        "\tglPixelStorei(GL_UNPACK_ALIGNMENT, m31a_prior_unpack_alignment);\n\n"
        "\tglBindTexture(GL_TEXTURE_2D, 0);",
        "CREATE_RESTORE")
    result = update_start + update + create + tail
    if (result.count("glGetIntegerv(GL_UNPACK_ALIGNMENT, "
                     "&m31a_prior_unpack_alignment)") != 2 or
            result.count("glPixelStorei(GL_UNPACK_ALIGNMENT, 1)") != 2 or
            result.count("glPixelStorei(GL_UNPACK_ALIGNMENT, "
                         "m31a_prior_unpack_alignment)") != 2 or
            result.count(MARKER) != 1):
        raise RuntimeError("M31A_PATCH_INCOMPLETE_OR_DUPLICATED")
    return result


def selftest():
    # Exercise the four strict source anchors independently.
    orig = (
        "static bool fx_texture_update_from_buffer(\n"
        "\tglBindTexture(GL_TEXTURE_2D, texture->tex);\n\n"
        "\tint rects_len = 0;\n"
        "\tglPixelStorei(GL_UNPACK_SKIP_ROWS_EXT, 0);\n\n"
        "\tglBindTexture(GL_TEXTURE_2D, 0);\n"
        "static struct wlr_texture *fx_texture_from_pixels(\n"
        "\tglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);\n"
        "\tglPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, stride / drm_fmt->bytes_per_block);\n"
        "\tglPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, 0);\n\n"
        "\tglBindTexture(GL_TEXTURE_2D, 0);\n"
        "static struct wlr_texture *fx_texture_from_dmabuf(\n"
    )
    updated = patch(orig)
    if (updated.count("glPixelStorei(GL_UNPACK_ALIGNMENT, 1)") != 2 or
            updated.count("glPixelStorei(GL_UNPACK_ALIGNMENT, "
                          "m31a_prior_unpack_alignment)") != 2):
        raise RuntimeError("M31A_ALIGNMENT_SAVE_RESTORE_MISSING")
    for missing in ("GL_UNPACK_ROW_LENGTH_EXT", "GL_TEXTURE_WRAP_T",
                    "GL_UNPACK_SKIP_ROWS_EXT"):
        try:
            patch(orig.replace(missing, "REMOVED"))
        except RuntimeError as e:
            if "SOURCE_DRIFT" not in str(e):
                raise
        else:
            raise RuntimeError("M31A_FALSE_ACCEPT_MISSING_SOURCE_ANCHOR")
    try:
        patch(updated)
    except RuntimeError as e:
        if "ALREADY_APPLIED" not in str(e):
            raise
    else:
        raise RuntimeError("M31A_PATCH_ALLOWED_REPLAY")
    print("SD_COMPANY_M31A_PATCH_SELFTEST patch_both_cpu_paths=1 "
          "prior_GL_state_restored=1 strict_anchor_refusal=1 PASS", flush=True)


def main(args: list[str]) -> int:
    if args == ["--selftest"]:
        selftest()
        return 0
    if len(args) == 1:
        target = Path(args[0])
        if target.is_symlink() or not target.is_file():
            raise RuntimeError("M31A_ONLY_REGULAR_CI_SOURCE_COPY")
        if str(target).replace("\\", "/").endswith(
                "render/fx_renderer/fx_texture.c") is False:
            raise RuntimeError("M31A_ONLY_PINNED_TEXTURE_MODULE")
        src = target.read_text()
        target.write_text(patch(src))
        print("SD_COMPANY_M31A_PATCHED_PRIVATE_SCENEFX "
              "create_and_update=1 GL_UNPACK_ALIGNMENT=1 "
              "prior_pixel_store_restored=1 production=OFF PASS", flush=True)
        return 0
    raise RuntimeError("M31A_EXPECT_SELFTEST_OR_ONE_PINNED_SOURCE_PATH")


if __name__ == "__main__":
    try:
        sys.exit(main(sys.argv[1:]))
    except (RuntimeError, OSError, UnicodeDecodeError) as exc:
        print("SD_COMPANY_M31A_PATCH_REFUSED", exc, file=sys.stderr)
        sys.exit(1)
