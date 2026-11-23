# SwayPX Scope A // Actual wlroots OUTPUT commit sequence vs PRESENT event (CI only)

**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`  
**ROOM //** TimeW Scope A selected-XDG compositor scene/render/presentation  
**STAFF ROLE //** SwayPX DOC, external Hospital-compatible; live Hospital attachment unverified  
**BASE //** [SwayPX draft #191](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/191) exact `c26c998ff00a1155a03814aaf73ab4b55a19ba0b`, native CI 2/2 PASS, Nurse 2 exact-head review pending  
**SHARED SEAM //** Operator explicitly delegated temporary **TimeW scene/render integration leadership** to this SwayPX DOC while the Company Map #126 owner is inactive. The [recorded bounded handoff](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/126#issuecomment-7113075301) does not transfer Company Map's M32d MapLibre widget/actor/input/popup work, authorize edits to its active draft files, or constitute Hospital live registration  
**PRODUCTION //** Restore HOLD · Sign-In HOLD · Autoscroll HARD OFF · Company embedded MapLibre OFF

## IMPLEMENTATION // Concrete native source mechanism

The actual SwayPX `sway/desktop/output.c::output_repaint_timer_handler` builds an output state using `wlr_scene_output_build_state`, then calls `wlr_output_commit_state`. After successful backend commit, wlroots exposes a **uint32 `wlr_output.commit_seq`**. Independently, `sway/desktop/output.c::handle_present` receives a **`wlr_output_event_present` with its own `commit_seq`**, which identifies the output submission referenced by that presentation event. This is more discriminating than merely counting independent output commits and output PRESENT callbacks. These are wlroots *output* event fields, **not** the actual selected XDG `wl_surface` commit ID or signed native receipt.

The new GitHub-only `tests/timew/ci_overlay_real_output_commit_serial.py` composes all prior #172/#178/#179/#182/#187/#188/#189/#191 native XDG/transaction/geometry/scene-buffer/output observations **in a disposable checkout only**, then logs the *real* output `commit_seq` immediately after the existing #191 accepted-backend-commit observation and logs the *real* PRESENT `event.commit_seq` immediately after #188's output PRESENT callback. Logs carry the same original live mapped XDG *diagnostic* lifetime, root, workspace, output object and post-scene sequence, and set all source/ACK/C5/receipt/MOVE/settlement flags false.

The new `ci_assert_real_output_commit_serial.py` first invokes the full parent native chronology chain. It requires actual `OUTPUT_SEQ_COMMIT` and `OUTPUT_SEQ_PRESENT` records in both mapped/unmapped XDG lifetimes, checks exact parent output commit/PRESENT ordering, unique nonzero uint32 serials, live source lifetime, seals and unmap. **Only a PRESENT referring to a previously recorded, same-lifetime output commit serial can be classified as a matched output submission.** A PRESENT arriving before the COMMIT observation, pointing to an unrecorded serial, crossing lifetimes or following unmap is **unbound/UNKNOWN_HOLD**, never retroactively adopted by a later COMMIT. Reuse, wrap, duplicate or fabricated positive source/selected-frame/C5/receipt/MOVE claims are refused.

The reference uses **no signer or privileged action**. It never claims `selected_xdg_buffer_rendered`, `selected_xdg_final_frame` or `settled`; output-only causal matching is a lower trust level than per-view buffer inclusion.

The new adversarial suite adds **29 cases** (realistic matching and unbound valid negatives, late/early/replayed/duplicate/bad serials, wrong lifetime, seal, fake source/C5/ACK/frame/MOVE and policy drift) to the prior **118**, for a target of **147**. The standalone GitHub workflow checks source isolation and reruns the inherited native XDG and output observation tests, builds pinned real SwayPX/wlroots/Mesa/foot and executes headless `foot` mapped/unmapped twice in each normal-READY and forced real timer-TIMEOUT session.

## VERIFY // What a PASS means

A passing native run may prove: **actual wlroots accepted a particular output-state commit sequence and later reported that same OUTPUT commit sequence as presented**, while the diagnostic XDG actor was mapped on the same output. If there are unbound/early event deliveries, report their exact count and preserve HOLD; do not force a match. A successful output-level commit serial match still **cannot prove**:
- which `wlr_scene_surface` / selected XDG buffer was drawn in the committed output;
- whether that source buffer had the correct most-recent client configure ACK;
- whether a selected-view terminal frame, output-fit, animation and exact post-resize logical rectangle settled;
- genuine operator selection, independently enrolled SwayPX installed source/release/seat/output epoch or client challenge;
- immutable operator-visible TimeW exact binary64 intent and independently durable current C5 one-use PRE-ACTION SPENT; or
- an independently signed per-step resize receipt that permits a MOVE.

The forced native timer's prior **three contradictory READY+TIMEOUT observations remain UNKNOWN/HOLD**, even if output commit serials match presentation serials. Both local output serial fields have 32-bit wrap and are not a durable global operation ID. Do not use them as a selected-XDG identity or protected packet field without separate accepted issuer/wire contracts.

## WATCH OUT FOR // Owner and production controls

This is the first bounded diagnostic milestone after the operator authorized SwayPX DOC to lead the **TimeW renderer seam** while Company Map's owner is inactive. It does **not** edit or merge the inactive Company Map #126 draft (head `3d0f7ce7`); its changed paths include seat/input, transaction, Meson and Company native scene files, but not `sway/desktop/output.c`. Autoscroll #190 remains separately owned and HARD OFF. No original `output.c`, `transaction.c`, `view.c`, compositor Meson, input/seat, IPC, resize/move, or Company Map source changes are committed in this increment. All native hooks live in temporary GitHub runner file copies.

Future **actual per-XDG contribution and terminal rendered frame** evidence may require renderer-layer observation in SceneFX/wlroots, where a scene-buffer's source and target output commit sequence can be traced without conflating frame-done scheduling, scene presence or backend acceptance. Such a deeper shared-source change remains separately bounded and independently reviewed, never a production Restore authorization. If the Company Map owner becomes active, reconcile current ownership and checkpoint first.

TimeW owns exact binary64 signed consumer and UNKNOWN crash recovery; C3e/C4b owns enrolled installed producer/process/release/seat/host and canonical source receipt; DOC 5/C5 owns independently durable one-use SPENT for immutable full visible intent. Local ordinary TimeW data now and separate protected NAS/hardware later are settled operator direction, not evidence of source authority.

## NEXT / DECISIONS

Verify the new exact-head CI and native logs, correct any test-only defects, ask Nurse 2 for exact-head scoped review, and record the bounded Company Map seam lead handoff and TimeW #2067 dependencies. No merge, installed system changes, source keys, protected SPENT, native user-window operation, automatic retry/undo/regrant or activation.

**SwayPX protected native integration REVISE · TimeW Restore PRODUCTION HOLD · Sign-In HOLD · Autoscroll HARD OFF.**
