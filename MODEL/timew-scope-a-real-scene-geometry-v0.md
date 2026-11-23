# TimeW Scope A // Real SwayPX post-scene exact geometry and output context (CI only)

**PATIENT //** `kudokudo1/Post-Apollo-SwayPx`  
**ROOM //** TimeW Scope A native selected-XDG and post-command settlement evidence  
**STAFF ROLE //** SwayPX DOC, external Hospital-compatible; live registration not verified  
**BASE //** [Reviewed source-only #185](https://github.com/kudokudo1/Post-Apollo-SwayPx/pull/185), exact `775b2519fddf2df63d6fde29e2b4229e03f8fc41`, stacked on independently reviewed #182  
**STATE //** Disposable native diagnostic, NOT an installed source/selected-window receipt; TimeW Restore HOLD, Autoscroll HARD OFF

## IMPLEMENTATION

This increment advances *actual* compositor observation, not another duplicate identity issuer.

The existing dormant SwayPX `timew_scope_a_native_txn_snapshot_read()` from #174 already reads **real in-process** XDG `container->pending` and `container->current`, exact IEEE754 binary64 x/y/width/height, real native output layout origin, transformed dimensions, `wlr_output->scale` and `transform`, workspace/output refs. It refuses missing/unmapped/destroyed/fullscreen/nonfloating/scratchpad/locked, pending/current workspace-output or parent drift, invalid geometry and missing output state, and sets every source/ACK/settlement/selection/C5/movement proof flag **false**. It is not normally in production Meson.

`tests/timew/ci_overlay_real_scene_geometry.py` reuses #172 actual mapped XDG, #178 private lifetime, #179 actual native instruction, #182 READY/TIMEOUT/CURRENT/SCENE overlays **in a disposable GitHub checkout only**. It links the original dormant reader by temporarily injecting one Meson line and reads the real selected fixture's native snapshot immediately after the existing CI `SCENE` callback, itself immediately after `arrange_root`. It confirms, within that same compositor process, that the container/root, output/workspace, output scale/transform match the #178 candidate lifetime. Missing or changed context produces a **deferred/drift HOLD** log, not a newly selected target.

For matching diagnostic context it emits the exact native `pending` and `current` binary64 bits, output layout origin and transformed width/height, scale and transform. The values are not a signed network byte format. A matching native rect **does not** demonstrate that a client ACKed a configure, that a frame was actually presented for the selected XDG surface, or that its final geometry is output-fit. The CI-only fixture's app_id and ordinary `swaymsg resize` remain untrusted and confined to a real `foot` actor on the GitHub runner.

`tests/timew/ci_assert_real_scene_geometry.py` cross-checks each emitted real snapshot against the reviewed #182 COMMIT→BIND→READY/TIMEOUT→CURRENT→SCENE→SEAL XDG chronology and requires at least one snapshot for each of the two true map/unmap lifetimes in a separate normal and forced-timeout compositor. It rejects reused/late/missing snapshots, forged positive trust, nonfinite/noncanonical binary64 values, invalid output transform/scale and invalid output dimensions. `tests/timew/test_ci_real_scene_geometry.py` adds 20 offline adversarial cases. New standalone workflow runs the original 28 adversarial outcome tests plus the 20 scene/geometry tests and builds real SwayPX/wlroots in isolated CI.

**Committed diff:** these CI overlay/checker/test/workflow files and this note only. Original `sway/desktop/transaction.c`, `sway/tree/view.c`, `sway/timew_scope_a_native_txn_observer.c`, `sway/meson.build`, input/seat, resize/move, IPC and Company Map/Autoscroll owner code remain unchanged. No production launch.

## VERIFY / IMPORTANT DISTINCTIONS

A green native run would support exactly this limited fact: **the real SwayPX compositor, after applying a transaction and arranging the scene, exposed finite exact current/pending binary64 logical values and output facts for the same mapped XDG diagnostic candidate and native instruction**. No actual authenticated human selected it. A generic output present event or equal `current` does not imply a settled chosen XDG frame. These tests deliberately do not add a presentation callback, protected source signature, client-ACK attestation, operator-selected lease, C5 SPENT, trustworthy receipt or permitted move.

Normal READY remains an internal serial callback, not a protected receipt. The corrected #182 forced debug-timer run observed contradictory READY+TIMEOUT, so these observations **remain UNKNOWN/HOLD** regardless of current layout values or scene arrangement. The native snapshot after SCENE cannot undo that ambiguity. No automatic MOVE, retry, undo or second consent grant is permitted. The source-owned selection/receipt proof obligation RFC in #185 remains **REVISE** pending TimeW, C3e/C4b, C5 and shared SwayPX input/scene owner dispositions.

**Output identity note:** in-process output refs/scale/transform and actual coordinates do not by themselves authenticate output epoch, physical display, workspace history or pixel placement. The test compares two observations within one disposable SwayPX process only. If native state is unavailable, the source records a defer/hold instead of fabricating a rectangle.

## HOW TO USE / NEXT / DECISIONS

Inspect GitHub Actions for the exact PR head. The test has no live desktop UI or installed CLI. After native CI, request an independent Nurse 2 scoped source/native review. The following owner decision remains **which operator gesture and independently enrolled SwayPX producer/seat/output source can supply an authenticated selection and independently settled per-step receipt**, not whether to store ordinary TimeW data locally.

Approved operator direction remains **local ordinary TimeW storage now, independent NAS/protected hardware later**, and **OS-managed-first Autoscroll with optional enrolled-device mode later**. Those decisions do not confer privileged native capability. Company Map #126 and Autoscroll #184/#186 own the shared compositor/input seams.

**DECISIONS // SwayPX protected integration REVISE · TimeW Restore PRODUCTION HOLD · Autoscroll HARD OFF. No merge, Fedora write, compositor restart, installed source key, protected C5 SPENT or production native window movement.**
