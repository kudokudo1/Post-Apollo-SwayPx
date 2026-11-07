# Stage Director // Social W native pair candidate ledger

**STATUS //** Additive, compiled/tested standalone C **candidate-only** core. No activation, native seatop wiring, or user desktop change.  
**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`; consumer `kudokudo1/The-Post-Apollo-Project`.  
**ROOM //** Social W chassis + external guest unified motion. Hospital-compatible external browser Doctor; not registered live.  
**DEPENDENCY //** Social proof chain drafts #99, #101, #102, #103 and #105. The immediately preceding #105 passed native guest-unmap and replacement in live CI SwayPX.  
**OWNER BOUNDARY //** Shared SwayPX `sway/input/seatop_*_floating.c`, `sway/desktop/output.c`, transaction, Stage Director, native Wayland protocol and Meson build files remain controlled by overlapping Company/Weather/Chrome owners.

## IMPLEMENTATION

This is a **first reusable identity lifecycle module**, separate from the CI fixture's owner-writable ID/PID file. It adapts the **existing** Stage Director NEWS/Weather candidate-ledger pattern rather than inventing a public Sway IPC API. Social needs a layer-shell host and a distinct XDG guest, so it receives its own narrowly scoped roles and invariants. The code is contained in new `include/sway/stage_director_social_pair_candidate.h` and `sway/stage_director_social_pair_candidate.c`. **Meson does not link it into the running production compositor.**

The additive state machine has empty → host candidate → paired candidate → revoked/empty states. On host admission it allocates a monotonically increasing generation internally. A guest candidate must be a separately mapped, exact XDG native root; the host must be a mapped layer role. Both require distinct, nonzero native container/client/root-surface references, exact output, workspace, seat and UID agreement, an explicit native mapping-generation, positive native process PID and start ticks. A candidate probe checks exact *fresh* observations, including all those fields. The generation floor survives revoke, so an old epoch cannot regain custody even after native pointers are reused.

Revocation is exact-generation and exact-native-event scoped: host or guest surface unmap, host or guest Wayland client death, output loss, workspace loss, seat loss, and authenticated private close. Stale destroy events from an earlier generation cannot revoke a replacement candidate.

**Security distinction:** The ledger accepts values supplied by a future in-process trusted compositor callback, which is **not connected in this PR**. It cannot by itself certify a client or trusted operator gesture. A successful native correlation returns **CANDIDATE_ONLY** with `authenticated_pairing=false` and every `can_move`, `can_resize`, `can_focus`, `can_clip`, `can_render`, `can_input` and `can_popup` permission **false**. Neither QML, IPC, a forgeable app ID, the launch process, nor an ordinary client may select native refs or mapping generations.

## HOW TO USE

The new standalone GitHub Actions workflow **Stage Director Social native candidate lease (no actuation)** builds the C core separately with strict warnings and undefined-behavior sanitization, runs negative and positive unit cases, and verifies Meson does not link the module into production. Existing Social native-seatop CI and all operator desktop processes are untouched.

## VERIFY

- Deny null/missing/unmapped/wrong-role or same-client native candidates
- Reject mismatched output, workspace, seat or UID; reject same native container/surface
- Refuse duplicate reservation, old generation, fake root surface, pointer reuse, old PID start time, wrong mapped generation
- Require all actual privileges to remain false even after an exact candidate match
- Check eight distinct native revocation causes, wrong source and stale-generation events
- Revoke active host-only candidate; reject `UINT64_MAX` generation wrap instead of recycling it
- Demonstrate exact native observation and new generation required after guest replacement
- Enforce CI-only build; clean repository checkout and absence from production Meson

A green unit suite verifies **the candidate ledger as a pure C contract**. It does **not** attest live mapped Wayland objects, real Quickshell/Vesktop, genuine operator intent, physical input or secure scene authority. Those remain outside this PR.

## WATCH OUT FOR

- `uintptr_t` records must come from verified in-process native compositor objects. This standalone core cannot prevent its eventual caller from lying. Production acceptance requires the Stage Director owner to supply client/resource/role/seat/output credentials from compositor-owned state and to validate them across every callback.
- A reused native pointer or PID is not proof of continuity; include explicit mapping generations and process start ticks.
- The private close event must be invoked only from an independently authenticated compositor callback, not generic client IPC. All other actuation capabilities are deliberately absent.
- The current Social native-move/resize CI source overlay **still uses its test-only ID/PID lease file**. This candidate ledger does not silently replace it or make any production window grouping possible.
- Existing Weather WATCH candidate ledger is a separate owner-controlled feature, not a generic helper that should be cross-edited by Social without coordination.
- A green isolated unit test does not fix lag in the installed Social widget.

## CHECKLIST

- [x] Recheck Syntagma AGENTS, Hospital permissions, current SwayPX and Social branch heads, Stage Director PR ownership
- [x] Reuse NEWS candidate-ledger semantics: no implicit authority
- [x] Implement independent Social roles, generation, exact-pair correlation and eight revocation events
- [x] Add strict compiled C tests and production-unlinked CI guard
- [ ] Verify exact-head GitHub CI
- [ ] Obtain shared compositor owner assignment / integration seam authorization
- [ ] Connect independent trusted Wayland callback and actual layer-shell host + XDG guest provenance
- [ ] Wire feature-gated native seatop and output pipeline to authenticated lease, with revoke-on-all-lifecycle events
- [ ] Test real Quickshell/Vesktop, physical pointer, scaling, popups and full-frame presentation, then operator accepts

## NEXT

After the standalone suite is green, provide this candidate contract and the live-unmap acceptance evidence of PR #105 to the current Stage Director compositor owner. That owner can decide whether to reuse or adapt the module as part of a secured production lease and private native protocol, keeping `sway/desktop/output.c` and seatop ownership unchanged until delegation. Continue to retain Social's existing IPC fast-follow fallback.

## DECISIONS

No merge, local file write, production SwayPX build activation, Quickshell reload, or permission grant. Stage Director native trust boundary is **awaiting owner integration**. This document is an implementation/continuity checkpoint, not a new Syntagma governing protocol.
