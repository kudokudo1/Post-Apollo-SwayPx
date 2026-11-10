# 🏥 Company M32t // Read-only pre-spawn descriptor custody

PATIENT // `Post-Apollo-SwayPx`
ROOM // Company → COVERAGE → VECTOR VIEW in-widget native MapLibre
STAFF ROLE // External Hospital-compatible Nurse 2; live Hospital registration unverified
ASSIGNMENT // Bind M32r current selected request to M32s listed file inventory and held candidate inode before trusted launch work
BASE // M32s draft #134, stacked on #133 → #132 → #130 → #129 → #126
PRODUCTION // OFF, no new physical gesture, no runtime launch

## IMPLEMENTATION
- Added an entirely **internal read-only** M32t descriptor custodian. It accepts a current M32r request generation/native context and M32s-listed file inventory, duplicates a borrowed pre-opened bundle root FD with `F_DUPFD_CLOEXEC`, opens the fixed `bin/company-vector-probe` with `O_NOFOLLOW`, and retains only read-only FDs. There is no method to export an executable for execution.
- On capture, repeats selected request and whole listed-file hash/inode checks around the descriptor hold. It also rechecks file/path inode and metadata against the held executable.
- On probe, validates the same M32r generation/host/output/mode/geometry, bounded monotonic timestamp (5s max, capped by the original request's expiry), retained root and executable inode, current installed path, and EVERY listed library/Qt plugin.
- Requests that change, old asynchronous generation, stale/expired time, modified dependency bytes, swapped executable, revoked native request or changed path are denied or revoked. `last_generation` persists after revoke.
- Always returns **listed bytes candidate only**, with **zero** process/scene/seat/launch/operator authority. No production protocol caller was connected.

## HOW TO USE
Nothing to install on Fedora. The Company MapLibre standalone QProcess/Python `execv` path remains unchanged and the native in-widget scene remains OFF.

## VERIFY
Exact-head CI will run synthetic C tests with temporary files and M32r synthetic request data plus source mutation and earlier regressions, ordinary/private Nix builds. An all-green result demonstrates only the read-only pre-spawn mechanics; it does not prove a trusted installed map actor.

## WATCH OUT FOR
- A pre-opened bundle-root FD does NOT authenticate the caller or immutable distribution root. M32s's manifest is a **listed-file subset**, not proven complete Qt `dlopen`, platform plugins or ELF `DT_NEEDED` closure.
- The host or owner can still mutate their own executable and dependencies; revalidation only detects changes when called. No lock, read-only mount, privilege-separated custodian, process execution or post-exec image proof is present.
- This code never starts a process, obtains a pidfd, approves the real installed ELF, authenticates physical input, creates a scene lease, or changes Company UI. F11 host selection is not actor authorization.
- Any trusted-fd executor must be separately approved, review file descriptor inheritance/close-on-exec, running loader image provenance and cancellation/teardown, and avoid introducing a new unsafe elevation path.

## CHECKLIST
- [x] Selected M32r generation and native context bound to held read-only descriptors
- [x] M32s listed files rechecked at capture and probe
- [x] Bounded expiry, old-generation refusal, file replacement/dependency mutation revocation
- [ ] Exact-head CI and normal/private compositor builds
- [ ] Reviewed immutable installation root plus complete dependency closure
- [ ] Trusted distinct physical approval and production process launch with pidfd custody
- [ ] Real lease, Company UI ownership coordination, physical Fedora/dual-monitor acceptance

## NEXT
Verify exact-head CI, then independently review the **real artifact build and full runtime loader closure** before designing a process launcher. Production remains OFF until operator-specific gesture, artifact, launch path, PR merge and Fedora activation decisions.

## DECISIONS
No production caller, no merge, no executable pin, no local Fedora change and no final acceptance.
