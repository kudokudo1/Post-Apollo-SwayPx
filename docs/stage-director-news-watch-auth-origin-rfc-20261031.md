# SD1 // Authenticated Weather NEWS WATCH origin, proposed native handoff

**STATE // RFC, non-authorizing, not an adopted architecture**  
**PATIENT //** `kudokudo1/Post-Apollo-SwayPx` (native compositor contract)  
**NEIGHBOR PATIENT //** `kudokudo1/The-Post-Apollo-Project` (Weather UI)  
**ROOM //** Weather NEWS / Stage Director SD1 host origin admission  
**ASSIGNMENT //** propose the smallest operator-reviewed trust bootstrap and one-client WATCH ingress that can connect native host observation to the inert NEWS lease, while keeping actual browser, render, input, focus and popup actuation separately blocked  
**PROVIDER //** OPEN AI // CHATGPT; external Hospital-compatible report; **no live Hospital registration**  
**CHECKPOINT //** GitHub source inspected 2026-10-31, draft PR #36 HEAD `e08d1cda8d3f9c3720fbed76024d99e59b3eba00`, fully passing two independent headless layer-shell runtime checks  
**AUTHORITY //** operator chooses architecture and assigns impacted owners before executable changes; this RFC makes no such decision.

## Mechanism // What actually must be proven

