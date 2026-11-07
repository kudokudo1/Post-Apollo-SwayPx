# Stage Director // Social W disposable native move proof

**STATE // TEST-ONLY PROTOTYPE. Not production code; native resize and live acceptance outstanding.**  
**PATIENT //** `kudokudo1/Post-Apollo-SwayPx` (compositor), with `kudokudo1/The-Post-Apollo-Project` Social W owner.  
**BASE //** SwayPX Social real-window branch (PR #95), after grouped geometry PR #94 and batched IPC PR #97 merged into their respective bases.  
**ROOM //** Social W unified window movement, external Hospital-compatible only; Staff Role and live Hospital registration unverified.  
**OPERATOR GOAL //** Chassis and Vesktop must behave like one window when moved or resized from either surface, regardless of how applications opened.

## IMPLEMENTATION

This PR adds **only files under `tests/stage_director/`, `.github/workflows/` and `docs/`**. In GitHub CI, `ci_social_native_move_overlay.py` copies the checked-out source into a private temporary directory, checks exact source anchors and makes *two changes to that disposable copy only*:

1. Temporarily instrument `sway/input/seatop_move_floating.c` so a narrowly leased, identified host/guest pair receives the same pointer-motion translation before the existing `transaction_commit_dirty()`. Pair IDs and process PIDs are independently checked against live mapped Sway containers, with exact same-workspace, floating, unambiguous and live-surface guards. A peer-unref callback invalidates the pointer.
2. Add a test-only cursor command in the copied `sway/commands/seat/cursor.c` to start the **real Sway floating move seatop**, deliver bounded pointer steps through `seatop_pointer_motion()`, and stop it. The command exists only in the ephemeral CI binary. This avoids the need for an unsafe global new IPC API or physical input devices.

The entire build is isolated under `/home/runner/work/_temp`, requires `GITHUB_ACTIONS=true`, a dedicated native CI flag, unprivileged UID, and ordinary test fixtures. The device wrapper snapshots/restores `/dev/udmabuf` access. GitHub's checkout and production compositor sources remain unmodified.

## VERIFY

The runtime harness starts genuine synthetic native Wayland XDG windows in headless SwayPX and asserts both leader directions:

- Host-led test motions update both mapped rectangles and real compositor pixels.
- Guest-led test motions update both mapped rectangles and real compositor pixels.
- A third, unrelated window never moves.
- A stale guest PID lease is denied and cannot move the guest.
- Sway logs demonstrate the follower was moved **before** the seatop's existing commit.
- CI temporary ACL is fully restored; exact checkout has no source diffs.

**Limitations:** The test-only cursor entrypoint calls Sway's native seatop motion routine, but does not prove physical mouse-button initiation, compositor frame-by-frame pixel continuity, actual Quickshell/Vesktop identity, cross-output behavior, clipping/popups, or native resize correctness. Avoid describing this as a finished zero-lag drag and resize fix.

## WATCH OUT FOR

The disposable C overlay contains proof-oriented guards and **must never be copied into production unmodified**. Real Stage Director pairing must be authenticated with compositor-side surface identity and a revocable generation lease, not an operator-writable file or app ID. Active Company M30 and Weather/Chrome compositor owners retain production source authority. No owner was assigned by this PR.

## NEXT

If the native seatop move CI passes, coordinate the SwayPX Stage Director owner to design an opt-in compositor-managed group with real native input pairing and frame-level rendering tests. Only after move is proven should resize be introduced, with client redraw/constraints and focus/input regression verification.

## DECISIONS

This is an isolated CI experiment and not a shared compositor source change. The existing Social geometry bridge remains the fallback. No PR merge, local deployment, Quickshell restart, or operator acceptance is implied.
