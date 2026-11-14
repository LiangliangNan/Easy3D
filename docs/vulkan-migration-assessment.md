# Vulkan migration: executive assessment

**Audit date:** 2026-11-13
**Audited revision:** `405076b7282a` (`formatting`)
**Scope:** the agreed shared Easy3D viewer and renderer surface, its OpenGL and Vulkan implementations, tracked tests and CI evidence, and the current `VulkanDevice` modularization.

## Executive assessment

The migration has reached **broad functional completion for the agreed shared surface**, but it is not yet validated complete across supported platforms. The core viewer, `MultiViewer`, `OffScreen`, renderer-backed drawing, picking, selection, snapshots, overlays, ImGui, effects, transparency, and supported tutorial workflows are implemented for either configured backend. Builds select one backend at configure time; they do not link both backends together.

The most important improvements since the previous assessment are now in place:

* A deterministic cross-backend snapshot/image comparison harness covers plain rendering, GTAO, PCSS soft shadows, average transparency, textured/material transparency, and scaled MSAA snapshots. Shared tests also cover representative color, depth, IDs, picking, interaction, resize, and DPI behavior.
* The earlier Linux `vulkan.interactionoverlay` baseline failure was reproduced and passed in the Ubuntu lavapipe/Xvfb workflow.
* Vulkan `render_device.cpp` has been decomposed internally around resource lifetime, pass recording, presentation, OIT, frame sequencing, validation, draw-buffer uploads, and target setup. `VulkanDevice` is now intentionally retained as the device-lifetime owner and high-level coordinator; further extraction of short, owner-bound methods is not justified.
* Linked-list OIT reports dropped fragments through `RenderDevice::oit_dropped_fragments()`, with focused validation tests.
* Current macOS Debug suites recorded in `ToDo.md` pass: Vulkan 141/141 and OpenGL 107/107 in the batch-76 migration trees, followed by Vulkan 147/147 and OpenGL 107/107 in the batch-72 test trees. These are recorded runs, not a fresh rerun for this assessment.

**Decision:** stop the current `VulkanDevice` extraction effort. Keep OpenGL as an established behavioral reference where tests demonstrate parity, and preserve Vulkan's explicit resource and command model. The next acceptance work is platform validation on the latest refactors, not another round of class splitting or an unmeasured asynchronous-rendering redesign.

## Status and evidence

