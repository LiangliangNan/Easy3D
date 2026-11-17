# Mapple on Dear ImGui

This document tracks the migration of Mapple from Qt Widgets to a Dear ImGui
application rendered through Easy3D's backend-neutral overlay path. The ImGui app is
now `applications/Mapple` (target `Mapple`); the Qt app it replaces is
`applications/Mapple-Qt` (target `Mapple-Qt`, OpenGL only, built with
`Easy3D_ENABLE_QT`). The Qt app remains buildable until a final capability pass
confirms nothing it offered is still needed.

Status: 2026-11-17 — Phases 0–3 complete: the app builds and runs on both OpenGL and
Vulkan and covers panels, menus, log capture, drawable styling, picking readout,
point/face selection, the clipping plane, processing commands, snapshots, the camera
path with video recording, settings/layout persistence, and the rendering effects
(ambient occlusion, eye-dome lighting, soft shadow, transparency). Phase 4 (retire
Qt): the ImGui app now carries the Mapple name; batch 127 completed the final
capability pass over the Qt dialogs and the interactive keyframe editing item, so
`Mapple-Qt` can be retired.

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
applications/Mapple/
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
| Camera state files | `Camera`/`ManipulatedCameraFrame` accessors, native dialogs, nlohmann JSON |
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
- [x] Models panel: flat model list, active model selection with the selected model's
      info directly below its name, delete, fit.
- [x] Rendering panel: background color, projection; the active model's drawables as a
      compact list (visibility, uniform color, name, current scheme) with one editor
      for the selected drawable (color scheme, scalar-field styles, texture image with
      optional stripes and repeat, lighting modes, imposters, vector field with scale,
      highlight range, smooth shading, opacity, back color).
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
      drawn in the Models panel directly below the clicked model's name so it always
      describes the highlighted row.
- [x] Point/picking readouts in the status bar: nearest visible primitive under the
      pointer (world coordinates, face or point index), outlined in Click mode only;
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

- Selection is stored as `v:select`/`f:select` booleans and displayed as a highlight
  mask over the drawable's own coloring (batch 122):
  `buffer::update_highlight_mask(model, drawable)` expands the property into one tint
  strength per rendered primitive (`setting::highlight_color` at 0.8), so a scalar
  field, colormap or texture stays visible while something is selected. Click/region
  changes only rebuild the mask. The selection display belongs to the selection, not to
  the tool: switching between Off/Click/Rect/Lasso keeps both the selection and the
  tint. Clearing the selection (Clear Selection, deselecting everything, or a
  replacement gesture that hits nothing) clears the mask, and the drawable's coloring
  was never touched.
- Deleting selected faces/points changes the topology, so the whole renderer is
  invalidated (`Renderer::update()`); otherwise edge/border/vertex drawables would keep
  drawing the deleted elements.
- The hover cue is an overlay, not a highlight mask (batch 124): in Click mode the
  hovered face itself is filled with a translucent depth-tested world-space overlay,
  so hovering never overwrites the selection tint (a Shift/Ctrl multi-selection keeps
  every selected face) and the whole polygonal face - not one of its triangles - is
  clearly indicated. The status-bar readout remains available in the other modes when
  Pick Readout is enabled; the cue belongs to Click only.
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
  the mesh and discards its properties, including `f:select`/`v:select`, so the
  refreshed selection display is empty and the drawable keeps its own coloring.
- Commands without parameters (subdivision, triangulation, dual mesh, connected
  components, planar partition, Delaunay 2D/3D, geodesic distance, clipping/splitting/
  slicing) execute straight from the menu item through `run_process`; only
  parameterized commands open a window. The painting commands replace the
  drawable's appearance with their color property; `finalize_process` then
  refreshes the selection tint over that new appearance, so a selection survives
  a command that only recolors the mesh (a topology rebuild drops its
  `f:select`/`v:select` with the other properties).
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
      import/export `.kf` files. It drives `MappleViewer`'s own
      `KeyFrameInterpolator`; the app's shortcuts and path overlay stay authoritative;
      the scene remains interactive while the window is open.
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
- [x] View state files: Camera > Save View State... / Restore View State... write and
      read a `.view` JSON file with the window size and position, the background color,
      the projection type, the clipping/near coefficients, the scene radius and center,
      the field of view, the camera position/orientation and pivot point, and the frame
      sensitivities (rotation, translation, wheel, zoom, zooms-on-pivot). Since batch
      97 the file also carries the Snapshot dialog's settings (scale, samples,
      background, expand) as a `snapshot` object, so restoring a figure's state
      reopens the dialog with the values that figure was rendered with; a file
      without the block keeps the dialog's current values. Since batch 98 the
      timestamp is the file's first field (`saved`), with the remaining fields in
      alphabetical order. A missing or mistyped key
      keeps the current value, a window position saved on a monitor that is no longer
      connected is ignored, and restoring stops camera-path playback.
      This replaces the Qt app's row-based `.view`/`.state` text format. Mapple's
      files are JSON: Mapple-Qt's old text files are not read, and Mapple-Qt still
      reads and writes only its own format.
- [x] Snapshot dialog (scale with the resulting pixel size shown, samples, current/
      white/transparent background, frustum expansion), matching the Qt
      `DialogSnapshot`'s capabilities. Saving reuses the native file dialog and
      `Viewer::snapshot(file, scale, samples, background, expand)`; png/jpg/bmp/tga
      extensions are validated before rendering. The output contains only the scene:
      the viewer already drops its HUD, and the dialog drops the app's UI overlays
      with `set_overlays({})` immediately before capturing (the viewer renders client
      overlays in snapshots on purpose, e.g. annotations). `pre_draw()` rebuilds them
      on the next frame, so the window is unaffected. A successful save also writes
      the viewer state to `<image>.view` next to the image through
      `save_view_state(path)`: the window size, camera, background and the dialog's
      settings all come back with Restore View State..., so a figure can be
      re-rendered after an algorithm change. The View menu item, the toolbar
      button and `S` all open it.
- [x] App settings and panel layout persistence: the Phase 0 settings/recent-files
      item is implemented (`AppSettings`, File > Open Recent), so Phase 3 is complete.

### Phase 4 — retire Qt

Verify the acceptance list, migrate remaining workflows, then decide the fate of
`applications/Mapple-Qt` and the `Easy3D_ENABLE_QT` gate (remove it, or keep it for
other Qt tools). Retirement is deferred until a final capability pass over the Qt
dialogs; `Mapple-Qt` stays in the tree meanwhile.

