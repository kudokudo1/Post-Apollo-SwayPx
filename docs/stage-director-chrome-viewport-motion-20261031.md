# Stage Director SD1 // Native Chrome viewport move and resize in isolated compositor

STATE // Draft, isolated CI-only dynamic geometry verification. No production WATCH, browser activation, operator workstation changes or Weather Station QML edits.

PATIENT // kudokudo1/Post-Apollo-SwayPx.

ROOM // Stage Director NEWS integration / protected browser viewport.

ASSIGNMENT // After PR #50's per-actor decorations, verify that a **real mapped browser** can be moved/resized under an authenticated native actor identity while actual compositor pixels remain bounded, including old-location cleanup.

WORKER // OPEN AI // CHATGPT, external Hospital-compatible Doctor, not verified as connected to live Hospital.

BASE // PR #50 HEAD `fd1f35535aac5699e4e2633ff30ede80aa6a25c1`. Upstream PRs are stacked drafts, with PR #49 unexpectedly non-draft; preserve independent review/merge ownership.

## IMPLEMENTATION

Existing CI-only SceneFX probe fixed one native Chrome XDG view to a 320×240 content-source clip and failed closed on any `view->geometry` change. The new opt-in `STAGE_DIRECTOR_CI_REAL_CHROME_MOTION=1` path retains the original fail-closed behavior when the flag is absent, but after each geometry change it **re-verifies the actual mapped `wl_client` PID/UID/starttime and exact systemd test service cgroup** before updating the held crop and source/destination frame size. Supported, bounded CI-only cases are an initial native view at least 1500 pixels wide with 320×240 crop, and a resized window at least 700×500 with 400×300 crop, both anchored at local geometry offset (96,96). Unexpected geometry or provenance forces restoration, never blanket cropping.

The first real run demonstrated that **`floating enable` on an already mapped tiled Chrome window caused its native XDG top-level to disappear**, even though the browser process continued inside its original systemd cgroup. Initial real Chrome mapping and 320×240 screenshot still passed, but the dynamic test **failed**. A repeat run confirmed the failure. The independent step-by-step diagnostic isolated it specifically to `floating enable`; we did not relax native identity or claim a moved screenshot pass. This tiled-to-floating transition is now an explicitly retained SwayPX blocker requiring separate isolation.

The narrower CI motion test now maps the **same native, cgroup-verified Chrome view as floating before its first clip**, using the existing per-actor `view_map()` hook and `container_set_floating()` ONLY in the opt-in CI variant. Normal SwayPX behavior and unrelated Wayland windows are unaffected. Chrome requests an initial 1600×900 window, and source constraints require its actual floating rectangle to be bounded on the output.

The disposable GitHub test uses the compositor's private IPC socket and the exact **native XDG container ID**, not app_id or title, to issue only:
- `resize set width 1100 px height 700 px`
- `move position 430 220`

It checks the native view survives each individual command before verifying three stable final geometry observations and screenshots.

The initial bounded floating Chrome actor must keep the SAME native container ID and kernel PID/starttime, have `border=none`, and settle for three repeated geometry observations inside the output. An unrelated Foot Wayland control retains its normal titlebar, proving the global border config has not changed. The browser is non-root, confined to a private systemd cgroup/network namespace, with ephemeral profile, fixed `about:blank`, no sandbox-bypass flag and no production browser process issuer.

An independent pixel checker compares three actual `grim` PPM screenshots from the SAME isolated SwayPX process: black desktop baseline, initial Chrome at its original location and moved/resized Chrome. It verifies that the originally bright point turns black, the moved 400×300 viewport has bright points including newly expanded width and height, four outer margins remain dark, and desktop background is unchanged. A green scene hit-test alone is insufficient. Exact browser process, profile, window teardown and runner /dev/udmabuf ACL restoration remain mandatory.

## HOW TO USE

Review the draft PR and `Stage Director real Chrome dynamic viewport motion and resize (CI)` GitHub workflow. No local terminal commands, installation or Weather QML changes are requested.

## VERIFY

Source gate ensures this logic is exclusive to `sd-ci-news-scene-probe=true`. Build ordinary production SwayPX and verify the CI Chrome probe is not linked; build the separately gated compositor; map real Chrome and unrelated Foot; verify first 320×240 screenshot; move/resize the same native actor; verify new 400×300 screenshot, no stale pixels, accurate native geometry and full teardown. **CI pending until full workflow completes.**

## WATCH OUT FOR

- This is **a disposable test-issued Sway IPC action**, not an authorized production API. Real production movement/resizing must be driven by the paired Weather Station native host/session contract, with transactions and bounds derived from its actual viewport.
- The CI crop is still tied to one Chrome toplevel, with fixed source offset, not host-specific geometry. Popup windows, redirected input, focus, Chrome sandbox internal attestation and independent child surfaces remain unverified.
- Geometry changes while a clip is held can invalidate a scene tree; the probe intentionally refuses unknown geometry or failed native cgroup validation. Preserve failure screenshots and compositor logs, do not silently disable clip checks.
- The private Chrome service's Ubuntu/AppArmor environment is CI only, not a production browser broker.
- The stack includes PR #43's private Wayland global publication fix; no partial deployment of #42 without it.

## CHECKLIST

- [x] Recheck Syntagma, PR #50, source, native actor and SceneFX post-layout contracts
- [x] Extend only CI scene probe to recompute bounded clip when geometry changes, with native PID/cgroup revalidation
- [x] Issue actual fixed move/resize against exact native XDG ID with stable geometry acceptance
- [x] Add independent initial/moved/resize pixel comparison, including expanded viewport and old-location darkness
- [x] Preserve unrelated app borders, browser cleanup and default compositor source isolation
- [x] Add separate GitHub workflow and screenshot artifacts
- [ ] Achieve full CI green, record any movement/resize render defects
- [ ] Production Weather host-driven viewport geometry, per-actor scene attachment
- [ ] Popup and focus/keyboard/pointer containment under resize
- [ ] Full browser broker integration and operator live acceptance

## NEXT

If green, advance to the separately blocking tiled-to-floating transition, then real popup/subsurface and input/focus containment. If red, preserve stepwise native tree/journal evidence without claiming geometry support. If red, preserve actual pixels and fix renderer geometry without weakening native actor/identity checks.

## DECISIONS

No merge necessary to test stacked branch. User authorized merges when required but final operator acceptance is separate. No live Hospital registration or workstation mutation.
