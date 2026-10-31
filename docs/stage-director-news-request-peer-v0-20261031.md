# SD1 // Native WATCH request peer evidence

STATE // Isolated implementation; not trusted ingress; no production authorization.
PATIENT // kudokudo1/Post-Apollo-SwayPx
NEIGHBOR // kudokudo1/The-Post-Apollo-Project (Weather Station; untouched).
BASE // stage-director/news-watch-auth-origin-rfc-20261031 @ 9675925330c8bdee6daa6b0381e9c829bb78f5c0
HOSPITAL // External-compatible. No live Room/Session/ownership registration asserted.

## IMPLEMENTATION

The read-only native probe stage_director_news_request_peer_probe compares three real compositor-owned Wayland resources: host layer role, host wl_surface, and WATCH request resource. It rejects missing/aliased resources, mismatched host role/surface wl_client, and foreign request wl_client. Exact native match returns CANDIDATE_ONLY, with every authority bit false.

The new source is deliberately excluded from normal sway/meson.build; no production caller, protocol global, or IPC exists. No operator trust root, source ID, session lease, browser spawn, scene painting, input, focus, popup, or teardown authority has been added. This is a prerequisite shared by either trust architecture proposed in PR #37, not a decision between them.

## HOW TO USE

Review the new draft PR and the Stage Director WATCH native request peer (non-authorizing) GitHub workflow. Weather NEWS remains in safe mode. No user desktop action is available yet.

## VERIFY

The isolated CI test uses actual libwayland-server resources and two different wl_client connections created in one OS process, proving that shared process/UID does not make the Wayland connections identical. Positive native matching grants no authority. Negative cases include a foreign request, mismatched layer role/surface, nulls, and aliased resources. CI also prohibits any production Sway source caller. GitHub CI success must be confirmed after PR opening.

## WATCH OUT FOR

A same-client result is necessary but insufficient. The future private protocol handler must independently verify custom resource interface, a truly operator-selected native Weather host, current mapped layer/output/generation, source allowlist, session epoch, and installed rendering/input/popup guards. The fake Weather layer namespace is not a credential. Never expose native pointers through Sway IPC.

## CHECKLIST

- [x] Inspect latest Stage Director stacked PRs and verify PR #37 base
- [x] Add candidate-only native Wayland resource comparison
- [x] Add real two-connection same-process unit test
- [x] Add standalone CI and no-production-caller guard
- [ ] Confirm GitHub Actions on draft
- [ ] Operator decides trusted compositor pairing or managed Quickshell launch
- [ ] Prove actual protocol resource on the same client as the Weather layer
- [ ] Coordinate separate Weather QML bridge and source/viewport contract
- [ ] Integrate session lease, browser custody and independent scene/input/popup/teardown guards
- [ ] Operator-controlled live acceptance

## NEXT

Once CI passes, the next authorization gate is the PR #37 trust root: Option A compositor-controlled pairing or Option B managed launcher. The next separate experimental PR should verify real protocol resource provenance with two spoofable layer hosts, but must still not spawn a browser or activate rendering.

## DECISIONS

No trust-bootstrap architecture adopted. No branch merge, local desktop change, Weather Station source change, or production activation. The new isolated primitive is only a zero-authority evidence test.
