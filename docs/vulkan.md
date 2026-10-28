# Native Vulkan migration

The `Easy3D-Vulkan` branch starts an incremental migration. It is **not yet a
replacement for the full OpenGL renderer**. A native interactive Vulkan viewer is
now enabled by default on this development branch. Mapple, FigureMaker, legacy GUI picking/effects, Python
rendering and most existing interactive tutorials still use OpenGL. They are retained until their Vulkan replacements are ready.

## Final migration goal and backend policy

The completed migration must support **all tutorials on Vulkan**, and targets
**all applications on Vulkan**, including Mapple, FigureMaker and toolkit-based
hosts. Porting the renderer alone is not completion: the rendering features,
interaction, picking, effects, text, snapshots and custom drawing used by those
programs must also work. Any remaining application limitation must be explicitly
tracked rather than silently excluded from the migration scope.

Users should be able to select Vulkan or OpenGL through a common public API and
consistent application configuration. The intended default is automatic
selection: prefer Vulkan, and use OpenGL if Vulkan initialization is unavailable
and OpenGL was built and is available. This includes missing loaders/drivers,
unsuitable devices or required capabilities, and unsupported host/surface setup.
An explicitly requested backend must either initialize successfully or report an
actionable error; it must not silently switch backends. Arbitrary rendering or
application errors must not trigger fallback and conceal defects. Automatic
fallback should identify the backend selected and explain why Vulkan was
unavailable. If neither backend can initialize, report both failures.

Backend selection and fallback are final-state requirements, **not implemented
by the current compile-time Viewer/OffScreen aliases**. Supporting automatic
fallback requires a dual-backend build and shared viewer/offscreen interfaces,
scene state and host lifecycle that can choose a backend before creating its
rendering resources. Vulkan-only and OpenGL-only builds should remain supported,
with clear diagnostics when the requested backend was not built.

Tutorials and applications should share their application logic across backends.
Replace backend-specific drawing callbacks with common rendering operations where
needed; retain explicit low-level backend APIs for advanced use. The maintained
OpenGL backend remains a supported choice and fallback after Vulkan parity.
Retiring obsolete implementation details must not remove that support.

Source layout is also a migration requirement. Keep Vulkan tutorial/application
code as close as practical to its OpenGL counterpart: retain the original class
names, responsibilities and `main.cpp` / `viewer.h` / `viewer.cpp` organization.
Use shared declarations and implementation files, with narrow backend-specific
sections where the graphics APIs differ. Prefer shared scene/control logic over
parallel class implementations. Explicit Vulkan device, pipeline, shader and
synchronization code may still need backend-specific implementation files.

Completion gates include:

- Build and run every tutorial with Vulkan, validating the features each teaches.
- Port every application and its host integration, checking feature parity and
  documenting any outstanding limitations.
- Exercise explicit Vulkan, explicit OpenGL and automatic selection in shared
  tutorial/application paths, including Vulkan initialization failure, fallback
  cleanup and the case where neither backend is available.
- Validate rendering, picking, MSAA, snapshots and resource lifetime on supported
  macOS, Windows and Linux configurations; retain OpenGL regression coverage.

## Implemented

- Backend-neutral, immutable geometry and image descriptions, draw commands and
  typed color/depth/object/primitive readback in `easy3d/graphics/render_device.h`.
- A native Vulkan 1.2 offscreen device, with no OpenGL context, window, surface,
  swapchain, Qt, GLFW or GLEW dependency in the `easy3d::vulkan` target.
- Indexed/nonindexed triangle lists, plain points and line lists, uniform and
  vertex colors, RGBA8 textures, basic directional lighting and depth testing.
- Portable large plain points, spheres and oriented surfels using six-vertex instanced
  quads, without geometry shaders or the optional `largePoints` feature. Sphere
  fragments compute perspective/orthographic ray intersections, silhouette discard
  and actual surface depth, retaining original point primitive IDs. Surfels use
  ray/plane disc intersections and inverse-transpose normal transforms.
- Portable plain wide lines using instanced, butt-capped screen-space ribbons,
  homogeneous near/far/eye clipping and normal triangle MSAA/depth. No `wideLines`
  feature, geometry shader or sample-rate shading is required for ribbons.
- Capped cylinder and cone line impostors with analytic side/cap intersections,
  actual surface depth, endpoint color/texture interpolation and original segment
  IDs, using instanced quads without geometry shaders or wide-line features.
- Screen-space triangle/image overlays and depth-tested translucent annotations,
  composed in color-only subpasses before final MSAA resolve. Neither writes
  scene depth or picking IDs; no independent-blend feature is required. Tutorial
  306 provides a native image viewer, and Tutorial 307 has a transparent filled
  clipping-plane guide.
- Per-viewer world-space clipping with up to two halfspaces/slabs, geometric
  triangle/line clipping, analytic impostor surface clipping and original source
  IDs. Tutorial 307 provides native plane/cross-section controls.
- Native visible-surface model/drawable picking with original SurfaceMesh face
  provenance, world points and retained source ownership. Model/drawable selection
  and primitive-range highlights work on textured geometry and every native style.
  Tutorials 401/402 provide native model/face picking hosts.
- CPU TrueType/UTF-8 text layout and immutable glyph atlases, using the native
  overlay path. Tutorial 309 provides font, kerning, spacing, alignment and origin controls.
- Shared CPU OBJ/MTL diffuse-material import, preserving material groups and UV
  seams. Tutorial 308 uses the same loader and scene logic on Vulkan and OpenGL.
- CPU-owned immutable texture data with explicit CPU-only factories, shared
  texture/scalar drawable state, vertex/halfedge UV extraction and default texture/
  scalar selection. Tutorials 303 and 305 run with Vulkan using the existing scene APIs.
- Per-image clamp/repeat and nearest/linear sampler descriptors, including a valid
  white fallback texture.
- Device-local geometry/images, staged uploads, coherent/noncoherent host memory,
  fenced submission/readback, explicit image transitions and transfer barriers.
- Independent resource realizations on multiple devices. CPU snapshots own their
  input; temporary vectors are safe. Replacing a snapshot uploads a new resource;
  expired realizations are retired after completed submissions.
- Per-device caches, transactional resource creation, resize and cleanup.
- Device-queried MSAA for capture and presentation. Color resolves use Vulkan's
  standard multisample resolve; a separate center-sample ID/depth pass preserves
  exact picking semantics without integer-MSAA or depth-resolve feature requirements.
- Vulkan-to-Metal portability enumeration and the portability subset extension
  when advertised. No geometry shaders, wide-line features or private Metal API.
- Build-time GLSL450 compilation and embedded SPIR-V: installed applications need
  a Vulkan loader/driver, but no shader compiler or source-tree shader paths.
- Optional `VulkanOffScreen`, reusing Easy3D Camera, Model, Renderer and drawable
  buffer extraction, including in Vulkan-only builds with no OpenGL link dependency.
  It creates no OpenGL context and runs with `DISPLAY` unset.
- Shared drawable constructors and CPU geometry generation separated from legacy
  OpenGL uploads/drawing in `renderer/backends/opengl`. Vulkan-only builds exclude
  legacy rendering sources and shader installation entirely; GL-only entry points
  report an explicit unavailable-operation error.
- A native `VulkanViewer`/GLFW host (enabled by default): GPU-only swapchain presentation,
  resize/hide/show, independent windows/devices, orbit/pan/zoom, projection toggle,
  file drop and depth-based world-point picking. The installed demo uses an
  embedded bunny by default and needs no source-tree model/shader files.
- Existing Tutorials 201, 301–312 and 401–404 build with the native Vulkan host in
  Vulkan-only window builds. Tutorial 302 uses the same sphere/surfel/cylinder/cone scene code
  as its OpenGL build, with bounded native execution for automated tests. Legacy
  custom OpenGL draw callbacks are not ported. Tutorial 304 exercises the native
  vector-field and plain wide-line path.
- A new finite Tutorial 208 and deterministic Vulkan tests, including actual
  bunny mesh loading, property colors, geometry updates, snapshots and picking IDs.

## Build and run

Fresh configurations default to `Easy3D_ENABLE_VULKAN=ON`,
`Easy3D_ENABLE_VULKAN_VIEWER=ON`, `Easy3D_ENABLE_OPENGL=OFF`, and
`Easy3D_BUILD_TUTORIALS=OFF`. Existing CMake caches retain their saved values;
reset the CLion/CMake cache or pass explicit `-D` options to update them.

Install the Vulkan SDK (headers, loader and `glslangValidator`). On Ubuntu:

```sh
sudo apt-get install libvulkan-dev mesa-vulkan-drivers glslang-tools \
    spirv-tools vulkan-validationlayers libboost-dev
```

For the complete existing build plus the experimental adapter/tutorial, also
install the usual Easy3D OpenGL/window-system dependencies, then:

```sh
cmake -S . -B build-vulkan -DEasy3D_ENABLE_VULKAN=ON \
    -DEasy3D_ENABLE_OPENGL=ON -DEasy3D_BUILD_TUTORIALS=ON \
    -DEasy3D_BUILD_VULKAN_TESTS=ON -DEasy3D_VULKAN_TEST_VALIDATION=ON
cmake --build build-vulkan --parallel 4
ctest --test-dir build-vulkan --output-on-failure -LE desktop
./build-vulkan/bin/Tutorial_208_Vulkan_Offscreen [mesh.ply] [output.png]
```

For CPU scene APIs, `VulkanOffScreen` and the Vulkan module, with **no GL/GLEW/GLFW/ImGui build**:

```sh
cmake -S . -B build-vulkan-only -DEasy3D_ENABLE_OPENGL=OFF \
    -DEasy3D_ENABLE_VULKAN_VIEWER=OFF \
    -DEasy3D_ENABLE_VULKAN=ON -DEasy3D_BUILD_TUTORIALS=OFF \
    -DEasy3D_BUILD_TESTS=OFF -DEasy3D_BUILD_PYTHON_BINDINGS=OFF \
    -DEasy3D_ENABLE_QT=OFF -DEasy3D_BUILD_VULKAN_TESTS=ON \
    -DEasy3D_VULKAN_TEST_VALIDATION=ON
cmake --build build-vulkan-only --parallel 4
ctest --test-dir build-vulkan-only --output-on-failure
```

Validation is explicitly requested by the last option. Missing validation layers
cause tests to fail instead of silently running without validation. Leave that
option OFF for deployments without the SDK/layers. Synchronization validation
uses `VK_EXT_validation_features` from the Khronos validation layer.

For deterministic Linux software tests, set `VK_DRIVER_FILES` to the installed
lavapipe ICD JSON (often `/usr/share/vulkan/icd.d/lvp_icd.x86_64.json`; distributions
use different filenames). No display server is required. This tests Vulkan
correctness, not GPU performance or macOS portability.

