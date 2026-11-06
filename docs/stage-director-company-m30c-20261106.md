# 🏥💜 Stage Director // M30c real compositor event wiring

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre Integration
STAFF ROLE // Nurse 2 · External Hospital-compatible succession
PROVIDER // OPEN AI // CHATGPT
CONNECTION // External, not registered in live Hospital
ASSIGNMENT // M30c connect real mapped XDG events and accepted M28 Wayland v2 pidfd to M30b's revocable native candidate ledger
BASE // M30b branch 4f11c1dcf64da85b852e9dc9787898df65636753 (PR #86, draft)
DEPENDENCIES // M30a PR #85 + M30b PR #86 stacked, neither production-activated
PHASE // Source implementation with CI validation; operator acceptance pending

## IMPLEMENTATION
- Add a separate **12-root maximum**, compositor-owned Company mapped XDG inventory. The existing Stage Director SD0 `actors[64]` registry only retains recognized profiles and can miss MapLibre, so M30c does not reuse that registry or any app-ID/title/namespace as actor authority.
- `stage_director_view_mapped` and `stage_director_observe_container` record genuine native M20 XDG actor resources only when the **physically selected** M24 host has an active M27 private launch-intent generation and M22 direct-child and birth-window checks pass. Every slot owns a real `wl_resource` destruction listener and is removed on XDG unmap, native root destruction, changed process identity, selection revocation or explicit cleanup. An opaque `sway_view *` is reobserved only while its root is still alive and mapped.
- `sway/server.c` registers compositor-only Company lifecycle callbacks at Wayland startup. The private protocol's `report_child_pidfd` calls the birth callback only after the M28 kernel PID/generation verification accepts the real fd. A before-map fd is kept by M28; a before-fd map is retained in the bounded inventory. No second Wayland connection or token is created.
- On either event, scan for a **unique** actual process match against the held kernel pidfd, freshly reobserved native Wayland peer, current selected host and intent. Reject duplicate XDG roots for the same process, stale epochs, a moved/replaced actor or unrecognized resources.
- Only the unique match is passed to M30b's `stage_director_company_reconcile_birth/map/try`, which in turn revalidates the real pidfd and attaches the M21 revocable **candidate**. An ambiguous set clears candidate evidence rather than selecting by first-seen title/app ID.
- Host revocation clears all entries and M21 listeners before invalidating native selected role/surface. No window movement, clip, render, focus, keyboard, pointer, popup or Wayland-global permissions were introduced.

## HOW TO USE
No user action. Company → Coverage → VECTOR VIEW retains **OFFLINE TEST** and separately consented **ONLINE STREETS**, both opening a standalone MapLibre Qt window. The Company Qt bridge and native physical selection remain opt-in/off by default. Nothing is installed or restarted on Fedora.

## VERIFY
- Native real Wayland-client/resource test for the bounded XDG inventory: duplicate/stale identities, mismatched client, explicit unmap, root destruction, whole-client teardown, and exhaustion.
- Existing M30b native process/real kernel pidfd, both receipt orders, M21 ledger revocation and full compositor build checked by exact-head CI.
- Source/CI asserts compositor callbacks actually connect the normal mapped-view lifecycle and M28 successful private birth receipt to M30b, without a new public protocol request or seat/scene authority.
- **Not verified:** running the new SwayPX binary against the user's Fedora/Quickshell/Qt installation, real QPA plugin activation, visible pixel clipping, mouse, keyboard, focus or native popup behavior. Static/native fixture tests do not substitute for a complete live Qt acceptance test.

## WATCH OUT FOR
- The 12-entry inventory is *bounded*: extra simultaneous eligible native candidates fail closed. An ambiguous same-PID set is not resolved by app-ID or map order. This can refuse the right window if multiple Qt roots appear; it cannot grant the wrong one a scene lease.
- The root listener and explicit unmap release opaque `sway_view` pointers. Runtime lifecycle sequencing still needs isolated headless real-Qt evidence before claiming an end-to-end match or enabling production scene changes.
- M30a's source-side SHA-256 digest remains UNPINNED until operator-approved independent executable provenance; a candidate from M30c is never executable approval.
- The available M30c diagnostic path can perform procfs reads during actual Wayland event handling. It performs no synchronous hashing while M30a is unpinned, but latency and teardown reentrancy remain subjects for future live verification.
- Weather WATCH and historic Chrome SceneFX work remain separate and untouched.

## CHECKLIST
- [x] Verify M30a/M30b branches, ownership boundaries and latest SwayPX master
- [x] Create separate native mapped XDG inventory independent of SD0 profile registry
- [x] Wire accepted M28 Wayland v2 pidfd event and compositor map/unmap callbacks
- [x] Require unique current kernel/client/intent/root proof before M21 candidate
- [x] Revoke on resource destruction, explicit unmap, selected host loss
- [x] Author isolated Wayland inventory tests, M30b regressions and full Nix build CI
- [ ] Exact M30c PR-head CI, source audit and authorized merge sequence
- [ ] Real-Qt headless end-to-end actor receipt and teardown tests
- [ ] Operator-approved M15 binary manifest and Qt dependency integrity
- [ ] Scene crop/pixel/input/popup isolation and final operator acceptance

## NEXT
Run exact-head CI, check for source lifecycle warnings, and keep this a draft on M30b's branch. The next isolated verification should exercise an actual Qt QPA map root with the native private pidfd callback in a disposable headless SwayPX (not the user's desktop), including sibling roots, close/unmap, output move, restart and failed process. Never silently install or enable the private bridge, release UI/input authority, or merge unrelated owner branches.

## DECISIONS
M30c remains within the operator-delegated Company MapLibre integration assignment as external Nurse 2. No live Hospital registration, Fedora local changes, production activation, Weather/Chrome ownership expansion or final operator acceptance is claimed. M30a and M30b stay independently reviewable stacked draft PRs.
