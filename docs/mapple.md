# Mapple on Dear ImGui

This document tracks the migration of Mapple from Qt Widgets (`applications/Mapple`) to a
Dear ImGui application rendered through Easy3D's backend-neutral overlay path
(`applications/Mapple-imgui`). Qt/Mapple remains buildable until the new app covers the
daily-use workflow.

Status: 2026-11-15 — Phases 0–1 complete and Phase 2 started: builds and runs on both
OpenGL and Vulkan; panels, menus, log capture, drawable styling, picking readout,
point/face selection, the clipping plane, and a first set of processing commands work.

## Decision

Rebuild Mapple on ImGui instead of adding a Qt Vulkan widget.

- The renderer side is already solved: `Viewer` runs on OpenGL and Vulkan, and the
  `ImDrawData -> graphics::DrawCommand` translation used by `Tutorial_202_Viewer_imgui`
  is backend-neutral (`vulkan.imgui` test). A Mapple shell is a `Viewer` subclass.
- Qt Widgets has no Vulkan widget. Option (1) would require per-platform
  `VkSurfaceKHR` glue (HWND / xcb / wl_surface / NSView + CAMetalLayer), resolving the
  `QWindow::setVulkanInstance` vs. viewer-owned `VkInstance` conflict, and native-child
  window composition problems with docks, menus, tooltips and modal dialogs — all in
  Easy3D core, tested on every platform, forever.
- Two UI stacks (GLFW for Viewer/tutorials/tests, Qt for Mapple) double the surface,
  device, DPI and event-loop maintenance.
- ImGui keeps one UI stack, one backend integration, and one event path, and its
  immediate mode suits 3D overlays, annotations and direct-manipulation gizmos.

## Guiding principles

1. **Capability parity, not UI parity.** Do not port the 22 `.ui` forms and 112 actions
   one-to-one. Expose what exists, in the place that makes it easy to find; add
   capabilities as they are implemented. No menu full of disabled stub items.
2. **One rendering path.** All UI geometry goes through `ImDrawData -> DrawCommand`;
   no window-toolkit drawing calls and no GL/Vulkan-specific UI code.
3. **Panels for persistent state, modal windows only for short parameter forms.**
4. **Reuse Easy3D.** `Viewer::open/save/snapshot/copy_view/paste_view`, `dialog::*`
   (native file/message dialogs via portable_file_dialogs), `RenderDevice` offscreen
   capture, `KeyFrameInterpolator`, `TextLayoutEngine`.
5. **The Qt app stays shippable** until the new app passes the Phase 1 acceptance list.

## Layout

```
menu bar                          (Viewer-independent ImGui bar)
toolbar                           (Open, Save, Fit, Snapshot, Copy/Paste View)
+-------------------+-------------------------------+-------------------+
| Rendering         |                               | Models            |
| (global + active  |        3D viewport            | (tree, current    |
|  model drawables) |                               |  model, visibility)|
+-------------------+-------------------------------+-------------------+
| Log                                                               |
+-------------------------------------------------------------------+
status bar                        (active model, counts, hint)
```

The central region is the viewer framebuffer; panels are opaque ImGui windows laid out
from `GetMainViewport()->Pos/Size` plus the app's own `menu_height_` (the ImGui work
rect reflects the menu bar only on the following frame). The side panels and the log
panel are resized by 4 px splitters drawn between them. No docking branch in Phase 0;
fixed, resizable, toggleable panels.

## Code structure

```
applications/Mapple-imgui/
  main.cpp               initialize(), argv files, run()
  mapple_viewer.{h,cpp}  Viewer + logging::Logger; frame hooks, input routing,
                         layout, app state (panels, actions, log buffer)
  imgui_layer.{h,cpp}    ImGui context, font atlas, DPI, ImDrawData -> DrawCommand
  ui/menu.cpp            menu bar and shortcuts
  ui/panels.cpp          toolbar, Models, Rendering, Log, status bar, About
  ui/process.cpp         Process menu parameter windows and the algorithms they run
```

Reuse inventory (verified):

