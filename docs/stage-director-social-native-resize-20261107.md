# Stage Director // Social W native resize precommit experiment

**STATE // DISPOSABLE CI-ONLY NATIVE MOVE+RESIZE TEST. Not a production change.**

**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`, dependent on Social W owned in `kudokudo1/The-Post-Apollo-Project`.  
**BASE //** SwayPX draft PR #98 native move proof, stacking on Social real Wayland PR #95.  
**ROOM / STAFF ROLE //** External Hospital-compatible Social W assignment; live Hospital registration and compositor Staff Role unverified.

## IMPLEMENTATION

This branch extends the successful PR #98 **disposable source-copy** test. A GitHub runner copies the checked-out SwayPX tree into its private temp directory, validates exact anchor snippets, and overlays three **temporary** C files:

- `sway/input/seatop_move_floating.c` (existing test-only paired native movement)
- `sway/input/seatop_resize_floating.c` (new test-only paired native resizing)
- `sway/commands/seat/cursor.c` (CI-only bounded synthetic pointer step / start / finalization commands)

No production compositor C is committed or modified on the operator machine.

The test-only resize seatop validates an already-paired, mapped host and guest by exact live Sway IDs and PIDs; rejects stale, unverified, mismatched-workspace identities; and calculates the follower rectangle from the leader's **native pending geometry**, with Social rail width=110 and inset=1. It mutates both pending node rectangles, including their content boxes, invokes the peer's normal `arrange_container()`, and reaches the existing `transaction_commit_dirty()` only afterward. This should produce one compositor-owned transaction for both members.

The CI client then tests bottom-right native resize in both directions using genuine mapped Wayland XDG windows, exact geometry assertions, actual compositor screenshot pixels, unrelated-window immobility, existing bidirectional move tests, and stale guest lease rejection.

## VERIFY

Passing requires a rebuilt private SwayPX compositor, its native seatop hooks, the actual client content rendering and all exact geometry checks. Test-only cursor motion dispatch exercises real seatop handlers, **not a physical mouse**. It does not prove there is zero visible split on *every rendered frame*. A native Wayland client can require an extra frame to redraw content, which is separate from compositor border alignment.

## WATCH OUT FOR

This is not production security architecture. Its test-only lease file and fixed 1920×1080 output dimensions are not a real authenticated group API and must never be installed as an ordinary desktop change. Production Stage Director needs ownership coordination, native compositor-side identity and generation lease, revocation on unmap/output changes, real pointer/keyboard + popup/input containment tests, multiple displays and scaling, and a frame-by-frame rendered geometry witness.

## NEXT

Coordinate the compositor Stage Director owner to convert only the proven native pair transform into a properly feature-gated production implementation after native input/frame proof. Preserve the existing Social fast-follow fallback and do not merge or activate the compositor experimentally without operator authorization.

## DECISIONS

Keep the SwayPX and Social UI ownership boundaries; do not edit active Company/Weather compositor code in GitHub. No local files, Quickshell, Sway services, or installed binary changed.
