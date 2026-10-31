### ToDo list (or on going):
- Vulkan support (experimental). Details, decisions and the ordered work items are in docs/vulkan.md
  ("Current status and work order"). One backend per build (`Easy3D_ENABLE_VULKAN` ON/OFF); user code and
  all tutorials stay the same for both backends (`Viewer`, `MultiViewer`, `OffScreen` from `easy3d/viewer/viewer.h`).
  - [x] Remove "easy3d/graphics" dir (content consolidated into "easy3d/renderer" and "easy3d/viewer")
  - [x] Exclusive backend choice (`Easy3D_ENABLE_VULKAN`), runtime selection and offscreen facade removed
        (the OpenGL*/Vulkan* class names this created are superseded by the principles below)
  - [x] Shared GLFW lifecycle, frame driving, portable input/frame callbacks and camera/surface queries
  - [x] W1-W3 docs sync; build blockers (single home for tutorials, enable_testing, bounded tutorial tests); Tutorial 208
  - [x] W4 Delete runtime-selection/headless leftovers (Backend enum, ViewerOptions, create_*_viewer, ...)
  - [ ] W5 Remove EASY3D_TUTORIAL_USES_VULKAN (tutorials 202, 501-505 gated; 401-406 still have a separate Vulkan path)
  - [ ] W6 Vulkan tests on lavapipe: 68/69 pass; fix vulkan.interactionoverlay border offset; MoltenVK pending
  - Top priority (docs/vulkan.md, "Migration principles"): minimal source changes; same class names and the same
    .h/.cpp files as the main branch (Viewer in viewer.h/.cpp for both backends); #ifdef EASY3D_HAS_VULKAN layout inside
    the classes; no new files except shaders, backend files and tests; minimal CMake changes; client code unchanged.
  - [ ] U1 One `Viewer` class (viewer.h/.cpp), ViewerBase kept as the unified API; delete opengl_viewer.*/vulkan_viewer.*
  - [ ] U2 One `OffScreen` and `MultiViewer` (offscreen.*, multi_viewer.*, snapshot.cpp as in main); Vulkan MultiViewer;
        re-enable Tutorial 205 and FigureMaker
  - [ ] U3 Classes without counterpart: same-file #ifdef structure (effects, pickers, ImGui)
  - [ ] U4 Python bindings, installed package and CI use the final names
  - [ ] U5 Tutorials and applications identical to main; no backend conditionals
  - [ ] U6 Diff audit against main (files, CMake); remove temporary gates
  - [ ] W10 Mapple, Qt/wx, Python rendering, transparency/SSAO/shadows/EDL

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
