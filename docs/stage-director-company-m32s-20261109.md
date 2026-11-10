# 🏥 Company M32s // Native MapLibre bundle inventory preflight (production OFF)

PATIENT // Post-Apollo-SwayPx
ROOM // Company → COVERAGE → VECTOR VIEW
STAFF ROLE // External Hospital-compatible Nurse 2 continuity; live registration unverified
ASSIGNMENT // Build a fail-closed file/dependency/Qt-plugin inventory check before an actual trusted launch custodian
BASE // M32r PR #133; stacked on #132 → #130 → #129 → #126
STATE // Source and CI preflight; no production launch or scene

## IMPLEMENTATION
- Added `stage_director_company_bundle_inventory_probe_fd`, a **read-only internal test/review seam**. Given a pre-opened bundle-root FD and a separately trusted manifest, it visits bounded `bin/`, `lib/`, and `plugins/` directories.
- Reuses the existing M30a SHA-256 file descriptor verifier instead of inventing another digest implementation. Verifies one fixed-name native `company-vector-probe` executable and at least one listed library and Qt plugin. Maximum 24 entries.
- Rejects malformed, placeholder, duplicate or escaping file entries, symlinks (including interior directory links), mismatched owner UID, group/world writable paths, device/fifo/nonregular files, unexpected hardlinks, file replacement during checking, wrong digests and incomplete **listed** manifest shape.
- Every result is **candidate-only**. There is no approved production bundle root, pinned binary hash, dependency set, runtime dynamic loader enumeration, compositor process birth, gesture, permission or scene caller.
- The current installed user-owned executable is **not silently reclassified** as a trusted immutable image. This verifier is compiled into SwayPX but never called by the production protocol. No external input can supply an approved manifest.
- A future deployment must choose a separately installed and reviewed *trusted bundle root*, artifact build reproducibility/ownership and the exact runtime dependency/plugin load closure. Merely listing matching files cannot establish full closure.

## HOW TO USE
Nothing to install or restart on Fedora. Existing standalone Company MapLibre still works as before; production embedded scene remains off.

## VERIFY
GitHub exact-head workflow: verify after PR creation. Standalone C tests use temporary, synthetic bundled files and exercise valid listed content, rejection of changed hash, duplicate or traversal manifest entries, zero digest placeholders, mismatched ownership, writable directories/files and symlinks. Source mutation checks and normal/private compositor builds are included; old M32r/M32p source checks continue. **Do not interpret a synthetic test match as a trusted build.**

## WATCH OUT FOR
- M32s is a *listed-file inventory*, not audited complete DT_NEEDED/Qt dlopen closure, not cryptographic origin signing, not protected image immutability and not safe anti-swap launch custody.
- It does not validate the running MapLibre PID. Existing M29/M30 checks remain diagnostic, and the trusted production digest is deliberately empty.
- No physical approval hotkey has been selected and M24 F11 host selection still does not authorize the MapLibre actor.
- No Fedora device or filesystem inspection; no actual GPU/dual-monitor acceptance.

## CHECKLIST
- [x] Source-only bounded hash inventory for intended ELF, libraries and plugins
- [x] FD-based non-following traversal, owner/mode/stability checks and manifest validation
- [x] No authority fields, no executable approval, no production caller
- [ ] CI exact-head verification
- [ ] Operator-reviewed artifact and **complete** dependency closure
- [ ] Compositor-controlled launch identity/custody and anti-swap image
- [ ] Distinct trusted physical approval ceremony, real revocable scene lease, UI, Fedora acceptance

## NEXT
After CI, continue with *pre-spawn custody protocol design* using a reviewed image root and file-descriptor based execution. Preserve the current Company QProcess standalone fallback. Do not choose a gesture, pin any CI hash, deploy, modify Company controls or enable production while ownership and operator approvals remain outstanding.

## DECISIONS
No production integration, merge, installed files, local machine changes or final operator acceptance. A successful list match alone has zero launch or rendering authority.