The migration order agreed on 2026-11-15 is gaps first, retirement later: the Qt app
stays untouched until the missing capabilities exist in the ImGui app. The inspection
group (the four Property-menu commands), the rendering panel's color-scheme selector,
the mesh repair/orientation group (Process > Surface Mesh > Repair, plus Point
Cloud > Normalize Normals), property manipulation (Property > Manipulate
Properties...), and the small utilities (Edit menu with model manipulation,
tetrahedralization, creating a mesh from text, polyhedral boundary extraction, and
ambient occlusion) are migrated, as are the rendering effects (ambient occlusion,
eye-dome lighting, soft shadow, depth-peeling transparency) and camera state files
(Camera > Save/Restore View State...). The walk-through animation is migrated too
(Camera Path window modes: free, walking and orbit; see the 2026-11-17 changelog
entry). The Models panel's Qt extras (multi-select Duplicate/Merge/Decompose/Delete,
Auto focus, Selected only, Apply to all and in-place rename) are migrated too (see the
2026-11-17 "Models panel: selection and the remaining model commands" entry). The
load-translation options are migrated as File > Loading option (session-only, see the
2026-11-17 "File > Loading option" entry), and the status bar and info block now cover
graph and polyhedral mesh models (see the 2026-11-17 "Status-bar and info-block
graph/polyhedral readout" entry). Every capability from the Qt capability pass is
migrated; retirement can proceed whenever it is wanted.

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
- **Textures**: the overlay supports the font atlas plus the icons registered through
  `ImGuiLayer::icon()`. `<resource directory>/icons/<name>.png` is decoded once into a
  `graphics::Image` (four channels, top row first, linear filtering) and the texture ID
  is the image pointer, as for the atlas; commands keep the image alive through their
  shared pointer. Icons are drawn at the font size, so the 32x32 files stay sharp on a
  2x framebuffer and are minified on a 1x one. Toolbar buttons use the icon with the
  label as a tooltip, and fall back to a text button when the file cannot be read.
  Popup-menu rows draw the icon in a leading column (`icon_menu_item()`), since
  `MenuItem()` cannot show an image.
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
- Changes confined to `applications/Mapple` only need the Mapple target tested;
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
  highlights the primitive through the highlight mask (`buffer::update_highlight_mask`,
  `gl_PrimitiveID` on both backends);
  Rectangle/Lasso/Off leave the scene untinted. Disable the readout with
  Select > Pick Readout.
- Select menu and toolbar toggles for Click/Rect/Lasso; Shift adds, Ctrl removes,
  plain gestures replace; Esc exits. `Ctrl+A`, `Ctrl+I`, Clear Selection, and
  Delete Selected (Del, native confirmation) operate on the active model's points or
  faces. Switching modes keeps the selection and its colors; an empty selection
  restores the drawable's own appearance.
- Selection display is a highlight mask over `v:select`/`f:select` (batch 122); the
  drawable keeps its own coloring and the mask is cleared when the selection becomes
  empty. Gestures are queued from input callbacks and executed in
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
  that no longer exist are dropped at load. Rendering parameters are not stored in
  this INI: since 2026-11-16 they live in `easy3d::setting` and are written to
  `rendering.json` in the same directory through `setting::save`/`setting::load`.
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
  *Removed 2026-11-16 (gap group 5): the command was unused, and Compute Ambient
  Occlusion took its place in the Property menu.*
- **Topology Statistics** logs the Qt report: connected components (first 10) with
  their sphere/disc/cylinder/torus classification and F/V/E/B counts, border sizes
  and the isolated-vertex count for a surface mesh; vertex/edge counts plus the degree
  histogram for a graph.
- Scope note: the Qt commands only store properties; displaying them goes through the
  rendering panel's color-scheme selector, which is migrated in the next entry. Until
  then the new properties were visible in Model Info, in the saved file and in the
  Log, not in the viewport.
- Validation (2026-11-15, macOS): `Mapple-imgui` builds in `cmake-build-release`
  (Vulkan), `build-vulkan-check` and `build-opengl-check` (OpenGL); each binary
  launches with `resources/data/bunny.ply` and reports "Mapple UI ready", then was
  stopped. The ImGui window is not scriptable, so the menu commands themselves were
  not exercised; the algorithm calls they wrap are the ones covered by
  `tests/algo/surface_mesh_algorithms_test.cpp`.

### 2026-11-15 — Rendering panel color schemes

- Every drawable in the Rendering panel has a "Color scheme" combo listing what its
  shader can consume, taken from the drawable's own model: "Uniform color", the
  `:color` properties, texture coordinates (`:texcoord`), and numeric scalar fields
  labelled "scalar - <name>". Triangles are offered the face properties, lines the
  edge properties, and all drawables the vertex properties; surface-mesh triangles
  also get the halfedge texture coordinates. `Graph` is tested before `PointCloud`,
  which it derives from. Vector properties (normals) and bool selection properties
  are not offered: the former are not scalars and the selection has its own display.
- Selecting a scalar field reveals the colormap controls: the eight bundled colormaps
  (`default`, `french`, `rainbow`, `blue_red`, `blue_white`, `blue_yellow`,
  `black_white`, plus the generated `random`), discrete coloring with 2-64 stripes,
  and value clamping with lower/upper percentiles kept below 100 together. Textures
  come from `TextureManager`, so identical requests share one texture; a colormap
  file that is missing falls back to the State's default scalar texture.
- The chosen style (colormap, discrete/stripes, clamping) is remembered per model and
  drawable name, so it survives frames and renderer updates that recreate drawables;
  the coloring itself stays in the drawable's State. Deleting a model drops its
  entries, like the selection styles.
- This closes the previous entry's scope note: height fields, curvatures and indexed
  colors are now viewable, and the combo also presents the `f:chart` segmentation
  coloring that the renderer applies automatically when a model has one (e.g.
  `bunny.ply` starts with the faces already scalar-colored).
- While a background command owns the model the combo is replaced by an "unavailable
  while processing" line, since the worker can add or remove properties concurrently.
- Divergences from Qt: Qt spread the same controls over six widgets (scheme, style,
  discrete, stripes, clamp, two clamp percentages) and offered `f:select`/`e:select`
  bool fields; the texture-file picker and texture repeat controls are still
  unmigrated, so textured schemes use the checkerboard texture.
- Validation (2026-11-15, macOS): builds in `cmake-build-release` (Vulkan),
  `build-vulkan-check` and `build-opengl-check` (OpenGL). All three launch with
  `bunny.ply` (surface mesh, already `f:chart` colored) and report "Mapple UI ready";
  the Vulkan release build additionally launched `graph.ply` (graph), `sphere.plm`
  (polyhedral mesh) and `polyhedron.bin` (point cloud), covering every model branch
  of the scheme list.
  Because the ImGui window is not scriptable, a temporary in-panel probe (removed
  afterwards) applied all eight colormaps - including the discrete variants - to
  bunny's `f:chart` and finished with a 20% clamp on both backends; `buffer.cpp`
  logged "scalar field range [0, 1000] clamped (20%, 20%) to [110, 737]", proving
  the style reached the texture-coordinate rebuild. The combo clicks themselves were
  not exercised.

### 2026-11-16 — Mesh repair and orientation (Phase 4 gap group 3)

- Process > Surface Mesh gains a Repair submenu with the Qt repair commands: Repair
  Polygon Soup, Orient and Stitch Polygon Soup, Stitch with/without Reorientation,
  Orient Outward, Reverse Orientation, Remove Isolated Vertices, Remove Duplicate and
  Folding Faces, Detect Self-Intersections, and Remesh Self-Intersections. Point Cloud
  gains Normalize Normals next to Re-orient Normals. The commands run immediately (no
  parameter window) and report to the Log panel.
- The heavy ones run on the existing worker thread with the indeterminate progress
  window: the two soup commands, both stitches, duplicate/folding removal, and
  self-intersection detection/remeshing. Reverse Orientation, Remove Isolated
  Vertices and Orient Outward stay on the main thread; Orient Outward validates
  "closed triangle mesh" before paying for the CGAL round trip, and Qt ran all of
  these synchronously.
- Repair/orient/stitch and Orient Outward rebuild the mesh through CGAL
  (`SurfaceMesh::clear()`), so every property - including the selection - is
  discarded. After the join the viewer drops the stored selection style and skips the
  selection display, instead of restoring a style whose `f:select` no longer exists
  (which would warn and fall back to uniform). Detect Self-Intersections instead
  marks both faces of every pair in `f:select` and lets the normal selection display
  show them; it wipes any previous selection first, as in Qt.
- CGAL gating matches Qt: repair, orient-and-stitch, remove duplicate/folding faces,
  detect and remesh log the "requires CGAL support" build hint when the app is built
  without it; both stitches and orient-and-stitch fall back to
  `SurfaceMeshStitching` with Qt's "install CGAL..." warning.
- Worker-thread commands now catch exceptions (CGAL preconditions throw) and log
  "failed" instead of letting the exception escape the thread and terminate the app;
  the main thread still finalizes, with a "stopped early" warning if it happened.
- Fixed: a color scheme could not return to a scalar field after "Uniform color".
  `apply_scalar_style` re-applied the *drawable's* property name, which uniform
  coloring resets to `uniform color`, so the buffer found no such property and fell
  back to uniform. The style now carries the chosen scheme's location and property;
  the colormap/clamp controls keep using the drawable's current property.
- Validation (2026-11-16, macOS): all three trees build; `cmake-build-release`
  (OpenGL on this machine), `build-vulkan-check` (Vulkan) and `build-opengl-check`
  launch and report "Mapple UI ready". A temporary probe (removed afterwards) called
  `run_process` with the same operations the menu items use, on files chosen for each
  path: `repair/self_intersection/pig.off` found "3 pairs of faces intersect" and
  remeshed 891 -> 899 faces (then 0 duplicate/folding faces); `blobby-shuffled.off`
  repaired 8844 -> 2027 vertices over 4050 faces, then oriented/stitched it and ran
  both stitches (0 border pairs after the repair) and isolated-vertex removal;
  `partial_overlap.off` removed 1 overlapping face (2 -> 1); `sphere.obj` (closed
  triangle mesh) reported "face orientations corrected to point outward" and then
  "face orientations reversed"; `fountain/pointcloud.ply` normalized 479187 normals.
  The duplicate/self-intersection probe was repeated on the Vulkan build and the soup
  probe on the OpenGL check build. The menu clicks themselves were not scripted.
  The non-CGAL `#else` branches were syntax-checked with the app's own compile flags
  minus `-DHAS_CGAL`. For the color-scheme fix, a probe switched bunny's faces to
  uniform and then back to `f:chart` on both backends: the drawable stayed
  `SCALAR_FIELD`, `buffer.cpp` logged
  "scalar field range [0, 1000] clamped (5%, 5%) to [11, 917]", and no
  "not found" warning appeared.

### 2026-11-16 — Property manipulation (Phase 4 gap group 4)

- Property > Manipulate Properties... opens a non-modal window on the whole model
  list: a model combo, vertex/face/edge/halfedge sections with property counts and
  types, and a toggle that reveals the properties the editor normally hides (mesh
  bookkeeping such as `v:locked`, and the selection display). Right-clicking a
  property offers Remove, Rename..., Convert to (the numeric types), and Generate
  color property. The window sizes to its content on first use, so a model with few
  properties gets a short window. The scene stays interactive while it is open.
- The four operations fix the defects of the Qt dialog instead of reproducing them:
  - Conversion now actually changes the data type. A typed conversion moves the
    values to a uniquely named temporary property (checked through the container's
    type query), removes the old one and gives the temporary the original name, so a
    pre-existing `<name><name>` property can no longer make the conversion fail
    silently. The menu offers only convertible numeric types; Qt listed vec2/vec3,
    which it could not convert.
  - Rename validates the destination first (`PropertyContainer::rename` itself
    replaces the name even when another property already has it) and rebuilds the
    name from the location prefix, so renaming `f:foo` on vertices yields `v:foo`
    instead of `v:f:foo`. Renaming a property to its own name just closes the dialog.
  - Drawables and the appearance saved by the selection display follow a rename or
    conversion; when the property a drawable displays is removed, the drawable keeps
    a valid uniform color instead of the old "property not found" fallback. Removing
    `v:select`/`f:select` also restores the style saved before the selection display.
  - A generated color property is shown immediately: the drawable that draws that
    location switches to it (or, while a selection is shown, it becomes the style
    restored when the selection ends), which is why generated colors are now visible
    without a trip to the Rendering panel. Names follow the Qt scheme
    (`v:color-<name>`, `f:color-<name>`); int/unsigned int/std::size_t/bool/unsigned
    char fields use one random color per segment and float fields the bundled rainbow
    colormap. The colormap sample is clamped to the last column (Qt read the first
    pixel of the following row at the maximum) and a constant float field is refused
    with a message, instead of producing a meaningless colormap.
  - `v:select`/`f:select` are listed only as internal properties and are not offered
    for conversion; both remain removable, which restores the saved selection style.
- Validation (2026-11-16, macOS): all three trees build (`cmake-build-release`,
  `build-vulkan-check`, `build-opengl-check`). A temporary probe (removed afterwards)
  called the same helpers the context menu calls and reported 26 passing checks on
  `bunny.ply` (four shared prefix checks plus: face int property converted to float
  with values preserved, rename, duplicate-destination detection, generated
  `f:color-probe-renamed` displayed on the faces drawable, drawable follows the
  renamed `f:colors`, uniform fallback after removal, random colors from an int
  segmentation, constant float field refused, cleanup) and 14 on
  `fountain/pointcloud.ply` (the four prefix checks plus: vertex segmentation colors
  displayed on the vertices drawable, int -> unsigned int, rename, uniform fallback).
  A second short probe put a float `f:color-chart` in the way before generating colors
  from bunny's `f:chart`: the colors were created as `f:color-chart-1` (vec3), and
  regenerating reused `f:color-chart`.
  The window was also forced open with internal properties shown for the duration of
  a run: no errors or assertions, and "Mapple UI ready" appears in the log. The
  context-menu clicks themselves were not scripted. Leaving changes uncommitted per
  the standing rule.

### 2026-11-16 — Small utilities (Phase 4 gap group 5)

- Edit menu, between Property and Process: Add Gaussian Noise... (point clouds and
  meshes; sigma defaults to 0.001, with a button that sets the Qt suggestion of 1% of
  the bounding-sphere radius), Translational Recenter, Apply Manipulated
  Transformation, and Give Up Manipulated Transformation.
- Every model now gets a `Manipulator` when it is added, so a model can be moved and
  rotated before its coordinates are changed. Alt+left-drag rotates and Alt+right-drag
  translates the current model; while the clipping plane is enabled Alt keeps
  belonging to the plane. The manipulation frame is drawn while Alt is held or while
  the model's transform is not the identity, so a pending transform stays visible.
  Apply bakes the matrix into the points and re-derives the mesh or point-cloud
  normals; Give Up resets the current model's manipulator; Translational Recenter
  subtracts the first model's bounding-box center from every model, resets their
  manipulators and fits the screen.
- Process > Surface Mesh > Tetrahedralization runs `SurfaceMeshTetrehedralization` in
  the background and adds `<base>_tetrahedralization.plm`, warning when the mesh is
  not a closed triangle mesh. Create from Text... drives `TextMesher` with the Qt
  defaults (text "Easy3D", 45 pt, extrusion 10, 4 bezier steps, collision-free) and a
  font picker; it is the one command that needs no current model, and when the text
  equals the current mesh's name it rebuilds that mesh in place instead of adding a
  second copy. Process > Polyhedral Mesh > Extract Boundary writes the boundary as
  `<base>_boundary.ply` through `SurfaceMeshBuilder`.
- Property > Compute Ambient Occlusion... replaces the unused Color from Indexed
  Colors... . It bakes the scalar `v:ao` and the vertex colors `v:ao_color` in the
  background (radius ratio, rays per vertex, bias ratio) on triangle meshes and shows
  `v:ao_color` on the faces drawable immediately, instead of leaving the result to be
  found in the Rendering panel. `v:ao_color` is listed by the Rendering panel's color
  scheme combo (which recognizes `:color` and `_color` names).
- Validation (2026-11-16, macOS): all three trees build (`cmake-build-release`,
  `build-vulkan-check`, `build-opengl-check`) and launch with "Mapple UI ready". A
  temporary probe (removed afterwards) called the algorithms the dialogs call and
  passed 23 checks on `sphere.obj`, `bunny.bin`, `general_closed.obj`, `sphere.plm`
  and the generated text mesh: noise moves mesh vertices and cloud points; adding a
  model attaches a manipulator; rotate/translate gestures change its matrix; Apply
  bakes exactly `matrix * p` into every vertex and resets the manipulator; Give Up
  leaves the points unchanged; recentering zeroes the first model's bounding-box
  center; tetrahedralization adds the PolyMesh; text creation without a current model
  adds a model and a rebuild regenerates the same one; boundary extraction produces
  80 faces/42 vertices from `sphere.plm`; AO writes `v:ao`/`v:ao_color` and colors the
  faces drawable. Menu and dialog clicks were not scripted. The probe closed the window
  before its first frame, which aborts during teardown; with the window kept open the
  run exits cleanly, so this is an artifact of that harness, not of the commands.
  Leaving changes uncommitted per the standing rule.

### 2026-11-16 — Rendering effects (view polish)

- The Rendering panel has an Effects section (collapsed by default) with the four
  full-frame effects the Qt app's Global Setting panel exposed: ambient occlusion
  (radius ratio 0.01–0.1, intensity 0–1, bias 0.001–0.05), eye-dome lighting, soft
  shadow (Poisson 25x25 / 32x64 / 64x128 / 100x100 and regular 49x225 patterns, light
  distance 3–50, softness 0–1, darkness 0–1), and depth-peeling transparency.
- The effects are mutually exclusive, as in Qt: enabling one turns the others off.
  `MappleViewer::draw()` selects the active effect and wraps the scene render in its
  `begin()/end()` pair (OpenGL composites in `end()`, Vulkan publishes frame params
  that the next `Viewer::draw()` consumes). Soft shadow refreshes its virtual
  background color from the viewer background each frame; transparency draws the
  surfaces itself only when there are visible triangle surfaces, and otherwise
  draws the plain scene (without falling through to the other effects' objects,
  which are null - that fall-through crashed the first frame of a restored
  transparency session with no model loaded).
- The effect switches and parameters are not app state: they are the `effect_*`
  variables of `easy3d::setting`, edited in the panel and pushed into the live effect
  when it is enabled or a control changes. They are persisted with the library's own
  JSON writer/reader (`setting::save(std::ostream&)` / `setting::load(std::istream&)`)
  to `rendering.json` next to `settings.ini`, so every rendering setting the library
  knows round-trips without the app keeping a copy; the restored switch re-creates
  the effect on the next launch. The app's INI file keeps only UI state.
- Validation (2026-11-16, macOS): a temporary probe (removed afterwards) loaded the
  bunny, applied non-default AO and shadow parameters, cycled AO → EDL → shadow →
  transparency → off, and passed 17/17 checks on both `build-opengl-check` and
  `build-vulkan-check`: each toggle leaves only the requested effect on, the AO and
  shadow parameters reach the live effects, and every effect frame differs from the
  plain frame. The shadow frame shows the soft ground shadow on both backends; the
  OpenGL penumbra on the white background is low-contrast, so its pixel delta is
  small. The Mapple-imgui target compiles and links in all three trees and launches
  with "Mapple UI ready"; UI clicks were not scripted. Persistence was checked from
  the CLI: a probe round-tripped `setting::save(std::ostream&)`/`load(std::istream&)`
  through a JSON file, a seeded `rendering.json` was restored at launch (the log
  shows "rendering settings loaded from ..." and "ambient occlusion enabled"), and a
  graceful Alt+F4 exit wrote it back ("rendering settings saved to ..."), so the
  effect state survives a restart. With transparency restored and no model loaded,
  the fixed builds of both backends stay up and exit cleanly.

### 2026-11-16 — Mapple promoted; Qt app becomes Mapple-Qt

- The ImGui app took over the Mapple name: `applications/Mapple-imgui` became
  `applications/Mapple` (CMake target and binary `Mapple`, app title "Mapple -
  OpenGL"/"Mapple - Vulkan"), and the Qt app became `applications/Mapple-Qt`
  (target `Mapple-Qt`, still OpenGL only and still behind `Easy3D_ENABLE_QT`).
  The rename was done with `git mv`, so both histories follow the directories.
- The per-user settings directory moved from `Mapple-imgui` to `Mapple`
  (`~/Library/Application Support/Mapple` on macOS, `%APPDATA%\Mapple` on Windows,
  `$XDG_CONFIG_HOME/Mapple` on Linux). On the first run as Mapple, `settings.ini`
  and `rendering.json` are copied over once from the old directory when they are
  missing, so recent files, layout and rendering state survive the rename; the log
  is now `<build>/bin/Mapple.log`.
- Validation (2026-11-16, macOS): all three trees reconfigured to pick up the new
  targets and fully rebuilt; the Qt bundle is now `Mapple-Qt.app` (release tree, Qt
  enabled). OpenGL `build-opengl-check` 116/116 and Vulkan `build-vulkan-check`
  150/150, green, no flakes. The Mapple binary launches and logs "Mapple UI ready"
  with the new `Mapple.log` and reads `~/Library/Application Support/Mapple`;
  `easy3d/util/version.h` is unchanged.

### 2026-11-16 — Camera state files (Phase 4 gap)

- Camera > Save View State... / Restore View State... (below Copy/Paste View)
  write and read a `.view` file holding the window size and position, the
  background color, the projection type, the clipping/near coefficients, the
  scene radius and center, the field of view, the camera position/orientation
  and pivot point, and the frame sensitivities (rotation, translation, wheel,
  zoom, zooms-on-pivot). The file is JSON (nlohmann, same library as
  `rendering.json`): a missing or mistyped key keeps the current value, so a
  file from another Mapple version restores the parameters it has.
- Restoring resizes the window first (the camera aspect follows), places it only
  when the saved position lies on a connected monitor's work area, stops
  camera-path playback (which would otherwise overwrite the restored camera on
  the next frame), and remembers the file's directory for the next dialog.
  Saving starts from `<model>.view` (or the last-used directory when no model is
  loaded).
- This is a capability migration, not a format migration: the Qt app's
  row-based `.view`/`.state` text files are not read, and Mapple-Qt still reads
  only its own format.
- Validation (2026-11-16, macOS): `Mapple` builds in `build-opengl-check` and
  `build-vulkan-check` and launches with "Mapple UI ready" on both backends.
  The library is unchanged, so the suites were not rerun; the save/restore
  dialogs were not scripted. `easy3d/util/version.h` is unchanged.

### 2026-11-16 — Rendering panel: drawable list and one editor

- The Rendering panel no longer repeats a full styling block for every drawable. The
  scene-wide controls stay at the top: background swatch, projection radios, and the
  Effects and Clipping Plane sections collapsed to one header each. The active
  model's drawables are then listed one line per drawable: visibility checkbox,
  uniform color swatch, name, and the current color scheme right-aligned. The line
  is a selectable spanning the row; the checkbox and the swatch take the clicks they
  cover. Hidden drawables are drawn dimmed.
- Below the list, a single editor shows the selected drawable: the color-scheme combo
  with its scalar-field controls (colormap, discrete/stripes, clamp range), then the
  type-specific controls (smooth shading, opacity and back color for triangles, width
  for lines, size and back color for points). The styling logic is unchanged; only
  the layout moved.
- The selection is stored by drawable name, the key the scalar styles already use,
  so it survives renderer updates that recreate the drawables. Picking a color in the
  row's swatch is an explicit request for uniform coloring; the editor's scheme combo
  stays the place to switch to a property or a scalar field. Previously every
  drawable had its own color edit sitting next to its scheme combo, so picking a
  color there silently reset a scalar coloring.
- The Effects and Clipping Plane headers name their state ("Effects (shadow)",
  "Clipping Plane (on)") and carry a stable id, so the title changes without losing
  the expanded state. The notes they used to print (one effect at a time, where the
  face opacity lives, the Alt-drag hint) moved into header/checkbox tooltips.
- Validation (2026-11-16, macOS): both check trees build. The panel was checked on
  OpenGL with a point cloud and a surface mesh: the five drawables of the mesh
  listed their visibility, color, name and scheme, and the selected row's editor sat
  below. The Vulkan tree launched ("Mapple UI ready (Vulkan)"); its window opened
  behind the OpenGL instance, so the panel was not re-checked there. The library is
  unchanged, so the suites were not rerun; no clicks were scripted.

### 2026-11-16 — Rendering editor: lighting modes, imposters, highlight range

- The drawable editor now opens with a `Lighting` combo that names the three modes the
  library's two flags encode: Unlit (`lighting()` off), Front (the default), and Front
  and back (`lighting_two_sides()` on). The `Back color` checkbox and its swatch stay
  disabled unless the drawable is two-sided, because the back color only takes effect
  then.
- Lines gained an `Imposter` combo (Plain, Cylinder, Cone) and points one (Plain,
  Sphere, Surfel), each next to the width or size control it interacts with. Points
  keep the width/size slider, whose tooltip already said it is the impostor's
  diameter for lines.
- A `Highlight range` block (checkbox plus From/To drag fields) tints a range of
  rendered primitives. Polygons are triangulated, so face indices count triangles;
  the range is stored per model and drawable name and survives renderer updates. While
  a configured range is on, the hover picking aid in Click mode leaves that drawable's
  highlight alone instead of overwriting it on every hover; enabling a range on the
  hovered drawable also clears the stale hover range.
- Still missing from the user's list: discrete textured shading with a stripe count
  and the vector-field display, which has no renderer/State API yet.
- Validation (2026-11-16, macOS): both check trees build. The OpenGL instance was
  checked with `bunny.ply` (Lighting "Front and back", Back color, Highlight range) and
  a point cloud (Imposter "Plain", Size); the Vulkan instance was captured by window id
  because CLion held focus, and its panel matched. The library is unchanged, so the
  suites were not rerun; no clicks were scripted.

### 2026-11-16 — Rendering editor: textures and vector fields

- Drawables colored by texture coordinates now get a `Texture` block: the image in
  use, a `Choose...` button (native dialog, starting in `resources/textures/`), a
  `Discrete` checkbox that quantizes the image into `Stripes` bands, and `Repeat` and
  `Fraction` drag fields. Picking a texture scheme applies the remembered image
  instead of resetting to the default checkerboard, and a drawable that already has a
  file texture shows its name.
- Every drawable gets a `Vector field` combo with the vec3 properties its primitive
  type samples (vertices for points, edges for lines, faces for triangles) and a
  `Scale` drag. The field is drawn by a `vector - <field>` lines drawable, which is
  shared by all drawables showing that field and stays out of the drawable lists
  (it is a rendering aid, not a drawable of the model); `Disabled` hides it. Graph
  models do not offer vector fields because the library's buffer update has no
  variant for them.
- Validation (2026-11-16, macOS): both check trees build and launch. Two temporary
  env-gated probes (removed before the final build) confirmed on OpenGL that
  `f:normal` produces visible arrows and the texture quantizes; the final builds on
  OpenGL and Vulkan show the new blocks with the texture scheme of `room.obj`. The
  library is unchanged, so the suites were not rerun; no clicks were scripted.

### 2026-11-16 — Rendering panel: controls appear only when they can take effect

- The drawable editor hides controls that cannot change the result instead of showing
  them disabled: `Smooth shading` only while the drawable is lit, `Back color` only
  for two-sided lighting (and only with lighting, since an unlit pass shades neither
  side), `Opacity` only while the Transparency effect is on (the only pass that
  applies alpha; the Vulkan renderer rejects an opacity other than 1 without it), and
  the highlight From/To fields only while the range is enabled.
- The Effects section shows the AO radius/intensity/bias fields only while ambient
  occlusion is on and the shadow pattern/light distance/softness/darkness fields only
  while the shadow is on. The Clipping Plane section shows its guide, cross-section
  and thickness controls only while the plane is enabled (thickness only with a cross
  section) and the guide color only while the guide is visible.
- The `vector - <field>` lines drawables the editor creates to draw field arrows are
  rendering aids of the field selection, not drawables of the model: they are left out
  of the Rendering panel's drawable list and the Models tree, whose groups list only
  the model's own drawables. The arrows stay visible and are still controlled by the
  `Vector field` combo.
- Validation (2026-11-16, macOS): both check trees build and launch ("Mapple UI ready
  (OpenGL/Vulkan)"). The gated panel was checked on OpenGL with `room.obj` (no Back
  color, Opacity or highlight fields when inapplicable) and on Vulkan with `bunny.ply`
  (two-sided lighting shows Back color while Opacity and the highlight fields stay
  hidden). A temporary env-gated probe (removed afterwards, together with its
  `<cstdlib>` include) selected `f:normal` at startup: the capture showed the arrows
  drawn while neither list contained a "vector - f:normal" row, and the probe-free
  builds list faces, edges, borders, vertices and locks only. The library is unchanged,
  so the suites were not rerun; no clicks were scripted.

### 2026-11-16 — Snapshots keep their view state

- A successful snapshot writes the viewer state to `<image>.view` next to the image
  through the `save_view_state(path)` overload (Camera > Save View State... shares the
  same writer). Restoring it brings back the window size, camera and background, so
  after an algorithm change a paper figure can be re-rendered from the previous view.
- The state file also records the Snapshot dialog's own settings under a `snapshot`
  object — scale, samples, background choice and frustum expansion — and restore
  applies them with the same limits the dialog enforces. A file without the block (or
  without one of its keys) keeps the dialog's current values. Opening the dialog after
  a restore keeps the restored sample count instead of re-seeding it from the window.
- The save block in `ui/snapshot.cpp` relies on `save_view_state`'s own logging (the
  duplicate "view state saved" line is gone) and no longer opens a throwaway ofstream
  as a writability test, so a failed write leaves no empty file behind.
- Validation (2026-11-16, macOS): both check trees build and launch. Temporary probes
  (removed afterwards; the only `getenv` left in the app is the pre-existing
  `XDG_CONFIG_HOME` one) saved a state at 1200x800 with the camera at (3.7, -2.1, 84),
  scale 2.5, samples 7, the white background and no frustum expansion; a fresh process
  restored it and logged exactly those values with the resized window, and the file
  held the `snapshot` block. No clicks were scripted; the library is unchanged, so the
  suites were not rerun.

### 2026-11-16 — macOS restore dialog and the `.view` header

- Restore View State... no longer greys out every file on macOS. The vendored
  portable-file-dialogs backend passed the filter to AppleScript as
  `choose file ... of type {"view"}`, and `choose file` reads a four-character string
  as a classic file type code, so the pattern matched nothing (`.jpeg` behaved the
  same; three-letter and five-character extensions worked). Four-character extensions
  are now left out of the `of type` list, and a filter with no expressible extension
  opens an unfiltered panel whose selection the caller validates.
- The `.view` file writes `saved` as its first field (through
  `nlohmann::ordered_json`); the other fields keep their alphabetical order, so a
  snapshot's state file still starts with the timestamp for a quick check of its age.
- Validation (2026-11-16, macOS): probes linked against both check trees' rebuilt
  `libeasy3d_util` opened the dialog with the app's filters and returned the `.view`
  path on both backends; a temporary init() probe (removed) confirmed the field order.
  Both binaries were rebuilt and relaunched afterwards. No clicks were scripted; the
  suites were run for the library change (OpenGL 116/116, Vulkan 149/150 with a
  desktop focus flake that passed in isolation).

### 2026-11-16 — Models panel: info under the clicked name

- The per-drawable visibility checkboxes left the Models panel; drawable visibility
  stays in the Rendering panel's drawable list. Models are now a flat list of
  selectable names (active row highlighted) with the same Fit to this model / Delete
  context menu.
- Clicking a name activates that model and draws its info - path, vertex/edge/face or
  point counts, bounding box and the collapsible property listing - directly below
  the name, indented; clicking another name moves the block there. The separate
  "Model Info" section above the list is gone, and a model being processed in the
  background shows its notice in the same place.
- Validation (2026-11-16, macOS): a mesh and a point cloud were opened on both
  backends; the info block sat under the active name with no drawable checkboxes and
  moved when the load order changed. The shared trees could not relink while
  `easy3d/util/setting.cpp` was mid-edit, so the panel file was compiled with each
  check tree's flags and linked against its libraries. Suites not rerun (app-only).

### 2026-11-17 — Distance measuring tool

- The toolbar gains `Measure` (`measure`, icon `resources/icons/measure.png`). It arms
  a one-shot two-click tool: the first left press in the scene places a marker, the
  second completes the segment, logs the distance, reports `Distance: <d>` (four
  decimals) in the status bar and disarms. While armed the button keeps the pressed
  background, the status bar prompts `Measure: click the first/second point`, and the
  scene hint reads `LMB measure   Esc cancel`.
- Esc clears an armed or completed measurement; with no measurement left the next Esc
  behaves as before (leave selection mode, or ask before closing). Arming Measure
  clears the previous measurement, cancels a camera-path tool and leaves the selection
  mode, and arming a selection mode or a path tool cancels an armed measurement, so
  only one tool consumes scene presses. Alt still belongs to the clipping plane and
  model manipulation.
- The annotation is a screen-space overlay (a warm-orange segment 2 logical pixels wide
  with square endpoint markers), not scene geometry: the chord between two picked
  surface points runs through the model, so a depth-tested line was invisible in the
  first implementation. The press defers its pick to `draw_gui()` like the selection
  picks, so the pick runs where the graphics context is current. Like the selection
  rubber band, the annotation is not part of snapshots or video captures.
- The armed button's pressed background is pushed under the pre-click armed state: the
  click toggles the state, and reading it again after the call pushed nothing on the
  arm click (and leaked the push on the disarm click), which aborted with
  `PopStyleColor() too many times` on the first arm. The push/pop pair now uses
  `measure_was_armed`, captured before `toolbar_button()`.
- Validation (2026-11-17, macOS, Release): both check trees build and launch; a
  temporary `MAPPLE_MEASURE_PROBE` probe (removed afterwards; `getenv` count zero)
  armed the tool and clicked two points on bunny.ply, and window captures on both
  backends show the segment, both markers, `Distance: 0.142902` in the status bar and
  the `Esc clears the measurement` hint; the probe log also shows the post-second-click
  `armed 0`. The armed one-point state was exercised by the probe but its window was
  used for a second click before the capture, so only the completed state was
  photographed. The reported crash was then reproduced by hand as loading graph.ply
  and clicking Measure; with the fix, the same real clicks arm and disarm the tool on
  both backends (graph.ply, `Distance: 12.9830` and `Distance: 35.6835` reported, no
  imgui assertion). Suites not rerun (app-only change).

### 2026-11-17 — Status-bar and info-block graph/polyhedral readout

- The status bar's current-model readout now covers the two remaining model types:
  a `Graph` shows `<name>  N vertices, N edges` and a `PolyMesh` shows
  `<name>  N vertices, N edges, N faces, N cells`. The four PolyMesh counts are the
  same set the Qt status bar displayed (Qt used separate `#faces`/`#vertices`/
  `#edges`/`#cells` labels; Mapple keeps its compact one-line form). The
  surface-mesh and point-cloud lines are unchanged.
- The Models panel's info block uses the same counts under the path: `Graph` adds
  `N vertices, N edges` and `PolyMesh` adds `N vertices, N edges, N faces, N cells`.
- Validation (2026-11-17, macOS, Release): Vulkan launched with `graph.ply` active
  showed `graph.ply 80 vertices, 120 edges` in the status bar and `80 vertices, 120
  edges` in the info block; Vulkan and OpenGL launched with `sphere.plm` active showed
  `42 vertices, 171 edges, 220 faces, 90 cells` in both places. Both check trees
  rebuilt. Suites not rerun (app-only change); no clicks scripted.

### 2026-11-17 — File > Loading option (load translation)

- The File menu starts with a Loading option submenu holding the Qt actions as an
  exclusive radio group: No translation (default), Translate relative to first vertex,
  and Translate relative to last known vertex. A choice sets the process-wide
  `easy3d::Translator` status that the file loaders consult and logs the mode; the
  last-known choice also logs the recorded offset, like the Qt messages.
- Session-only, as agreed: the choice is not saved in `settings.ini`, and the viewer
  constructor resets the translator to `DISABLED`, so every launch starts with No
  translation.
- Validation (2026-11-17, macOS, Release): a temporary env-gated probe called
  `set_load_translation` before loading two command-line models. First-vertex mode
  logged each mesh's first vertex as the recorded offset and shifted the bounds
  accordingly (`hemisphere.ply` by (0 0 1)); switching to last-known before the second
  file applied exactly the recorded offset (`fandisk.off` z-range became -1.511..-0.489).
  Default launches of both backends showed no translation log. The probe was removed,
  both check trees rebuilt. Suites not rerun (app-only change); the menu was not
  click-scripted.

### 2026-11-17 — Models panel: selection and the remaining model commands

- The panel is a clean list of model names; the active model's info (path, counts,
  bounds, properties) lives in its own region at the bottom of the panel, directly
  above the footer buttons. The region's height follows its content (measured on the
  previous frame, so expanding Properties or a longer wrapped path grows it) and the
  list takes all remaining room.
