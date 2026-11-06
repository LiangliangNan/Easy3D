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
  - [ ] W6 Mac: OpenGL 91/91; MoltenVK 127/127 (2026-11-05, W10 batch 23; `vulkan.imgui`/`opengl.imgui` can fail once under full-suite load and pass alone; prior intermittent window-test timeout remains under observation); Linux baseline 68/69 still needs the interactionoverlay border check
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
  - [x] Shared Picker and SurfaceMeshPicker APIs on both backends; deleted/transformed faces, regions, DPI and OpenGL query/context isolation.
  - [x] ModelPicker and PointCloudPicker counterparts on both backends.
  - [x] Scene transparency on both backends: depth-sorted straight-alpha triangle pass with preserved depth/IDs; `AverageColorBlending`/`DualDepthPeeling` camera callbacks and Tutorial 504 enabled (batch 13; true accumulation/peeling pending).
  - [x] Eye-dome lighting on both backends: depth-sampled multiplicative overlay, `EyeDomeLighting` begin/end frame params and Tutorial 505 enabled (batch 14).
  - [x] Ambient occlusion on both backends: shared `AmbientOcclusion` begin/end contract and Tutorial 501 (batch 15), now horizon-based (GTAO) on both backends: both reconstruct view-space positions and normals from the projection buffer's depth; Vulkan runs a single-sample depth+ID prepass followed by GTAO and denoise compute dispatches, OpenGL runs depth-only fragment passes (batch 24).
  - [x] Hard and soft shadows on both backends: Vulkan depth-only shadow map with PCSS overlay, shared `Shadow`/`SoftShadow` begin/end contract, OpenGL original passes preserved and Tutorials 502/503 enabled with their gates removed (batch 16). Matching quality (batch 21): both hard paths filter four ±0.75-texel taps, Vulkan reproduces `sampler2DShadow` comparison-then-bilinear filtering, PCSS kernels rotate per pixel (IGN) and clamp radii to one texel; Vulkan uses 32 blocker/64 filter taps versus OpenGL's 64/128 default.
  - [x] Coplanar mesh edges over filled faces on both backends: Vulkan dynamic depth bias mirrors the OpenGL `glPolygonOffset(0.5, -0.0001)` per model with visible lines; picking stays unbiased (batch 17).
  - [x] GPU-only picking on both backends: OpenGL `Viewer::pick()` and `SurfaceMeshPicker` rasterize visible triangles with the primitive-ID shader and map IDs through `f:triangle_range`, honoring `MultiViewer` cell viewports and clipping; CPU ray casts, CPU point fallback and capability flags removed (batch 19).
  - [x] GPU point-cloud region selection on both backends: OpenGL `PointCloudPicker` uses the transform-feedback word shader over the rendering VBO, Vulkan uses `RenderDevice::select_points()`, detached clouds run on a synthetic NaN-padded GPU snapshot, and the archived CPU implementations stay under `#if 0` (batch 20).
  - [ ] W10 Mesh GPU/rasterization parity, Mapple, Qt/wx, remaining Python coverage (transparency done in batch 13 and per-triangle ordering in batch 18; EDL done in batch 14; ambient occlusion done in batch 15 and upgraded to GTAO in batch 24; shadows done in batch 16; mesh edge depth-bias parity done in batch 17; GPU-only picking parity done in batch 19; point-cloud region selection done in batch 20; shadow quality parity done in batch 21; Python mesh drawable-state coverage on both packaged modules done in batch 25, including unlit highlight parity)
  - [ ] Tutorial_207_RealCamera based on Vulkan remains slower than the OpenGL counterpart. Its hover query now reads one pixel rather than a full frame, native `largePoints` reaches about 70 FPS at 1600×1200 with 4× MSAA on this Mac, and the portable expanded-point fallback still needs profiling on MoltenVK devices without that feature.
  - [ ] TrianglesDrawable::opacity() is effective through the scene transparency path: straight alpha blends over the opaque scene, depth/IDs are written, and triangles are sorted far-to-near per drawable and per triangle (batch 18). Order-independent accumulation remains pending; the legacy AverageColorBlending/DualDepthPeeling classes still map to the same sorted pass (true accumulation/peeling pending).
  - [x] Ambient occlusion quality (batch 24): both backends replace the sparse-kernel SSAO and box blur with GTAO — eight azimuthal slices per pixel, horizon marching along the projected sample radius, cosine-weighted visibility and a normal/plane-aware bilateral denoise. Samples are limited to a spherical kernel, so the depth-jump dark strips that the old non-vanishing `clamp(radius/distance)` weight produced at small radii are gone. Vulkan computes AO and denoise in compute shaders from a single-sample depth prepass and composites the denoised texture; OpenGL runs the same horizon marching as fragment passes over the projection buffer's depth, with no geometry pass or G-buffer, which removes the second scene traversal and the large OpenGL frame-time gap. Background pixels stay untouched and the focused AO regression plus Tutorial 501 visual checks pass on both backends.
  - [x] Hard/soft shadow quality parity (batch 21): hard shadows use four ±0.75-texel PCF taps on both backends (Vulkan compares 2x2 texels and blends the results like `sampler2DShadow`); PCSS searches/filters 32/64 taps on Vulkan and 64/128 on OpenGL with per-pixel interleaved-gradient rotation and one-texel radius floors. Tutorial 502/503 snapshots on OpenGL show smooth edges and no floor moiré. Remaining approximation: Vulkan's lower tap count can leave slightly more noise in very large penumbrae.
  - [ ] Vulkan snapshots do not include effects that a tutorial brackets inside `draw()` (ambient occlusion, shadows, eye-dome lighting, transparency): `snapshot.cpp` renders `frame_commands()` through `RenderDevice::render()` and does not call the virtual `draw()`. `Viewer::capture()` intentionally stays a raw scene capture on both backends because tests and picking compare geometry/depth/IDs through it.
  
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
