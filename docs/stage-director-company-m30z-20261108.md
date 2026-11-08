# 🏥💜 SwayPX Company M30z // Original vs corrected MapLibre pixel origin

HOSPITAL REPORT

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Company native MapLibre pixel fidelity / rendered-frame localization
DOCTOR // Nurse 2, external Hospital-compatible successor
STAFF ROLE // NRS-2
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE (no live Hospital registration verified)
ASSIGNMENT // Compare actual original and corrected Qt source/compositor fiducials and locate visual distortion without authorizing production compositor surgery
PHASE // Disposable CI pixel-origin experiment technically verified
STATE // NATIVE CI VERIFIED; PRODUCTION OFF; OPERATOR ACCEPTANCE PENDING
REPORT KIND // IMPLEMENTATION CHECKPOINT
BASE // M30y draft PR #118 exact head `5def7be1270e8e8966862cf53082c617e6f5a988`
OWNERSHIP // SwayPX Company test-only fidelity and source-copy runner; no Social or production compositor actor ownership
PERMISSIONS // GitHub branch, draft PR, automated tests, no merge, no Fedora or live Quickshell changes

## IMPLEMENTATION

[Company M30x #1561](https://github.com/kudokudo1/The-Post-Apollo-Project/pull/1561) merged its correct separation of Qt CPU readback versus real wlroots native pixel format. [SwayPX M30w #117](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/117) remains draft; M30y [draft #118](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/118) directly measured both source sizes with 4/4 checks green at `a5854346e2`: original SHM RGB565 1493×788 at **2986 measured bytes/row**, corrected SHM RGB565 1494×788 at **2988 measured bytes/row**. Both have zero physical row padding. The corrected 1494px compositor image has zero vertical drift. The original source's pixel shear was previously reported but *not quantified with identical fiducials in that run*.

M30z adds a **disposable test-copy** fiducial source at the same exact QML coordinates for both the original 1493px and corrected 1494px source. It retains M30t's original corrected Qt readback and adds an independently guarded QQuickWindow::grabWindow() readback for the original width in the second, already isolated M30y compositor session. Each real Wayland screenshot is captured after its Qt source evidence is present.

- `tests/stage_director/ci_company_m30t_prepare.py` extends only the prepared test copy (under RUNNER_TEMP), not the owner-managed native MapLibre app, and keeps M30t's original code path unchanged when M30z is disabled.
- `tests/stage_director/ci_company_m30o_real_resize.py` archives an original native Qt PPM and compositor PPM after genuine selected actor and root-surface checks. M30y process, separate QML-copy path, and native resize/input/close gates remain.
- `tests/stage_director/ci_company_m30z_pixel_origin.py` measures green vertical/magenta horizontal fiducials in both sources and final frames using the earlier M30t geometry algorithm. It reports source drift, compositor drift, placement, missing markers, and whether the corrected reference remains straight; explicitly records no certified shear cause or production authority.
- `.github/workflows/stage-director-company-m30z.yml` runs source/grammar/selftest gates and separately booted corrected and original genuine Wayland/MapLibre sessions, paired real imported-pitch proof, and the final pixel-origin comparison. Artifacts retain both source/compositor PPMs, actual wlroots metadata, summaries, and diagnostics.

The original Company QML checkout remains immutable; the normal SwayPX binary must not contain the CI-only observer; Social's GDM/Vesktop pairing is not modified. No user photo, email, location or online tiles are touched.

## HOW TO USE

Review the draft PR's CI and `sd-company-m30z-original-vs-corrected-pixel-fidelity` artifact:

- `sd-company-m30z-original-native-fullscreen.ppm`: Qt source pixels at 1493×788.
- `sd-company-m30o-m30z-original-composited-fullscreen.ppm`: actual headless SwayPX output of the same test markers, 1920×1080.
- `sd-company-m30t-native-fullscreen.ppm`: previously passing corrected 1494×788 Qt source pixels, freshly captured in same workflow's first session.
- `sd-company-m30y-corrected-1494-fullscreen.ppm`: 1920×1080 corrected Wayland compositor pixels, saved before second session.
- `sd-company-m30z-real-pixel-origin.json`: computed geometry classifications and explicit limitations.
- M30w/M30y JSON imported native buffer details: original **2986**, corrected **2988** actual bytes/row.

Nothing to install or enable on the operator desktop.

## VERIFY

**Latest verified M30z checkpoint:** GitHub Actions [run #42522285743](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/42522285743) passed **4/4 workflows** for commit `3004e2c4f67dc69fdab04b267249da32f8a86a39`. Its [pixel and buffer artifact](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/42522285743/artifacts/13634032767) holds the actual original/corrected native Qt and compositor PPM frames, strict M30w/M30y imported buffer metadata, and the new `sd-company-m30z-real-pixel-origin.json`.

The original M30t *narrow* scan had marked the original compositor reference bars unmeasurable because they left the expected x window. The M30z **full-width wrapped-bar** scan now tracks both fiducials over **14 exact source rows** at 25-pixel increments, identifies which of the paired green bars wrapped across the 1493px edge, and fits their pixel displacement without presuming its slope:

| Buffer / capture | Actual row pitch | Horizontal shift per rendered row | Measured fit |
|---|---:|---:|---|
| Original 1493px real Qt source | 2986 bytes native imported buffer | 0 px | Straight |
| Original 1493px final compositor output | 2986 bytes native imported buffer | **−1.0 px/row** | **0.0px maximum residual across 14 samples**, wrap observed |
| Corrected 1494px Qt source | 2988 bytes native imported buffer | 0 px | Straight |
| Corrected 1494px final compositor output | 2988 bytes native imported buffer | 0 px | Straight |

Example original compositor green-bar first-column positions: source-left x305 changes to x135 at row170, x125 at row180, x100 at row205, x5 at row300, and **x1473 at row325** as it wraps around the 1493px canvas. This is a real pixel observation, not an extrapolation from the buffer format.

**Source-level mechanism found, not yet experimentally corrected:** the exact pinned [SceneFX commit `26200c5`, `render/fx_renderer/fx_texture.c`, lines 291–342](https://github.com/wlrfx/scenefx/blob/26200c5d9fa947a1d3b0935d3f9237229cd3e5ba/render/fx_renderer/fx_texture.c#L291-L342) obtains the actual buffer stride, sets `GL_UNPACK_ROW_LENGTH_EXT = stride / bytes_per_block`, then calls `glTexImage2D` without explicitly setting `GL_UNPACK_ALIGNMENT` in that upload path. Its [buffer update path at lines 25–89](https://github.com/wlrfx/scenefx/blob/26200c5d9fa947a1d3b0935d3f9237229cd3e5ba/render/fx_renderer/fx_texture.c#L25-L89) similarly does not explicitly set unpack alignment around `glTexSubImage2D`. Per the [Khronos OpenGL pixel storage reference](https://wikis.khronos.org/opengl/GLAPI/glPixelStore), the initial unpack alignment is 4 bytes: a 1493px RGB565 row naturally spans **2986 bytes**, but a 4-byte-aligned unpack calculation rounds to **2988 bytes**, which is **one 2-byte RGB565 pixel too long per row**. A 1494px RGB565 row already spans 2988 bytes and does not suffer that mismatch.

This matches the measured **−1 pixel/row with wrapping**, making unpack alignment the **leading code-level hypothesis**. No isolated alignment-corrected SceneFX build has yet been compared against the original control. Do not label the production fix or universal GPU behavior verified on this evidence alone.

Prior [M30y native CI #42519372425](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/42519372425) has passed. M30z must independently prove both sources contain **straight fiducial bars** before comparing with actual composited pixels; corrected final screenshot must also be straight. If original final bars are measurable and distorted, classify them as compositor-origin distortion; if unmeasurable, explicitly report unmeasurable, **not** a diagnosed shear. The experiment must not silently accept missing original Qt bars or unknown native pixel format/stride.

Run the existing native resize, real pointer pan/zoom, clip, fullscreen, restore, and clean-close gates in both independent sessions. SwayPX production compositor must have no M30z flag and no touched release files.

## WATCH OUT FOR

M30z can **localize** an original-source vs compositor-side difference but does not, alone, certify which function is at fault (upload pitch, shader sampling, source cropping, transform). Earlier M30t output showed the corrected view aligned; this is still a CI-only software Mesa reference, not Fedora physical GPU/multi-output validation.

Because both real imported buffers were tightly packed RGB565, a "padding fixed it" claim is unsupported. There is a 4-byte divisibility difference (`2986 mod 4 = 2`, `2988 mod 4 = 0`); its role in any renderer-upload path needs controlled testing before production design.

If the original composited fiducials cannot be measured, the report must state that explicitly rather than manufacturing a zero drift.

## CHECKLIST

- [x] Recheck active M30w/M30y Company state, Syntagma and ownership
- [x] Preserve M30y previous branch and base a fresh M30z branch
- [x] Prepare the same test-only original and corrected source reference bars
- [x] Add original Qt source capture and actual compositor screenshot
- [x] Add bounded source/compositor geometry classifier and negative selftests
- [x] Add independent GitHub native CI workflow and artifacts
- [x] Verify exact-head M30z workflow, full real MapLibre build, image measurements and regression tests (4/4 passed)
- [x] Classify compositor-origin original shearing at −1.0 pixel per row, with exact RGB565 imported pitches and wrap proof
- [ ] Build a separately pinned **CI-only SceneFX alignment control** without altering the production dependency or Social owner files
- [ ] Review SwayPX Company selected-host production identity/scene/input with owning team
- [ ] Operator-authorized local Fedora GPU/multi-output verification
- [ ] Explicit operator merge and final visual acceptance

## NEXT

CI quantified the original source as straight and compositor as exactly −1px per row with wrap. The next bounded experiment should **build a separately overridden SceneFX in isolated CI only**, retaining the old renderer as the control, and explicitly set/restore the correct GL unpack pixel-store alignment during both texture creation and texture updates. Demand original 1493px output straightness, no corruption, unchanged corrected 1494px behavior, and all native actor/input/lifecycle tests. After this proof, coordinate a proposed source correction with the owning SceneFX/SwayPX integration team; do not silently patch the production compositor or expand Social ownership.

## DECISIONS

Verified source-only CI experiment and unmerged draft PR; no production GL unpack alignment change, no local desktop or Social mutation, no merge, and no final DONE. Exact pixel movement is measured; unpack alignment remains a code-level hypothesis pending a separate controlled corrected-renderer CI test. Original Company data remains pinned and unchanged.
