# SwayPX / TimeW Scope A // actual transaction instruction lineage witness v0

**PATIENT //** kudokudo1/Post-Apollo-SwayPx  
**ROOM //** TimeW Scope A selected XDG lifetime and transaction identity  
**STAFF ROLE //** External Hospital-compatible SwayPX DOC owner, live registration unverified  
**BASE //** [draft #178](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/178), exact ea4cb59e39e40ce44da86770d74f685876e9e1bc, stacked on reviewed #176  
**STATE //** CI-only native diagnostic, SwayPX REVISE, production Restore HOLD

## IMPLEMENTATION

Parent #178 already issues process-local private 128-bit boot tags and monotonically fresh XDG lifetime and diagnostic transaction counters. Its transaction counter is generated at the real XDG map callback, **not a compositor transaction instruction**. This increment does not alter #178's existing source or assert that its counters are credentials.

Five additive source/CI/docs files provide a strictly narrower test:

- A new private header, include/sway/timew_scope_a_ci_real_instruction.h, declares an unprivileged CI-only callback.
- The guarded Python overlay tests/timew/ci_overlay_real_instruction_lineage.py executes only on ephemeral GitHub Actions runners after the parent real-XDG + #178 lineage instrumentation. It temporarily inserts a call into the actual sway/desktop/transaction.c transaction_commit instruction loop after a view's node instruction has been installed. It implements the callback only inside the temporary test XDG probe. **No shared production source is committed or linked.**
- The callback demands the same in-process mapped XDG view/container/root Wayland resource and private process-local lifetime/boot tag, then increments a separate monotonic **diagnostic commit sequence at the actual native transaction instruction**. It checks but does not publish the process-local transaction pointer. A nonzero internal configure serial is observed but **not a protected ACK**.
- In the disposable runner only, an ordinary untrusted swaymsg resize command targets a disposable mapped foot XDG actor, causing a true native configure instruction. Both mapped lifetimes must see at least one matching actual instruction, fresh commit sequence, and listener/lineage closure on real unmap.
- The new CI assertion fails missing/extra/repeated lifetimes, missing configure serial, reused commit sequence, out-of-order map/instruction/seal/unmap callbacks, and an incorrect count of native instructions. The dedicated workflow compiles and runs the real pinned SwayPX/wlroots/Mesa/foot headless compositor. No production seat, cursor, transaction, Meson, resize, move or IPC file is changed in GitHub.

## VERIFY AND TRUST LIMITATIONS

A PASS can show only that an actual SwayPX transaction instruction included a specific live in-process mapped XDG candidate during this disposable test. This new commit sequence differs from #178's previously at-map diagnostic transaction counter. Neither is an independently enrolled SwayPX source or externally protected receipt.

A match does **NOT** authenticate physical operator selection, installed release/process/host/seat, a durable transaction key, confirmed client configure ACK, authenticated final output frame, exact output-safe placement, C5 one-use SPENT, native MOVE/RESTORE, or crash recovery. The actor app_id and ordinary resize IPC are spoofable test fixtures, not trusted selection or protected intent. No serial or native pointer is a bearer credential. After timeout, CURRENT and scene arrange can still advance without ACK, as independently reviewed #176 showed.

This CI diagnostic intentionally avoids the shared Company Map #126, Autoscroll #158/#175/#177, and live Fedora paths. The unchanged parent #178 still needs its own Nurse 2 review; this increment also needs exact-head independent scoped review before further authority-sensitive integration.

## NEXT / DECISIONS

Verify exact-head GitHub Actions including the pinned real XDG test. Reconcile independent Nurse 2 results, then coordinate separately accepted compositor/TimeW/C3e-C4b/C5 source, seat/output/scale epoch, protected canonical fractional geometry and transaction/terminal receipt contract. Production command or callback linkage, keys, merge and workstation activation require separate operator approval.

**SwayPX OWNER REVISE · TIMEW RESTORE PRODUCTION HOLD · AUTOSCROLL HARD OFF · SCOPE A FIRST.**
