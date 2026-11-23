# SwayPX / TimeW Scope A // protected live-surface selection and guarded geometry (proposed v0)

**STATE //** SwayPX owner technical decision: **REVISE**, source contract only; **not implemented, installed, or activated**  
**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`  
**ROOM / ASSIGNMENT //** SwayPX native operator-selected live surface, revocable lifetime, guarded resize/move and verifiable outcomes for TimeW Scope A  
**STAFF //** External Hospital-compatible DOC, OPEN AI / CHATGPT; no verified live Hospital registration  
**SOURCE CHECKPOINT //** SwayPX `master` `fb064cf3c82aa7b9e8d2375bb631773d5eb5a12f`, 2026-11-22  
**CONSUMER //** TimeW-owned [consumer contract #2117](https://github.com/kudokudo1/The-Post-Apollo-Project/pull/2117), [issue #2067](https://github.com/kudokudo1/The-Post-Apollo-Project/issues/2067); Connections [RFC #2115](https://github.com/kudokudo1/The-Post-Apollo-Project/pull/2115)  
**AUTHORITY //** Operator selected Scope A first (one already-live floating nonfullscreen surface, same session, geometry only); Scope B Brave browser-instance identity remains separate. This document does not approve a production protocol or expand any other worker's files.

## 1. Observed source and one decisive geometry correction

The current compositor already has relevant mechanisms but **none is an authenticated TimeW selection or native one-use geometry permission**:

| SwayPX source | Actual existing mechanism | Boundary |
| --- | --- | --- |
| `sway/input/seatop_default.c` `handle_button` | `node_at_coords` resolves the live pointer-target node and floating/fullscreen status on a seat input event; normal button flow also performs focus and drags | Hit test or focus alone does not establish an explicit, trusted TimeW selection ceremony; physical input provenance remains to be designed |
| `sway/input/seat.c` `seat_set_focus_container` | Moves ordinary focus; `server.session_lock.lock` influences focus | Focus is neither a protected selection nor a C5 approval |
| `sway/tree/container.c` `container_begin_destroy` | Emits `con->node.events.destroy` before teardown; node can later be freed | A future selection must clear itself synchronously before stale pointer use |
| `sway/tree/node.c` `node_init` | Assigns process-local monotonically increasing `node->id` | A `con_id` is transient and cannot identify a surface across restart or prove same-window lifetime |
| `sway/ipc-server.c` `IPC_GET_TREE` | Returns the ordinary JSON scene/tree snapshot | Read-only diagnostic, **not** a trusted selector/attestor or guarded command |
| `sway/commands/resize.c` `resize_set_floating` | Clamps target width/height and changes `pending.x -= grow_width / 2`, `pending.y -= grow_height / 2` before `arrange_container` | **Resize can move x/y.** TimeW's CI assumption of unchanged position is false for this native path |
| `sway/commands/move.c` `cmd_move_to_position` / floating move helpers | Change floating coordinates, including output/constraint handling for certain paths | A generic `swaymsg` command is not a protected operation on the selected object |
| `sway/tree/arrange.c` `arrange_workspace` | Output/workspace geometry changes can reposition floating containers | Geometry and output must be reobserved after each command and on topology changes |
| `sway/desktop/transaction.c` | Pending/current compositor states and scene animations/commits | Command success is not proof of final settled geometry or a durable completion receipt |

In particular, the exact source-level behavior for ordinary floating `resize set` is centered resizing: `new_x = old_x - (new_width - old_width)/2`, and analogously for y, subject to integer rounding, min/max constraints and subsequent compositor behavior. **Do not bake a promise of unchanged `x/y` into the protected protocol.** Runtime acceptance must confirm final settled geometry on the pinned release.

The separate Social `stage_director_social` grouping is not a general TimeW authorization issuer and should not be reused as one.

## 2. SwayPX owner decision: REVISE, conditional native design

SwayPX **accepts the principle** of a compositor-origin, explicitly selected, revocable live surface. We **revise** the proposed native interface pending the following versioned decisions:

1. **Explicit selection:** A future TimeW selection mode must be independently armed via an approved protected request and completed by a distinct intentional operator action on the actual compositor hit-tested *live* floating XDG surface. Ordinary focus, an unguarded `GET_TREE`, remembered `con_id`, title, PID, app ID, or window-class match cannot select it. A synthetic/remote/input-injection event must not be misrepresented as verified physical operator input; the input provenance/ceremony needs owner review.
2. **Object lifetime:** SwayPX holds a **private**, non-exported container/surface reference only while valid, with compositor boot epoch + per-surface minted generation/lifetime token. External consumers get an opaque short-lived selection handle, never a pointer. Any new compositor process, destroy, unmap, replacement, token reuse, approved output/workspace drift, seat/session/lock drift, or relevant geometry change revokes or requires an explicit fresh authorized selection. Seat and output policy must be specified rather than assuming universal focus revocation.
3. **Guarded operations:** The compositor must bind each command to *the same* selected live object, pre-command rectangle/seat/output/workspace, source/issuer lease, TimeW saved checkpoint and target, **already durably spent C5 operation authorization**, and operation+step identity. Revalidate within the compositor-controlled mutation critical section. A raw IPC `[con_id=...] resize/move` cannot substitute.
4. **Separate authorities:** SwayPX owns the real selected object and geometry commands, **not** C3e/C4b independently enrolled release/process/host/key verification, and **not** C5 visible human intent or protected one-use spend. No Hospital or Brave PAIR v1 key/audience may be reused.
5. **Observable outcome:** Command accepted, configure/transaction acknowledged, and exact protected post-command observed state are different claims. Do not return `COMPLETE_VERIFIED` on a shell exit, ordinary IPC success, or only matching `GET_TREE`.
6. **No retry on ambiguity:** Lost response, crash, stale source, expired lease, changed object or uncertain C5 spend produces `UNKNOWN/HOLD`. Never replay, issue a second grant, silently undo, or continue the move after an uncertain resize.

## 3. Proposed SwayPX conceptual interface (NOT existing commands or wire schema)

These names identify responsibilities for joint review, **not actual CLI verbs, socket names, trusted identities, or an install instruction**.

| Stage | Compositor-controlled input and check | Proposed result |
| --- | --- | --- |
| `ARM_SELECTION` | Separately authenticated TimeW request, independently issued C3e challenge and approved seat/session, short expiry; no selected object yet | `ARMED`, no geometry authority |
| `SELECT_EXPLICIT` | Approved gesture in `handle_button` hit-tests exactly one mapped, floating, nonfullscreen top-level surface on bound seat; native identity/lifetime minted by SwayPX | Opaque handle + selected object snapshot, not authorization |
| `WITNESS_CURRENT` | Recheck live pointer-held object, epoch/lifetime/generation, seat/session/lock, output/workspace, floating status, before rectangle, target commitment and challenge | Read-only positive or `REVOKED/HOLD`; C3e must still authenticate producer |
| `GUARDED_RESIZE` | Authenticated bound operation, confirmed C5 SPENT evidence and pre-state, same selected object, positive dimensions/constraints, no concurrent operation; mutate only under compositor guard | Command-specific correlation + *provisional* outcome; no claim that placement is complete |
| `OBSERVE_RESIZE` | Match same object and epoch, accepted resize bounds, actual post-resize `x/y/width/height` and any x/y shift; include transaction/configure state | Protected observation or `UNKNOWN/HOLD` |
| `GUARDED_MOVE` | **Same spent operation**, new expected rectangle from `OBSERVE_RESIZE`, fresh source/seat/lease check, exact target position and object | Provisional move result or `UNKNOWN/HOLD` |
| `OBSERVE_FINAL` | Fresh exact object/epoch, final settled rectangle, output/workspace/seat and operation receipts match target, approved constraint/tolerance policy | Evidence TimeW may reconcile as verified, otherwise `UNKNOWN/HOLD` |
| `REVOKE` | Destroy/unmap/reuse, lock/seat/session/output/workspace drift, expiration, unexpected geometry, new compositor epoch or operator cancel | Invalidate opaque selection; no further commands |

### Minimum per-operation correlation

Propose separate versioned **source witness** and **guarded geometry** contracts containing:

- `compositor_epoch` + `selection_generation` + opaque `surface_lifetime` token, with `con_id` only a transient diagnostic reference.
- Exact selected top-level `wl_surface` lifetime privately pinned and verified by SwayPX; map/unmap and `node.events.destroy` revocation.
- Authenticated active seat/session, lock state and allowed pointer-input provenance; selected output/workspace, floating/nonfullscreen and pre-command rectangle.
- C3e-issued challenge/lease identity, separately enrolled SwayPX producer release/process/host proof and revocation epoch. C3e/C4b owns verification and approved trust roots.
- Immutable TimeW checkpoint ID, saved window key, before/target rectangle hashes; no Scope B Brave UUID inference.
- Separate C5 exact-geometry consent/grant ID and **protected durable SPENT outcome before resize**, correlated to operation ID; no SwayPX claim that a TimeW-local SQLite marker itself proves spend.
- Per-command step ID, expected pre-state, compositor mutation/transaction/configure observation and final geometry, result causality and lease/seat/source continuity.
- Versioned failure domain for `DENIED`, `REVOKED`, `UNKNOWN/HOLD`; ambiguous outcomes must not masquerade as `REFUSED` or `COMPLETE_VERIFIED`.

The **protected transport/receipt schema and emitter enrollment remain unchosen**. The future service must not accept trust roots, policy, socket, release floor, proof timestamps or authority from a caller-supplied packet. C3e/C4b/C5 owners must separately approve those custody mechanisms. No generic Sway IPC extension may become a privileged issuer merely because it returns a new JSON field.

## 4. Geometry and crash semantics for the consumer

TimeW Scope A is one operation containing **resize then move**, not two independent C5 approvals.

1. Read the selected live floating surface under `WITNESS_CURRENT`. TimeW records a durable exact intent, and C5 independently **spends once before the first side effect**. A missing/unknown spend record is `UNKNOWN/HOLD`, not unspent.
2. Guard resize using the old observed rectangle and the original approved operation. `resize_set_floating` may change x and y while clamping dimensions. SwayPX must report **actual** post-resize rectangle and whether configure/transaction settled.
3. Only if that protected observation succeeds, guard move on the same live object using **the newly observed resize rectangle** and original spent operation. Revalidate expiry, seat/lock, output/workspace, native release and object identity. Do not assume the original `x/y` or synthesize a substitute grant.
4. Observe final settled rectangle with native receipts correlated to the same object, C5 spend and operation. Any source, acknowledgment, geometry or write ambiguity means `UNKNOWN/HOLD`; TimeW records manual review, no retry/undo.
5. C5 SPENT without a durable TimeW dispatch marker, or the inverse, is an **independent cross-authority crash gap**, not fixed by SwayPX. Until C5/TimeW approve durable reconciliation semantics, guarded dispatch must remain disabled.

A compositor crash may erase ephemeral selected-object state. Reopening an old `con_id` cannot authorize another surface even if a later window has identical title/PID or the same numeric ID. A *new* selection and review is required; whether a formerly spent intent can be reconciled is a C5/TimeW owner decision, never an automatic replay.

## 5. Ownership / implementation collision plan

**Documentation-only scope now.** A future implementation (separately reviewed and authorized) could keep TimeW-owned compositor state isolated in a proposed `include/sway/timew_selected_surface.h` + `sway/timew_selected_surface.c`, but **do not create those files as a privileged implementation before trust and owner consent**.

Required shared hooks, only after coordination:

| Seam | Why it is needed | Concurrent ownership / constraint |
| --- | --- | --- |
| `sway/input/seatop_default.c` | Verified explicit pointer selection on a bound seat | Mouse/Keyboard seatop work; decide selection gesture and physical-input provenance first |
| `sway/input/seat.c` and seat/session lock lifecycle | Seat origin, lock/revoke, focus-related eligibility | Company MapLibre [draft #126](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/126) owns active focus/seat changes |
| `sway/tree/container.c`, native XDG lifecycle | Destroy/unmap/replacement revocation, no pointer-after-free | Coordinate shared scene/transaction hooks; do not layer an independent stale object cache |
| `sway/desktop/transaction.c`, `sway/tree/arrange.c`, `sway/commands/resize.c`, `sway/commands/move.c` | Guard mutation vs pending/current and settled x/y/w/h; define receipts | Preserve Social resize behavior; current `resize set` is centered, not fixed x/y |
| `sway/meson.build`, input/output epoch state | Compile isolated module and track compositor/output generation | Autoscroll/input lifecycle [draft #158](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/158) owns concurrent build + epoch work |
| Sway native client/IPC boundary | Authenticated producer-to-C3e and C5-spent operation admission | Cannot simply expose `ARM_SELECTION` or `GUARDED_MOVE` through ordinary `swaymsg` |

Do not edit #126 or #158 branches, or merge this architecture into Social seatops, from this contract. The current Fedora `1cbca167` gray-screen/SELinux investigation is **separate**; no proposed native source/receipt is considered physically accepted until a safely installed build starts with Quickshell and survives controlled test/restoration.

## 6. Required tests before enabling any command

**Isolated source/CI refusal tests:** no selection on focus/GET_TREE alone; two same-PID/title/app Brave-like windows; unapproved synthetic/virtual input; stale challenge; foreign seat/session; lock; destroy/unmap/reuse; compositor restart; changed output/workspace; hidden/fullscreen/tiled; changed before rectangle; replaced/revoked key/release; grant unspent/unknown/replayed; wrong Hospital/Brave audience; duplicate operation/step; expired lease between commands.

**Native disposable compositor tests:** explicit trusted click picks only the second of two identical-app windows; pointer/focus interference; revoke-before-free and per-input-seat paths; size min/max clamp; centered `resize set` changes x/y; post-resize state actually observed before guarded move; lost resize or move ack; configure delayed; compositor restart after C5 spend; two concurrent ops on one surface; no output or usable-area changes falsely treated as exact final placement; mutation blocked when any input fails.

**Physical Fedora acceptance, only with separate operator approval:** installed exact release/process/SELinux/UID/seat provenance, real selected-object and C3e/C5 trust chain, one-use spend evidence, reliable native command and settled receipt, zero unexpected desktop harm. Do not use the failing `1cbca167` GDM entry as an installation test without resolving its separate diagnostic blocker.

## 7. Decisions / rollout gates

| Decision owner | Required ACCEPT / REVISE / HOLD | Present decision |
| --- | --- | --- |
| **SwayPX** | Exact selection gesture, lifetime/revocation, guarded native operations, actual post-resize x/y and receipt stage | **REVISE** as above; no native implementation authorized |
| **C3e/C4b** | Independently approved installed producer/release/seat, protected challenge/receipt and replay/antirollback/revocation | PENDING |
| **C5** | Human-visible TimeW-specific exact intent, unlocked seat, separately protected pre-action one-use spend and crash-gap reconciliation | PENDING |
| **TimeW** | Consume only the accepted witness; use actual post-resize geometry; preserve UNKNOWN/HOLD; keep Scope B Brave distinct | Proposed in merged docs-only #2117; no production contract |
| **Nurse 2 and operator** | Independent native security review, source acceptance, separate merge/install/production approval | PENDING / production HOLD |

**Next bounded SwayPX task:** coordinate this v0 design on [TimeW issue #2067](https://github.com/kudokudo1/The-Post-Apollo-Project/issues/2067), request exact C3e/C4b/C5 responses, then implement *only* an isolated deny-by-default object-lifetime/state fixture on an owned source branch after the explicit gesture and custody boundary are accepted. No existing native command implementation, real Sway move, local workstation change, key issuance or runtime activation is included in this PR.
