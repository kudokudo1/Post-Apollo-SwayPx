# 🏥💜 Stage Director // M30e original MapLibre source provenance

PATIENT // kudokudo1/Post-Apollo-SwayPx
ROOM // Stage Director / Company MapLibre integration
STAFF ROLE // Nurse 2, operator-assigned external / Hospital-compatible
PROVIDER // OPEN AI // CHATGPT
CONNECTION // EXTERNAL; no verified live Hospital registration
ASSIGNMENT // M30e reconstruct the actual native Company Qt/MapLibre source and verify its M13/M15/M29 acceptance contract against SwayPX
BASE // M30d PR #88, commit 9e31b58d7b985bd652a7905c4d185785fd0f390d
DEPENDENCIES // M30a #85, M30b #86, M30c #87, M30d #88 open stacked drafts
SOURCE PIN // The-Post-Apollo-Project commit e3595b0035bb3d89aa87c6ba2bd1aac04a975181
PHASE // Source provenance and fail-closed dependency audit, not runtime admission

## IMPLEMENTATION

Discovery corrected: the actual Company MapLibre application source **is present** in public `kudokudo1/The-Post-Apollo-Project` at `experiments/company-vector-qt/`. The previous M30d report's blanket "source unavailable" was too broad. The **owner-local built ELF and its Qt 6.11.2 MapLibre Native Qt 3.0 patched library install** are unavailable to this external GitHub session, which is a different gap.

The real cross-repository chain is:

1. `services/company/CompanyVectorSidecarController.qml` starts a tracked Quickshell `Process` and exposes its actual `processId`.
2. `services/company/CompanyVectorSidecarLaunch.py` uses `os.execv` into the installed owner-only **Python host bridge** at `~/.local/libexec/post-apollo/company-vector-probe`.
3. `experiments/company-vector-qt/CompanyVectorHostBridge.py` checks owner/Qt6/QtLocation/MapLibre file path, manifest and dependency closure and `os.execve`s the final **Qt ELF** at `~/postapollo-vector-lab/company-vector-build/company-vector-probe` without changing PID.
4. M29's kernel inode comparison intentionally points at that **final** lab ELF, not at the installed Python bridge. Earlier impression that the two file paths conflict was erroneous; no change to M29's trust boundary is proposed.
5. `experiments/company-vector-qt/CMakeLists.txt`, `main.cpp`, `main.qml`, and `style-fixture.json` really reference `QMapLibre 3.0`, `QMapLibre::Location`, `QtLocation`, `QtPositioning`, `MapLibre 3.0`, MapView, Qt OpenGL RHI and the offline synthetic GeoJSON fixture.
6. The Company M28c optional QPA bridge stays default-off and can send a kernel pidfd only after selected private intent. Source review by itself never satisfies M24 physical operator selection or exact M15 binary provenance.

New `tests/stage_director/ci_company_maplibre_source_contract.py` reads only known source files, refuses source symlinks/oversize, rejects an unexpected Company commit or dirty checkout, verifies M13/M15/M29 path coherence and checks the Qt MapLibre, offline/online and consent semantics. `test_ci_company_maplibre_source_contract.py` mutates disposable copies to ensure fail-closed behavior on wrong MapLibre target, changed QML plugin, moved OpenGL setup, network injected into offline style, bad path, detached process, missing native consent and unapproved ELF pin.

The M30e GitHub workflow checks out **the exact original Company commit** into a separate checkout, runs the new cross-repository gate, and reruns the original Company's M5/M14/M15 Python tests and M28c Node PID-bridge tests.

## HOW TO USE

Nothing to install or run locally. Inspect the M30e PR and exact-head GitHub CI. The user-facing Company VECTOR VIEW remains unchanged. Offline synthetic mode remains default. ONLINE STREETS still requires deliberate consent and may disclose viewed area and IP to the map tile provider.

## VERIFY

- Exact Company source commit is checked in CI, not an empty generic QWidget substitute.
- Read-only source syntax contracts and refusal tests cover MapLibre 3.0 CMake/QML imports, Qt OpenGL RHI, synthetic local GeoJSON, live map sources requiring explicit online mode.
- The installed M13 Python bridge path and final M15 Qt ELF path are different BY DESIGN; exact PID is preserved by `execv/execve` and M29 checks the correct final ELF.
- Original M5/M14/M15 and M28c tests run against the pinned source checkout.
- Prior M30d real QtWayland XDG/pidfd CI 3/3 green at `9e31b58d` is retained independently.
- **Not yet proven:** recreating MapLibre Native Qt 3.0 plus the required Fedora 44 Qt 6.11.2 patch, executing the actual Company Qt app under SwayPX, pixel capture of real vector layers, same-QPA M24/M28b receipt with real physical Company selection, binary/Qt plugin attestation, focus/input/crop/popup isolation.

## WATCH OUT FOR

- The real binary is a Fedora Toolbox-built, owner-local build with a QtLocation runtime bundle and MapLibre C++/QML plugins. An ordinary GitHub Nix Qt QPA build (M30d) does not prove ABI compatibility or map pixels.
- **Do not invent** the M14 local GeoJSON patch, 3.0 native Qt dependency hashes or owner's actual ELF digest. The operator's prior proof of building/running the library is not a reproducible CI bundle.
- A GitHub checkout pin proves only source continuity. Same-UID local artifact substitution, loaded plugins and process memory are out of scope.
- No local install, desktop modifications, production bridge activation, unrelated Weather/Chrome changes, automatic tile requests or Company client data access.
- New mainline updates to Company must be reviewed and then deliberately repinned before M30e source-contract status is carried forward.

## CHECKLIST

- [x] Verify M30a–M30d PR dependency chain and current SwayPX master
- [x] Recover original Company M5/M14–M17 source, offline style, M13/M15 launcher and M28c bridge
- [x] Correct the false M13/M29 path-mismatch diagnosis
- [x] Add exact-commit cross-repository source gate and adversarial refusal tests
- [x] Reuse original Company tests in GitHub CI
- [ ] Exact M30e PR-head CI pass
- [ ] Reproduce Fedora 44 Qt6.11.2 / MapLibre Native Qt 3.0 source patch set in a disposable build
- [ ] Run the REAL `company-vector-probe` with a selected Quickshell Company layer and successful M28b Wayland fd receipt
- [ ] Compare process/Qt plugin hashes against independently approved provenance
- [ ] Contained map pixel, popup, keyboard and pointer acceptance
- [ ] Operator merge authorization and final acceptance

## NEXT

First pass M30e CI on its exact commit. Then arrange a CI-reproducible native MapLibre v3 dependency build using the actual M14 GeoJSON/Qt6.11.2 patch set, rather than assuming unpatched upstream or prebuilt Qt6.5 artifacts match Fedora. Perform an isolated **real map** SwayPX runtime test only after that dependency is trustworthy. Do not enable actor/scene/seat rights or mutate the user's local host.

## DECISIONS

External Nurse 2 continues only the Company Stage Director MapLibre integration contract. PRs remain stacked and draft; source verification is **not** final renderer acceptance, and M30a's compiled digest manifest stays empty. Neither repository's default branch is changed.
