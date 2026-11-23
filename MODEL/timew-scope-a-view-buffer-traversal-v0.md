# SwayPX Scope A // Real selected-XDG scene-buffer traversal (deny-only CI)

**PATIENT:** `kudokudo1/Post-Apollo-SwayPx`  
**ROOM:** TimeW Scope A actual XDG scene-buffer source observations  
**STAFF ROLE:** SwayPX DOC · external Hospital-compatible, no verified live Hospital registration  
**STACK:** [#188](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/188) exact `0565cc425ecc8feb76f832c1c8b84f4d6e31fb24` (independently Nurse-2 scoped reviewed), on #187, #185, #182  
**PRODUCTION:** TimeW Restore HOLD · Autoscroll HARD OFF

## IMPLEMENTATION

This CI-only SwayPX-owned increment answers one question missing after #188: **Does the original compositor's output frame-done scene traversal encounter a real `wlr_scene_surface` buffer with a native `SWAY_SCENE_DESC_VIEW` ancestor equal to the current mapped XDG diagnostic candidate, whose primary scene output matches the active output?**

In `sway/desktop/output.c`, `send_frame_done_iterator` is called by `wlr_scene_output_for_each_buffer` in the real output `handle_frame` path. Existing source first refuses buffers whose `primary_output` is different, refuses non-surface scene buffers via `wlr_scene_surface_try_from_buffer`, then climbs scene node ancestors with `scene_descriptor_try_get(..., SWAY_SCENE_DESC_VIEW)`. The new `tests/timew/ci_overlay_real_view_buffer_scan.py` inserts an **observer only at the existing found-view location**, in a disposable GitHub Actions checkout after #188's native output PRESENT overlay. This does not modify the GitHub committed `output.c`, `view.c`, transaction, input, scene owner, Meson, IPC, resize/move, or installed desktop.

The new SwayPX test probe logs at most one matching buffer traversal per local instruction scene sequence. It checks the exact mapped XDG candidate view pointer and live root surface, actual native output/workspace and the pre-existing candidate generation, with output/frame-done source context. Unmatched buffers from other views are silently ignored; matched view with wrong output or revoked root emits a `DRIFT` observation and cannot pass the checker. The lifetime is sealed/reset on original XDG unmap.

The `ci_assert_real_view_buffer_scan.py` read-only validator runs **all inherited #182 transaction chronology, #187 native binary64 geometry and #188 actual output-PRESENT checks first**, then requires a real candidate scene-buffer scan after its corresponding post-scene snapshot and before its lifetime seal and unmap. Replay, wrong lifetime, missing scans, duplicate sequence, forged XDG/primary output, forged enrolled source/C5/ACK/receipt/final frame, malformed count and unmap order are refused by 25 additional hostile tests.

**Crucial mechanism:** traversal is a scene-graph iteration performed to deliver **frame-done callbacks** to scene surfaces. A real XDG surface buffer found in this iteration is **not proof that its buffer was rendered in or contributed to the output's next presented frame**, still less that a final client-submitted buffer matched an exact protected configure and completed animation or that a human selected it. An output PRESENT from #188 remains a **different** output-level event. Their timestamps, local sequence number or coincident identity cannot be upgraded to a canonical signed selected-view terminal settlement receipt.

Every successful diagnostic remains `contributed_to_present=UNVERIFIED rendered=UNVERIFIED selected_final_frame=UNVERIFIED client_ack=UNVERIFIED c5_spent=NONE source=UNENROLLED receipt=NONE move=OFF retry=OFF authority=NONE`. Forced actual native timeout with READY+TIMEOUT contradiction remains **UNKNOWN/HOLD** even if the view buffer is traversed. `safe_actions=[]`, `settled=false`. No MOVE, RETRY, UNDO, new C5 grant or Restore.

## VERIFY

The new isolated workflow invokes the prior #188 actual pinned SwayPX/wlroots/Mesa/foot headless CI, plus the additional guarded overlay and new parser; runs separate normal READY and real forced timer modes, two mapped/unmapped real XDG lifetimes per mode. It also runs the inherited 28 outcome, 20 native scene, 20 output present and 25 new buffer traversal adversarial tests. **Run/CI result must be verified at the exact new head; do not treat this paragraph as a passing test.**

## WATCH OUT FOR // Owner and trust boundaries

This increment does not authorize modifying the Company Map owner’s shared scene/cursor/focus code or Autoscroll owner’s OS-managed input path. SwayPX future *actual selected-view presented frame and settlement* will require an accepted compositor scene-buffer contribution/presentation protocol, configure/ACK or timeout distinction, post-resize exact fractional geometry, selected lifetime/output/scale/workspace epochs, and independent source enrollment. Source signing/custody remains C3e/C4b; protected one-use consent `PRE-ACTION SPENT` remains DOC 5/C5; TimeW owns exact binary64 receipt consumer/recovery and ordinary local storage. C5’s read-only recovery lookup plan is not an actual protected SPENT or signed outcome.

The operator has settled ordinary local TimeW storage now, separate protected NAS hardware later, and OS-managed-first Autoscroll. Neither direction confers native authority. All PRs remain draft pending operator merge authorization.

## NEXT / DECISIONS

Request exact-head Nurse 2 scoped **native source/CI** review after verification. If real buffer traversal is observed, continue only toward independently accepted selected-XDG buffer **contribution** and terminal presentation evidence through a new owner-coordinated milestone; do not infer it from frame-done iteration. Preserve the source experiment’s GitHub PR/CI logs as recovery pointers.

**SwayPX protected integration REVISE · TimeW Restore PRODUCTION HOLD · Autoscroll HARD OFF.**
