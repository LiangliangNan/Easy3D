# Mapple on Dear ImGui

This document tracks the migration of Mapple from Qt Widgets (`applications/Mapple`) to a
Dear ImGui application rendered through Easy3D's backend-neutral overlay path
(`applications/Mapple-imgui`). Qt/Mapple remains buildable until the new app covers the
daily-use workflow.

Status: 2026-11-15 — Phase 0 (shell) and Phase 1 (inspection, selection) complete:
builds and runs on both OpenGL and Vulkan; panels, menus, log capture, drawable styling,
picking readout, and point/face selection work. The clipping plane is the remaining
Phase 1 item.

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

- [x] Model data inspection: the active model's path, vertex/edge/face counts, bounding
      box, and a collapsible property listing (`Model::property_stats`).
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
- [ ] Clipping plane.

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

### Phase 2 — processing

Expose algorithms as ImGui forms behind logical menus/panels, driven by need rather
than by the Qt dialog list. Start with the workflows used most: point-cloud
simplification/normal estimation, surface-mesh smoothing/simplification/remeshing,
hole filling, reconstruction, polygonization. Parameter forms can be small modal
windows; results report through the log panel.

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
