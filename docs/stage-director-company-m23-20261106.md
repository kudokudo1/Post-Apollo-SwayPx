# 🏥 Stage Director M23 // Private Company vector intent and geometry protocol

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre containment
STAFF ROLE // Nurse 2 (external, operator-authorized Company implementation owner)
PROVIDER // OPEN AI // CHATGPT
CONNECTION // HOSPITAL-COMPATIBLE, NOT LIVE REGISTERED
ASSIGNMENT // Company-specific private Wayland ABI for launch-intent and active map viewport candidate
BASE // master f33b0e7996c057c1968559a375c8f81562e1e5ed
PHASE // Source milestone; exact-head CI and merge gates apply
DEPENDENCY // Company M18/M19b merged, SwayPX M19-M22 merged

## IMPLEMENTATION

M23 adds `post_apollo_company_vector_v1` as a new, separate native Wayland interface. It is **not** Weather's WATCH API and carries no URL, arbitrary PID, token secret or map imagery. The compositor's `server.c` initializes the Company service and explicitly filters registry visibility by its selected wl_client, then calls finish on shutdown. Critically, **no production call site publishes the global**. Startup remains inert and existing Company M15/M17 map still opens separately.

A future compositor-owned physical Company host selection can call `stage_director_company_vector_protocol_publish(role,surface,probe,context)` with the *actual* selected native layer-role and host wl_surface Wayland resources, and a callback that re-verifies the live native Company host/geometry. The publish path rejects missing, split-client, reused or stale native resources; attaches two resource-destruction revocation listeners; initializes a monotonic selection epoch; and registers a dynamically advertised global only after native host checks. The global-filter code matches the generated interface, including during the synchronous advertisement window.

The private server protocol checks the same exact selected wl_client on **every bind and request**; one selected client binds at most one resource. It exposes:
- `launch_intent(mode)` with fixed offline/online values. It records candidate intent only. It does not start the Qt process and does not prove separate online consent.
- `report_viewport(frame_x/y/w/h, map_x/y/w/h)`. Frame rectangle is relative to the selected native layer, map rectangle is relative to the Company frame. A compositor callback must freshly validate that host's dimensions and revision for each request. A pure geometry policy refuses invalid, negative, out-of-bounds, overflowing or nonzero partial-hidden rectangles, while accepting exact all-zero octuple as hidden. This is a candidate rectangle, not a wl_surface or a scene crop.
- `close()` clears intent/candidate, acknowledges closed and revokes global. The protocol-resource destructor frees the bound record, and lost native host role/surface also revokes.

The response contains a compositor selection epoch and fresh host revision for future client repair. It never grants actor custody, launch, rendering, clipping, focus, keyboard, input, popups, or permission to access Company records.

M23 deliberately has no production native selection issuer, no Company-side QML plugin, no M21 candidate-session record call, and no SceneFX actuation. These are follow-on milestones; the protocol cannot be reached by ordinary applications yet.

## HOW TO USE

No user-local steps or new Company buttons. The installed Company → Coverage → VECTOR VIEW stays as-is. The public source is reviewed as [M23 PR]. The protocol is deliberately unavailable on a live compositor until a separate trusted native selection mechanism is approved, implemented, and tested.

## VERIFY

- Current main, Syntagma AGENTS/Hospital/Authority rules, Weather WATCH native implementation and Quickshell fixed execv launch path inspected.
- Exact same Wayland server client role/surface requirement and fail-closed native host callback are checked before publication; global filter and each request independently validate the selected client.
- Real Wayland server test uses `wl_display`, separate clients and `wl_resource` objects to reject foreign host/actor, check revocation on native host resource destroy, recovery and exact-host geometry. A source test verifies the compositor has NO publisher call site.
- CI compiles actual scanner-generated Company protocol and native C code with strict warnings, then builds full ordinary SwayPX. These tests do NOT prove live Qt QPA client binding or actor ownership, pixel cropping and input.

## WATCH OUT FOR

- The `publish` function takes native pointers and a compositor callback, not data from QML, IPC or a public untrusted endpoint. A future producer must prove **physical Company selection** and recheck actual layer role, client, map actor launch provenance and output on every callback. Namespace alone remains spoofable.
- The protocol's `launch_intent` is a visible request candidate, not proof of the child process's birth. M22 parentage is also only evidence; an additional trusted launch observation is needed before any compositor-selected M19 custody.
- The M23 wire does not yet pass geometry into the M21 native candidate ledger; never claim a working embedded map from the presence of this protocol.
- A generated wl_resource is not interchangeable with Company mapCanvas or independent wl_surface.
- No local host commands, package installation, Quickshell reload, SwayPX restart or Chrome/Weather worktree changes occurred.

## CHECKLIST

- [x] M19–M22 source and live M15/M17 baseline reconciled
- [x] Separate Company-only XML interface with fixed offline/online enum
- [x] Native global display-filter/bind/request gate; default unpublished
- [x] Bounded active map rectangle and hidden revocation
- [x] Native host resource lifetime callbacks
- [x] Strict CI and full SwayPX build workflow written
- [ ] Verify exact-head test and production SwayPX compilation
- [ ] Review and authorized merge if green
- [ ] M24 trusted native Company physical selection issuer
- [ ] M25 same-QPA-client Qt QML geometry bridge and M21 ledger receipt
- [ ] M26 compositor actor scene crop, size/input/popup tests and rollback
- [ ] Operator final desktop acceptance

## NEXT

After M23 CI, begin **M24** to implement explicit compositor-owned physical Company layer selection. Only on a verified selected real Company wl_client should the private global be advertised. The Qt QML client adapter must bind via `QNativeInterface::QWaylandApplication`, never create its own Wayland socket. A subsequent approved click/launch provenance ledger should link M23's intent to the exact M15 Qt actor process before scene/input privileges.

## DECISIONS

Company subsystem GitHub implementation and verified merges authorized by operator. No claim of live Hospital ownership, no silent local writes, no production activation. Preserve Weather WATCH and historical Chrome work.
