# 🏥💜 Company M32d // Native map input, popup and focus containment

HOSPITAL REPORT

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Company native MapLibre in-widget embedding
STAFF ROLE // Nurse 2 · external Hospital-compatible successor
CONNECTION // EXTERNAL; live Hospital registration unverified
ASSIGNMENT // Attach actual normal-SwayPX pointer, keyboard and popup interception to the native map scene without granting unapproved production admission
BASE // master at d40ea1fa04ed0d55741257fe530de4d69e4c63cd (M32c merged)
PHASE // M32d source implementation and CI
STATE // DRAFT, NOT YET VERIFIED/ACCEPTED
SCOPE // SwayPX Company-only scene input with small shared cursor/seat/XDG hook changes. No Company QML, Social scene ownership transfer, merge, or Fedora write.

## IMPLEMENTATION

**Operator target:** Company → COVERAGE → VECTOR VIEW displays a real interactive native MapLibre map **inside Company's existing map canvas**, not another Qt window. M32a and M32b merged compositor-owned Company geometry and XDG process pairing. M32c merged guarded wlroots scene operations, but production actuation is still OFF.

M32d implements real compositor interaction containment and lifecycle hooks:

1. New production Company input policy: active embedded actor root is checked against the compositor-owned viewport using exact half-open global logical bounds, preserving fractional and negative-monitor coordinates. Reject nonfinite/outside/locked/stale actor events; leave unrelated client input unchanged.
2. Native scene backend exports current compositor-observed actor root and view identity from its currently active private CI scene. Production has no active scene and normal input remains unchanged. On revoke, active input state clears.
3. Normal cursor event handlers enforce the policy for motion, press/release and wheel following real scene hit testing, before ordinary event dispatch.
4. Normal seat entrypoints reject keyboard focus on the embedded actor through both workspace focus and direct/subsurface focus. No live keyboard broker exists.
5. Normal XDG popup creation rejects native MapLibre popups at both root and nested paths while the actor is embedded. XDG root size changes revoke the scene before normal SwayPX can redraw an uncropped native buffer.
6. Company selected-host geometry or output revision revokes the scene immediately. Native source or host changes require a fresh authorized lease, never automatic restoration.
7. Normal Meson compositor includes the new input policy. A standalone C test checks root identity, stale epochs, fractional boundaries, NaN/infinity, locked sessions, negative-coordinate displays, popups, keyboard and unrelated-client passthrough.

No production actor/scene lease issuer is added, the executable digest is deliberately blank, and the M32c scene setter remains compiled OFF in ordinary SwayPX. **This is implementation of protective hooks, NOT completion of the visible map embedding.**

## HOW TO USE

Review the M32d draft PR and its CI. Do not install as finished in-widget VECTOR VIEW. The existing button still opens the separate Qt window. Standalone viewer behavior is NOT accepted as the requested result.

## VERIFY

Must pass exact-head compiled production input policy tests, normal SwayPX build, source-hook audit and existing private Wayland/Social/input regressions. Real native Company/MapLibre M32c scene placement, pointer event movement and pixel capture in isolated Wayland are STILL required. An ordinary Nix build cannot prove the private placement branch executed.

## WATCH OUT FOR

- Approved MapLibre executable SHA-256 manifest is still empty. No signed operator scene permission exists. Production scene/input activation remains OFF.
- Popups created BEFORE activation are not refused by a new-popup hook. The eventual lease issuer must reject pre-existing popup subtrees before showing native pixels.
- Active embedded MapLibre keyboard focus is denied until a proper in-panel focus/keyboard broker is reviewed. Pointer pan and wheel remain candidates for confined input.
- A revoked native XDG actor may remain hidden and must be terminated or safely restored by a separately approved lifecycle owner.
- Physical Fedora, dual outputs, UI button mode change, SceneFX adoption and visual acceptance remain outstanding.
- No live Hospital registration or final operator acceptance is claimed.

## CHECKLIST

- [x] M32c merged and initial 3/3 CI PASS, exact current master verified
- [x] Actual cursor motion/press/scroll containment source and hook wiring
- [x] Actual seat focus and both XDG popup interception hooks
- [x] Host revision and native source resize revocation
- [x] Strict policy negative test and full-build CI written
- [ ] Exact-head M32d CI success
- [ ] Real isolated Company/MapLibre execution of M32c scene setter
- [ ] Pre-existing popup quiescence and authenticated short-lived scene lease issuer
- [ ] In-widget Company VECTOR VIEW control and operator physical visual acceptance

## NEXT

Finish exact-head CI and compiler fixes, then run isolated real native MapLibre scene placement and input against the Company panel. Integrate independently reviewed popup/input/lease authority before admitting a production scene, and only then connect the button.

## DECISIONS

Draft/unmerged, no local machine modifications, no production rendering/input, no operator executable pin, no merge or final DONE. Recovery branch: stage-director/company-m32d-input-popup-focus-containment-20261109.
