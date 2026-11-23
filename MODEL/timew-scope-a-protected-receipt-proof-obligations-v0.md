# SwayPX // Scope A selected XDG and protected native receipt proof obligations (v0 proposal)

**PATIENT //** Post-Apollo-SwayPx  
**ROOM //** TimeW Scope A compositor selected-window/transaction/settlement  
**STAFF ROLE //** SwayPX DOC, external Hospital-compatible; not verified Hospital-registered  
**BASE //** [Nurse-2-reviewed SwayPX #182](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/182) at `dd62844e251435800f596365f6844322d3747757` (open draft)  
**COUNTERPART //** [TimeW native receipt RFC #2272](https://github.com/kudokudo1/The-Post-Apollo-Project/pull/2272), [TimeW coordination #2067](https://github.com/kudokudo1/The-Post-Apollo-Project/issues/2067)  
**STATE //** **SwayPX REVISE / PROPOSAL ONLY / NOT A CANONICAL RECEIPT / production Restore HOLD**  
**NO SOURCE PROMOTION //** This document does not define a signed wire version, accepted receipt, physical input trust mode, grant, native privileged command, or future compatibility guarantee

## IMPLEMENTATION // What is actually observed

The SwayPX draft stack (still unmerged) currently demonstrates *diagnostic*, compositor-local source behavior:

| Existing SwayPX witness | Real source fact supported | What it cannot establish |
| --- | --- | --- |
| #170/#172 | Exact real XDG map/unmap/destroy and revoke-before-free listeners | Human intent or authorization |
| #173 | Finite versioned binary64 values for logical x/y/width/height including `y=92.5` | Output-safe physical placement, signer or wire bytes |
| #174 | `pending` vs `current` state and native transaction timeout semantics | An ACK or final presented frame |
| #176 | Real native configured READY versus real forced timer TIMEOUT; both can precede current-copy/scene arrangement | Trustworthy terminal resize |
| #178/#179 | Process-local boot/lifetime, matching real mapped XDG and a separate per-instruction diagnostic number minted at actual `transaction_commit` | Enrolled/portable selected object or signed transaction ID |
| #182 corrected | Real per-instruction READY/TIMEOUT/CURRENT/SCENE logs, parent BIND chronology; 28 negative tests; conflicting READY+TIMEOUT is `UNKNOWN/HOLD` | Actual protected client ACK, operator-selection, settled per-view presentation, native permission |

In particular, #182's **normal** diagnostic run has 6 instructions and 2 qualifying callback correlations, while its forced real-timer run has 5 instructions and **3 contradictory READY+TIMEOUT cases**, **zero clean outcomes**. Calling all three successful timeouts would be false. `transaction_apply` can update `current` after timeout; `arrange_root` is not a terminal frame receipt. An internal configure serial, a transaction pointer, an app_id or a process-local counter is not a credential.

