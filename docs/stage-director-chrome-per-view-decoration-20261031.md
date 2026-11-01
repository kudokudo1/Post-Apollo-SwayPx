# Stage Director // Per-actor Chrome decoration suppression, ordinary titlebar retained

STATE // Draft CI-only native actor decoration-policy verification; not live Weather browser activation.

PATIENT // kudokudo1/Post-Apollo-SwayPx.

ROOM // Stage Director Weather Station NEWS browser scene containment.

ASSIGNMENT // Continue approved Option A; fix the previous global borderless CI workaround with an exact-browser native per-view policy, and verify other Wayland windows retain their ordinary titlebars.

WORKER // OPEN AI // CHATGPT, external Hospital-compatible Doctor; no verified live Hospital registration.

BASE // PR #49, `b5c3082e248c7e34b2e37f87502a6e0c653ffbfb`. Upstream stacks #41–#49, including mandatory PR #43 private WATCH global discovery correction, remain separate pending proper merge review. PR #49 was observed open, mergeable and no longer draft at this preflight; do not silently change its review status.

OWNERSHIP // SwayPX repository and bounded GitHub CI only. The-Post-Apollo-Project Weather Station Quickshell/QML is owned by another worker and not edited.

## IMPLEMENTATION

Previously the real Chrome four-edge screenshot passed only when **all test compositor windows** inherited `default_border pixel 0` and `default_floating_border pixel 0`. That made a useful crop experiment but globally removed unrelated window decorations, which is not a suitable production pattern.

This slice restores the **normal server-side titlebar as the disposable SwayPX test configuration default**. Instead, a source-gated C hook in the existing `sd-ci-news-scene-probe=true` translation unit is called from `view_map()` after border initialization and before initial container arrange. The hook derives the actual native `wl_client` for the XDG toplevel, verifies Linux credentials (PID/UID and kernel starttime), and matches the exact systemd service cgroup for the running disposable Chrome fixture. Only then may it set **that mapped view's** `container->pending.border = B_NONE` and border thickness 0. No app-id/title, forged source ID, IPC command or caller-supplied pointer can authorize the override. No compositor-global configuration or unrelated actor is altered.

The independent CI pilot maps a **second native Wayland control window using pinned Nixpkgs Foot** before Chrome is launched. It requires that window to be observed as an XDG toplevel with native process identity and `border: normal` in the real Sway compositor tree, then shuts it down and verifies it disappears. The Chrome fixture independently requires `border: none`, its exact systemd cgroup PID/starttime and a mapped XDG window. The existing `grim` screenshot gate then requires two bright 320×240 inside points, black baseline plus all four outside edges unchanged, and black background corners. It finally requires Chrome subprocess/cgroup, mapped window and private profile teardown.

All of this is **test-only**: normal SwayPX must build without the `SD_CI_CHROME_PER_VIEW_BORDER` hook or `SD_CI_CHROME_PIXEL_SCENE` selector, and CI uses a separate Meson opt-in compositor. There is still no production WATCH browser launch or input/focus/popup authority.

## HOW TO USE

Review this draft PR and `Stage Director scoped Chrome decoration with unrelated Wayland control (CI)` workflow. It runs only on the disposable GitHub runner. No operator desktop commands, new browser installation or Weather QML edits.

## VERIFY

Source checks require native `wl_client`, exact cgroup and Linux PID/starttime, per-container `B_NONE`, normal Sway defaults, the separate unrelated `border=normal` control, Chrome-only `border=none`, native screenshot crop and absolute test-only Meson guards. CI also builds full ordinary SwayPX to ensure test hooks are absent. The final full run is still pending.

## TEST EVIDENCE AND REPEATABILITY

First full real-browser run on PR #50 passed normal SwayPX compilation and the unrelated Foot XDG view's `border=normal`, and the compositor's native process/cgroup matcher logged `SD_CI_CHROME_PER_VIEW_BORDER` plus `SD_CI_CHROME_PIXEL_SCENE ... hit_test_crop=PASS`. However, the browser-window test reported no unique externally observable Chrome XDG candidate before its deadline. **That attempt failed and is not counted as a pixel pass.**

The following CI diagnostic run `41259461594` passed source boundary, normal and CI compositor builds, unrelated-window titlebar, exact Chrome `border=none`, real Chrome cgroup/native peer, and four-edge screenshot. Measured brightness: inside `255.0, 255.0`, all five outside samples `0.0`, baseline and desktop `0.0`; all Chrome process identities, Wayland view, private profile and temporary runner device ACL were cleaned up. The diagnostic code also captures native PID, shell, border and rectangle to an artifact on any future candidate mismatch, **without accepting ambiguous windows**.

Because there was an earlier intermittent native-window mismatch, the current acceptance now requires **three consecutive observations of one identical, cgroup-owned XDG PID, border and rectangle** during the pixel test, instead of accepting a single transient candidate. This change requires another GitHub Actions verification. CI success on the preceding commit is not yet proof that the strengthened current HEAD passes.

## WATCH OUT FOR

- A CI-only **per-view** border override is not a vetted, operator-authenticated **production** per-view decoration policy; production admission still requires a trusted WATCH session, exact browser ownership and all native actor/host/frame boundaries.
- This is a single fixed 320×240 viewport and one non-networked `about:blank` Chrome xdg_toplevel, not the actual Weather Station host-supplied viewport. Dynamic view geometry, popups, subwindows, pointer/keyboard focus and input remain unchecked.
- Sway renderer's content subtree clip does not affect titlebar paint outside that subtree. Test must fail if default borders are globally removed or if a different actor loses a titlebar.
- Chrome's internal sandbox state is not independently attested. Systemd's transient cgroup on GitHub is not the production broker.
- PR #43 fixes the private WATCH discovery leak in its ancestor PR #42; do not merge/deploy #42 alone.

## CHECKLIST

- [x] Recheck source, live GitHub PR state, Syntagma ownership/permissions, actual border creation path in `view_map()` and `transaction.c`
- [x] Add CI-only native process/cgroup-matched per-view border override before initial layout
- [x] Remove global CI borderless workaround and keep normal titlebar defaults
- [x] Add independent Foot Wayland control, native border assertion and orderly unmap
- [x] Require Chrome border=none, actual four-edge screenshot pass and full browser teardown
- [x] Add full production-vs-CI binary separation workflow
- [ ] Verify GitHub CI, diagnose and repair any failure without weakening native identity or screenshot requirements
- [ ] Link authorized production browser lease to per-view scene and decorations
- [ ] Real Weather host-driven viewport/transaction resizing and popup/input boundaries
- [ ] Weather worker native Quickshell bridge and operator live acceptance

## NEXT

Once verified, advance to dynamic real-actor viewport movement/resizing and repeated real screenshots across layout changes, followed by independently-positioned popups and focus/pointer/keyboard restrictions. Do not activate production browser playback until each boundary passes.

## DECISIONS

No merge, local machine modification, production browser activation, or Weather QML change is required for this CI-only milestone. User permission to merge when necessary remains, but stacking tests do not require merging and unfinished partial-security ancestors should not be promoted without review. Technical CI is not final operator acceptance.
