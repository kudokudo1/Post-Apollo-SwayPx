# 🏥 Company Map // M32r inert launch-request context

PATIENT // Post-Apollo-SwayPx
ROOM // Company → COVERAGE → VECTOR VIEW native MapLibre embedding
ROLE // External Hospital-compatible Nurse 2; no verified live registration
ASSIGNMENT // Retain existing Company mode selection and prepare a separate, bounded native request for future trusted operator approval
PHASE // Source implementation / CI; production still OFF
BASE // M32p/q PR #132, stacked on #130 → #129 → #126

## IMPLEMENTATION

- The operator's "continue" follows the M32q Option A recommendation: retain Company's existing OFFLINE TEST / ONLINE STREETS buttons as **intent**, not as a trusted approval. No new shortcut was selected.
- Added a compositor-local request ledger, separate from M32o's **post-actor** scene candidate. M32r stores a **pre-approval request** for the exact currently selected native host/output, launch intent/mode, validated map rectangle, host revision, and compositor-monotonic generation.
- The real selected-host Wayland handler can reserve this inert request only following an accepted viewport report and current native host re-observation. Each current viewport replacement revokes the old request, even when malformed or hidden. CLOSE, client disconnect, native selection loss and shutdown revoke before releasing identities.
- The ledger rejects old generations, invalid rectangles/modes, clock rollback, expired requests (10-second cap), overflow, and changed native identity. Stale generations cannot cancel a newer pending request.
- A compositor-only probe re-observes the current native state and yields at most `CONTEXT_ONLY`. It never authenticates operator consent, launches a process, grants a scene, sends a new public protocol event, or claims that the current owner-local executable/dependencies are approved.
- Compiled into normal/private SwayPX as inert infrastructure. The actual positive scene setter remains CI-private, normal production OFF.

## HOW TO USE

No local installation, Fedora/Quickshell restart, Company UI change, new hotkey or production activation. The existing Company standalone map remains the fallback.

## VERIFY

M32r GitHub CI: exact-head results must be checked after PR creation. C unit tests cover generation replay, TTL, geometry/mode/output changes and non-authorizing receipts. Source/mutation tests cover selected-host re-observation and revoke ordering; earlier M32o/M32p tests and both normal/private Nix builds are included.

## WATCH OUT FOR

- This is **not** the operator's trusted physical approval ceremony. M24's F11 host selection is still not approval of an independent Qt actor; a libinput device is not automatically hardware-attested because of uinput.
- The QProcess / Python `execv` sidecar and user-owned ELF remain outside compositor-controlled anti-swap launch custody. No reviewed dependency/plugin manifest has been pinned.
- A pending request can originate from the selected Company's private Wayland intent. That is deliberate, but **client intent cannot grant authority**, even when context matches perfectly.
- The true physical Fedora dual-monitor and actual selected-client CLOSE/DESTROY acceptance remain open.

## CHECKLIST

- [x] Exact selected Company request context and compositor-owned generation
- [x] Bounded expiry, stale generation protection and native identity binding
- [x] Revoke on viewport replacement, CLOSE and protocol teardown
- [ ] Exact-head CI tests and build evidence
- [ ] Trusted installed MapLibre artifact/dependency manifest
- [ ] Compositor-custodied launch and kernel-backed process birth
- [ ] Distinct operator-approved physical input ceremony with device provenance checks
- [ ] Real revocable scene lease and Company status/cancellation UI
- [ ] Authorized Fedora dual-monitor hardware test and final acceptance

## NEXT

Verify exact-head CI, then build a **trusted artifact/launch custody** seam independent of the existing user-owned Company sidecar. Do not wire a physical approval key or production scene until the operator approves its exact gesture and the Company owner agrees to the UI seam. Keep all PRs draft and unmerged.

## DECISIONS

Option A architecture accepted for continued development. The exact hotkey, verified hardware provenance, launch custody approval, approved executable hash, Company UI edits, production actuation, merge, and local desktop changes remain unapproved.
