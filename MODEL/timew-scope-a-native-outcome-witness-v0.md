# SwayPX / TimeW Scope A // Native per-instruction outcome witness v0

**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`  
**ROOM //** TimeW Scope A · selected XDG lifetime, real native instruction and transaction outcome  
**STAFF ROLE //** External Hospital-compatible SwayPX DOC owner; live Hospital registration unverified  
**BASE //** [Reviewed draft #179](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/179) exact `b6687e1d533115eeacf02f48cefbc5f624e894a7`, itself stacked on independently reviewed #178 and #176  
**PHASE //** Source-isolated GitHub-only native test, NOT a runtime feature  
**STATE //** SwayPX REVISE · TimeW Restore HOLD · Autoscroll HARD OFF

## IMPLEMENTATION // Actual native observations

Earlier #178 provided a process-local XDG diagnostic lifetime issuer; #179 observed six real `transaction_commit` view instructions across two mapped XDG lifetimes and minted separate diagnostic commit sequence numbers. Neither number is trusted authority. #176 separately showed that SwayPX may copy `current` and arrange its scene after a native timer timeout.

This increment composes **the same existing CI-only overlays** then instruments four distinct **real SwayPX callback sites in the disposable checkout only**:

1. **READY**: `transaction_notify_view_ready_by_serial`, before `set_instruction_ready`, checks the same test XDG view, original transaction pointer and exact nonzero internal configure serial.
2. **TIMEOUT**: actual `handle_timeout`, before forcing the transaction waiting count to zero. The second headless SwayPX session deliberately enables existing native `-D txn-wait -D txn-timeout=180`, producing a real negative case even if the client reported readiness.
3. **CURRENT**: immediately after real `transaction_apply` copies the queued instruction into compositor `current`. This event **can follow TIMEOUT**, not merely a successful ACK.
4. **SCENE**: after actual `arrange_root`, before the compositor starts animations. This is not an individual frame-presented or settled-view receipt.

The same disposable `sway/timew_scope_a_ci_real_xdg.c` test probe receives those callbacks through the new private CI-only header. It maintains a bounded in-process table keyed by **live** transaction pointer and the #179 diagnostic sequence, and includes only candidates matching #178's already mapped XDG view/container/root lifetime. A diagnostic pointer can be reused; it is not a persistent or protected identity. Each callback emits only the known local lifetime and commit sequence, stage, and explicit `receipt=NONE authority=NONE`. Real unmap records a seal and the parent #178/#179 listeners still revoke and check generation floors before container destruction.

The CI-only actor receives an ordinary test-only `swaymsg resize set` while mapped. That command is **not** TimeW authorization and never runs against the user's Fedora session.

The Python log checker independently correlates parent `LINEAGE_OPEN`, #179 `REAL_INSTRUCTION_BIND`, the new per-instruction stages, `OUTCOME_SEAL`, and actual unmap. It rejects forged positive authority, missing READY/timeout, duplicate/reused instruction sequence, reordering, stale lifecycle, inconsistent configure flags, incomplete seals, orphan callbacks and other malformed results. Standalone adversarial tests exercise these refusals before native CI.

## VERIFY // Expected pass criteria

- The dedicated offline log-contract job passes source-isolation, Python syntax and adversarial HOLD-only tests.
- The pinned native SwayPX/wlroots/Mesa/foot build compiles with disposable temporary callback injection and runs **two independent compositor sessions**, one normal and one using the real timer-forced debug path. Each session maps/unmaps two actual XDG actor lifetimes, observes parent #179 real instruction records and at least one matching configured instruction with the required normal READY→CURRENT→SCENE or forced TIMEOUT→CURRENT→SCENE sequence.
- The checker refuses to infer final presentation from SCENE. Normal READY remains an untrusted internal configure serial, not a C3e/C4b authenticated signed receipt. Forced mode remains `UNKNOWN/HOLD`, even when a later READY or `CURRENT` occurs.
- Original production `sway/desktop/transaction.c`, `sway/tree/view.c`, `sway/meson.build`, input/seat, cursor, resize, move, IPC and other owners' files remain entirely unmodified in the committed PR.

## WATCH OUT FOR

- Parent #178's at-map transaction counter and #179's at-commit diagnostic sequence are not nonreplayable protected operation credentials. The bounded in-process callback table is a **disposable test correlation**, not the final production memory/receipt design.
- Neither this test nor #176 proves actual selected-view frame presentation, correct output transform/scale and workspace-safe placement, or animation settlement.
- The operator's storage direction remains **TimeW normal data local first, separately administered NAS/protected hardware later**. Test-only local SPENT flags, same-user SQLite and in-process lineage must not be mistaken for independent rollback-resistant C5 custody.
- The separate Autoscroll workstream follows **OS-managed mouse input first**, with enrolled-device proof optional advanced; it remains HARD OFF. This SwayPX TimeW PR does not edit that owner's mouse/seat or Company Map code.
- Genuine selected operator gesture, enrolled source release/process/seat, exact canonical signed fractional operation/receipt wire, **durable one-use C5 SPENT before effect**, real ACK/timeout distinction and per-step settled final evidence remain distinct integration gates. Lost ACK, timeout, stale output, crash or unknown receipt = **UNKNOWN/HOLD**, with read-only reconciliation only; no automatic move, retry, regrant or undo.

## NEXT

Reconcile exact-head workflow evidence and request Nurse 2's independent scoped review of this new increment. Then coordinate with TimeW, DOC 1/2 C3e/C4b, and DOC 5 C5 owners on an accepted per-step native source/selection/receipt interface. Production callback linkage and installed Fedora use require separate owner/operator authorization.

**DECISIONS // No merge, no user workstation modification, no source/enrollment keys, no live native window movement, no consent spend and no production activation.**