On macOS use a Vulkan SDK with MoltenVK and a matching loader/ICD. Follow the SDK's
runtime environment/bundling instructions. This implementation requests Vulkan
1.2 and checks target-format support. The point/impostor stage has been tested on this Mac with MoltenVK 1.4.2
and Apple M4 Pro. Windows hardware execution remains outstanding.

## Interactive Vulkan window

The window host is enabled by default. In a fresh build directory:

```sh
cmake -S . -B build-vulkan-window -DEasy3D_ENABLE_VULKAN=ON \
    -DEasy3D_ENABLE_VULKAN_VIEWER=ON -DEasy3D_ENABLE_OPENGL=OFF \
    -DEasy3D_BUILD_TUTORIALS=OFF
cmake --build build-vulkan-window --parallel 4
./build-vulkan-window/bin/VulkanViewer
# Or load your own model:
./build-vulkan-window/bin/VulkanViewer path/to/model.ply
```

Use a separate build/install directory and rebuild consumers: these interfaces
remain experimental, with source compatibility for the documented subset.
GLFW's normal platform development dependencies are required (X11 or Wayland on
Linux; native Cocoa/Win32 on macOS/Windows). The host requests `GLFW_NO_API` and
never creates an OpenGL context. `Easy3D_ENABLE_OPENGL=ON` is also supported when
you want to compile the existing applications alongside the Vulkan demo.

Controls: left drag rotates, right/middle drag pans, the wheel zooms, **F** fits
all models, **P** toggles perspective/orthographic, **F2** pauses/resumes an
available animation, and **Escape** closes. Shift-left
click sets the camera pivot to a depth-picked surface point. You can also drop a
model file on the window. `--frames N` bounds demo execution; `--validation`
requires the Khronos and synchronization validation layers. Use `--samples 4`
to request four-sample color antialiasing (the device must support it).

For C++ consumers, link `easy3d::viewer` and use:

```cpp
#include <easy3d/viewer/vulkan_viewer.h>
easy3d::VulkanViewer viewer("Mesh");
viewer.add_model("mesh.ply");
viewer.run();
```

In a Vulkan-only window build the existing `<easy3d/viewer/viewer.h>` and
`easy3d::Viewer` name select the same host. `Tutorial_201_Viewer_default` is built
unchanged in that configuration. Tutorials 301–312 and 401–404 are also built in this configuration. Other tutorials remain gated
until their features and callbacks are ported; the global legacy tutorials option stays OFF.
In a dual build `Viewer` continues to select the legacy host, while
`VulkanViewer` selects Vulkan explicitly.

The host reuses Camera/Model/Renderer/Drawable and snapshot APIs from
`VulkanOffScreen`. `draw_frame()` plus `poll_events()` support an external loop;
input and pre/post-frame/resize hooks can be overridden. All native callbacks use
framebuffer-pixel coordinates, including HiDPI conversion. `point_under_pixel()`
uses the same top-left pixel convention and synchronously reads back depth on
request. Legacy GL `draw()` callbacks, overlays and full viewer shortcut parity
are not emulated. GL-version constructor arguments have no native counterpart.
Use `VulkanViewerOptions.samples` or `set_samples()` for MSAA.

Create/use/destroy GLFW windows on the UI/main thread. Multiple Vulkan windows
share only GLFW lifetime, not their logical GPU devices or mutable scenes. Do not
mix these windows with the legacy GLFW Viewer in one process, whose GLFW lifetime
management is separate. A custom toolkit can supply required instance extensions
and a surface callback through `graphics::VulkanSurface`; the device owns the
returned surface and must be destroyed before its native window. Borrowed
Qt-owned VkDevices are not supported by that adapter yet.

Window tests require a display, unlike the renderer/capture tests:

```sh
ctest --test-dir build-vulkan-window --output-on-failure -LE desktop
# Linux software WSI tests (with VK_DRIVER_FILES set to lavapipe):
xvfb-run -a ctest --test-dir build-vulkan-window --output-on-failure -L desktop
```

Enable `Easy3D_BUILD_VULKAN_TESTS` and optionally
`Easy3D_VULKAN_TEST_VALIDATION` when configuring to build these tests. The Vulkan
window demo runs on the actual desktop on macOS/Windows; Xvfb is Linux-only.

## Similar existing APIs

```cpp
#include <easy3d/viewer/vulkan_offscreen.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>

// Link easy3d::viewer from either a Vulkan-only or dual-backend build.
easy3d::VulkanOffScreen renderer(800, 600);
auto* model = renderer.add_model("bunny.ply");
if (model) {
    auto* faces = model->renderer()->get_triangles_drawable("faces");
    faces->set_uniform_coloring(easy3d::vec4(0.7f, 0.85f, 1, 1));
    renderer.camera()->showEntireScene();
    renderer.render("bunny.png");
}
```

Standalone drawables must enter CPU-buffer mode **before** their first buffer
update. Add them to `VulkanOffScreen` first, then supply geometry. Alternatively
call `drawable->enable_cpu_buffers()` before updating it. A drawable with existing
GL buffers cannot be transferred by this API; re-create it from its CPU data.
`Renderer(model, create_drawables, true)` creates CPU-backed default and subsequent
custom drawables. The default third argument remains false for existing OpenGL clients. In a
Vulkan-only build all drawables use CPU buffers automatically.

In a Vulkan-only build, the existing include `<easy3d/viewer/offscreen.h>` and
class name `easy3d::OffScreen` select this same native adapter. This preserves
common offscreen source usage without introducing a window or GL context. In a
dual build, use `VulkanOffScreen` explicitly; `OffScreen` retains its old backend.
This is source compatibility for the documented subset, not binary compatibility
or parity for GL-specific inheritance/callbacks and unsupported options.

The adapter uses the existing geometry extraction/tessellator, including polygon
faces and property coloring. Call `model->renderer()->update()` after geometry or
property changes, as before. Existing GL upload behavior stays immediate; only
explicit CPU-mode drawables retain CPU buffers. CPU mode is permanent for that
drawable. `draw(camera)`/`gl_draw()` fail explicitly in CPU mode.

For a Vulkan-only consumer, link `easy3d::vulkan`, build immutable
`graphics::Geometry`/`graphics::Image` descriptions, and submit a vector of
`graphics::DrawCommand` to `graphics::RenderDevice::render()`. See
`easy3d/graphics/tests/render_test.cpp` for a complete runnable example. The CMake
package exports the `vulkan` component and finds its Vulkan dependency.

## Multisampling and test diagnostics

`VulkanOffScreen::capture(samples)` and `render(filename, scaling, samples, ...)`
now support the device's RGBA8/D32 sample counts. The default remains one sample.
`RenderDevice::render()` and `present()` also accept a trailing sample-count
argument. Query `device().supported_sample_counts()` before offering a count:
`mask & 4` means four samples are available. Unsupported counts are rejected
explicitly rather than silently reduced.

For a window, set `VulkanViewerOptions.samples` at construction or call
`viewer.set_samples(4)`. `snapshot()` and no-argument `capture()` use the window's
current count. Changing the count does not recreate the swapchain; render passes
and pipelines are cached by pass kind and sample count, and framebuffer resources
are recreated safely. Rendering remains synchronous at this stage.

For MSAA, color and depth use multisampled attachments and color is resolved into
the normal output image. IDs and picking depth are rendered at pixel centers in a
second pass. This costs an extra geometry pass but works when integer formats
support only one sample, requires no depth-resolve extension, and never averages
object/primitive IDs. A partially covered color edge may have no center-sample
picking hit; this is intentional. Raw capture returns resolved RGBA values;
transparent-background image saving converts coverage-premultiplied edges to
straight alpha so PNG compositing does not produce dark fringes.

The previous scene test intentionally called the logging `render()` API with
unsupported four-sample MSAA, so it printed an error and then passed. That check
is now a positive four-sample image test where supported. Invalid sample-count
checks use the throwing capture/device APIs and catch the expected exceptions;
normal successful tests print no error diagnostics. Callback-failure tests use
the same approach. Real `render()` failures still return false and log their
cause.

## Current boundaries

| Area | Current behavior |
|---|---|
| Default backend | Native Vulkan/GLFW by default; `Viewer`/`OffScreen` alias their Vulkan implementations. Explicitly enabling OpenGL restores legacy `Viewer`/`OffScreen` in a dual build. |
| `VulkanOffScreen` defaults | Opaque mesh faces; uniform/property, texture-coordinate and scalar-property defaults follow the shared renderer selection. Plain one-pixel points/lines remain the initial style. |
| Material/lighting | Basic fixed world-space diffuse term; no full material or back-face-color parity yet. Normal transforms use the inverse transpose. Disable lighting for exact unlit colors. |
| `TrianglesDrawable::opacity()` | As in the old opaque pass, effective only in a separate transparency pass; such a pass is not implemented here. |
| CPU drawable textures/scalars | `Texture` pixel snapshots, vertex/halfedge UVs and scalar fields are supported without GL calls. Clamp/repeat, nearest/linear filtering and OBJ/MTL diffuse-material groups work; no mipmap, anisotropic or PBR material pipeline yet. |
| Transparency, SSAO, shadows, EDL, text, clipping | General 3D transparency/effects remain pending. Straight-alpha 2D overlays, depth-tested translucent annotations and basic horizontal TrueType/UTF-8 text are supported; shaping, bidi and automatic font fallback remain pending. The scene adapter still rejects SSAO; overlays are not an effect-pass API. |
| Clipping | Up to two per-viewer world-space halfspaces. Geometric triangle/line-centerline cuts, center-clipped plain points and front-surface analytic clipping preserve IDs/MSAA. No automatic cut caps; native plane guides use depth-tested transparent fill and outlines. |
| Point and line styles | Plain points of any positive pixel size, spheres and oriented surfels are implemented. Capped cylinders/cones and butt-capped plain wide lines are implemented; joined strokes and rounded plain-line caps remain pending. |
| Snapshot options | Device-supported MSAA (default 1); `expand=false` remains unsupported. Scaling, white/current/transparent background and image saving work. No tiled oversized captures. |
| Readback | RGBA8, D32 and RG32UI; top row first; depth [0,1]; no-hit object/primitive IDs are `UINT32_MAX`. |
| Picking IDs | Raw captures retain model/standalone order and generated primitive indices. `VulkanOffScreen::pick()` maps a visible hit to its model/drawable and original SurfaceMesh face when provenance exists. GPU point-cloud rectangle/lasso through-selection is supported. General vertex/edge hit provenance and the legacy GUI picker classes remain pending. |
| Selection/highlighting | Renderer/drawable selection and inclusive primitive ranges use the shared highlight color, after native lighting/texturing; alpha, depth, IDs and MSAA coverage are preserved. |
| Resource scheduling | Synchronous capture/uploads and one submitted frame at a time; presentation stays on the GPU with no CPU pixel readback. Per-image presentation semaphores; queue/device idle only at swapchain recreation and destruction. Multiple frames in flight and asynchronous capture are pending. |
| Resource limits | Fixed 1,024 live texture descriptor sets; expired resources retired at the next capture. Individual allocations, not a suballocator yet. |
| Scene ownership | Like Viewer, adding a model replaces its renderer. Do not simultaneously attach the same mutable model to GL and CPU-mode hosts. Native immutable geometry can be reused across independent devices. |
| Threading | Externally synchronize device calls and model edits. CPU snapshots do not make concurrent model/property mutation safe. |
| Hosts/bindings | Native GLFW is implemented; Qt's two views, wxWidgets, ImGui and Python Vulkan integration remain pending. |
| Presentation requirements | One queue family supporting both graphics and presentation, transfer-destination surface images, RGBA/BGRA blit support, identity surface transform. Unsupported configurations fail explicitly. FIFO presentation. UNORM formats are preferred; an sRGB-only surface introduces format conversion. |

