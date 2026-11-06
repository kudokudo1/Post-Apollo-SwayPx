# 🏥💜 Stage Director Company M29 // Real running MapLibre ELF identity evidence

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre native containment
ASSIGNMENT // Runtime M15 installed-file inode evidence on a held Company M28 pidfd
STAFF ROLE // Nurse 2 (operator-delegated external Company integration ownership)
PROVIDER // OPEN AI // CHATGPT
CONNECTION // Hospital-compatible external; not registered in live Hospital
BASE // SwayPX master 2bf5bb4ec558df3be67c6b24f53c1477cceeaf51, M28b #83 merged
DEPENDENCY // Company M28c #1392 merged; SwayPX M28a–M28b merged
PHASE // Isolated source milestone subject to exact-head GitHub CI

## IMPLEMENTATION

The real user-local launch chain is:
1. `CompanyVectorSidecarController.qml` owns a Quickshell.Io.Process and launches fixed `CompanyVectorSidecarLaunch.py`.
2. That Python component uses `os.execv` into `~/.local/libexec/post-apollo/company-vector-probe`, an **M15 Python bridge**. That bridge runs owner-only Qt runtime checks and then `os.execve` into the actual **ELF executable** `~/postapollo-vector-lab/company-vector-build/company-vector-probe`.
3. Both exec operations preserve the same Linux PID. M28a holds a kernel pidfd proving child identity, and M28b receives it only through the physically selected Company QPA client after a private launch-intent request.

M29 adds a read-only **actual executing ELF inode** comparison to distinguish the M15-approved *runtime installation path* from a foreign Quickshell child:

- A new `stage_director_company_executable.c/.h` first rechecks the already-held M28 pidfd and Linux `/proc/<pid>/stat` parent, UID, PID and process start time.
- The compositor obtains the selected host's system home using `getpwuid_r(uid)`, not `HOME`, a client-supplied file path, Wayland request, app ID or title.
- It opens the home and `postapollo-vector-lab/company-vector-build` directory components with `O_NOFOLLOW`, `O_DIRECTORY` and `O_CLOEXEC`; checks each is owned by the same non-root user and not group/other writable.
- It opens `company-vector-probe` with `O_PATH|O_NOFOLLOW` and checks a regular, owner-executable file, no setuid/setgid bits, no group/other writable bits and a live link count.
- It independently opens the *kernel's actual executing inode* at `/proc/<child pid>/exe`, reads its `st_dev` and `st_ino` with `fstat`, checks exact equivalence to the owner-checked file, then `fstatat(AT_SYMLINK_NOFOLLOW)` verifies that the current approved path still names the same regular inode. All handles close regardless of the result.
- It rereads host and child Linux identities and checks the kernel pidfd again after path comparison. Replaced files, symlink injections, missing paths, other binaries, process exit, changed UID/PPID/start generation and unsafe owner directories fail closed.
- The existing selected Company protocol checks its M28b actor candidate first; a M29 result appears only in the read-only `[stage-director][COMPANY_M29][EXECUTABLE]` diagnostic.
- Native CI tests fork and `execv` a real copied ELF from an owner-only synthetic installation, acquire a real Linux pidfd, verify exact inode and then attempt malicious path replacement, symlinks, mode loosening and process generation mismatch. Full SwayPX and older M23/M28b protocol regression suites compile with the new module.

## HOW TO USE

No new user action, local installation, Quickshell restart or SwayPX activation. Existing **Company → Coverage → VECTOR VIEW** uses the original separate Qt MapLibre window with the optional bridge OFF by default. The reported state `CURRENT_INSTALL_INODE_CANDIDATE` means *that live Linux process executes the same inode currently installed at the user-local lab path*, nothing more.

## VERIFY

- Current SwayPX master `2bf5bb4e` and Company main `eeaad9ea` checked before branch creation, with no active competing Company/MapLibre PR in either target.
- Syntagma AGENTS, OPERATE/HOSPITAL-AGENT-PROTOCOL and AUTHORITY-AND-PERMISSIONS rechecked, continuing external Nurse 2 delegated Company scope without assuming live Hospital registration.
- Original M15 Python bridge and installer source inspected. Its last exec target is the lab ELF, **not** the installed M15 Python bridge. The M29 check names the former.
- GitHub CI native real exec and pidfd tests, v1/v2 Wayland server compatibility, full Nix compositor build and headless/Weather checks required before merging.
- **Not yet verified**: live user-local Qt child inode, pinned executable content hash/signature, exact installer manifest trust, per-click process provenance under compromised Quickshell, real scene/input/popup containment, operator visual acceptance.

## WATCH OUT FOR

- **Critical trust distinction**: the expected ELF file lives under the user's writable home. A same-UID program can replace its contents before the legitimate launch and still pass the inode comparison if the replacement is the one executing. M29 proves *actual ELF inode equals current installation path*, not independent content approval or a cryptographic signature.
- The check does not hash ELF content against a trusted manifest and does not verify plugins and dynamically loaded Qt libraries. Never set `executable_approved`, `launch_authenticated`, `owner_verified`, actor custody, renderer, clip, focus, keyboard, pointer, or popup rights true because of M29.
- Physical M24 pairing and pidfd M28b reduce spoofed clients/process races but cannot defend against a compromised selected Quickshell process.
- The compositor determines home through passwd/NSS. If the home is a symlink, lacks safe ownership, or is inaccessible under procfs restrictions, M29 reports an unsafe/missing candidate; do not weaken checks simply to obtain a positive status.
- Native map windows may map before the pidfd receipt; current mapping observer is opportunistic. Do not claim complete end-to-end birth matching without a later retained XDG inventory/retry path.
- No local machine modifications, package installation, Wayland socket replacement, Quickshell restart, SwayPX production activation or unrelated Weather/Chrome work were undertaken.

## CHECKLIST

- [x] Inspect M15 exact installed bridge and final real Qt ELF target
- [x] Verify M28 pidfd and selected native client/host actor candidate gates
- [x] Implement Linux real executable inode and owner path comparison
- [x] Detect replaced binary/symlink/mode/unsafe parent/generation
- [x] Connect to native read-only Company map observer after M28 checks
- [x] Add real-fork/exec native tests and full SwayPX CI
- [ ] Exact-head CI green
- [ ] Independent source review and operator-authorized merge decision
- [ ] Trusted operator-pinned M15 executable digest/manifest, plugin load integrity
- [ ] Retained actor ledger with M21 revocation and asynchronous receipt reconciliation
- [ ] Isolated scene pixels, crop, pointer, keyboard and popup verification
- [ ] Controlled local deployment and operator final acceptance

## NEXT

After successful M29 CI, merge only within delegated Company source scope. Next establish an **operator-pinned trusted executable digest** (not supplied by the Company process itself) and account for the owner-local Qt binary/plugins. A kernel process generation and currently installed inode alone are insufficient to approve scene authority. In parallel, reconcile map-time vs fd-receipt ordering in M21's revocable actor ledger under isolated SwayPX tests. Only then consider real compositor window embedding.

## DECISIONS

M29 is an isolated read-only evidence milestone. It adds no new client request and does not alter M15/M17 local artifacts. Stage Director Weather/Chrome ownership remains separate. No production state or operator acceptance is inferred from CI.
