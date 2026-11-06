# 🏥💜 Stage Director // M30b asynchronous actor/receipt reconciliation foundation

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre Integration
STAFF ROLE // Nurse 2, external Hospital-compatible continuation
PROVIDER // OPEN AI // CHATGPT
CONNECTION // External; no live Hospital registration assumed
ASSIGNMENT // M30b isolated native actor and pidfd birth order reconciliation
BASE // M30a branch 3efe4872553f0a6c864a7a7d46a8924bbcbf542f (M30a PR #85, draft)
PHASE // Standalone test/compiled source; NOT installed or production-wired

## IMPLEMENTATION
- Add `stage_director_company_reconcile.c/.h` as a bounded, single-candidate state machine that holds a mapped native Company XDG actor candidate and a separate held M28 kernel child receipt, in **either event order**: map first, receipt second; or receipt first, map second.
- Both events must agree on exact compositor-native host identity, Linux process generation, selected host client and resource identity. Matching rechecks the original live held kernel pidfd and M22 direct-child lineage through M28 `stage_director_company_birth_pidfd_match_actor`, then records exact Wayland resources in the existing M21 revocable candidate ledger. No client string, namespace, title or PID supplies a trusted pairing.
- Attach actual `wl_resource` destruction listeners while waiting, including on map-before-receipt. Explicit unmap/revoke and role/surface/root destruction close the staged state and the M21 child candidate. Wrong epoch, stale process generation or wrong actor invalidates the sequence.
- Reconciliation requires fresh compositor-native host and actor observations at time of completion, rather than trusting the map-time cached geometry.
- This module is built into SwayPX but has **no production caller yet**. The actual M28b Wayland receipt callback and mapped XDG inventory must be connected in a separate source milestone after this isolated regression suite. It cannot currently pair a real desktop map.
- M21 candidate status is strictly evidence. All output fields for renderer, clip, pointer, focus, popup, seat and owner remain false; no M19 approval lease.

## HOW TO USE
Nothing to install or press. Company → Coverage → VECTOR VIEW still uses the existing standalone launcher; private Qt bridge is disabled by default. No compositor restart or local file writes occurred.

## VERIFY
- Isolated CI creates real Wayland server clients/resources and a real fork/exec child with Linux pidfd. The native Wayland client process metadata in the fixture is synthetic, however; tests do not prove a real Qt/QPA map child attached to the desktop.
- Test map-first/birth-first, wrong process generation, stale intent, role destruction and explicit unmap. Preserve original M21 resource teardown tests.
- Rebuild the full ordinary compositor without activation.
- Still missing: production event hook, safe mapped XDG inventory across callback order, pinned executable trust, real Fedora Qt actor and scene/input/popup verification.

## WATCH OUT FOR
- `stage_director_company_reconcile_birth` accepts a pointer to an **already validated compositor-owned M28b held pidfd**, not a client-claimed PID. The final matching function revalidates the kernel pidfd and process before the M21 candidate.
- There is deliberately no production callback yet, because stale pointers to user-managed windows must never survive unmap. Integrating real compositor hooks needs independent lifetime review and CI.
- M30a still has an unpinned production digest; do not treat successful M21 candidate pairing as binary/launch approval or permission to manipulate pixels, focus or input.
- Separate Weather WATCH and Chrome experimental owner scopes are not modified.

## CHECKLIST
- [x] Build two-arrival-order candidate state machine with exact native resource identities
- [x] Link to live M28 pidfd + M22 parentage check and M21 resource candidate ledger
- [x] Revoke on root/role/surface destruction, explicit unmap and mismatch
- [x] Add real-process/Wayland test fixtures and full SwayPX CI
- [ ] Exact-head CI, independent source review, source merge
- [ ] Production mapped-actor inventory and receipt callback wiring
- [ ] Operator-approved executable digest, Qt dependency integrity
- [ ] Real native pixel/input/popup containment and final operator acceptance

## NEXT
Verify exact PR checks. Then attach the bounded reconciler to the real compositor's native mapped-view inventory and the M28 Wayland fd handler, preserving map-before-receipt and receipt-before-map safety. This requires explicit unmap/destroy revocation, fresh reobservation before candidate, and no scene authority. Do not merge stacked PRs out of order or perform local activation under this source assignment.

## DECISIONS
Stay in Company MapLibre integration scope, keep Weather/Chrome untouched, no local machine mutation, no production activation, and no final user acceptance assumption. M30a source pin and M30b candidate ledger remain independent evidentiary gates until trust and runtime validation can be completed.