| Need | Existing API |
|---|---|
| File open/save + native dialogs | `Viewer::open/save`, `easy3d/util/dialog.h` (pfd) |
| Snapshot / offscreen capture | `Viewer::snapshot`, `RenderDevice` synchronous capture |
| Camera clipboard | `Viewer::copy_view/paste_view` (GLFW clipboard) |
| Active model selection | `Viewer::current_model()`; new `Viewer::set_current_model()` added for panels |
| HUD toggles | protected `show_corner_axes_`, `show_frame_rate_`, `show_easy3d_logo_` |
| Point/face selection | `PointCloudPicker`, `SurfaceMeshPicker` (`easy3d::gui`) |
| Hover primitive readout | `Viewer::pick` → `PickResult` |
| Log sink | `easy3d::logging::Logger` dispatch callback |
| ImGui build | `3rd_imgui` target (GLFW backend; OpenGL3 backend excluded under Vulkan) |
| Settings/layout | not implemented yet; `io.IniFilename` is disabled, so ImGui writes no layout file |

## Phases

### Phase 0 — shell (this iteration)

- [x] CMake target `Mapple-imgui`, linked `3rd_imgui`, `easy3d::core/renderer/util/viewer`
      (`easy3d::algo` joins when processing features land).
- [x] `MappleViewer : Viewer, logging::Logger`, ImGui frame hooks and input routing.
- [x] `ImGuiLayer`: context, Droid Sans atlas, DPI scaling, ImDrawData translation.
- [x] Menu bar (File, View, Camera, Help), toolbar, status bar.
- [x] Models panel: tree, active model selection, per-drawable visibility, delete, fit.
- [x] Rendering panel: background color, projection; active-model drawable styling
      (visibility, uniform color, point size, line width, smooth shading, back color).
- [x] Log panel: level-colored lines, auto-scroll, clear, copy, export.
- [ ] Settings persistence, recent files.

Acceptance: open OBJ/PLY, orbit/zoom, switch active model, toggle drawables, restyle,
snapshot, copy/paste view, log feedback — on OpenGL and Vulkan.

### Phase 1 — inspection and interaction (this iteration)

- [x] Model data inspection: the selected model's path, vertex/edge/face counts,
      bounding box, and a collapsible property listing (`Model::property_stats`),
      shown in the Models panel so it always describes the highlighted row.
- [x] Point/picking readouts in the status bar: nearest visible primitive under the
      pointer (world coordinates, face or point index), highlighted in Click mode only;
      the query is throttled and uses `Viewer::pick` in logical window coordinates.
- [x] Selection tools: click/rect/lasso for point clouds and surface meshes written
      straight to the picker API (`PointCloudPicker`/`SurfaceMeshPicker`) instead of
      moving `tools/tool.cpp` to the overlay path. The rubber band/lasso is drawn
      through `graphics::DrawCommand`, so it is backend-neutral.
- [x] Selection operations: select all, invert, clear, delete selected (with a native
      confirmation). "Show/hide selected" was considered and left out: in a viewer,
      hiding selected points/faces hides geometry without a recovery affordance, and
      delete already covers the destructive case.
- [x] Clipping plane: a collapsible section in the Rendering panel drives the shared
      `ClippingPlane` (enable, fit to scene, guide visibility/color, cross section and
      relative thickness). The plane guide, `Alt+C/V/X` shortcuts, and `Alt`+drag
      manipulation come from `Viewer`, so Mapple adds UI/state wiring only.

Phase 1 design notes:

- Selection is stored as `v:select`/`f:select` booleans and displayed by temporarily
  switching the faces/points drawable to scalar coloring with a 2x1 colormap (texel 0 =
  the drawable's own color, texel 1 = `setting::highlight_color`). Click/region
  changes only rewrite the texture-coordinate buffer. The selection display belongs to
  the selection, not to the tool: switching between Off/Click/Rect/Lasso keeps both the
  selection and its colors. The drawable's own appearance is saved once and restored
  when the selection becomes empty (Clear Selection, deselecting everything, or a
  replacement gesture that hits nothing).
- Deleting selected faces/points changes the topology, so the whole renderer is
  invalidated (`Renderer::update()`); otherwise edge/border/vertex drawables would keep
  drawing the deleted elements.
- The hover highlight is a picking aid of the Click (pick) mode only; the status-bar
  readout remains available in the other modes when Pick Readout is enabled.
- Press/release/drag in selection mode are consumed by the app so the camera never
  sees them; `Esc` leaves selection mode. Input callbacks only queue work
  (`pending_pick_`, `pending_region_`, ...) because the graphics context is not
  guaranteed current during event dispatch; `process_pending_work()` runs at the start
  of `draw_gui()`, where pickers and textures can safely touch the device.
- Clipping is scene-wide, so it lives in the Rendering panel rather than per model, and
  stays usable with no model open. Enabling it fits the plane to the scene bounding
  sphere (the scale the guide and relative cross-section thickness assume). `Alt` is
  reserved for plane manipulation: a press with `Alt` bypasses an active selection
  tool so the plane remains draggable in every mode.

### Phase 2 — processing

Expose algorithms as ImGui forms behind logical menus/panels, driven by need rather
than by the Qt dialog list. Parameter forms can be small modal windows; results report
through the log panel.

- [x] Process menu with Point Cloud and Surface Mesh submenus; items are enabled only
      for the matching active model, and the point-cloud/mesh split keeps the
      "which model type" question out of the UI.
- [x] Point cloud: estimate normals (`PointCloudNormals::estimate`, neighbors +
      curvature) and simplify to a target point count
      (`PointCloudSimplification::uniform_simplification`, followed by
      `delete_vertex`/`collect_garbage`).
- [x] Surface mesh: smoothing (`SurfaceMeshSmoothing`, explicit iterations or implicit
      with the Qt dialog's boundary-aware rescaling), simplification to a target vertex
      count (`SurfaceMeshSimplification` with aspect ratio 10 and normal deviation 180
      degrees), uniform remeshing by a factor of the mean edge length
      (`SurfaceMeshRemeshing`), hole filling by maximum hole size
      (`SurfaceMeshHoleFilling::fill_holes`), and polygonization by angle threshold
      (`SurfaceMeshPolygonization`).
- [x] Point cloud: extract primitives (`PrimitivesRansac`, type toggles, the five
      detector parameters, and an option to use only the selected points; the result is
      shown with per-primitive colors in `v:color-segments`) and Poisson reconstruction
      (`PoissonReconstruction`, octree depth, samples per node, optional density trim).
      The reconstruction adds a new model named `<cloud>_poisson.ply`.
- [x] Surface mesh: fairing (minimize area, curvature, or curvature variation),
      Catmull-Clark/Loop/Sqrt3 subdivision (direct, scheme chosen in a Subdivide
      submenu), minimum-area triangulation, dual mesh, connected components and planar
      partition painted with per-component face colors (`f:color_components`,
      `f:color_planar_partition`), and uniform surface sampling into a new
      `<mesh>_sampled.ply` point cloud.
- [x] Point cloud: Delaunay 2D (`Delaunay2` on the XY projection, keeping the original
      z) and 3D tetrahedralization (`Delaunay3`; the result is a `PolyMesh`). Both add
      a named model (`<cloud>_delaunay_XY.ply`, `<cloud>_delaunay.ply`) and stop with
      a log warning instead of adding an empty model when the input is degenerate.
- [x] Surface mesh: parameterization (`SurfaceMeshParameterization`; LSCM or discrete
      harmonic with cotan/uniform weights, UVs in `v:texcoord`, optional checkerboard
      preview) and geodesic distance seeded by the vertices of the selected faces
      (`SurfaceMeshGeodesic`, shown as the `v:geodesic:distance` scalar field; the
      selection is cleared afterward so the distance colors are visible).
- [x] Surface mesh: clipping, splitting, and slicing by the clipping plane
      (`Surfacer::clip/split/slice` from `easy3d::algo_ext`, enabled by
      `Easy3D_ENABLE_CGAL`). Clip/split modify the active mesh in place; slice adds
      the cut polylines as a new graph model (`<mesh>_slice`). The commands appear
      only in CGAL-enabled builds and warn when the clipping plane is disabled or
      the plane misses the mesh.
- [ ] Long-running commands (reconstruction, remeshing of large meshes) need progress
      reporting and cancellation instead of blocking the frame.

Phase 2 design notes:

- Every command runs synchronously inside `draw_gui()` and logs its duration and the
  resulting element counts. The shared `ProcessParams` struct keeps the last-used
  parameters; targets that depend on the model (point/vertex counts) are derived when
  the window opens.
- While a parameter window is open it is modal for the scene: mouse, keyboard, and
  hover queries are consumed before they reach `Viewer`, and a dim rectangle is drawn
  behind the window.
- After a command the whole renderer is invalidated (`Renderer::update()`) and the
  face/vertex normals are recomputed, so drawables and shading match the new geometry;
  the hover readout and the selection count cache are dropped. Polygonization rebuilds
  the mesh and discards its properties, including the selection; a saved selection
  style whose scalar property no longer exists falls back to uniform coloring through
  `buffer::update`'s missing-field path.
- Commands without parameters (subdivision, triangulation, dual mesh, connected
  components, planar partition, Delaunay 2D/3D, geodesic distance, clipping/splitting/
  slicing) execute straight from the menu item through `run_process`; only
  parameterized commands open a window. The painting commands
  replace the drawable's appearance with their color property, so they drop any saved
  selection style and skip the post-command selection display refresh that would
  otherwise repaint the mesh with the selection colormap.
- Geodesic seeds are the vertices of the selected faces, which reuses the existing
  selection tools; the command clears the selection afterward so the distance field
  is visible.
- Parameterization requires a triangle mesh with a boundary and warns otherwise. The
  checkerboard preview binds the default texture to `v:texcoord`; editing the faces'
  Color in the Rendering panel restores uniform coloring.
- Clipping, splitting, and slicing reuse the Rendering-panel clipping plane
  (`plane0()`), so the cut matches the guide exactly. Clip keeps the negative side
  (`clip_volume = false`), split keeps both sides as separate components, and slice
  adds a graph model whose closed polylines reuse their first vertex (no duplicate or
  degenerate closing edge). The commands exist only in CGAL-enabled builds and warn
  when the plane is disabled, the mesh is not triangulated, or the plane misses the
  mesh.

### Phase 3 — animation, capture, persistence

- Walk-through keyframes using `KeyFrameInterpolator`; camera-path import/export.
- Video recording via `RenderDevice` offscreen capture + `easy3d::video`.
- Snapshot dialog (size, samples, background).
- App settings and panel layout persistence.

### Phase 4 — retire Qt

Verify the acceptance list, migrate remaining workflows, remove `applications/Mapple`
and the `Easy3D_ENABLE_QT` gate (or keep Qt for other tools, not Mapple).

## ImGui notes

- **Menu-bar height** = `FontSize + 2*FramePadding.y`; content is offset by
  `max(DisplaySafeAreaPadding.y - FramePadding.y, 0)`. Never push a `FramePadding`
  with `y < DisplaySafeAreaPadding.y` around `BeginMainMenuBar`: that was the real bug
  in `Tutorial_202_Viewer_imgui` (its `FramePadding.y = 0` clipped the labels), fixed
  on 2026-11-15. Default padding gives a 22 px bar.
- **Shortcuts**: the string in `MenuItem(label, "Ctrl+O")` is display-only; real
  shortcuts use `IsKeyPressed`/`Shortcut` and must be suppressed while
  `io.WantTextInput`.
- **Font/DPI**: atlas is rasterized at `16 * dpi_scaling()` with
  `FontGlobalScale = 1 / pixel_ratio`; rebuild when the pixel ratio changes.
- **Textures**: the overlay translation currently supports the font atlas only.
  Icons need an `ImTextureID -> graphics::Image` registry.
- **Docking**: the vendored ImGui is 1.91.8 WIP, non-docking. Fixed panels are used
  now; switching to the docking branch later only affects the `3rd_imgui` sources.
- **HUD overlap**: the Viewer draws its own HUD in screen space; the new app disables
  the logo/fps HUD and repaints the information in the panels/status bar.
- **Input ownership**: decided at press time. A press over a panel is swallowed for
  the whole gesture; a press in the scene keeps its drag/release even if the pointer
  crosses a panel, so the camera never loses the button release.

## Validation

- Build the new target in both trees: `build-opengl-check` and `build-vulkan-check`.
- Smoke test both backends: launch, open a mesh, orbit/zoom, select model, toggle
  drawables, restyle, snapshot, copy/paste view, verify log output.
- Full suites stay green before any commit (`ctest -j 1 --output-on-failure`).
- Changes confined to `applications/Mapple-imgui` only need the Mapple target tested;
  the full Easy3D suites are reserved for library-wide changes.
- Changes are left uncommitted; summarize and wait.

## Changelog

### 2026-11-15 — Phase 0 shell

- Added `applications/Mapple-imgui` (target `Mapple-imgui`).
- `MappleViewer` with ImGui lifecycle, input routing, panel layout, log sink.
- Models/Rendering/Log panels, toolbar, status bar, menu bar, About.
- Added `Viewer::set_current_model(Model*)` so panels can change the active model.

### 2026-11-15 — Phase 0 built and smoke tested

- Target builds cleanly in `build-opengl-check` (OpenGL) and `build-vulkan-check`
  (Vulkan, tests ON).
- Smoke test with `resources/data/bunny.ply` on both backends: menu bar, toolbar,
  Rendering/Models/Log panels, splitters and status bar render; the model loaded from
  the command line is listed and active, its drawables appear in both panels, and log
  messages (loading, scalar-field clamping, UI-ready) reach the Log panel. The
  interactive controls were not scripted in this pass.
- Build fixes: use `easy3d::clamp()` (the project is C++11, so `std::clamp` is
  unavailable); forward declare `easy3d::Model` instead of a global `Model`;
  `ui/*.cpp` include the shell header as `"../mapple_viewer.h"`.
- Validation: serial `ctest -j 1 --output-on-failure` with tests ON in both check
  trees — OpenGL 111/111, Vulkan 145/145. `easy3d/util/version.h` reverted after
  configuring. The rebuilt OpenGL binary was launched once more to confirm it starts;
  dragging from the scene across a panel is covered by the press-time input latch
  (compiled, not scripted).

### 2026-11-15 — Tutorial_202 menu-bar clipping fixed

- `ViewerImGui::draw_gui()` pushed `FramePadding = (5, 0)` around `BeginMainMenuBar()`,
  which sets `MenuBarOffsetMinVal.y = max(DisplaySafeAreaPadding.y - FramePadding.y, 0)`
  while sizing the bar as `FontSize + 2*FramePadding.y` — labels were shifted down inside
  a bar with no room for them. It now keeps the style's `DisplaySafeAreaPadding.y` as the
  vertical padding (horizontal stays 5), so the default 22 px bar renders fully.
- Documented the rule in the `ViewerImGui` class comment (`viewer.h`) and in the
  "ImGui notes" above, so Mapple's own menu bar copies the corrected form.
- Validation: `opengl.imgui` and `vulkan.imgui` regression tests pass; both snapshots
  show the File/View/Help labels uncut.

### 2026-11-15 — Phase 1 inspection and selection

- Active-model inspection in the Rendering panel: path, counts, bounds, and a
  scrollable property listing.
- Status-bar pick readout: hovering resolves the nearest visible primitive and reports
  the model name, face/point index, and world position. In Click mode it also
  highlights the primitive (`set_highlight_range`, `gl_PrimitiveID` on both backends);
  Rectangle/Lasso/Off leave the scene untinted. Disable the readout with
  Select > Pick Readout.
- Select menu and toolbar toggles for Click/Rect/Lasso; Shift adds, Ctrl removes,
  plain gestures replace; Esc exits. `Ctrl+A`, `Ctrl+I`, Clear Selection, and
  Delete Selected (Del, native confirmation) operate on the active model's points or
  faces. Switching modes keeps the selection and its colors; an empty selection
  restores the drawable's own appearance.
- Selection display uses a 2x1 colormap over `v:select`/`f:select`; the drawable's
  appearance is snapshotted when the first selection is made and restored when the
  selection becomes empty. Gestures are queued from input callbacks and executed in
  `draw_gui()`; the rubber band/lasso is overlay geometry clipped to the scene area.
- Deleting selected primitives invalidates all drawables of the model
  (`Renderer::update()`), so edges, borders, and vertices are rebuilt with the new
  topology.
- `Mapple-imgui` now links `easy3d::gui` for the pickers.
- Validation: target builds in `build-opengl-check` and `build-vulkan-check`; launched
  on both backends with `easy3d.ply`/`bunny.ply`. On Vulkan, rect/lasso gestures were
  exercised against `easy3d.ply` (1836 faces): selecting all and selecting none both
  logged the expected counts, the status bar showed `Select: <mode> (1836 selected)`,
  and the selected faces rendered in the highlight color. Click-selection, delete, and
  leaving selection mode were not scripted in this pass (the desktop was controlled
  through TeamViewer during validation).

### 2026-11-15 — Phase 1 follow-up fixes

- Deleting selected faces left stale edge/border/vertex buffers, because only the
  faces drawable was refreshed. The delete path now calls `Renderer::update()` so every
  drawable rebuilds from the new topology.
- Hovering highlighted primitives in Off mode. The highlight is now restricted to
  Click (pick) mode; Rectangle/Lasso/Off only report the readout.
- Switching Off/Click/Rect/Lasso visually cleared the selection because entering Off
  restored the saved drawable style. Selection display now persists across tool
  changes and is restored only when the selection becomes empty (Clear Selection,
  deselecting everything, or a replace gesture that misses).
- `refresh_selection_display()` re-applies the scalar selection coloring if a Rendering
  panel restyle replaced it while the selection was retained.
- The lasso outline is closed while drawing (the closing segment from the last point to
  the first) so the region matches how the picker closes the polygon.
- `Esc` leaves an active selection tool without touching the selection; with no tool
  active it would reach `Viewer`'s close-on-Esc behavior, so Mapple asks for
  confirmation before closing the window.
- Validation: both backends build. Runtime re-checking of these three paths was not
  scripted (keyboard/mouse injection was unavailable while the desktop was under
  TeamViewer control).

### 2026-11-15 — Phase 1 clipping plane

- Added a "Clipping Plane" section to the Rendering panel: Enabled (fits the plane to
  the scene on enable), Show guide, Cross section, relative Thickness (log slider), and
  guide color. It drives `ClippingPlane::instance()`; the guide rendering,
  `Alt+C/V/X` shortcuts, and `Alt`+drag manipulation are `Viewer` behavior and were
  reused unchanged.
- `Alt` presses bypass an active selection tool, so the plane can be dragged while
  Rectangle/Lasso/Click is selected.
- Validation: target builds in `build-opengl-check` and `build-vulkan-check`. The
  section was not exercised through scripted clicks.

### 2026-11-15 — Phase 2 processing (first slice)

- Added `ui/process.cpp` and the Process menu. Point cloud: estimate normals, simplify.
  Surface mesh: smooth, simplify, remesh, fill holes, polygonize. Each opens a small
  modal parameter window on the active model; results and timings go to the Log panel.
- `Mapple-imgui` now links `easy3d::algo`.
- Commands run synchronously in the frame; the parameter window consumes scene input
  while open. After a command the renderer and the face/vertex normals are refreshed,
  and the hover/selection state is revalidated.
- Validation: both backends build; the Vulkan binary was launched with `bunny.ply`
  without startup errors. Running each command through the UI was not scripted.

### 2026-11-15 — Phase 2 reconstruction and primitive extraction

- Point cloud: Poisson reconstruction (octree depth, samples per node, optional trim
  by density and island area ratio) adds a new `<cloud>_poisson.ply` mesh and makes it
  active; primitives via RANSAC (plane/sphere/cylinder/cone/torus, detector parameters,
  selected-points-only) color the cloud with `v:color-segments`.
- Both require `v:normal`; the command warns and stops otherwise instead of failing
  inside the algorithm.
- RANSAC colors become the cloud's appearance: the saved selection style is dropped so
  a later selection change cannot repaint over the segmentation.
- Validation: both backends build. The commands were not exercised through scripted
  clicks.

### 2026-11-15 — Model info moved to the Models panel

- "Model Info" left the Rendering panel and now sits above the Models list, driven by
  the selected model. With several models loaded it describes the highlighted row
  instead of whatever model happened to be active in the rendering panel.

### 2026-11-15 — Phase 2 remaining surface-mesh algorithms

- Surface mesh menu gained fairing (area/curvature/curvature-variation criterion),
  a Subdivide submenu (Catmull-Clark, Loop, Sqrt3), triangulation, dual mesh,
  connected components, planar partition, and surface sampling. Sampling adds a new
  `<mesh>_sampled.ply` point cloud.
- Commands that need no parameters run directly from the menu through `run_process`;
  only fairing and sampling open a parameter window.
- Connected components and planar partition paint faces (`f:color_components`,
  `f:color_planar_partition`) and drop any saved selection style, like RANSAC, so a
  later selection refresh cannot overwrite the painting.
- Validation: both backends build; the OpenGL binary was launched with
  `general_closed.obj` without startup errors. The commands were not exercised through
  scripted clicks.

### 2026-11-15 — Phase 2 Delaunay reconstruction

- Point cloud menu gained Delaunay 2D (XY projection) and Delaunay 3D. The 2D result is
  a `SurfaceMesh` carrying the original z values; the 3D result is a tetrahedral
  `PolyMesh`. Both add a named model and refresh face/vertex normals before it is
  shown.
- Degenerate input (fewer points than the simplex needs, or a projection with no
  triangles / tetrahedra) logs a warning and adds no model.
- `PolyMesh` needs no viewer changes: the rendering panel treats its drawables like
  any other, while the model info and status bar show the name only.
- Validation: both backends build; the app was launched with the `bunny.bin` point
  cloud. `algo.point_cloud_delaunay_triangulation_2D` and `_3D` pass in the OpenGL
  tree; the menu commands were not exercised through scripted clicks.

### 2026-11-15 — Phase 2 parameterization and geodesics

- Surface mesh menu gained Parameterize... (LSCM or discrete harmonic with cotan or
  uniform weights) and Geodesic Distance.
- Parameterization writes `v:texcoord`, requires a triangle mesh with a boundary, and
  can preview the result with the default checkerboard texture; editing the faces'
  Color in the Rendering panel restores uniform coloring.
- Geodesic distance is seeded by the vertices of the selected faces, reuses the
  selection tools instead of picking random vertices as the Qt demo did, and clears
  the selection afterward so the `v:geodesic:distance` scalar field is visible.
- Validation: both backends build; the app was launched with `general_open.obj`.
  `algo.surface_mesh_geodesic` and `algo.surface_mesh_parameterization` pass in the
  OpenGL tree; the menu commands were not exercised through scripted clicks.

### 2026-11-15 — Phase 2 clipping, splitting, and slicing (CGAL enabled)

- Enabled CGAL (6.0.1, `CGAL_DIR=~/Documents/Dev/CGAL-6.0.1/lib/cmake/CGAL`) in both
  validated trees and linked `easy3d::algo_ext` into Mapple-imgui when that target
  exists; the commands are compiled out when CGAL is unavailable.
- Surface mesh menu gained Clip by Clipping Plane, Split by Clipping Plane, and Slice
  by Clipping Plane. They use the Rendering-panel clipping plane, so the cut is
  exactly what the guide shows.
- Clip keeps the negative side (`clip_volume = false`); split cuts the mesh into two
  components without removing either side; slice adds the cut polylines as a new graph
  model (`<mesh>_slice`), with closed loops sharing their first vertex.
- The commands warn (instead of failing silently) when the plane is disabled, the mesh
  is not triangulated, or the plane misses the mesh.
- Validation: `algo.surface_mesh_clip`, `algo.surface_mesh_split`, and
  `algo.surface_mesh_slice` pass with CGAL 6.0.1; both backends build and launch with
  `general_open.obj`. The linker warns that Homebrew's gmp/mpfr were built for a newer
  deployment target (environment-specific, not an error).
