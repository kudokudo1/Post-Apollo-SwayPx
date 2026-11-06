# 🏥💜 Stage Director Company M28a // Linux PIDFD child-birth custody evidence

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre
ASSIGNMENT // Kernel pidfd continuity foundation for a future selected-Company protocol v2 receipt
STAFF ROLE // Nurse 2, operator-delegated external implementation owner
PROVIDER // OPEN AI // CHATGPT
SESSION // External Hospital-compatible; no claim of live Hospital registration
BASE // master e620efe944b8f224fb902a9d4960c9f136da5905
DEPENDENCY // M27 #81 merged; Company M26 #1385 merged
STATUS // Isolated source milestone awaiting exact-head CI

## IMPLEMENTATION

In M27, the physically selected Company Wayland client submits a launch-intent timestamp. SwayPX checks the native Qt XDG window's real Linux parentage and birth window. This is insufficient to distinguish two simultaneous Quickshell children. Quickshell.Io.Process has a read-only `processId` property, allowing the existing attached Process to identify its own child without an arbitrary shell call.

M28a adds `stage_director_company_birth_pidfd.c/.h` to the ordinary SwayPX compilation as an **unactivated private native ledger**:

- Accepts a Linux pidfd received through a future *selected Company-client* Wayland fd request; it cannot be a username, app ID, PID string or public environment token.
- Calls the kernel `pidfd_send_signal(fd,0)` as a no-signal lifetime check; reads `/proc/self/fdinfo/<fd>` to obtain the actual kernel PID, rather than trusting the claimed PID.
- Checks the child against **live** `/proc/<pid>/stat` identity for the exact observed Quickshell/Company host PID, owner UID, start ticks and direct parentage, with bounded timing relative to the compositor-observed M27 intent.
- Duplicates the incoming pidfd with `F_DUPFD_CLOEXEC` and rechecks process identity after duplication. The caller can close its transfer descriptor without losing the tracked kernel process generation.
- On a candidate XDG map, verifies the real wl_client differs from Company host, the root XDG actor's native PID/start/UID exactly matches the held pidfd process, M22 direct-child lineage still holds, and a final kernel/process read remains stable.
- Revocation closes the held fd. After exit/reaping, matching fails and cannot silently promote a reused integer PID.
- **Every result is evidence only.** The struct explicitly returns `exact_executable_attested=false`, `launch_authenticated=false`, `owner_verified=false`, and no permission to pair actor, move pixels, crop, accept input, change focus or show popups.

## HOW TO USE

There is no new Company UI control or local command. The existing Company → Coverage → VECTOR VIEW → OFFLINE TEST / ONLINE STREETS continues through the original attached Quickshell M15/M17 process. This M28a class is **compiled but has no producer/caller** in the ordinary compositor. It cannot run until a reviewed Company-specific v2 Wayland fd request and M25 QML pidfd transfer are implemented.

## VERIFY

- Canonical Syntagma AGENTS / Hospital Agent / Authority rechecked.
- M27 exact merged SwayPX head and current Company main inspected. No conflicting open Company PRs.
- Official Quickshell Process interface checked for `processId` (read-only) and managed lifetime.
- Native C tests use a **real forked Linux child**, a real pidfd opened by `pidfd_open`, a real kernel `/proc` identity, and process exit/reaping. Negative cases include a pipe masquerading as pidfd, wrong claimed PID, foreign host client, missing intent, old birth, mismatched XDG actor and changed host/process generation.
- CI reruns the M27 origin tests, fails if any scene/seat/executable-authentication authority appears, and builds ordinary pinned-Nix SwayPX.
- No actual Qt map Wayland handshake, executable path attestation, or local machine runtime was tested in this source milestone.

## WATCH OUT FOR

A pidfd is stronger than a PID because it holds a kernel process identity through integer reuse; it still does **not** prove which executable ran or that an explicit Company click launched it. A compromised Quickshell process might pass a pidfd of an unrelated direct child. Never call this a production authenticated MapLibre actor. A future v2 Wayland handler must verify exact host wl_client and one M27 selection-epoch/intent generation, then transfer the fd while tying it to Quickshell's actual Process.processId. A separate executable identity check and per-launch authority exchange remain necessary.

This work does not modify Weather WATCH, historic Chrome SceneFX experiments, Company desktop QML or the user-local M15 installer.

## CHECKLIST

- [x] Verify exact M27/main and relevant Hospital rules
- [x] Inspect Quickshell Process.processId and existing fixed exec chain
- [x] Implement kernel pidfd validation, kernel PID lookup, CLOEXEC holding and rechecks
- [x] Validate native parent generation and intent birth window
- [x] Verify actor XDG/native identity against held pidfd with no rights
- [x] Real Linux fork/pidfd/exit native test and full SwayPX CI
- [ ] Verify all checks on exact PR head
- [ ] Review and merge under operator-authorized Company scope if green
- [ ] M28b: Company v2 Wayland fd request + M25 pidfd transfer + QML Process.processId
- [ ] M28c: verified M15 executable identity / per-launch provenance
- [ ] Integrate M21 revocable Company XDG actor candidate
- [ ] Isolated scene clip/pointer/keyboard/popup and live acceptance

## NEXT

M28b is the wire: extend private Company protocol to version 2, additive fd request accepted **only** by the physically selected wl_client with fresh M27 intent. Pass actual Quickshell.Process.processId to the optional M25 Qt bridge, open pidfd in C++, transfer it via Wayland (no second display connection), hold it on SwayPX, and match it to the mapped native Qt actor. Gate on protocol version 2 so older plugin/compositor combinations fail closed.

After that, verify process executable provenance and connect M21's revocable native actor ledger. Only then review controlled pixel/input containment.

## DECISIONS

Operator authorized bounded Company Stage Director GitHub development/merges. No local-machine write, package installation, Quickshell/SwayPX restart, production activation, unrelated Hospital room ownership or final user acceptance is implied.
