# Stage Director SD1 // Controlled NEWS browser launch plan V0

**STATE:** Development-only, no browser launch authority, no live deployment.
**PATIENT:** `kudokudo1/Post-Apollo-SwayPx`.
**BASE:** `stage-director/weather-news-pidfd-custody-v0-20261026`, following merged development PR #10; PR #8 remains stacked and unmerged to the SD0 branch.
**OPERATOR:** Final authority. No Hospital Assignment/Room transfer or local modification is implied.

## IMPLEMENTATION

The operator wants **NEWS → WATCH IN STATION → video inside the Weather Station bay → × CLOSE**, not another floating browser. QtWebEngine aborted Quickshell; a separate browser process must be supervised and compositor-contained.

The earlier SD1 components can identify a Weather Station layer candidate, classify NEWS browser actors, test crop geometry, inspect a process lineage and hold a kernel pidfd. Those components still lack an authenticated WATCH request and implemented popup/pointer/teardown containment.

This milestone introduces `include/sway/stage_director_news_launch_plan.h` and `sway/stage_director_news_launch_plan.c` as a **strict, non-executable command plan**:

- Exact 18 catalog source IDs/HTTPS URLs mirrored from Weather Station `scripts/weather/news_viewer_allowlist.json` schema 1. Reject arbitrary URLs, extra flags, shell strings, directory traversal, unknown or differently cased IDs.
- Fixed absolute candidate executables for Chromium, Brave, Chrome and Edge. These are **candidate paths** only; actual availability, file identity, signature and kernel executable inode have not been verified on the operator host.
- Fixed flags to open a dedicated NEWS app-style window with `--class=post-apollo-weather-news`, separate profile path under `/run/user/<UID>/post-apollo/weather-news/<epoch>` and muted audio. The profile flag is a **string only**: no directory is created, and no browser is executed. Later trusted code must securely create a mode-0700, owned, symlink-free profile and handle any existing files, child processes and cleanup.
- Reject missing host handle, empty/zero epoch, root UID, unavailable browser enum, unrecognized source and missing claimed gate (operator WATCH intent, native host, compositor clip, input, popup and teardown). Even with every claimed Boolean true, state is **CANDIDATE_ONLY** and `can_spawn=false`.
- `stage_director_news_plan_argv()` exposes a fixed argv array for testing and eventual trusted integration. There is no shell, URL concatenation from user input or IPC entry point; no runtime call site in SD0, SD1 clip, viewer session or pidfd custody.

This is deliberately *not* yet the process-spawning controller. Caller-provided flags are not cryptographic or compositor-authenticated proofs.

## HOW TO USE

Review the separate draft PR. Run the standalone C11 contract test or the corresponding GitHub workflow. NEWS safe mode and all live controls remain unchanged. No operator-side SwayFX reload, build, git pull or changes have been performed.

## VERIFY

CI compiles fixed-argument generation and checks all 18 IDs against static URLs, browser selections, invalid or arbitrary IDs/flags/URLs, no-launch gates and stable profile path. A static contract checks that the plan cannot call process spawn, signal, network, compositor clip, seat/focus or pidfd custody operations.

Standalone CI proves **plan generation only**, not existence or safety of browser executables, actual process supervision, compositor build, browser video playback or input containment. It cannot attest runtime Weather Station geometry or multi-output scaling.

## WATCH OUT FOR

- **Source registry drift:** This C table mirrors Weather Station schema 1. Maintain synchronized updates and compare the corresponding repository commits when adding/removing sources. No C-generated source should become a parallel unchecked URL catalog.
- **No arbitrary executable:** The four absolute binary paths are candidates, not verified host availability. Later owner must inspect the installed executable and start it by trusted kernel handle/identity, not blindly accept an executable path.
- **Flags are not authority:** A caller can set gate Booleans to true; therefore, these Booleans never authorize launch. The future compositor-owned session controller must enforce actual host/actor, clip/input/popup and rollback mechanisms *before* spawning.
- **Private profile safety:** The proposed runtime directory must be created using controlled ownership and no-follow operations; no symlink-following path and no cross-session reuse. Do not start a browser merely because an argument says `--user-data-dir`.
- **No audio yet:** `--mute-audio` prevents a false MUTE control claim until audio can be explicitly attached and controlled in the verified renderer lifecycle.
- **Browser class / app ID:** The `--class` flag may not give an exact native Wayland app ID for every Chromium-derived browser. Confirm under the isolated compositor and fail closed if class mismatches.
- **Stage Director still dev-only:** Existing PRs are merged into stacked branches rather than `master`; a local taskbar pull will not activate them.

## CHECKLIST

- [x] Inspect current PR #8, merged #10, Stage Director code and Weather NEWS registry
- [x] Build fixed registered-source and browser command planning
- [x] Keep all launch/input/clip authorization false
- [x] Add C policy tests, no-actuation checks and CI
- [ ] CI passes on draft PR
- [ ] Trusted compositor-owned WATCH request and secure profile/executable creation
- [ ] Spawn and supervise one browser child, capture pidfd, associate its real mapped view
- [ ] Install popup/input containment, post-layout clip reapply, CLOSE and failure rollback
- [ ] Full SwayPX nested-compositor build/runtime acceptance, operator-approved merge/deployment

## NEXT

The next **implementation boundary** is a trusted spawn controller, not more URL catalog work. Its input must be an authenticated operator WATCH intent bound to a particular mapped Weather layer and actual NEWS viewport. It must create/check the private profile and known executable securely, launch only after renderer/input/popup protections exist, capture the child pidfd/process generation, and terminate/reap exactly that child on CLOSE. Test in nested SwayPX before enabling WATCH playback.

## DECISIONS

Build a deliberately strict source/executable plan and retain all runtime denial defaults. Do not weaken browser isolation, merge incomplete dependencies to `master`, or quietly launch a desktop browser outside Weather Station.
