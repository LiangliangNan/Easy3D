- Vulkan-OpenGL API Unification
  - Target: one public API for applications, drawables, and custom rendering, with OpenGL and Vulkan differing only in private implementations and reported capabilities. Backend selection may remain build-time; runtime switching is not required. The proposed contract is in [Vulkan-OpenGL-unification.md](Vulkan-OpenGL-unification.md), especially sections 6 and 7.

  - [x] Milestone 0 — Freeze the public contract and compatibility policy
    - [x] Review every proposed type and member in section 7 of the unification report against the current C++ and Python APIs; classify each as retain, add, adapt, deprecate, make private, or remove. See [section 8](Vulkan-OpenGL-unification.md#8-milestone-0-current-api-audit-and-frozen-direction).
    - [x] Resolve the details not yet fixed by the proposal: `PresentationOptions`, the ownership/lifetime of `Surface`, `DrawableRenderer` ownership boundaries, `FrameImage` readback defaults, effect behavior when a capability is unavailable, and protected versus public methods in `OffScreen` and `MultiViewer`. See [section 8.6](Vulkan-OpenGL-unification.md#86-frozen-public-contract-decisions).
    - [x] Coordinate model/drawable ownership and return types with the separate raw-pointer modernization TODO rather than introducing a second ownership migration.
    - [x] Record exact old-to-new signatures, deprecation attributes, Python replacements, and the release in which each compatibility wrapper may be removed. See [section 8.7](Vulkan-OpenGL-unification.md#87-migration-and-deprecation-policy).
    - [x] Add minimal C++ compile-smoke programs for interactive, offscreen, multi-view, custom-drawable, and low-level `RenderDevice` usage; each program must compile unchanged against both backend builds without backend macros or backend-specific includes. Both builds pass `viewer.apismoke`; see [section 8.9](Vulkan-OpenGL-unification.md#89-milestone-0-validation).

  - [x] Milestone 1 — Complete the backend-neutral graphics contract
    - [x] Consolidate the public value vocabulary (`Backend`, topology, point/line styles, transparency methods, readback modes, geometry, images, commands, frame images, and selection requests), reusing existing types where they already match the proposal.
    - [x] Group the long `RenderDevice::render()` argument lists into `RenderOptions`; keep presentation behind the private host adapter and do not add a generic public `Surface`; retain deprecated positional overloads only during migration.
    - [x] Add backend-neutral `create_device()` using common `DeviceOptions`; keep native surface integration in the explicit backend interop boundary and the existing OpenGL/Vulkan factories as temporary forwarding wrappers.
    - [x] Make device identity, driver information, validation diagnostics, sample-count queries, and optional capability values available without exposing native handles. Virtual `backend()`/`capabilities()` queries remain deferred to the coordinated 3.0 vtable change.
    - [x] Add command factories for common scene geometry, screen-space lines, textured quads, and overlays, while keeping explicit `DrawCommand` fields for source stability.
    - [x] Implement and test the consolidated contract in both adapters before routing Viewer through it. See [section 8.10](Vulkan-OpenGL-unification.md#810-milestone-1-implementation-and-validation).

  - [x] Milestone 2 — Make drawable geometry and realization backend-neutral
    - [x] Standardize `Drawable::BufferData` and revision tracking so `buffer_data()` is the portable source for geometry, indices, primitive/source mappings, and cache invalidation.
    - [x] Route all drawable buffer updates through the shared realization/cache path; cover dynamic updates, element-buffer disabling, resizing, detachment, and repeated updates in both adapters.
    - [x] Move `DrawableRenderer` from command translation toward ownership of portable geometry/image caches; it must not own devices, windows, cameras, or native GPU resources.
    - [x] Express point, line, triangle, and screen-space line styling through portable command state; verify plain/impostor rendering, smooth shading, opacity, line width, and framebuffer-pixel conventions on both backends.
    - [x] Migrate ordinary in-tree rendering callers of `Drawable::draw()`, `gl_draw()`, `vao()`, and raw buffer getters to `DrawableRenderer` or command assembly, including Python bindings, tutorials, tests, supported Mapple, and the retired Mapple-Qt reference source. Retain native calls only in centralized compatibility/effect/picking fallbacks.
    - [x] Deprecate native drawable rendering/resource access in 2.8 after both adapters pass parity tests; reserve declaration, definition, binding, header, and build/install removal for the coordinated 3.0 release. See [section 8.11](Vulkan-OpenGL-unification.md#811-milestone-2-implementation-and-validation).
    - [x] Audit corner-axis lighting/depth/blending, native EDL, live/capture parity, immediate native compatibility, and MultiViewer divisions; compare deterministic baseline rendering, pass the complete serial OpenGL 125/125 and Vulkan 154/154 suites, and run visual macOS Mapple validation with both backends. See [section 8.11](Vulkan-OpenGL-unification.md#811-milestone-2-implementation-and-validation).

  - [x] Milestone 3 — Turn rendering effects into portable settings
    - [x] Add `RenderSettings` and its shadow, ambient-occlusion, transparency, eye-dome-lighting, and clipping substructures with documented defaults, units, and validation.
    - [x] Add `set_render_settings()` to all rendering hosts and make shared command assembly consume those settings.
    - [x] Implement the shadow settings through both adapters, including hard/soft shadows and virtual background; verify matching light-space transforms and fallback behavior.
    - [x] Implement AO and eye-dome-lighting through both adapters without publishing native AO texture IDs or effect framebuffers through `State`.
    - [x] Route transparency through `TransparencyMethod` and layer settings; test sorted, average-color, depth-peeling, and linked-list paths, including unsupported-capability fallbacks and dropped-fragment diagnostics. OpenGL falls back below 4.3 and does not expose a native linked-list fragment counter.
    - [x] Apply clipping planes in shared command/state assembly and verify point, line, and rendered capture results on both backends. OpenGL native effect paths apply only the process-wide clipping plane, and the OpenGL adapter rejects analytic point/line clipping in the current tree; the planned follow-up in [section 8.15](Vulkan-OpenGL-unification.md#815-milestone-5-follow-up-plan) removes that rejection.
    - [x] Keep temporary effect compatibility facades that translate existing controls into `RenderSettings`; retire them only after application and Python migration. The facades remain unchanged in this milestone, with settings taking precedence where both are active; their control translation is deferred to the Python/tutorial migration.

  - [x] Milestone 4 — Unify Viewer, OffScreen, and MultiViewer
    - [x] Introduce common window/device construction and remove public `OpenGLOptions`/`VulkanOptions` conditionals while preserving backend-specific settings through compatibility overloads.
    - [x] Make `device()`, `draw_commands()`, overlays, clipping, render settings, and in-memory capture available consistently across `Viewer`, `OffScreen`, and `MultiViewer`.
    - [x] Preserve Viewer lifecycle/customization hooks and move custom drawing to backend-neutral commands; keep window-only operations out of the public `OffScreen` surface.
    - [x] Unify overlay submission through `DrawCommand` lists with explicit foreground/background ordering; migrate HUD and application overlays while preserving depth testing, depth writing, transparency, and capture inclusion.
    - [x] Implement presented-scene capture behind the shared host/device contract so it includes effects, overlays, HUD, and custom drawing with the same readback options as offscreen rendering.
    - [x] Remove Viewer's direct capture FBO path after offscreen and presented captures agree for color, depth, IDs, sample counts, resizing, and clipping.
    - [x] Add grid-aware `MultiViewer` parity tests for assignment, layout changes, divisions, coordinate conversion, picking, overlays, and snapshots.

  - [x] Milestone 5 — Unify picking and remaining exact fallbacks
    - [x] Route point, pixel, and region-ID queries through the shared `RenderDevice` contract, including clipping and model transforms, for the drawable states the shared command assembly supports.
    - [x] Preserve CPU/geometric selection helpers where deliberately backend-independent, but remove rendered native fallbacks once shared query behavior is equivalent, except the OpenGL analytic-clipping fallback planned below.
    - [x] Retire picker-owned framebuffers and native picking shaders after visible-surface, occlusion, boundary, hidden-object, and empty-region tests pass on both backends.
    - [x] Search for `EASY3D_HAS_VULKAN` branches in public headers and migrate client-visible branches to capabilities or common options; retain only private adapter/build implementation branches.

  - [ ] Milestone 5 follow-up — OpenGL analytic clipping and exact-query fallback removal
    - [ ] Add normalized view-space clip-plane uniforms and actual-surface fragment tests to the OpenGL analytic point and line impostor shaders; remove the adapter rejection while keeping the existing CPU control cull for the whole-primitive policy. See [section 8.15](Vulkan-OpenGL-unification.md#815-milestone-5-follow-up-plan).
    - [ ] Remove the OpenGL analytic-impostor clipping fallback from `PointCloudPicker` so clipped single picks use the shared `RenderDevice` query.
    - [ ] Stop rejecting clipping in `Viewer::supports_device_query` once the adapter supports analytic clipping, so `Viewer::pick`, `Viewer::read_scene_depth`, and capture use the shared query for clipped supported scenes; keep the exact pass only for unsupported drawable/effect states.
    - [ ] Extend the clipping regression to both backends, add clipped-impostor shared-pick and invalid-plane coverage, and rerun the complete OpenGL and Vulkan suites. Record the results in section 8.15 and section 8.14.

  - [ ] Milestone 6 — Retire redundant OpenGL-facing APIs
    - [ ] Re-audit installed headers, Python bindings, tutorials, examples, and tests immediately before each removal; zero in-tree callers alone is not sufficient for public API deletion.
    - [ ] Move native framebuffer, shader, VAO, timer, utility, texture-realization, and immediate-mode shape implementations out of the installed public API once their replacements are complete.
    - [ ] Remove `OverlayRenderer` only after shared overlay presentation and capture parity are verified.
    - [ ] Replace live OpenGL presentation, effect, texture, and drawable resource plumbing incrementally; do not delete `FramebufferObject`, shaders, or VAOs merely because offscreen `RenderDevice` no longer needs them.
    - [ ] Delete each deprecated declaration, definition, binding, install/export entry, and test dependency together in a coordinated release.

  - [ ] Milestone 7 — Bindings, documentation, validation, and release
    - [x] Verify the one-argument Python `Drawable.update_highlight_mask(strengths)` binding and compile the Python bindings.
    - [ ] Expose the unified hosts, drawable data/style APIs, `RenderSettings`, low-level device/value types, and array-like `FrameImage` access in Python without native buffer, texture, VAO, or framebuffer handles.
    - [ ] Update tutorials and examples to demonstrate user-facing APIs first, then command customization and low-level rendering; no example should require client-side backend selection macros.
    - [ ] Publish a migration guide covering C++ and Python replacements, compatibility wrappers, behavioral differences, and capability-dependent fallbacks.
    - [ ] Maintain a backend-parity test matrix for scene rendering, drawable updates, effects, overlays, picking, capture, resize, and MultiViewer; document intentional implementation tolerances and unsupported features.
    - [ ] Run both complete backend test suites plus public-header/install and Python smoke tests before deprecations and again before removals.
    - [ ] Remove the final compatibility wrappers only in the approved release, after the migration guide and downstream-facing release notes are published.


  - [ ] Functions (like Viewer::draw_corner_axes()) have almost identical implementation code in the two backends. One should be removed at the end of the API unification.




  - [ ] Regarding raw pointers (vs shared_ptr), shared_ptr is preferred and it is modern C++. I want to discard the old raw pointer functions for add_model and add_drawable. Should they also return a shared_ptr (instead of raw pointer)? Relevant classes (not a complete list): SurfaceMeshIO, PolyMeshIO, GraphIO, PointCloudIO, ImageIO. and many other places.
  - [ ] Implement an application called "CloudExpert" that adapts potree for huge point cloud visualization (https://github.com/potree/potree) and editing (e.g., lasso selection, property management).
  - [ ] Use a coding agent to *proofread* the code, documentation, and comments.
