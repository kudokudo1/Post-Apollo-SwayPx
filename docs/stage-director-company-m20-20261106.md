# Stage Director M20 // Company native host and XDG map observations

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre containment
STAFF ROLE // External Nurse 2 continuation, operator delegated when original worker stalled
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE, NOT LIVE REGISTERED
ASSIGNMENT // Read-only native Wayland host/actor observation, process generation and public-source boundary
BASE // SwayPX M19 branch head 6535f459a8152af2356a438719080656cdb93b02
DEPENDENCY // SwayPX draft PR #74, Company M18 merged PR #1365 and Company M19b merged PR #1369
PHASE // Stacked isolated GitHub development, no local deployment

## IMPLEMENTATION

SwayPX master retains Weather Station's explicit WATCH physical-input pairing and its Weather-only QML module. The separate Company M19 PR #74 adds a fail-closed geometry preflight but has no native observation issuer. M20 builds a **native, read-only observer**, not a compositor-owned lease or clip.

- `stage_director_company_native_observe_host()` checks the mapped Quickshell layer role and surface from the real compositor, the dedicated `post-apollo-company-network` discovery namespace, overlay/output bounds, exact same `wl_client` for role and wl_surface, and stable Linux PID/UID/start-time read twice from /proc. It returns actual global layer/output coordinates, exact native references, and `candidate=true` only on success. It never returns `owner_verified`, `can_clip` or input authority.
- `stage_director_company_native_observe_actor()` checks a mapped, non-destroying **native XDG toplevel**, not Xwayland, popup, sibling surface or guessed window geometry; obtains real root wl_surface and native Wayland peer process evidence, with current compositor scene-node root bounds. It does not claim the actor is the Company's map until a separate authenticated pairing exists.
- `stage_director_company_native_peer_probe()` examines real server `wl_resource` objects; role and host surface must have the same client, while the map actor root must have another client. Distinct clients are *necessary but insufficient* for authorization.
- `stage_director.c` invokes the Company layer observer only on material layer changes, and the actor observer only as an explicitly non-authoritative XDG diagnostic for the known Qt application-name hint. SwayPX logs candidate status but does not store an owner, pair, move, crop or suppress any surface. The actor hint is not used by M19 preflight or as a security principal.
- `sway/meson.build` includes both new modules; tests and CI cover real Wayland-resource client separation, the source authorization boundary, M19 regressions and a full ordinary pinned-Nix SwayPX build.

## HOW TO USE

No UI or local commands for this source milestone. The installed M15/M17 Company map remains a separate Qt window at Company → Coverage → VECTOR VIEW. M20 activates no Stage Director menu feature. After a separately approved SwayPX install later, its debug log can show `[stage-director][COMPANY_M20][LAYER]` and `[stage-director][COMPANY_M20][XDG]` candidate classifications. They are not proof of embedded rendering, even if a log says `candidate=yes`.

## VERIFY

- Syntagma AGENTS and Hospital authority, reporting, and continuity rules checked.
- SwayPX master and M19 PR #74 inspected; Company M19b #1369 confirmed merged.
- Actual `sway_layer_surface`, `sway_view`, `wlr_xdg_toplevel`, `wl_resource_get_client`, `wl_client_get_credentials`, `output_get_box`, scene node observation, and existing `stage_director_read_linux_process` implementation inspected.
- CI: test exact PR head native peer matrix and full SwayPX production compilation. No claim of successful live Wayland host/actor pairing or actual window clipping.
- Cross-repo Company M18 reports `map_in_frame` geometry, but a trusted `frame_in_host` and fresh compositor epoch/revision still require a separate Company-specific geometry protocol.

## WATCH OUT FOR

- Any Wayland client can use the namespace string or guessed Qt app ID. The observer only classifies candidates. Never pair automatically by namespace, app ID, title, pathname or PID. M19's `compositor_selected` and actor-process evidence are **not** set by this module.
- The native Qt map's root window may not cover Company's panel viewport while it is floating separately; geometry-only containment must fail closed until a separate authorized compositor sizing/scene solution exists.
- The root-scene rectangle is logical surface geometry, not proof of buffer-pixel visibility, clipped popups, pointer hit testing, keyboard focus or correct transform on mixed-scale displays.
- No user-local copy, Git reset, Quickshell reload, SwayPX restart or new host credentials under this assignment.
- All Stage Director Weather and historical Chrome drafts remain owned by their existing rooms; avoid bulk merges.

## CHECKLIST

- [x] Read Syntagma and recheck M19 / Company M19b current status
- [x] Build M20 on the exact M19 draft head, not master
- [x] Observe native layer-shell and XDG actor without granting ownership
- [x] Require matching actual host wl_client resources, distinct actor client
- [x] Reuse stable kernel process identity and native output geometry
- [x] Add passive map hooks with no scene mutation
- [x] Native unit/CI/full compositor checks written
- [ ] All exact-head CI passing and source review
- [ ] Stacked PR reviewed, no merge without required approval
- [ ] M21 authenticated native Company selection and geometry receipt
- [ ] M22 real scene/size, popup, focus, pointer/keyboard, cleanup and pixel tests
- [ ] Live operator acceptance and safe rollback

## NEXT

Once M20 passes CI, hand its read-only native snapshots to a separately reviewed SwayPX **Company-only physical pairing + revocable epoch/host revision** controller. That controller must verify the *actual* native actor process descendant of the M15 launcher, not just a matching app name. Only then can Company M18's actual frame-local rectangle be submitted through an authenticated same-QPA-client protocol and checked by M19. Pixel clipping and input remain an independent later milestone.

## DECISIONS

Bounded cross-repository continuation permitted by operator. Draft PR stacked on #74; no merge or production activation. No source changes to Company M19b or Weather WATCH. This development is compatible with Hospital reporting but no live Hospital session or owner transfer is claimed.
