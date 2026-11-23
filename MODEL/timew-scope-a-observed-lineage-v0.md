# SwayPX // TimeW Scope A process-local observed XDG lineage v0

**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`  
**ROOM //** TimeW Scope A native selected-surface, lifetime and transaction identity  
**STAFF ROLE //** SwayPX DOC owner, external Hospital-compatible `OPEN AI // CHATGPT`; live Hospital registration unverified  
**STATE //** **REVISE, isolated source/CI only, no production linkage, production Restore HOLD**  
**STACK //** Based directly on reviewed [#176](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/176), exact `74a4c5fe240f981a8a6556e877264fb28f1c52e6`; parents #174/#173/#172/#170/#169/#168 retain their exact-head scoped reviews  
**TRUST //** No physical selection, C3e/C4b installed producer, C5 durable SPENT, protected native commands or trusted receipts.

## Source problem and existing composition

`#168` correlates candidate snapshots but previously received **synthetic caller-supplied epoch/generation/nonce**. `#169` observes real mapped floating XDG root under a seat cursor, `#170` revokes on native unmap/container/root destroy, `#172` and `#173` prove lossless fractional `PENDING` geometry and real unmap callbacks, `#174` models pending/current/ready/timeout/scene and `#176` proves actual compositor timeout can apply current without client ACK. These are compatible pieces, not a protected selector or authorized Restore.

This increment replaces only one weak **diagnostic identity issuance premise**: issue distinct process-local lineage tags from inside a native process rather than accepting arbitrarily repeated test counters from callers. It does **not** retrospectively upgrade previous evidence or replace their legacy ledger.

## Source-only implementation

- `include/sway/timew_scope_a_observed_lineage.h` and `sway/timew_scope_a_observed_lineage.c` implement a private issuer. It obtains a 128-bit random **boot correlation tag** using Linux `getrandom(2)`, with no PID/clock/fixed-number fallback, then monotonically increments 64-bit **lifetime** and **transaction diagnostic** sequence floors. A shortage of entropy or counter exhaustion refuses new observations. Tags are **not** cryptographic authorization tokens, independently registered release identities, signed wire IDs or protected receipts.
- An observed candidate is bound to the exact in-process view/container/root Wayland resource/client/seat/output/workspace references, caller-provided seat/session/output/workspace epochs, current output scale/transform bits and #173 exact binary64 `PENDING` rectangle. If any tracked reference/epoch/scale/transform/value changes or the observation becomes unmapped, locked, destroyed, non-XDG, fullscreen or nonfloating, the candidate and its linked diagnostic transaction are **revoked**. A stale token cannot cancel a newer generation even if a native pointer is reused. Internal sequence floors remain after revocation; no counter wraps.
- `begin_transaction` may mint one local **correlation number** tied to the candidate; it is **not called from actual `transaction_commit`** and does not prove XDG configure ACK, settled current, source, C5 spend, movement or a real transaction instruction's identity. There is no reset/advance into a successful command.
- `tests/timew/test_timew_scope_a_observed_lineage.c` uses strict + UBSan Linux builds and hostile cases: two separately randomized boot sources, pointer reuse and monotonic generations, stale/replayed candidate and transaction tags, one-ULP `92.5` drift, seat/session/output/workspace and scale/transform drift, invalid geometry and output bits, overflow and irrevocable shutdown. Every assessment hard-codes all authority/resize/move/receipt flags **false**.
- The **disposable GitHub-runner-only** `tests/timew/ci_overlay_observed_lineage.py` composes with reviewed `tests/timew/ci_inject_real_xdg_lifecycle.py`. It temporarily adds this C source to the runner's Meson, and attaches mint/probe/revocation calls to the already instrumented real `foot` XDG map and unmap callbacks. Native compositor references and exact fractional geometry come from the real view/resource/seat/output/workspace objects, while seat/output/workspace **epoch numbers remain explicitly CI fixture values** until independently generated, owned and trust-bound.
- The real-native test must observe two separate mapped/unmapped XDG windows with `lifetime_sequence=1,2` and `transaction_sequence=1,2` in the **same headless compositor process**, and must revoke each on real unmap before container teardown. It logs no random boot bytes or native addresses. The logged tags are data correlation only.
- `.github/workflows/timew-scope-a-observed-lineage.yml` runs strict+UBSan hostile C tests and the original pinned real SwayPX/wlroots/Mesa/foot headless pipeline using CI-only source modifications. The GitHub branch **does not edit production `sway/meson.build`, `sway/tree/view.c`, `sway/desktop/transaction.c`, `seatop_default.c`, or move/resize/IPC**.

