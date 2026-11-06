# 🏥 Stage Director M24 // Physical Company native host selection

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre native containment
STAFF ROLE // Nurse 2 (external, operator-authorized implementation owner)
PROVIDER // OPEN AI // CHATGPT
CONNECTION // HOSPITAL-COMPATIBLE, NOT LIVE REGISTERED
ASSIGNMENT // Authorize the Company-only private Wayland geometry interface through exact compositor-side physical hit-testing
BASE // SwayPX master 296c3fd9d9da584ba366a70cd2bd874b17d59355 (M23 merged)
PHASE // Isolated GitHub source milestone; not installed or enabled

## IMPLEMENTATION

M23 added the Company-only `post_apollo_company_vector_v1` Wayland interface: it is unpublished on ordinary SwayPX startup. M24 adds the first carefully scoped native selection issuer for that interface.

- A *different* physical-input ceremony: when SwayPX is **explicitly launched** with `POST_APOLLO_STAGE_DIRECTOR_COMPANY_PAIRING=1`, Ctrl+Super+Shift+F11 on a libinput keyboard arms a one-shot 3-second selection. A libinput left click then selects the exact compositor scene-hit painted Company layer. No public IPC, Wayland request, QML property or automatic namespace scan can arm this.
- Weather retains its own existing Ctrl+Super+Shift+F12 protocol. Company has independent pairing state, source and opt-in environment switch. Shared pointer delivery now evaluates both ceremonies so a swallowed press/release cannot stick when both were armed; each still hit-tests its own distinct native namespace.
- `stage_director_company_pairing_select_at()` uses actual `node_at_coords`, mapped root wl_surface and exact native layer-shell role. It validates the expected Company namespace `post-apollo-company-network` as a **candidate filter only**, overlay layer, role->data native sway_layer_surface and fresh M20-observed peer/client/process identity before publishing M23's private global.
- The geometry callback for every M23 private request reobserves the *same exact* native host, client, process generation and output. Native host movement and resizing advance a revision; identity loss or output transfer refuses further reports. Session lock rejects geometry requests.
- On Company host unmap/destroy, immediately revoke the private Wayland interface and selected client. On host commit, revalidate and revoke on loss.
- No M21 actor candidate session is recorded. No map process launch, native viewer custody, scene crop, window resizing, pointer, keyboard, focus or popup authority is conferred. M24 enables only private Company host **candidate** geometry reporting.

**Important:** Linux uinput can emulate libinput physical devices; this is an explicit operating-system input gesture but not a cryptographic hardware-presence guarantee against a compromised local UID. The user must separately authorize eventual live SwayPX installation/activation; M24 does not modify that desktop.

## HOW TO USE

This milestone is GitHub-only. No user-local action now. After a separately authorized, tested SwayPX installation and activation, an operator may explicitly enable `POST_APOLLO_STAGE_DIRECTOR_COMPANY_PAIRING=1` at SwayPX launch. On the running desktop, Ctrl+Super+Shift+F11 followed within 3 seconds by a left click on a **painted visible Company layer area** offers only a native pairing candidate and makes Company M23 protocol discoverable to that one Wayland client. Clicking a transparent hole or another layer must fail.

The existing Company → Coverage → VECTOR VIEW still opens a separate native Qt MapLibre window. No embedded Company map or user-facing completion is claimed.

## VERIFY

- Syntagma AGENTS / Hospital authority and working protocols checked.
- SwayPX Weather physical keyboard+pointer pairing code and M23 native private-protocol filter examined before changes; no Weather pairing source changed.
- Dedicated source tests verify independent F11/Weather F12, exact libinput, scene hit, native Wayland role/client/process/output, protocol publication only on hit, revocation on map change/unmap/destroy, and absence of scene/input privileges.
- Full pinned-Nix SwayPX production compilation required. Native libinput hardware gesture and QPA Company binding are not verified by static tests alone.

## WATCH OUT FOR

- The M24 gesture can publish private Company wire but this **does not authenticate any native Qt MapLibre XDG actor**. M22 process parentage and M21 candidate session must be joined to the *exact Company intent* before compositor can bind an actor.
- `stage_director_company_vector_protocol_publish()` remains a native C function; it is invoked solely after physical compositor-side selection, not arbitrary QML/swaymsg.
- The current Company QML contains an M18 map rectangle in companyFrame coordinates; it does NOT yet have the same-QPA-client `PostApollo.CompanyVector` reporter plugin. The wire will be unused by Company until a later isolated client integration.
- Company keyboard focus, input masks, existing Weather WATCH, Chrome experiment branches and original native map launch must remain unchanged.
- No local file operations, Quickshell reload, SwayPX restart or environment modification was performed.

## CHECKLIST

- [x] M23 merged and exact master reconciled
- [x] Native Company-only opt-in physical input pairing with F11
- [x] Exact compositor scene-hit Company layer/client, overlay and process check
- [x] M23 private global publication only after selected native layer
- [x] Fresh native geometry callback and monotonic revision
- [x] Host unmap/destroy/commit revocation
- [x] Weather F12 gesture remains independent
- [x] Security source and full compositor CI authored
- [ ] Exact-head CI verified and reviewed
- [ ] M24 merge after successful exact-head checks
- [ ] M25 same-QPA Qt QML Company client bridge and geometry report
- [ ] M26 trusted explicit launch-intent→Qt process birth custody
- [ ] M27 pixel, crop, resize, input, keyboard, focus and popup containment
- [ ] Operator final visual acceptance

## NEXT

Once CI confirms M24, merge only under the operator's delegated Company integration authority. Then build a Company-specific **Qt/QML bridge** under `PostApollo.CompanyVector` that borrows `QNativeInterface::QWaylandApplication` from the existing Quickshell process and reports exactly M18's active map rectangle and actual companyFrame-in-PanelWindow rectangle. The bridge must be disabled unless imported/explicitly enabled, handle compositor epochs/revocation, and never connect a separate Wayland socket or auto-launch the Qt map.

## DECISIONS

No changes to Weather WATCH native source, no unsandboxed scene authority, no merge without CI, no user-local activation. Role continuity and handoff are external Hospital-compatible only.
