# V5F-C2 // Native Virtual Device + Transient Seat Lifecycle Receipts

PATIENT // `kudokudo1/Post-Apollo-SwayPx`  
ROOM // Mouse & Keyboard · Universal Middle-Click Autoscroll  
STAFF ROLE // External Hospital-compatible DOC; no verified live Hospital registration  
BASE // V5F-C1 PR #160, `0e178585a457f995e78d1f37ee17fc4edad03f52`  
AUTHORITY // Bounded user-approved GitHub source/test work, not a merge, local host write or production activation.

## IMPLEMENTATION

Seven test, CI and documentation files only. No production C, header, Meson, input manager, seat, cursor, XDG, output, Company Map or Weather source changes. The only source modifications occur inside a private throwaway GitHub-hosted runner copy of the exact Git checkout.

The source overlay instruments **real production owner callbacks in a disposable copy**:
- `sway/input/input-manager.c` `handle_virtual_pointer` called from the genuine Wayland `zwlr_virtual_pointer_manager_v1` protocol. A real compositor-managed virtual pointer gains an owner-issued device epoch on registration. The separate trusted/eligible accessor must still return zero because `is_virtual=true`. The normal `handle_device_destroy` callback must revoke its epoch to zero before any seat device detach.
- `sway/input/seat.c` `seat_create` called by the genuine `ext_transient_seat_manager_v1` protocol. The newly created transient seat gains an epoch before publication, and `handle_seat_destroy` must revoke it to zero before focus changes or freeing members.

A test-only C Wayland client is compiled with `wayland-scanner` using **pinned protocol XML** from the same Nix flake's `wayland-protocols` and `wlroots_0_20.src` dependencies. It connects to the compositor's private Wayland socket, requests a genuine transient seat, binds the returned `wl_seat` global, creates one virtual pointer **targeted at that seat**, and holds the resources while the runner reads the native owner logs. On the runner's `stop` command, the client destroys the virtual pointer **before** the transient seat, observes Wayland round-trip completion and exits. Run two distinct cycles and reject missing callbacks, reused epoch values, cross-domain token collisions, nonzero eligibility or incomplete invalidation.

The synthetic Wayland pointer **NEVER sends any motion, button or axis events**. Its lifetime receipt is evidence of a compositor-owned *virtual* input device, not real hardware. Headless SwayPX still has `WLR_LIBINPUT_NO_DEVICES=1`, `PA_AUTOSCROLL_LIVE_ACTIVATION 0`, `physical_verified=false` and `native_scroll_lock_active=true`.

## VERIFY

Dedicated CI must:
1. Enforce exact seven-file changed-path manifest and zero production/Map changes.
2. Confirm overlay source anchors, owner bind/revoke timing and hard-OFF negative trust checks.
3. Build actual instrumented headless SwayPX and pinned Wayland protocol client.
4. Verify the transient seat and virtual pointer really exist via protocol acknowledgments, then verify the owner-minted token before destruction and zero after removal in the compositor logs.
5. Repeat in a second fresh protocol session and reject identical/colliding epochs.
6. Always restore temporary runner-only /dev/udmabuf ACL, clear temporary runtime sockets and leave the original source checkout clean.

No live compositor on the user's machine is accessed. A running headless compositor plus successful callbacks is **native lifecycle evidence**, but never proves real mouse hardware, libinput topology authenticity, native Scroll Lock state, XDG scene consent, complete output generation, or positive V5C guard authorization.

## HOW TO USE

View CI and exact-head PR checks only. The overlays, protocol client, wrapper and harness require exact `CI=true`, `GITHUB_ACTIONS=true`, temporary runner directory and explicit `PA_AUTOSCROLL_CI_NATIVE_TRACE=2`. Do not execute the ACL wrapper on the operator workstation.

## WATCH OUT FOR

Because the Wayland virtual pointer protocol is privileged, the disposable test client must be unsandboxed and use the compositor's *own* private socket. The test must not create a synthetic `struct wlr_input_device` by memory fabrication or claim one is physical. The native trusted-device accessor should deliberately return zero despite the underlying slot owning a nonzero token. This is the security invariant under test.

The output/window dynamic evidence from C1 remains valid on its separate exact head. C2 does not rerun or replace that test; both child PRs require final integration review and operator acceptance. Earlier parent PRs still contain cursor, seat and Meson conflicts with paused Company Map PR #126 even though C2 itself changes no owned production paths.

## CHECKLIST

- [x] Read active Syntagma and check exact F-C1, master and Map ownership
- [x] CI-only input manager and seat owner callback overlay
- [x] Real Wayland protocol client with pinned protocol scanner inputs
- [x] Native two-cycle build/test, negative physical trust and ACL restore workflow
- [x] Zero production C/Meson changes and zero new Company #126 overlap
- [ ] All exact-head workflows PASS, with real native callback log receipts
- [ ] Independent security review, operator PR acceptance, parent-stack/master reconciliation
- [ ] Trusted physical hardware/provenance, live event freshness and positive guard integration
- [ ] Native Scroll Lock, Esc, indicator, live desktop acceptance and approved activation

## NEXT

If exact-head CI passes, post the native owner receipt identifiers and durable Map zero-overlap notice. Then move to the separate physical-input trust stage without making a virtual pointer positive. Keep full runtime HARD OFF.

## DECISIONS

DRAFT · UNMERGED · CI-ONLY · VIRTUAL POINTER **NOT PHYSICAL** · PRODUCTION HARD OFF · NO OPERATOR DESKTOP CHANGES · COMPANY MAP OWNER PRESERVED.
