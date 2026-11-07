# Stage Director // Social W native seatop unmap, replacement and revocation witness

**STATE // CI-only runtime proof, draft, not installed or production-authenticated.**  
**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`. Consumer host/widget: `kudokudo1/The-Post-Apollo-Project`.  
**ROOM //** Social W unified external app move/resize and revocable guest lifecycle. External Hospital-compatible session only; live Hospital Room and explicit Staff Role not established.  
**DEPENDENCY //** Previous PR #103: 206 coherent output commits and actual rendered pixel evidence, stacked on #102/#101/#99.  
**OWNER BOUNDARY //** Stage Director Company/Weather/Chrome currently occupy shared renderer/seatop paths. This PR changes **only tests and documentation**, never production compositor C.

## IMPLEMENTATION

This is an intentional attempt to invalidate the current test-only native pair while it is actively in use, not a fresh policy-only geometry example. Existing CI leverages a private ID/PID lease file to let test-owned SwayPX seatops move a synthetic chassis and guest together. Its native `handle_unref` callback clears the test seatop's peer pointer when the guest is destroyed. Until now the runtime verified wrong-PID denial *at drag start*, but not peer destruction *during* an operation.

The extra runtime module adds two lifecycle scenarios against the already-proven temporary SwayPX build:

1. **Active move:** dock a valid pair, begin real native floating move, move both, terminate the guest process **before releasing the move seatop**, and wait for the guest's XDG container to disappear. Require exactly one native peer-unref revocation. Send another cursor motion and prove the original chassis continues moving independently. Launch a new guest with the **same app ID**, different native container ID and PID, and put it at a sentinel location. While the old seatop remains active, a further pointer motion must leave the replacement untouched. Start another seatop with the old lease and prove the replacement still does not follow. Only update to the new exact ID/PID, dock it, and begin a fresh native seatop to regain paired movement.
2. **Active resize:** begin real native floating resize with that newly bound guest; verify both client geometry ACKs. Destroy the guest while native resize is still active, require the resize peer-unref callback, then resize the chassis again and verify unrelated actor isolation. Spawn a *second* replacement guest using the same app ID but new native identity. It can resize with the chassis only after a fresh exact lease. Confirm actual rendered pixels and a successful output-frame trace naming the new guest ID.

The existing suite remains mandatory: native two-way move and paced resize, 8 in-flight pixel scenes, concurrent screenshot streams, frame-commit geometry/scene trace, unrelated third window, and temporary `/dev/udmabuf` original ACL restoration.

## VERIFY

The **existing** GitHub Actions `Stage Director Social W native seatop move overlay (disposable CI)` workflow is triggered by the new test file. The build still instruments exactly four C files **in an ephemeral source copy**, and checks the repository checkout is clean. The new module must print positive live-unmap move, resize and replacement-rebind milestones; missing `handle_unref` evidence, stale identity reuse, cross-window motion, failed real client configure ACK, missing output-commit proof or stale replacement following all fail CI.

Evidence is from a true headless SwayPX compositor and genuine CI-created Wayland windows. This still does **not** validate a production-secure authority source, because the existing lease is an owner-writable test file. It neither uses the actual Quickshell layer-shell chassis nor the actual Vesktop client.

## WATCH OUT FOR

An app ID is **not** a secure identity and can be reused/spoofed. A PID alone can be reused; the temporary test only validates an exact observed container ID and PID while both windows are live. **Production pairing must originate from authenticated native Wayland client custody and a compositor-generated, revocable generation/epoch lease**, not this test-only file or untrusted IPC criteria. Real production must also revoke on unmap, destroyed Wayland peer, compositor output/workspace handoff, changed popup/input boundary, seat ownership change, provider shutdown and client-replacement attempts. The browser worker has not been assigned these core compositor files.

A running Sway client may temporarily display an old buffer during resize configure/ACK; tests of successful settled resizes do not prove every physical presentation frame is gap-free. This experiment specifically covers pointer peer lifetime, not user acceptance.

## NEXT

If exact-head CI passes, preserve the move/resize active-unmap proof as a production-design acceptance test. Handoff to the Stage Director compositor owner to map compositor-generated lease identity to the existing native seatop and output render pathways, with Wayland-credential source attestation, nonce/epoch changes and no silent same-app replacement. Keep the existing Social IPC fast-follow bridge as rollback.

## DECISIONS

No shared compositor edits, live workstation writes, Quickshell restart, production activation, PR merge, or acceptance claim. This is a bounded, reversible CI-only continuation of the Social W Stage Director work.
