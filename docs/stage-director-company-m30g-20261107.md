# 🏥💜 Stage Director // M30g actual Qt native source-crop prototype

HOSPITAL REPORT

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre native integration
DOCTOR // External ChatGPT · Nurse 2
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL / HOSPITAL-COMPATIBLE
ASSIGNMENT // M30g first real compositor clipping experiment, synthetic native Qt map
PHASE // Isolated headless screenshot verification
BASE // M30f PR #90, head fda5da8f63d400bcf1e10c0f4dbf8d386d4791be
STATE // DEVELOPMENT / NOT FINAL ACCEPTANCE

## IMPLEMENTATION

M30f achieved **native Qt placement only** by resizing an 840×540 Qt window to its 600×380 frame. M30g deliberately leaves the actual Qt Wayland XDG window **840×540** and paints an additional orange region in the right 240px and bottom 160px. The compositor is required to show only the top-left 600×380 **source** crop, with the rest of the 840×540 painted source invisible.

The CI-only new `stage_director_ci_company_crop.c` is built exclusively with `-Dsd-ci-company-crop-probe=true`. Its compositor hooks are conditionally compiled out of ordinary SwayPX. Native `view_map` uses CI-only fixture name plus actual Linux Wayland client credentials, PID, UID, process starttime and Nix-built executable path, never title alone as authority. No M30a digest pin, Company v2 Wayland receipt, actor custody, seat or production input is granted.

The real 840×540 Qt6 Wayland source remains unchanged in geometry while SwayPX compositor calls `wlr_scene_subsurface_tree_set_clip` to narrow it to 600×380 and independently adjusts the scene buffer destination size (a known SceneFX interaction). The source clip is reasserted after Sway transactions and after per-frame output configuration. It is explicitly revoked on XDG unmap.

The test boots a throwaway headless SwayPX and two actual Qt Wayland windows: a purple host stand-in and a cyan/orange native window. Python holds a kernel pidfd for each, floats the true oversized native window at (520,300) and then (650,350) **without resizing the source**. It captures four actual compositor PPM screenshots: frame-only, initial cropped source, moved cropped source, closed actor. It independently verifies visible inside cyan, purple outside, output black background, no leaked cyan or orange anywhere outside its proper crop and no stale pixels after close.

The source and test are intentionally synthetic. They do **not** run MapLibre Native Qt, real Company Quickshell, physical host selection, actual input events, popup handling or production window broker.

## HOW TO USE

No operator desktop changes. Review this draft's GitHub Actions logs and attached `sd-company-m30g-qt-actual-source-crop-pixels` artifact. Ordinary `Company -> Coverage -> VECTOR VIEW` behavior and optional opt-in QPA bridge remain unchanged.

## VERIFY

- `source-boundary` gate: Python parsing, Bash syntax, exact CI option and no shipment of test code into ordinary production build.
- `native-crop-actual-pixels`: ordinary SwayPX build excludes CI source; separate CI compositor build includes it; actual Qt Wayland host+actor, real Linux pidfds, real screenshot with orange overflow as adversarial witness, moved root, exit cleanup and temporary device ACL restoration.
- **No pass may be declared until exact-head CI and actual screenshots pass**. A compositor state/log or hit-test alone cannot substitute for pixel evidence.

## WATCH OUT FOR

- Deliberately tests a fixed 600×380 viewport and a stationary 840×540 painted source; no dynamic host-derived Quickshell geometry yet.
- Source clip and output buffer destination are separate renderer inputs. Sway/SceneFX may overwrite one during repaint. Failing screenshots must remain red until fixed, never silently relaxed.
- The Nix Qt wrapper may use a wrapped binary basename; CI selector explicitly accepts only narrow known `/nix/store/` Qt probe paths and the right CI environment. It does not authenticate production MapLibre.
- Source crop does not automatically prove correct pointer coordinates or rejection of clicks outside the crop. That requires a separate real input event test.
- CI test program paints synthetic geometry, not real interactive street maps. No selected Company host, native M28 Wayland fd receipt, or real Qt MapLibre binary is allowed authority by this code.

## CHECKLIST

- [x] Verify PR #90 and live master, inspect scene/layout/output hooks and concurrent Weather-owned PRs
- [x] Separate CI-only Company source-crop code from existing Weather scene probes
- [x] Keep native Qt source 840×540 and paint obvious orange overflow
- [x] Add isolated compositor Nix build, CI harness and actual screenshot pixel checks
- [ ] Exact-head GitHub CI pass and screenshot review
- [ ] Native pointer/button/wheel input bounds in actual cropped viewport
- [ ] Dynamic viewport geometry directly from Company QML and native host receipt
- [ ] Real owner-local MapLibre Qt v3 dependency build and runtime
- [ ] User Fedora native acceptance, operator merges and final review

## NEXT

Run and repair the isolated compositor screenshot gate without weakening pixel assertions. If true crop passes, advance to actual pointer/scroll event delivery and interaction containment in the SAME oversized Qt client, then a genuine Quickshell company host and original MapLibre renderer.

## DECISIONS

Operator authorized the "put it in first, build around it" development order. Changes remain a draft PR stacked on M30f. This does not extend worker ownership into another Weather Stage Director lane, live Fedora workstation, production activation or final operator acceptance. Keep M30a–M30f draft dependency chain and fail-closed actor custody unchanged.