- Rows are now multi-selectable: a plain click selects only that row, Ctrl/Cmd toggles
  it, and Shift extends from the anchor; the clicked row also becomes the active model
  (the info block follows it). With no explicit selection the active model is the
  selected row, so the footer and context menu keep acting on it. Double-click a row
  (or Rename) edits the name in place; the edit keeps the parent directory and the
  extension, and an empty name is rejected. Esc drops the edit.
- Batch commands act on the selection in scene order (or on the active model when
  nothing is selected). The context menu offers Fit to this model, Duplicate, Merge
  selected, Decompose, Rename, Invert selection and Delete; the footer keeps only the
  Auto focus / Selected only checkboxes. The Fit and Delete buttons left the footer:
  Fit stays on the context menu (and the F key), Delete on the context menu, and
  Apply to all moved to the bottom of the Rendering panel, because it copies
  rendering styles. Delete asks once for the whole selection and is deferred to the
  end of the panel so the list is never modified while it is being drawn.
- Duplicate clones a surface mesh, point cloud, graph or polyhedral mesh to
  `<name>_copy` and makes the copy active. Merge joins the selected surface meshes
  into `merged_mesh` and the selected point clouds into `merged_point_set` (writing
  `f:index`/`v:index` like the Qt app), deletes the merged-away sources and selects
  the results. Decompose extracts the connected components of selected surface meshes
  into `<parent>/<base>_part_N` models, deletes the originals and selects the parts.
