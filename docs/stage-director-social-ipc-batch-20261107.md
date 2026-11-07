# Stage Director // Social W single-IPC paired transaction witness

**STATE // CI-only real Wayland transaction experiment. NOT native drag/resize synchronization.**  
**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`. Depends on Social geometry PR #94 and real window baseline PR #95.  
**ROOM //** Social W unified external app grouping, browser Hospital-compatible only. No live Room/Staff assignment verified.  
**OPERATOR GOAL //** Social W chassis and Vesktop move and resize without visible lag regardless of initiation method.

## IMPLEMENTATION
Actual `sway/ipc-server.c` runs `execute_command(buf, NULL, NULL)` for the whole IPC command payload and calls `transaction_commit_dirty()` **once** afterward. `sway/commands.c`, `sway/commands/move.c` and `sway/commands/resize.c` do not call that transaction function. This permits a native compositor preflight to stage both windows' pending rectangles before the transaction is submitted.

The stacked CI run takes the verified mapped synthetic Social chassis and guest from PR #95 and submits *one IPC packet per paired move/resize*: host resize/move; guest resize/move; exact guest focus. It verifies all command results, exact real Sway rectangles, and real compositor PPM screenshot pixels for three different group sizes and locations. A stale guest identity is deliberately replayed and must be rejected before any command. The unrelated control remains unchanged.

**Critical difference from the actual fix:** Native modifier-drag still goes through `sway/input/seatop_move_floating.c`, and native modifier-resize goes through `sway/input/seatop_resize_floating.c`. They currently update only the selected container and commit. This experiment never modifies those shared files, installs compositor hooks, or verifies the frame timing of interactive pointer movement. One IPC packet from an external script is **not** a compositor-authenticated group operation, and it must not be promoted into a public IPC window-group API.

## VERIFY
CI requirements: build and run the actual SwayPX software compositor, genuine Wayland XDG clients, three batched transformations, exact-state checks, sampled frame pixels, stale ID negative case, proper CI `/dev/udmabuf` temporary ACL teardown. Source-level contract guards enforce one IPC commit after command execution and no nested move/resize commit. No false claims from synthetic geometry alone.

## WATCH OUT FOR
Only composited *settled* pixels are sampled. Client content redraw may lag. A genuine zero-gap pointer drag needs compositor-side authenticated pairing and updating both native scene/container states before the seatop's transaction commit, with teardown, cross-output, popup, input and frame-by-frame tests. The Company M30 / Weather/Chrome owned compositor paths remain untouched.

## NEXT
Share the measured paired batch behavior with the current SwayPX Stage Director owner and obtain seam ownership before authoring a CI-flagged native seatop group transform. Do not overwrite the active Company M30 compositor stack. Preserve Social's external-focus activation and current geometry bridge fallback.

## DECISIONS
Pure experiment only; no PR merge, live desktop change or production activation. This document is a recovery pointer, not an authorization of arbitrary window grouping.
