# Mapple on Dear ImGui

This document tracks the migration of Mapple from Qt Widgets (`applications/Mapple`) to a
Dear ImGui application rendered through Easy3D's backend-neutral overlay path
(`applications/Mapple-imgui`). Qt/Mapple remains buildable until the new app covers the
daily-use workflow.

Status: 2026-11-15 — Phases 0–3 complete: the app builds and runs on both OpenGL and
Vulkan and covers panels, menus, log capture, drawable styling, picking readout,
point/face selection, the clipping plane, processing commands, snapshots, the camera
path with video recording, and settings/layout persistence. Phase 4 (retire Qt) is
pending.

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
  ui/process.cpp         Process/Property commands: parameter windows and the
                         algorithms they run
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
| Settings/layout | app-owned `AppSettings` (`settings.h`): panel visibility/sizes, view toggles, last directory and recent files in a per-user INI; `io.IniFilename` stays disabled because the panels are fixed windows |

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
- [x] Settings persistence, recent files: `AppSettings` (`settings.h/.cpp`) writes
      panel visibility and sizes, the pick-readout and log auto-scroll toggles, the
      video bitrate, the last-used directory and up to five recent files to a
      per-user INI (`~/Library/Application Support/Mapple-imgui/settings.ini`,
      `%APPDATA%/Mapple-imgui/settings.ini`, or `$XDG_CONFIG_HOME/Mapple-imgui/`).
      The app-owned Open/Save dialogs start in that directory and File > Open Recent
      lists the newest files (basename plus full-path tooltip, Clear Recent Files);
      missing files are pruned when the settings load.

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
      curvature), re-orient existing normals (`PointCloudNormals::reorient`, same
      neighbors parameter; warns instead of failing silently when the cloud has no
      normals), and simplify to a target point count
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
- [x] Long-running commands (Poisson reconstruction, point-cloud/surface simplification,
      remeshing, uniform sampling) run on a worker thread behind a modal progress
      window instead of blocking the frame. The percentage comes from `ProgressLogger`
      where the algorithm reports it; otherwise the bar is indeterminate. Cancellation
      is cooperative, so the Cancel button (and Esc) is offered only for the two
      commands whose algorithms poll `ProgressLogger::is_canceled()` (remeshing and
      sampling); the others finish on their own.

Phase 2 design notes:

- Quick commands run synchronously inside `draw_gui()`; the long ones hand the model to
  a `ProcessJob` worker thread and only a modal progress window is drawn until it
  finishes. While the job runs, the panels are skipped because the worker mutates the
  model's properties (reading them from the main thread would race), and the open
  `process_op_` already blocks scene input. The worker publishes progress through
  `ProgressClient::notify()` into an atomic that the window reads each frame; the bar is
  indeterminate for algorithms that report none. `ProgressClient::cancel()` sets a flag
  that `ProgressLogger::is_canceled()` polls from the other thread, which is why that
  flag is atomic. The job's new model (Poisson, sampling) is adopted with `add_model()`
  on the main thread when the thread joins; the viewer destructor requests cancellation
  and joins before tearing down. Every command logs its duration and the resulting
  element counts when it finalizes. The shared `ProcessParams` struct keeps the
  last-used parameters; targets that depend on the model (point/vertex counts) are
  derived when the window opens.
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

- [x] Camera path: a non-modal Camera Path window (keyframe count, Prev/Next
      navigation, playback, speed, frame rate, interpolation method, path visibility)
      plus Camera-menu commands to add (Alt+K), remove the last keyframe, clear the
      path, play/stop (Ctrl+K), show the path and keyframe cameras (Ctrl+T), and
      import/export `.kf` files. It drives the viewer's own `KeyFrameInterpolator`, so
      the viewer shortcuts and the HUD path overlay stay authoritative; the scene
      remains interactive while the window is open.
