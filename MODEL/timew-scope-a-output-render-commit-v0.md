# SwayPX Scope A // Real output render-build and backend commit are not selected-XDG frame settlement

**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`  
**ROOM //** TimeW Scope A compositor-native selected XDG, exact geometry, frame/receipt evidence  
**STAFF ROLE //** SwayPX DOC · external Hospital-compatible, live Hospital registration unverified  
**BASE //** [Draft #189](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/189) exact `50b311274fdb1dab8cb772bbbffe408bf1d3a069` (CI PASS, Nurse 2 review pending); stacked on independently scoped-reviewed #188/#187/#185/#182  
**STATE //** CI-only source witness; SwayPX protected integration REVISE, TimeW Restore HOLD, Autoscroll HARD OFF

## IMPLEMENTATION

Native SwayPX already separates several compositor stages. The existing `sway/desktop/output.c::output_repaint_timer_handler` calls `wlr_scene_output_build_state(output->scene_output, &pending, &opts)` and, on successful build, `wlr_output_commit_state(output->wlr_output, &pending)`. A failed commit logs a page-flip error; a successful call means the output state was accepted by the backend. Later, `handle_present` observes an **output** presentation event. The earlier #189 `send_frame_done_iterator` is yet another distinct surface iteration used for frame callbacks, not a source of rendered-buffer identity.

The new `tests/timew/ci_overlay_real_output_render_commit.py` reuses #172/#178/#179/#182/#187/#188/#189 disposable CI-only native probes, and temporarily adds a passive callback **only in the GitHub runner copy** of `output_repaint_timer_handler`, immediately after `wlr_output_commit_state` returns true for the real built output render state. It checks that the test's still-mapped XDG diagnostic candidate has the same root, current workspace and real output object as the last #187 post-scene snapshot. It logs at most one accepted output commit per local scene sequence and seals/reset its local sequence when that exact native XDG lifetime unmaps.

The new checker `ci_assert_real_output_render_commit.py` first runs all inherited actual XDG lifetime, native transaction, binary64 scene, output PRESENT and view-ancestor traversal validators, then requires at least one genuine compositor accepted-output-render-state observation for each of the two real test XDG lifetimes. It refuses duplicate/replayed/stale/malformed commits, attempts to promote `xdg_rendered`, `xdg_contributed`, `xdg_final_presented`, source enrollment, C5 spending, client ACK, native MOVE, retry or a signed receipt, and malformed/late/unbound seals. A new hostile test file contains **25** independent negative cases. The new pinned workflow runs those 25 plus all **93** prior adverse cases and two disposable compositor sessions: ordinary READY versus actual forced timer TIMEOUT.

## VERIFY // The precise source claim

A successful CI outcome demonstrates that **the compositor built a real output render state and its backend accepted the commit while this XDG diagnostic candidate was mapped on the same output**. It does NOT prove:
- The selected XDG scene-surface buffer was included among the renderer's draws, contributed to this committed output, or was the latest buffer to match the protected configure
- The independently observed output PRESENT corresponds to this exact backend commit or the XDG surface content; a local scene sequence and ordering cannot replace a protected buffer contribution/presentation ID
- The view's final animation or per-view terminal frame settled, or its exact logical geometry is physically output-fit
- An actual operator selected the right mapped XDG using an enrolled seat, or an independently protected current C5 full-intent one-use PRE-ACTION SPENT happened
- A C3e/C4b source signed a canonical per-step receipt, or TimeW received a protected resize result authorizing the next MOVE

Thus all records keep `xdg_rendered=UNVERIFIED`, `xdg_contributed=UNVERIFIED`, `xdg_final_presented=UNVERIFIED`, `client_ack=UNVERIFIED`, `source=UNENROLLED`, `c5_spent=NONE`, `receipt=NONE`, `move=OFF`, `retry=OFF`, `authority=NONE`. A real native TIMEOUT can still lead to CURRENT, scene, an accepted output commit and an output PRESENT; the three previously observed contradictory READY+TIMEOUT cases remain **UNKNOWN/HOLD**, even if new commits are observed. No automatic move, retry, undo, regrant, protected grant or Restore.

This increment does not introduce a new XDG lifetime issuer, protected parser, client signer, native selection gesture, release/seat enrollments, buffer contribution certificate, or per-view terminal-frame proof. All production SwayPX `output.c`, `view.c`, `transaction.c`, native geometry observer, seat/input, cursor, IPC, resize/move and Meson files remain **unchanged in this PR**. Runner-only instrumentation is discarded after CI.

## OWNER BOUNDARIES / NEXT

Company Map retains shared compositor scene/focus/seat/output ownership. [Coordination request on #126](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/126#issuecomment-7112257512) has not been accepted as permission to edit those production files. Actual per-selected-XDG rendered contribution and terminal presentation must wait for an owner-approved observation hook, robust buffer/commit/presentation correlation and signed source identity. TimeW owns exact binary64 receipt consumption and read-only crash UNKNOWN reconciliation; C3e/C4b independently enrolls installed release/process/seat/challenge and signs canonical source receipts; C5/DOC 5 independently protects one full immutable visible intent, one pre-action SPENT and rollback/custodian checks.

Normal TimeW data local-first, separate protected NAS hardware later and OS-managed-first Autoscroll are settled operator directions, not trusted source, consent, or frame proof. C5's read-only recovery lookup plan and the M5 UI do not change SwayPX settlement authority.

**Review workflow:** Confirm exact head GitHub native CI, ask Nurse 2 to review its precise output-level claims, and request Company Map technical ACCEPT/REVISE/HOLD on a future source-side renderer/contribution attribution seam. No merge, installed Fedora mutation, signing keys, native user-window operation, C5 spending or production activation.

**DECISIONS // TimeW Restore PRODUCTION HOLD · Sign-In HOLD · Autoscroll HARD OFF.**
