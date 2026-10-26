# Stage Director // GitHub-only /dev/udmabuf access smoke

**STATE:** Bounded temporary-device-access experiment on a disposable GitHub runner, not production activation.  
**PATIENT:** `kudokudo1/Post-Apollo-SwayPx`. Base `stage-director/scenefx-software-headless-v0-20261026` (PR #16, already merged into development).  
**AUTHORITY:** Operator final. No live Hospital Room/Assignment or ownership transfer presumed.

## IMPLEMENTATION

The previous SwayPX runtime tests narrowed the blocker in three reproducible stages: (1) SceneFX without DRM or software selection could not create a renderer; (2) `WLR_RENDERER_FORCE_SOFTWARE=1` bypassed DRM detection but the unwrapped Nix program could not discover the EGL vendor; (3) pinned Mesa paths enabled a genuine OpenGL ES 3.2 Mesa llvmpipe EGL context and initialized SceneFX shaders, but `wlr_allocator_autocreate` failed when `/dev/udmabuf` returned **Permission denied** on the standard GitHub runner. The complete compositor already compiled and linked.

This PR does not change SwayPX's allocation, graphics, keyboard or clipboard rules. It introduces a GitHub-only wrapper `tests/stage_director/ci_udmabuf_smoke.sh` and calls it from the existing `.github/workflows/stage-director-headless-runtime.yml`:

- Refuse to run unless `GITHUB_ACTIONS=true`, `CI=true`, `RUNNER_TEMP` is present, `/dev/udmabuf` is a root-owned character device, and the built compositor binary is executable.
- First attempt opening the device as the unprivileged CI user. If that already succeeds, make **no permission change**.
- If access is denied, require `getfacl`/`setfacl`, no preexisting explicit ACL for that numeric user, and passwordless, CI-scoped `sudo`. Save the full original device ACL/mask to a private temporary runner file before any mutation.
- Give **only the CI user** an explicit `rw` ACL on `/dev/udmabuf`, verify the user can open the device, run the existing strictly bounded headless smoke using **the unprivileged user**, and restore the exact original device ACL/mode on every exit path.
- Never use `chmod 666`, run the compositor as root, install a system service, launch any browser, map any NEWS layer, or write to the operator's Fedora system. Device access is not elevated outside the disposable CI runner.

If the device or ACL operation is unsupported, or a cgroup/security policy still denies its open, the test must **fail closed** with a specific BLOCKED reason. The device is not replaced or mocked to force success.

## HOW TO USE

Inspect the new draft PR and its **Stage Director isolated headless SwayPX runtime smoke** GitHub Actions check. No local commands or desktop restart. The wrapper is expressly not a user-side helper and refuses to run outside GitHub Actions.

## VERIFY

A passing check must establish all of: complete Nix-locked SwayPX build, real SceneFX software renderer, kernel allocator, a usable headless Wayland display socket, one still-running compositor, bounded shutdown, and original `udmabuf` ACL restoration. Failure is not a pass. There is no browser or live viewport clipping in this test; runtime containment remains unverified.

A correct device ACL is *not* guaranteed to allow `udmabuf` IOCTL operations. The runner may lack necessary kernel capabilities and `setfacl` support. We must use actual job logs to distinguish those cases.

## WATCH OUT FOR

- The wrapper changes only a device ACL **inside an ephemeral GitHub-hosted runner**, not in Fedora. Its original ACL is snapshotted and restored, including the effective ACL mask. If restoration fails, the workflow fails visibly.
- The use of `sudo` is scoped to `setfacl` on one fixed root-owned character device. No `sudo sway`, process privilege escalation, broad ACL, or symlinked path.
- This does **not** prove browser/process ownership, popup, pointer, touch, keyboard, focus or graphics clipping. Stage Director's actual viewer entry points remain deliberately disconnected.
- The standard GitHub runner might reject creating or accessing UDMA buffers despite the device being openable. If so, stop rather than weakening compositor allocator safety.
- The development branch chain is still separate from SwayPX `master`; do not interpret a branch merge as a live deployment.

## CHECKLIST

- [x] Verify PR #16 is merged into its Stage Director development branch and inspect current GitHub state
- [x] Retain exact pinned Nix Mesa/SceneFX software renderer contract
- [x] Add narrow, reversible udmabuf device ACL wrapper for CI only
- [x] Fail closed on unsupported device/permissions; no browser or compositor action authority
- [ ] Actual CI complete compositor startup, socket and shutdown accepted
- [ ] Isolated actor paint/input/popup tests with active compositor containment
- [ ] Authenticated WATCH session and user-approved merge/deployment

## NEXT

Run the complete isolated smoke and inspect original logs. If allocator initialization still fails, record the precise kernel error and stop; another permission workaround should not be attempted without evidence. If it succeeds, the next milestone is a nested actor with verifiable paint, pointer, popup and teardown behavior, not production NEWS playback.

## DECISIONS

Test kernel device access only on a disposable GitHub runner and restore the exact original ACL. Do not alter SwayPX's production buffer allocator, use an elevated compositor, merge into `master`, or change any live system.
