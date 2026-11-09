# 🏥💜 M31b // SceneFX GL unpack row-layout production review proposal

HOSPITAL REPORT

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Company native MapLibre / renderer pixel-format safety
DOCTOR // Nurse 2, external Hospital-compatible successor
STAFF ROLE // NRS-2
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE, **not** confirmed live Hospital-connected
ASSIGNMENT // Prepare a safe SceneFX production correction proposal, backed by independent multi-format OpenGL regression coverage
PHASE // CI-only proposal and testing
STATE // DRAFT, UNMERGED; UPSTREAM OWNER REVIEW PENDING
REPORT KIND // DESIGN + IMPLEMENTATION / VERIFICATION
PERMISSIONS // Add isolated tests, CI and an owner-review document only; no upstream or production compositor modification
PRODUCTION AUTHORITY // NONE

## IMPLEMENTATION

### Evidence and immediate result

[M31a PR #120](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/120) is a **clean, unmerged draft**. Its [exact-head CI #42524698081](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/42524698081) passed 4/4 and directly demonstrated that the **1493×788 original SHM RGB565 buffer, 2986 bytes/row**, undergoes exactly **−1 pixel/row** diagonal shear with the existing pinned SceneFX but **0 pixel/row** with a separately overridden SceneFX that sets/restores `GL_UNPACK_ALIGNMENT=1`. The actual Qt native source, final compositor pixels, real selected wlroots root, pan/zoom, resize and close were checked. The corrected **1494px** reference still passed.

The pinned [SceneFX `render/fx_renderer/fx_texture.c` at `26200c5`](https://github.com/wlrfx/scenefx/blob/26200c5d9fa947a1d3b0935d3f9237229cd3e5ba/render/fx_renderer/fx_texture.c) has **two CPU upload paths**:

- `fx_texture_from_pixels()`: `glTexImage2D`, with `GL_UNPACK_ROW_LENGTH_EXT` but no explicit unpack alignment at the upload.
- `fx_texture_update_from_buffer()`: `glTexSubImage2D`, including row length and skip pixel/row offsets, also without explicit unpack alignment.

**M31b identifies a broader correctness issue:** the universal `alignment=1` experiment is right for 1493px RGB565 (stride 2986), but it is **not sufficient for every legal source row pitch**. Example: 5px RGB888 contains 15 bytes of pixels, yet a buffer may have a 16-byte physical row stride. With `GL_UNPACK_ROW_LENGTH=5`, alignment 1 advances **15** bytes per row, not the actual 16; alignment 2 or 4 advances 16. An unconditional alignment=1 in upstream SceneFX could therefore introduce a new distortion. Do **not** copy M31a's hardcoded alignment setting into production.

### Proposed production correction (review-only, not applied)

For each CPU-backed upload:

1. **Validate** `stride >= width × bytes_per_pixel`, with checked integer conversion and overflow. Keep existing DRM-format and one-pixel-per-block checks.
2. Choose `row_length = stride / bytes_per_pixel` only if it is a valid GL integer and no smaller than `width`.
3. Choose the *smallest* supported GL unpack alignment `A` from `{1, 2, 4, 8}` for which the actual pitch is reproduced exactly:

   `round_up(row_length × bytes_per_pixel, A) == stride`

4. If no alignment/row-length combination can represent the source pitch, **UNREPRESENTABLE: refuse this direct upload** or explicitly repack into an independently bounded tight staging buffer. Never silently truncate row length, invent padding, or read past the source. A repacking path is a separate design/permission decision.
5. Within the existing valid EGL context, save `GL_UNPACK_ALIGNMENT`, set the selected `A`, set the computed `GL_UNPACK_ROW_LENGTH_EXT`, perform `glTexImage2D` or `glTexSubImage2D`, reset row-length/skip fields per existing SceneFX contract and restore the original alignment on every successful path.
6. Ensure any new failure branch occurs **before** texture creation/context acquisition where possible; preserve the upstream buffer-access cleanup and error pathways. Never leave GL context state changed.

Illustrative C helper contract for owner review (this is **not** applied to upstream):

```c
/* bytes_per_pixel comes only from a validated one-pixel DRM block. */
static bool choose_unpack_row(size_t stride, uint32_t width,
        size_t bytes_per_pixel, GLint *length, GLint *alignment) {
    if (!bytes_per_pixel || !width || !length || !alignment ||
            width > SIZE_MAX / bytes_per_pixel ||
            stride < (size_t)width * bytes_per_pixel) {
        return false;
    }
    size_t pixels = stride / bytes_per_pixel;
    if (pixels > INT_MAX || pixels < width) {
        return false;
    }
    size_t raw = pixels * bytes_per_pixel;
    for (GLint a = 1; a <= 8; a *= 2) {
        if (raw <= SIZE_MAX - (size_t)(a - 1) &&
                ((raw + (size_t)(a - 1)) & ~(size_t)(a - 1)) == stride) {
            *length = (GLint)pixels;
            *alignment = a;
            return true;
        }
    }
    return false;
}
```

Owner must check integration with SceneFX supported packed-block formats, lifetime of `wlr_buffer_begin_data_ptr_access`, GL error/reporting semantics and all texture creation/update exit paths. The helper rejects unrepresentable strides; it does not implicitly authorize copying buffers or changing an upstream renderer design.

### M31b regression scope

This branch adds only:

- `tests/stage_director/ci_company_m31b_gles_pixelstore_matrix.c`: standalone **real GLES3** texture upload + FBO readback matrix for **RGB565, RGBA4444, RGBA5551, RGB888, RGBA8888**, widths **3, 4, 5, 7, 8, 13**, and **three physical source row layouts**: tight, one complete extra pixel, and 4-byte-aligned byte padding. It tests `glTexImage2D` creation, `glTexSubImage2D` with skip-pixel/skip-row subrect updates, GL unpack state restoration after starting from alignments 4 and 8, and exact pixel identity. **90 independent matrix configurations**, each testing both upload routes. It deliberately reproduces the *old* 4-byte default mismatch on tight RGB565 and RGB888 and demonstrates that *blind* 1-byte alignment corrupts padded RGB888. Unrepresentable layouts fail.
- `tests/stage_director/ci_company_m31b_gles_pixelstore_matrix.nix`: pinned Nixpkgs compilation of the standalone GLES program. No SceneFX source is changed.
- `.github/workflows/stage-director-company-m31b.yml`: source/authority gate and separate headless Mesa EGL/GLES3 test, with retained log and explicit refusal if no real graphics context is available.

Existing M31a three-session 1493px negative-control/positive-patch experiment is left unchanged and remains the accepted **specific-case correction proof**. M31b adds regression evidence needed before an upstream/production design.

## HOW TO USE

Open the M31b draft PR. Review both `review-scope-and-row-stride-contract` and `real-pinned-mesa-gles-pixelstore-matrix` checks. A success must include the artifact `sd-company-m31b-real-gles-row-stride-matrix` with lines proving all **90 actual GLES3 creation/update cases**, the old-default failure and the RGB888 blind-alignment=1 failure.

No menu, code activation, application installation, Quickshell reload or Fedora command is required.

## VERIFY

- M31a previously verified the *original distorted* 1493px frame and exact **0px** patched result in real SwayPX compositor.
- M31b **must** pass actual Mesa GLES3 pixel identity for 5 formats × 6 widths × 3 stride layouts, independently of M31a's happy path.
- A failure must preserve the specific unsupported format/layout; never pass a synthetic-only result as real GL execution.
- The source gate must verify SceneFX's pinned commit, default SwayPX production source untouched, matrix under `tests/`, no live host change.

The separate review is incomplete until exact-head M31b CI succeeds, and **UPSTREAM OWNER approval remains separate even after a green check**.

## WATCH OUT FOR

- The proposed selector is limited to strides expressible through GL row-length in pixels and GL unpack alignment {1,2,4,8}. It explicitly rejects `RGB888 width=5 stride=17`, which cannot be represented by those settings without repacking.
- GLES3 + Mesa software CI proves buffer upload semantics on this pinned driver, **not** NVIDIA/AMD/Intel production, DMA-BUF/GPU-only buffers, different GLES extension support, fractional scaling or compositor selected-host/focus authority.
- A valid generalization may require a staging copy for unrepresentable strides; no permission to introduce new buffering cost or memory management has been granted.
- Do not overwrite upstream SceneFX `wlrfx/scenefx` or switch the production SwayPX `flake.lock` without owner review and operator approval. SceneFX owns texture-upload code, SwayPX owns integration, Social owns shared production compositor policy, Company owns Company map UI.
- This is a **review proposal**, not an upstream PR, production patch, signed package, or verified live host installation.

## CHECKLIST

- [x] Reconfirm active M31a draft, exact-head native tests and Syntagma permissions
- [x] Locate pinned upstream SceneFX upload paths, current upstream main and live open PRs
- [x] Identify why the M31a hardcoded 1-byte alignment does not cover all row strides
- [x] Propose format-aware, measured-pitch row-layout selection and explicit rejection
- [x] Implement standalone real GLES3 create/update regression matrix and Nix CI
- [ ] Pass exact-head M31b source and actual Mesa GPU-software tests
- [ ] Have the responsible SceneFX/SwayPX code owners review the proposed generic contract
- [ ] Decide staging-copy handling for inherently unrepresentable source strides
- [ ] Prepare explicitly authorized upstream/production code change after review
- [ ] Perform user-authorized Fedora physical GPU/multi-output/focus/clip/input verification
- [ ] Obtain operator's final visual acceptance

## NEXT

Run M31b CI, fix only test/CI defects and preserve evidence. Once green, hand the generic row-layout contract and negative-control proof to the SceneFX/SwayPX owners for an **independent code review**. If approved, prepare an actual production patch in its owning repository/branch and test before any package activation.

## DECISIONS

M31b is **UNMERGED**, review-only, and does not grant production authority, selected-host custody, scene clipping or input. The user is the final approver. No live Hospital registration, local system mutation or final DONE is claimed. Preserve M31a [PR #120](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/120), exact verified commit `d10d26e48c10b8484537eb026f950ec241f1fad9`, and [native CI #42524698081](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/42524698081) for recovery.