- [x] Video recording: Camera Path > Record Video... renders the interpolated path
      through `easy3d::video::VideoEncoder` (FFmpeg builds only; the button is
      disabled and labeled otherwise). Frames come from
      `KeyFrameInterpolator::interpolate()` at the path's frame rate, are rendered
      with `Viewer::capture(..., graphics::FrameReadback::color)` at the current
      view size (cropped to the encoder's multiple-of-8 requirement), and are
      scene-only: the capture skips the viewer HUD (`snapshot_hud_` is off while
      recording) and `pre_draw()` drops the app's UI overlays for the captured
      frame. `FrameReadback::color` also skips the picking depth/ID work that the
      encoder never reads, which is the dominant per-frame cost otherwise. One
      frame is rendered and encoded per drawn frame behind a modal progress window
      (Cancel/Esc keeps the frames written so far); the camera and HUD state are
      restored at the end, and a view resize aborts the recording with the error
      logged. The bitrate is adjustable in the Camera Path window.
- [x] Snapshot dialog (scale with the resulting pixel size shown, samples, current/
      white/transparent background, frustum expansion), matching the Qt
      `DialogSnapshot`'s capabilities. Saving reuses the native file dialog and
      `Viewer::snapshot(file, scale, samples, background, expand)`; png/jpg/bmp/tga
      extensions are validated before rendering. The output contains only the scene:
      the viewer already drops its HUD, and the dialog drops the app's UI overlays
      with `set_overlays({})` immediately before capturing (the viewer renders client
      overlays in snapshots on purpose, e.g. annotations). `pre_draw()` rebuilds them
      on the next frame, so the window is unaffected. The View menu item, the toolbar
      button and `S` all open it.
- [x] App settings and panel layout persistence: the Phase 0 settings/recent-files
      item is implemented (`AppSettings`, File > Open Recent), so Phase 3 is complete.

### Phase 4 — retire Qt

Verify the acceptance list, migrate remaining workflows, remove `applications/Mapple`
and the `Easy3D_ENABLE_QT` gate (or keep Qt for other tools, not Mapple).

The migration order agreed on 2026-11-15 is gaps first, retirement later: the Qt app
stays untouched until the missing capabilities exist in Mapple-imgui. The inspection
group (the four Property-menu commands) is migrated; the remaining groups are mesh
repair/orientation, property manipulation and the rendering panel's color-scheme
selector, small utilities, and view polish.

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

### 2026-11-15 — Phase 2 background commands with progress and cancellation

- Poisson reconstruction, point-cloud/surface simplification, remeshing, and uniform
  sampling now run on a worker thread behind a modal progress window instead of
  blocking the frame, so the window keeps repainting and can show elapsed time.
- The window shows `ProgressLogger`'s percentage where the algorithm reports one and
  an indeterminate sweeping bar otherwise. Remeshing and sampling are cancelable
  (Cancel button or Esc; closing the window cancels and joins); the other algorithms
  never poll the cancel flag and say so.
- While a job runs only the progress window is drawn: the models and rendering panels
  stay out because the worker mutates the model's properties, and `process_op_` already
  blocks scene input. The job's new model (Poisson mesh, sampled cloud) is adopted with
  `add_model()` on the main thread when the thread joins; the viewer destructor
  requests cancellation and joins before teardown.
- `easy3d/util/progress.cpp`: `internal::Progress::canceled_` is now
  `std::atomic<bool>` because the main thread sets it through
  `ProgressClient::cancel()` while the worker polls `ProgressLogger::is_canceled()`.
- Validation: library suites green in both check trees (OpenGL 116/116, Vulkan 150/150,
  serial; batch 83). A threaded scratch probe confirmed cancellation stops the remesh
  early (67,330 faces at 1% versus 169,608 on completion) and that sampling returns no
  cloud when canceled. Both backends build and launch with `general_open.obj`; the
  progress window, Cancel button and Esc path were not exercised through scripted
  clicks.

### 2026-11-15 — Point-cloud normal re-orientation

- Added Re-orient Normals... directly below Estimate Normals in Process > Point Cloud,
  matching the Qt app's `pointCloudReorientNormals()`.
- It runs `PointCloudNormals::reorient` (Hoppe et al. 1992, minimum spanning tree over
  the k-neighbor graph) with the same Neighbors parameter as estimation. Unlike the Qt
  dialog it warns with "(Process > Point Cloud > Estimate Normals)" when the cloud has
  no `v:normal` property instead of relying on the algorithm's error log.
- Validation: both backends build and launch with `bunny.bin`; the algorithm itself is
  covered by `algo.point_cloud_algorithms` (green in both check trees). The menu command
  was not exercised through scripted clicks.

### 2026-11-15 — Phase 3 snapshot dialog

- View > Snapshot..., the toolbar button, and `S` now open a modal ImGui dialog instead
  of going straight to the viewer's one-shot native-dialog snapshot. It offers the Qt
  `DialogSnapshot`'s settings: scale 0.1-8 with the resulting pixel size shown, samples
  (defaults to the window's), background (current color, white, transparent), and
  frustum expansion for a different output aspect ratio.
- Saving uses the native save dialog with the viewer's png/jpg/bmp/tga filters; the
  extension is validated before rendering and the path (or the failure) is logged.
- The scene-only capture needs one extra step: the panels, menus and dialog are client
  overlays, and the viewer's file snapshot renders client overlays on purpose
  (`opengl.annotations`/`vulkan.annotations`), while only the HUD is dropped. The
  dialog therefore calls `set_overlays({})` right before `snapshot()`, and `pre_draw()`
  re-submits the UI on the next frame. An earlier manual save had the UI baked into the
  image; this is the fix.
- Like the process window, the dialog blocks scene input, closes on Esc or Cancel, and
  process commands cannot start while it is open.
- Validation: both backends build and launch with `general_open.obj`.
  `Viewer::snapshot(file, scale, samples, background, expand)` is covered by the viewer
  suites (green in both check trees). A scratch probe on both backends drew a
  full-screen client overlay and took three file snapshots: with the overlay set the
  image contains it, after `set_overlays({})` it does not, and after two more frames it
  is back (`kept=1 cleared=0 restored=1`); the probe then removed itself.

### 2026-11-15 — Phase 3 camera path window

- The Camera menu now carries the camera-path commands: Camera Path... (a floating,
  non-modal window), Add Keyframe (Alt+K), Remove Last Keyframe, Clear Camera Path,
  Play Camera Path / Stop Playback (Ctrl+K), Show Camera Path (Ctrl+T), and Import/
  Export Keyframes (.kf). Copy/paste view and the projection choice stay above them.
- The window shows the keyframe count, a Prev/Next navigator, playback, interpolation
  speed, frame rate, interpolation method (spline interpolation or fitting), and the
  path visibility. It drives the viewer's own `KeyFrameInterpolator`; the existing
  viewer shortcuts therefore stay the single source of truth, and the scene remains
  interactive while the window is open (unlike the modal process/snapshot dialogs).
  Closing the window stops playback so the camera does not keep moving unattended;
  Ctrl+K still toggles it.
- Showing the path widens the scene radius over the keyframe positions (as Ctrl+T
  does) so the whole path and the keyframe cameras stay in the depth range, and
  hiding or clearing it restores the scene bounds. Import expands the radius when the
  path is already visible.
- Validation: both backends build and launch with `general_open.obj`; the
  `KeyFrameInterpolator` interpolation, manual playback and `.kf` round-trip it calls
  are covered by the viewer camera-path tests (green in both check trees). The menu
  and window were not exercised through scripted clicks.

### 2026-11-15 — Phase 3 video recording

- The Camera Path window gains Record Video... (disabled below two keyframes) and a
  bitrate slider (1-50 Mbit/s, default 8). Saving goes through the native save
  dialog (mp4/mov/avi or any extension FFmpeg recognizes); the video uses the
  window's current frame rate, so the clip plays at the configured interpolation
  speed.
- Recording does not step the camera in real time. It copies
  `KeyFrameInterpolator::interpolate()` and renders one frame per drawn frame
  between the UI build and the overlay submission in `pre_draw()`, so the modal
  progress window updates, Esc/Cancel work, and no other command can run while
  frames are written.
- Frames come from `Viewer::capture(samples, graphics::FrameReadback::color)`, i.e. the
  same view size as the window; a size cropped to multiples of 8 (the encoder's
  requirement) drops at most 7 rows or columns, keeping the view framing.
  `capture()`'s RGBA rows are top row first, matching the encoder. Frames are
  scene-only: `snapshot_hud_` is turned off for the recording (so neither backend
  composes the viewer HUD, including the camera-path overlay), and the app's client
  overlays are cleared just before each capture (the same `set_overlays({})`
  contract as the snapshot dialog; `pre_draw()` reinstalls the UI right after). The
  camera is restored when recording ends (success, cancel or failure), as is
  `snapshot_hud_`; a view resize mid-recording aborts it with an error rather than
  mixing frame sizes, and the frames written so far stay in the file.
- `FrameReadback::color` skips the picking output that the encoder never reads:
  Vulkan no longer copies IDs and depth (62 MB at 2880x1800) or allocates their
  host buffers per frame, and OpenGL no longer re-renders the scene once per
  drawable and scans the full depth buffer. Measured steady-state capture at
  2880x1800 with 4 samples on the `bunny.ply` + `bunny.kf` path: OpenGL Release
  21.8-23.1 -> 11.5-12.4 ms, Vulkan Release 5.3-6.1 -> 1.9-3.2 ms, Vulkan Debug
  (validation, `-O0`) 454-462 -> 140-146 ms; the color image is identical in both
  modes and the encoder still costs about 7-9 ms per frame. The 438-frame 60 fps
  recording that took 233 s in the Vulkan Debug build was dominated by the skipped
  readback; the app-level recording time was not re-measured because the dialog
  cannot be scripted.
- FFmpeg is optional: `applications/Mapple-imgui/CMakeLists.txt` links
  `easy3d::video` when `Easy3D_HAS_FFMPEG` is set (which also defines `HAS_FFMPEG`
  for the sources). Without it the button is disabled with a "requires FFmpeg"
  hint and the recorder compiles to no-ops.
- Validation: both backends build and launch with `general_open.obj`. A scratch
  probe linked against both build trees reproduced the recorder's exact sequence -
  `Viewer::OffScreen` at 645x481 (deliberately not multiples of 8), a three-keyframe
  path, per-frame `capture()`, top-left crop to 640x480, and
  `VideoEncoder::encode(..., PIX_FMT_RGBA_8888)` - and produced playable H.264
  files (61 frames at 30 fps) on OpenGL and Vulkan; an extracted frame is upright
  and scene-only. The probe then removed itself, and the app's menu/window flow was
  not exercised through scripted clicks.

### 2026-11-15 — Camera Path window layout

- Import... and Export... (the `.kf` keyframe files) now sit in the first row of the
  Camera Path window, above the keyframe count and editing controls, because loading
  or saving a path is the usual entry point when the window is opened. They had been
  at the bottom, below the recording controls.
- Record Video... and its "MP4, scene only" hint are now the last row, below the
  bitrate slider and the "needs at least two keyframes" hint that configure it.
- Validation: `applications/Mapple-imgui` builds and the app launches; the layout was
  not exercised through scripted clicks.

### 2026-11-15 — Phase 3 settings persistence and recent files

- `AppSettings` (settings.h/.cpp) stores the state that should outlive a session:
  panel visibility and sizes (left/right widths, log height), the pick-readout and
  log auto-scroll toggles, the video bitrate, the last-used directory and up to five
  recent files. It is a plain `key=value` file under the per-user configuration
  directory (`~/Library/Application Support/Mapple-imgui/settings.ini` on macOS,
  `%APPDATA%\Mapple-imgui\settings.ini` on Windows, `$XDG_CONFIG_HOME/Mapple-imgui/`
  or `~/.config` on Linux), written when the app exits and read at start-up.
  Malformed entries are skipped, missing keys keep their defaults, and recent files
  that no longer exist are dropped at load.
- File handling now belongs to the app: `MappleViewer` overrides `open()`/`save()`
  (both virtual in `ViewerBase`) so the dialogs start in the last-used directory and
  use Mapple's own filter list, which carries the aggregate "Supported formats" line
  like the Qt dialogs. Successful opens and saves feed the recent list, and File >
  Open Recent shows each entry as a numbered basename with the full path as a tooltip;
  a recent file that fails to load is dropped. `Viewer::open()`/`save()` are
  untouched, so the tutorials and other apps keep their behavior.
- Panel layout is persisted by the app rather than ImGui: `io.IniFilename` stays
  disabled because the panels are fixed windows laid out by `draw_panels()`, so there
  is no ImGui window state to save. The floating Camera Path window keeps its
  first-use default position and is not persisted yet.
- Validation: both trees build and launch. A scratch round-trip probe (linked against
  `easy3d_util`, removed afterwards) saved and reloaded every field, verified the
  newest-first recent order, and confirmed that a deleted recent file is pruned. A
  scripted run of the app itself (temporary exit timer, removed afterwards) wrote the
  file on a clean exit; a second run started from a hand-written file with custom
  sizes, visibility and bitrate and wrote the same values back, proving the
  constructor applies them, with a dead `recent.0` entry pruned. The native file
  dialogs and the Open Recent menu clicks were not scripted.

### 2026-11-15 — Rendering panel drawable order

- The per-model drawables of the Rendering panel now appear in a fixed order:
  faces, edges, borders, vertices, locks. The panel previously listed the
  renderer's three drawable collections in turn (triangles, then lines, then
  points), so the three loop passes are replaced by one pass over the collected
  drawables, stably sorted by a name rank. Names are matched by prefix so the
  polyhedral mesh's `faces:border`/`faces:interior` stay in the faces group and
  unknown drawables come last.
- Each drawable's "Visible" checkbox and "Color" swatch share one line; the
  type-specific controls (Smooth/Opacity for faces, Width for lines, Size for
  points) remain below. The "Back color" controls are shared by faces and points,
  since distinct back coloring is a `State` property of both; points drawables now
  enable it by default (batch 85 in docs/vulkan.md), so the checkbox starts checked.
- Validation: both trees build and the app launches; the row layout was not
  exercised through scripted clicks.

### 2026-11-15 — Panels stay visible during processing and recording

- A long-running background command used to take the whole UI off screen: `draw_gui()`
  returned after the progress window, so the Rendering, Models and Log panels vanished
  until the job finished. The menu, toolbar and the Rendering and Models panels now
  stay in place, drawn disabled so nothing can run a command against a model the
  worker owns. The status bar stays out (it reads vertex/face counts the worker is
  changing), and the Model Info section of the Models panel shows a "being processed"
  line instead of the live counts, bounds and properties.
- Video recording had the same early return, so the whole UI disappeared for the
  duration of a capture. The menu, toolbar and panels now stay laid out for the whole
  recording as well, drawn disabled because the recorder owns the scene: no control
  may change the model, its appearance or the camera while frames are written. The
  status bar stays out, as before.
- The Log panel remains fully usable in both cases: it is mutex-protected and never
  reads the model, so its Clear/Copy/Export buttons and scrolling keep working while
  a job runs or frames are captured.
- None of the UI reaches the output: the snapshot dialog and the video recorder drop
  the UI overlay commands before rendering their frame (`pre_draw()` captures between
  `draw_gui()` and the overlay submission), independently of whether the UI was drawn
  that frame.
- Validation: both trees build and the app launches. The disabled state during a
  running job or a recording was not exercised through scripted clicks.

### 2026-11-15 — Property menu (inspection commands)

- A `Property` top-level menu sits between Camera and Process, matching the Qt menu
  order, and exposes the four Qt inspection commands. Each is a `ProcessOp`, so it
  reuses the parameter window, the background-job/progress path and the Log feedback,
  and is disabled with the rest of the menu while the UI is busy.
- **Compute Height Field** writes `v:height_x/y/z` (positions), `e:height_x/y/z` (edge
  midpoints) and `f:height_x/y/z` (face centroids). Surface meshes additionally get
  refreshed `f:normal` and edge normals averaged from the adjacent faces (`e:normal`);
  polyhedral meshes get all three fields plus `f:normal`; point clouds get vertices
  only; graphs get vertices and edges. Unlike the Qt code, graphs are handled before
  the point-cloud branch: a `Graph` derives from `PointCloud`, so the Qt graph branch
  could never run.
- **Compute Curvatures...** carries the Qt dialog's two parameters (post-smoothing
  iterations 0-100, default 1; two-ring neighborhood, default on), runs
  `SurfaceMeshCurvature::analyze_tensor` and then writes `v:curv-min`, `v:curv-max`,
  `v:curv-mean`, `v:curv-gauss` and `v:curv-max_abs`. It is the one inspection command
  that runs in the background, since the tensor analysis is the slow part.