The existing OpenGL sources and shaders therefore remain necessary. Cleaning them
out now would remove working applications and tutorials. The Vulkan-only build
excludes legacy renderer/GUI/viewer implementations, GLEW and ImGui. The headless
configuration also excludes GLFW; the optional native window host includes GLFW
for window/input management. Both retain shared CPU scene APIs and the new offscreen adapter. Retire obsolete
implementation details only after their replacements pass application/feature
tests, while retaining the supported OpenGL backend and fallback path.

## Validation and remaining migration

The new CI workflow builds both Vulkan-only and dual configurations, validates
SPIR-V and renders with lavapipe and synchronization validation without X/Wayland.
It also tests native window configurations under Xvfb, including resize, multiple
devices/windows, callback failure containment and independent window lifetime.
The existing legacy build workflows explicitly select OpenGL and disable Vulkan;
the Vulkan workflow covers headless, native-window and dual configurations.

Local verification for this stage used Linux, GCC 14, Qt 6.8.2 and Mesa lavapipe:
Vulkan-only and dual-backend rendering tests passed with Khronos validation and
synchronization validation; the installed-package consumer rendered successfully;
SPIR-V validation passed; native shared libraries had no GL/GLEW/Qt linkage or
imported GL calls, and the headless configuration excluded GLFW. The enabled legacy tutorials, FigureMaker and Mapple
compiled, and all 34 automated legacy test groups passed under Xvfb. This does not
validate native macOS/Windows execution, interactive Qt behavior, or Python
rendering on Vulkan. The new GitHub workflow is added but has not been run on
GitHub as part of this local work.

The native-window stage additionally passed headless and Xvfb presentation tests
with synchronization validation, including live window-image inspection. Tutorial
201 builds unchanged through the native Viewer alias. Installed-resource
validation no longer requires legacy GL shaders in Vulkan-only packages.

The MSAA stage verifies real partial edge coverage at four samples, unchanged
center-sample IDs/depth for triangles/points/lines, sample-count changes and resize,
transparent PNG edges, and window presentation. Expected negative tests no longer
use the logging wrappers. Fresh-configure checks verify the new four defaults.
The subsequent point/impostor stage also validates this MSAA path on macOS/MoltenVK; Windows validation remains outstanding.

Next implementation gates are native Qt/wx surface/frame adapters,
ID mapping/pickers, oversized capture,
effects/transparency, all applications/tutorials and Python integration, then
hardware validation and packaging on macOS/Windows/Linux. The defaults now select
Vulkan for development. Final backend selection and automatic OpenGL fallback
remain migration gates; the OpenGL backend will remain supported.

## Portable points, spheres and surfels

`PointsDrawable::PLAIN` accepts positive finite framebuffer-pixel sizes; sizes
other than one use instanced triangle quads and never request `largePoints`.
`PointsDrawable::SPHERE` uses a view-space radius of
`point_size() * camera.pixelGLRatio(camera.pivotPoint()) / 2`, matching the legacy
radius convention. Manipulation transforms the center; radius is independent of
model scale. Model/default drawable styling remains plain one-pixel points until
explicitly changed. `PointsDrawable::SURFEL` now renders oriented circular discs
with the same radius convention. Supply point normals (for example `v:normal`
on a point cloud); missing or zero normals produce explicit errors.

Native `DrawCommand` clients set `point_size` for plain points, or set
`point_style = PointStyle::sphere`, `sphere_radius`, `model_view` and `projection`
for spheres. For discs, use `point_style = PointStyle::surfel` with the same
radius/matrices, plus a nonzero normal in each referenced `Vertex`. The latter matrices use the Easy3D/OpenGL convention; sphere centers
are transformed by `model_view`, and `projection` must be invertible. The existing
`model_view_projection` continues to control plain points/lines/triangles. Sphere
lighting uses `sphere_light` in view space; the scene adapter transforms the fixed
world-space light by the camera rotation. This is basic diffuse lighting, without
full legacy material parity. Uniform/vertex colors and native textures work.

The sphere proxy conservatively bounds the projected view-space cube, falling
back to the viewport when it crosses the eye plane. Its rasterization depth does
not clip the true surface. Fragments intersect a ray reconstructed from the
inverse projection, discard misses and clipped front surfaces, and write the
surface depth. This handles off-axis perspective views and orthographic views;
near-plane clipping can expose only a rim, and does not add a solid cap.

Color MSAA evaluates sphere intersections/depth at each sample using queried and
enabled `sampleRateShading`. Devices lacking this optional feature support
single-sample spheres and all plain-point sizes; multisampled sphere draws fail
explicitly instead of rendering incorrect coverage/depth. Query
`device.supported_sample_counts(PointStyle::sphere)` for the usable counts.
Picking always uses
the existing separate center-sample pass, with the same sphere intersection and
surface depth, independently of the color sample count. Proxy triangles never
become picking primitive IDs. Per-draw matrices/parameters use an aligned dynamic
uniform buffer; push constants remain within Vulkan's guaranteed 128-byte limit.
These sample-shading requirements also apply to surfels; query
`device.supported_sample_counts(PointStyle::surfel)` before selecting MSAA for discs.

Local hardware verification on 2026-10-27 used the existing Debug configuration,
Apple M4 Pro and **MoltenVK 1.4.2**, explicitly selected with
`VK_DRIVER_FILES=/usr/local/share/vulkan/icd.d/MoltenVK_icd.json`. Renderer and
scene tests passed with `--validation` (Khronos plus synchronization validation);
native window/presentation tests also passed with sphere depth picking, style
switches, resize and multiple windows. Both new shaders passed
`spirv-val --target-env vulkan1.2`. Tests cover pixel
sizes, indexed/explicit point IDs, analytic sphere depth and silhouettes,
mesh/sphere occlusion in both draw orders, off-axis perspective and orthographic
views, near/far/eye clipping, four-sample coverage, unchanged center-sample
picking/depth, style changes, resize and independent draw parameters.

