# 🏥💜 Stage Director M19 // Company MapLibre native actor preflight

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre window containment
STAFF ROLE // Nurse 2 (operator-authorized external continuation), not live registered
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE, NOT REGISTERED
ASSIGNMENT // Build SwayPX-owned exact native actor/host geometry preflight for the now-merged Company M18 rectangle
SCOPE // New pure Company C policy/header, standalone tests, inert ordinary-Meson source inclusion, CI and handoff
PERMISSIONS // Bounded GitHub branch/CI/draft PR, no desktop modifications, no merge/production activation
BASE // SwayPX master dd612f430467d78d30841fa6a3abc877191cee30
COMPANY CHECKPOINT // The-Post-Apollo-Project PR #1365 (M18) now merged
PHASE // Isolated policy milestone, live compositor integration pending

## IMPLEMENTATION

The operator verified that the standalone Company MapLibre native Qt map launches through Company and that its PID appears only while opened, with the M15 host bridge READY and M17 QtLocation+QML bundle staged. M18 on Company `main` exports `CompanyNetworkW.activeVectorViewportRect` relative to its inner `companyFrame`, derived from the live `mapCanvas`. It zeroes on close, other mode, editor/consent/raster overlay or invalid geometry. **Those coordinates are NOT compositor-global or native host-local**, so passing them directly as a Stage Director viewport would be incorrect.

Existing SwayPX Stage Director (merged through PR #73) implements opt-in native Wayland pairing for *Weather WATCH*. The `PostApollo.StageDirector 1.0` `WatchBridge` is BBC/C-SPAN source-specific and cannot be reused as Company renderer authority. Older Chrome scene/mask PRs #49-#52 remain historical stacked drafts. Do not merge them automatically or overwrite their branches.

M19 adds:
- `include/sway/stage_director_company_map.h`: an exact native **lease**, compositor observations, untrusted Company geometry report, and explicitly zero-permission candidate statuses.
- `sway/stage_director_company_map.c`: pure preflight rejecting missing compositor selection, wrong Wayland host client, wrong native actor XDG view/root, popup or sibling roots, missing process attestation, mismatched Linux PID/UID/start-time, stale epoch or host-geometry revision, hidden/closed map and invalid geometry. It first validates `frame_in_host` against the exact native layer dimensions, then `map_in_frame` against that frame and uses the existing `stage_director_viewport_from_host_local()` + `stage_director_viewport_plan()` code to derive compositor-global coordinates and actor-local crop. Actor geometry must cover the reported viewport or the candidate is denied until a future compositor sizing controller makes it so. This is a PLAN, not a scene transform.
- `sway/meson.build`: include this inert C policy in the ordinary SwayPX binary. No call sites, new startup flags, global IPC, compositor mutation, Wayland protocol, window launch or input wiring.
- `tests/stage_director/test_company_map_candidate.c`: native C cases for exact refs, spoofed host/actor, reused Linux PID, stale revisions, hidden/closed map, overflows, second output with negative logical origin, and reactive compact/standard geometry.
- `.github/workflows/stage-director-company-map-m19.yml`: native no-dependency C compile/tests with strict warnings, existing viewport/crop regressions, and no-privilege source boundary checks. Existing full native Nix compositor CI is triggered by the Meson/source changes.

## HOW TO USE

Review the draft PR and CI. There are **no operator desktop commands** for M19. The Company UI continues to open and close its existing separate native map through the already installed M15/M17 bridge. The new C function does not yet receive native Wayland observations or Quickshell geometry reports and therefore cannot alter the live map.

Future authorized Stage Director work must:
1. In native SwayPX, identify the operator-selected, live `CompanyNetworkW` layer and the exact native Qt XDG map root, including kernel PID/start-time provenance and output generation. Do not trust namespace, app_id, title, pid string, or user-supplied pointer alone.
2. Establish a **Company-specific**, opt-in same-Qt-QPA-client geometry report mechanism for `frame_in_host` and M18 `map_in_frame`. The current Weather WATCH protocol is not a Company authorization mechanism.
3. Bind both to an externally owned, revocable compositor lease; propagate epoch/host revision, recalc on compact/standard/fullscreen, output transfer, close and process restart.
4. Only after independently verified physical input consent, actor containment/first-frame clip, keyboard/pointer/focus/popups/siblings and child teardown, make a separate **production authority** milestone; M19's candidate must never grant it.

## VERIFY

- Syntagma `AGENTS.md`, Hospital Agent/Organization/Authority/Working Guide read before change.
- Latest SwayPX master was `dd612f4304...` and old SwayPX open PRs target development-stack branches, not master. Company M18 confirmed merged.
- Actual `stage_director_viewport.h/.c`, existing Weather session, Stage Director source, test/build patterns inspected.
- Tests and CI are part of this PR. Their results must be verified on exact head. The native compositor build and runtime test are separate gates.
- No claim of a successfully embedded Company Qt window, QML geometry reporter, scene clip or input/hit-test transformation.

## WATCH OUT FOR

- M19's `compositor_selected`, native pointer references, actor process evidence and host-geometry revision fields MUST be populated exclusively by a future SwayPX-owned live Wayland observation/physical selection mechanism. An ordinary QML or swaymsg caller cannot supply them and gain authority. M19 intentionally contains no issuer.
- `frame_in_host` (Company frame origin inside PanelWindow) is distinct from `map_in_frame` (M18's map canvas inside the Company frame). Values require verified same-native-host custody; never hardcode 12/9 for production despite this test fixture's layout.
- Exact QML `companyFrame` is not its own Wayland surface; it is one item inside the Company PanelWindow layer.
- A geometrically valid report says nothing about QML plugin input, buffer-to-logical scaling, focus, native popup, multi-XDG windows, compositor stacking or frame pixels. M19 returns every authority bit false even on success.
- No new SwayPX or Company file may be installed on user Fedora without explicit local-write approval; do not reset dirty Quickshell worktree or restart live compositor.

## CHECKLIST

- [x] Operator explicitly delegated stalled Stage Director Company work to this external chat
- [x] Verify Company M18 merged and current SwayPX HEAD/open PR ownership
- [x] Preserve Weather WATCH protocol and historical draft branches
- [x] Implement native Company actor preflight using existing viewport code
- [x] Add native tests and ordinary Meson source for compilation
- [ ] Exact-head CI source, policy and full-compositor build pass
- [ ] Review and authorized merge of M19
- [ ] Live SwayPX authenticated Company host/native actor snapshot
- [ ] Company-specific same-QPA Wayland rectangle report + revision protocol
- [ ] Real native actor clip+resize, popup/keyboard/pointer/sibling containment
- [ ] User-local activation and operator final acceptance

## NEXT

Check exact PR head and CI. If green, review/merge only under project approval. Then implement **M20 native Company actor custody** as a separate isolated SwayPX milestone. Recheck latest master and other worker ownership before touching Stage Director shared hooks. Do not activate a Company actor until every native clip/input/seat gate is proven independently in isolated CI.

## DECISIONS

Nurse 2 external continuation limited to Company integration. No claim of live Hospital registration, Weather room takeover, Chrome branch cleanup, merge authorization, production compositor activation, or final operator acceptance. Preserve existing M15/M17 native MapLibre launcher and the M18 real Company viewport handoff.
