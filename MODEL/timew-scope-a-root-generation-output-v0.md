# SwayPX Scope A // Real XDG root committed-state generation across renderer and PRESENT

**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`  
**ROOM //** TimeW Scope A compositor-native selected XDG, transaction, scene/render, protected receipt  
**STAFF ROLE //** SwayPX DOC, external Hospital-compatible, live registration not verified  
**BASE //** [SwayPX draft #194](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/194), exact `c25bd3502cdca9395aba2303066db95d1fa55292`, independently Nurse 2 scoped reviewed on exact head (the GitHub review record is the source of truth). Previous #193 also exact-head Nurse 2 scoped PASS.  
**OWNER //** Operator delegated TimeW shared scene/render seam leadership to SwayPX DOC while Company Map #126 owner inactive. Company Map M32d MapLibre actor/input/popup work, SwayPX Autoscroll and installed host control remain separately owned.  
**PRODUCTION //** TimeW Restore HOLD · Sign-In HOLD · Autoscroll HARD OFF · Company Map embedded scene OFF

## IMPLEMENTATION // Concrete new evidence boundary

Previous draft #194 proved an actual pinned SceneFX `wlr_scene_buffer.events.output_sample` on the exact mapped XDG ROOT surface, after the native SceneFX render-path `fx_render_pass_add_texture` (or prepared direct scanout). Parent #193 separately bound an accepted output's **actual wlroots `wlr_output.commit_seq`** to later **`wlr_output_event_present.commit_seq`**. Those two events do NOT by themselves show that the sampled XDG surface's content generation was stable across the output submission.

The pinned wlroots `wlr_surface` contains `current.seq`, a **uint32 surface committed-state sequence number**. A client can commit different surface state/contents over the course of XDG resize/animation. This sequence may wrap and is NOT a cryptographic content digest or a standalone immutable buffer identifier. New `tests/timew/ci_overlay_real_surface_generation.py` composes the prior native observers in **a disposable GitHub Actions checkout only** and reads the exact mapped XDG ROOT `surface->current.seq` at three real native callback boundaries:

1. **SceneFX root SAMPLE:** in the existing exact-root output_sample callback, capture `current.seq` with the actual mapped XDG diagnostic candidate and local post-scene instruction generation;
2. **Output COMMIT:** immediately after the existing real accepted `wlr_output_commit_state`/wlroots output serial witness, record whether that same local scene instruction had a real root sample and read `current.seq` again;
3. **Output PRESENT:** immediately after the actual output-event serial witness, read `current.seq` again and preserve that output serial for checker-only association.

The new parser `ci_assert_real_surface_generation.py` invokes **all inherited native chronology validators first**, then requires one real generation-read event per real root sample, accepted output commit, and output PRESENT. It matches their exact **output** serials to the original real wlroots parent records and correlates a root sample only when it belongs to the **same mapped native XDG lifetime, actual scene instruction, earlier real sample, and output submit**. If the root `current.seq` differs at commit or presentation, wraps to zero, is unbound, missing, duplicated, stale or moves across an XDG lifetime, the evidence is **UNKNOWN/HOLD**, never retroactively promoted into a frame receipt. Output submissions with **no sampled XDG buffer** remain explicitly `UNSAMPLED`, not inferred from a later output PRESENT. Parent forced native READY+TIMEOUT contradiction remains UNKNOWN/HOLD irrespective of any sample/generation matches.

**Important precision:** even if `current.seq` stayed identical from a root sample to an output commit and its matching PRESENT, this only establishes **process-local consistency of a single XDG root surface committed state** while that output submission was observed. It does **NOT** uniquely identify all underlying `wl_buffer` contents, prove a draw or completed GPU work, that exactly these pixels were visible, that no occluding content/animation altered the final selected window, client configure ACK, exact output-fit, operator selection, independently enrolled source, protected C5 spend or an actual signed terminal-frame receipt. Surface state can be unchanged while earlier/out-of-band buffer content or render effects complicate attribution. A local uint32 is not a protected cross-process operation ID.

The workflow runs the previous **179 hostile cases** plus **42 new cases**, checking replacement and wrap HOLD; wrong output serials, missing events, duplicate/late/replayed callbacks, counterfeit root generation, forged immutable buffer and final-frame statements, source/C5/ACK/receipt/MOVE promotion, and native seal/unmap order. It builds and runs the same pinned real headless SwayPX/SceneFX/wlroots/Mesa/foot test actor twice per mode (normal and forced native timeout). All five new files are additive CI/tests/MODEL. The GitHub-committed SwayPX `sway/desktop/output.c`, `transaction.c`, `xdg_shell.c`, `view.c`, Meson, cursor/seat/input, IPC, native move/resize, Company Map #126 and Autoscroll code remain untouched. The native instrumentation is injected **only into disposable GitHub CI checkout** after source-drift guards.

## VERIFY // Strict classification

The validator distinguishes **surface-state unchanged at accepted output commit** from **surface-state unchanged when the output PRESENT callback arrives**, and explicitly counts already-replaced, zero/wrapped, unsampled and unbound cases as HOLD. Even an unchanged state plus matching real output serial remains `immutable_buffer=UNVERIFIED xdg_final_frame=UNVERIFIED client_ack=UNVERIFIED enrolled_source=NONE c5_spent=NONE protected_receipt=NONE move=OFF retry=OFF safe_actions=NONE settled=0 authority=NONE`.

Normal and forced timer actual callbacks must be inspected at the exact new PR head. This document does not assert CI PASS until those checks and logs actually succeed. Any failed native build, changed source anchors or failure to see real samples is a blocker, not an authorization. A scoped independent Nurse 2 review is a separate gate, as are operator merge and installed-host activation.

## WATCH OUT FOR // Follow-on dependency

The next **genuine content generation and per-view final frame** proof needs authoritative association of a specific client `wl_buffer` attachment/commit, its surface state and client configure ACK, sampled texture/render-pass inclusion, accepted output submission, exact presented output event, terminal animation and output/output-scale/seat epoch, without stale buffer reuse. A surface `current.seq` and a scene pointer are not an immutable source signature. C3e/C4b must enroll installed source/host/release/process/seat and sign canonical exact binary64 native results; DOC 5/C5 must independently protect one whole operator-visible immutable intent and one **PRE-ACTION SPENT**; TimeW must verify per-step signed receipt and crash UNKNOWN without automatically performing MOVE/retry/undo. Local ordinary TimeW storage now, protected separate NAS hardware later, OS-managed-first Autoscroll are settled operator decisions, not native authority.

## NEXT / DECISIONS

Complete exact-head native CI and tests, fix only CI-owned implementation issues, request Nurse 2 scoped source/native review and record TimeW/Company Map handoffs. No merge, production scene hook, keys, installed host, consent spend, native user-window actuation, automatic retry or activation.

**SwayPX protected integration REVISE · TimeW Restore PRODUCTION HOLD · Sign-In HOLD · Autoscroll HARD OFF · Company Map embedded scene OFF.**
