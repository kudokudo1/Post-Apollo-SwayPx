# TimeW Scope A // Real output PRESENT is not a selected XDG terminal frame

**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`  
**ROOM //** TimeW Scope A native XDG transaction/settlement diagnostics  
**STAFF ROLE //** SwayPX DOC, external Hospital-compatible; no verified live Hospital registration  
**BASE //** [SwayPX draft #187](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/187) exact `8b92f00640bf54665e3d93d05d8d194337251a9d`, pending independent review; stacked on scoped-reviewed #185/#182  
**PRODUCTION //** TimeW Restore HOLD, Autoscroll HARD OFF

## IMPLEMENTATION

After #187, the native SwayPX compositor was observed to have copied real `current` geometry and arranged its scene for a mapped XDG candidate. That does not prove a frame has been presented. SwayPX already handles actual `wlr_output->events.present` inside `sway/desktop/output.c::handle_present`. It tests `output_event->presented` before updating `output->last_presentation`. This increment observes **that existing real output presentation callback in disposable CI only**, without creating an installed listener or modifying production source.

The GitHub-only `ci_overlay_real_output_present.py` composes earlier #172/#178/#179/#182/#187 CI overlays, then temporarily instruments the existing original `handle_present` callback *after* its `presented` check. In the already isolated real-XDG test probe it retains the most recent local scene snapshot diagnostic sequence from #187 and checks the output pointer, mapped XDG root/lifetime, current workspace output, and live in-process candidate output. A matching actual output-present event is recorded **once per most recent scene sequence**. Output drift/missing candidate yields a non-authorizing diagnostic refusal. A separate lifetime seal resets all event state before unmap/destroy; no pointer becomes a durable operation key.

The strict log checker `ci_assert_real_output_present.py` requires an actual native scene snapshot before output PRESENT for the same live lifetime/sequence, output PRESENT before lifetime closure, and counted presentations before unmap. It invokes the inherited #187 binary64 and #182 native chronology validators first. It rejects a fake ACK, signed receipt, independently enrolled source, C5 SPENT, native MOVE, selected XDG frame, duplicated or stale output event. New hostile tests include output PRESENT before scene, after seal, wrong lifetime, missing callbacks and forged trust. A pinned native SwayPX/wlroots/Mesa/foot headless workflow exercises ordinary READY and forced native timer cases against two real XDG map/unmap lifetimes each.

**The crucial distinction:** the backend may have presented an output frame. The in-process candidate may have been mapped on the same output after a native SCENE observation. Neither fact proves **which XDG surface buffer contributed** to that output frame, that the chosen XDG's latest configure was acknowledged, that its final animation completed, that output-fit was safe, or that a user selected it. A successful output presentation must remain explicitly `selected_xdg_frame=UNVERIFIED`, `final_view_frame=UNVERIFIED`, `source=UNENROLLED`, `receipt=NONE`, `authority=NONE`. The output present event is an independent but still insufficient signal.

This does **not** define an approved frame-settlement test or canonical receipt wire; SwayPX and Company Map/scene owner agreement is required for actual per-view frame attribution. No production `desktop/output.c`, `desktop/transaction.c`, native observer, Meson, input/seat, cursor, move/resize, IPC or Autoscroll source files are modified on GitHub; the overlays edit only ephemeral GitHub Actions checkout copies.

## VERIFY AND WATCH OUT FOR

A PASS would establish actual SwayPX output PRESENT event(s) after a native post-arrange scene observation for the same output and live XDG diagnostic lifetime, **not** successful selected-view frame settlement. The CI run must preserve the corrected #182 finding that actual READY and TIMEOUT can conflict on an instruction: even if the output subsequently presents and CURRENT geometry matches, that conflict is **UNKNOWN/HOLD**, never authorization for a follow-on MOVE, retry, undo or regrant.

A test fixture's ordinary `swaymsg resize set` acts only on disposable headless `foot`, not an installed SwayPX selected window. An internal configure serial, local process pointer/counter, fake source name, `app_id`, output PRESENT, v3 SQLite data or C5 SPENT fixture is not an authenticated source/consent/receipt.

This increments SwayPX native evidence only. TimeW owns signed exact binary64 consumer and journal; DOC 1/2 C3e/C4b own independently enrolled installed source release/process/host/seat and challenge/custody; DOC5/C5 owns genuine operator-visible intent and independently durable one-use PRE-ACTION SPENT; Company Map and input/scene owners retain shared callback seams. No protected receipt/selection/custody contract has been accepted. The operator has already decided normal local TimeW data now and separate protected hardware/NAS later, with OS-managed-first Autoscroll in its separate owner lane.

## NEXT / DECISIONS

Verify exact-head workflows; request Nurse 2 review scoped to output-present attribution and negative trust claims. Future SwayPX actual selected-XDG final-frame settlement requires scene-buffer identity, committed per-view scene inclusion, output serial/fence timing and revocation observation, plus explicit approval of shared owner seams and signed producer receipts. **Do not** infer them from this output-level event.

**SwayPX protected integration REVISE · TIMEW RESTORE PRODUCTION HOLD · AUTOSCROLL HARD OFF.** No merge, workstation changes, installed service, source/consent keys, trusted Restore, native user-window actions or activation.
