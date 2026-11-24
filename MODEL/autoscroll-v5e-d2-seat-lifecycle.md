# SwayPX // V5E-D2 Real Seat Lifetime Hooks (HARD OFF)

STATE // Separate draft stacked on V5E-D1. Real seat source callbacks; no live activation, merge, or host writes.

## HOSPITAL
PATIENT // Post-Apollo-SwayPx
ROOM // Mouse & Keyboard · Universal Autoscroll V5E-D2
STAFF // External Hospital-compatible DOC; no verified live Hospital registration
BASE // V5E-D1 PR #152, exact parent ef92eaa26ee789bfd446fa67396e580a919268ab
AUTHORITY // Operator authorized narrowly separated shared-input changes while Company Map PR #126 owner is paused, provided the returning owner receives a durable GitHub handoff. No general Map ownership transfer or production approval.

## IMPLEMENTATION
- The real sway_seat object now stores its own PA_LIFETIME_SEAT token slot.
- Real seat_create() binds a fresh token using the existing single input manager issuer, after successful seat and cursor creation but before destroy-listener registration or insertion into the input-manager seat list. This covers default seat0 and transient seats, which also call seat_create().
- Real handle_seat_destroy() revokes that token before any focus reassignment, device detachment, cursor destruction or memory release.
- A read-only seat_autoscroll_lifetime_epoch() accessor returns zero for absent/revoked seats; it must be called only while a sway_seat pointer is still valid.
- A missing/uninitialized/exhausted issuer yields zero without interrupting normal compositor seat creation.
- The V5C object guard is still a separate unmerged branch; no real input authorization, autoscroll activation, new IPC, pointer consumption or axis dispatch is added.

## OWNER SPLIT
D2 changes only two production source files: sway/input/seat.c and include/sway/input/seat.h. Both overlap paused Company Map PR #126. D2 does NOT change cursor.c, meson.build, Company scene/focus/input, seatop_down.c, transaction, xdg_shell or outputs. Other modified files are tests, this document, and inherited CI allowlist updates confined to this child PR. The Map owner retains their work and should inspect these exact two files before rebasing.

## VERIFY
Dedicated exact-head D2 workflow covers strict diff scope, real seat callback order, input manager initialization ahead of seat0 creation, transient seat path, C11 lifetime revocation/overflow, existing device and cursor regressions, and full Nix-built SwayPX. Source-level callback integration plus compilation is not a dynamic witness of runtime seat create/destroy events. Real hardware authenticity is not proven.

## WATCH OUT FOR
Seat lifetime tokens establish only object continuity. They are not credentials, physical mouse proof, or reusable across compositor sessions without a separate trusted session identity. Device and seat are two of four required object domains. Actual target, output and layout epochs remain unwired.
Production remains PA_AUTOSCROLL_LIVE_ACTIVATION 0 with physical_verified=false, native_scroll_lock_active=true and no proven layout generation.

## CHECKLIST
- [x] Recheck current source, canonical Syntagma and Company owner state
- [x] Bind/revoke real compositor seat token in own callbacks
- [x] Keep scope to two shared seat files in separate draft
- [x] Add exact-head CI and source ordering tests
- [ ] Pass every relevant CI workflow
- [ ] Notify Map PR #126 with exact head and two-file overlap
- [ ] Independent review, merge authorization and operator final acceptance
- [ ] D3 real target/output lifetimes and distinct hardware authority proof
- [ ] Dynamic lifecycle event trace, compositor-owned axis, Escape and indicator acceptance

## NEXT
After verification, record test evidence and recovery pointers on both PRs. D3 must separately coordinate target/view and output owner callbacks. Do not automatically merge or activate input.

## DECISIONS
DRAFT · UNMERGED · PRODUCTION OFF · NO LOCAL MACHINE WRITE · COMPANY MAP OWNERSHIP PRESERVED.