- Apply to all copies the active model's drawable state (including the colormap
  choice) and the point/line/triangle extras (point size and impostor, line width and
  impostor, smooth shading and opacity) onto same-named drawables of every other
  model. The button is pinned at the bottom of the Rendering panel (below a scrollable
  content child, disabled with fewer than two models), since the copy is rendering
  state rather than a model command. Visibility stays per model, and the panel's
  scalar-field/texture records are copied so its controls stay consistent; the scalar
  colormap is applied immediately. A selection display is restored before the copy and
  re-applied afterwards, so the highlight never becomes part of the propagated style.
- Auto focus fits the camera when the active model changes (and when the option is
  turned on). Selected only shows just the active model and is enforced every panel
  frame, so newly loaded or activated models obey it; turning it off shows every model
  again, as the Qt panel did. Both options persist in `settings.ini`
  (`models_auto_focus`, `models_selected_only`).
- The list height reserves exactly the fixed chrome below it: the two footer rows
  (`footer_height`), the separator above the info region, and the item gaps after the
  list, the separator and the info child. It previously reserved an extra text line,
  which surfaced as a gap under the Auto focus / Selected only row; the checkboxes now
  sit the window's normal 6 px bottom padding above the border (measured 12 device
  pixels on OpenGL and Vulkan).