This MoltenVK version emits a known spurious warning about blending on the
RG32UI ID attachment even though blending is disabled. No validation errors are
reported. The upstream fix checks `blendEnable` before issuing that warning:
[MoltenVK diagnostic fix](https://skia.googlesource.com/external/github.com/KhronosGroup/MoltenVK/+/4aaf714aa1b3e78e26ecfcefa9c75e9a576c500b%5E2..4aaf714aa1b3e78e26ecfcefa9c75e9a576c500b/).

### Oriented surfel stage

Surfels reuse the instanced quad and conservative projected cube bounds. Each
fragment intersects its camera ray with the disc plane, rejects hits outside the
radius, and writes the projected plane depth. No geometry shader, large-point
feature or mesh approximation is used. Perspective and orthographic views, both
disc faces, texture/uniform/vertex colors and the existing diffuse lighting are
supported. Legacy material/back-face-color parity remains pending; the later
clipping stage adds scene planes.

The disc normal is transformed with the inverse transpose of the affine
`model_view` linear part, independently of whether lighting is enabled. Its
linear part must be invertible. Model scaling moves centers and changes normal
orientation while the radius stays in view-space units, as for spheres. Surfels
that are exactly edge-on have zero projected area and are discarded. Near/far
clipping preserves the visible portion of a tilted disc. Picking retains the
original point ID and plane depth in the center-sample pass.

On 2026-10-27 the renderer, scene and native-window regressions passed on the
Apple M4 Pro with MoltenVK 1.4.2, Khronos validation and synchronization validation.
The point shaders also passed SPIR-V validation. New checks cover an independent
CPU ray/plane oracle, nonuniform normal transforms with lighting disabled,
explicit indexed IDs, mesh/disc occlusion in both draw orders, reversed normals,
clipping, edge-on discs, invalid normals/transforms, four-sample silhouette
coverage, unchanged picking/depth, scene normal-property updates and native
presentation/depth picking while switching sphere/surfel/plain styles. The
previously documented MoltenVK integer-attachment warning persists; validation
error counts remain zero.

This completed the three `PointsDrawable` styles. The subsequent cylinder/cone
stage below enables Tutorial 302 in full on Vulkan. Native Qt/wx host integration
and the other roadmap gates remain outstanding.

## Cylinder/cone line impostors and Tutorial 302

`LinesDrawable::CYLINDER` and `LinesDrawable::CONE` are now supported by the native
scene adapter. Radius is `line_width() * camera.pixelGLRatio(camera.pivotPoint()) / 2`,
matching the existing radius convention. Cylinders have two flat caps; cones have
a base cap at the first endpoint and taper to the second endpoint. Model/view
transforms move the endpoints, while radius remains in view-space units. A
collapsed segment draws nothing. Plain lines use the separate ribbon path below
when their width differs from one.

Native clients use `LineStyle::cylinder` or `LineStyle::cone`, set `line_radius`,
and provide an affine `model_view` plus an invertible OpenGL-convention
`projection`. The shared `sphere_light` field supplies the view-space light for
all impostors. Query `supported_sample_counts(LineStyle::cylinder)` or
`LineStyle::cone`; multisampled line impostors require `sampleRateShading`, with
an explicit error on unsupported devices. Single-sample rendering works without
that optional feature.

The device reads each pair of packed line endpoints as one instance and emits a
six-vertex quad bounding their projected solid. Fragment ray intersections choose
the nearest side/cap surface and write its projected depth. Stable cross-product
discriminants avoid cone-tip/grazing cancellation. Near/far clipping follows the
front surface without revealing a back surface or introducing a clip-plane cap.
Colors and native texture coordinates interpolate along the solid's axis, rather
than across its proxy triangles. Primitive IDs stay those of the original line,
including indexed geometry and explicit IDs. Full legacy material parity,
rounded caps and arbitrary profile/frustum styles are not provided by this
stage. The later clipping stage adds scene planes.

Vulkan-only window builds now produce `Tutorial_302_Imposters`, alongside Tutorial
201, even while the global legacy tutorials option is OFF. The tutorial retains
its shared scene construction code for both backends. Its native build supports:

```sh
./cmake-build-debug/bin/Tutorial_302_Imposters --frames 3 --samples 4 --validation
```

Without `--frames`, it remains interactive. The desktop CTest
`vulkan.tutorial302` runs a bounded three-frame tutorial; enabling
`Easy3D_VULKAN_TEST_VALIDATION` also enables its Khronos/synchronization checks.
Validation errors make the tutorial smoke test fail. The Vulkan CI workflow
validates both new line shaders and runs this test in native-only window jobs.

Local verification on 2026-10-27 used Apple M4 Pro and MoltenVK 1.4.2, explicitly
selected with `VK_DRIVER_FILES=/usr/local/share/vulkan/icd.d/MoltenVK_icd.json`.
Renderer and scene regressions passed with Khronos and synchronization validation;
native window tests verified cylinder/cone presentation and world-point picking.
The complete Tutorial 302 ran for three frames with four samples and validation.
New checks cover side/cap/tip depth, reversed segments, axis-parallel rays,
independent double-precision perspective intersections, partial near/far clipping,
eye/far rejection, mesh occlusion in both draw orders, endpoint colors/textures,
lighting, multiple indexed segments, mixed point/line uniform state, collapsed
segments and invalid inputs. Four-sample tests retain identical center picking
IDs/depth and verify actual partial edge coverage. Both new shaders pass
`spirv-val --target-env vulkan1.2`. Validation error counts remain zero; the
previously documented MoltenVK integer-attachment warning persists.

## Plain wide lines and Tutorial 304

`LinesDrawable::PLAIN` now accepts positive finite widths, including fractional
widths. The native `DrawCommand::line_width` uses framebuffer pixels and
`model_view_projection`, independently of `model_view`/`projection` and the
view-space radius used by cylinders/cones. Width one retains the existing native
line pipeline. Other widths expand each original segment into a six-vertex
screen-space ribbon. Pixel width remains constant across perspective/orthographic
views, framebuffer aspect ratios, camera distance and endpoint depths.

The vertex shader clips the centerline against homogeneous near/far/positive-W
planes before perspective division, preserving original endpoint interpolation
parameters. It offsets the clipped endpoints perpendicular to their projected
screen direction. Caps are flat at the endpoints (butt caps); segments are
independent line-list strokes with no joins or rounded caps. A segment collapsed
to one projected point draws nothing in the ribbon path. GPU viewport clipping
still permits a thick line whose centerline lies just outside the viewport.

A dedicated fragment shader uses perspective-correct endpoint colors/texture
coordinates and the rasterizer's depth, with no explicit `gl_FragDepth`, silhouette
discard or sample-rate shading. This preserves normal per-sample triangle depth
and coverage even on devices without `sampleRateShading`; query
`supported_sample_counts(LineStyle::plain)` for its available counts. The separate
center-sample picking pass uses the same ribbon geometry and the original segment
ID. Endpoint normal lighting follows the existing native one-pixel line convention.

Native-only window builds now also build `Tutorial_304_VectorField`, with its
original mesh, face-normal vector-field construction and three-pixel plain lines.
The sphere explicitly uses flat shading and uniform color to match its face-normal
vector field, regardless of texture coordinates stored in the example OBJ file.
The Vulkan scene adapter honors `TrianglesDrawable::smooth_shading()`: flat
snapshots assign geometric triangle normals to all three corners, preserving
vertex colors, primitive order and picking. Switching shading invalidates the
cached snapshot without requiring a model update.
It shares bounded execution/validation options with Tutorial 302 through
`tutorials/native_viewer_options.h`; their OpenGL scene paths are retained.

```sh
./cmake-build-debug/bin/Tutorial_304_VectorField --frames 3 --samples 4 --validation
```

Without `--frames` it runs interactively. Desktop CTest `vulkan.tutorial304` runs
three frames and honors `Easy3D_VULKAN_TEST_VALIDATION`, alongside Tutorial 302.
CI also validates both new wide-line shaders.

On 2026-10-27, Apple M4 Pro/MoltenVK 1.4.2 passed renderer and scene regressions
with Khronos and synchronization validation. Tests check horizontal/vertical,
fractional and oversized pixel widths, offscreen centerlines, a diagonal screen-space coverage/depth reference, reversed endpoints,
mesh occlusion, near/far/eye clipping, perspective color/UV interpolation,
lighting, projected degeneracy, independent per-draw widths, resize and four-sample
coverage with unchanged center picking/depth. Reference-depth comparisons allow
normal rasterizer subpixel roundoff; MSAA picking comparisons remain strict.
Native-window tests exercise wide-line presentation and depth picking. Tutorials
302 and 304 both run with four samples and validation. Both new shaders pass
SPIR-V validation, and the wide-line fragment SPIR-V has no depth-replacement or
sample-rate-shading capability. Validation error counts remain zero; the earlier
MoltenVK integer-attachment warning persists.

## Texture/scalar state and Tutorials 303/305

The scene adapter now supports `State::TEXTURED` and `State::SCALAR_FIELD` using
CPU image snapshots and the existing texture/scalar property extraction. Model
renderers select texture-coordinate/scalar properties by the same priority as
the OpenGL renderer. Vertex and halfedge UVs, texture repeat plus fractional
repeat, uniform/property switching, face/vertex scalar fields and scalar palette
replacement work with flat or smooth shading. As before, call
`model->renderer()->update()` after changing model properties or requesting a new
property extraction on an already uploaded model.

`Texture` retains an immutable `PixelData` snapshot of its input. In Vulkan-only
builds the existing `Texture::create()` and `TextureManager::request()` APIs
create CPU textures. In dual builds, use `Texture::create_cpu()` or
`TextureManager::request_cpu()` when there is no OpenGL context. These explicit
factories never call GL, and CPU drawables use the CPU-only cache for implicit
checkerboards/colormaps. The legacy factories still upload GL textures in OpenGL
builds, while retaining pixels usable by the Vulkan adapter. CPU-only textures
have ID zero; GL binding of such a texture reports an explicit error. Texture
manager ownership/release conventions remain in effect; drawables keep borrowed
`Texture` pointers and require the texture to remain alive while used.

The shared texture factories/manager and file loading are compiled in both
configurations. GL upload, binding and destruction live separately in
`renderer/backends/opengl/texture.cpp`, excluded from Vulkan-only builds. No
OpenGL library/context is needed by native texture loading, scalar normalization
or rendering. Texture cache entries distinguish CPU/legacy creation and sampler
choices; releasing a cached texture removes its actual cache entry. Cache lookup
and mutation are serialized for concurrent CPU requests. Callers still control
borrowed texture lifetimes and must synchronize release/termination with users.

`Texture` preserves the existing bottom-first input/file-loading convention.
The Vulkan adapter converts pixels to top-first RGBA8 and transforms texture
coordinates explicitly, including repeat factors, keeping images upright without
changing native `graphics::Image` conventions. Color and depth readback remain
top-first. Each immutable `graphics::Image` has a `Sampler` describing clamp or
repeat addressing and nearest or linear filtering. Its defaults remain
nearest/clamp for existing native clients. RGBA8 linear-filter support is checked
before realization. No mipmaps or anisotropic filtering are introduced.

Texture/palette changes do not alter geometry order or picking IDs. GPU image
snapshots are shared while bound by live drawables; the image-conversion cache
holds weak references, so replacing a binding retires unused descriptors even
when CPU texture caches retain the old pixels. A stress regression replaces more
than 1,024 live CPU snapshots on one drawable without exhausting the descriptor
pool. Scene texture alpha is sampled through the existing opaque/depth-writing pass;
this does not implement general 3D transparency. The overlay layer below supplies
explicit straight-alpha composition for screen-space content.

Constant non-boolean scalar fields map to the palette midpoint rather than
producing non-finite texture coordinates. Shared scalar extraction checks finite
values and valid clamp fractions before sorting/indexing, and computes ranges in
double precision to avoid float-range overflow. Boolean fields keep their 0/1
mapping. CPU polygon index uploads also work through the nested-index overload,
which no longer requires an OpenGL VAO for CPU-backed drawables.

Native-only window builds now include `Tutorial_303_ScalarField` and
`Tutorial_305_Texture`, using their existing scalar/texture scene code and shared
native execution options:

```sh
./cmake-build-debug/bin/Tutorial_303_ScalarField --frames 3 --samples 4 --validation
./cmake-build-debug/bin/Tutorial_305_Texture --frames 3 --samples 4 --validation
```

Desktop CTests `vulkan.tutorial303` and `vulkan.tutorial305` honor
`Easy3D_VULKAN_TEST_VALIDATION`. New headless test `vulkan.texture` covers pixel
ownership, raw/file orientation, sampler cache variants, repeat and fractional
repeat, linear filtering, vertex/halfedge UV seams, face/vertex/constant scalar
fields, invalid inputs, descriptor retirement and unchanged MSAA picking/depth.
The renderer test independently checks native sampler behavior. Dual builds also
provide desktop `opengl.texture`, exercising GL upload/readback, sampler state,
retained CPU data and cache release with a real GL context.

Local verification on 2026-10-27 used Apple M4 Pro/MoltenVK 1.4.2, selected with
`VK_DRIVER_FILES=/usr/local/share/vulkan/icd.d/MoltenVK_icd.json`. Native and dual
configurations compile. MoltenVK renderer/texture tests and the dual build's
headless suite pass with Khronos/synchronization validation. Tutorials 303/305
run with four samples and validation. The real OpenGL texture smoke test also
passes. Vulkan-only renderer linkage still contains no GL dependency. The earlier
MoltenVK integer-attachment warning persists; validation error counts stay zero.

## Screen-space overlays and Tutorial 306

`graphics::DrawCommand::overlay` selects an unlit, straight-alpha triangle draw
composited after all opaque scene commands. Ordering is preserved within each
overlay class: depth-tested annotations first, then HUD overlays. It
uses the normal texture/sampler and per-vertex color/tint state. Depth writes are
always disabled; default HUD overlays also disable depth testing. The color-only overlay subpass does not bind the
integer ID attachment, so blending requires neither `independentBlend` nor
integer-format blend support. Scene depth and IDs are preserved for picking.
RGBA8 attachment blending support is checked when selecting the physical device.

The overlay subpass writes the same color attachment/sample count as the scene.
For MSAA, final color resolve follows overlay composition, preserving geometric
coverage and the scene's per-sample colors. Picking still uses the existing
center-sample ID/depth pass and ignores overlays. A covered image is therefore
not a world-space picking hit, while overlays over a model leave that model's
world-point picking intact. This does not introduce general 3D transparency,
widget clipping, text shaping or legacy GL draw-callback emulation.

`VulkanOffScreen::set_overlays()` retains an ordered copy of immutable draw
commands and marks them as overlays. Use an identity MVP for clip-space geometry,
or `graphics::screen_space_projection(width,height)` for top-left framebuffer
pixel coordinates. Rebuild the projection/geometry when its intended viewport
placement changes; a scaled capture preserves its normalized placement. Calling
`set_overlays({})` removes them, and `clear_scene()` clears overlays too. Overlay
bounds do not affect the 3D camera's fit operation. Native `graphics::Image` data
and UVs retain their top-first convention. Transparent PNG export uses the
existing conversion from coverage-premultiplied readback to straight alpha.

Native-only window builds now also provide `Tutorial_306_ImageViewer`, using the
overlay API without an OpenGL context. Its native host handles image fitting,
wheel zoom, recentering on resize, **F** to fit, **Ctrl+O** to open, and image-file
drop/reload. Positions and dimensions use framebuffer pixels, including the
host's HiDPI conversion. The existing OpenGL tutorial host remains available in
OpenGL builds.

```sh
./cmake-build-debug/bin/Tutorial_306_ImageViewer --frames 3 --samples 4 --validation
```

The desktop smoke test `vulkan.tutorial306` honors
`Easy3D_VULKAN_TEST_VALIDATION`. A separate desktop `vulkan.imageviewer` test
checks orientation, aspect-ratio fitting, registered zoom/fit/resize/drop
callbacks, sample-count changes and absence of scene picking hits. Native-window
regressions check that overlays preserve all model depth/IDs and world-point
picking, and that clearing overlays restores scene color. The renderer test
checks ordered alpha blending, transparent backgrounds, top-first image UVs,
actual four-sample overlay edge coverage, unchanged scene picking, resize and
invalid input. Both overlay shaders are embedded and validated in CI.

The image viewer exposed a shared `ImageIO::load()` bug: forced channel conversion
copied the source channel count rather than the requested output count. RGB-to-
RGBA loads were truncated, while RGBA-to-RGB loads could overread decoded storage.
The loader now copies the correct checked byte count, preserves source-channel
metadata, frees decoded storage through RAII and updates output dimensions only
after success. Per-load row orientation uses stb's thread-local setting rather
than its shared global setting. Texture regressions cover both conversion
directions; existing texture/scalar and OpenGL texture paths remain covered.

On 2026-10-27, Apple M4 Pro/MoltenVK 1.4.2 passed renderer, texture, native-window,
image-viewer and bounded Tutorial 306 checks with Khronos/synchronization
validation, including four samples. Overlay shader SPIR-V validation passed. The
dual Vulkan/OpenGL regression suite also passed, including the retained OpenGL
texture smoke test. The previously documented MoltenVK integer-attachment warning
persists; Vulkan validation error counts remain zero. The next section adds basic
native text; native Qt/wx host integration remains a migration gate.


## Native text and Tutorial 309

`graphics::TextRenderer` rasterizes trusted TrueType/OpenType font files on the
CPU using the bundled stb_truetype, without a graphics context. It returns a
`graphics::TextLayout` containing ordinary overlay commands, ink bounds, the last
line's advance and line height. It is separate from the retained immediate
OpenGL `easy3d::TextRenderer`; legacy GL callbacks are not emulated.

```cpp
#include <easy3d/graphics/text_renderer.h>
easy3d::graphics::TextRenderer text;
text.add_font(font_file);
easy3d::graphics::TextOptions options;
options.x = 20; options.y = 20; options.size = 28;
auto layout = text.layout(u8"Easy3D — Vulkan", viewer.width(), viewer.height(), options);
viewer.set_overlays(layout.commands);
```

All coordinates, size and character spacing use framebuffer pixels. Hosts scale
logical UI coordinates by their framebuffer/window ratio exactly once. Layout
supports UTF-8 Unicode scalar values (including non-BMP characters), explicit
font selection, horizontal kerning, character spacing, multiline spacing and
left/center/right alignment within the longest line. CRLF is one newline; tabs
expand to four spaces. With the top-left origin, `y` is the top of the first font
box and lines advance downward. With the bottom-left origin, `y` measures the
bottom of that box from the framebuffer bottom and lines advance upward. Ink
bounds always use top-left framebuffer coordinates. Malformed UTF-8, missing font
indices, non-finite options and oversized glyphs fail explicitly. Missing
characters use the selected font's glyph zero. Complex-script shaping, bidi,
automatic font fallback, wrapping, vertical text and distance-field text are
not implemented.

Glyphs are cached by font, pixel size and glyph index in paged RGBA8 atlases with
transparent gutters and linear filtering. Consecutive glyphs on the same page
batch into one draw; page transitions preserve string order. New glyphs publish
new immutable atlas snapshots, preserving previously returned layouts, including
on other devices. `clear_cache()` releases cached sizes/pages without invalidating
retained commands or loaded fonts. Rebuild layouts when viewport placement or
DPI changes; otherwise retain them. Cache use is externally synchronized, and
applications control its lifetime and size retention. The current device's
1,024-live-texture descriptor limit still applies to simultaneously retained
atlas versions. Fonts are runtime resources and should come from trusted sources;
stb_truetype is not a validator for untrusted font data.

Native-only window builds now include `Tutorial_309_TextRendering`. It loads the
resource fonts and retains the tutorial's size (`+`/`-`), character spacing
(`<`/`>`), line spacing (up/down), alignment (`L`/`C`/`R`), origin (`O`) and kerning
(space) controls. Resize/DPI changes rebuild text; unchanged frames reuse it.
Text shares the color-only overlay pass, retaining MSAA and leaving scene depth,
IDs and world-point picking untouched. The OpenGL tutorial host is retained.

```sh
./cmake-build-debug/bin/Tutorial_309_TextRendering --frames 3 --samples 4 --validation
./cmake-build-debug/bin/easy3d_vulkan_text_test --cpu-only
```

Headless CTest `vulkan.text` checks layout, Unicode decoding, font selection,
kerning/spacing/alignment/origins, immutable and multi-page atlases, and GPU glyph
coverage/alpha against an independent bilinear CPU reference. It also checks
one/four-sample scene depth and IDs, overlay-only no-hit readback and resize.
Desktop `vulkan.textviewer` exercises registered shortcut callbacks, resize,
sample changes, snapshots and sphere world-point picking; `vulkan.tutorial309`
is a bounded native tutorial smoke test. They honor
`Easy3D_VULKAN_TEST_VALIDATION`. Installed packages include the new text header;
the installed-consumer CI check loads an installed font and renders a text overlay.
Transparent PNG export now converts overlay alpha at both one and multiple
samples; previously the conversion only ran for multisampling, leaving dark
single-sample text/image edges in exported files.

On 2026-10-27, Apple M4 Pro/MoltenVK 1.4.2 passed the new text renderer and
native text interaction tests and Tutorial 309 with four samples, Khronos
validation and synchronization validation. The native suite passed all 13 tests;
the dual Vulkan/OpenGL suite passed all five, including the retained real OpenGL
texture check. Snapshot regressions also passed with validation after adding
one-sample straight-alpha conversion. The existing MoltenVK integer-attachment
warning persists; validation error counts remain zero.


## OBJ material import and Tutorials 301/308/310

Native-only window builds now include `Tutorial_308_TexturedMesh`, using the same
viewer source and OBJ import implementation as its OpenGL build. The import code
now lives in `easy3d/renderer/obj_materials.h`, where other applications can use it
without constructing a window or graphics context:

```cpp
#include <easy3d/renderer/obj_materials.h>
auto asset = easy3d::load_obj_materials(obj_file);
viewer.add_model(asset.mesh, false); // attach with no default drawables
asset.create_drawables();           // match the attached renderer's CPU/GL mode
viewer.fit_screen();
```

`load_obj_materials()` returns a CPU `ObjMaterialMesh` snapshot. It builds a
SurfaceMesh, retains halfedge UVs and face diffuse colors, and tessellates each
material group into a separate part. UVs participate in tessellator vertex
identity, preserving seams even when two corners share an OBJ position. Concave
polygons and relative/negative OBJ indices work. Faces with incomplete UVs form
untextured parts instead of sampling undefined coordinates. Valid `map_Kd`
textures take priority; the original tutorial's ambient/specular maps remain
color-map fallbacks when no diffuse map is available. Missing maps/UVs use the
material's `Kd` diffuse color, and absent materials default to white. Texture paths resolve against the OBJ
location (or use an explicit absolute map path), independently of the process's
working directory. This follows the bundled fast_obj parser's OBJ/MTL conventions;
it does not add support for map options or texture filenames containing spaces.

`create_drawables()` creates flat-shaded triangle parts on the mesh's current
renderer. CPU renderers explicitly request CPU textures even in dual builds;
OpenGL renderers retain GL buffers/textures. Duplicate part names fail before
creating additional drawables. Each drawable owns an import snapshot in its
update callback, so a general `Renderer::update()` refreshes the material subsets
without replacing each one with the full model. The asset object can be released
after creation. Topology, UV or material edits require reimporting/rebuilding the
parts; these snapshots do not automatically follow mesh edits. Texture pointers
retain the existing borrowed TextureManager lifetime contract.

The parser checks face position/UV/material indices, finite positions/UVs/colors,
mesh-builder failures and empty tessellations before attachment. The fast_obj
allocation is released through RAII on success and failure, fixing the tutorial's
previous parsed-data leak. Tutorial 308 loads and realizes a replacement before
removing the previous models; OBJ read failures preserve the existing scene.
This is diffuse-material import, not full MTL rendering: normals are recomputed,
independent ambient/specular material lighting and MTL opacity are not applied,
and general 3D transparency,
PBR remains outstanding. The following picking stage adds original-face provenance.

`Tutorial_301_Drawables` now runs its shared standalone-drawable example on Vulkan,
including sphere points and a five-pixel wire bounding box. Its native path uses
CPU buffers and bounded execution options. `Tutorial_310_TextMesher` also uses the
shared meshing/scene code. It refreshes buffers and fits the view after appending
the second string, so both generated strings enter the initial scene bounds.
Its existing mesher can report degenerate-face diagnostics; this stage does not
change text triangulation algorithms. The retained OpenGL paths use the same
scene construction.

```sh
./cmake-build-debug/bin/Tutorial_301_Drawables --frames 3 --samples 4 --validation
./cmake-build-debug/bin/Tutorial_308_TexturedMesh --frames 3 --samples 4 --validation
./cmake-build-debug/bin/Tutorial_310_TextMesher --frames 3 --samples 4 --validation
```

Headless `vulkan.objmaterials` checks grouped rendering, shared-position UV seams,
diffuse texture orientation, missing texture/UV fallbacks, concave polygon
coverage, negative indices, CPU texture creation in dual builds, rejected input,
refresh/reload/resize and unchanged center-sample picking/depth with four samples.
Desktop `vulkan.objviewer` checks the native Tutorial 308 host's snapshots,
refresh, registered file-drop callback, resize and world-point picking. Desktop
`opengl.objmaterials` compiles the same tutorial host against OpenGL and checks
actual GL texture creation, rendered UV/colors, snapshots, refresh and reload.
The three bounded native tutorial CTests honor `Easy3D_VULKAN_TEST_VALIDATION`.

On 2026-10-27, Apple M4 Pro/MoltenVK 1.4.2 passed the material renderer and
native material-host regressions with Khronos/synchronization validation, plus
Tutorials 301, 308 and 310 with four samples and validation. The complete native
suite passed all 18 tests; the dual Vulkan/OpenGL suite passed all seven,
including the real OpenGL material-viewer and texture checks. Pixel probes and
snapshot inspection confirmed orientation, UV seams, concave coverage and color
fallbacks. The existing MoltenVK integer-attachment warning remains; validation
error counts stay zero. Native Qt/wx integration, remaining GUI picking/effects
and runtime backend selection/fallback remain migration gates.


## Source picking, selection and Tutorials 401/402

`VulkanOffScreen::pick(x,y)` synchronously resolves a visible framebuffer-pixel
hit to its source. `VulkanViewer` inherits this API; its input callbacks already
convert logical cursor positions to framebuffer pixels, including HiDPI.

```cpp
const auto hit = viewer.pick(x, y); // top-left framebuffer pixels
if (hit) {
    // hit.model is null for a standalone drawable; hit.drawable owns its source.
    const auto point = hit.position; // world-space surface position at pixel center
    if (hit.face_index >= 0) {
        auto face = easy3d::SurfaceMesh::Face(hit.face_index);
        // Use with the SurfaceMesh in hit.model.
    }
}
```

`PickResult` retains shared model/drawable ownership, generated drawable-local
`primitive`, `[0,1]` depth and world-space position. `face_index` is an original
SurfaceMesh face index when triangle provenance exists, otherwise -1. Background
and out-of-bounds coordinates return an empty result. The pick request creates
its own draw-source table alongside the submitted commands; removing/reordering
models, hidden drawables or several material drawables on one model cannot
reinterpret a previous table. Results survive source removal, but their face
indices describe the mesh at pick time, not future topology edits or garbage
collection. Externally synchronize scene edits and device access as before.

Picking uses the normal visible geometry, textures, impostor silhouettes and
analytic surface depth. It renders one sample at pixel centers, independently
of the window's MSAA count, preserving the existing center-sample selection
policy. Overlays do not block source picking. A visible point/line returns its
model/drawable and generated primitive, without a face mapping. This API does
not select a hidden face through a visible edge/point and does not implement
nearest vertex/edge picking or region/lasso selection. It currently performs a
full synchronous frame readback per request; it is not a hover-readback scheduler.
Raw `FrameImage` IDs retain their existing model/standalone order and generated
primitive semantics; only `pick()` uses the request-local drawable IDs.

CPU `Drawable::BufferData::source_faces` stores an optional original face index
per rendered triangle. Shared SurfaceMesh extraction records this mapping from
its generated triangle ranges for uniform/property/texture/scalar paths,
including polygon tessellation and deleted handle gaps. OBJ material parts keep
and re-upload their own source-face arrays. Manual position/index uploads clear
provenance. Custom CPU triangle drawables can call `set_source_faces()` after
their geometry uploads; the setter checks count and negative IDs (-1 is unknown).
Custom buffers without metadata still return their source and primitive, without
inventing a polygon handle. Call `Renderer::update()` after shared mesh edits;
OBJ import snapshots still require rebuilding after topology/attribute changes.

Renderer/drawable selection and `State::highlight_range()` are now supported by
the native scene adapter. It marks each primitive's corners with the shared
`setting::highlight_color` RGB and mixes after texture sampling and basic native
lighting. Selected models/drawables use strength 0.6; inclusive highlighted
primitive ranges use 0.8; overlapping selection/range yields 0.92. Invalid/empty
ranges select nothing. These are RGB tints: alpha, depth, silhouette coverage and
object/primitive IDs remain unchanged. Triangle, native/large point, sphere,
surfel, native/wide line, cylinder and cone paths all support them. Geometry
snapshots are rebuilt only when relevant geometry/coloring/highlight state
changes. Low-level clients can set `graphics::Vertex::highlight` consistently on
all corners of a primitive; its default strength is zero and overlays ignore it.
No new optional device feature or push-constant space is required.

Native window builds now include `Tutorial_401_ModelPicker` and
`Tutorial_402_FacePicker`, with bounded execution and validation options. Their
native paths use the source-aware API; the OpenGL hosts retain their GUI picker
implementations. A left click toggles model selection in 401. Tutorial 402
highlights every generated triangle belonging to the chosen polygon; a
background click clears the highlight. Native modified/right clicks retain the
base host's navigation behavior. Mesh face picking remains available after
resize and sample-count changes.

```sh
./cmake-build-debug/bin/Tutorial_401_ModelPicker --frames 3 --samples 4 --validation
./cmake-build-debug/bin/Tutorial_402_FacePicker --frames 3 --samples 4 --validation
```

Headless `vulkan.picking` checks RGB highlighting on all native styles with
textures and lighting, unchanged alpha/coverage/IDs/depth at one/four samples,
model/drawable visibility and source ownership, transformed world points,
standalone sphere hits, overlays, removal/reordering, resize, polygon/deleted
handle mappings and property/UV/scalar extraction. Material regressions check
original face IDs across separate OBJ parts. Desktop `vulkan.pickerviewer`
drives the registered click callbacks for model toggle and polygon selection,
HiDPI/resize, MSAA changes, background clear and snapshots. Bounded tutorial
CTest entries are `vulkan.tutorial401` and `vulkan.tutorial402`; validation-enabled
configurations require Khronos/synchronization validation. CI validates all
modified shader binaries through the existing SPIR-V checks.

On 2026-10-27, Apple M4 Pro/MoltenVK 1.4.2 passed the new picking and registered
picker-host interaction tests with Khronos/synchronization validation, plus
Tutorials 401/402 with four samples and validation. All eight modified shaders
passed SPIR-V validation. The complete native suite passed 22 tests; the dual
Vulkan/OpenGL suite passed eight, including the retained real GL material/texture
checks. Both retained OpenGL picker tutorial sources also compile. The existing
MoltenVK integer-attachment warning persists; validation error counts remain zero.

Full-suite verification also exposed an intermittent existing text-viewer
shortcut baseline mismatch; five bounded diagnostic repeats passed. Its harness
now drains initial window/DPI callbacks, uses a smaller initial window, checks
framebuffer dimensions before comparing pixels and saves expected/actual PNGs on
a mismatch. The final complete native run passed; the original mismatch was not
reproduced with diagnostics. Qt/wx/Python integration, original
general vertex/edge and mesh-region pickers, advanced effects and runtime backend selection/
automatic OpenGL fallback remain migration gates.


## Portable clipping and Tutorial 307

`VulkanOffScreen::set_clipping_planes()` configures up to two world-space plane
equations for one native viewer/offscreen instance. Each equation keeps
`dot(plane, vec4(world_position,1)) >= 0`; two opposing planes form a slab.
Empty input disables clipping. The setter validates and normalizes finite
coefficients with nonzero normals before changing state. Scene clearing retains
this configuration, and overlays remain unclipped and unpickable. Native state
is per viewer; the retained OpenGL `ClippingPlane` singleton is not used by the
Vulkan backend.

```cpp
viewer.set_clipping_planes({easy3d::vec4(1,0,0,0)}); // keep world x >= 0
viewer.set_clipping_planes({easy3d::vec4(1,0,0,0), easy3d::vec4(-1,0,0,0.2f)});
// now keep 0 <= world x <= 0.2
viewer.set_clipping_planes({});
```

Triangles are clipped geometrically in CPU-owned snapshots, preserving polygon
winding and interpolating positions, UVs and colors. Original vertex lighting is
baked before clipping, preserving its interpolation across new cut vertices.
Generated fan triangles retain the original triangle's primitive ID. Plain line
centerlines are clipped before native-line/ribbon rasterization, producing butt
caps perpendicular to the line, with interpolated endpoint colors/UVs. Plain
points retain center-based clipping, including large screen-space squares.
`State::plane_clip_discard_primitive()` additionally rejects a primitive when any
of its control vertices is outside; it applies before curved surface clipping.

Sphere, surfel, cylinder and cone impostors retain their proxy geometry and
clip their computed front-surface intersections in the fragment shader. A sphere
whose center lies outside can retain a visible partial surface. MSAA uses the
existing per-sample analytic paths; no new optional device feature is required.
Clipping does not synthesize caps or substitute a back intersection when the
front surface is removed. Closed cut surfaces/back-hit rendering remain future
work. Geometric cut boundaries use normal rasterizer MSAA, without requiring
`shaderClipDistance`, geometry shaders or sample-rate shading for plain geometry.

The device caches derived geometric snapshots by source and clipping/transform/
lighting policy. Prepared commands retain their effective geometry throughout
submission; independent draws sharing one source can use different cut volumes.
Expired clipping snapshots retire with the existing fenced resource caches.
Clipping rebuilds and uploads are synchronous; large meshes with moving planes
can incur CPU/upload costs. Stable settings reuse the cached cut geometry.
Low-level `graphics::DrawCommand::clipping_planes` are **view-space** equations;
when clipping is active, provide an affine `model_view` mapping the geometry into
that view space, consistently with its MVP. The scene adapter performs world-to-
view conversion and honors model manipulation. Original primitive IDs survive
clipping, so raw readback, range highlights and `pick()` source-face mappings
continue to identify the original primitives rather than generated cut pieces.

Native-only window builds now include `Tutorial_307_CrossSection`. Left/right
drag rotates/translates its plane; **Alt+C** toggles clipping, **Alt+V** toggles the
guide, and **Alt+X** toggles a slab ending at the plane. The native tutorial uses
a slab width of 1% of the scene radius. Its guide does not change scene bounds, depth or picking. The following
annotation stage adds depth-tested transparent fill to the initial wireframe
guide, using the shared clipping-plane color and opacity. The guide's MVP follows camera changes,
including resize; its geometry is rebuilt after plane motion. The original
OpenGL host remains available.

```sh
./cmake-build-debug/bin/Tutorial_307_CrossSection --frames 3 --samples 4 --validation
```

This stage also corrects native depth-to-world reconstruction. The legacy
`Camera::unprojectedCoordinatesOf()` uses a height-minus-one coordinate convention;
passing framebuffer pixel centers to it introduced a one-pixel Y offset.
`VulkanOffScreen::unproject_pixel()` instead reconstructs the exact top-left
framebuffer center from readback depth and the inverse MVP. `pick()` and
`VulkanViewer::point_under_pixel()` share this conversion. The common legacy
Camera/OpenGL helpers retain their existing convention. Oblique-plane tests now
verify world coordinates and clip-volume membership, rather than checking depth
alone.

Headless `vulkan.clipping` checks halfspaces/slabs, indexed source IDs,
interpolated vertex lighting/color/UVs, independent per-draw cuts, whole-primitive
rejection, point/line policy, actual sphere/surfel/cylinder/cone hits, new cut-edge
MSAA coverage, unchanged center picking/depth, world/model/camera transforms and
source-face mapping. Invalid planes/transforms fail explicitly. Desktop
`vulkan.crosssection` exercises the registered toggle/drag callbacks, guide-only
visibility, world-point reconstruction, slab membership, resize, MSAA changes and
snapshots. Bounded Tutorial 307 CTest honors `Easy3D_VULKAN_TEST_VALIDATION`, and
CI validates the modified impostor/ribbon shader binaries.

On 2026-10-27, Apple M4 Pro/MoltenVK 1.4.2 passed the new clipping and native
cross-section interaction tests with Khronos/synchronization validation,
including four samples. Tutorial 307 passed bounded four-sample execution with
validation, all five modified shader binaries passed SPIR-V validation, and the
retained OpenGL Tutorial 307 source compiles. The complete native suite passed
25 tests; the dual Vulkan/OpenGL suite passed nine, including real GL material
and texture checks. The previously documented MoltenVK integer-attachment
warning remains; validation error counts stay zero.

The recurring text-viewer shortcut harness now uses a hidden native window to
isolate synthetic key callbacks from desktop focus/input. It retains the actual
host, presentation, resize, color and depth checks; five bounded repeats and the
final full suite passed. Remaining gates include closed cut caps/back-hit clipping, Qt/wx/Python integration,
original general vertex/edge and mesh-region pickers, advanced effects and runtime backend
selection/automatic OpenGL fallback.


## Depth-tested transparent guides

Tutorial 307 now draws a filled translucent plane and an opaque outline, using
`setting::clipping_plane_color` (red with opacity 0.2 by default). The fill follows
the manipulated plane in both clipping and slab modes. It is visible through
the retained geometry from the cut side, and opaque surfaces nearer the camera
occlude it. Alt+V hides/shows both fill and outline; the guide remains exempt from
scene clipping, camera bounds and model picking. The secondary slab boundary is
outlined. This is a spatial guide, not generated mesh cut-cap geometry.

`graphics::DrawCommand::overlay_depth_test` enables this annotation path when
`overlay` is true. `VulkanOffScreen::set_overlays()` sets `overlay` automatically,
so callers can supply a triangle/image annotation and its world-to-clip MVP:

```cpp
easy3d::graphics::DrawCommand plane;
plane.geometry = plane_triangles;
plane.model_view_projection = viewer.camera()->modelViewProjectionMatrix();
plane.color = easy3d::vec4(1, 0, 0, 0.2f);
plane.overlay_depth_test = true;
viewer.set_overlays({plane});
```

The color render pass has three subpasses: opaque scene, depth-tested alpha
annotations, then screen/HUD overlays. The annotation subpass binds the scene's
matching sample-count depth attachment read-only and uses `LESS_OR_EQUAL`, with
depth writes disabled. It binds only RGBA color, leaving integer IDs preserved.
The final MSAA resolve occurs after both alpha passes, so opaque edge samples
occlude/blend the guide independently. A separate center-sample picking pass
continues to ignore all annotations. Depth attachment store operations and final
layout transitions have explicit synchronization dependencies, even with a
read-only fragment-test attachment. No `independentBlend`, depth-resolve feature
or new shader code is required.

Both modes use unlit straight-alpha color/texture composition and preserve scene
readback depth/IDs. HUD overlays follow the depth-tested annotations, regardless
of their interleaving in the input vector. Ordering within each mode follows
input order. Callers must sort overlapping translucent annotations back-to-front;
there is no automatic triangle sorting or order-independent transparency. General
transparent model materials and `TrianglesDrawable::opacity()` remain migration
gates; this path supplies explicit unlit annotations rather than a new material
pass. Transparent PNG exports retain straight-alpha conversion at one and
multiple samples.

Headless `vulkan.depthoverlay` checks front/behind opaque occlusion, coplanarity,
alpha readback, ordered overlapping layers, HUD precedence, resize, no-hit empty
scenes and unchanged depth/IDs. A per-sample oracle verifies the guide blends only
uncovered samples along an opaque MSAA edge before resolve. Native cross-section
checks verify filled interior color, perspective occlusion from the retained
side, guide-only visibility and unchanged picking. Scene snapshot regressions
exercise straight-alpha PNG export for both overlay modes at one/four samples.

Verified on this Mac with Apple M4 Pro / MoltenVK 1.4.2: all 26 native
Vulkan CTests and all 10 dual-backend CTests pass. The new depth-overlay and
native cross-section tests also pass with synchronization validation enabled,
as does Tutorial 307 at four samples. The previously documented MoltenVK
integer-attachment blending warning remains; validation error counts are zero.


## Native animation and Tutorial 311

Tutorial 311 now shares its animation code across Vulkan and OpenGL. A CPU-owned
copy of the embedded bunny vertices is stretched along Z and published through
`TrianglesDrawable::update_vertex_buffer(..., true)`. Each cycle recomputes from
the original positions, avoiding accumulated deformation. This replaces the
original OpenGL-only `VertexArrayObject::map_buffer()` example; native Vulkan
applications do not map an OpenGL buffer or require an OpenGL context.

`VulkanViewer` now provides `animation_func_`, `set_animation()` and
`is_animating()`, with F2 pause/resume. The callback runs on the UI thread before
`pre_draw()` and scene extraction for each renderable frame attempt. Capturing,
saving a snapshot or picking does not advance animation. Closed, iconified and
zero-sized windows skip updates. A callback can pause itself or request closure;
exceptions propagate from `draw_frame()` and are reported as failure by `run()`.
As in the legacy Viewer, the bool callback result is advisory; stop explicitly
with `set_animation(false)`. Replace or clear a callback between frames, rather
than from inside the callable itself.

These are frame-based callbacks, not a fixed-rate simulation clock. A real-time
simulation should compute its own elapsed time. Scene mutation, drawable updates
and viewer lifetime remain UI-thread operations. Tutorial 312, described below,
uses this callback for safe worker-to-UI handoff; scene models are not thread-safe.

```sh
./cmake-build-debug/bin/Tutorial_311_Animation --frames 50 --samples 4 --validation
```

The native `vulkan.animation` regression checks stateful callbacks, execution
before scene extraction, F2 pause/resume, self-pause, current animated surface
picking/depth, one/four samples, resize, the tutorial's stretch/reset cycle,
finite run budgets, callback failures and closure. Tutorial 311 also joins the
bounded native tutorial CTests. The shared tutorial source remains compatible
with the retained OpenGL Viewer.

Verified on Apple M4 Pro / MoltenVK 1.4.2: all 28 native CTests and all
10 dual-backend CTests pass. The animation regression and Tutorial 311
(`--frames 50 --samples 4`) pass with Khronos/synchronization validation enabled
and zero validation errors. The retained OpenGL tutorial source also passes a
C++11 compilation check. The previously documented MoltenVK integer-attachment
blending warning remains.


## Worker-thread streaming and Tutorial 312

Tutorial 312 now shares its point-generation and scene-update code across
Vulkan and OpenGL. Its worker generates batches of 100 positions/colors using
its own random generator, then publishes an immutable CPU batch through a
mutex-protected, one-slot mailbox. Only the viewer thread appends those points
to the model and calls `Renderer::update()`, through the animation callback.
The worker never accesses a model, renderer, viewer, GLFW or a graphics API.
This replaces the former detached timer threads editing the displayed cloud.

One pending batch bounds producer memory while F2 pauses consumption or the
window is minimized. The worker waits when that slot is occupied; it does not
overwrite/drop earlier batches. Consumption during normal frames is nonblocking.
A completed-and-empty check holds the mailbox lock, so a final publication
cannot race with automatic pausing. Worker exceptions are delivered to the
consumer. A startup wait ensures even a short bounded native run consumes real
worker output rather than showing only the initial static cloud.

The default produces 66 batches at 300 ms intervals, starting immediately, on top
of the original 100 red points. Backpressure can extend this duration. The owner
interrupts pacing/backpressure waits and joins the worker before viewer teardown,
including early window closure, finite frame budgets and exception unwinding.
No detached callbacks retain pointers to destroyed scene/viewer objects.

```sh
./cmake-build-debug/bin/Tutorial_312_MultiThread --frames 100 --samples 4 --validation
```

`vulkan.multithread` checks finite ordered delivery, immutable retained batches,
backpressure, cancellation with pending work, destruction during a long pacing
wait, repeated stop, empty streams and invalid intervals. Its native viewer
checks that paused workers cannot modify the displayed model, that callbacks
apply updates on the UI thread, and that streamed point colors, depth and source
IDs remain correct across one/four samples and resize. It also checks final
completion and closing with pending work. Tutorial 312 joins the bounded native
tutorial CTests, completing native build coverage of Tutorials 301–312.

The existing animation regression now follows the tutorial's actual stretch
cycle instead of assuming the old 1.01 step size; the current 1.002 playback
rate is preserved. Presentation budgets are counted independently of animation
steps so swapchain retries do not change exact cycle probes.

Verified on Apple M4 Pro / MoltenVK 1.4.2: all 30 native CTests and all
10 dual-backend CTests pass. Streaming and animation regressions pass with
Khronos/synchronization validation enabled, as does Tutorial 312 for 100 frames
at four samples; validation error counts remain zero. The shared OpenGL
tutorial source passes a C++11 compilation check. The previously documented
MoltenVK integer-attachment blending warning remains.


## GPU lasso selection and Tutorial 403

Tutorial 403 now has a native Vulkan host. Shift-left drag selects and Shift-right
drag deselects point centers; releasing Shift during a drag still completes the
gesture. L chooses lasso mode (default), R chooses rectangle mode. Escape cancels
an active gesture; focus loss or framebuffer resize also cancels it. Outside a
selection gesture the normal camera controls remain available. A red transparent
fill and opaque boundary use the existing HUD path and never modify scene depth
or picking IDs, including under MSAA.

`graphics::RenderDevice::select_points(PointSelectionRequest)` binds the shared
point-position/source-ID buffer, and uploads the lasso polygon and packed
selection words. These form three shader storage buffers. An embedded GLSL450 compute shader projects each point and performs
the even-odd in-polygon test **on the GPU**. It updates one bit per source point
with `atomicOr` / `atomicAnd`, following the OpenGL SSBO lasso implementation.
Using compute avoids the optional vertex-stage storage-write/atomic feature and
requires no fragment shader, render pass, depth target or geometry shader. A
128-byte push block carries the camera/model MVP, viewport, mode and up to two
local clipping planes. Larger dispatches use a two-dimensional workgroup grid.

This is through-selection of centers, including occluded points, rather than a
visible-fragment ID query. The shader uses continuous top-left framebuffer pixel
coordinates, includes polygon boundaries, and rejects centers behind the eye,
outside the camera frustum or outside the clipping halfspaces. The native host
transforms its world-space clipping planes into each drawable's local space,
and uses the same manipulated-model MVP as rendering. Deleted vertex slots are
skipped while live source vertex indices continue to address their original
selection bits. This does not add general mesh vertex/edge hit provenance or
mesh region picking.

The operation is synchronous: per-operation polygon/flag SSBO inputs are flushed if needed,
then explicit host/compute barriers, a submission fence and noncoherent readback
invalidation make updated words available to the caller. Tutorial 403 copies the
result into `v:select` and updates per-point display colors on the UI thread.
Unmatched flags and unused tail bits are preserved. Empty/degenerate lassos do
nothing; malformed inputs and storage-buffer/dispatch limit violations fail
explicitly. The compute pipeline/descriptors are private to each device and are
created transactionally on first use. A graphics queue without compute support
reports selection as unavailable; ordinary rendering remains usable. The path
uploads the polygon/current flags and reads back flags per operation, without
asynchronous selection or persistent GPU-only selection flags. Position resources
are retained and shared with rendering, as described below.

```sh
./cmake-build-debug/bin/Tutorial_403_PointSelection --frames 3 --samples 4 --validation
```

The headless `vulkan.pointselection` test covers 100,003 input points, concurrent
bit-word updates, concave/even-odd polygons, boundary inclusion, orientation,
deselection, padding, nonfinite/deleted slots, clipping, transforms, input
validation, render-state preservation and independent devices. The desktop
`vulkan.pointselectionviewer` test drives native gesture handlers and registered keyboard callbacks through
lasso/rectangle selection, cancellation, modifier release, deleted/occluded
points, selection colors, unchanged depth/IDs, MSAA, model transforms and
HiDPI/resize. CI validates the new compute SPIR-V. The retained OpenGL tutorial
continues to use its existing picker and drawing implementation.

Verified on 2026-10-28 with Apple M4 Pro / MoltenVK 1.4.2: all 33 native
CTests and all 11 dual-backend CTests pass. The GPU selection and native gesture
regressions, including an explicit translucent interior-pixel check, pass with
Khronos/synchronization validation enabled and zero validation errors. Tutorial
403 passes bounded four-sample execution with validation. The compute SPIR-V
passes `spirv-val --target-env vulkan1.2`, and the retained OpenGL tutorial
sources pass a C++11 compilation check. The existing MoltenVK integer-attachment
blending warning remains. Synthetic gesture tests use explicit framebuffer
coordinates in a hidden window to avoid macOS cursor-warp/focus timing; native
event routing remains covered by the existing window tests.


## Shared point positions for rendering and selection

`graphics::PointData` is an immutable snapshot of point positions and original
source slots in point draw/instance order. `Geometry::point_data()` and
`PointSelectionRequest::points` retain the same snapshot. Each device realizes it
once as a device-local buffer with both `VERTEX_BUFFER` and `STORAGE_BUFFER`
usage. The 16-byte record contains three position floats and a source-slot ID.
Rendering binds that exact allocation as its position vertex stream; lasso
compute binds it as its read-only position/source-ID SSBO. No separate selection
position buffer or per-gesture position upload remains.

Colors, normals, UVs, generated picking IDs and highlight tints use a separate
attribute vertex buffer. A color/selection/highlight change can replace those
attributes while retaining the point-position allocation. Point-cloud extraction
keeps live source vertex indices and skips deleted slots; indexed/repeated point
instances retain their source mapping, so atomic selection bits address original
slots rather than draw order. Source IDs are distinct from generated picking
primitive IDs.

`VulkanOffScreen::point_resource(drawable)` returns the drawable's current,
unclipped snapshot. The adapter compares refreshed positions/order/source slots
and retains it across color-only rebuilds, transforms, style changes, resize and
MSAA changes. Actual position/topology changes create a new immutable resource,
uploaded once on first use. Retained older snapshots stay independently usable.
The adapter can still extract/compare CPU drawable snapshots on state updates;
this optimization removes redundant GPU position transfers rather than making
all CPU scene extraction incremental.

Plain-point center clipping and whole-impostor control rejection now run in the
point vertex shader, while sphere/surfel surface clipping remains in the fragment
shader. Moving clipping planes therefore does not rebuild/upload a filtered point
position buffer. Native one-pixel point rasterization is retained using a vertex
shader specialization; larger points and analytic impostors retain instanced
quads. Depth, MSAA and picking policies remain unchanged.

A staging upload makes the shared positions visible to vertex input and compute
reads. Per-operation polygon/selection-word transfers retain host/compute barriers,
submission fences and noncoherent flush/invalidate handling. Realizations are
private to each device and expired snapshots retire after completed submissions,
including when a scene stops drawing points.

For low-level callers, retain a `PointData` snapshot and pass it to successive
point `Geometry` objects when changing attributes, then also use it in selection:

```cpp
auto points = geometry->point_data();
easy3d::graphics::PointSelectionRequest request;
request.points = points;
request.width = width;
request.height = height;
request.polygon = lasso;
request.words = selection_words;
auto selected = device->select_points(request);
```

`RenderDevice::point_upload_count()` reports cumulative position-resource uploads
for transfer diagnostics. Regressions assert one realization for render-first and
selection-first use, no new position uploads for repeated selection, color edits,
clipping, transforms, MSAA or resize, and exactly one replacement for a position
edit or deletion. They also cover indexed/duplicate points, sparse source IDs
across bit words, retained old snapshots and independent devices.

Verified on Apple M4 Pro / MoltenVK 1.4.2: all 33 native CTests and all
11 dual-backend CTests pass. Shared-resource headless/viewer regressions and
clipping checks pass with Khronos/synchronization validation enabled and zero
validation errors. Tutorial 403 also passes bounded four-sample validation.
The modified point vertex and selection compute shaders pass Vulkan 1.2
SPIR-V validation. The previously documented MoltenVK integer-attachment
blending warning remains.


## Visible-surface scanning and Tutorial 404

`VulkanOffScreen::scan_visible_points(samples)` returns a position-only virtual
scan of the currently visible scene in world coordinates. Each non-background
framebuffer pixel contributes one point at its exact pixel center, in top-to-bottom
row order. This samples rendered surfaces rather than returning the original
mesh vertices or point-cloud centers. Hidden/occluded geometry and transparent
HUD/plane annotations do not produce points. Clipping and manipulated-model
transforms follow the rendering path, including actual sphere/surfel/cylinder/cone
surface depth rather than proxy geometry.

One effective camera snapshot drives both capture and reconstruction. Its inverse
MVP is computed once for the whole scan, and custom/calibrated projection and view
matrices and the interaction pivot (used for impostor sizing) are preserved
explicitly. The pixel-center reconstruction helper is
shared with `unproject_pixel()` and picking. Samples come from the existing
center-sample depth/ID path, so changing color MSAA does not change scan positions;
an MSAA edge pixel without a center hit contributes no point. Scans use physical
framebuffer dimensions on HiDPI displays, with no legacy logical-pixel stride or
height-minus-one conversion. The API does not change the camera/scene or advance
animation, and an empty scene returns an empty vector.

Tutorial 404 now builds with the native Vulkan host. **Space** acquires the visible
scene and adds a new PointCloud; **N** toggles the shared Gaussian-noise algorithm.
Native noise sigma is `0.0001 * camera.sceneRadius()` in world units. Scanned clouds
remain visible and participate in later scans, as other visible scene models do.
Acquisition uses the additive `add_model(cloud, true, false)` overload to attach
results without refitting, keeping the complete camera state and custom matrices.
It synchronizes framebuffer dimensions if a resize
arrived before the next draw. Empty acquisitions add no model. The retained OpenGL
tutorial keeps its existing FBO implementation.

```cpp
auto world_points = viewer.scan_visible_points(); // synchronous, one center hit per pixel
```

```sh
./cmake-build-debug/bin/Tutorial_404_VirtualScanner --frames 3 --samples 4 --validation
```

`vulkan.scan` tests pixel-center reprojection, perspective/orthographic/custom
camera matrices, tilted transformed surfaces, occlusion/background rejection,
transparent overlays, clipping, resize, MSAA invariance, analytic point/line
impostor surfaces and invalid sample counts. `vulkan.scanviewer` drives the
registered scanner shortcuts, checking exact acquired clouds, noise toggling,
camera preservation, resize/HiDPI, MSAA and empty/cleared scenes. Tutorial 404 joins
the bounded native tutorial tests.

Verified on Apple M4 Pro / MoltenVK 1.4.2: all 36 native CTests and all
12 dual-backend CTests pass. The headless scan and native scanner regressions
pass with Khronos/synchronization validation enabled and zero validation errors,
including custom matrices and non-default pivots. Tutorial 404 passes bounded
four-sample execution with validation. Both retained OpenGL tutorial sources
pass a C++11 compilation check. The existing MoltenVK integer-attachment
blending warning remains.


## Tutorial source-layout alignment

Tutorials 306 (image viewer), 307 (cross-section), 309 (text), 403 (point selection)
and 404 (virtual scanner) now use their original `viewer.h` / `viewer.cpp` pairs
for both backends. Each header contains one tutorial class declaration and its
state. All method definitions live out of line in `viewer.cpp`; backend-specific
includes and implementations stay in that same source file. The five parallel,
header-only `viewer_vulkan.h` files have been removed. Clients and tests include
`viewer.h` for either backend.

The shared `tutorials/viewer_backend.h` selects Viewer or VulkanViewer once
and exposes the `easy3d::TutorialViewer` alias. Tutorial headers use this common
base without repeating backend includes or per-class aliases. Remaining
constructor, hook and state differences use `EASY3D_TUTORIAL_USES_VULKAN`. Native-only builds select Vulkan as before. A private
`EASY3D_TUTORIAL_VULKAN` target definition also selects the Vulkan variant for
native tutorial tests in combined OpenGL/Vulkan-window builds. This does not
change the framework's current backend-selection policy or supply runtime
fallback. OpenGL tutorial code remains in the same source files and keeps its
existing behavior.

The native tutorial build now compiles each existing `viewer.cpp` when present,
matching the original tutorial source lists. Interaction tests link those same
implementation files instead of obtaining implementations through headers, and
include their GLFW, image I/O and model dependencies explicitly. The moved native
method bodies and retained OpenGL implementation bodies are unchanged.

This stage aligns the five existing tutorial viewer variants. Core backend
interfaces, application hosts and further tutorial ports still follow the
migration gates above; renderer-wide layout consolidation must preserve native
headless operation and supported OpenGL builds.

Combined-host regression coverage also corrected the animation test setup: it
now attaches its drawable to the native viewer before uploading data, so CPU
buffers are selected in dual-backend builds as well as Vulkan-only builds.
Production animation behavior is unchanged.

Verified on Apple M4 Pro / MoltenVK 1.4.2: all 36 native CTests and all
17 combined OpenGL/Vulkan-window CTests pass. All five refactored viewer paths
pass with Khronos/synchronization validation: image/text in the combined build,
and clipping/selection/scanning in explicit native runs. The five OpenGL
main/viewer source pairs also pass C++11 compilation checks, as do their native
viewer implementations in a dual-backend compilation context. No validation
errors were reported; the existing MoltenVK integer-attachment warning remains.
