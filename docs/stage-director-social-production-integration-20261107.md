# Stage Director // Social W production native floating-window integration

**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`; consumer `kudokudo1/The-Post-Apollo-Project`.  
**ROOM / ASSIGNMENT //** Unified Social W FloatingWindow + Vesktop native move/resize, operator-led Stage Director integration.  
**PROVIDER //** OPEN AI // CHATGPT. External Hospital-compatible session, not live registered.  
**ROLE //** Operator has assigned lead coordination for Stage Director in this task; not a claim that Hospital Staff Role or organizational ownership was registered in live Hospital.

## IMPLEMENTATION

Unlike earlier CI-only overlays, this branch **links a native group into the actual SwayPX compositor build** and the real native seatop movement path. It corrects the prior PR #106 proposed layer-host assumption: real Social W in `MessagingW.qml` is a normal floating XDG toplevel with title `QS_SOCIAL_MENU`; its docked guest is Vesktop XDG. The system does not bind to arbitrary layer-shell windows.

- Runtime Sway IPC operation `stage_director_social pair HOST_ID GUEST_ID RAIL INSET` is **opt-in**, and requires two distinct mapped and visible floating XDG views, exact host title, exact Vesktop application ID, two distinct native Wayland clients, same non-fullscreen workspace, and an **already geometrically aligned** Social/guest pair. `stage_director_social status` confirms whether the group is still current; `clear` revokes it. No pairing happens from title/app ID discovery alone.
- The live group stores exact Sway container/view/surface/wl_client references, a generation, workspace and measured layout widths. Each native gesture rechecks the current mapping and identity. View unmap/destroy revokes the group before pointers can become stale; replacing the app ID does not automatically rebind.
- Both real `seatop_move_floating.c` and `seatop_resize_floating.c` ask the Stage Director module to stage the companion rectangle **before their existing `transaction_commit_dirty()`**. A host-led operation raises the guest above the opaque host before the initial commit. No separate delayed geometry-follow IPC is required while a native pair remains valid.
- The existing Sway command socket is a **same-user local-desktop control surface**, already privileged to move/resize arbitrary containers. This command is not a new security boundary for arbitrary clients and must not be described as independent operator credential attestation. Native references come from Sway's own tree after an explicit command and are revalidated; an unprivileged network/Wayland client cannot pass raw pointer values.
- Pairing is **not auto-enabled on compositor launch**. The accompanying Social bridge PR must issue `pair` only after its existing exact-window and docking handshake, suppress polling fast-follow while `status` remains true, and return to the legacy bridge on refusal or teardown. No other Stage Director Company/MapLibre/Weather paths are reworked.

## VERIFY

The `Stage Director Social native production compositor build` GitHub Actions workflow compiles **actual production SwayPX** from the PR and checks command registration, Meson linkage, native seatop ordering, revocation hooks and clean checkout. This build test is a necessary prerequisite, not automatic proof of an installed no-lag experience. A separate headless real-window test and operator workstation visual acceptance are still required before merging/deployment.

## WATCH OUT FOR

- The existing Social bridge's real host is **floating XDG**, not layer-shell; PR #106's candidate ledger is unrelated/unlinked and must not accidentally grant the production group authority.
- Only **native Wayland** Vesktop XDG is covered by this first native implementation. XWayland Vesktop, other providers, unknown/splash and hidden scratchpad views retain the old bridge. Real physical modifier drag/resize, multi-output transitions, focus and popup clipping need integration testing.
- Native group pairing is exposed through the existing local same-user Sway IPC privilege model. For stronger credential custody or a future multi-tenant model, replace this with a private authenticated protocol, not untrusted window text.
- Generic IPC move/resize commands still bypass the native pointer seatop; the Social bridge must not issue follower commands while native active. Explicit group release is required on mode switch/hide.
- This branch changes focused compositor files but does **not** merge or deploy. The operator assigned integration lead and prioritization; MapLibre/Weather workers' unrelated source remains intact. Keep branch integration and conflict checks current.
- The UI's dimensions/rail/Settings++ are unchanged. `clear` and the existing 8ms fast-follow remain rollback.

## CHECKLIST

- [x] Operator delegated Stage Director integration leadership, with MapLibre coexistence as priority
- [x] Recheck actual Social W FloatingWindow type, title, bridge and Stage Director SwayPX build paths
- [x] Implement production-linked native group command and move/resize seatop update
- [x] Native identity/generation, mapping lifetime and unmap/destroy invalidation
- [ ] Production SwayPX CI and headless native pointer + pixels
- [ ] Wire Social helper to native pair after verified docking and fallback on failure
- [ ] Merge/deploy only under operator's separately authorized acceptance path
- [ ] Live actual Quickshell + native Vesktop visual and modifier drag/resize acceptance

## NEXT

Finish the bridge PR, build and run real headless SwayPX tests, repair actual compilation/runtime issues, and preserve a precise rollout/rollback command. Do not claim finished zero-lag behavior without actual deployment acceptance.