A Wayland **socket is a connection to a server process**. A particular `wl_client *` identifies that connection inside SwayPX. A layer-shell namespace, title, app-id, root PID, UID, `/proc` ancestry and `pidfd` describe a window/process, but do not show that *the operator selected this connection as Weather Station*. They are insufficient to authorize a WATCH click. Exactly this distinction was verified with two simultaneously mapped, independent clients using `post-apollo-weather-station` on [PR #36](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/36): both had valid native identity and neither was authorized.

The desired proof is a **chain of separate assertions**:

1. **Operator designation**: SwayPX receives an explicit, controlled user choice of one actual mapped Weather layer/surface, not a public IPC claim or “the only window with that namespace.”
2. **Native connection binding**: in compositor memory SwayPX pins the exact `wl_client *`, `sway_layer_surface *`, `wlr_surface *`, output and generation of that designation. Revalidate they are currently related and mapped before every admission.
3. **Request provenance**: a WATCH/CLOSE request must arrive via a private native Wayland protocol resource whose `wl_resource_get_client()` is that **same paired `wl_client`**. The payload carries a registered catalog identifier and operation, never an arbitrary URL, `PID`, pointer, claimed UID or “trusted=true.”
4. **Revocable ingress record**: only an accepted request can ask the separate broker owner to reserve an epoch in `stage_director_news_session_lease.c` (currently intentionally **uncalled**). A host unmap, client disconnect, output loss, CLOSE, changed generation or duplicate request cancels it. This still does **not** permit launch or render.
5. **Broker, browser and guards**: a separately authorized broker subsequently launches/owns one browser and proves the actual browser `wl_client`/XDG root-surface under the same epoch, while independently installed scene, input, focus and popup guards authorize each effect. These are NOT achieved by admission alone.

**Security scope**: protects against other ordinary Wayland clients or Sway IPC senders impersonating the Weather namespace or guessing a PID. A fully compromised operator account, compositor process, a process allowed to ptrace the actual Quickshell memory, or unrestricted synthetic input can defeat ordinary same-UID desktop isolation. Do not claim cryptographic proof against that stronger attacker without an OS trust boundary (e.g., separate principal/privileged launcher and restricted input). This RFC makes no universal same-UID compromise guarantee.

## Source hooks found, not new inventions

| Source | Existing mechanism | Proposed use / boundary |
| --- | --- | --- |
| `sway/desktop/layer_shell.c` | `stage_director_layer_mapped/committed/unmapped/destroyed` already fires | Track exact Weather layer lifetime; no second observer or polling daemon |
| `sway/stage_director_weather_host_adapter.c` | Native layer role and `wl_surface` `wl_client` agreement, real Linux peer PID/starttime | Candidate evidence; not the trusted pairing issuer |
| `sway/stage_director.c` | Read-only shadow registry; PR #36 corrected delayed map→host candidate transition | Observe candidates without implicitly choosing among duplicate Weather namespaces |
| `sway/server.c` | `wl_display_set_global_filter`, `filter_global` and `wlr_security_context_manager_v1_lookup_client` | Potential place to withhold proposed custom control global until the pair exists, then **also enforce on bind/request**; current sandbox-vs-unsandboxed filter is not Weather authentication |
| `sway/input/keyboard.c` / command registry | Existing Sway binding handling | Candidate for a *compositor-owned* operator pairing action, if its input provenance can be constrained; do not expose generic `swaymsg` pairing authority |
| `widgets/weather/WeatherStationW.qml` (neighbor Patient) | Quickshell `WlrLayershell.namespace`, layer, OnDemand keyboard, local `newsViewerViewportRect` | Existing host + UI; namespace and local geometry remain hints |
| `widgets/weather/StationNewsHub.qml` (neighbor Patient) | `chooseItem()` sets `chosenRecord` and `viewerOpen`; `closeViewer()` clears it and defaults mute | Future WATCH/CLOSE producer once a native Wayland request bridge exists; do not mutate before joint ownership |
| `widgets/weather/StationChannelScreen.qml` (neighbor Patient) | Existing static fail-closed viewer, preflight `can_launch=false` | Keep safe-mode fallback and never import QtWebEngine |
| `sway/stage_director_news_session_lease.c` | One candidate source/host/client/output/epoch, process lineage/actor refs and teardown, all authority flags FALSE | Consume only after real trusted admission, never from arbitrary `swaymsg` or plain QML boolean |

## Option A // Compositor-controlled pairing (recommended candidate)

**Why proposed:** preserves the operator's current independent Quickshell lifecycle. No takeover of `shell.qml`, systemd services or desktop startup merely to identify the host.

**Proposed non-public pairing ceremony**:

1. Operator opens the actual Weather Station. SwayPX enters an explicitly selected, **compositor-owned** one-shot pairing mode via an approved native input path. Design must account for virtual keyboard/pointer injection, Sway IPC command spoofing and focus.
2. A compositor-native selection identifies the exact live layer-shell `wlr_surface` under the designated seat/output, not the first/only namespace match. If pointer hit testing on the translucent Weather panel cannot prove the selected surface, **refuse pairing** and instead require another trusted compositor-owned selection interface. No auto pairing when `hosts=1`.
3. SwayPX compares the selected layer's actual role and underlying surface `wl_client`, current output and map generation, stores only **in-process** native refs, and registers `PAIRED` for that client. A second same-namespace client remains unpaired. Pairing expires on destroy/disconnect/unmap/output invalidation/compositor restart, and cannot be replayed with serialized pointers.
4. A new deliberately narrow Wayland extension (proposed name `post_apollo_weather_watch_v1`; actual protocol name/version subject to owning-team review) is advertised **only** to that paired connection via compositor global filtering, and its bind and request handlers **independently require the same pinned client**. Hiding a global alone is not an authorization check. The client sends `watch(source_id)`, `close()`, optionally `mute()`; input payloads never contain native refs, URLs, raw commands or a user-declared trusted bit.
5. The compositor verifies host/client/output generation again; recognized `source_id` exists in the approved shared catalog; reject stale/duplicate requests, foreign Wayland clients, wrong output/host surface and revoked pairing. It records `WATCH_ADMITTED_CANDIDATE_ONLY`, with every `can_*` false, and returns a request/result state. **No browser launch at this milestone.**
6. The Weather owner later adds a small native QtWayland/Quickshell-compatible bridge (not a spawned `swaymsg` helper). `chooseItem` and `closeViewer` call through the bridge **on the same Wayland connection as the Weather layer**. If Quickshell/Qt routes the protocol over a different connection, acceptance must fail and the architecture be revisited. No assumption that QML plugin connection sharing works without a real test.

**Technical caveats:** compositor input pairing is not automatically secure against virtual-input injection; a native command reachable from ordinary Sway IPC does not prove operator consent. The input/security worker must define a trustworthy operator selection and test it. If this cannot be established, **Option A must not be promoted**.

## Option B // Trusted compositor/launcher-controlled Quickshell start

A launcher explicitly owned and authorized by the operator creates Quickshell's Wayland connection via a dedicated inherited FD/`WAYLAND_SOCKET` or equivalent protocol-matched startup contract. The compositor records the exact accepted client and associated layer refs. That can avoid an interactive pairing ceremony but changes **who starts and restarts Quickshell**. It intersects the existing desktop lifecycle, shell owner, and local systemd startup, so it is **not** an acceptable silent substitute. Enforce no unrelated client can use the private connection; a copied socket *name* or ordinary same-UID credential is insufficient. Restart/recovery provenance and dependency cycles require separate review.

**Decision pending:** approve A, approve B, or request a revised trust root. No auto fallback from failed pairing to namespace or PID guessing.

## Proposed admission states (no code authorized by this RFC)

```text
UNPAIRED
  operator-controlled pairing, real native layer/client/output validated
    -> PAIRED
       explicit native watch(sourceId), request wl_client == paired client
         -> WATCH_ADMITTED_CANDIDATE_ONLY
            separate broker authorization/provenance/guard verification
              -> (future) BROKER_OWNED
       close, unmap, disconnect, output change, pairing loss
         -> REVOKED -> UNPAIRED after new operator designation

Any failed pairing provenance, duplicate ambiguous layer, stale generation,
foreign wl_client, unknown source, or uncertain teardown
  -> DENY (no state elevation; no browser, scene or seat action)
```

`PAIRED` identifies an **operator-selected Wayland connection**, not safe browser rendering. `WATCH_ADMITTED_CANDIDATE_ONLY` identifies intent only; not a completed broker launch or clip. These separate gates may not be collapsed.

## VERIFY // Required attack and behavior evidence

1. Reuse [PR #36](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/36) CI: two independently mapped matching layer-shell clients and one foreign namespace client coexist, both native identities correct, and all remain unauthorized before pairing.
2. One deliberate compositor-owned pairing selects only one of the two live same-namespace clients, even if their creation order is reversed. A third process cannot gain WATCH rights by creating another matching layer or changing `app_id`.
3. The unpaired client must receive no accepted WATCH even with a guessed source key, identical UID, same namespace, Sway IPC, fake PID/starttime, or malicious `wl_resource` object IDs. Test non-binding global discovery *and* direct request/bind authorization.
4. No plain `swaymsg`, public socket, activation token, `launcher_ctx`, generic runtime file or subprocess preflight can trigger a pairing; if physical input provenance cannot be verified, admission remains **blocked**.
5. The paired `wl_client` can submit only known source IDs over the same Wayland connection, with exact host/output generation. Unmapped/recreated Weather layers, client reconnect, output loss, compositor restart and explicit CLOSE immediately revoke; old requests and epochs cannot be replayed.
6. Unknown catalog source, user-provided URL, malformed UTF-8/resource type, duplicate WATCH or out-of-order CLOSE must fail closed. Pointer/ref values must never cross public IPC.
7. No production code sets `can_launch`, `can_clip`, `can_render`, `can_bind_viewer`, `can_accept_input` or `can_show_popup` true at the host-admission stage. Ordinary SwayPX controls, XDG actors and 320×240 CI renderer regressions continue passing.
8. **Cross-repo, operator-authorized proof** that the native Qt/Quickshell bridge attaches to the *same* `wl_client` owning the Weather layer. If not, do not try to forge client identity; reopen the architecture choice.
9. Only after admission acceptance, separately authorize the real browser process broker, popup/focus/input containment, per-frame crop+destination and audio/teardown tests. LIVE deployment/merges remain operator-gated.

## Work ownership and phased PR boundaries

**A0 // operator design decision and owner alignment:** select pairing versus managed launch; identify Weather UI, native compositor, input/security, and process broker owning Rooms. No live Hospital registration is implied by this document.

**A1 // SwayPX owner, isolated draft:** implement a *disabled-by-default* native pairing capability and custom protocol access gate that binds an exact `wl_client`, plus real two-client malicious case; no WATCH launch or renderer interaction. Do not modify ordinary existing Sway IPC authorization or generic keyboard binding permissions without owner review.

**A2 // Weather UI owner, separate draft:** implement the minimal protocol bridge, unchanged public WATCH layout, source ID and CLOSE wiring, safe-mode fallback, no QtWebEngine import. Verify same Wayland connection.

**A3 // shared integration room after A1/A2 acceptance:** wire authentic admission to the existing inert lease, and broker custodianship; still no live scene or input. Cross-workstream authority is required.

**A4 // renderer + input owners:** install native clip/destination and focus/pointer/popup guards with real browser test and rollback. Only then consider launching a permitted NEWS source in an isolated approved environment.

## WATCH OUT FOR

- Same-UID kernel process credentials, `/proc` ancestry, pidfd and unsandboxed SwayPX `filter_global()` are **not WATCH origin authentication**.
- There is no existing QML API in Weather's source that is proven to send a request on the same underlying native Wayland connection. A client-side bridge needs actual Qt/Quickshell implementation/verification, not an assumed plugin hook.
- The compositor's `filter_global` currently gates privileged protocols by wlroots security-context presence. A new Weather control global must not be treated as secure just because it is visible only to unsandboxed clients. It must bind to an independently paired or launch-proven exact client, and enforce on every request.
- Pairing by pointer location may be ambiguous under transparent/input-pass-through layer-shell surfaces, other overlays or headless setups. Refuse ambiguous matches rather than silently pick a namespace.
- A malicious same-UID process with direct arbitrary input injection or ptrace of the real client may require privilege separation. State the supported adversary model and test it, rather than claiming perfect authentication.
- Never persist raw Wayland pointers, session epochs or browser profile secrets to QML, a plain shared file or an unauthenticated Unix socket.

## CHECKLIST

- [x] Verify PR #36 real duplicate layer-shell native host runtime and latest HEAD
- [x] Inspect actual `sway/server.c` Wayland global filter and security-context scope
- [x] Inspect existing layer callbacks, native host adapter and inert session lease
- [x] Inspect Weather `chooseItem`, `closeViewer`, layer-shell namespace, keyboard mode and preflight
- [x] Record operator pairing and managed-launch alternatives with clear threat/rollback boundaries
- [ ] Operator selects trust bootstrap and approves cross-owner A1/A2 assignment
- [ ] Implement and verify same-native-connection WATCH request, pairing provenance and spoof refusal
- [ ] Integrate broker/actor, guard installation, actual browser and teardown
- [ ] Operator review, controlled merge and live activation

## NEXT // Authorization gate

**Recommended proposal:** Option A, compositor-native operator pairing plus a Wayland request resource tied to the exact paired Weather client. It preserves current independent Quickshell startup. Before touching `sway/server.c`, `sway/input/keyboard.c`, Quickshell or any live launch controller, obtain operator and affected-room approval for the input trust root and protocol bridge design. The acceptance criterion is a real spoofing test where **two native clients share the Weather namespace and only the operator-selected connection can request a candidate WATCH**, with every browser/render/input permission still false.

## DECISIONS

**None adopted.** This is a proposed architecture, not a code implementation, production capability, assigned staff role, permission to mutate neighboring repositories or an approval to merge/restart/activate anything. Earlier PRs #33–#36 remain separately reviewable and their existing diagnostics cannot authenticate WATCH.
