# Stage Director SD1 // Real native NEWS actor mapping smoke

**PATIENT:** `kudokudo1/Post-Apollo-SwayPx`  
**STATE:** Isolated GitHub PR stacked on `stage-director/ci-udmabuf-access-v0-20261026`; not in SwayPX master or any live desktop.  
**AUTHORITY:** Operator final. No unverified Hospital Room, Assignment, staff role or deployment assumed.

## IMPLEMENTATION

PR #17's GitHub run `40408364501` successfully **built** the complete SwayPX compositor, initialized pinned Mesa/SceneFX software rendering, provisioned temporary per-user access to `/dev/udmabuf` on an ephemeral runner, created real Wayland socket `wayland-1`, shut the compositor down and restored the original device ACL/mode.

This milestone makes the smallest next step toward runtime scene verification: **map one real oversized native Wayland window into that same compositor and measure it**.

- `tests/stage_director/ci_native_news_actor_smoke.sh` launches `foot` from the pinned Nixpkgs input (not the operator's terminal), with app ID `post-apollo-weather-news`, a distinct CI-only title, a requested 1400×850 rectangle and a fixed `/usr/bin/sleep 30` child. It uses the disposable compositor's private XDG runtime and config directory.
- The actor is deliberately *not a browser*. It demonstrates that **any ordinary native Wayland client can claim the expected NEWS app ID**. Stage Director must never use matching app ID as ownership/clip/input authority.
- The wrapper discovers SwayPX's own IPC socket and repeatedly requests the compositor's actual `get_tree` data, then checks that exactly one actor with this ID has a native `xdg_shell` view, a genuine compositor-observed PID, the fixed disposable title and an actual scene rectangle **larger than** the 824×494 reference NEWS bay. A requested initial window size alone is not accepted evidence. Invalid/missing/stale actors cause CI failure.
- `tests/stage_director/assert_native_news_actor.py` checks actual IPC JSON, not log inference. It does not infer a host position or a valid viewer session from the actor rectangle.
- The existing `headless_sway_smoke.sh` runs the optional fixed actor probe only when CI passes an explicit pinned actor binary, and otherwise retains its successful startup-only behavior. The existing runner ACL wrapper still restores device permissions, and all test children are individually terminated.
- `.github/workflows/stage-director-headless-runtime.yml` gets `foot` from the **same pinned Nixpkgs revision** as SwayPX and passes its exact binary path plus the already-built SwayPX `swaymsg` binary.

## HOW TO USE

Only inspect the GitHub Actions check on this draft PR. No command, package, compositor reload or user-machine write is needed. The test actor is created on the disposable GitHub runner only.

## VERIFY

Passing this run demonstrates a native xdg-shell actor maps and can be observed with actual PID, title and oversized compositor geometry while running full SwayPX, followed by clean compositor shutdown and device ACL restoration.

**Not covered by this test:** Actual wlroots clipping, screenshot/pixel masking, the Weather Station layer-shell host, pointer/keyboard/touch dispatch, popup clipping, browser lifecycle or authorized WATCH input. These remain explicitly unverified. The dummy actor is not a video player.

## WATCH OUT FOR

- The app ID is intentionally spoofable; this test is a diagnostic reminder, **not authentication**.
- Sway's window manager may resize a requested 1400×850 window. Verify its real observed bounds are bigger than the reference bay rather than pretending the requested dimensions were obtained.
- A mapped window does not prove pixels are clipped. There is still no trusted session owner calling the dormant `stage_director_viewport_clip_view()` or enforcing `wlr_scene_node_at`/popup/focus containment.
- A fixed `sleep` may still be a child of the disposable `foot` actor. Only one pinned actor and its fixed child are allowed, no arbitrary commands, URLs or browser playback.
- The ephemeral runner and Nix llvmpipe renderer differ from the operator's NVIDIA configuration. Never copy runner device ACL operations to Fedora.

## CHECKLIST

- [x] Verify current PR #17, development branch, source/protocol boundaries and Syntagma authority
- [x] Reuse exact successful isolated compositor/allocator environment
- [x] Pin a disposable native xdg-shell actor in CI
- [x] Assert actual compositor-observed actor geometry and PID
- [x] Preserve no-clip/no-input/browser-authority boundary
- [ ] GitHub CI green for the new actor mapping test
- [ ] Real clipped painting and hit-test acceptance with a controlled scene owner
- [ ] Popup, keyboard/focus and teardown isolation
- [ ] Authorized browser WATCH/CLOSE, then operator-approved integration/deployment

## NEXT

With a verified mapped disposable actor, add a **test-only native wlroots crop and hit-test measurement** using actual scene nodes, without fabricating a production WATCH session. Verify visible pixels, pointer targeting outside the bay, popup overflow and correct clip reapplication in nested SwayPX. Keep the production browser spawn disabled and do not treat fake app IDs as ownership.

## DECISIONS

First verify that a native Wayland actor actually exists and is oversized, then test real render/input containment. Preserve production scene behavior, authority gates and the live machine untouched.
