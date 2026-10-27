# Native Vulkan migration

The `Easy3D-Vulkan` branch starts an incremental migration. It is **not yet a
replacement for the full OpenGL renderer**. A native interactive Vulkan viewer is
now enabled by default on this development branch. Mapple, FigureMaker, GUI picking/effects, Python
rendering and most existing interactive tutorials still use OpenGL. They are retained until their Vulkan replacements are ready.

## Implemented

- Backend-neutral, immutable geometry and image descriptions, draw commands and
  typed color/depth/object/primitive readback in `easy3d/graphics/render_device.h`.
- A native Vulkan 1.2 offscreen device, with no OpenGL context, window, surface,
  swapchain, Qt, GLFW or GLEW dependency in the `easy3d::vulkan` target.
- Indexed/nonindexed triangle lists, plain one-pixel point/line lists, uniform and
  vertex colors, RGBA8 textures, basic directional lighting and depth testing.
- Explicit sampler descriptors, including a valid white fallback texture.
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
- The existing Tutorial 201 compiles unchanged using the `Viewer` alias in a
  Vulkan-only window build. Legacy custom OpenGL draw callbacks are not ported.
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
1.2 and checks target-format support. macOS and Windows hardware execution are
still required before making a cross-platform support claim.

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
unchanged in that configuration. Other tutorials are still gated until their
features and callbacks are ported; the global legacy tutorials option stays OFF.
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
| `VulkanOffScreen` defaults | Opaque mesh faces; uniform or vertex/face property color; plain one-pixel points/lines. Scalar/texture properties are not automatically chosen as defaults yet. |
| Material/lighting | Basic fixed world-space diffuse term; no full material or back-face-color parity yet. Normal transforms use the inverse transpose. Disable lighting for exact unlit colors. |
| `TrianglesDrawable::opacity()` | As in the old opaque pass, effective only in a separate transparency pass; such a pass is not implemented here. |
| CPU drawable textures/scalars | Legacy `Texture` coloring is rejected. Native `graphics::Image` texturing is implemented. CPU-only implicit legacy texture creation throws before making GL calls. |
| Transparency, SSAO, shadows, EDL, text, clipping | Not ported. Do not infer support from the low-level draw-command API. The scene adapter rejects SSAO/highlighting/selection; no effect-pass interface is exposed yet. |
| Large points, spheres/surfels, cylinders/cones, wide lines | Not ported; explicit unsupported-style error from the scene adapter. |
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
for window/input management. Both retain shared CPU scene APIs and the new offscreen adapter. Delete old
implementations only after each replacement passes its application/feature tests.

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
macOS/MoltenVK and Windows validation of this new path remain outstanding.

Next implementation gates are native Qt/wx surface/frame adapters, portable
impostors, text and texture/scalar state, ID mapping/pickers, oversized capture,
effects/transparency, all applications/tutorials and Python integration, then
hardware validation and packaging on macOS/Windows/Linux. The defaults now select
Vulkan for development; removing the remaining OpenGL code depends on these gates.
