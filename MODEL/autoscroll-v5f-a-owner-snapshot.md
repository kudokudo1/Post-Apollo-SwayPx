# V5F-A // Compositor-Owned Four-Object Snapshot (HARD OFF)

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Mouse & Keyboard · Universal Autoscroll
STAFF // External Hospital-compatible DOC, not verified as a live Hospital session
ASSIGNMENT // Build fail-closed observation of four owner-minted lifetime tokens and test against the pinned V5C guard, WITHOUT live input authorization
BASE // D3-C feature commit 610997493c005be0ebe574c0773265355f78739a
V5C SOURCE PIN // master commit fb064cf3c82aa7b9e8d2375bb631773d5eb5a12f

## CHECKPOINT CORRECTION / GITHUB TOPOLOGY

As of November 19, GitHub marks the V5E child PRs merged because they were merged **into their parent feature branches**. The D1–D3 source integration has NOT reached master; the master baseline at preflight was fb064cf3c82aa7b9e8d2375bb631773d5eb5a12f. Master has the V5C portable epoch guard; the D3-C feature branch has the real D1/D2/D3 lifetime owners but **not** the V5C guard files.

F-A therefore stacks on D3-C. Do not assert that its source already runs on the operator's desktop or that the V5C guard is linked into the feature branch. A later conflict-aware integration is required to reconcile the two histories before any production merge.

## IMPLEMENTATION

- A new compositor-private, read-only pa_autoscroll_owner_snapshot_read(cursor, event_device, out) gathers **four actual owner-minted epoch values** from live objects. It does not create a token or mutate a seat, cursor, device, output or window.
- The input device is found by pointer equality in the input manager's live device list (not by dereferencing event_device->data or by trusting client-provided IDs), must be nonvirtual, have a live PA_LIFETIME_DEVICE slot and be attached to the specific live seat.
- The seat is found in the input manager's live seat list before dereferencing, must own the cursor and have a live PA_LIFETIME_SEAT slot.
- The scene under the cursor must resolve to an actual mapped container with a directly hit view surface. Reject popups, subsurfaces, layers, XWayland unmanaged, nonview nodes, node destruction, dirty/pending transactions and revoked PA_LIFETIME_TARGET.
- The container's current workspace output must be enabled, nonfallback, nondestroying and free of pending node transactions, and equal the real wlr_output_layout_output_at result for current pointer coordinates, with a nonzero PA_LIFETIME_OUTPUT token.
- Locked sessions, pointer constraints and pointer-from-touch/tablet simulations are refused; missing evidence returns false and zeroes the output record. Tokens must be nonzero and pairwise distinct.
- This new owner snapshot module is compiled into actual SwayPX via Meson, but **no runtime caller is enabled** and no authorization is derived from the snapshot.

## V5C GUARD INTEROPERABILITY

The dedicated CI obtains the already-merged V5C epoch/witness portable sources from the **pinned master SHA**, builds them with the current lifetime issuer and a synthetic-only C test, then demonstrates:

- Four nonzero owner-issued slots are accepted by a mock positive V5C witness.
- Revoking any one of device, seat, target or output causes the next V5C epoch tick to cancel.
- Rebinding to the **same C slot memory address** does not restore the old guard, because the issuer gives a new token.
- Replayed witness timestamps, virtual origin, Scroll Lock and changed layout witness cancel the guard.

**This is synthetic contract interoperability, NOT a live compositor event trace and NOT physical provenance.** An attacker-controlled structure can still forge the boolean/ID witness. We do not permit any synthetic positive witness in production.

## HOSPITAL / OWNER BOUNDARY

Company Map PR #126 remains paused/open and owns cursor.c, seat.c, seat.h, Meson and Company scene. F-A production source changes are **one new .c and one new header**, plus a one-line Meson registration. Meson is the ONLY new changed-file intersection with Company #126, and is necessary to compile the standalone read-only collector. Cursor/seat/Company scene/transaction/xdg/desktop output handlers remain unchanged.

Create a durable Company owner handoff identifying Meson overlap. No reassignment, blind merge or activation.

## VERIFY

Dedicated GitHub workflows enforce exact F-A changed paths, source refusal code, unchanged stage-pairing order, production PA_AUTOSCROLL_LIVE_ACTIVATION 0, strict C11 synthetic guard interoperability and a real complete Nix-built SwayPX. Inherited V5E safety workflows are adjusted on this child branch only. No native physical-pointer, window unplug, monitor move or actual compositor-epoch dynamic witness is claimed.

## HOW TO USE

GitHub review and CI only. No CPU++ feature toggle, Sway reload, user workstation write or production activation.

## WATCH OUT FOR

The V5C guard is NOT in the D3-C branch, and F-A deliberately does not import/duplicate its files into production. The CI pins it from master. F-A proves read-only source provenance for the four object-lifetime numbers, NOT physical hardware, trusted Scroll Lock, target permission, output layout generation, real event freshness or cross-restart session identity.

Even if the snapshot returns four live epoch values, that is NOT an input authorization. No code should call pa_scroll_dispatch_enable() or set physical_verified=true based solely on F-A. A future real event-source check and session-bound witness are required for every tick.

## CHECKLIST

- [x] Reconcile GitHub stacked-merges versus master reachability
- [x] Real read-only four-owner snapshot and zero-on-refusal contract
- [x] No cursor/seat/Company source edits; only Meson overlap with Map
- [x] Synthetic C11 V5C guard interoperability fixture and source tests
- [ ] Exact-head CI PASS, independent security review
- [ ] Company #126 explicit owner handoff at verified head
- [ ] Reconcile D3-C feature branch with new master and V5C epoch source
- [ ] Real dynamic headless device/seat/target/output event receipts
- [ ] Physical device provenance and native Scroll Lock proof
- [ ] Trusted session binding, CI-only native axis/Escape/indicator, operator acceptance

## NEXT

V5F-B must integrate the pinned V5C guard into a conflict-reconciled branch with a **real, freshly observed compositor witness** for every authorized tick. It must fail closed under unknown hardware origin, changing scene focus, output layout mutation or cancellation. Do not merge or activate any live input without operator permission and independent review.

## DECISIONS

DRAFT · UNMERGED · PRODUCTION HARD OFF · NO LOCAL HOST WRITES · NO COMPANY ROOM OWNERSHIP TRANSFER.
