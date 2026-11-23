# SwayPX / TimeW Scope A // actual native transaction callbacks (disposable CI v0)

**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`  
**ROOM //** TimeW Scope A compositor-native observation and transaction/geometry contract  
**STAFF ROLE //** SwayPX DOC owner, external Hospital-compatible; no verified live Hospital registration  
**ASSIGNMENT //** Verify the order of real XDG configure-readiness and real compositor timer-driven layout application without granting any native operation  
**SOURCE BASE //** [Reviewed #174](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/174), exact `15f345da991ec8cff3a8901142d652800ea97527`  
**STATE //** CI-only native diagnostic, source/CI review required, no protected source/selection/receipt, production Restore HOLD

## What the real compositor does

SwayPX `sway/desktop/transaction.c` already supplies actual code paths for:

- `transaction_commit`: conditionally issues `view_configure` and tracks `num_waiting` for views that need to acknowledge content-size changes. An XDG move with no size change may not require such a configure.
- `transaction_notify_view_ready_by_serial`: matches the compositor's **internal** instruction serial and calls `set_instruction_ready`. This is not a cryptographic or client-process attestation.
- `transaction_notify_view_ready_by_geometry`: optional legacy geometry match, also not protected.
- `handle_timeout`: actual Wayland event-loop timer callback, forces `num_waiting=0` even if a client has not acknowledged.
- `transaction_progress`: calls `transaction_apply` (copies instruction to `current`), `arrange_root` (scene positioning), then cursor rebase, transaction destruction and animations.

The source-backed danger is that **TIMEOUT can still lead to CURRENT_COPIED and SCENE_ARRANGED**. Neither implies an authenticated client ACK, exact selected-window final rendering, protected receipt, permission for TimeW's follow-up move, or a safe retry/undo.

## Source-only CI implementation

This increment adds four isolated files and changes no production native file:

1. `tests/timew/ci_inject_real_transaction_events.py` refuses to run outside ephemeral GitHub Actions. It temporarily injects **log-only diagnostics** into the existing real compositor paths `transaction_commit`, `transaction_notify_view_ready_by_serial/geometry`, `handle_timeout`, and the actual apply/current/arrange block. It temporarily adds two small test changes to the existing headless smoke: launching the native debug `txn-wait` mode for the timeout case, and using an ordinary untrusted `swaymsg resize set` against the **disposable foot actor only** to provoke a size-configure/ready event. No `sway/meson.build`, `sway/desktop/transaction.c`, input, resize, IPC or test-smoke changes are committed.
2. `tests/timew/ci_assert_real_transaction_trace.py` groups diagnostic events into **ephemeral intervals** bounded by each `COMMIT`. In normal mode it demands actual serial/geometry readiness before current-copy and scene arrange, with no timeout. In `forced_timeout` mode it demands an actual timer callback with an inflated debug waiting count followed by current-copy and scene arrangement. Pointer addresses are **not** treated as nonreusable identities. Both outputs always deny source/selection/C5/settlement/permission.
3. `.github/workflows/timew-scope-a-real-transaction-events.yml` builds the pinned native SwayPX compositor in GitHub Actions with temporary log-only injection and executes **two separate disposable headless compositor sessions**, one normal and one with SwayPX's own `-D txn-wait -D txn-timeout=180` debug settings, using pinned Mesa and a real native `foot` XDG actor.
4. This MODEL note records the exact scope, evidence, caveats, and cross-owner continuation. It is not canonical TimeW/Connections protocol.

The fixture's ordinary `swaymsg` resize is **not** an authorized TimeW Restore command and never touches the user workstation. It only causes a real native XDG configure event in a disposable CI compositor. `get_tree` and the actor's spoofable app_id select a fixture, not a protected live surface.

## Required pass criteria

- Exact CI head workflow and Grammar PASS.
- GitHub runner's normal XDG session actually hits `READY_SERIAL` (or real geometry-ready) followed in order by `APPLY`, `CURRENT_COPIED`, and `SCENE_ARRANGED` for one diagnostic transaction interval; no timeout on that interval.
- Separately, the real SwayPX event-loop timer actually hits `TIMEOUT waiting_before >= 1000000` in forced native debug mode, then `APPLY`, `CURRENT_COPIED`, and `SCENE_ARRANGED` for that interval. This is not fake invocation of the #174 fixture stage function.
- Explicit evidence marker `settled=0`, `presented=UNVERIFIED`, `operator_selection=NONE`, `source=UNENROLLED`, `c5_spent=NONE`, `native_move=OFF`, `native_restore=OFF`, `retry=OFF`.
- No permanent `transaction.c`, `view.c`, `seat.c`, `sway/meson.build`, `resize.c`, `move.c`, cursor handler, autoscroll or Company Map modifications. No production SwayPX activation, physical Fedora session or protected issuer.

## Limitations and next gates

This is a **native callback timing test**, not an actual-source TimeW receipt. Serial and raw transaction pointers are process-local, can be reused, and are not bound to an enrolled SwayPX producer, a selected view's lifetime, C3e/C4b release/process/host/seat evidence, C5 one-use SPENT, a source-signed operation, or an independently attested final output frame. `SCENE_ARRANGED` says only that the compositor's arrange routine ran. The forced timeout is a **deliberate debug-mode negative path** and cannot be presented as a normal user window's failure rate.

A subsequent independently approved design must establish a compositor-generated, nonreplayable transaction and surface lifetime correlation, output/workspace/seat/scale epoch, exact binary64 pending/current and **actual post-resize** geometry, configure-ACK-versus-timeout classification, and protected per-step receipt/settled frame with external trust/consent. If any source/step is unknown, TimeW must HOLD and never issue a second move, retry or undo.

## DECISIONS

SwayPX owner **REVISE** on trusted native receipt. TimeW additive fractional value #2207 is merged and Nurse-2-reviewed retrospectively; it is not an approved wire or Restore consumer. C3e/C4b producer enrollment and C5 durable one-use spend remain separate. Company Map #126 and Autoscroll #158 own shared input/seat/transaction/epoch code; this branch changes none of those files. **Autoscroll HARD OFF. Production TimeW Restore HOLD.**
