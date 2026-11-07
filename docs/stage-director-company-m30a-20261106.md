# 🏥💜 Stage Director // M30a trusted-source digest candidate

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre Integration
STAFF ROLE // Nurse 2, external Hospital-compatible succession
PROVIDER // OPEN AI // CHATGPT
CONNECTION // External, not live Hospital registered
ASSIGNMENT // M30a read-only source-pinned ELF SHA-256 evidence
BASE // SwayPX master 24116486feb48492ce3a1f98761abbe1d64cd79d
DEPENDENCIES // M29 #84 merged; Company M28c #1392 merged
PHASE // Isolated GitHub branch and CI; no local activation

## IMPLEMENTATION
- A compositor-side, single-entry source-pinned manifest records the approved ELF SHA-256. **It is empty until the operator supplies/approves an independently established digest**. Production is therefore `UNPINNED` by default. There is no client-, HOME-, environment-, QML-, argv-, or runtime-file-provided digest.
- `stage_director_company_sha256_fd` implements streaming SHA-256 over a bounded (512 MiB maximum) open read-only file descriptor, with pre/post inode, size, mode, mtime and ctime stability checks. The hash is verified against standard empty-string and `abc` vectors.
- Once a nonempty trusted pin is separately established, the verifier gates on existing M29 kernel pidfd + current-installed inode evidence **before and after** hashing the live `/proc/PID/exe` kernel target. It distinguishes malformed pins, changed process/inode, I/O failure, mismatched content, and a candidate hash match.
- The production path is exposed only through the M28b selected-client/native-actor gate and M29 owner-home protection. No new Wayland requests, QML imports, launcher changes, global publication, or network calls.
- The result always sets `executable_approved`, `launch_authenticated`, `owner_verified`, all actor, clip, render, focus, pointer, keyboard and popup flags **false**. A file hash is evidence about an on-disk ELF, **not** a signature of executable provenance, loaded libraries/plugins, actual running memory pages or the legitimate button click.
- `stage_director_view_mapped` logs `[stage-director][COMPANY_M30A][DIGEST]` without granting rights. As with M29, actor maps before receipt may be missed; M30b must reconcile that separately.

## HOW TO USE
No UI or desktop change. Company → Coverage → VECTOR VIEW → OFFLINE TEST / ONLINE STREETS remains the original standalone launch. The optional native bridge and private compositor selection remain disabled by default. Do not pin a hash sourced only from the current user-writable installation; the operator must independently approve the intended build/artifact before a production manifest can be populated.

## VERIFY
- Strict native SHA-256 test vectors and real Linux child + pidfd integration.
- Negative cases: no pin, malformed pin, wrong digest, replaced installed inode containing identical bytes, unsafe directory, exited child and revoked pidfd.
- Existing M29 executable-inode tests preserved, production SwayPX Nix build and existing M23/M28b regressions required in CI.
- Not verified: live Fedora bridge/runtime match, trusted release signing, library/plugin integrity, asynchronous actor inventory, scene/input containment, final operator acceptance.

## WATCH OUT FOR
- Trust bootstrap is **not completed**. Neither an empty digest nor a hash copied from a user-owned binary by the same potentially compromised runtime establishes independent approval.
- A digest matching the **file** does not prove loaded Qt library/plugin content or memory integrity; owner-writable ELF can change later. This is purposely a candidate with no actor authority.
- Synchronous hashing of a pinned large binary may affect compositor latency. The fixed 512 MiB bound prevents unbounded reads but **does not** make synchronous hashing safe for production frames; defer any activation until bounded asynchronous verification is designed and reviewed.
- Previous Weather and Chrome Stage Director branches are separate and untouched.

## CHECKLIST
- [x] Independent M29, M28, M21 and Company launch source review
- [x] Safe default without approved trusted digest
- [x] SHA-256 vectors and real pidfd/exec negative tests
- [x] Selected Company M28 actor gate and M29 inode gates
- [x] Source/CI wiring without scene/seat changes
- [ ] Exact-head CI and code review
- [ ] Operator approval of independent ELF artifact and pinned digest
- [ ] M30b late-birth receipt reconciliation with revocable M21 ledger
- [ ] Dependency/library provenance and eventual asynchronous verification
- [ ] Isolated scene/input/popup containment and operator visual acceptance

## NEXT
Verify M30a PR exact-head checks. Keep the manifest unpinned without independent operator evidence. M30b must retain the native XDG actor inventory across map-before-receipt and fd-before-map orderings, matching only the current selected Company session and M21 revocable resources. No embedding permission until subsequent proof and operator review.

## DECISIONS
Continuing the previous worker's Company integration assignment under operator-directed succession; no local machine writes, SwayPX restarts, production activation, approval of installed binaries or work in other Stage Director lanes. GitHub source work only. The operator's final acceptance remains pending.
