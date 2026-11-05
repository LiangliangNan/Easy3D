### ToDo list (or on going):
- Vulkan support (experimental). Details, decisions and the ordered work items are in docs/vulkan.md
  ("Current status and work order"). One backend per build (`Easy3D_ENABLE_VULKAN` ON/OFF); user code and
  all tutorials stay the same for both backends (`Viewer`, `MultiViewer`, `OffScreen` from `easy3d/viewer/viewer.h`).
  - [x] Remove "easy3d/graphics" dir (content consolidated into "easy3d/renderer" and "easy3d/viewer")
  - [x] Exclusive backend choice (`Easy3D_ENABLE_VULKAN`), runtime selection and offscreen facade removed
        (the OpenGL*/Vulkan* class names this created are superseded by the principles below)
  - [x] Shared GLFW lifecycle, frame driving, portable input/frame callbacks and camera/surface queries
  - [x] Unify `point_under_pixel()`: Viewer and MultiViewer inherit the sole ViewerBase implementation;
        logical window coordinates, fresh scene-only depth, exact framebuffer pixel centers. Native callers,
        Python bindings and tests use the unified contract without explicit base-qualified calls.
  - [x] Shared annotations on both backends; Tutorials 202/306/309 share image, text and GUI presentation.
  - [x] Independent 3D HUD depth and diffuse/material lighting on both backends; preserve MSAA, scene queries and scaled/transparent snapshots.
  - [x] Shared shape builders and Manipulator guide API; Tutorials 207/405/506 use shared annotations and GPU depth previews.
  - [x] W1-W3 docs sync; build blockers (single home for tutorials, enable_testing, bounded tutorial tests); Tutorial 208
  - [x] W4 Delete runtime-selection/headless leftovers (Backend enum, ViewerOptions, create_*_viewer, ...)
  - [x] Shared GPU point selection and Tutorial 403; OpenGL aliases rendering VBO positions, Vulkan retains SSBO compute, selection recolors attributes only.
  - [x] W5 Remove EASY3D_TUTORIAL_USES_VULKAN; Tutorial 404 uses shared capture and local pixel-center reconstruction.
  - [ ] W6 Mac: OpenGL 90/90 (one `opengl.imgui` flake passed on re-run); MoltenVK 119/119 (2026-11-05, W10 batch 13; prior intermittent window-test timeout remains under observation); Linux baseline 68/69 still needs the interactionoverlay border check
  - Top priority (docs/vulkan.md, "Migration principles"): minimal source changes; same class names and the same
    .h/.cpp files as the main branch (Viewer in viewer.h/.cpp for both backends); #ifdef EASY3D_HAS_VULKAN layout inside
    the classes; no new files except shaders, backend files and tests; minimal CMake changes; client code unchanged.
  - [x] U1 One `Viewer` class (viewer.h/.cpp), ViewerBase retained; signed int constructors and override cleanup; counterpart host files/aliases removed
  - [x] U2 OffScreen/MultiViewer unified; snapshot.cpp restored; Tutorial 205 and FigureMaker enabled
  - [x] Shared Viewer/OffScreen capture, MSAA-independent scene depth/object IDs, and MultiViewer ownership IDs; OpenGL primitive IDs remain unavailable.
  - [x] Cell-aware source picking on both backends; shared pixel unprojection and OffScreen query/selection/annotation operations with fixed canvas DPI.
  - [x] U3 Classes without counterpart: same-file #ifdef structure (effects, pickers, ImGui)
  - [x] U4 Python bindings, installed package and CI use the final names (full Mac builds/rendering)
  - [ ] U5 Tutorials/applications: U5a cleanup, U5b collision, U5c camera paths and U5d Vulkan ImGui/Tutorial 202 done; Tutorials 401/402 use shared visible-surface picking; Tutorial 404 now uses shared capture
  - [ ] U6 In progress: shared smoke harness, inventory, drawable constructor homes, shared renderer CMake lists, texture factories, drawable buffer policy unavailable-method ownership and root test consolidation done; remaining file audit/gates open
  - [x] Shared Picker and geometric SurfaceMeshPicker APIs; concave/deleted/transformed faces, regions, DPI and OpenGL query/context isolation.
  - [x] ModelPicker and PointCloudPicker counterparts on both backends.
  - [x] Scene transparency on both backends: depth-sorted straight-alpha triangle pass with preserved depth/IDs; `AverageColorBlending`/`DualDepthPeeling` camera callbacks and Tutorial 504 enabled (batch 13; true accumulation/peeling pending).
  - [ ] W10 Mesh GPU/rasterization parity, Mapple, Qt/wx, remaining Python coverage, SSAO/shadows/EDL (transparency done in batch 13)
  - [ ] Tutorial_207_RealCamera based on Vulkan remains slower than the OpenGL counterpart. Its hover query now reads one pixel rather than a full frame, native `largePoints` reaches about 70 FPS at 1600×1200 with 4× MSAA on this Mac, and the portable expanded-point fallback still needs profiling on MoltenVK devices without that feature.
  
- Send the computed value of "MVP * MANIP" to shaders (avoiding computing the matrix product for each vertex/fragment)
- Walkthrough and animation; allow to modify the cameras interactively
- Implement potree for huge point cloud visualization (https://github.com/potree/potree)
- Add a measuring tool
- Add contents and brief info for each model in WidgetModelList.
- Add tutorials for algorithms:
    - Delaunay2D,
    - simplification
    - subdivision
    - smoothing
    - hole filling
    - curvature
    - parameterization
    - CSG operations
- Check if clipper can be used to handle overlapping faces.
- Generate LODs using [meshoptimizer](https://github.com/zeux/meshoptimizer).
- Transparency on macOS with AMD graphics has artifact along the edges (an issue with dFdx/dFdy in the fragment shader). 
  A workaround is to provide a per-face normal (instead of using the normal computed from dFdx/dFdy);
- Previous timer events may interrupt the current one when visualizing pivot points;



### About deployment on mac:
Missing or Invalid Code Signature:
The application or one of its dependencies is not properly code-signed.

Solution: 
- Re-sign the application and all its dependencies using the codesign tool. This will ensure that all libraries and frameworks are properly signed.
    `codesign --force --deep --sign - Mapple.app`
  - `--force`: Overwrite any existing code signature.
  - `--deep`: Recursively sign all nested binaries and libraries.
  - `--sign -`: Use an ad-hoc signature (for development). Replace - with your code signing identity for distribution.
- Use the codesign tool to verify the code signatures of the application and its dependencies:
  `codesign --verify --deep --strict Mapple.app`
  If there are any issues, the tool will provide detailed error messages.
