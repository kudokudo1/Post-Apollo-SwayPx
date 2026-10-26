# Stage Director // Isolated complete SwayPX build gate

**PATIENT:** `kudokudo1/Post-Apollo-SwayPx`.  
**STATE:** GitHub CI integration milestone, stacked on Stage Director NEWS pointer/popup boundary PR #13.  
**OPERATOR:** Final authority. No Hospital assignment, worker session, production promotion or local machine deployment is presumed.

## IMPLEMENTATION

This milestone intentionally does **not** add another independent security policy or enable the browser. The SD1 chain already contains 22 passing standalone contracts for viewport geometry, dormant wlroots clipping, layer/actor observation, process lineage and pidfd custody, fixed NEWS source planning, a CI-only supervised benign child, and popup/input **candidate** boundaries. Standalone C tests cannot establish whether those files actually compile and link into the SwayPX compositor with its wlroots and SceneFX ABI.

The repository's `meson.build` declares **wlroots 0.20.x**, `scenefx-0.5` and the usual Wayland dependencies. Its existing `flake.nix` declares a `swayfx-unwrapped-git` package that builds this repository's sources with `wlroots_0_20` and the `scenefx-git` input. `flake.lock` pins dependency revisions.

Add `.github/workflows/stage-director-full-compositor-build.yml`:

- Trigger on a pull request against this Stage Director development branch (and future `master` integration) when the compositor, protocol, Nix or workflow sources change, or by explicit `workflow_dispatch`.
- Use an ephemeral, isolated GitHub-hosted Ubuntu runner. Install Nix using the documented Cachix action. Limit parallel jobs/cores to constrain runner memory.
- Checkout the exact PR integration ref, record its SHA and the Nix version, and require both the pinned flake and actual SD1 source files.
- Run `nix build .#swayfx-unwrapped-git --no-link --print-out-paths --show-trace`. A zero-exit build means Meson compiled and linked the **full SwayPX package**, not just the isolated policy units. Confirm its `bin/sway` executable exists. All results stay in the temporary runner's Nix store; nothing is deployed to the operator's machine.
- Emit a verification summary identifying the narrow meaning of compile/link success and explicitly excluding runtime containment claims.

## HOW TO USE

From the draft PR, inspect the **Stage Director full compositor build (isolated)** GitHub Actions check and its raw job logs. This is the preferred Post-Apollo UI workflow. No local commands, worktree changes, Fedora package installs, or SwayFX restarts are required.

## VERIFY

Two distinct gates must be reported:

1. **Source-contract CI:** standalone NEWS policy and no-activation tests. These already passed 22 checks at PR #13 head `a90fbe90af94523a14f10915201b8b4e9081843a`.
2. **Full SwayPX build CI:** the new workflow compiling the real binary with locked Nix dependencies. **Pending until its actual GitHub run succeeds.** CI configuration is not a successful build.

If the build fails, inspect its stage and classify it as dependency/flake evaluation, underlying upstream baseline, Stage Director compile/API mismatch, linker, or resource/external download problem. Do **not** silently change user/local environment, lower security gates or claim the compositor is operational. Compare with a build of the same baseline when necessary.

## WATCH OUT FOR

- GitHub Actions can take significant time and may fail for dependency availability, runner limits or Nix evaluation; failure is diagnostic and must not be relabeled a pass.
- The existing flake uses upstream SwayFX package overrides and the `scenefx-git` input, which may have drifted from the repository's Meson dependency ABI. Reconcile source compatibility only with inspected evidence.
- The repository's legacy Dockerfile/compose examples reference older wlroots/SceneFX/SwayFX versions and **must not** be silently used as proof for the modern SwayPX source.
- A Nix build does not launch Sway, verify layer-shell namespace, verify XDG popups remain in the NEWS bay, test keyboard/touch/relative-pointer safety, or prove a trusted browser launch/session.
- The current `stage_director_viewport_clip_view` remains unwired. Sway transaction code may overwrite content clipping. The NEWS pointer/popup helper is still nonactuating.
- This workflow is stacked on PR #13 and its SD0/SD1 development predecessors, not a production deployment to SwayPX `master`. Merging intermediate branches into one another is not equivalent to integrating that chain into `master`.

## CHECKLIST

- [x] Check current GitHub heads, open PRs and canonical Syntagma rules
- [x] Inspect actual Meson, Nix, Docker and compositor source paths
- [x] Add isolated complete-compositor build/link workflow
- [x] Include reproducible source SHA and build-result evidence
- [ ] Workflow passes on the current PR HEAD
- [ ] Reconcile any compilation or dependency errors without bypassing authorization
- [ ] Nested compositor smoke test of a real, clipped test actor
- [ ] Session-scoped pointer, popup, keyboard, focus and teardown enforcement
- [ ] Operator-approved merge/integration/production activation

## NEXT

After the **actual full SwayPX binary compiles and links**, establish a nested Wayland compositor test environment to verify that a deliberately oversized test actor is painted only in the authorized NEWS viewport, that `wlr_scene_node_at` and all relevant event paths refuse clicks outside, and that popups never escape. Start only a harmless test actor, then evaluate an isolated browser when session ownership is trustworthy. Keep the production browser launch and WATCH controls disabled until those checks pass.

## DECISIONS

Prefer the repository's existing pinned Nix build contract to dependency guesses or local desktop experiments. Create no live spawn/clip/keyboard changes during this milestone. Preserve all prior authority and rollback boundaries.
