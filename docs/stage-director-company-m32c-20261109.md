# 🏥💜 Company M32c // Native in-widget scene placement and crop implementation

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Company MapLibre in-widget native embedding
STAFF ROLE // Nurse 2, external Hospital-compatible successor
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL; live Hospital registration not verified
ASSIGNMENT // Implement actual wlroots compositor node placement/cropping behind a fail-closed nonproduction authorization boundary
BASE // SwayPX master `39aecd7e0f68b1ce4db49e8b70de333c54cb75e4` (M32b merged)
PHASE // M32c source implementation and CI
STATE // DRAFT, no normal compositor scene authority
OWNERSHIP // SwayPX Stage Director Company-only compositor source, scoped to newly added module; no Company UI, Social scene, or user Fedora files
PERMISSIONS // Repo branch/PR/test/build only. No merge, physical host write, live compositor restart, new executable digest pin, or production activation.

## IMPLEMENTATION

The operator's requested result is the native MapLibre interactive map occupying the actual **Company → COVERAGE → VECTOR VIEW** mapCanvas. The existing native Qt standalone window is **not** the requested preview. Previous M30o private CI demonstrated placing/cropping the real map inside the real Quickshell Company PanelWindow and native pointer input, and M31a/b/c proved correct pixel upload. M32a (merged) retains the real Company's Wayland map rectangle; M32b (merged) computes the selected native Qt MapLibre root's global target via kernel pidfd/ELF evidence and live private Company geometry.

M32c now implements the **actual wlroots scene operations** in a dedicated `sway/stage_director_company_scene.c` compiled into normal SwayPX. The private test-build path can hide the selected native XDG scene node, reparent it into the popup-above-overlay layer, translate it to the selected Company's current map rectangle, crop the actual root surface to that exact rectangle, raise the actor, and reveal it only after placement and crop. Normal build has a hard `#ifndef SD_STAGE_DIRECTOR_CI_COMPANY_SCENE_ACTUATION` refusal for every apply call.

The separate `sway/stage_director_company_scene_plan.c` requires the current M32b selected Company host/actor/pidfd/ELF/digest pair plus **all** operator-approved actor, launch, scene lease, input hit-test, keyboard focus, popup and teardown guards, and correct selected epoch/intent/revision/native refs. It also requires an exact 1:1 Qt native source size matching the Company canvas; unsupported size transitions are refused instead of stretching/shearing imported RGB565 rows. A synthetic all-gates-ready test can produce a *private CI scene plan*, never a production scene permission.

The actual default SwayPX path now **observes** the guarded scene plan in `stage_director_view_mapped()` with zero guards. On native XDG unmap and physical Company selection loss, it calls `stage_director_company_scene_revoke()`. CI-only cleanup hides the actor before returning to its prior node parent so it cannot leak out as a floating desktop window, and restores normal clip geometry while the native view is still alive. The normal build cannot create an active scene, so revocation is a no-op there.

No digest is pinned, no operator live actor lease exists, and scene/input remain OFF in the default compositor. Importantly, **this is a compiled backend, not an activated in-widget Company feature**. Actual applying of scene operations needs the future M32d focus/popup/input guard issuer and exact binary approval; otherwise a window could appear above another part of the desktop or intercept input.

## HOW TO USE

Review the M32c draft PR. Its source and normal-compositor CI demonstrate the backend exists in real SwayPX and that live activation remains impossible without a later controlled authorization. **There is no new Company UI control to use yet.** Do not install or merge this as a user-visible embed.

## VERIFY

Expected GitHub CI:
- Compile actual C `stage_director_company_scene_plan.c` + M32b pair + M32a geometry code under `-Werror`, and run tests for missing trusted executable, every separately required input/popup/focus guard, stale epoch/intent/actor root, wrong source dimensions, and negative-coordinate output.
- Compile full normal Nix SwayPX with guarded scene backend.
- Source inspection confirms `wlr_scene_node_reparent`, `wlr_scene_node_set_position`, `wlr_scene_subsurface_tree_set_clip`, `wlr_scene_node_set_enabled` and revocation calls are present, but normal `stage_director.c` has **no apply call**.
- The specific CI-only actuation branch itself still requires a separate disposable actual Wayland/MapLibre runtime test before it can be claimed pixel-verified. Building normal SwayPX does not compile that private branch or prove the full visual result.

## WATCH OUT FOR

- M32c production actuation is **OFF by design**. The approved native ELF digest `company_m30_manifest_elf_sha256` is still empty; there is no compositor scene/input lease issuer or approved physical Fedora test.
- Popup, keyboard, pointer/scroll, mouse click-through, output changing and session-lock handling must be independently guarded BEFORE source is ever enabled in production.
- A source size differing from the reported viewport must stay hidden; no automatic re-scaling or 1494px workaround may be assumed.
- Teardown must happen before wlroots deletes an actor scene node; previously valid Qt process PID, title or app_id must never be treated as continuing identity.
- The CI-only scene backend has no real host side effects and does not change the running Company menu. No full native scene screenshot has been collected from M32c yet.
- SwayPX owner, Company QML owner and Social compositor scene owner have separate responsibilities; no ownership transfer is claimed.

## CHECKLIST

- [x] Verify M32b merged at `39aecd7e`, inspect current SwayPX source and open PR ownership
- [x] Implement guarded exact-size scene plan with proof and missing-guard refusals
- [x] Implement actual wlroots scene hide/reparent/position/root crop/raise/reveal in dedicated compositor module
- [x] Wire normal SwayPX observation and host-loss/XDG-unmap revocation, no apply
- [x] Add pure real-C plan regression and normal-compositor build workflow
- [ ] Verify exact-head M32c CI
- [ ] Compile and exercise private CI-only actuation branch against the real Company QML/Qt MapLibre scene
- [ ] Implement independently reviewed guard issuers for approved executable/launch, pointer/keyboard/popup, focus, output and teardown
- [ ] Admit a true short-lived compositor scene lease, wire Company VECTOR VIEW and run Fedora dual-output visual acceptance
- [ ] Operator approval for merges/production use and final DONE

## NEXT

Complete M32c source/normal build verification and a CI-only native scene execution test, then M32d authenticates live popup/input/focus and teardown before any production scene permission. Only then convert Company VECTOR VIEW from separate-window launch into the actual contained map. Preserve present working raster STREETS mode as fallback until accepted.

## DECISIONS

M32c is an unmerged **production-source backend candidate**, not a production activation. All scene/seat authority remains OFF. No local Fedora changes, no Social ownership expansion, no live Hospital registration or final operator acceptance. Recovery branch `stage-director/company-m32c-guarded-native-scene-20261109`.