## What a PASS would and would not establish

| Evidence | May say | Must not say |
| --- | --- | --- |
| Linux `getrandom` + monotonic generation | This process-local issuer did not reissue its earlier diagnostic tags during this run | Installed compositor enrollment, externally protected boot identity, replay-proof wire credential |
| Real mapped `foot` XDG twice and actual unmaps | The private model correlated two distinct live native callback lifetimes and revoked each before native destruction | Operator physically selected either window |
| Current window refs + binary64 geometry | In-process objects and exact `PENDING` values matched the local snapshot at that callback | Values were signed, allowed inside output bounds, transaction-settled or a Restore target |
| Local transaction sequence | A local generation number was assigned once for correlation | It is an actual `transaction_commit` instruction ID, client ACK or source-signed receipt |
| CI compiler and real compositor | Source compiles and the isolated fixture can observe native events | Safe or installed production compositor runtime |

**The older #170 checked `double`→`int32` bridge still denies `y=92.5`.** The legacy #172 synthetic 1×1 ledger remains event-only, not target geometry. This increment uses #173's **actual exact fractional** `PENDING` values in a separate candidate-only lineage.

## Cross-owner remaining gates

**SwayPX:** A genuine explicit operator-triggered selection gesture owned by the compositor, with physical input provenance and revocable seat/lock/lifetime, must be separately accepted before any candidate is called *selected*. The current native test filter is a spoofable `app_id`, not human selection. A real compositor-owned transaction generation must be assigned at an exact transaction instruction hook, with ready-vs-timeout and output/scene/final presentation evidenced independently. Output/workspace scale/transform epochs must be minted and revoked by the owning native paths. Matching pointers or counters alone will never authenticate a process/seat/source.

**TimeW:** Keep the merged #2207 exact binary64 in-memory value lane separate from the existing integer-only V1/V2 checkpoint/journal. A versioned *canonical signed* before/target/post-resize wire protocol and terminal receipt must be jointly accepted before native consumer use.

**C3e/C4b:** Independently enroll release/process/host/seat producer and protected challenge/current revocations; process-local random bytes are not external custody.

**C5:** Bind exact human-visible original/presented before/target, selected lifetime and source epoch to one grant, durably SPEND before first side effect, and reconcile ambiguous dispatch as UNKNOWN/HOLD. C5's test-only report cannot authorize commands.

**Nurse 2:** New exact-head independent review is required for this incremental work. The previous #176 scoped PASS does not certify this new native overlay.

**Operator:** Any merge, installed Fedora action, Sway restart/Quickshell reload, trusted key creation, permission issuance or production activation requires separate approval.

## NEXT / DECISIONS

1. Complete exact-head hostile C and two real mapped-XDG CI jobs. If a failure occurs, correct only this isolated branch.
2. Request independent NRS-2 scoped source/native CI review of the actual process-issued counters, context mismatch revocation and test-source provenance.
3. Negotiate compositor-owned physical selection/transaction instruction hooks with Company Map #126 and Autoscroll #158 owners **without modifying their shared seat/input/transaction files**. Use TimeW #2067 for C3e/C4b/C5 owner contract responses.
4. Only after acceptance, implement a separately scoped **real transaction callback ID binder**, not a raw IPC permission shortcut.

**DECISIONS // SwayPX owner REVISE. TimeW Restore production HOLD. Autoscroll HARD OFF.** No local workstation or production modifications, merge, native move/resize, or signed final receipt.
