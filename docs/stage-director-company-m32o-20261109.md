# 🏥 Company M32o // Native scene lease candidate lifecycle

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
ROOM // Company native MapLibre embedded in Company → COVERAGE → VECTOR VIEW
ROLE // External Hospital-compatible Nurse 2 continuity; live Hospital registration unverified
ASSIGNMENT // Safe, bounded lease-candidate state machinery, no issuer, no scene activation
BASE // M32m/n draft PR #129, itself stacked on M32d draft PR #126
STATUS // Draft, production OFF, no physical operator acceptance

## IMPLEMENTATION

The existing production source (M24/M27/M28b/M29/M30a/M32a/M32b) has **selected native Company**, Wayland intent, viewport, Linux pidfd child lifetime, installed executable inode, trusted digest and exact XDG actor checks. The M32m admission preflight intentionally returns `OPERATOR_LEASE_ABSENT` even when these observations match. The digest is still unpinned.

M32o adds a **candidate-only**, compositor-built lifecycle modeled on the existing Weather NEWS candidate lease ledger. It is not an operator ticket or scene lease:
- One reserved candidate per strictly increasing physical selection epoch; canceled/expired epochs never replay.
- Snapshot includes exact native host layer/surface/client, output, actor view/root/client, process PID and start ticks, launch intent sequence and host revision.
- Reservation requires all supplied, compositor-observed evidence conditions; this module does not derive them from QML, IPC, app-id or titles.
- The candidate expires within **at most five seconds**, using future compositor-owned monotonic time. Zero, backward, expired and overflow timestamps fail closed.
- Every probe rechecks the entire snapshot and selected-host, live intent/viewport, kernel pidfd, ELF inode/digest and mapped output/actor evidence. Any mismatch or missing evidence irrevocably clears the candidate.
- Host, actor, output and exact PID/start-ticks loss revoke the candidate; unrelated old event references cannot revoke a newer one.
- Every probe receipt, **including an exact synthetic match**, has `operator_authenticated=false`, `launch_authenticated=false`, `scene_lease_issued=false`, and all native scene/input/keyboard/popup permissions false.
- No production caller, scene setter, Wayland global, IPC, QML hook or authorization issuer is added. The normal build includes the inert state machine but cannot activate it.

## HOW TO USE

No current desktop operation. The live Company VECTOR VIEW still uses its existing disabled bridge. Running these tests does not authorize a map window. No installation or restart is part of this PR.

## VERIFY

`tests/stage_director/test_company_m32o_scene_lease_candidate.c` is compiled with strict C11 warnings and independently exercises exact identity changes, all eight prerequisite flags, one-time epochs, expiration boundaries, zero/backward/overflow time, explicit revocation, unrelated-loss refusals, exact PID/start-ticks matching and zero authority bits. Conditions that would require a real pinned digest are explicitly **synthetic test fixtures only**.

`.github/workflows/stage-director-company-m32o.yml` includes source/authority checks, native C unit tests and normal/private SwayPX builds. It blocks any scene-apply call or public Wayland role in this module.

## NEXT // Physical acceptance plan (NOT yet authorized or performed)

The M24 selection producer requires **genuine physical libinput**, with the optional pairing feature enabled in an operator-approved installation: Ctrl+Super+Shift+F11, followed within three seconds by a left click on the actual mapped Company overlay. This selection only publishes the private Company protocol, NOT a scene. Virtual pointer/IPC, synthetic environment flags and a matching app-id are not substitutes.

When the operator explicitly approves a disposable Fedora acceptance session, the scoped test must:
1. Record current SwayPX executable/build, both output identities, active workspace, Company surface and a rollback to the original compositor. Do not modify production before that approval.
2. Use a genuinely mapped Company layer and the physical gesture to select that exact Wayland client; independently verify selected host, intent and viewport on the native protocol.
3. Exercise CLOSE, explicit protocol DESTROY, client disconnect, and physical Company host/layer removal with a live native Qt actor. Confirm scene revocation precedes protocol/intent/pidfd cleanup, Qt cannot leak as ordinary floating, and unrelated/stale bindings cannot revoke newer sessions.
4. Verify both outputs and input containment, restore previous desktop state, and report whether the operator accepts the behavior.
5. Only afterward, in a **separate authorized task**, decide an executable hash pin, authenticated launch custody, a trusted compositor-only operator gesture/issuer, and a short-lived actual scene lease. This M32o candidate never becomes such an issuer automatically.

Do not interpret this document as permission to enable the pairing feature, change Fedora, load CI flags locally, merge any PR, or activate Company native embedding.

## WATCH OUT FOR

The candidate evidence fields are plain structs for a future **trusted internal compositor caller**. They do not authenticate their own origin or prove kernel pidfd validity. The **M32m preflight and future issuer must do that on each scene operation**. Five seconds is a maximum local candidate lifetime, not authorization. This module is safe because it never returns a positive scene permission. The current real source digest is empty, so normal real observations cannot satisfy an exact-digest candidate gate.

Production readiness additionally needs approved executable/dependency measurements, real launch custody, lease issuer and revocation, Company UI screen selection, hardware hot-unplug/scaling, and operator visual acceptance.

## DECISIONS

Keep a separate draft stacked on #129. No merge, hash pin, runtime activation, local desktop changes or claimed live Hospital registration. The operator retains final acceptance authority.