- **Color from Indexed Colors...** reads the Qt `.ic` format - a `v:<name>` or
  `f:<name>` header naming the index property, then `<index> <r> <g> <b>` lines with
  0-255 colors, ignoring blank lines and `#` comments - and writes
  `v:color_indexed`/`f:color_indexed`; indices without a color become black. The Qt
  implementation only handled point-cloud vertices and logged "not implemented yet"
  for meshes; the ImGui command also handles surface-mesh vertices and faces.
- **Topology Statistics** logs the Qt report: connected components (first 10) with
  their sphere/disc/cylinder/torus classification and F/V/E/B counts, border sizes
  and the isolated-vertex count for a surface mesh; vertex/edge counts plus the degree
  histogram for a graph.
- Scope note: the Qt commands only store properties; displaying them goes through the
  Qt rendering panel's color-scheme selector, which Mapple-imgui does not have yet.
  The new properties are therefore visible in Model Info, in the saved file and in
  the Log, not in the viewport, until that rendering-panel gap is migrated.
- Validation (2026-11-15, macOS): `Mapple-imgui` builds in `cmake-build-release`
  (Vulkan), `build-vulkan-check` and `build-opengl-check` (OpenGL); each binary
  launches with `resources/data/bunny.ply` and reports "Mapple UI ready", then was
  stopped. The ImGui window is not scriptable, so the menu commands themselves were
  not exercised; the algorithm calls they wrap are the ones covered by
  `tests/algo/surface_mesh_algorithms_test.cpp`.
