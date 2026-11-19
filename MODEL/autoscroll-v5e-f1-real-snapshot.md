# V5E-F1 // Real Compositor Owner Epoch Snapshot + Portable V5C Guard (HARD OFF)

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Mouse & Keyboard / Universal Autoscroll
STAFF ROLE // External Hospital-compatible DOC; live Hospital registration not verified
ASSIGNMENT // Bounded bridge from owner-issued device, seat, mapped target and output lifetime epochs to an isolated guard
BASE // V5E-D3C #156, head 610997493c005be0ebe574c0773265355f78739a
AUTHORITY // Operator said continue; prior approval allows distinct, documented shared-source changes while Company Map #126 is paused. No local workstation mutations, merge or production actuation authorized.

## Current GitHub topology

As verified at F1 preflight, PRs #146, #149 and #151-#156 were marked MERGED into their respective PR base branches, but this is NOT equivalent to the whole nested stack reaching master. master had advanced to fb064cf3 while D3C had diverged (ahead 82, behind 27 at first compare). This child is deliberately stacked on D3C, not silently merged into master. Actual rebase/conflict review is required before any mainline integration.

## IMPLEMENTATION

V5C epoch header/source are ported from the current master version, with a corrected source comment because they are now compiled, not live-active. Existing portable witness implementation is also compiled.

The new source-owned collector is compiled into actual SwayPX and:
1. Accepts only a currently live compositor cursor and underlying pointer device handle. Its parameters are **not accepted from Wayland clients or IPC**.
2. Finds the seat and input-device wrapper by pointer equality in the input manager's live lists BEFORE dereferencing them.
3. Verifies that the pointer actually belongs to the identified live seat, refusing virtual pointers and devices detached from that seat.
4. Refuses session lock and active pointer constraint.
5. Uses the actual compositor node_at_coords hit-test and requires a mapped N_CONTAINER surface.
6. Uses current wlroots output layout at pointer coordinates and checks the returned backend output is in root's enabled output list.
7. Conservatively refuses a target node whose compositor owning output is not the pointer's output.
8. Reads the four owners' private lifetime accessors and returns **only** if every token is nonzero; otherwise completely zeros the result.

This snapshot is an owner-epoch collection, **not** an authentication credential or a new active-scrolling capability. The V5C guard is linked for future private invocation but has no live input caller. A separate explicit and independently verified positive witness producer is still missing. Never construct one from IPC/client flags or plain libinput/udev classification.

## Test evidence

Dedicated exact-head workflow: strict native source diff, hard-OFF gate checks, owner-membership source ordering tests, four-domain C11 lifetime issuer plus V5C guard cancellation fixtures, and the actual Nix-built SwayPX binary.

The C fixture uses deliberately *synthetic positive witnesses*. For each of device, seat, target, and output, it proves that revoked tokens cancel the active guard and reissued tokens cannot revive an earlier session. This is a stronger direct producer-to-consumer compatibility test, NOT a native device-removal, window-unmap, seat-destroy or monitor-move event trace. Tests never access real input devices or activate scrolling.

## Company Map ownership split

The F1 child adds private epoch/snapshot C headers and sources plus tests/docs/workflows, and changes sway/meson.build to link those private modules. Among paused Company Map #126 changed files, **Meson is the only overlap in F1**. The child does NOT edit cursor.c, seat.c/seat.h, Stage Director Company input/scene, popup, transaction, xdg_shell, output configuration or any autoscroll live runtime. Prior B and D2 ownership collisions remain their own separately documented changes.

The returning Company Map owner must inspect Meson source additions before rebasing PR #126, and must not treat a passing CI matrix as final operator acceptance.

## HOW TO USE

GitHub source/CI review only. Do not reload SwayPX or switch on a mouse setting. This is not a user-facing autoscroll feature yet.

## VERIFY

[Pending exact-head matrix.] Native build proves the collector and epoch guard compile together. It does not prove native callback event receipts. Review source-level guards and the positive-witness limitations separately.

## WATCH OUT FOR

- The V5C guard does not currently know a nonrepeating **compositor-session identity across restarts**; issuer tokens reset per new process. Do not grant cross-session input rights based only on these numbers.
- The owner snapshot has no current proof of physical vs synthetic USB HID; libinput/udev paths are only candidates. It is not a live witness.
- A node hit-test and output-list match are not complete Stage Director scene containment, keyboard focus, or operator permission evidence.
- No one has verified every asynchronous layout/scene mutation source. D3C covers successful output-config commits, not all possible changes.
- Production remains PA_AUTOSCROLL_LIVE_ACTIVATION 0; physical_verified=false, native_scroll_lock_active=true, and no live guard, timer or active axis dispatch.

## CHECKLIST

- [x] Reinspect current master and nested PR merge topology, Company #126 and canonical Hospital protocol
- [x] Port V5C private portable guard into D3C child
- [x] Compile real compositor owner-only four-epoch snapshot without changing cursor/seat/runtime
- [x] Add four-domain cancel-on-revoke and cancel-on-rebind synthetic guard tests
- [x] Add hard-OFF source and actual SwayPX binary workflow
- [ ] Exact-head workflows green and independent security review
- [ ] Company PR #126 handoff with exact Meson overlap
- [ ] Trusted compositor-session identity and authenticated physical source issuer
- [ ] Real native lifecycle source event traces with fresh observation samples at each tick
- [ ] Explicitly gated disposable compositor axis/frame+ESC+indicator tests
- [ ] Operator merge/activation acceptance

## NEXT

After exact-head CI, report the tested result and notify Company Map's paused owner in #126. The next milestone must create genuine compositor-origin positive evidence with explicit session authority, then join V5C guard only in a disposable CI host. Do NOT turn on production scrolling based on this F1 bridge.

## DECISIONS

DRAFT · UNMERGED · PRODUCTION HARD OFF · NO LOCAL HOST MUTATIONS · MAP OWNER UNCHANGED.
