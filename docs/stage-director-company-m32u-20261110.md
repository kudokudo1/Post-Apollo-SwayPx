# 🏥 Company M32u // Actual source lineage and offline ELF/Qt discovery audit

PATIENT // `kudokudo1/Post-Apollo-SwayPx`
DEPENDENCY PATIENT // `kudokudo1/The-Post-Apollo-Project`
ROOM // Company → COVERAGE → VECTOR VIEW, native MapLibre inside Company
INCOMING ROLE // Hospital-compatible external DOC by current Syntagma default, unless an authenticated Hospital Assignment specifies otherwise; previous Nurse 2 report author is not proof of role assignment
PROVIDER // OPEN AI // CHATGPT
CONNECTION // External; live Hospital registration not verified
ASSIGNMENT // Audit executable's declared dynamic ELF dependencies and Qt plugin candidates, fail closed before authorizing an executable
BASE // M32t source on branch `stage-director/company-m32t-pre-spawn-custody-20261110`; upstream PR #135 was merged **into an intermediate stacked branch, not master**
PRODUCTION // OFF, read-only source/CI diagnostics

## IMPLEMENTATION // Real Company source evidence

On `The-Post-Apollo-Project` current default branch, `experiments/company-vector-qt/CMakeLists.txt` builds the independent native `company-vector-probe` executable. Its explicit link inputs are `Qt6::Quick`, `Qt6::Location`, `Qt6::Positioning`, `QMapLibre::Location`; `qmaplibre_location_setup_plugins(company-vector-probe)` also configures Qt plugin behavior. The app includes a QML module `CompanyVectorProbe` and `main.qml`.

`services/company/CompanyVectorSidecarController.qml` currently starts a `/usr/bin/python3` child whose `services/company/CompanyVectorSidecarLaunch.py` calls `os.execv` on the owner-local `~/.local/libexec/post-apollo/company-vector-probe`. This is a **separate Company-controlled live path**, NOT compositor-owned launch custody.

Company M17's `experiments/company-vector-qt/HANDOFF_M17.md` records an additional independent QtLocation QML bundle of five files: `libQt6Location.so.6`, `libdeclarative_locationplugin.so`, `MapView.qml`, `plugins.qmltypes` and `qmldir`. Thus, M32s's hypothetical **flat `bin/lib/plugins` three-ELF fixture** does **not** capture all real QML resource files or runtime modules. This is a verified source mismatch requiring a future reviewed packaging contract, not something to hide behind a synthetic green CI result.

Added `tools/company/m32u_elf_qt_audit.py`, a *read-only offline* analyzer that:
1. Requires a strict test/review JSON manifest with one fixed `bin/company-vector-probe`, at least one `lib/` ELF and one `plugins/` ELF, at most 24 artifacts and SHA-256 for each.
2. Rejects unsafe names, duplicate SONAME filenames, unexpected/unlisted files, symlinks, writable group/world paths, wrong owner/mode, altered inodes/digests, bad ELF class/type, mixed machine/class, nonmatching SONAME and arbitrary loader RPATH/RUNPATH.
3. Invokes `readelf -W -h -l -d` as a *data parser*, **never `ldd`** or the inspected binary. Lists direct ELF `DT_NEEDED` for each declared artifact; flags any needed library absent from the explicit `lib/` set, including unknown host/system libraries.
4. Even when every static listed ELF dependency resolves, reports only `LISTED_ELF_NEEDED_CANDIDATE_ONLY`. All flags for dynamic Qt plugin discovery, system loader, immutable image, operator consent, production launch and scene/input authority remain **false**. There is no compositor production call or manifest pin.

## HOW TO USE

No Fedora change. Synthetic CI builds three inert ELF fixture objects, checks the static graph and rejects undeclared shared objects, malicious paths, missing plugins, mismatched digests and runpath injection.

For **future separately authorized read-only use on a trusted exported build** (do not treat a supplied manifest as approval):

```bash
python3 tools/company/m32u_elf_qt_audit.py --bundle /reviewed/immutable/bundle --manifest /reviewed/manifest.json
```

The illustrative bundle consists only of `bin/`, `lib/`, `plugins/` flat entries. The real QtLocation `qml/QtLocation` module and additional QML assets are **not** represented and must be modeled/reviewed before any trusted launch. The tool does not install, import, run, copy or approve a binary.

## VERIFY

CI tests compile *disposable synthetic ELF files* and validate real `readelf` graph traversal; see `tests/stage_director/test_company_m32u_static_elf_qt.py`. The workflow also retains M32t/M32s/M32r C and Python tests and both normal/private SwayPX builds. Latest exact-head CI must be checked independently.

## WATCH OUT FOR

- No native MapLibre artifact bytes, authoritative SHA-256s or complete Fedora/Toolbox runtime dependency closure have been supplied to this worker. Source-level CMake declarations are not `DT_NEEDED` proof for a particular compiled binary.
- Qt `dlopen`, QML imports/qmldir, graphics driver and installed loader may load files not listed in ELF `DT_NEEDED`. Tool reports these as **unverified even after a candidate pass**. Runtime manifest from someone else's development runner cannot be silently trusted for Fedora.
- M32u currently *audits* a strict hypothetical flat bundle with ELF files only; it does not replace M17's live QtLocation QML staging, M32s inventory, M32t descriptor candidate or Company launcher.
- The current compiled production digest is still intentionally blank, M24 F11 host pairing does not authenticate the native map actor, and production embedded scene remains disabled.
- GitHub marks #132–#135 merged through branch stacking. #135's source was **not on `master`** on preflight. Do not call a stacked PR merge a production landing or independently merge/activate those branches.

## CHECKLIST

- [x] Recover current source-owned executable, Qt/QMapLibre CMake links and QML import/staging contract
- [x] Add strict static ELF DT_NEEDED reader; no binary execution, no source trust escalation
- [x] Distinguish listed-file static graph from runtime Qt plugin and system loader validation
- [ ] Verify exact-head M32u CI
- [ ] Operator-reviewed actual *built ELF/QML bundle bytes*, complete DT_NEEDED, QML modules, plugin and system-loader provenance
- [ ] Trusted immutable deployment contract with fail-closed full dependency and runtime loader verification
- [ ] Distinct trusted operator input, compositor-controlled launch/pidfd and revocable embedded scene lease
- [ ] Authorized branch-to-master integration, Fedora hardware acceptance and final operator acceptance

## NEXT

Obtain or generate an actual **read-only exported artifact inventory** from the separately owned Company Qt build (without installing it on Fedora). Reconcile `DT_NEEDED` and QML plugin paths against known host/Toolbox Qt versions, classify system dependencies and define a complete approved packaging schema *before* authorizing process birth or pinning an installed executable digest. Coordinate Company repo ownership before touching its build/launcher.

## DECISIONS

No Company QML/source edits, no new OS or launcher install, no production scene/spawn/authorization, no PR merge, no trusted hash selection, no physical desktop mutation. Operator final authority and Hospital assignment boundaries retained.
