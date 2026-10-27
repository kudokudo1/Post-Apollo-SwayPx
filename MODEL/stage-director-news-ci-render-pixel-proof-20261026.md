# Stage Director SD1 // Real Wayland rendered-pixel containment probe

**PATIENT:** `kudokudo1/Post-Apollo-SwayPx`.  
**STATE:** GitHub-only, opt-in compiled test, stacked on [native clip/hit-test PR #19](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/19).  
**AUTHORITY:** Operator final; no Hospital Room/Assignment, local system write, merge or production activation presumed.

## IMPLEMENTATION

Prior CI already compiled full SwayPX, started a headless wlroots/SceneFX compositor using pinned Nix/Mesa and a temporary restored per-user `/dev/udmabuf` ACL, mapped a real oversized disposable `foot` Wayland window, and measured `wlr_scene_node_at()` inside/outside native clipping. PR #19 recorded both points present before clipping and only the inside hit after.

**Pixel-level acceptance now requires actual compositor screencopy content, not merely scene geometry**.

- Reuse the existing default-off `sd-ci-news-scene-probe` Meson option. A separate CI Nix output compiles the instrumentation; the ordinary build excludes it.
- Under both `GITHUB_ACTIONS=true` and `STAGE_DIRECTOR_CI_RENDER_PROBE=1`, with one exact fixed disposable actor title and app ID, the CI-only probe **temporarily holds** the 320×240 wlroots clip for a screenshot rather than restoring it immediately. It logs compositor-global pixel sample coordinates. On subsequent client commits it reapplies the clip if the original native view geometry still matches; changed geometry causes refusal/restoration. It restores the normal Sway crop on actor unmap.
- The fixed `foot` actor gets a mode-isolated configuration with opaque **white** background, while the headless Sway output background is **black**. No browser, URL, Internet media or arbitrary executable is involved.
- Retrieve `grim` from the **same pinned Nixpkgs** as SwayPX; use the real `wlr-screencopy` protocol to produce P6 PPM screenshot of the virtual output. A pure Python standard-library pixel verifier parses the screenshot and samples 5×5 regions at independent inside, outside and desktop points. It demands bright inside content, dark outside/background and statistically significant contrast. Any malformed screenshot, missing native clip log, unexpected color, premature exit, or clip failure is **FAIL**.
- Capture a PNG screenshot from the same isolated compositor and publish it as a short-lived GitHub Actions artifact (7 days) for independent review, not as user desktop media.
- Normal software rendering, exact actor PID/rect observation, CI allocator access and automatic original ACL restoration remain unchanged.

**First measured result:** [CI run #40423078584](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/40423078584) compiled both binaries and passed real native hit-testing (`before_inside=1 before_outside=1 after_inside=1 after_outside=0`), but **the first pixel assertion failed**. Its logged inside/outside positions `(160,160)` and `(24,24)` were from the scene subtree **before** Sway's first transaction placed the actor at actual IPC position `(259,117)`. Both sampled pixels were black desktop pixels, not an observed rendering leak. The test correctly failed without declaring pixel containment.

**Correction under test:** Use the native Sway IPC `get_tree` rectangle **after the actor is mapped** as the screenshot coordinate origin. Check two inside and two outside sample patches plus a separate desktop reference. Capture/preserve the PNG screenshot before evaluation, including when assertions fail, to permit independent diagnosis. This change does not alter production compositor rendering or scene policy.

**Second measured result:** [CI run #40423982356](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/40423982356) successfully built both compositor variants, mapped the 1402×847 native Wayland test actor at **(259,117)**, and captured the real screenshot. Its samples were `inside_luma=[255,255]`, `outside_luma=[255,255]`, `desktop_luma=0`. **Visual containment FAIL** was correctly reported and the [screenshot artifact](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/40423982356/artifacts/12699476653) preserved. This result must not be described as successful rendering containment.

**Root cause under examination:** SwayFX's `transaction_progress()` calls `arrange_root(root)` **after** `transaction_apply()`. Normal Sway view arrangements invoke `wlr_scene_subsurface_tree_set_clip()` with the full view content rectangle, overwriting the earlier temporary crop. The first native clip/hit-test probe ran before that final arrangement, so its positive result was not a promise about the painted frame. The follow-up adds an **explicit CI-compile-only** post-`arrange_root` hook that reapplies the temporary crop and measures native hits again; pixel verification still must independently pass. Normal production builds exclude the hook entirely.

**Artifact review finding:** I downloaded and visually examined the preserved screenshot from run `40424774909`. The solid-white foot surface begins approximately at the clip's source offset (`x≈357,y≈213`), but its bright area continues far beyond the 320×240 intended viewport. This is not just a sampling error. `wlr_scene_subsurface_tree_set_clip` restricted the hit-test but did not produce the required painted destination bounds in this test configuration. **Do not accept render containment.**

**Narrow CI experiment:** wlroots documents `wlr_scene_buffer_set_source_box()` (which controls the sampled crop) separately from `wlr_scene_buffer_set_dest_size()` (which controls its destination painting dimensions). Source cropping without a matching destination size can leave content painted far beyond its intended viewport. The opt-in CI variant now sets the disposable root scene-surface destination to the 320×240 crop size after `arrange_root()`, logs the actual number of normalized surface buffers, then restores its natural destination size at test actor unmap. This is a **test-only exploration of a necessary mechanism**, not a production fix, and must pass an actual screenshot before conclusions change.

## HOW TO USE

Inspect the new draft PR's **Stage Director isolated headless SwayPX runtime smoke**, source output and (only on success) GitHub artifact `stage-director-news-ci-pixel-screenshot`. Nothing is installed or modified on the operator's desktop.

## VERIFY

A green result requires the full Nix build, separate test-enabled build, actual native Wayland actor map, native scene hit-test, real `grim` screenshot plus pixel comparison, client teardown, compositor shutdown and restoration of the CI-only device ACL.

**Not proven:** The Weather Station layer shell host is not present, there is no real VIDEO browser process, no exact NEWS bay mapping, no global compositor hit-test/seat dispatch proof, and no XDG popup, touch, keyboard focus, XWayland unmanaged or browser teardown acceptance. A screenshot only verifies this *specific isolated native test actor* has visible pixels clipped within the injected test region under the headless renderer.

## WATCH OUT FOR

- A main surface clip alone is not a verified popup or pointer/keyboard containment system. Stage Director cannot enable WATCH from this passing test.
- The disposable actor's app ID/title are spoofable. They restrict CI instrumentation only; authenticated operator intent and native process ownership remain absent.
- Window decorations, CSD and output scaling can affect actual pixel locations. The test uses compositor-global content-tree coordinates logged by native SwayPX, not requested window geometry alone. A mismatch is a test failure and should be diagnosed from the screenshot artifact/log, **not** waved away.
- Multiple Sway transaction/commit paths reset clips. The opt-in CI probe reapplies on recognized subsequent commits, but no production clip persistence is enabled.
- This special software-rendered Nix runner is not the operator's NVIDIA Wayland system. Do not copy `sudo setfacl` runner provisioning to the live host.
- The screenshot artifact must not include personal data: this compositor has a private runtime, only a disposable terminal with `/usr/bin/sleep`, and no connected apps.

## CHECKLIST

- [x] Verify current PR #19 and production/dev branch boundaries
- [x] Add compile-gated screenshot hold and explicit actor-unmap restoration
- [x] Use pinned foot configuration and pinned grim screenshot client
- [x] Validate real P6 screenshot pixel brightness/contrast using standard library
- [x] Add static CI activation guard and bounded screenshot artifact
- [ ] GitHub CI green and screenshot artifact independently inspected
- [ ] Real compositor global pointer/touch and popup/focus isolation
- [ ] Registered Weather host/bay, process-authenticated WATCH and teardown
- [ ] Operator approves any branch merging or live deployment

## NEXT

If the actual screenshot proves a white inside region and black outside region, advance to **global compositor input and popup isolation tests** using the already working native test actor and private Nix runtime. Do not enable browser playback until authenticated Weather host/session ownership and input/popup/teardown gates are verified.

## DECISIONS

Pixel evidence must come from a real compositor screenshot, not inferred hit testing. Keep the crop probe test-only and automatically restore the normal clip on actor unmap. No merges, production activation, local files or desktop process changes.
