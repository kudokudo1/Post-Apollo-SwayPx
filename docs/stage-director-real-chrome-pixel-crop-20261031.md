# Stage Director SD1 // Actual real-Chrome viewport crop pixels, isolated CI

STATE // Draft CI-only real browser SceneFX experiment, actual screenshot acceptance pending. NOT production NEWS viewer or Weather Quickshell integration.

PATIENT // kudokudo1/Post-Apollo-SwayPx.

ROOM // Stage Director NEWS native browser actor / viewport scene containment, external Hospital-compatible, not verified live Hospital registration.

ASSIGNMENT // Continue user-approved Weather NEWS-first development following PR #48: verify whether native actual browser pixels, not merely compositor hit-tests, are confined to a 320×240 proposed viewport.

HEAD BASE // PR #48 HEAD `1f94c869785fa0c30af2a4cfbc9ae47d08975f47`. The SwayPX default branch remains outside the stacked PR #41–#48 work.

WORKER / PERMISSIONS // External ChatGPT Doctor, SwayPX repo GitHub draft PR edits allowed and merges user-authorized when necessary. No live desktop changes, no Weather repository ownership transfer, no production activation.

## IMPLEMENTATION

The existing `sway/stage_director_ci_scene_probe.c` is already only built with opt-in Meson `sd-ci-news-scene-probe=true`. Add a **separate, CI-only real Chrome selector** in this translation unit. It requires `GITHUB_ACTIONS=true`, `CI=true`, exact run ID and attempt, explicit `STAGE_DIRECTOR_CI_REAL_CHROME_PIXEL=1`, a mapped native XDG toplevel and its **actual `wl_client`** credentials. It cross-checks Linux kernel `/proc` PID/UID/starttime and the exact final cgroup path component for that runner's transient systemd browser service. Window titles, app IDs, URLs and scene coordinates are never interpreted as identity or authorization.

For that one uniquely selected actor, the existing CI SceneFX/wlroots crop primitive holds a 320×240 local source rectangle (96,96) in the root content tree. The existing post-layout reapplication, source crop, buffer destination normalization and output repaint override are reused. Native hit-test acceptance is diagnostic only; it explicitly logs `render_pixels=UNVERIFIED` and all authority remains none.

The **real test gate is a screenshot**, not the scene graph. Before launching Chrome the harness records the actual SwayPX 1920×1080 black screen through pinned `grim` and later records a screenshot with the still-mapped, cgroup-matched browser `--app=about:blank`. `assert_real_chrome_clip_pixels.py` imports the repository's existing P6/patch sampling helper, identifies the one actual native XDG actor via the Sway tree and exact PID, and compares white-ish browser pixels at two points inside the intended crop against black/unchanged pixels on **four independent outside edges** and desktop background. It fails on any visible content leakage, absent browser rendering, unexpected output geometry, ambiguous native window, or loss of baseline.

The same test still stops the full systemd browser cgroup and requires no surviving original PID/starttime, no remaining members, no mapped native window and no persisted private profile. The established `ci_udmabuf_smoke.sh` always restores temporary runner device ACL. GitHub Actions optionally preserves disposable screenshots and mapped tree JSON for failed-pixel forensics. Screenshots contain only the controlled blank browser, not operator desktop content.

## HOW TO USE

Open the stacked draft PR and `Stage Director real Chrome screenshot crop (isolated CI)` workflow. This is not installed on Fedora. The only compositor source change is in a **test-only C translation unit** excluded from ordinary builds, and is gated at runtime by strict isolated GitHub conditions.

## VERIFY

Source boundary checks require native `wl_client` credentials, exact cgroup, kernel starttime, CI-only Meson flag and four independent screenshot edge tests. Build ordinary SwayPX and assert the `SD_CI_CHROME_PIXEL_SCENE` selector is absent from its binary. Build a separate CI probe SwayPX and verify the selector exists. Launch the ordinary Chrome binary under an isolated non-root transient systemd cgroup with no external network or sandbox bypass, map its real XDG window, take baseline and held-clip compositor screenshots, assert actual pixel containment and original process/window/profile teardown. GitHub validation is pending until workflows execute.

## WATCH OUT FOR

- Historic scene test discovered real painting can leak beyond `wlr_scene_subsurface_tree_set_clip`: SceneFX root source crop and destination rectangle are separate. **Never call hit-test success a visual PASS**. This PR may reveal the rendering path still leaks; a failed pixel job is a blocker, not evidence of a finished viewport.
- No real Weather Station host geometry or popup/focus/input lease. The fixed 320×240 rectangle is an isolated proof sample, not approved final Weather UI dimensions.
- Chrome has its own subprocess and potential multiple surfaces; this test checks a single native top-level's visible pixels and four margins. It does not prove all future Chrome popups, subsurfaces or focus, overlay, keyboard and pointer states.
- The systemd service has `ProtectHome=read-only` only because the runner's disposable Sway socket is under HOME; production needs stronger isolation. Internal Chrome sandbox status is not independently verified.
- No `--no-sandbox`, external source URL, production actor broker, live WATCH launch issuer, real Weather Quickshell integration or operator desktop changes.
- The stacked PR #43 private Wayland global publication security fix remains mandatory; do not deploy PR #42 alone.

## CHECKLIST

- [x] Reverify current Syntagma and PR #48, existing CI-only scene crop, SceneFX render pitfall, native browser cgroup and screenshot tools
- [x] Add exact native cgroup/Wayland browser CI scene selection (no app-id trust)
- [x] Reuse existing 320×240 clip, destination normalization and per-frame override only in CI
- [x] Implement before/after real screenshot and four independent pixel-edge comparison
- [x] Preserve native browser cgroup process, Wayland window and profile teardown
- [x] Add production-vs-CI binary boundary tests and upload disposable screenshot artifacts
- [ ] GitHub native compile and pixel test pass; if not, record leak and repair independently
- [ ] Host-derived Weather Station viewport placement and normal compositor transaction persistence
- [ ] Popup, focus/keyboard/pointer and click/drag/zoom guard tests
- [ ] Production verified actor broker, source allowlist and WATCH admission
- [ ] Weather worker native Qt bridge and operator live acceptance

## NEXT

If pixels pass, advance to multiple browser surfaces/popups, resize/monitor movement and native input/focus containment. If pixels leak, inspect the retained PPM/PNG and SceneFX source/destination rendering telemetry and fix crop without ever enabling production browser startup until visual containment is independently verified.

## DECISIONS

This slice is CI-only, draft, unmerged pending verification. User has authorized merges when necessary; no stack merge is required merely to run CI. Technical completion is not operator final acceptance.
