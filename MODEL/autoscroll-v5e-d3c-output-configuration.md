# V5E-D3C // Output Configuration Commit Epoch Rotation (Production HARD OFF)

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Mouse & Keyboard · Universal Autoscroll
STAFF ROLE // External Hospital-compatible DOC, no live Hospital registration verified
ASSIGNMENT // Real SwayPX output layout/configuration change invalidation
BASE // V5E-D3B PR #155, exact head 56d2e37cd36bbd549409d85f456a15f84b1073e2
PERMISSION // Continue authorized bounded draft source work without Map owner takeover, local workstation modification, merge or production activation.

## IMPLEMENTATION

SwayPX may reconfigure monitor coordinates, mode, scale, transform or color while a monitor stays enabled. D3B's enable/disable token does not detect that. D3C adds a conservative commit generation boundary.

- Actual output_autoscroll_config_committed() rotates (revokes then optionally reissues) the output's availability epoch. It refuses null, disabled, fallback, destroying, no-wlr-output, missing input-manager issuer and exhausted token source, always leaving epoch zero in failure cases.
- The real output configuration owner calls it once per configured output after wlr_backend_commit() has succeeded, after finalize_output_config() and output_update_position(), but before arrange_layers() and arrange_root().
- Test-only configuration branches and failed backend commits exit before rotation. Any committed config, including no-op reapplication or a transform/scale without geometry size change, rotates the token, intentionally erring on the side of cancelling an existing future autoscroll session.
- A configuration can reposition other outputs through automatic layout; the second pass covers all configured outputs, not only the one explicitly named.
- This is still a token producer, NOT an active scroll consumer or physical input credential. V5C epoch guard is separately unmerged; it has not been connected to these producers.

## OWNER SPLIT

D3C production changes are confined to sway/config/output.c, sway/tree/output.c and include/sway/output.h, plus tests, workflow and docs. None is touched by paused Company Map #126. Parent D3B output availability, D3A target, D2 seat and D1 device remain distinct drafts; inherited workflow allowlists are updated only on this child branch.

Company Map's cursor, seat, focus, popup, transaction, XDG, output scene and Stage Director code are untouched. Post an exact-head comment on #126 so the owner can find this work without inferring acceptance or owner reassignment.

## VERIFY

Exact-head source tests enforce that rotation follows successful native backend commit, skips test-only and failed commits, runs after position assignment and before arrangement; plus fail-closed revoked/disabled/fallback behavior and strict C11 lifetime rules. Complete SwayPX Nix build verifies compiled integration. Dynamic monitor moves/reconfiguration receipt is NOT demonstrated.

## WATCH OUT FOR

This protects SwayPX's observed output-configuration commit path, not every possible layout mutation. Compositor-native dynamic epoch evidence, other low-level layout event paths, scene/surface visibility/hit-test verification, restart identity and concurrency review remain required before treating it as trusted layout generation. The current output token combines availability/configuration generations and does not prove the output actually contains the cursor or target.

Production PA_AUTOSCROLL_LIVE_ACTIVATION remains 0. physical_verified remains false, native_scroll_lock_active remains true, layout_generation remains unverified. No live desktop behavior changes.

## HOW TO USE

GitHub source and CI review only. All drafts stay unmerged and no operator workstation files, services or input settings are changed.

## CHECKLIST

- [x] Reinspect actual backend commit, finalize and output_update_position callback order
- [x] Rotate output epoch for each successful committed config, deny test-only/failed
- [x] Zero changed-file overlap with Company Map PR #126
- [x] Add exact-head regression and full native build workflow
- [ ] All relevant checks PASS
- [ ] Durable owner handoff with all D3 heads and overlap
- [ ] Independent security/source review and operator acceptance
- [ ] Dynamic native output-configuration event receipts and final source inventory
- [ ] Physical origin, V5C trusted guard, CI-only compositor axes/ESC/indicator and actual desktop acceptance

## NEXT

Verify D3C; post the complete D3A/B/C checkpoint on Company #126. Next stage: owner-authenticated source and epoch guard integration plus real native event receipts, still no production activation or merge without explicit permission.

## DECISIONS

DRAFT · UNMERGED · PRODUCTION OFF · NO LOCAL CHANGES · MAP OWNER PRESERVED.
