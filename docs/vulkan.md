# Native Vulkan migration

The `Easy3D-Vulkan` branch starts an incremental migration. It is **not yet a
replacement for the full OpenGL renderer**. A native interactive Vulkan viewer is
now enabled by default on this development branch. Mapple, FigureMaker, GUI picking/effects, Python
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
- Screen-space triangle/image overlays with straight-alpha composition in a
  color-only subpass before the final MSAA resolve. Overlays do not alter scene
  depth or picking IDs and require no independent-blend feature. Tutorial 306
  runs as a native image viewer with fit, zoom, resize and image reload.
- CPU TrueType/UTF-8 text layout and immutable glyph atlases, using the native
  overlay path. Tutorial 309 provides font, kerning, spacing, alignment and origin controls.
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
- Existing Tutorials 201, 302–306 and 309 build with the native Vulkan host in
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
all models, **P** toggles perspective/orthographic, **Escape** closes. Shift-left
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
unchanged in that configuration. Tutorials 302–306 and 309 are also built in this configuration. Other tutorials remain gated
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
| CPU drawable textures/scalars | `Texture` pixel snapshots, vertex/halfedge UVs and scalar fields are supported without GL calls. Clamp/repeat and nearest/linear filtering work; no mipmap, anisotropic or multi-material texture pipeline yet. |
| Transparency, SSAO, shadows, EDL, text, clipping | General 3D transparency/effects remain pending. Straight-alpha 2D overlays and basic horizontal TrueType/UTF-8 text are supported; shaping, bidi and automatic font fallback remain pending. The scene adapter still rejects SSAO/highlighting/selection; overlays are not an effect-pass API. |
| Point and line styles | Plain points of any positive pixel size, spheres and oriented surfels are implemented. Capped cylinders/cones and butt-capped plain wide lines are implemented; joined strokes and rounded plain-line caps remain pending. |
| Snapshot options | Device-supported MSAA (default 1); `expand=false` remains unsupported. Scaling, white/current/transparent background and image saving work. No tiled oversized captures. |
| Readback | RGBA8, D32 and RG32UI; top row first; depth [0,1]; no-hit object/primitive IDs are `UINT32_MAX`. |
| Picking IDs | Draw-command object ID and draw-primitive index (or explicit native primitive IDs). The adapter uses model/drawable order and generated primitive indices, not original polygon/vertex handles. Existing GUI pickers are not ported. |
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
supported. Legacy material/back-face-color parity and scene clipping planes
remain pending.

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
rounded caps, scene clipping planes and arbitrary profile/frustum styles are not
provided by this stage.

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
composited after all scene commands. Ordering is preserved among overlays. It
uses the normal texture/sampler and per-vertex color/tint state, with depth testing
and depth writes disabled. The color-only overlay subpass does not bind the
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
