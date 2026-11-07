# 🏥💜 Stage Director M30k // Dynamic real Company native viewport, fail-closed fullscreen

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre compositor integration
STAFF ROLE // Nurse 2, external Hospital-compatible worker
ASSIGNMENT // M30k continuous Qt source across standard/compact, fullscreen hide, return
STATE // Draft CI verification, operator and reviewer acceptance pending
BASE // M30i including nested M30j (PR #93)
COMPANY SOURCE // e3595b0035bb3d89aa87c6ba2bd1aac04a975181
CONNECTION // External; no live Hospital registration verified
PERMISSIONS // GitHub-only isolated test development; no Company source writes, PR merges, desktop modifications or activation

## IMPLEMENTATION

M30j passed a real Quickshell Company PanelWindow with a source-cropped 840x540 Qt Wayland XDG actor in the same disposable SwayPX compositor. The original M30j test was **standard size only** and its compositor crop/position were fixed.

M30k tests the **same native XDG actor and PID** across actual QML-driven Company chassis changes. In the existing separate CI-only SwayPX crop module (disabled in ordinary production builds), the compositor selects an exact Company layer-shell resource and pinned QML-derived host geometry:

| Company state | Original PanelWindow | Original QML Canvas in panel | Native Qt source treatment |
| --- | --- | --- | --- |
| Standard | 1000x700 | 166,242,573,369 | source 840x540 clipped to 573x369 |
| Compact | 640x490 | 166,281,441,128 | SAME source clipped to 441x128 and moved with real host |
| Fullscreen | 1920x1080 | 166,203,1493,788 | 840x540 source insufficient: SCENE HIDDEN, input refused |
| Return standard | 1000x700 | 166,242,573,369 | SAME XDG/PID re-shown, correct crop/position |

The test does not assert that a small map renderer can magically fill fullscreen. Instead it proves a **fail-closed** transition while a future, resizable native renderer is pending.

The compositor validates the real mapped Company overlay, kernel process identity and known exact host dimensions. Unknown host geometry, multiple layers, invalid source, process replacement, or fullscreen without enough source cause the CI actor to be hidden. The selected viewport is not provided by an untrusted client. A private Qt scene-node reparent and crop is allowed only under the M30g test build AND explicit M30j+M30k disposable runner flags. Ordinary SwayPX contains no experimental scene code.

The test launches the unchanged reviewed Company source in a **disposable copy** with M30i's two QML geometry-correction hypotheses, a temporary frame mouse-event observer and a QML timer that actually drives chassisLevel standard -> compact -> fullscreen -> standard -> close. The Qt child is launched **once** and lives across all transitions.

The real Wayland virtual-pointer sender must show Qt press/held-drag/release/wheel **inside** each supported viewport and zero Qt input from hidden source regions. Outside clicks must be positively received by the real Company QML. Fullscreen must hide the undersized map despite its process remaining alive and must deliver input to Company. Returning to standard must show the **same Qt PID and XDG root**, with no wrong pixels. The Qt window is closed before Company closes; unchanged native candidate and bridge remain OFF.

This remains a **synthetic Qt-painted native source**, not the actual MapLibre Qt v3 renderer. M30k does not implement native Qt source resizing or actual fullscreen MapLibre.

## HOW TO USE

No local installation, restart or UI action. Review the M30k draft PR, its exact-head GitHub Actions jobs, native screenshots and pointer logs. The original Company checkout and user desktop are unchanged.

## VERIFY

A successful source check is not proof of dynamic native behavior. Require exact-head GitHub CI:
- ordinary SwayPX build excludes test-only scene hook;
- separate CI compositor builds;
- exact reviewed Company checkout + matching Qt Quickshell;
- real QML standard/compact/fullscreen/return snapshot geometries match source pins and authorized native candidate stays zero;
- same kernel pidfd/XDG title and Qt process before/after each transition;
- actual cyan pixels confined to supported visible viewports, zero orange/teal leaking outside;
- real Qt drag/wheel and positive real Company outside-click reception;
- fullscreen hides source and blocks every Qt event even while process remains alive;
- standard return restores same native source, not a replacement;
- unmap restores Company Canvas and subsequent close revokes UI geometry;
- CI temporary device access/permissions restored.

## WATCH OUT FOR

- The native Qt 840x540 source is **undersized for 1493x788 fullscreen**, so M30k deliberately does not display it there.
- M30i Company coordinate-correction hypotheses are still only in a CI copy. The Company's owner must independently implement reviewed correct geometry before shipping.
- The test-only layer identity checks do NOT constitute approved production host selection, v2 fd custody or persistent native compositor actor permissions.
- Qt's Wayland globalPosition may be window-relative. The test checks actual **root-widget local coordinates** against the known real QML viewport.
- The single-output, fixed three known chassis geometries are a deliberate controlled CI contract. Multi-monitor, scale factors, translation, drag-moving Company and arbitrary resize are not accepted.
- Source clip and pointer hit-test must be independently verified: a correct screenshot does not prove interaction containment.
- Popup layering, keyboard/focus isolation, native MapLibre tiling/pan/zoom, performance, operator desktop acceptance and final merge review remain separate gates.

## CHECKLIST

- [x] Re-check actual GitHub merged-vs-stacked state and parallel Social owner
- [x] Audit M30j source, scene stack and real Company geometry contracts
- [x] Add CI-only pinned Company standard/compact layout switching with fail-closed fullscreen
- [x] Add real Quickshell chassis timer source and same native Qt input test harness
- [x] Preserve ordinary SwayPX build boundary and original Company source
- [ ] Exact-head GitHub workflow all checks pass
- [ ] Independent review, change requests, operator merge decision
- [ ] Build genuine resizable MapLibre Qt ELF under verified provenance
- [ ] Production authentic host/v2 pidfd receipt and Company owner geometry fix
- [ ] Multi-output, popup, keyboard and real Fedora acceptance

## NEXT

Run M30k exact-head CI and fix only evidence-supported failures. After dynamic standard/compact/hide/revive passes, implement a **resizable native source** for fullscreen and move from a synthetic colored Qt witness to genuine MapLibre Qt 3.0 while preserving all current test-only security boundaries. Keep the separate Social Stage Director lane untouched.

## DECISIONS

This is an isolated, private CI-only native viewport experiment. No production input or scene permission is granted, and there were no user-local changes, Company repository writes, or PR merges. No green status may be claimed before actual runtime evidence.