| Area | Current assessment |
|---|---|
| Agreed feature surface | Broadly implemented on both backends. Mapple remains intentionally OpenGL-only; Qt/wx viewer tutorials were retired; Python bindings intentionally expose the backend-neutral API. These are scope decisions, not unimplemented migration requirements. |
| Cross-backend behavior | Representative, measurable parity checks exist for image output and interaction contracts. They reduce risk but do not prove equivalence for every scene, driver, effect combination, or device. |
| macOS validation | The tracker records green local full-suite runs on both backends. MoltenVK's unsupported transparency path is excluded from the cited GitHub macOS Vulkan job; local coverage and CI coverage should not be conflated. |
| Linux validation | Ubuntu CI passed the configured Vulkan window suite on lavapipe/Xvfb, the interaction-overlay regression, cross-backend image comparisons, and installed-package consumer checks. Linux/Mesa is therefore no longer an open baseline for those checks. |
| Windows validation | OpenGL/Vulkan builds, Vulkan SPIR-V, and Vulkan non-graphics tests passed in the recorded matrix. Windows Vulkan graphics/hardware execution and Windows installed-package consumers remain unverified. |
| Latest cross-platform CI | All six backend/OS jobs in [run 43315089551](https://github.com/LiangliangNan/Easy3D/actions/runs/43315089551) passed on `fcb7a23`. That run predates the subsequent OIT and `RenderDevice` modularization commits, so it does not validate the current audited revision. |
| Vulkan lifetime and synchronization | Focused `vulkan.resourcelifetime` checks exercise cache retirement/recreation, captures, resize, and repeated validation-enabled device teardown. The existing one-command-buffer/fence-wait model remains unchanged. |
| Performance evidence | The plain-point early-depth regression and repeated drawable-buffer extraction regressions were addressed with measured improvements. A later point-fallback profile conflicts with an earlier matched-scene result, so relative native/fallback throughput is unresolved. CPU/API profiling did not justify multi-frame submission or a staging pool. |
| Public API | The supported common surface is documented and installed consumers were checked on macOS and Linux. It is not symbol-identical: some `Viewer` members are Vulkan-only. Constructor order/default changes are intentional, but legacy positional calls can still compile with different meanings. |

The MoltenVK validation output concerning blending on the integer picking format `VK_FORMAT_R32G32_UINT` remains documented in the migration notes. Do not treat the absence of a test failure as proof that the validation warning is harmless or fixed.

## Findings and disposition

### 1. Cross-backend parity — substantially addressed; retain as an ongoing quality practice

The shared tests and `tests/cross_backend/image_diff.py` now compare representative color output and functional contracts across separately configured builds. The image comparator applies workload-specific limits for mean RGB error, p99 per-pixel error, high-error pixel fraction, and alpha mismatch. The Ubuntu workflow's lavapipe peer passed the image comparisons.

This is stronger evidence than independent backend test suites, but it is not exhaustive. Driver-specific rasterization, unsupported capabilities, untested combinations, and known algorithmic approximations can still diverge. In particular, Vulkan shadow filtering uses fewer PCSS taps than OpenGL, and some transparency algorithms have inherent quality/capacity boundaries.

**Disposition:** the planned parity harness and its current representative workloads are complete. Extend it when a user-visible behavior changes or a concrete discrepancy is reported; do not make “pixel-identical everywhere” an acceptance claim.

### 2. Viewer API compatibility — intentional breaking behavior, clearly document

The common Viewer API and installed-consumer checks are documented, including Vulkan-only extensions. Constructor ordering/defaults intentionally changed. A legacy positional call can compile while assigning integer arguments different meanings; the project deliberately chose not to add a deprecated compatibility overload.

**Disposition:** no compatibility overload is planned. Keep migration examples and the warning about positional arguments prominent, and treat future constructor changes as explicit API changes. This is a source-compatibility hazard, not evidence that the rendering backends are functionally incomplete.

### 3. Vulkan maintainability — current extraction boundary reached

The previously centralized Vulkan implementation now has internal owners/builders/coordinators for GPU caches, pass caches and encoding, presentation, AO, OIT resources/descriptors/frame coordination, frame sequencing, capture, clipped geometry, validation, transparent ordering, draw buffers, and frame-target setup. Ownership and the existing submission/fence ordering remain explicit. `prepare()` is primarily a frame-level coordinator and command-to-resource assembler.

The helpers remain in the Vulkan implementation translation unit; this work has established responsibility and lifetime boundaries, not a mandate to split every helper into another source file. `VulkanDevice` still owns device-lifetime handles and coordinates operations that depend on those handles. Pulling more short owner-bound setup methods out would add pass-through layers without a cohesive independent lifetime.

**Disposition:** stop extraction here. Reopen modularization only when a new responsibility has an independent ownership/lifecycle boundary or a concrete maintenance problem demonstrates the need.

### 4. Throughput and resource scaling — correctness diagnostics improved; larger redesign deferred

The linked-list OIT path now exposes dropped-fragment counts, and tests assert overflow and non-overflow behavior. Recent measurements also found and fixed two substantive performance defects: plain Vulkan points had unnecessarily disabled early depth testing, and empty SurfaceMesh lock drawables caused repeated CPU buffer extraction. The recorded huge-mesh measurements show the latter correction bringing Vulkan debug frame time from about 211 ms to about 28 ms in the measured workload.

The renderer still submits synchronously and waits on a fence, with one frame in flight and individual resource allocations. However, profiling found command recording and static-scene staging allocation small compared with synchronous execution, and staging uploads were not recurring in the timed loop. The native-versus-expanded point discrepancy is now resolved for the current code: a 2026-11-14 Windows run on an RTX 2080 Ti reported `largePoints` enabled and disabled for the two paths, recorded 223 and 211 FPS at identical rasterized coverage, and put the entire difference in fence time with CPU recording within 3% — timings above the display refresh, so uncapped presentation rather than vsync. The expanded path's extra cost is one-time staging creation (72.5 ms against 1.96 ms). The profiler still reports CPU/API wall timings, not GPU timestamps.

**Disposition:** retain the simple synchronous lifetime model. Do not implement frames in flight, staging reuse/suballocation, or allocator redesign without repeatable GPU-aware evidence showing a bottleneck. The controlled fallback benchmark has now been re-run with verified path selection (2026-11-14, `docs/vulkan.md`), so native-versus-expanded conclusions may be drawn from it: the portable path costs 5-8% of frame time on that hardware, and the point-fallback discrepancy is no longer a reason to doubt the synchronous model. Revisit fixed capacities only when a concrete workload reaches them.

### 5. Platform and packaging validation — final material acceptance gap

The known Linux overlay failure is closed by a successful lavapipe/Xvfb CI run. The workflow is active and the recorded six-job matrix passed on macOS, Windows, and Ubuntu. Linux installed-consumer checks passed. Windows build/SPIR-V/non-graphics checks passed, but the workflow did not execute Vulkan graphics on Windows hardware or validate Windows installed-package consumers. More importantly, the latest successful full matrix predates the audited OIT and modularization changes.

**Disposition:** closed on 2026-11-14. CI run 12 executed on the migration head with all six matrix jobs green, Windows Vulkan graphics ran on supported hardware in Debug and Release, and the installed-package consumers were built and run there for both backends. `ToDo.md` and `docs/vulkan.md` record the results, including what the hosted Windows runner still cannot do. Keep the gate open for future revisions: a change to the packaging or backend code reopens it until the current revision is tested.

### 6. Product scope and known differences — keep bounded

Mapple is an intentional OpenGL-only application. Qt/wx viewer tutorials were retired, and Python bindings remain limited to the supported backend-neutral API. Advanced text shaping/bidi/fallback, line joins/caps, automatic clipping caps, per-drawable AO, and PBR-grade material behavior are not established as migration parity requirements. OpenGL's colored/textured line shader variants remain separate where their inputs or lighting/alpha semantics differ.

**Disposition:** keep these boundaries explicit. Do not reopen them as migration blockers unless the supported product/API requirements change. Distinguish intentional unsupported features, approximations, and platform-specific failures in future documentation.

## Recommended next steps

1. **Run the active backend CI matrix on the current migration head.** *Closed 2026-11-14: workflow run 12, head `4e3292f7`, all six matrix jobs success.* The Windows jobs still cannot render - the hosted image exposes no Vulkan ICD and no desktop GL driver, which the workflow now measures with non-blocking probes rather than asserting.
2. **Close the Windows-only gaps.** *Closed 2026-11-14: the graphics and hardware suites ran on an RTX 2080 Ti in both backends and both configurations, and the three installed-package consumers ran there 10/10 over Debug and Release.*
3. **Record and triage current platform results.** *Done 2026-11-14 in `ToDo.md` and `docs/vulkan.md`; investigate regressions as they appear rather than broadening migration scope.*
4. **Resolve the point-fallback measurement discrepancy before optimizing it.** *Closed 2026-11-14: the matched workload repeated on Windows hardware with `largePoints` reported per path and timings above the display refresh, so the gap is 5-8% rather than 6.5x; the cause of the 2026-11-09 reading is still unidentified, and GPU timestamps remain the missing instrument for anything finer.*
5. **Leave `VulkanDevice` at the current coordination boundary.** Pursue asynchronous submission, staging pools, or further source-file decomposition only when a measured bottleneck or a clear independent ownership boundary justifies the added complexity.

## Overall comparison

Neither backend is uniformly superior. OpenGL remains the mature behavioral reference for established Easy3D rendering and has a direct mapping to the legacy renderer, but it depends on current-context discipline and mutable global state. Vulkan has explicit resource ownership, command submission, capability checks, and validation, and its internal responsibilities are now substantially better separated. Its remaining risks are incomplete Windows graphics/package evidence, known platform-specific validation output, and performance assumptions that are not yet supported by consistent measurements.

The appropriate strategy is to preserve the established shared behavior, keep targeted cross-backend tests as regression guards, close the remaining Windows gate, and make any performance redesign evidence-driven.

## Source references

* `ToDo.md` — current test, CI, benchmark, API, and modularization status.
* `docs/vulkan.md` — backend policy, platform coverage, known constraints, profiling notes, image-comparison limits, and the current extraction boundary.
* `easy3d/viewer/viewer.h`, `viewer.cpp`, `multi_viewer.cpp`, and `offscreen.cpp` — shared API and compile-time backend implementations.
* `easy3d/renderer/render_device.h` and `easy3d/renderer/backends/vulkan/render_device.cpp` — backend-neutral command contract and Vulkan implementation.
* `tests/cross_backend/image_diff.py` and shared Viewer/renderer tests — representative cross-backend image and behavior checks.