- Validation (2026-11-17, macOS, Release): a temporary `/tmp` probe added two
  in-memory triangles and exercised rename (path and extension kept), duplicate
  (`b_copy`, count grows), merge (2 faces, one result, sources gone), decompose (two
  parts named `merged_mesh_part_N`), Apply to all (the target faces took the source's
  red), Selected only (visibility 0/1/0 around the active model), Auto focus (camera
  radius became the active triangle's) and the deferred delete (active fell back to the
  last model). The probe was removed, both check trees rebuilt, and Vulkan/OpenGL
  launches with two models showed the name-only list, the highlighted active row, the
  info block above the one-row footer, and the Apply to all button pinned at the
  bottom-left of the Rendering panel with the two-model state enabled; the settings
  files were unchanged.
  Suites not rerun (app-only change); dialogs and clicks were not scripted.

### 2026-11-17 — toolbar icons (ImGui texture registry)

- `ImGuiLayer` now has an icon registry. `icon("open")` decodes
  `resources/icons/open.png` on first use into a `graphics::Image` (four channels,
  top row first, linear filtering) and caches the texture ID, failures included;
  `render()` resolves each draw command's texture through the font atlas or that
  registry, so the old "font atlas texture only" exception is gone. Commands carry the
  image's shared pointer, which keeps it alive while they are queued.
- `MappleViewer::toolbar_button(icon, label)` draws one toolbar button: the named
  icon, or a text button when the file cannot be read (the failure is cached, so a
  missing icon is logged once). The label is the hover tooltip. The icon is drawn at
  the font size, so an ImageButton keeps the height of the text buttons.
- Toolbar: Open (`open`), Save (`save`), Fit (`fit`), Snapshot (`snapshot`), Copy
  View (`copy`), Paste View (`paste`) and Measure (`measure`, added with the distance
  tool) are icon-only buttons; Click (`pick`), Rect (`rect`) and Lasso (`lasso`) are
  icon-only toggle buttons that keep the pressed background while their mode is
  active. The text fallback remains for any icon whose file cannot be read.
- Menus: `MappleViewer::icon_menu_item(icon, label, shortcut)` draws a popup row with
  the icon in a leading column - `MenuItem()` cannot show an image. The row is a
  hidden `Selectable` (which owns hover, activation and popup closing) with the icon,
  label and shortcut drawn over it. The shortcut reserves MenuItem's checkmark column
  so it lines up with the other rows of the same popup. The icon is drawn with a white
  tint (alpha only when disabled) because `AddImage()` multiplies the texture by the
  vertex color, while the text takes the item colors. Used by Quit (`quit`, "Alt+F4")
  and About Mapple (`about`).
- The icons are 32x32 PNGs in `resources/icons` (the Qt tree is scheduled for
  retirement, and an installed app cannot reach it), and the top-level install rule
  ships that folder next to fonts, colormaps and textures.
- Validation (2026-11-17, macOS, Release): Mapple rebuilt in both check trees; both
  backends launch ("Mapple UI ready (Vulkan)" / "(OpenGL)") and window captures show
  all nine icons crisp in the toolbar, with the Save icon dimmed while no model is
  open. The File and Help menus were opened with a temporary `/tmp` probe: the Quit
  row shows the stop-sign icon, "Quit" and right-aligned "Alt+F4" level with the
  "Ctrl+S" of the text rows; the About row shows the question icon and "About
  Mapple". The probe was removed and the final builds re-captured. The text fallback
  was exercised when the files were renamed mid-way (Open and Save rendered as text
  buttons until the code used the new names, and Copy View stayed text after
  `duplicate.png` left the set). The settings files were unchanged afterwards. Suites
  not rerun (app-only change); activating the icon rows from the keyboard or mouse
  was not scripted.

### 2026-11-17 — Walk-through keyframe creation (Phase 4 gap)

- The Camera Path window now has a `Mode` row - `Free`, `Walk` and `Orbit` - over
  the Qt walk-through tool. `Free` is the existing behaviour (move the camera, press
  Alt+K to record the view, and the Add Keyframe button is enabled only there). The
  `Walk` and `Orbit` modes instead record with an Alt+click on the scene; the hint
  under the radio row says which: "Alt+click a surface to record a step." for walking
  and "Alt+click to set the rotation axis." for orbit. The Add Keyframe button is
  disabled in both, and Alt+K explains that it belongs to the free mode.
- Walking: the clicked surface point becomes the character's foot. The eye keyframe
  sits at the character's head, `character_height = 0.2 x scene` above the foot and
  `1.8 x character_height` behind it, matching the Qt formulas. `set_scene()` seeds the
  bounds from the visible models, and the first step enables the path overlay so the
  new keyframes are visible. The section's two sliders are `Follow the character`
  and `Character height` (0-100, "%.2f x scene") plus `Character distance to eye`
  (0-100, "%.2f x height"); removing the last keyframe also steps the camera back to
  the new last view in walking mode.
