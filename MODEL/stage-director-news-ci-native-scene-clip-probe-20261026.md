# Stage Director SD1 // Actual wlroots NEWS scene clip/hit-test probe

**PATIENT:** `kudokudo1/Post-Apollo-SwayPx`  
**STATE:** GitHub-only, compile-gated probe stacked above PR #18's native actor mapping; not in master.  
**AUTHORITY:** Operator final. No live Hospital Room, Assignment, staff role, local writes or deployment presumed.

## IMPLEMENTATION

PR #18 verified that the actual Nix-built SwayPX starts with SceneFX + llvmpipe in disposable GitHub CI, maps a real native Wayland `foot` test actor with spoofable `post-apollo-weather-news` app ID and IPC-observed PID and 1402×847 rect, then shuts down and restores the original `/dev/udmabuf` ACL.

This separate, **strictly test-only** increment measures a real compositor primitive rather than more modeled geometry:

- Add `meson_options.txt` option `sd-ci-news-scene-probe`, **false by default**. Only an explicitly Nix-overridden CI compositor package compiles `sway/stage_director_ci_scene_probe.c` and adds its call sites. The normal SwayPX build has no compiled probe and never exposes a runtime switch to activate it.
- With `GITHUB_ACTIONS=true`, `STAGE_DIRECTOR_CI_SCENE_PROBE=1`, exact disposable title and NEWS candidate app ID, once a real content buffer and sufficiently large view exist, sample actual `wlr_scene_node_at` on inside and outside points of the real scene. Then call the real `wlr_scene_subsurface_tree_set_clip` with a smaller root-surface-local test box; repeat native hit tests; **immediately restore Sway's normal clip with `view_center_and_clip_surface`**.
- Record exact before/after hit observations and `result=PASS/FAIL` in SwayPX's isolated debug log. CI insists on inside=present and outside=absent after clipping, plus the marker that the normal clip was restored. Missing test buffer, missing marker or mismatched hit tests fail the workflow.
- Trigger the probe only from test-enabled mapped-actor observer/XDG buffer commit paths. There is no trusted WATCH event, mapped Weather layer, browser execution, user input routing, popup handling, clipboard, focus or persistent clip.
- The GitHub workflow first builds the **unchanged normal Nix compositor**, then a distinct opt-in test compositor by overriding Meson flags on the disposable runner; this second binary alone is used for the fixed `foot` actor smoke.
- Add `test_ci_scene_probe_boundary.py` to forbid accidental production enablement and require default-off build, compile guards, narrow test identity, and zero browser/session/focus authority.

## HOW TO USE

Review the draft GitHub PR's **Stage Director isolated headless SwayPX runtime smoke** and raw logs. Do not run the test against the operator's Fedora desktop. Only disposable CI environments are authorized for this measurement.

## VERIFY

**Before CI:** source change exists, but no claim of compositor crop success is made. A full native debug build and actually mapped actor are necessary for evidence. A passing test means only **real wlroots scene hit testing** observed an inside scene node and rejected the outside point under a temporary clip in this isolated actor. It does NOT mean the surface was visibly cropped on a screenshot or that `node_at_coords` (which traverses the full compositor layer scene), pointer events, XDG popups, focus or real Weather NEWS session were contained.

If the clip does **not** affect `wlr_scene_node_at` as expected, treat the failing observation as evidence of a missing hit-test boundary. Do not force a PASS or claim the clip is sufficient to contain input; the correct next step may be a separate compositor-side input filter.

## WATCH OUT FOR

- **Safety:** This is a CI-only compiler variant; a matching spoofable app ID/title is acceptable *only* for a disposable test actor, NEVER for trusted WATCH authority. No production viewer clip path is enabled.
- **Clip reset:** Normal Sway transaction/client-buffer code also resets clip. The probe restores normal clipping immediately. Real session integration will require properly ordered post-layout reapplication with verified ownership and input/popups.
- **Scene coordinate system:** The measurement uses actual content-tree global coordinates and the current wl_surface geometry; if the native scene node API requires different relative coordinates, its failing CI result must drive correction rather than inventing a pass.
- **Scope:** No pixel screenshot or pointer injection occurs. Popup scene and XWayland unmanaged paths are untouched; they must be tested independently.
- **Runner:** Reuses the already-green pinned Nix/Mesa/SceneFX headless environment with exact original `/dev/udmabuf` ACL restoration. No production device flags are changed.
- **Branch:** This work is stacked above PR #18, which is still a draft development PR. Even a successful CI run is not a merge into SwayPX `master` or local deployment.

## CHECKLIST

- [x] Reinspect live GitHub branches, Syntagma, mapped test actor and wlroots/Sway clipping hooks
- [x] Add default-disabled compile-gated native scene test
- [x] Apply and restore a real temporary clip inside CI
- [x] Record actual before/after scene hit tests and fail closed on unexpected behavior
- [x] Preserve default build separately from opt-in build
- [ ] GitHub CI compile, run and observed native hit-test result
- [ ] Real paint/pixel clipping, `node_at_coords` routing, pointer/touch, keyboard, popup and teardown tests
- [ ] Verified Weather host↔browser session ownership and operator-approved integration/activation

## NEXT

Inspect the exact CI compiler and runtime result. If native clipping excludes outside hit tests, build a follow-up isolated screenshot/pixel measurement plus real seat/popup tests. If not, implement a proper authenticated compositor hit-test guard before attempting any browser WATCH. Keep NEWS safe-mode until all safeguards are verified.

## DECISIONS

Prove the exact wlroots behavior with reversible CI-only instrumentation; do not equate a model policy with compositor behavior or let the test gate open production browser controls.