TimeW independently owns the untrusted value/review consumer ([#2207](https://github.com/kudokudo1/The-Post-Apollo-Project/pull/2207), [#2260](https://github.com/kudokudo1/The-Post-Apollo-Project/pull/2260)), the deny-only lineage shadow (#2272), and ordinary local fractional captures (#2284). DOC 5 owns local C5 proof/quarantine and the local interlock work; none supplies independently protected one-use consent. TimeW's existing V1/V2 integer-only checkpoints/journals cannot be silently widened or rounded from binary64.

## HOW TO USE // Proposed native per-step proof boundaries

This is a **SwayPX-side ownership proposal for joint review**, not a named IPC method or packet. The labels below name needed evidence and possible outcomes, not active APIs.

| Source boundary | SwayPX must eventually be able to prove | Required counterpart | Absent/uncertain result |
| --- | --- | --- | --- |
| `ARM/SELECT` | Explicit, intentional operator-selection gesture on the exact currently mapped, floating, nonfullscreen XDG via the approved seat; independently verified input origin and live root resource | C3e/C4b and Company Map/input owners agree gesture/seat source policy | `NO_PROTECTED_SELECTION_HOLD` |
| `LIFETIME` | Compositor process boot epoch, a fresh private selected view/root generation and synchronous unmap/destroy/replacement revocation; no `con_id`/PID/title/app_id/pointer-as-key | C3e/C4b enrolled actual installed source release/process/host/seat | `NATIVE_IDENTITY_UNENROLLED_HOLD` |
| `CONTEXT` | Active seat/session/lock, output/workspace identity/epochs, output logical position, scale and transform, topology drift cancellation | SwayPX output/seat and C3e/C4b trust owners | `CONTEXT_DRIFT_HOLD` |
| `INTENT` | Immutable checkpoint/window/operation/step, exact logical binary64 before and desired resize/move target, and selected lifetime binding | TimeW owns original checkpoint; DOC 5 owns operator-visible exact intent | `INTENT_UNVERIFIED_HOLD` |
| `PRE-ACTION SPENT` | Independently verified one-use C5 record for the *entire* resize-then-move operation, committed **before the first effect** | C5/DOC 5, independently protected current-head custody and revocation precedence | `SPEND_UNKNOWN_HOLD` |
| `RESIZE DISPATCH` | Single compositor guarded mutation on selected live object, same exact expected CURRENT, issue/commit generation and operation nonce; capture real post-resize x/y/w/h since centered resize can shift x/y | TimeW expected before/target; C3e/C4b protected dispatch channel | `RESIZE_OUTCOME_UNKNOWN_HOLD` |
| `RESIZE OUTCOME` | Actual configure ACK classified distinctly from timer-forced `CURRENT`, scene arrangement and individually attributable rendered/settled selected-view frame, before follow-on move | SwayPX + C3e/C4b evidence provenance and TimeW receipt consumer | `RESIZE_NOT_PROTECTED_SETTLED_HOLD` |
| `MOVE DISPATCH` | Same selected lifetime, already verified protected post-resize rectangle, same spent operation, new step/dispatch ID, unexpired epochs and output constraints | TimeW consumer, C5 single operation, C3e/C4b source | `MOVE_BLOCKED_HOLD` |
| `FINAL OUTCOME` | Exact logical post-move CURRENT and output/scale/transform and distinct terminal selected-view presentation, with independently protected step receipt | TimeW + C3e/C4b signed receipt, separately reviewed target matching | `FINAL_OUTCOME_UNKNOWN_HOLD` |
| `CRASH/REVOKE` | No stale object or in-memory counter survives source restart, lost ACK, unmap/destroy, lock, canceled gesture, seat/output drift or lease expiry | TimeW and C5 durable read-only reconciliation | `UNKNOWN/HOLD`; no retry/undo/regrant |

**Important: `RESIZE OUTCOME` must come before `MOVE DISPATCH`.** The SwayPX floating resize path can center by shifting the original x/y. TimeW may not reuse the old rectangle or synthesize new coordinates from the requested width/height. Even matching final binary64 values alone are not proof that the right XDG surface was affected or its last frame presented.

### Proposed field families for cross-owner discussion, not a wire format

- **SwayPX-native private identity:** compositor boot nonce, exact in-process view/container/root wl_surface lifetime, fresh monotonic candidate and native *actual instruction* generations; never export raw pointers or trust source-supplied counters.
- **Selected context:** seat/session/lock generation; output/workspace/scale/transform epochs and policies; selected floating XDG status and unmap/destroy/revoke observation.
- **Exact geometry:** versioned binary64 logical x/y/width/height for before CURRENT, desired target, actual post-resize CURRENT and post-move CURRENT, with strict phase semantics. IEEE754 numbers are values; C struct layout is not a canonical network byte stream.
- **Operation and step:** one complete immutable TimeW checkpoint/window intent, one C5 spend for resize+move, separate one-use native dispatch ID and selected lifetime-bound transaction instruction for each step.
- **Observation result:** distinguish `CLIENT_READY_REPORTED`, `TIMER_FORCED_APPLY`, `CURRENT_COPIED`, `SCENE_ARRANGED`, `SELECTED_FRAME_SETTLED` and `REVOKED/UNKNOWN`. **Only the first four are currently exercised by SwayPX diagnostics; selected-view frame settlement is not implemented.**
- **Independent custody:** C3e/C4b issuer/release/host/seat/challenge and signed receipt envelope; C5 durable antirollback/revocation/SPENT; TimeW verified consumer and crash-reconciliation journal. These components **must not be minted inside SwayPX's temporary CI code**.

The eventual wire schema needs a joint decision for **version, exact field order, lengths, endian/IEEE754 encoding, domain separation, audience, nonce, challenge, release floor, duplicate/unknown-field refusal, key rotation and revocation**. These values are *questions to be agreed*, not accepted packet field names or signing claims.

## VERIFY // Deny-only cross-owner proof inventory

New test-only `tests/timew/ci_scope_a_proof_obligations.py` reuses the already-reviewed #182 diagnostic parser rather than fabricating a new native lineage issuer. It accepts **untrusted log text**, reports what can be classified as diagnostic-only, explicitly records the missing proof families and always returns `safe_actions=[]`, `protected_receipt=None` and false source/selection/ACK/frame/C5/resize/move/retry/undo flags.

It intentionally refuses to interpret a caller's fake `claimed_prior_resize_receipt` as a protected resize result. A simulated MOVE after perfect-looking READY/CURRENT/SCENE is still `MOVE_BLOCKED_WITHOUT_PROTECTED_RESIZE_RECEIPT_HOLD`. A forced mode with real READY+TIMEOUT contradiction is `CONTRADICTORY_NATIVE_READY_TIMEOUT_HOLD`. A forged late parent BIND, missing native events, malformed log or mismatch is also HOLD. This is a **reference proof-gap test only, not an accepted cross-process SwayPX or TimeW parser**.

The dedicated GitHub CI compiles Python, reruns #182's original 28 hostile chronology checks and the new receipt proof-gap denial tests. It must not inject or run a new native command or claim that a source-only passing test has established per-view presentation.

## WATCH OUT FOR // Ownership / approved direction

- **Operator-approved storage direction:** ordinary TimeW data local now; Post-Apollo NAS and separately protected custody hardware later. A current local SQLite capture, local C5 `SPENT` fixture or an old development grant does not become genuine protected consent merely by matching values or moving to NAS.
- **Autoscroll:** OS-managed mouse input first, optional advanced enrolled-device verification; independently owned G3G/G3H/G3I/G3J stack remains **HARD OFF**. This TimeW branch does not edit seat/input, libinput, cursor or company scene code.
- **Company Map #126:** shared seat/focus/output/scene hooks remain with its owner; the proposal does not assume authorization to modify them.
- **Scope B Brave:** identity, instance mapping, launch and tabs are a separate future assignment; do not infer them from XDG app IDs.
- No document, fixture or test in this PR defines a protected production command, real human selection, accepted time/scale policy, source signing key, final-frame terminal proof or completed native Restore.

## NEXT / JOINT ACCEPT-REVISE-HOLD

Each owner should respond to the **technical** question in [TimeW #2067](https://github.com/kudokudo1/The-Post-Apollo-Project/issues/2067), not re-ask the settled local/NAS direction:

1. **SwayPX / input owners:** specify accepted operator-selection gesture and exact live selected XDG + seat/output/workspace/scale/revocation witness. Shared hooks require explicit collision resolution before production edits.
2. **TimeW owner:** define correct interpretation of per-command resize receipt and actual post-resize x/y, final observation, timeout/unknown crash gaps and V1/V2 compatibility. A read-only local v3 preview is not source authentication.
3. **C3e/C4b:** choose installed source enrollment/challenge, signer, packet custody, current release/seat binding and rollback resistance. Do not reuse Hospital/Brave keys.
4. **DOC 5 / C5:** choose immutable visible exact intent, protected one-use PRE-ACTION SPENT, revoke ordering and independent rollback-proof reconciliation.
5. **Nurse 2 / operator:** independent exact-head scoped source review first, then separate merge, installed-host tests and production decision.

Until those contracts are accepted, the SwayPX DOC disposition on **protected native receipt remains REVISE**. **No protected selection, resize, move, retry, undo or Restore can be executed**.

**DECISIONS // production Restore HOLD · Autoscroll HARD OFF · source/CI-only.**
