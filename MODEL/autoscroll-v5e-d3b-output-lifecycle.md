# V5E-D3B // Real Output Availability Lifetime, Production HARD OFF

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Mouse & Keyboard · Universal Autoscroll
STAFF ROLE // External Hospital-compatible DOC, no verified live registration
ASSIGNMENT // Distinct actual output enable/disable/begin-destroy lifetime hooks
BASE // V5E-D3A PR #154, exact head 97034fc65d96124d27fe9208cf8a36b05048cf4c
AUTHORITY // Operator authorized continuing narrowly split shared-input development while Company Map PR #126 owner is paused. This child does not take or modify the Map assignment, and is not a merge or activation approval.

## IMPLEMENTATION

An output is a monitor or other display that the compositor knows about. A display that is created can later be enabled, disabled and re-enabled without being freed. A monotonically increasing number tied only to its allocation would be insufficient: an old scrolling session might survive disable/reenable. This package binds an output token for **each successful enable interval** instead.

- Actual sway_output now stores a typed PA_LIFETIME_OUTPUT slot.
- Real output_enable() invalidates any prior slot and mints a new one from the existing input-manager session issuer before marking the output enabled or exposing it through root->outputs. It explicitly skips the fallback/NOOP output. An unavailable/exhausted issuer leaves token zero while ordinary SwayPX output management continues normally.
- Real output_disable() revokes the slot before removing the display from root->outputs, marking it disabled, destroying layers or evacuating workspaces.
- Real output_begin_destroy() revokes the slot again before the node-destroy signal, so a never-enabled or previously disabled output has no live token.
- Read-only output_autoscroll_lifetime_epoch() returns zero for absent, disabled, fallback, destroyed or revoked outputs. Call only while the sway_output object itself is live.
- This composes with the existing D3A window-target token, D2 seat token and D1 input-device token, all from one issuer. It does **not** join those into a trusted authorization or active scroll session.

## OWNER COLLISION

This child only changes two production files: sway/tree/output.c and include/sway/output.h. Both are **absent from Company Map PR #126's changed-file list**. No cursor, seat, Company scene/focus, compositor desktop/output.c, xdg_shell.c, transaction, seatop, Meson or output-layout callback is edited.

Inherited CI scope check updates on this child recognize the exact two output files and D3B tests without changing any parent PR branch. The Company Map owner retains their Room and receives a linked report on #126 as a durable record.

## VERIFY

Exact-head CI should check output-enable fresh issuance, same-object reenable invalidation, disable-before-workspace-evacuation, begin-destroy revocation before signal, fallback/inactive/null refusal and full native SwayPX compilation. The strict portable issuer test verifies monotonic tokens; native source check verifies hooks but is **not a dynamic monitor unplug/reenable trace**.

## WATCH OUT FOR

This proves only output AVAILABILITY lifetime. A monitor can move, change resolution, orientation, scale, or reassign its layout while staying enabled. This child does NOT increment a layout generation in those cases. A future D3C must track actual layout/configuration mutation and invalidate a live scrolling session on those changes; a mere geometry fingerprint is not a certified layout generation unless minted at trusted compositor callbacks.

Root fallback and headless output policies still need explicit final permission checks: a non-fallback enabled output token is not itself proof that a given target is visible on it, that the surface is allowed to scroll, or that the underlying pointer is physical.

Production continues PA_AUTOSCROLL_LIVE_ACTIVATION 0, physical_verified=false, native_scroll_lock_active=true, layout_generation=0 and no axis authorization. V5C epoch guard remains a separate unmerged draft. Tokens are session-local and require distinct restart identity for trusted use.

## HOW TO USE

Review draft and GitHub CI only. No local user settings, compositor restart, desktop file write or live input.

## CHECKLIST

- [x] Inspect real output owner code and existing D3A status
- [x] Issue output token on enable, revoke before disable/destroy
- [x] Keep D3B separate from D3A and Company Map source
- [x] Add output source tests and native compositor build CI
- [ ] Exact-head tests pass
- [ ] Company owner recovery comment at exact head
- [ ] Independent technical review and operator acceptance
- [ ] D3C geometry/layout mutation callback invalidation
- [ ] Complete physical origin, private epoch guard, native scrolling/ESC/indicator tests

## NEXT

Once this branch is verified, notify Company #126 that D3A/D3B each introduce zero new overlapping source files. Then review actual output-layout and configuration commit callbacks before changing any layout owner seam. Keep all PRs drafts until separate merge approval.

## DECISIONS

DRAFT, UNMERGED, PRODUCTION HARD OFF, USER MACHINE UNTOUCHED, MAP OWNERSHIP PRESERVED.
