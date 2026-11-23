# SwayPX / TimeW Scope A // native transaction-stage observation (proposed v0)

**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`  
**ROOM //** TimeW Scope A compositor-origin selected-surface / geometry witness  
**STAFF //** External Hospital-compatible SwayPX DOC owner, live Hospital registration unverified  
**STATE //** **REVISE / SOURCE-ONLY / NO NATIVE AUTHORITY / PRODUCTION RESTORE HOLD**  
**STACK //** This draft is based on [#173](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/173) at `81a3acef567d3e4adb470e5fa0d76fe707a25448`, including its NRS-2 scoped binary64 PASS; parents #172/#170/#169/#168 remain independent drafts  
**TIMEW CONSUMER //** [#2067 owner REVISE](https://github.com/kudokudo1/The-Post-Apollo-Project/issues/2067#issuecomment-7085803105). Preserve legacy TimeW integer schemas.

## IMPLEMENTATION // Actual source mechanism, not proposed behavior

Current pinned `sway/desktop/transaction.c` shows:

1. A mutable container **`pending`** `sway_container_state` is copied to a frozen transaction instruction in `copy_container_state`. These are double-precision **logical layout coordinates**.
2. `transaction_commit` may send a Wayland configure and await an XDG view-ready **serial** (or XWayland geometry), but `should_configure` is conditional. A move without a size change can need no XDG configure. The internal serial is **not a protected TimeW receipt**.
3. `transaction_progress` blocks while `queued_transaction->num_waiting > 0`, then applies instruction state: `apply_container_state` copies instruction state to **`con->current`**. It runs `arrange_root`, rebases cursors, destroys the queued transaction and starts animations. **`current` may not match `pending` if a newer change is pending.**
4. Critically, `handle_timeout` sets `transaction->num_waiting = 0` and invokes **the same** `transaction_progress` path. Thus a fresh `con->current` is **not proof of a client configure ACK**, and `current == target` is neither proof of frame presentation nor an authorized restore. The native source comments also explicitly allow imperfect frames on timeout.
5. The output's `lx/ly` are layout coordinates; `output->width/height` are described as **transformed buffer size**, while `wlr_output->scale/transform` and workspace/output ownership are separate facts. It is incorrect to infer a physical-pixel placement or output-safe target from a numeric rectangle alone.
6. Scene arrangement occurs after copying state; `start_animations` follows transaction destruction. An output `present` event on its own would not bind a particular selected XDG view, exact transaction, output/seat epoch, authorized operation and terminal frame. **No existing end-to-end settled/receipt proof** has been identified.

## Proposed observation tiers, with no privileged result

| Tier / origin | What SwayPX can claim locally | What it CANNOT claim |
| --- | --- | --- |
| `PENDING` (`con->pending`) | A mutable desired logical rectangle, may contain real fractions such as `y=92.5` | Applied, source-authenticated, consented, moved, or final |
| `INSTRUCTION_QUEUED` | A proposed snapshot is held in a queued transaction, maybe waiting for configure | Client ready or frame presented |
| `VIEW_READY_REPORTED` | Internal serial/geometry callback has marked instruction ready | Operator consent, protected signed receipt, final geometry |
| `TIMEOUT_FORCED` | Wait expired and SwayPX may continue to apply current | Client ACK or verified final state; **UNKNOWN/HOLD** |
| `CURRENT_COPIED` (`con->current`) | Transaction instruction was copied to current, possibly after timeout | Scene arranged, frame rendered/presented or TimeW completion |
| `SCENE_ARRANGED` | Layout nodes positioned after current copy | A specific XDG surface's frame was presented with its target |
| `PRESENT_REPORTED` | Hypothetical locally observed output present event | An authenticated per-command receipt or exact view finality without source/scene binding |
| `COMPLETE_VERIFIED` | **Not implementable in this draft** | Must stay **UNAVAILABLE / HOLD** until separately approved source, consent, lifetime, output and receipt contracts |

The new `timew_scope_a_txn_stage` is a **pure CI fixture** with strict event ordering and correlation of synthetic `compositor_epoch/selected_generation/transaction_generation/output_epoch`. Even the best `PRESENT_REPORTED` result sets `selected_surface_authenticated`, `configure_ack_authenticated`, `compositor_receipt_authenticated`, `geometry_settled_verified`, `native_resize_allowed`, `native_move_allowed`, retry, and undo **all false**. Timeout remains tainted even after later current/scene/present labels. Wrong-order event causes HOLD; stale IDs do not advance a new trace.

The dormant native `timew_scope_a_native_txn_snapshot_read` additionally reads actual in-process `con->pending/current` using #173 lossless binary64 value captures plus exact workspace/output references, output `lx/ly`, transformed dimensions, `wlr_output->scale/transform`, and whether queued/pending transactions currently exist. It refuses destroyed, unmapped, locked, non-XDG, nonfloating, fullscreen, workspace/output drift, nonfinite/invalid geometry, or missing current values. All proof/command flags remain false. It is **absent from normal `sway/meson.build`**. It can demonstrate current data in a future disposable compositor test, not authenticate `CURRENT` as terminal.

## Source/transaction acceptance still REQUIRED

Before any production protected command, a separately reviewed native protocol must bind:

- Private compositor-origin explicit **operator selection event** and exact view/root lifetime/boot epoch/seat generation, revoked on destroy/unmap/lock/output/workspace/session changes.
- Per-step **real** transaction generation and readiness classification distinguished from forced timeout. No caller-generated transaction stage or `swaymsg GET_TREE` is trusted.
- Exact fractional before/target and **observed after-resize** binary64 logical geometry; record actual native current after the relevant transaction with same selected object, output/workspace and phase. No assumptions that centered resize preserves old x/y.
- Output physical transforms/scales and workspace fit: source must bind layout origin + output identity/epoch, configuration/scale/transform and usable-area checks **before** any placement. Merely reading raw dimensions and `wlr_output->scale` is not an authorized layout transform.
- Protected C3e/C4b enrolled release/host/process/seat/challenge/receipt source, independently held authority; C5 exact reviewed operation and **durable once-only SPENT before first effect**; TimeW protected intent/dispatch/UNKNOWN crash-gap reconciliation.
- Configure ACK vs applied-current vs scene arrangement vs actual presented **selected view** semantics, including compositor animation or timeout. A genuine final receipt must be correlated to one operation/step, one selected lifetime, the unchanged output/seat epoch and the exact post-step state.
- Per-step protected native `resize` then `move`; if resize has an uncertain acknowledgment, stale output, changed lifetime or no protected final observation, block move and return **UNKNOWN/HOLD**. No automatic retry, undo or new grant.

**Cross-process serialization is NOT decided here.** `#173`'s in-memory uint64 bit patterns are an exact value reference, **not** a canonical byte order, signed envelope, wire schema or TimeW checkpoint migration. TimeW's existing integer-only V1/V2 schemas remain intact. Next design consultation must choose canonical endian/version/field order and protected transaction source binding with TimeW, C3e/C4b and C5 owners.

