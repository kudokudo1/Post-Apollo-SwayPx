# Stage Director // SceneFX pinned software EGL runtime probe

**Patient:** `kudokudo1/Post-Apollo-SwayPx`. **State:** Isolated CI runtime follow-up on PR #15, no live activation. **Authority:** Operator final; no Hospital Room/Assignment ownership assumed.

## IMPLEMENTATION

[PR #15](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/15) compiled the complete SwayPX binary successfully but its runtime startup failed before opening a Wayland socket on a standard GitHub runner:

```text
Failed to find any DRM render node
Cannot create GLES2 renderer: no DRM FD available
Failed to create renderer
```

The test originally set `WLR_RENDERER=pixman` and later added a hard DRM-node preflight. That assumed SceneFX always required a DRM render device, which turned out to be **too strong**. I independently examined the **exact SceneFX commit locked by the SwayPX flake**: `wlrfx/scenefx@26200c5d9fa947a1d3b0935d3f9237229cd3e5ba`. In `render/fx_renderer/util.c::open_preferred_drm_fd`, `WLR_RENDERER_FORCE_SOFTWARE=1` explicitly sets the DRM FD to -1 and returns success rather than enumerating DRM render nodes. The actual `fx_renderer_create` in SwayPX uses this path. Its software EGL initialization still has to succeed on the runner: no result is assumed.

**Second measured result:** [GitHub Actions run #40401536000](https://github.com/kudokudo1/Post-Apollo-SwayPx/actions/runs/40401536000) **failed**, but advanced beyond the DRM probe. Its raw logs confirmed `WLR_RENDERER_FORCE_SOFTWARE` was read, then showed `Supported EGL client extensions:` empty, `EGL_EXT_platform_base not supported`, and `Failed to create EGL context`. This demonstrates a missing EGL vendor/driver discovery path on the unwrapped Nix runner; it does not establish that the software renderer is operational.

**Third diagnostic step:** The test now resolves the Mesa package from **the same pinned nixpkgs flake input**, requires its `share/glvnd/egl_vendor.d/50_mesa.json`, `lib/libEGL_mesa.so.0` and `lib/dri`, and passes `__EGL_VENDOR_LIBRARY_FILENAMES`, `LIBGL_DRIVERS_PATH`, and a temporary `LD_LIBRARY_PATH` **only to the CI child**. This matches the GLVND Mesa ICD discovery contract rather than importing arbitrary host graphics libraries. That follow-up CI result is pending.

This PR updates only the disposable headless test:

- Use `WLR_RENDERER_FORCE_SOFTWARE=1`, `WLR_RENDERER_ALLOW_SOFTWARE=1`, `LIBGL_ALWAYS_SOFTWARE=1`, and `EGL_PLATFORM=surfaceless` while launching the isolated compositor.
- Remove `WLR_RENDERER=pixman` and the hard DRM-node gate, which prevented testing the pinned SceneFX fallback.
- Preserve the private mode-0700 XDG runtime, minimal test config, headless backend, bounded Wayland-socket readiness test, exact-child termination and cleanup.
- Keep GitHub Nix full binary build, no browser, no Weather Station WATCH activation, no local SwayFX changes.
- Enable existing workflow to run against the latest stacked development base.

## HOW TO USE

Inspect the **Stage Director isolated headless SwayPX runtime smoke** check on the draft PR. Nothing to run on the operator's Fedora desktop.

## VERIFY

A passing job would prove only an isolated software-rendered SwayPX process initialized a Wayland socket and shut down. No visual crop, browser process, popup/seat/focus policy, input isolation or accepted NEWS viewport is exercised. If the software EGL path fails, preserve exact runner logs and classify the failure; do not silently require a paid GPU runner or downgrade compositor behavior.

## WATCH OUT FOR

This software mode is **CI-specific**. It does not alter the operator's graphics configuration, installed SceneFX or SwayFX renderer. `WLR_RENDERER_FORCE_SOFTWARE` is supported by the lock's SceneFX revision, but success also depends on Mesa/EGL capabilities. Successful software startup does not imply compatibility with the user's NVIDIA graphics or prove any scene containment.

## CHECKLIST

- [x] Verify PR #14/#15 merge state and current SwayPX master separation
- [x] Inspect pinned SceneFX renderer function and actual SwayPX initialization
- [x] Select documented software EGL fallback in isolated test
- [x] Remove false hard-DRM blocker, preserve cleanup and CI boundaries
- [ ] CI startup and shutdown successful on updated PR HEAD
- [ ] Verify actual NEWS paint/hit-testing/popup confinement with a mapped disposable test actor
- [ ] Operator-approved integration/deployment

## NEXT

If startup succeeds, develop a disposable Wayland test actor and establish observation and enforcement of **real** viewport clipping and input/popups in the isolated software-rendered compositor. Keep production NEWS WATCH disabled until complete actor session ownership and containment are accepted.

## DECISIONS

Do not treat an unavailable DRM render node as an absolute SceneFX prerequisite. Test the exact supported software path first. Do not enable actual browser playback, modify local systems or merge drafts without operator authorization.
