# 🏥💜 Company M31a // Isolated SceneFX OpenGL unpack-alignment A/B test

HOSPITAL REPORT

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Company native MapLibre / renderer stride and pixel fidelity
DOCTOR // Nurse 2, external Hospital-compatible successor
STAFF ROLE // NRS-2
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE (live Hospital registration not verified)
ASSIGNMENT // Test whether restoring correct GL unpack row alignment removes actual 1493px RGB565 shear, compared against an unpatched control
PHASE // CI-only renderer control test, not production implementation
STATE // IMPLEMENTED / NATIVE TEST PENDING
REPORT KIND // IMPLEMENTATION CHECKPOINT
BASE // SwayPX M30z draft PR #119 at `78882c28e5255d9aaf1a87043c4df0397d51cf5f`
OWNERSHIP // SwayPX Company isolated test-only SceneFX derivation and existing CI runner; exclude Social production compositor, live widget, Stage Director app grouping
PERMISSIONS // GitHub branch/draft PR and ephemeral CI automation; no merge, local Fedora mutation or production activation.

## IMPLEMENTATION

The predecessor [M30z PR #119](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/119) verified [4/4 native checks](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/42522285743): the original **1493×788** SHM RGB565 native buffer has **2986 bytes per row** with zero padding and its straight Qt fiducials shift **exactly −1.0 pixel per successive row after SwayPX composition**, with horizontal wrap and zero residual over 14 measured rows. The corrected **1494×788** buffer has **2988-byte stride**, straight source and compositor bars, and unchanged real pan/zoom/resize/close. M30z's documentation-only rerun is also green.

The pinned [SceneFX `26200c5` `fx_texture.c`](https://github.com/wlrfx/scenefx/blob/26200c5d9fa947a1d3b0935d3f9237229cd3e5ba/render/fx_renderer/fx_texture.c#L291-L342) sets `GL_UNPACK_ROW_LENGTH_EXT` but does not explicitly override GL's default unpack alignment in the CPU texture upload path. OpenGL's usual default is `GL_UNPACK_ALIGNMENT=4`. Interpreting a tightly packed 2986-byte RGB565 row at a 2988-byte aligned pitch would account for the exact observed pixel shift. This is a **code-based mechanistic hypothesis**, not yet a verified production fix.

M31a adds *only* under `tests/stage_director/`:

1. `ci_company_m31a_patch_scenefx.py`: strict once-only patcher for the pinned Nix source copy of `fx_texture.c`. It saves the active GL unpack alignment, sets alignment **1** around both `glTexImage2D` creation and `glTexSubImage2D` updates, and restores the prior alignment before leaving those code paths. Exact source anchors and replay/refusal tests prevent silently patching other upstream revisions. A **test-only runtime marker** prints on the first 1493px RGB565 upload.
2. `ci_company_m31a_alignment_compositor.nix`: independently overrides only the flake-locked SceneFX library in a separate `swayfx-company-m31a-private-gl-unpack-ci` derivation while retaining the existing CI-only Company crop probe. The original unpatched compositor is still independently built and run.
3. `ci_company_m30o_real_resize.py` and `headless_sway_smoke.sh`: gate the third Wayland compositor session to exact private GitHub Actions and the patched Nix store binary, isolate its HOME/QML copy, require the patched SceneFX marker in the real compositor log, and archive actual 1493px Qt, composed and wlroots evidence.
4. `ci_company_m31a_alignment_impact.py`: compares independently captured source and compositor PPMs from baseline and patched sessions. **Fail if the original baseline does not have −1 px/row**, if either Qt source is not straight, or if the patched original compositor still shears. Preserve corrected 1494px reference from the first run.
5. `.github/workflows/stage-director-company-m31a.yml`: isolated three-session CI: corrected 1494 reference, original 1493 unpatched negative control, original 1493 separately patched SceneFX. Verifies real MapLibre, Quickshell, selected root wlroots buffer, pointer/camera/resize/close and captured pixel rows.

No change to upstream `flake.lock`, default SceneFX, production `sway/stage_director.c`, native Company app, Social Stage Director or user workstation.

## HOW TO USE

Review the draft PR's separate **Stage Director M31a SceneFX unpack alignment A/B native CI** workflow. After a successful job, artifact `sd-company-m31a-scenefx-unpack-native-control-vs-patch` contains:

- Original baseline Qt and compositor PPMs (required **−1 px/row** negative control)
- Corrected 1494px reference PPMs (required straight)
- Patched 1493px Qt and compositor PPMs (must be straight for success)
- Both native wlroots imported-buffer JSON profiles (must remain **1493×788 SHM RGB565, pitch 2986**, no implied extra padding)
- `sd-company-m31a-real-alignment-impact.json` recording the measured effect and nonproduction bounds

No live Company UI control is added; no local install or reload required.

## VERIFY

Acceptance requires **actual** CI-started compositor binaries and actual MapLibre Qt/Wayland PPMs, not a theoretical stride calculation. Source validation proves the CI-specific Nix dependency and patched SceneFX runtime marker, with refusal when the original unpatched or new patched profiles are missing. The native tests must explicitly show original distorted control, patched straightness, unchanged corrected reference, camera continuity, real pointer input and close on private CI only.

The patch must be reversible entirely by deleting the experimental branch; no default SwayPX/SceneFX source has been modified.

## WATCH OUT FOR

The environment is a software-rendered, disposable headless Mesa scene. A passing result would establish that this specific GL unpack patch fixes the reproducible CI distortion, **not** certify AMD/Intel/NVIDIA, DMA-BUF, scaling, full production host custody, scene clipping, focus or multi-output Fedora behavior.

This introduces a **temporary forked library derivation only within GitHub CI**. Do not merge the experimental patch as a production library update without owner review. SceneFX upstream ownership and Social SwayPX Stage Director production pairing are separate. The M30z/M30y Company geometry and tests remain accepted as prior CI evidence, not final user acceptance.

If the Nix override fails to patch or load, or the original 1493px image remains distorted, **report a failed experiment**, do not claim the production bug fixed.

## CHECKLIST

- [x] Verify Syntagma, M30z exact-head CI, PR ownership and flake-locked SceneFX input
- [x] Create separate M31a branch from M30z (no collision)
- [x] Add strict source-only GL unpack alignment save/set/restore patch for create/update
- [x] Add isolated Nix library override and runtime-loaded marker proof
- [x] Add third private compositor with source-copy isolation and test-only gate
- [x] Add negative-control-first image comparison and failing tests
- [x] Add draft GitHub Actions workflow and recovery documentation
- [ ] Verify M31a exact-head grammar/source and native A/B CI
- [ ] Validate original 1493px real output becomes straight under patched SceneFX
- [ ] Review production source-correction strategy with owning SceneFX/SwayPX workers
- [ ] Operator-authorized Fedora physical multi-output and focus/input verification
- [ ] Operator-approved merge and final visual acceptance

## NEXT

Run exact-head M31a CI and inspect the A/B result. If strict real MapLibre tests prove corrected native GL unpack alignment eliminates the original shear without adding a 1494px column, record the exact code path and coordinate the independent production owner handoff. If CI fails, isolate the specific source, Nix, marker or pixel assertion without touching production.

## DECISIONS

Candidate, experimental, unmerged. No SceneFX production package change; no selected native actor, scene or input permission granted; no local Fedora write, no Social production modification and no claim of live Hospital registration or final DONE.