## VERIFY / NEXT / DECISIONS

- `tests/timew/test_timew_scope_a_txn_stage.c` tests normal, timeout-applied, late ACK, duplicate, wrong-order, stale generations/output epochs, missing source and permanent NO-authority.
- CI workflow compiles strict optimized + UBSan fixture and performs a **CI-disposable-only** pinned SwayPX/wlroots build that temporarily adds the new observer and #173 exact geometry to Meson, without committing a production integration.
- No native resize/move, operator UI, seat input hook, compositor release enrollment, trusted signing, C5 SPENT, production installer or host modifications.
- Request Nurse 2 **new exact-head** scoped source/native-compile review. Parent #168/#169/#170/#172/#173 scoped reviews stay valid for their original heads.
- TimeW owner has chosen **REVISE / additive exact-native lane**, not accepted this transaction model as a protected interchange. [C5 #2163](https://github.com/kudokudo1/The-Post-Apollo-Project/issues/2163) remains owner-revision; C3e/C4b enrollment is independently pending.
- Company Map #126 and Autoscroll #158 retain their shared seat/input/output/transaction ownership. No production source is edited, and no Fedora SwayPX Social GDM session is touched.

**SwayPX DOC OWNER DECISION // REVISE. Production Restore HOLD. Scope A first, Scope B Brave identity later.**
