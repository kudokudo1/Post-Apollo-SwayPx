# 🏥 Company Map // M32v real-layout QML package inventory checkpoint

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
DEPENDENCY PATIENT // `kudokudo1/The-Post-Apollo-Project` (Company Qt owner)
ROOM // Company → COVERAGE → VECTOR VIEW native MapLibre
STAFF ROLE // External Hospital-compatible DOC by Syntagma default; live Hospital session/assignment unverified
ASSIGNMENT // Source-aligned QtLocation + MapLibre QML packaging audit, no new runtime authority
BASE // M32u PR #136 open draft, verified head `5b71d61126209e5db495a0970a3159bd7b2ea205`
STATE // Diagnostic, synthetic tests, production OFF

## IMPLEMENTATION

M32u mapped direct ELF `DT_NEEDED` only. Current source in the **separately owned** Company Patient confirms native `company-vector-probe` imports `QtLocation`, `QtPositioning` and `MapLibre` from `experiments/company-vector-qt/main.qml`. Its `CompanyVectorHostBridge.py` names two MapLibre ELF libraries, the maplibre geoservices plugin and `qml/MapLibre/qmldir`, while M17's `CompanyQtLocationBundleM16.py` lists `libQt6Location.so.6`, `libdeclarative_locationplugin.so`, `MapView.qml`, `plugins.qmltypes`, `qmldir`. They occupy **different existing owner-local trees** and must not be reclassified as an already trusted distribution.

Added `tools/company/m32v_qml_bundle_audit.py` as a **read-only exported-package test**. It accepts a newly proposed, explicitly manifested synthetic/review layout containing `bin/`, `lib64/`, `qml/QtLocation/`, `qml/MapLibre/`, `plugins/geoservices/`. This is a **review schema, not a deployment instruction**. Requires the fixed executable, QtLocation shared library, both MapLibre ELF libraries, geoservices ELF plugin, QtLocation's four files, MapLibre's `qmldir` and its declared QML binary plugin. Includes bounded optional hashed files for other declared plugins/assets (max 64).

The verifier:
- Opens directories and files relative to already-open descriptor roots using `O_NOFOLLOW`; rejects unlisted trees, symlink swaps, changed metadata, group/world-write, wrong ownership, hardlinks, bad names/hashes and invalid ELF magic on candidate binaries.
- Parses UTF-8 `qmldir` module and plugin declarations for both QtLocation and MapLibre, checks exact QtLocation declarative plugin and qmltypes references, and requires the maplibre QML plugin named by the actual descriptor to be present and individually hashed.
- Does **not** import or load Qt, invoke the MapLibre executable or use `ldd`. A file can match while QtPositioning, system libraries, dynamic Qt platform plugins, the graphical driver and loader remain entirely unverified.
- Always sets all privilege bits FALSE: no executable approval, process launch, operator authentication, scene/input/focus/popup, immutable root, complete dependency proof or live production authority. There is no compositor runtime caller, no Company UI edit or new Wayland protocol.

The pre-existing Company native workflow `.github/workflows/company-network-vector-probe-m5.yml` currently uploads screenshots and diagnostic logs, **not** a downloadable ELF/QML native bundle. Hence no honest real build-byte audit can be claimed for M32v. The fixture is compiled exclusively in disposable CI.

## HOW TO USE

No changes to Fedora or Company menus. An owner may eventually provide an **approved read-only exported candidate bundle** already containing the files above and a review JSON manifest:

```json
{"schema":"post-apollo.company-maplibre-m32v-qml-inventory.v1",
 "files":[{"path":"bin/company-vector-probe","sha256":"<64 lowercase hex>"}]}
```

The sample illustrates structure only. A real input requires a matching nonzero hash for **every single file** in the proposed package and all mandatory modules. No automatic collection, install, staging or launch occurs. With a reviewed export, audit through:

```bash
python3 tools/company/m32v_qml_bundle_audit.py \
  --bundle /path/to/reviewed/export \
  --manifest /path/to/reviewed/manifest.json
```

A `QML_LISTED_FILES_CANDIDATE_ONLY` result is not permission to use that package.

## VERIFY

CI tests create disposable, compiler-generated synthetic ELF objects and UTF-8 QML descriptors. Tests cover full declared fake fixture, missing Qt asset, unknown MapLibre plugin, unexpected siblings, traversal, manifest/hashes, altered qmldir, symlink plugin and writable files. Inherited M32u/M32t checks and ordinary/private compositor builds remain applicable. Verify exact-head GitHub run after PR creation.

## WATCH OUT FOR

M32v **does not possess the real Company's native compiled binary or staged runtime files**, nor does it verify their RPM signature, QtPositioning, host Qt ABI, actual `dlopen` execution, process-origin permission or physical user gesture. It is a structured intermediate contract for later artifact review, and the current owner-local M17 stage + QProcess fallback is unaffected.

GitHub M32t and prior stacked PRs were marked merged to development branches without landing on SwayPX `master` at the previous preflight. Do not merge or enable any production branch without explicit operator authorization and owner coordination.

## CHECKLIST

- [x] Recover exact named Company MapLibre ELF/QML and geoservices assets from source
- [x] Add bounded fail-closed regular-file manifest and qmldir module/plugin consistency audit
- [x] Preserve no runtime proof/permission and no Company owner changes
- [ ] M32v exact-head synthetic + compositor CI
- [ ] Obtain real native Company build and QtLocation/MapLibre export from its owner
- [ ] Audit actual ELF `DT_NEEDED`, QML import closure, QtPositioning and platform/graphics plugins against matching Fedora artifacts
- [ ] Approved immutable install bundle + provenance, trusted operator gesture and compositor-controlled process/pidfd/scene
- [ ] Authorized branch integration and real desktop acceptance

## NEXT

Coordinate with Company Qt build owner to generate a reviewed **read-only native build artifact export** and full installed runtime inventory (including QML non-ELF files). Only after byte-level and runtime provenance review may an operator-approved separate launch custodian be considered. No live host changes at this checkpoint.

## DECISIONS

Keep M32v draft, no merge, no host writes or restart, no CI-hash pin, no real launch, no physical gesture, no production scene, no final acceptance.