- Orbit: the Alt+click ray serves as the rotation axis through the scene center,
  exactly like the Qt dialog's axis picking. The section holds `Zoom out`, `Vertical
  offset` (-100..100), `Pitch` (-89..89 deg), `Keyframes per loop` (1..99) and
  `Loops` (1..99); each edit regenerates the path immediately. When a path already
  exists the click opens a `Replace Camera Path?` modal instead of silently
  discarding it.
- `WalkThrough` (new `walk_through.h/.cpp`) is a redesign rather than a copy: it
  borrows the viewer's camera and its `KeyFrameInterpolator` instead of owning
  them, and it adds to the scene radius rather than widening it up front, because
  `MappleViewer` already uses the same interpolator for playback, `.kf` import/export
  and video recording. `move_to()` places the camera immediately through the frame
  and updates the pivot point (the Qt version animated on a timer). Deliberately not
  ported: the editor's threaded `preview()`, because `start_manual_interpolation()`
  refuses an interpolator that was ever timer-driven; the tool-side path drawing,
  which `MappleViewer`'s own overlay already does; and the separate "record path" flag.
  Alt+click priority is the clipping plane (when on), then the path tool, then model
  manipulation, and the GPU picks run at draw time from `process_pending_work()`.
- Validation (2026-11-17, macOS): all three trees build; both backends launch
  cleanly with `torusknot.obj` ("Mapple UI ready (Vulkan)" / "(OpenGL)"). A
  temporary in-app probe (removed) generated a walk keyframe at the designed eye
  position (z = -11.243 on a box of range 37.48) and a 20-keyframe orbit at radius
  77.63 = the scene diagonal with constant z. Suites not rerun (app-only); the radio
  row, Alt+click recording and the replace modal were not scripted.

### 2026-11-17 — the camera path is owned by the app, not the Viewer

- The base `Viewer` no longer carries a `KeyFrameInterpolator` (batch 104): no path
  state, playback step, Vulkan path renderer, Alt+K/Alt+D/Ctrl+K/Ctrl+T handlers or
  usage lines. `MappleViewer` owns the interpolator it already drove, so the Camera
  menu, the Camera Path window, `.kf` import/export, video recording and `WalkThrough`
  behave as before; the shortcuts now live in `MappleViewer::key_press_event`.
- Playback keeps its UI-thread clock: `pre_draw()` calls `advance_camera_path()`, which
  applies one manual-interpolation sample per interpolation period. The Vulkan guide
  commands are registered in `draw()` through the protected annotation slot (and again
  before a video capture, which renders outside `draw()`); OpenGL still draws the guides
  directly after `Viewer::draw()`.
- Validation (2026-11-17, macOS, Release): both check trees rebuilt; Vulkan 149/149 and
  OpenGL 116/116 serial; Mapple launches with `sphere.obj` to "Mapple UI ready (Vulkan)"
  and "(OpenGL)". The path keys and overlay were not exercised through scripted input.

### 2026-11-17 — camera-path builders in the walkthrough tool (batch 106)

- `WalkThrough` now separates path generation from the interactive tools. Three pure
  builders return keyframe poses and never move the camera or touch the interpolator,
  so a caller can preview, filter or edit them first:
  `orbit_frames(axis, center)`, `ground_frames(feet, up, eye_height)` and
  `flight_frames(positions, targets, up)`. The rotate-around-axis mode delegates to
  `orbit_frames`, so the existing entry points and callers are unchanged.
- Orbit: the rotate offsets are now relative to the scene extent along the picked axis
  instead of the box's maximum z (which assumed the scene sat on z=0), and the path
  closes the requested number of loops: `keyframes per loop x loops` segment steps with
  the last pose equal to the first, instead of stopping one step short of the start.
  A 10 x 2 orbit therefore has 21 keyframes rather than 20.
- Ground: eye poses one `eye_height` above the walking plane, aiming along the path
  with central-difference tangents. A corner bisects its turn, repeated waypoints are
  skipped instead of producing a NaN heading, and fewer than two feet yield no path.
  Terrain following needs per-foot heights (and scene ray casts), so it is not applied.
- Flight: positions and aim points are separate tracks of equal length; the camera
  looks at the matching target and roll stays referenced to the `up` vector. The
  interpolator smooths both tracks; between stations the aim is the interpolated
  orientation, not an exact target lock.
- Timing stays with the interpolator: per-segment easing and slowing down near points
  of interest are not exposed by this class. The naming changes discussed for the tool
  (a capability name, a `Mode` enum, quantity-in-name parameters) were deferred so the
  existing callers keep compiling.
- Validation (2026-11-17, macOS, Release): Mapple rebuilt in both check trees; a
  temporary headless probe linked against the app's `walk_through.cpp.o` verified the
  orbit count/closure/radius/level plane, constant eye height and path-aligned headings,
  duplicate-waypoint finiteness, dual-track aim, mismatched-input rejection, and that
  `generate_camera_path()` inserts the builder poses. Both backends launch with
  `sphere.obj` ("Mapple UI ready (Vulkan)/(OpenGL)", no warnings or errors). Suites were
  not rerun (Mapple-only change); the probe was removed.

### 2026-11-17 — the walkthrough tool is `CameraPathBuilder` (batch 107)

- The tool is named after its capability: `WalkThrough` is now `CameraPathBuilder`, in
  `camera_path_builder.h/.cpp` (the Qt app keeps its own `WalkThrough` class). The
  status enum becomes `enum class Mode { Off, Free, GroundWalk, Orbit }` with
  `mode()`/`set_mode()`, replacing `Status`, `STOPPED`, `FREE_MODE`, `WALKING_MODE`
  and `ROTATE_AROUND_AXIS`. `MappleViewer::set_camera_path_mode()` now takes that
  enum instead of 0/1/2, and the viewer member follows (`camera_path_builder_`); the
  unused private `camera_path_tool()` accessor is gone.
- Parameters now state the quantity and the unit they are expressed in:
  `height_factor` is `eye_height_in_scene_heights`, `third_person_forward_factor` is
  `eye_distance_in_character_heights`, `zoom_out_factor` is
  `orbit_radius_offset_in_object_heights` (an offset added to the scene diagonal, not
  the radius itself), `vertical_offset_factor` is
  `orbit_vertical_offset_in_object_heights`, `pitch_angle` is `pitch_degrees`, and
  `keyframe_samples` is `keyframes_per_loop`. The private helpers follow:
  `character_height()` is `eye_height()`, `third_person_forward_distance()` is
  `eye_distance()`, and `character_head()` is `eye_position()`. The Camera Path
  window's labels, ranges and behavior are unchanged.
- The app source list and the callers were updated (`mapple_viewer.h/.cpp`,
  `ui/animation.cpp`, `applications/Mapple/CMakeLists.txt`). No test target builds
  this tool, so no test file changed.
- Validation (2026-11-17, macOS): Mapple rebuilt in both check trees; both backends
  launch with `sphere.obj` ("Mapple UI ready (Vulkan)/(OpenGL)") and exit cleanly.
  Suites were not rerun (app-only, pure rename with identical mode mapping); no
  clicks were scripted. `easy3d/util/version.h` is unchanged.

### 2026-11-17 — one-shot path tools and a dragged walking route (batch 108)

- The Camera Path window's persistent `Free`/`Walk`/`Orbit` radio row is gone. Two
  buttons, `Walk a route...` and `Orbit...`, arm a tool for **one** gesture: while it
  is armed the window shows that tool's hint and its parameters, `Cancel` (or Esc, or
  clicking the armed button again) disarms it, and applying the gesture disarms it
  automatically. Free recording is not a mode - `Add Keyframe` is always enabled, and
  Alt+K keeps recording the current view.
- Walking is now a single left-drag on the scene instead of one click per step. The
  press starts a stroke on the horizontal plane at the bottom of the visible scene's
  bounding box (the walking plane is therefore the scene's floor, not the picked
  surface under the cursor), which needs no GPU pick, so every sample is available
  while the drag runs. The stroke is drawn live as a highlight-colored line in the
  scene overlay layer (same coordinate and DPI contract as the lasso). On release the
  stroke is resampled by arc length at the new
  `keyframe_spacing_in_character_heights` setting (default 1, i.e. about one character
  height apart), converted by `ground_frames()` into first-person eye-height poses
  that aim along the route, and appended to the path - so one gesture records a whole
  street walk, and a second stroke continues it. A stroke too short to produce a
  keyframe changes nothing and leaves the tool armed; rays that graze the ground and
  would meet the plane implausibly far away (`> 4 x` the scene radius from the center)
  are rejected. Arming requires a visible model, and the scene box is recomputed at
  that moment.
- Orbit remains a one-shot click: the ray through the click becomes the axis, the
  path is generated when it is empty, and a non-empty path is still replaced only
  through the existing `Replace Camera Path?` modal. Its parameter sliders are now
  chosen before the click; editing them no longer regenerates an existing orbit.
- The click-to-walk API this supersedes is removed rather than left dead:
  `walk_to()`, `set_follow_up()`/`follow_up()`,
  `set_eye_distance_in_character_heights()`/`eye_distance_in_character_heights()`,
  and the helpers `eye_distance()`, `eye_position()`, `move_to()`,
  `delete_last_keyframe()`, `clear()`, `current_keyframe_index()`,
  `add_keyframe(pos, dir)`, plus `Mode::Free` (the enum is now
  `{ Off, GroundWalk, Orbit }`). The orbit setters lost their `re_generate`
  parameter, and `set_scene()` now invalidates the box when no visible model remains,
  so the tools can refuse to arm. The Camera Path window, `MappleViewer` input
  routing (`arm_camera_path_tool()`, `cancel_camera_path_tool()`,
  `commit_ground_route()`, `apply_orbit_pick()`, `route_point_at()`,
  `append_path_stroke_overlay()`) and `camera_path_builder.h/.cpp` were updated; the
  removed walk-specific UI controls are replaced by `Eye height` and
  `Keyframe spacing`.
- The remaining six-part interaction plan is recorded in `ToDo.md`: orbit by dragging
  the axis with a turntable default and Replace/Append plus undo (3), editable
  keyframe handles (4), a path snapshot with local undo replacing the modal (5), and
  a flight recorder feeding `flight_frames()` (6).
- Validation (2026-11-17, macOS, Release): Mapple rebuilt in both check trees
  (`build-opengl-check`, `build-vulkan-check`); both backends launch with `sphere.obj`
  ("Mapple UI ready (OpenGL)" / "(Vulkan)") and exit. A temporary headless probe
  (removed afterwards) exercised `generate_ground_route()`: a straight 10-unit stroke
  at eye height 2 produced 6 keyframes 2 units apart, all one eye height above the
  plane and aiming along the route; a 20-unit L-stroke produced 11 with the corner
  pose on the bisector; a repeated point was skipped; a stroke shorter than the
  spacing kept its two ends; a second stroke appended; a single point generated
  nothing. Suites were not rerun (Mapple-only change) and the arm/drag gestures were
  not scripted. `easy3d/util/version.h` is unchanged.

### 2026-11-17 — route walks follow the picked surface (batch 109)

- The walking route no longer assumes that world Z is up and that the walking plane
  is a horizontal plane at the bottom of the scene box. That assumption is wrong for
  any model that is not Z-up: `resources/data/domik/domik.obj` is Y-up (its extents
  are 7207 x 3305 x 10858, so Z is the house's depth), and the old code put the
  stroke plane on a vertical plane through the house (z = -5418.67) and lifted the
  keyframes along Z, producing a path through the air along the house's depth.
- Every drag sample is now picked on the visible surface with `Viewer::pick()`, so
  the stroke heights follow the terrain: `route_point_at()` returns the hit position,
  the `route_plane_` member is gone, and a drag that leaves the visible surface ends
  the current segment (the remaining stroke becomes another route rather than a
  straight cut through the gap). Samples are picked at least ~4 logical pixels apart
  because each pick re-renders the scene into the pick target; the live ribbon still
  shows the whole accepted drag trail in the overlay layer.
- `CameraPathBuilder` gained
  `enum class GroundUpAxis { Auto, X, Y, Z }`, `ground_up_axis()` /
  `set_ground_up_axis()` and `ground_up_direction()`. Auto (the default) resolves to
  the shortest extent of the visible scene box - the height of a building- or
  terrain-like scene (domik: +Y) - and the Walk tool's parameters now show an
  `Up axis` combo (`Auto (box shortest)`, `+X`, `+Y`, `+Z`) for scenes whose height
  is not the shortest extent (a standing figure). The former member
  `ground_plane_normal_` is removed.
- Every pose is `foot + up * eye_height` and aims level along the route, so the feet
  keep their own surface heights (the builder documents `ground_frames()` as
  per-foot rather than plane-constrained). `eye_height_in_scene_heights` now scales
  the scene extent along the up axis - the Z extent when +Z is chosen, exactly as
  before; the eye height slider and tooltip say so.
- Validation (2026-11-17, macOS, Release): Mapple rebuilt in both check trees
  (`build-opengl-check`, `build-vulkan-check`); both backends launch with `domik.obj`
  ("Mapple UI ready (OpenGL)" / "(Vulkan)"). A temporary headless probe (removed
  afterwards) set a renderer on `domik.obj` so `set_scene()` produced a valid box:
  Auto resolved +Y; a stroke climbing 800 units in Y over 4000 in X produced
  keyframes each 0.2 x 3304.89 = 660.98 above their own foot along Y, on the
  stroke's Z plane, aiming level with zero vertical component, about one eye height
  apart; the explicit +Z override measured the scene along Z (2171.53) and kept the
  foot heights. Suites were not rerun (Mapple-only change), the arm/drag gestures
  were not scripted (picking is the same `Viewer::pick()` path the hover readout
  uses), and `easy3d/util/version.h` is unchanged.

### 2026-11-17 — walker upright in the view; keyframe cameras as scene triangles (batch 110)

- The walking route no longer guesses the up axis from the scene box. The walker
  stands upright in the current view, so `CameraPathBuilder::ground_up_direction()`
  is the camera's (normalized) up vector, and the route rises from the traced
  surface along it; a rolled view tilts the walker. `eye_height()` measures the
  scene extent along that direction, so the eye-height ratio keeps its meaning in
  any view. The `GroundUpAxis` enum, `ground_up_axis()`/`set_ground_up_axis()` and
  the Walk tool's Up axis combo are removed (batch 109's Auto/X/Y/Z choice is
  superseded); the window instead shows the resolved direction, e.g.
  `Up axis  (0.00, 1.00, 0.00)`, with a tooltip. `domik.obj` viewed upright still
  yields +Y, but the answer now follows the view rather than the box.
- The keyframe camera glyphs are scene triangles instead of annotations
  (`MappleViewer::update_camera_markers()`): one glyph per keyframe, posed by
  `Frame::matrix()` into a standalone `TrianglesDrawable`
  ("camera path markers", uniform green `(0.2, 0.7, 0.3)`) added with
  `add_drawable()` (the glyph builder moved into the core as
  `KeyFrameInterpolator::update_cameras_drawable()`, batch 117, which Tutorial 206
  uses too). They are rebuilt in `pre_draw()` only when the poses or the
  glyph width change, hidden with `set_visible(false)` while playback runs (or when
  the width is 0), and removed by `release_camera_markers()` before
  `restore_scene_bounds()`
  in `set_camera_path_visible(false)` and `clear_camera_path()` so the hidden path
  leaves no keyframe extent in the scene bounds. Width is 3% of the models' box
  diagonal (the keyframe spread when no model is loaded) rather than 5% of
  `camera()->scene_radius()`, because the glyphs are now part of the scene bounds
  and a radius-based width would grow with its own bounds.
- This gives the glyphs a written depth: the scene pass hides them behind the
  surfaces (the far side no longer shades through), and because standalone
  drawables are in the scene depth and pick passes on both backends,
  `point_under_pixel()` (the Z / Shift + left-click pivot gesture) lands on a glyph
  and `pick()` reports it (`PickResult::model` is null). `Viewer::draw()` and its
  effects draw them with the models (the OIT transparency mode, which renders only
  the model surfaces itself, still omits every standalone drawable - a pre-existing
  limitation of that mode).
- The Vulkan annotation slot now carries only the path polyline
  (`KeyFrameInterpolatorRenderer::draw_path()`); Mapple no longer calls
  `draw_cameras()`, and the OpenGL `draw_camera_path()` likewise draws only the
  line. The core renderer's `draw_cameras()` stays for the camera-path tests and
  Mapple-Qt (Tutorial 206 switched to the shared scene drawable in batch 117), and
  the viewer-level annotation pivot hook and the Vulkan
  culled-overlay pipeline tried on the way here were removed again (the core
  libraries are unchanged from HEAD).
- Validation (2026-11-17, macOS, Release): Mapple rebuilt in both check trees
  (`build-opengl-check`, `build-vulkan-check`); both backends launch with
  `domik.obj` ("Mapple UI ready (OpenGL)" / "(Vulkan)"). A temporary headless probe
  (removed afterwards, built against both trees) placed glyphs over a ground quad
  and required, on both backends: the glyph is in the scene depth (the probe point
  returns z > 1 for a glyph at z = 2 over ground z = 0), it is drawn (the capture
  differs), `pick()` returns the glyph drawable with no model, a hidden glyph
  leaves the depth alone, a glyph behind the ground is occluded (the probe point and
  the pick report the ground) and deleting it restores the bare capture. Suites were
  not rerun (Mapple-only change), keyframes and the pivot gesture were not
  scripted, and `easy3d/util/version.h` is unchanged.

### 2026-11-17 — the walker stands upright on the terrain (batch 111)

- The walking route's up direction is now the surface under each foot, not the
  view (batch 110's camera-up choice is superseded). `MappleViewer::route_point_at()`
  also returns the walker's up at the picked point: the hit face's normal
  (`SurfaceMesh::compute_face_normal(Face(PickResult::face_index))`), flipped to
  face along `ground_up_direction()` because the winding decides its sign, and the
  view's up when the hit has no face (a point cloud or a standalone drawable).
  A dragged route keeps a `RouteSample {point, up}` per sample instead of a bare
  point, and `commit_ground_route()` hands the points and directions to
  `CameraPathBuilder::generate_ground_route(stroke, foot_up_directions)`.
- The builder resamples points and directions through one arc-length plan
  (`arc_length_samples()` records `{segment, t}` and `sample_value()` interpolates
  either array), normalizes each direction and aligns it with the view's up,
  smooths them along the route with two 1-2-1 passes (picked facet normals step at
  facet edges), and `ground_frames(feet, up_directions, eye_height)` now builds
  every pose along its own foot's up, projecting the central-difference heading
  onto that foot's tangent plane. The walker therefore tilts with the terrain and
  the route still follows it. An empty direction vector keeps the view's up
  (`ground_up_direction()` is now documented as that fallback and still scales the
  eye height); directions that do not pair with the stroke, or a degenerate one,
  produce no keyframes.
- The Walk window's `Up axis  (x, y, z)` readout (batch 110) is replaced by
  `Up  surface normal under each foot`, with a tooltip that names the fallback; the
  eye-height tooltip now says each pose rises along its foot's normal, measured
  against the scene extent along the view's up.
- Validation (2026-11-17, macOS, Release): Mapple rebuilt in both check trees
  (`build-opengl-check`, `build-vulkan-check`); both backends launch with
  `domik.obj` ("Mapple UI ready (OpenGL)" / "(Vulkan)"). A temporary headless probe
  (removed) walked a 30-degree ramp with constant surface normals: 11 keyframes
  were generated, every keyframe's up equals the ramp's normal, every pose sits one
  eye height above the ramp line along that normal, every heading lies in the
  terrain plane, the keyframes advance along the stroke, a route without
  directions falls back to the view's up, and mismatched directions are refused.
  The drag gesture itself (picking a face normal under the cursor) was not
  scripted, so the on-screen tilt was not captured; suites were not rerun
  (Mapple-only change, core libraries at HEAD), and `easy3d/util/version.h` is
  unchanged.

### 2026-11-17 — route ups turn to the camera-facing side (batch 112)

- Batch 111 oriented each picked face normal toward the view's up, which is not
  always the side of the surface the camera sees: a slope can face any way relative
  to that up, so some `RouteSample::up` directions were inverted and the eye centers
  landed behind the terrain. `MappleViewer::route_point_at()` now orients the normal
  against the camera's view direction (`Camera::view_direction()` points from the
  camera into the scene): the walker's up must oppose it - the front-facing side of
  any rendered surface - and is negated otherwise. The view's up remains only the
  fallback for hits without a face (point clouds, standalone drawables).
- `CameraPathBuilder::generate_ground_route()` applies the same rule to the whole
  input before resampling, so a route traced with arbitrary (or mixed) face winding
  cannot cancel out in the interpolation; the 1-2-1 smoothing preserves the sign
  because every sample is already on the camera side. `ground_up_direction()` no
  longer flips the directions; it still supplies the no-normal fallback and the
  eye-height scale.
- Validation (2026-11-17, macOS, Release): Mapple rebuilt in both check trees;
  both backends launch with `domik.obj` ("Mapple UI ready (OpenGL)" / "(Vulkan)").
  A temporary headless probe (removed) looked down at a 30-degree ramp: keyframes
  generated from the normal keep their up on the camera-facing side, the same
  stroke with every normal inverted produces the same ups (the inversion is
  corrected), and an every-other-sample inversion resolves to the same up
  (resampling does not cancel). The drag itself was not scripted; suites were not
  rerun (Mapple-only change, core at HEAD), and `easy3d/util/version.h` is
  unchanged.

### 2026-11-17 — orbit by dragging its axis, with a turntable default (batch 113)

- Arming Orbit now defines the axis with a press-drag in the scene instead of an
  Alt+click that used the scene center. The press must land on a visible model
  surface and the axis passes through that hit point. Dragging sets the axis along
  the dragged screen segment, mapped through the camera's right and up vectors; a
  press-and-release without a drag (under 2 logical pixels) keeps the turntable
  default - the view's up axis through the picked point, giving a horizontal circle
  around it.
- The preview is the axis itself: the overlay extends the drag direction into a
  full-length line across the view through the press point (the screen trace of
  the 3D axis), in the same highlight ribbon as the walking stroke. A click previews
  the view's up axis, so the default is visible before the release.
- The axis direction is sign-corrected so `orbit_frames()`'s `up = -axis.direction()`
  stays the view's up: an upward drag orbits upright instead of upside down. The
  orbit's `Zoom out`, `Vertical offset`, `Pitch angle`, `Keyframes per loop` and
  `Loops` parameters are unchanged.
- The "Replace Camera Path?" modal is gone. The Orbit section offers `Replace path`
  / `Add to the path`; the gesture applies the choice directly. Arming either path
  tool snapshots the path and `Ctrl+Z` restores that snapshot (one level; it is
  consumed by the restore), so a replaced or appended orbit - and a walking route
  appended by mistake - can be undone without a dialog. The window shows the
  restore hint while a snapshot is pending. This also closes the separate
  camera-path snapshot/undo roadmap item.
- Validation (2026-11-17, macOS, Release): Mapple rebuilt in both check trees;
  both backends launch with `domik.obj` ("Mapple UI ready (OpenGL)" / "(Vulkan)").
  A temporary headless probe (removed) drove `CameraPathBuilder::orbit_frames()`
  directly: with the view's up axis every camera center stays at the picked height,
  every camera up equals the view's up, every camera looks level, the centers stay
  one scene diagonal from the axis, the path closes, the vertical offset moves the
  circle up the axis, and an upward axis (the pre-flip gesture direction) orbits
  upside down as the sign rule expects; a snapshot restore keeps every pose and
  time. The drag/press/clicks were not scripted; suites were not rerun
  (Mapple-only change, core at HEAD), and `easy3d/util/version.h` is unchanged.

### 2026-11-17 — editable keyframe handles (batch 114)

- The Camera Path window gains `Edit keyframe handles`. Turning it on shows the path
  if it was hidden and selects the last keyframe; a plain press on a visible keyframe
  camera glyph then grabs that keyframe. The grab uses the scene pick: the keyframe
  glyphs stay one triangle drawable, and `update_camera_markers()` stores the
  glyph's triangle count so the picked primitive maps back to its keyframe
  (`primitive / faces_per_glyph`). A press anywhere else keeps its usual meaning
  (camera navigation, selection); an armed path tool still owns the press.
- The drag moves the pose's position only (the orientation is untouched) in the
  screen plane through the pose, so it follows the cursor; holding Shift at the
  press uses the pose's own up as the plane normal - the terrain plane for a walking
  pose, the orbit plane for a turntable pose - which keeps the pose's height along
  its up. The plane is fixed at the press; a ray parallel to it or a plane behind
  the press is ignored, and the scene radius grows with the moved pose as when a
  keyframe is added. Esc (or losing focus, or closing the window) restores the
  position; the undo snapshot is taken on the first movement, so a press that only
  selects does not consume the existing undo point.
- `Insert After` inserts a keyframe between the selected keyframe and the next: the
  pose is the position midpoint and the orientation slerp of the two, and the time is
  their midpoint, so the section keeps its timing and shape. Past the last keyframe
  the last segment is extended (the position step and the time gap are repeated) so
  the new glyph does not sit on top of an existing one. `Delete Selected` removes the
  selected keyframe. Both buttons act on the selection shared with Prev/Next and the
  handle grab, and the overlay marks that handle with a highlight-colored square.
- `KeyFrameInterpolator` only appends, so the edits rebuild the path:
  `rebuild_camera_path()` stops playback, clears the path and re-adds the frames with
  their times; `Ctrl+Z` uses the same rebuild from a snapshot, so an insert or a
  delete can be undone. Both actions snapshot the path first.
- Validation (2026-11-17, macOS, Release): Mapple rebuilt in both check trees; both
  backends launch with `domik.obj` ("Mapple UI ready (OpenGL)" / "(Vulkan)"). A
  temporary headless probe (removed) checked the drag and rebuild arithmetic: the
  screen-plane intersection starts under the cursor and follows it to a new cursor
  within one logical pixel, stays in the screen plane, the pose-up plane keeps the
  pose's height along its up (also for a 30-degree tilted pose), a ray parallel to
  the plane and a plane behind the press are rejected; an insert produces the
  midpoint pose and time (1.5 s between 1 s and 2 s), rebuilding keeps the ordering,
  deleting keeps the surviving times, and the past-the-end extension repeats the last
  step. The glyph pick, Shift modifier and buttons were not scripted; suites were not
  rerun (Mapple-only change, core at HEAD), and `easy3d/util/version.h` is unchanged.

### 2026-11-17 — the camera path is scene geometry too; no Vulkan path renderer (batch 118)

- The path polyline joins the keyframe glyphs as ordinary scene geometry:
  `update_camera_markers()` became `update_camera_path_guides()`, which maintains
  "camera path markers" (`TrianglesDrawable`) and "camera path line"
  (`LinesDrawable`, filled by the new `KeyFrameInterpolator::update_path_drawable()`)
  side by side. Both are rebuilt on the same pose/width change and hidden while
  playback runs, while the path is hidden, when the width is 0, or (the line) when
  there is no curve to show; `release_camera_path_guides()` removes both before
  `restore_scene_bounds()`. The line is a 2-pixel cylinder impostor in the library's
  blue, and because it is scene geometry the models occlude it and it writes depth
  and IDs like the glyphs. A video capture includes both, since the capture renders
  the scene the guides were just rebuilt into.
- The Vulkan annotation slot and the immediate `draw_camera_path()` are gone, along
  with `vulkan::KeyFrameInterpolatorRenderer` and its `camera_path_renderer_` member
  (the class is deleted from the library, batch 118). `draw()` no longer refreshes or
  draws a path itself, and `pre_draw()` no longer has an annotation refresh before a
  video frame.
- Validation (2026-11-17, macOS, Release): Mapple rebuilt in both check trees; both
  backends launch ("Mapple UI ready (OpenGL)" / "(Vulkan)") with the settings files
  unchanged afterwards. The paths were not exercised by scripted interaction; the
  suites (which cover the shared helpers through `vulkan.camerapath` and
  `opengl.cameraguides`) ran with Vulkan 150/150 and OpenGL 118/118. See the
  batch-118 entry in docs/vulkan.md.

### 2026-11-17 — the hover cue is a face overlay (batch 124)

- The Click-mode hover no longer uses the drawable's highlight mask. It installed a
  one-interval mask over the hovered *rendered triangle*, which replaced the
  selection's whole mask - a Shift/Ctrl multi-selection lost its tint while the
  pointer moved - and tinted only one triangle of a polygonal face. `update_hover()`
  now resolves the picked primitive to its mesh face (the drawable's
  `source_faces`, or `f:triangle_range` through `face_from_primitive()` where that
  mapping is unavailable), tessellates the face the same way the renderer does
  (`Tessellator`, nonzero winding, so a concave face is not covered by a fan that
  strays outside it) and stores its triangles in world space (`hover_face_`).
  `append_hover_overlay()` projects them and submits the fill as a depth-tested
  overlay among `pre_draw()`'s hint commands - the same overlay path as the
  selection rubber band - in `setting::highlight_color` at alpha 0.35, so the face's
  own shading stays readable. A selection or a Rendering-panel highlight range
  therefore keeps its tint while hovering, and the cue always covers exactly the
  face that the click selects. Batch 94's "the hover aid yields to a configured
  range" and the panel's reset of a stale hover range are gone with the mask
  bookkeeping.
- Validation (2026-11-17, macOS): Mapple rebuilt in both check trees; both backends
  launch ("Mapple UI ready (OpenGL)" / "(Vulkan)") with the settings files restored
  afterwards. A temporary headless probe (removed) added a quad, forced its buffer
  upload, and checked that both rendered triangles map to the one face
  (`f:triangle_range` 0..1, Vulkan `source_faces` 0 0), that `pick()` resolves the
  rendered triangle to face 0, and two snapshots around the face fill: the face
  center shifted toward the highlight color ((255,230,136) -> (255,149,88)) while a
  point just outside the face was unchanged, identically on OpenGL and Vulkan. The
  hover gesture itself was not scripted; the suites were not rerun (Mapple-only
  change).

### 2026-11-17 — interactive keyframe handles on the glyph (batch 127)

- The Camera Path window's `Edit keyframe handles` now puts a gizmo on the selected
  keyframe that is sized to its camera glyph: `keyframe_handle_at()` projects three
  pose axes (the keyframe orientation's local X/Y/Z) as arrows reaching 1.25x the
  marker width and three rings at 0.75x that width, and resolves a hover or press to
  a `KeyframeHandle` (`move_x..move_z`, `turn_x..turn_z`, or `free` for the glyph).
  Arrows and rings are screen-space tests with a 7-logical-pixel tolerance, accepted
  before the scene pick so they win where they overlap the glyph; an axis seen
  end-on (|dot(axis, view)| > 0.97, or a projected arrow shorter than 10 pixels)
  offers no arrow, and a ring seen edge-on (< 0.2) offers no ring. The same
  handlers draw the gizmo as overlay lines in axis colors - red/green/blue, the
  hovered or grabbed element full and 2.5x thick, the others dimmed to alpha 0.25 -
  so the element a gesture will act on is unmistakable.
- Dragging an arrow moves the pose along that axis: the cursor ray's nearest
  parameter on the axis line (the new `ray_axis_parameter()` helper) is applied to
  the position captured at the press, so the pose stays exactly on the axis.
  Dragging a ring turns the pose around that axis by the accumulated cursor angle in
  the ring's plane (unwrapped with `std::remainder`, so crossing the seam does not
  flip the pose) applied to the orientation captured at the press;
  the glyph drag keeps its batch-114 meaning (screen plane, or the pose's up plane
  with Shift). The undo snapshot is still taken on the first movement, Esc restores
  both position and orientation, and `end_keyframe_drag()` logs whether the pose was
  moved or turned.
- Fixed after the first build: the drag axis was derived as `handle % 3`, but the
  handle enum starts with `none, free` (`move_x` is 2), so every handle acted on the
  next pose axis (x dragged along z, y along x, z along y) - arrows and rings alike.
  The axis is now the handle's offset within its `move_`/`turn_` triple, the same
  index the gizmo draws and hit-tests, so a constrained drag follows the element
  under the cursor. Both check trees rebuilt.
- Fixed after the first build: a glyph drag moved the station to the cursor's plane
  intersection, so grabbing the glyph anywhere but exactly at the station square
  teleported the pose to the clicked point on the first drag event. The press now
  records the in-plane offset from that intersection to the station and adds it back
  on every move, so the grabbed point of the glyph stays under the cursor (a press
  that never moves no longer takes an undo snapshot or logs a move).
- The handling otherwise follows batch 114: an armed path tool or a running playback
  keeps its meaning, the gizmo appears only while the edit tool is on, and
  `Ctrl+Z` restores the path from before the drag. `ToDo.md`'s open "attach a
  Manipulator to each camera" item is closed by this gizmo - the path is not a
  model, so a per-keyframe handle is the direct-manipulation equivalent.

### 2026-11-17 — the Qt dialog capability pass (batch 127)

- A last audit of `applications/Mapple-Qt/dialogs` against the ImGui app found five
  real gaps in `ui/process.cpp`, plus the missing PolyMesh save, all closed:
  - Point-cloud simplification keeps the expected-count strategy but regains Qt's
    distance-threshold strategy: `Uniform (minimum spacing)` calls
    `uniform_simplification(cloud, epsilon)` and the unchecked variant calls
    `grid_simplification(cloud, cell)`, with the read-only average spacing filled by
    `average_space(cloud, nullptr, 6)` on demand (reset to unknown when the window
    reopens).
  - Remeshing regains the scheme choice - uniform (`mean edge * factor`, factor
    0.1..5, default 1.0) or adaptive (`0.001 / 0.1 / 0.001` of the bounding-box
    diagonal) - and feature preservation: `SurfaceMeshFeatures::clear()`,
    `detect_angle(dihedral)` and `detect_boundary()` run before either scheme, with
    the dihedral angle (1..180, default 60) enabled only when the box is checked.
  - Hole filling now matches the Qt semantics instead of `fill_holes(size)`, which
    compares halfedge counts and excludes the bound: it scans the boundary loops
    once, skips holes around non-manifold vertices, fills each hole whose boundary
    edge count is at most the requested size (default 50), and reports the smallest
    and largest hole when none qualifies.
  - Defaults and bounds follow Qt: smoothing starts at 5 iterations, `RANSAC`
    starts with the plane only, normal estimation/reorientation clamps k to
    6..1000, and Poisson clamps the octree depth to 1..15 and samples per node to
    1..20. The inline trim now checks the reconstruction's `v:density` range first
    and skips with a warning when the trim value is outside it, as the Qt dialog
    did (rather than silently producing an empty mesh).
- Also fixed: `File > Save` now writes `PolyMesh` through `PolyMeshIO::save()`
  (the filters already advertised `*.plm *.pm *.mesh`).
- Judged intentional, not gaps: the ImGui slice cuts with a clipping plane instead
  of Qt's ten-plane 2D section; a snapshot always writes the matching `.view`
  state (Qt made it optional); Qt's "vec2/vec3" conversion targets were dead (the
  conversion switch never handled them) and are deliberately absent; the library
  has no PPM writer, so PPM is not offered, and the `*.ic` indexed-colors import is
  not worth carrying over (Qt implemented it for point-cloud vertices only); Qt's
  `OrientClosedTriangleMesh` (CGAL-gated) exists in the ImGui app as
  `Process > Surface Mesh > Orient Outward` with the same closed-triangle-mesh
  guard, so there is nothing to port there.
  Everything else in the Qt dialogs (normal estimation/reorientation, RANSAC,
  smoothing, fairing, simplification, sampling, remeshing, hole filling, Poisson,
  parameterization, curvature, text, Gaussian noise, snapshot, properties,
  walkthrough) already existed in the ImGui app, which additionally offers AO,
  geodesic, planar partition, components, triangulation, dual, subdivision,
  repair/stitch, self-intersection removal, tetrahedralization, height fields and
  topology statistics.
- Validation (2026-11-17, macOS): Mapple rebuilt in the OpenGL release tree and the
  Vulkan debug tree; both backends launch with `bunny.ply` ("Mapple UI ready
  (OpenGL)" / "(Vulkan)") with no errors or warnings in `Mapple.log`, and the
  Mapple settings files were unchanged afterwards. The process dialogs and the
  gizmo gestures were not scripted (no synthetic input); suites were not rerun
  (Mapple-only change) and `easy3d/util/version.h` is unchanged.

### 2026-11-17 — deleting the selection no longer crashes on a stale highlight mask

- Deleting selected elements exited the viewer with
  `Viewer failed: highlight mask must have one strength per rendered primitive`.
  `delete_selected_primitives()` compacts the model and invalidates the renderer,
  but `finish_selection_change()` -> `refresh_selection_display()` then applied the
  mask through `buffer::update_highlight_mask(model, drawable)` while the
  selection drawable still held the buffers from before the edit. A vertex mask is
  sized per surviving vertex of the model, so it no longer matches the drawable's
  pre-deletion primitive count and the consistency check in
  `Drawable::update_highlight_mask()` throws. A new model's drawable (e.g. a
  duplicate that carries `f:select`/`v:select`) hits the same check from the other
  side, zero built primitives against a non-empty mask.
- `MappleViewer::refresh_selection_after_model_change(model)` now rebuilds the
  selection drawable with `buffer::update(model, drawable)` (which re-extracts the
  geometry and applies the mask itself) when the model still holds a selection, and
  is called at the model-change funnels before `finish_selection_change()`:
  `delete_selected_primitives()` and `finalize_process()` (every process dialog that
  removes geometry). A drawable that has not been extracted yet is rebuilt first in
  `refresh_selection_display()` (a duplicated or never-drawn model with a
  selection). With no selection left the mask is cleared without a size check, so
  the drawable stays lazy. No library code changed.
- Validation (2026-11-17, macOS): a headless probe outside the repository, linked
  against the Vulkan debug libraries, reproduced the exact exception with the old
  order on a deleted point cloud (10 primitives in the drawable, 8 in the model)
  and on a fresh drawable, and showed no exception with the rebuild first (8 mask
  entries in both cases). Both check trees rebuilt; both backends launch with
  `bunny.ply` ("Mapple UI ready (OpenGL)" / "(Vulkan)") on an error-free
  `Mapple.log`, with the settings files restored afterwards. The delete gesture was
  not scripted; suites were not rerun (Mapple-only change) and
  `easy3d/util/version.h` is unchanged.
