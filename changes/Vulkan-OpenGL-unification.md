# Vulkan/OpenGL Unification: OpenGL Renderer Redundancy Audit

**Date:** 2026-11-27
**Scope:** `easy3d::renderer`, the OpenGL `Viewer`, GUI pickers, effects, Mapple and its retired Mapple-Qt reference source, tutorials, tests, and Python bindings.
**Status:** Architectural and caller audit, followed by the Milestone 2 compatibility repairs in section 8.11, the Milestone 3 declarative effect settings in section 8.12, the Milestone 4 host unification in section 8.13, the Milestone 5 picking unification in section 8.14, the completed analytic-clipping follow-up in section 8.15, and the completed unified-effect-presentation follow-up in section 8.16. Section 8.15 removed the remaining OpenGL analytic-impostor clipping limitation and its backend-specific picking/query fallbacks; section 8.16 moved settings-driven effects onto the shared command path and retired the remaining OpenGL effect-composition limitations.

## Executive summary

The Vulkan/shared-device migration has replaced selected OpenGL offscreen query paths, but it has not replaced the live OpenGL renderer. No major OpenGL rendering class is currently safe to delete as a unit.

The confirmed findings are:

1. **Two complete OpenGL utility units are already unused in-tree:**
   - `easy3d::opengl::read_pixel` free functions.
   - `easy3d::OpenGLTimer`.

2. **Several public member APIs have no active in-tree callers.** These are cleanup candidates, but many are installed public C++ API and therefore require a compatibility decision.

3. **The core native renderer remains required:**
   - `ShaderProgram` and `ShaderManager`.
   - `Texture` and `TextureManager`.
   - `VertexArrayObject` and native `Drawable` buffers.
   - `FramebufferObject`.
   - `ClippingPlane`.
   - Shadow, AO, EDL, and transparency effects.
   - `opengl::OverlayRenderer`.
   - Native shape drawing and `LinesDrawable2D`.

4. **The best future class-removal candidate is `OverlayRenderer`**, after `RenderDevice` gains a presentation/compositing path that preserves overlay ordering, depth testing, HiDPI behavior, capture behavior, and GL-state isolation.

5. **Native picking/depth fallbacks were retired in Milestone 5 and its follow-up.** Section 8.15 removed the last OpenGL analytic-impostor clipping fallback. The `Viewer` exact pass remains only for drawable/effect states the shared query cannot serve (for example AO textures, non-opaque drawables, unsupported coloring, or custom draw calls), not for clipping or standard impostors.

6. **`FramebufferObject`, shaders, textures, and VAO-backed drawables are not small-deletion candidates.** They are used by the live renderer, effects, overlays, and the new OpenGL adapter itself.

## Method and interpretation

The non-Vulkan renderer source list is defined in [easy3d/renderer/CMakeLists.txt:89](../easy3d/renderer/CMakeLists.txt#L89). The caller inventory counted actual calls and excluded comments, documentation, declarations, and `#if 0` examples.

Terms used in this report:

- **Unused in-tree:** no caller in Easy3D production code, tests, applications, tutorials, or bindings outside the symbol's own definition.
- **Python-only:** no C++ caller, but exposed through Python bindings.
- **Tests-only:** no production caller, but exercised by tests.
- **Required:** used by live rendering, exact fallback behavior, effects, or the new shared-device adapter.

“Unused in-tree” does not prove that downstream users do not depend on an API. The public include tree is installed by [CMakeLists.txt:357](../CMakeLists.txt#L357).

## Current architectural boundary

The shared backend-neutral pieces are:

- `graphics::RenderDevice`: offscreen rendering, capture metadata, picking, readback, and selection.
- `graphics::DrawCommand`: backend-neutral draw description.
- `graphics::DrawableRenderer`: converts a `Drawable` into shared commands.
- `graphics::OpenGLDevice`: headless OpenGL realization borrowing the caller's current context.

The OpenGL `Viewer` now uses `RenderDevice::present()` for ordinary live presentation, including portable overlays, effects, and transparency. Native drawable implementations remain the exact fallback for unsupported custom drawables, application-owned legacy shadows, and other scenes that cannot be encoded as portable draw commands; that fallback begins at [easy3d/viewer/viewer.cpp:2953](../easy3d/viewer/viewer.cpp#L2953) and calls:

- `LinesDrawable::draw()`.
- `PointsDrawable::draw()`.
- `TrianglesDrawable::draw()`.
- Native shadow and clipping rendering.
- Native overlay and effect rendering.

Consequently, the shared device provides the complete presentation path for portable scenes; native rendering remains available for compatibility cases the shared command model cannot represent.

## 1. Confirmed unused in-tree

### 1.1 Complete units

| Component | Finding | Recommended treatment |
|---|---|---|
| `easy3d::opengl::read_pixel` | No callers outside `read_pixel.{h,cpp}`; no tests or active Python bindings. This includes single-pixel and buffer `read_color`/`read_depth`, MSAA variants, and color/depth snapshot functions. See [easy3d/renderer/read_pixel.h:55](../easy3d/renderer/read_pixel.h#L55). | Remove declarations, definitions, and CMake entries if public API breaks are acceptable. Otherwise deprecate first. |
| `easy3d::OpenGLTimer` | No callers outside its own header/source; no tests or bindings. See [easy3d/renderer/opengl_timer.h:55](../easy3d/renderer/opengl_timer.h#L55). | Same treatment as `read_pixel`. |

Both are listed in the OpenGL-only headers and sources at [easy3d/renderer/CMakeLists.txt:92](../easy3d/renderer/CMakeLists.txt#L92). Removing them requires no rendering adaptation.

### 1.2 Public members with no active in-tree callers

The following are cleanup candidates. Public API policy still applies.

| Component | Unused surface | Notes |
|---|---|---|
| `ShaderManager` | `create_program_from_codes()`, `all_programs()`, and the individual-file `create_program_from_files()` overload | The base-name creation overload, `get_program()`, `reload()`, and `terminate()` remain used. See [easy3d/renderer/shader_manager.h:82](../easy3d/renderer/shader_manager.h#L82). |
| `ShaderProgram` | `set_verbose()`, `program_output()`, `is_program_valid()`, `load_binary()`, `save_binary()` | These belong to diagnostics/binary-program support with no external callers. `set_program_output()` remains used by `ShaderManager`. See [easy3d/renderer/shader_program.h:130](../easy3d/renderer/shader_program.h#L130). |
| `VertexArrayObject` | `is_supported()`, `create_storage_buffer()`, `update_storage_buffer()`, `map_buffer()`, `unmap_buffer()` | Core array/element buffer creation and `get_buffer_data()` remain required. See [easy3d/renderer/vertex_array_object.h:59](../easy3d/renderer/vertex_array_object.h#L59). |
| `FramebufferObject` | `is_supported()`, `attach_color_texture()`, `copy_color_to_texture()`, `copy_depth_to_texture()`, `is_bound()`, `num_color_attachments()`, `has_color_attachment()`, `has_depth_attachment()`, `depth_bits()`, `has_stencil()`, `texture_target()`, `has_color_texture()`, `has_depth_texture()`, `deactivate_read_buffer()`, and all `blit_framebuffer()` overloads | `attach_depth_texture()` is used by `OpenGLDevice`; draw/read-buffer activation and color readback remain used. See [easy3d/renderer/framebuffer_object.h:129](../easy3d/renderer/framebuffer_object.h#L129). |
| `FramebufferObject` depth helpers | Buffer `read_depth()` overloads and `snapshot_depth()` | The vector `read_depth()` overload is tests-only; the pointer overload has no caller. `snapshot_depth()` appears only in disabled code. Single-pixel `read_depth()` remains production-used. |
| `FramebufferObject` diagnostics | `print_attachments()`, `print_draw_buffers()`, `print_read_buffer()` | Referenced only from disabled debug code. |
| `OpenglUtil` | Profile, vendor, renderer, version, extension, GLSL/GLEW, sample, viewport, GPU-memory, output-stream, and `get_*_info` queries | Only `init()`, `is_supported()`, and `has_extension()` have production callers. See [easy3d/renderer/opengl_util.h:47](../easy3d/renderer/opengl_util.h#L47). |
| OpenGL error helpers | `check_gl_error()`, `check_frame_buffer_error()`, `easy3d_gl_error`, and `easy3d_frame_buffer_error` | The logging variants remain used throughout production code. See [easy3d/renderer/opengl_error.h:38](../easy3d/renderer/opengl_error.h#L38). |
| Native `shape` functions | Textured `draw_quad_filled()` and textured `draw_full_screen_quad()` | The bound-shader fullscreen-quad overload is heavily used. See [easy3d/renderer/shape.h:87](../easy3d/renderer/shape.h#L87). |
| `Texture` | `bind()` and `release()` | Tests-only; production rendering uses texture IDs through shader binding. Public API nonetheless. See [easy3d/renderer/texture.h:123](../easy3d/renderer/texture.h#L123). |

### 1.3 No external caller does not always mean dead code

Several `ShaderProgram` methods have no external caller but are used internally, including:

- `name()` and `set_name()`.
- `clear()`.
- `is_program_linked()` and `is_bound()`.
- `shader_info_log()` and `program_info_log()`.
- `print_active_*()`.
- `load_shader_source()`.

These should not be deleted merely because Viewer does not call them directly. Removal would require refactoring the class's internal diagnostics, loading, or lifetime logic.

## 2. Python-only or public-API-blocked members

The following have no C++ production caller but are exposed through Python:

- `Drawable::vao()`.
- `Drawable::texcoord_buffer()`.
- `Drawable::element_buffer()`.

The bindings are at [python/bindings/easy3d/renderer/drawables.cpp:227](../python/bindings/easy3d/renderer/drawables.cpp#L227) and [python/bindings/easy3d/renderer/drawables.cpp:331](../python/bindings/easy3d/renderer/drawables.cpp#L331).

`Drawable::vertex_buffer()`, `color_buffer()`, and `normal_buffer()` also have production callers in effects and native shape drawing. They are not deletion candidates today; the retired Mapple-Qt reference source was updated for API consistency.

Recommended treatment for Python-only methods:

1. Introduce backend-neutral predicates or resource queries.
2. Deprecate the raw GL accessors.
3. Remove the Python bindings only in a coordinated API release.

## 3. Components that remain required

| Component | Production dependencies | Conclusion |
|---|---|---|
| `FramebufferObject` | Viewer capture and fallbacks; `MultiViewer`; GUI pickers; AO, EDL, shadows, transparency; `OverlayRenderer`; `OpenGLDevice` | Required. The adapter itself allocates targets with it at [easy3d/renderer/backends/opengl/render_device.cpp:796](../easy3d/renderer/backends/opengl/render_device.cpp#L796). |
| `ShaderProgram` / `ShaderManager` | Native drawables, corner axes, picking fallbacks, overlays, and effects | Required. Viewer uses the manager directly at [easy3d/viewer/viewer.cpp:3044](../easy3d/viewer/viewer.cpp#L3044). |
| `Texture` / `TextureManager` | Native textured/scalar-field rendering; material handling; shared command extraction; Mapple UI and tutorials | Required. The shared device consumes CPU pixel data but does not replace live texture realization. |
| `VertexArrayObject` | Native buffer creation, element buffers, `Drawable::buffer_data()` readback, screen-space lines, shape drawing | Required. |
| Native `Drawable` storage | Live rendering and CPU snapshots used by `DrawableRenderer`/`OpenGLDevice` | Required. |
| `ClippingPlane` | Viewer controls, native shader setup, clipping visualization, pickers, effects, Mapple, tutorials | Required. See [easy3d/viewer/viewer.cpp:2628](../easy3d/viewer/viewer.cpp#L2628). |
| Shadow/AO/EDL/transparency | Viewer integration, Mapple, and rendering tutorials | Required as application/library functionality, even where base Viewer does not call every effect directly. |
| `opengl::OverlayRenderer` | HUD text and client overlays, including depth-tested overlays | Required today. See [easy3d/viewer/viewer.cpp:2507](../easy3d/viewer/viewer.cpp#L2507). |
| `LinesDrawable2D` | Pivot marks, screen-space guides, `MultiViewer`, portable command extraction, tutorials | Required. |
| Native `shape_draw` | Corner axes, fullscreen effect passes, manipulator visualization, and supported Mapple tools; retired Mapple-Qt reference code was updated for API consistency | Required for active rendering and compatibility fallbacks. |
| GLEW, `OpenglUtil::init()`, capability checks, error logging | Context initialization and native backend support checks | Required. |

### Why `FramebufferObject` cannot be removed merely by changing capture

Changing `Viewer::capture()` would remove only one caller. `FramebufferObject` would still be required by:

- `OpenGLDevice` color, scene-depth, and output targets.
- `OverlayRenderer` depth and HUD targets.
- Shadow maps.
- Ambient occlusion.
- Eye-dome lighting.
- Transparency passes.
- Native picker fallbacks.
- `MultiViewer`.

Removing the class therefore requires either migrating all those users or introducing a private adapter/effect target abstraction. That is not a small cleanup.

## 4. Candidates after bounded adaptation

### 4.1 `OverlayRenderer`: best future class-removal candidate

`OverlayRenderer` could be removed after `RenderDevice` gains a presentation/compositing operation and Viewer routes HUD and application overlays through it.

Required compatibility work:

- Foreground/background overlay ordering.
- Depth-tested and non-depth-tested overlays.
- HiDPI and top-left framebuffer coordinates.
- Snapshot/capture behavior.
- Caller GL-state isolation.
- Text/image command upload and lifetime.
- Exact interaction with corner axes and native scene rendering.

The earlier headless blocker is resolved: `OpenGLDevice::present()` now renders and blits to the current destination at [easy3d/renderer/backends/opengl/render_device.cpp:1417](../easy3d/renderer/backends/opengl/render_device.cpp#L1417). It still depends on the destination context being current.

**Assessment:** coherent future migration, but not a simple caller replacement.

### 4.2 Native picking and depth fallbacks

The old FBO/shader fallback blocks and `Picker::fbo_` can be removed incrementally as shared-device support expands to:

- Enabled clipping.
- AO and non-opaque drawables.
- Unsupported coloring or unavailable CPU texture data.
- Point/line impostors.
- Non-native line widths.
- `MultiViewer`.

The capability gate is visible at [easy3d/viewer/viewer.cpp:2659](../easy3d/viewer/viewer.cpp#L2659).

**Assessment:** retire individual fallback branches as parity is added. Do not remove the fallback system as one change.

**Status update:** Milestone 5 retired the picker-owned fallback blocks and routed model, pixel, point, and region picks through the shared query. Section 8.15 completed that work: the shared OpenGL adapter now handles clipping for triangles, plain points/lines, wide plain lines, and analytic point/line impostors, and the last `PointCloudPicker` native fallback is gone. The `Viewer` exact pass remains necessary for unsupported drawable/effect states and custom draw calls.

### 4.3 Viewer capture's local FBO usage

The local `FramebufferObject` in `Viewer::capture()` could eventually disappear if complete presented-scene capture moves behind `RenderDevice`.

That migration must preserve:

- Custom `Viewer::draw()` overrides.
- Shadows, AO, EDL, and transparency.
- Native overlays and HUD.
- Clipping-plane visualization.
- MSAA and background expansion.
- Existing snapshot semantics.

**Assessment:** possible later, but it does not make the `FramebufferObject` class redundant.

### 4.4 Raw GL resource members

`State::ao_texture_` is a raw OpenGL texture ID. It could be replaced by a backend-neutral resource handle, but only alongside an AO resource-handoff migration. It remains used by native AO and Mapple-Qt.

Similarly, raw `Drawable` buffer IDs and `vao()` could be replaced by backend-neutral resource queries, but the underlying native GL storage remains necessary while live OpenGL rendering uses it.

**Assessment:** API cleanup opportunity, not a small functional deletion.

## 5. Public API constraints

The renderer headers are installed as public includes. Therefore, a symbol with no in-tree caller may still be downstream API.

Additional constraints:

- Python exposes `Drawable` GL buffer getters, `gl_draw()`, and `vao()`.
- Python exposes `LinesDrawable2D`.
- Python exposes texture loading through `TextureManager` and AO controls through `State`.
- No active Python bindings were found for `FramebufferObject`, `read_pixel`, `ShaderManager`/`ShaderProgram`, VAO management methods, `OpenGLTimer`, `OpenglUtil`, native shape functions, `ClippingPlane`, or `OverlayRenderer`. `VertexArrayObject` is exposed only through a minimal compatibility class returned by deprecated `Drawable::vao()`, with `id()` as its sole bound method.

A safe cleanup policy is:

1. Confirm zero in-tree callers.
2. Check tests and bindings.
3. Decide whether downstream C++ compatibility matters.
4. Deprecate public APIs before removal when required.
5. Remove implementation and build entries together.
6. Run both OpenGL and Vulkan/shared test suites.

## 6. Recommended roadmap

### Phase 1: Low-risk internal cleanup

- Remove `read_pixel` and `OpenGLTimer` if API breaks are acceptable.
- Otherwise mark them deprecated.
- Prune unused `ShaderManager`, `ShaderProgram`, VAO, FBO, `OpenglUtil`, error-helper, and shape APIs after compatibility review.
- Keep internally used diagnostics and loading helpers unless their surrounding subsystem is deliberately removed.

### Phase 2: Unify overlay presentation

- Add a shared presentation/compositing contract.
- Implement it for OpenGL without violating the headless offscreen device contract.
- Migrate HUD and application overlays.
- Preserve depth-tested overlays and capture behavior.
- Delete `OverlayRenderer` only after parity is verified.

### Phase 3: Expand shared query parity

- Add clipping, AO, transparency, impostor, line-width, and `MultiViewer` support as appropriate.
- Remove each native fallback branch once its shared path is proven equivalent.
- Retire picker-owned FBOs and native picker shaders when no unsupported cases remain.

Milestone 5 completed the picker-owned FBO and native-picker retirement, and section 8.15 closed the remaining OpenGL analytic point/line clipping gap and its query fallbacks. The exact `Viewer` pass stays for genuinely unsupported drawable/effect states.

### Phase 4: Unify presented-scene capture

- Move complete capture, including effects, overlays, HUD, and custom drawing, behind a shared interface.
- Remove Viewer's direct capture FBO usage.
- Keep `FramebufferObject` until adapter, effects, and overlays no longer need it.

### Phase 5: Deep renderer unification

Only after live OpenGL presentation is migrated should the project consider:

- Replacing native `ShaderProgram`/`ShaderManager`.
- Replacing native `Texture` realization.
- Replacing VAO-backed `Drawable` storage.
- Removing `FramebufferObject`.
- Removing native shape and screen-space line implementations.

These are architectural migrations, not small deletion tasks.

## Conclusion

The migration has made **specific offscreen OpenGL implementations redundant**, not the OpenGL renderer as a whole. The immediately removable code consists primarily of unused utilities and convenience APIs. The meaningful class-level simplification will come from unifying overlay presentation and retiring exact native fallbacks; deeper removal of framebuffers, shaders, textures, and VAO-backed drawables requires migrating the live rendering path itself.




## 7. Proposed final OpenGL/Vulkan public API

This section is the target contract, not a description of APIs that all exist today. Section 8 records the Milestone 0 audit against the current C++, Python, example, and test surfaces; each future-looking example below is classified there.

**Status:** architectural proposal, not an implemented API or a commitment to exact signatures.
**Assumption:** all five roadmap phases above have been completed.

The end state should expose **one public API with no OpenGL/Vulkan conditionals**. Backend selection may remain a build-time choice; API unification does not by itself require shipping both backends in one library or switching them at runtime.

### 7.1 Design principles

1. **Familiar high-level API.** Ordinary users continue to use `Viewer`, `OffScreen`, `MultiViewer`, `Camera`, `Model`, `Renderer`, and drawables.
2. **One low-level rendering vocabulary.** Custom renderers use `graphics::RenderDevice` and `graphics::DrawCommand`, regardless of backend.
3. **No native handles in normal public APIs.** OpenGL IDs, VAOs, FBOs, shader programs, Vulkan objects, and swapchains remain private to backend implementations.
4. **Declarative effects.** Shadows, ambient occlusion, eye-dome lighting, transparency, and clipping are portable frame settings rather than backend-specific effect implementations.
5. **Identical source code.** Client code should not need `#ifdef EASY3D_HAS_VULKAN` or backend-specific includes.
6. **Capabilities are queried, not guessed.** Device limits and optional features are reported through `RenderDevice`.

The resulting layering is:

```text
Application API
  Viewer / OffScreen / MultiViewer
  Camera / Frame
  Model / Renderer / Drawable
  RenderSettings, capture, picking, overlays
                 |
                 v
Backend-neutral graphics API
  RenderDevice
  DrawCommand / Geometry / Vertex / Image
  FrameImage / ScenePixel / PointSelectionRequest
  DrawableRenderer
                 |
                 v
Private backend implementations
  OpenGL adapter       Vulkan adapter
  GL shaders/FBO/VAO   SPIR-V/pipelines/swapchain
```

### 7.2 High-level API

The existing foundation is already portable: see [WindowOptions and Viewer](../easy3d/viewer/viewer.h#L74), [ViewerBase](../easy3d/viewer/viewer_base.h#L79), and [OffScreen](../easy3d/viewer/offscreen.h#L49).

#### `WindowOptions`

```cpp
struct WindowOptions {
    int width = 800;
    int height = 600;
    int samples = 1;
    bool full_screen = false;
    bool visible = true;
    bool resizable = true;
};
```

#### `DeviceOptions`

Low-level `graphics::RenderDevice` creation uses the common diagnostics and device-selection options:

```cpp
namespace graphics {
struct DeviceOptions {
    bool validation = false;
    bool sync_validation = false;
    bool profiling = false;
    std::string device_name;
    bool use_native_large_points = true;
};
}
```

OpenGL context-version negotiation and Vulkan instance/device construction become implementation details.
The three rendering hosts instead take `easy3d::DeviceOptions`, whose public fields describe the configured
backend: Vulkan device/validation settings or OpenGL context requests. Keeping the host type in one name avoids
backend conditionals in constructor declarations; it is distinct from `graphics::DeviceOptions` because the host
options configure window/context construction, while the low-level options configure a standalone render device.

#### `Viewer`

Important public members would remain recognizable:

```cpp
class Viewer : public ViewerBase {
public:
    Viewer(const std::string& title = "Easy3D Viewer",
           const WindowOptions& window = {},
           const DeviceOptions& options = {});

    int run(bool see_all = true);
    int run_frames(unsigned max_frames = 0);
    bool draw_frame();

    void show();
    void hide();
    void resize(int width, int height);
    void update() const;

    Camera* camera();
    const Camera* camera() const;

    Model* add_model(const std::string& file,
                     bool create_drawables = true,
                     bool fit_scene = true);
    Model* add_model(std::shared_ptr<Model> model,
                     bool create_drawables = true,
                     bool fit_scene = true);
    Drawable* add_drawable(std::shared_ptr<Drawable> drawable);

    bool delete_model(Model* model);
    bool delete_drawable(Drawable* drawable);
    void clear_scene();
    void fit_screen(const Model* model = nullptr);

    PickResult pick(int x, int y) const;

    void set_overlays(std::vector<graphics::DrawCommand> commands,
                      bool foreground = false);
    void set_clipping_planes(const std::vector<vec4>& planes);
    void set_render_settings(const RenderSettings& settings);

    graphics::FrameImage capture() const;
    graphics::FrameImage capture(unsigned samples) const;
    graphics::FrameImage capture(
        unsigned samples,
        graphics::FrameReadback readback) const;

    bool snapshot(const std::string& file,
                  float scaling = 1.0f,
                  int samples = 4,
                  int background = 0,
                  bool expand = true) const;

    const graphics::RenderDevice& device() const;

protected:
    virtual void init();
    virtual void cleanup();
    virtual void pre_draw();
    virtual void post_draw();
    virtual void draw() const;

    virtual std::vector<graphics::DrawCommand> draw_commands(
        const Camera& camera,
        int width,
        int height,
        std::vector<DrawSource>* sources = nullptr,
        bool include_hidden_models = false) const;
};
```

The final change is that `device()`, `set_clipping_planes()`, and command assembly stop being Vulkan-only. Their current transitional forms are visible in [Viewer overlays and clipping](../easy3d/viewer/viewer.h#L854), [capture and device access](../easy3d/viewer/viewer.h#L918), and [draw_commands()](../easy3d/viewer/viewer.h#L1187).

A subclass customizes rendering by returning `DrawCommand` objects or adding backend-neutral commands, not by issuing GL calls.

#### `OffScreen` and `MultiViewer`

`OffScreen` should expose the same scene, camera, picking, overlay, clipping, device, and in-memory capture operations as `Viewer`, while keeping window controls protected:

```cpp
class OffScreen : protected Viewer {
public:
    OffScreen(int width = 800, int height = 600,
              const DeviceOptions& options = {});

    bool render(const std::string& file,
                float scaling = 1,
                int samples = 4,
                int background = 0,
                bool expand = true) const;

    graphics::FrameImage capture() const;
    graphics::FrameImage capture(unsigned samples) const;
    graphics::FrameImage capture(
        unsigned samples,
        graphics::FrameReadback readback) const;

    void resize(int width, int height);

    // Scene management, camera(), picking, overlays, clipping,
    // render settings, and device().
};
```

`MultiViewer` retains its grid-specific API:

- `assign(row, col, model/drawable)`
- `set_layout(rows, cols)`
- `rows()`, `columns()`
- `set_division_visible()`, `division_visible()`
- grid-aware `snapshot()`, coordinate conversion, and picking

It gains the same portable capture, overlays, clipping, render settings, and device access as the other hosts.

### 7.3 Drawables and scene resources

The public classes remain:

- `Drawable`
- `PointsDrawable`
- `LinesDrawable`
- `TrianglesDrawable`
- `LinesDrawable2D`
- `Renderer`
- `State`
- `Texture` and `TextureManager` as CPU-resource/cache facades

#### `Drawable`

```cpp
class Drawable : public State {
public:
    struct BufferData {
        std::vector<vec3> vertices;
        std::vector<vec3> normals;
        std::vector<vec3> colors;
        std::vector<vec2> texcoords;
        std::vector<unsigned int> indices;
        std::vector<int> source_faces;
        std::size_t revision = 0;
    };

    const std::string& name() const;
    void set_name(const std::string& name);

    const Box3& bounding_box() const;
    std::size_t num_vertices() const;
    std::size_t num_indices() const;

    const BufferData& buffer_data() const;

    void update_vertex_buffer(const std::vector<vec3>& vertices,
                              bool dynamic = false);
    void update_color_buffer(const std::vector<vec3>& colors,
                             bool dynamic = false);
    void update_normal_buffer(const std::vector<vec3>& normals,
                              bool dynamic = false);
    void update_texcoord_buffer(const std::vector<vec2>& texcoords,
                                bool dynamic = false);
    void update_element_buffer(const std::vector<unsigned int>& indices);
    void disable_element_buffer();

    void update();
    void set_update_func(const std::function<void(Model*, Drawable*)>& func);

    void update_highlight_mask(const std::vector<float>& strengths);
    const std::vector<float>& highlight_mask() const;
};
```

The backend-specific APIs and members currently declared in [Drawable native storage and rendering](../easy3d/renderer/drawable.h#L505) would be removed:

```cpp
// Removed from the final public API
virtual void draw(const Camera*) const;
void gl_draw() const;
VertexArrayObject* vao();
unsigned int vertex_buffer();
unsigned int color_buffer();
unsigned int normal_buffer();
unsigned int texcoord_buffer();
unsigned int element_buffer();
```

`buffer_data()` becomes the portable geometry/resource query. GPU realization is private and may still use native VBOs, EBOs, and VAOs inside the OpenGL adapter.

#### Primitive-specific drawables

```cpp
class PointsDrawable : public Drawable {
public:
    enum ImposterType { PLAIN, SPHERE, SURFEL };

    void set_impostor_type(ImposterType type);
    ImposterType impostor_type() const;
    void set_point_size(float diameter);
    float point_size() const;
};

class LinesDrawable : public Drawable {
public:
    enum ImposterType { PLAIN, CYLINDER, CONE };

    void set_impostor_type(ImposterType type);
    ImposterType impostor_type() const;
    void set_line_width(float width);
    float line_width() const;
};

class TrianglesDrawable : public Drawable {
public:
    void set_smooth_shading(bool enabled);
    bool smooth_shading() const;
    void set_opacity(float opacity);
    float opacity() const;
};
```

`Renderer` continues to own and retrieve drawables through methods such as:

- `get_points_drawable()`
- `get_lines_drawable()`
- `get_triangles_drawable()`
- `add_points_drawable()`
- `add_lines_drawable()`
- `add_triangles_drawable()`
- `update()`
- `points_drawables()`, `lines_drawables()`, `triangles_drawables()`

### 7.4 Effects become declarative `RenderSettings`

The current `Shadow`, `SoftShadow`, `AmbientOcclusion`, `EyeDomeLighting`, and transparency classes duplicate backend-specific implementations. Their public controls should converge on portable settings:

```cpp
struct ShadowSettings {
    bool enabled = false;

    unsigned map_size = 1024;
    float light_distance = 50.0f;
    float darkness = 0.6f;
    float softness = 0.0f;

    bool virtual_background = false;
    vec4 virtual_background_color = vec4(0.8f);
};

struct AmbientOcclusionSettings {
    bool enabled = false;
    float radius_ratio = 0.045f;
    float bias = 0.005f;
    float intensity = 1.0f;
};

struct TransparencySettings {
    graphics::TransparencyMethod method =
        graphics::TransparencyMethod::sorted;
    unsigned layers = 4;
};

struct RenderSettings {
    ShadowSettings shadow;
    AmbientOcclusionSettings ambient_occlusion;
    bool eye_dome_lighting = false;
    TransparencySettings transparency;
    std::vector<vec4> clipping_planes;
};
```

Example:

```cpp
easy3d::RenderSettings settings;

settings.shadow.enabled = true;
settings.shadow.softness = 0.5f;

settings.ambient_occlusion.enabled = true;
settings.ambient_occlusion.radius_ratio = 0.04f;

settings.transparency.method =
    easy3d::graphics::TransparencyMethod::peeling;
settings.transparency.layers = 8;

viewer.set_render_settings(settings);
```

This makes the following obsolete as public backend-facing APIs:

- `Shadow::draw()`
- `AmbientOcclusion::generate()`
- `AmbientOcclusion::ao_texture()`
- `State::set_ao_texture(unsigned int)`
- `Transparency::set_frame_renderer()`

The algorithms remain; only their framebuffer, shader, and command-assembly plumbing becomes private. Compatibility facades such as `SoftShadow` or `DualDepthPeeling` may temporarily translate into `RenderSettings`.

### 7.5 Backend-neutral graphics API

The existing [RenderDevice interface](../easy3d/renderer/render_device.h#L522) and its [command/value types](../easy3d/renderer/render_device.h#L40) are already close to the desired final form.

#### Public value types

```cpp
namespace easy3d::graphics {

enum class Backend { opengl, vulkan };
enum class Topology { triangles, lines, points };
enum class TextureWrap { clamp_to_edge, repeat };
enum class TextureFilter { nearest, linear };
enum class PointStyle { plain, sphere, surfel };
enum class LineStyle { plain, cylinder, cone };
enum class TransparencyMethod { sorted, average, peeling, linked_lists };
enum class FrameReadback { color, color_depth_ids };

struct Vertex {
    vec3 position;
    vec3 normal;
    vec4 color;
    vec2 texcoord;
    vec4 highlight;
};

class PointData {
    // positions(), source_ids(), source_count()
};

class Geometry {
    // vertices(), indices(), topology(), primitive_ids(), point_data()
};

struct Sampler {
    TextureWrap wrap;
    TextureFilter filter;
};

class Image {
    // width(), height(), rgba(), sampler()
};

struct DrawCommand {
    std::shared_ptr<const Geometry> geometry;
    std::shared_ptr<const Image> texture;

    mat4 model_view_projection;
    mat3 normal_matrix;
    vec4 color;
    std::uint32_t object_id;
    ivec4 viewport;

    // Material, clipping, point/line style, transparency,
    // overlay/depth, AO, EDL, and shadow state.
};

struct FrameImage {
    struct Id {
        std::uint32_t object;
        std::uint32_t primitive;
    };

    std::uint32_t width;
    std::uint32_t height;
    std::vector<unsigned char> rgba;
    std::vector<float> depth;
    std::vector<Id> ids;
};

struct ScenePixel {
    FrameImage::Id id;
    float depth;
};

struct PointSelectionRequest {
    // points, polygon, selection words, MVP, dimensions, clipping
};

}
```

`DrawCommand` can retain explicit state fields for source stability. Helper factories can cover common cases such as scene triangles, screen-space lines, textured quads, and overlays.

#### `RenderDevice`

The current long positional argument lists should be grouped into options:

```cpp
struct RenderOptions {
    vec4 background = vec4(1);
    unsigned samples = 1;
    TransparencyMethod transparency = TransparencyMethod::sorted;
    unsigned peel_layers = 4;
    FrameReadback readback = FrameReadback::color_depth_ids;
};

struct DeviceCapabilities {
    bool shadow = false;
    bool ambient_occlusion = false;
    bool eye_dome_lighting = false;
    std::vector<TransparencyMethod> transparency_methods;
    bool native_large_points = false;
};

class RenderDevice {
public:
    virtual ~RenderDevice() = default;

    // Added with the 3.0 virtual interface; absent from the 2.8 vtable.
    virtual Backend backend() const = 0;
    virtual const std::string& name() const = 0;
    virtual const std::string& driver_version() const = 0;
    virtual DeviceCapabilities capabilities() const = 0;

    virtual std::uint32_t supported_sample_counts(
        PointStyle style = PointStyle::plain) const = 0;
    virtual std::uint32_t supported_sample_counts(LineStyle style) const = 0;

    virtual FrameImage render(
        std::uint32_t width,
        std::uint32_t height,
        const std::vector<DrawCommand>& commands,
        const RenderOptions& options = {}) = 0;

    virtual ScenePixel sample_pixel(
        std::uint32_t width,
        std::uint32_t height,
        const std::vector<DrawCommand>& commands,
        std::uint32_t x,
        std::uint32_t y) = 0;

    virtual std::vector<FrameImage::Id> sample_ids(
        std::uint32_t width,
        std::uint32_t height,
        const std::vector<DrawCommand>& commands,
        std::uint32_t x,
        std::uint32_t y,
        std::uint32_t region_width,
        std::uint32_t region_height) = 0;

    virtual std::vector<std::uint32_t> select_points(
        const PointSelectionRequest& request) = 0;

    virtual unsigned validation_errors() const = 0;
    virtual std::uint64_t point_upload_count() const = 0;
    virtual std::uint32_t oit_dropped_fragments() const = 0;
};
```

Factories become backend-neutral:

```cpp
std::unique_ptr<RenderDevice> create_device(
    const DeviceOptions& options = {});
```

`create_opengl_device()` and `create_vulkan_device()` remain temporarily as deprecated wrappers. Public concrete classes such as `OpenGLDevice` and `VulkanDevice` are unnecessary; callers program against `RenderDevice`.

Presentation is deliberately absent from this headless contract. A configured window/surface is owned by the private host adapter; `RenderDevice::present()` and the proposed generic `Surface`/`create_surface_device()` API are not part of the normal public surface. Embedders that need native window integration use the explicit backend interop header, isolated from backend-neutral application code.

`DrawableRenderer` remains the bridge from scene drawables to commands. Its final role is to own portable geometry/texture caches and produce one or more `DrawCommand` objects per drawable; it does not own devices, windows, cameras, or native GPU resources.

### 7.6 Example: interactive viewer

The same source builds and runs against either backend:

```cpp
#include <easy3d/viewer/viewer.h>
#include <easy3d/util/initializer.h>

int main() {
    easy3d::initialize();

    easy3d::Viewer viewer("My application");

    auto* model = viewer.add_model("bunny.ply");
    if (model) {
        auto* points = model->renderer()->get_points_drawable("vertices");
        if (points) {
            points->set_impostor_type(easy3d::PointsDrawable::SPHERE);
            points->set_point_size(3.0f);
        }
    }

    return viewer.run();
}
```

No backend macro or backend-specific include is required.

### 7.7 Example: custom drawable

```cpp
auto points = std::make_shared<easy3d::PointsDrawable>("my points");

points->update_vertex_buffer(std::vector<easy3d::vec3>{
    {0.0f, 0.0f, 0.0f},
    {1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.5f}
});

points->set_uniform_coloring(easy3d::vec4(1.0f, 0.2f, 0.1f, 1.0f));
points->set_point_size(5.0f);

viewer.add_drawable(points);
```

The drawable contains portable geometry and appearance state. The host and selected backend handle upload, drawing, picking, capture, and resource lifetime.

### 7.8 Example: offscreen rendering and capture

```cpp
easy3d::OffScreen canvas(1024, 768);
canvas.add_model("bunny.ply", true, true);

auto frame = canvas.capture(
    4, easy3d::graphics::FrameReadback::color);

// frame.rgba is tightly packed RGBA8, top row first.
canvas.render("result.png");
```

For depth and picking data:

```cpp
auto frame = canvas.capture(
    4, easy3d::graphics::FrameReadback::color_depth_ids);

if (!frame.depth.empty() && !frame.ids.empty()) {
    const std::size_t pixel =
        (frame.height / 2) * frame.width + frame.width / 2;
    const float depth = frame.depth[pixel];
    const auto id = frame.ids[pixel];
}
```

### 7.9 Example: low-level custom triangle

```cpp
auto device = easy3d::graphics::create_device();

std::vector<easy3d::graphics::Vertex> vertices(3);
vertices[0].position = easy3d::vec3(-0.8f, -0.8f, 0);
vertices[1].position = easy3d::vec3( 0.8f, -0.8f, 0);
vertices[2].position = easy3d::vec3( 0.0f,  0.8f, 0);

auto geometry = std::make_shared<easy3d::graphics::Geometry>(
    vertices,
    std::vector<std::uint32_t>{0, 1, 2},
    easy3d::graphics::Topology::triangles);

easy3d::graphics::DrawCommand command;
command.geometry = geometry;
command.color = easy3d::vec4(0.2f, 0.7f, 1.0f, 1.0f);
command.object_id = 7;

easy3d::graphics::RenderOptions options;
options.samples = 4;

auto image = device->render(512, 512, {command}, options);
```

This code does not change when `create_device()` returns an OpenGL or Vulkan implementation.

### 7.10 Python surface

Python should expose the same conceptual API:

- `Viewer`, `OffScreen`, and `MultiViewer`
- `Camera`, models, `Renderer`, and drawables
- appearance/state setters and geometry updates
- render settings, overlays, clipping, picking, and capture
- `FrameImage` with array-like or NumPy-compatible color, depth, and ID data
- optionally the low-level `RenderDevice`, `DrawCommand`, `Geometry`, and `Image` types for advanced users

Raw GL buffer IDs, `vao()`, `gl_draw()`, and texture IDs should not be exposed.

```python
import easy3d

easy3d.initialize()

canvas = easy3d.OffScreen(1024, 768)
canvas.add_model("bunny.ply")

frame = canvas.capture(samples=4)
pixels = frame.rgba  # height x width x 4 RGBA8 array

canvas.render("result.png")
```

### 7.11 Public APIs removed or made private

| Current API | Final replacement |
|---|---|
| `Drawable::draw()`, `gl_draw()` | `DrawableRenderer` / `draw_commands()` |
| `Drawable::vao()` and raw buffer IDs | `Drawable::buffer_data()` |
| `FramebufferObject` | Private backend render targets |
| `ShaderProgram`, `ShaderManager` | Private backend pipelines/shaders |
| `VertexArrayObject` | Private backend vertex layouts |
| `Texture::id()`, `bind()`, `release()` | `graphics::Image` / CPU `Texture` data |
| `State::set_ao_texture()` | `RenderSettings::ambient_occlusion` |
| `opengl::OverlayRenderer` | `DrawCommand` overlay presentation |
| Native immediate-mode `shape` drawing | Shape geometry/command builders |
| `opengl::read_color`, `read_depth`, their `_ms` variants, and `snapshot_color`/`snapshot_depth` helpers | `RenderDevice::render()` / `FrameImage`, `sample_pixel()`, and `sample_ids()` |
| `OpenGLTimer` | Render-device diagnostics |
| Backend-specific public option structs | Common `DeviceOptions` |
| Backend-specific device factories | `create_device()`; native window integration stays in the explicit backend interop header |

### 7.12 Compatibility and migration policy

The final API should be reached without forcing every downstream application to migrate at once:

1. Add the portable replacement alongside the existing API.
2. Route both backends through the replacement.
3. Verify image, picking, capture, overlay, and interaction parity.
4. Deprecate native public APIs with documented replacements.
5. Remove native declarations, definitions, bindings, and build entries in a coordinated release.
6. Keep a compatibility release only where downstream C++ or Python compatibility justifies it.

The essential outcome is **a smaller backend-facing API and a familiar application API**. OpenGL and Vulkan should differ only in private implementations, performance characteristics, and reported capabilities—not in public classes, native handles, or client-side `#ifdef`s.

---

## 8. Milestone 0: current-API audit and frozen direction

Audit date: 2026-11-26. This audit compares section 7 with the current tree, including installed headers, Python bindings, examples, and tests. It freezes direction and compatibility rules; it does not claim that the target API has been implemented.

Status vocabulary:

- **Retain** — already matches the target contract.
- **Add** — absent and required by the target contract.
- **Adapt** — exists, but its signature, backend coverage, ownership, or semantics must change.
- **Deprecate** — keep temporarily with a documented replacement.
- **Private** — remove from the normal installed/public surface while retaining the implementation internally.
- **Remove** — delete the redundant declaration, definition, binding, and build/install entry together.

### 8.1 User-facing hosts and scene ownership

| Proposed surface | Current implementation evidence | Classification | Milestone 0 conclusion |
|---|---|---|---|
| `WindowOptions::{width, height, samples, full_screen, visible, resizable}` | [viewer.h](../easy3d/viewer/viewer.h#L74) already defines all six fields with the proposed defaults. | Retain | Keep the struct and its current sample-count validation/fallback contract. |
| `DeviceOptions::{validation, sync_validation, profiling, device_name}` | Vulkan has these controls in `VulkanOptions`; OpenGL instead has `OpenGLOptions` for context requests. | Add | Introduce one common struct. OpenGL context negotiation becomes private implementation detail. Preserve the existing Vulkan large-point comparison control in an advanced/common option rather than silently dropping it. |
| `Viewer(title, window, options)` | Constructor selects `VulkanOptions` or `OpenGLOptions` through `EASY3D_HAS_VULKAN` in [viewer.h](../easy3d/viewer/viewer.h#L144). | Adapt | Replace the client-visible conditional type with `DeviceOptions`; retain temporary backend-option overloads during migration. |
| `run`, `run_frames`, `draw_frame`, `show`, `hide`, `resize`, `update`, `camera` | All are available on both backends through `Viewer`/`ViewerBase`; Python omits `draw_frame`, `show`, and `hide`. | Retain C++; add Python parity later | No signature redesign is needed. |
| `add_model(file, create_drawables, fit_scene)` | Available on both backends; Python's file overload omits `fit_scene`. | Retain C++; adapt Python | Add `fit_scene` to Python without changing existing calls. |
| `add_model(shared_ptr<Model>, create_drawables, fit_scene)` | Shared ownership is already supported; raw-pointer overloads transfer ownership. | Adapt | Retain the `shared_ptr` overload. Deprecate raw-pointer ownership transfer and coordinate return-type modernization with the separate raw-pointer TODO. |
| `add_drawable(shared_ptr<Drawable>)` | Shared ownership is already supported; raw-pointer overloads transfer ownership. | Adapt | Retain the `shared_ptr` overload; deprecate raw-pointer ownership transfer and coordinate return types with the model migration. |
| `delete_model`, `delete_drawable`, `clear_scene`, `fit_screen` | Available on all C++ hosts. Python exposes deletion on `OffScreen` but not on `Viewer`/`MultiViewer`. | Retain C++; add Python parity | Raw pointer parameters here are non-owning observers, distinct from the ownership-transfer overloads. |
| `pick`, `set_overlays` | Portable C++ APIs already exist and are used by both hosts. | Retain | Keep command-list overlay submission and `PickResult`. |
| `select_points`, `point_resource` | Both exist, but OpenGL currently requires a viewer-created native point snapshot while Vulkan also accepts portable CPU `PointData`. | Adapt | Keep the methods and make portable `PointData` requests behave consistently on both backends. |
| `scan_visible_points` | Public only in Vulkan builds; deleted because it had no production callers (tests only). In 2.8.x a tutorial demonstration will live in `Tutorial_405_VirtualScanner`, using public `Viewer`/`OffScreen` capture APIs rather than a `Viewer` member. | Removed | Deleted as a preparation step for Milestone 6; no replacement `Viewer` member. |
| Other existing host queries/callbacks (`models`, `drawables`, `current_model`, window properties, animation/event hooks) | Already shared through `ViewerBase`/`Viewer` and omitted only for brevity in section 7. | Retain | The proposal is not a deletion list for unrelated portable APIs. |
| `set_clipping_planes` | Public only in Vulkan builds; OpenGL currently has no equivalent setter. | Adapt | Make the same portable operation available in both builds. |
| `set_render_settings` | No declarative settings API exists. | Add | Add with the effect settings in section 8.3. |
| `capture(samples, readback)` and legacy capture overloads | `Viewer` already has no-argument, sample-count, and readback overloads; `OffScreen` re-exposes them. | Retain | Preserve all overloads during migration; freeze their readback semantics in section 8.2. |
| `snapshot(file, scaling, samples, background, expand)` | Already portable and Python-bound. | Retain | Keep it as the file-output convenience API. |
| `device()` | Public only in Vulkan builds. OpenGL has a private query device. | Adapt | Expose `const RenderDevice&` on all three hosts without exposing native handles. |
| `init`, `pre_draw`, `post_draw`, `draw`, `cleanup` | Lifecycle/drawing hooks exist; `cleanup()` is not virtual. | Adapt | Retain the hooks and make `cleanup()` protected virtual so subclasses have the documented symmetric cleanup hook. |
| `draw_commands(camera, width, height, sources, include_hidden_models)` | Protected shared declaration exists, but only the Vulkan path defines it. | Adapt | Define the shared assembly contract for both backends; keep it protected for customization. |
| `DrawSource` | Protected `{shared_ptr<Model>, shared_ptr<Drawable>}` already exists. | Retain | Keep it protected as a command-assembly snapshot descriptor. Its `shared_ptr` members keep snapshots alive; the host remains authoritative for scene membership. |
| `ViewerBase::window()` / `Viewer::window()` | Public native `GLFWwindow*` accessor. | Deprecate, then private/protected | Native window handles are not part of the normal portable API. Internal toolkit integration must use a documented interop boundary. |
| `OffScreen` visibility | Already inherits `Viewer` protected and re-exposes scene/rendering operations. | Retain/adapt | Keep window-only operations protected; expose common scene, camera, picking, overlay, clipping, settings, capture, and device operations publicly. Keep `draw_commands()` protected for subclasses. |
| `MultiViewer` grid API | `assign`, layout, rows/columns, divisions, grid-aware snapshot/coordinates/picking already exist. | Retain | Keep the grid-specific API and add the same portable effects/device surface inherited by other hosts. |

### 8.2 Drawables, resources, and effects

| Proposed surface | Current implementation evidence | Classification | Milestone 0 conclusion |
|---|---|---|---|
| `Drawable::BufferData` fields | [drawable.h](../easy3d/renderer/drawable.h#L67) already has vertices, normals, colors, texcoords, indices, source faces, and revision. | Retain | Keep the existing fields and revision meaning. |
| `name`, `set_name`, `bounding_box`, `num_vertices`, `num_indices` | All exist in C++. `num_indices` is missing from Python. | Retain C++; add Python | Bind the portable query surface, not native resource IDs. |
| `buffer_data() const` | Exists but is non-const because it may refresh/read the backend snapshot. | Adapt | Provide a const query without exposing backend handles. Cache refresh may remain mutable internally; callers receive a stable snapshot for the current revision. |
| Vertex/color/normal/texcoord/element updates and element disabling | Portable C++ implementations exist; Python broadly exposes them. | Retain | Keep current defaults and dynamic-update behavior. |
| `update`, `set_update_func`, `highlight_mask` | Portable C++ APIs exist. The Python `update_highlight_mask` binding still names a removed two-argument overload. | Retain C++; adapt Python | Keep the one-argument C++ API; repair the stale binding and compile Python before claiming parity. |
| `set_source_faces`, source-face provenance | Already supported by `BufferData`; omitted from the abbreviated proposal. | Retain | Add to the final API listing because picking/provenance depend on it. |
| `Drawable::draw`, `gl_draw`, `vao`, raw buffer getters | Public C++ APIs; `draw`, `gl_draw`, `vao`, and raw buffer IDs are also Python-bound. | Deprecate, then private/remove | Replace with `DrawableRenderer`/command assembly before removal. Delete C++, Python, tests, tutorials, declarations, definitions, and install entries together. |
| `PointsDrawable` style/size API | Plain/sphere/surfel and logical-pixel size API already exist in C++ and Python. | Retain | Keep logical size at the drawable boundary; convert once to framebuffer pixels in command assembly. |
| `LinesDrawable` style/width API | Plain/cylinder/cone and logical-pixel width API already exist in C++ and Python. | Retain | Keep the same unit boundary as points. |
| `TrianglesDrawable` smooth shading/opacity | Already exists in C++ and Python. | Retain | No new primitive-specific abstraction is needed. |
| `Renderer` drawable ownership/retrieval/update API | All proposed getters, adders, lists, and `update()` exist. | Retain | Keep the existing ownership model. |
| `State` appearance API | Portable coloring/material/clipping state is broadly exposed in C++ and Python. | Retain/adapt | Keep appearance state; remove native AO handoff from the normal API. |
| `Texture`, `TextureManager`, `State::set_ao_texture` | CPU texture pixels exist, but public `Texture::id/bind/release` and raw AO texture IDs remain. Python uses `TextureManager` internally but does not expose the class. | Adapt; deprecate native members | Keep CPU-resource/cache facades. Correct the proposal's claim that Python exposes `TextureManager`. |
| Texture/image row order | `Texture::PixelData` is bottom-row-first; `graphics::Image` is top-row-first; `DrawableRenderer` converts between them. | Adapt documentation, retain explicit types | Never introduce implicit global flip state. Document the conversion boundary and add parity coverage. |
| `ShadowSettings`, `AmbientOcclusionSettings`, `TransparencySettings`, `RenderSettings` | Imperative effect classes exist; no declarative settings type exists. | Add | Add the proposed value types with defaults/units/validation. |
| `Shadow`, `SoftShadow`, `AmbientOcclusion`, `EyeDomeLighting`, transparency classes and native handoffs | Backend-facing public controls exist. | Deprecate/private | Algorithms remain internally; public facades translate to `RenderSettings` until applications and Python migrate. |

### 8.3 Backend-neutral graphics contract

| Proposed surface | Current implementation evidence | Classification | Milestone 0 conclusion |
|---|---|---|---|
| `Backend` | Absent; `name()` embeds backend/device information. | Add | Add an explicit enum and `RenderDevice::backend()` while retaining `name()` for human-readable identity. |
| `Topology`, `PointStyle`, `LineStyle`, `TransparencyMethod`, `FrameReadback` | Already defined in [render_device.h](../easy3d/renderer/render_device.h#L40). | Retain | Reuse these enums. |
| `TextureWrap`, `TextureFilter` | Already defined but omitted from the abbreviated proposal. | Retain | Add them to the final public listing. |
| `PointData` | Already defined and used by selection/geometry. | Retain | Add it to the final public listing. |
| `Vertex`, `Geometry`, `Sampler`, `Image` | Already backend-neutral; geometry snapshots own copied data and images use top-row-first RGBA8. | Retain | Keep current value/resource semantics. |
| `DrawCommand` | Already contains the proposed core fields plus material, clipping, overlay/depth, AO/EDL, shadow, impostor, and bias state. | Retain | Keep explicit fields for source stability; helper factories may be added later. |
| `FrameImage`, `FrameImage::Id`, `ScenePixel` | Already defined with top-row-first color and [0,1] depth. | Retain | Keep defaults and add array-like Python access later. |
| `PointSelectionRequest` | Already defined with point mapping, polygon, bitset, MVP, dimensions, and clipping. | Retain | Reuse it. |
| `normalize_clipping_planes`, `screen_space_projection` | Already public backend-neutral helpers. | Retain | Keep them in the final listing. |
| `RenderDevice::name`, `driver_version`, sample-count queries | Already portable. | Retain | Keep human-readable identity and numeric sample-count masks. |
| `RenderDevice::render` | Exists with positional background/samples/transparency/peel/readback arguments. | Adapt | Add `RenderOptions`; retain positional overloads as deprecated forwarding wrappers during migration. |
| `RenderDevice::present`, `PresentationOptions` | `present()` exists only for a configured presentation device; OpenGL reports it unsupported. No `PresentationOptions` exists. | Private/remove from normal `RenderDevice` | Presentation belongs to the private host/surface adapter, not the headless rendering contract. High-level users configure windows through `WindowOptions`; no generic public `Surface` is required. |
| `sample_pixel`, `sample_ids`, `select_points` | Already portable in both adapters, with documented CPU/GPU implementation differences. | Retain | Keep result semantics; performance may differ. |
| `validation_errors`, `oit_dropped_fragments` | Already portable diagnostics. | Retain | Keep them. |
| `point_upload_count` | Already exists but was omitted from the proposal. | Retain | Add this transfer diagnostic to the final listing. |
| `create_device` | Absent; callers choose a backend-specific factory. | Add | Add a build-time-selected factory using `DeviceOptions`. |
| `create_surface_device`, generic `Surface` | Absent and underspecified; current native Vulkan interop uses `graphics::VulkanSurface`. | Remove from normal final API | Do not invent a generic native-handle wrapper in Milestone 0. Keep native integration under an explicit backend interop boundary for embedders. |
| `create_opengl_device`, `create_vulkan_device` | Already public factories with backend-specific parameters. | Adapt/deprecate | Forward to the common factory and retain temporarily for compatibility. |
| `OpenGLDevice`, `VulkanDevice` | Concrete OpenGL class is installed; Vulkan implementation is internal. | Private/remove public declaration | Applications program against `RenderDevice`. |
| `DrawableRenderer` | Already owns CPU command/texture caches and explicitly does not own scene objects, cameras, devices, windows, or GPU resources. | Retain | Keep this ownership boundary. Each host owns its bridge; callers may create an independent bridge for command generation. |

### 8.4 Python, examples, and tests

| Surface | Current implementation evidence | Classification | Milestone 0 conclusion |
|---|---|---|---|
| Python `Viewer`, `MultiViewer`, `OffScreen`, `Camera`, models, `Renderer`, drawables | Bound today, with inconsistent constructor and scene-operation coverage. | Retain/adapt | Keep the classes; align common operations and constructor options with C++. |
| Python appearance/state and geometry updates | Broadly bound, including primitive styles and highlight masks. | Retain | Reuse current bindings; add missing portable queries such as `BufferData` and `num_indices`. |
| Python render settings, overlays, clipping, picking, capture, `FrameImage` | Missing. | Add | Add after the C++ contract is stable; `FrameImage` should expose NumPy-compatible arrays. |
| Python low-level device/value types | Missing. | Add (optional advanced surface) | Expose only after the C++ API is frozen and both adapters pass the same tests. |
| Python raw buffer IDs, `draw`, `gl_draw`, `vao`, raw AO texture setter | Currently bound. | Deprecate, then remove | Replace with portable data/command APIs before deleting bindings. |
| Python `TextureManager` | Not exposed; `State.set_texture` uses it internally. | Correct proposal | Do not document a class that Python users cannot currently import. |
| Section 7.6 interactive example | Uses APIs that already exist in both C++ backends. | Retain | Keep as the primary top-down example. |
| Section 7.7 custom-drawable example | Uses existing portable updates/appearance and shared ownership. | Retain | Add to compile-smoke coverage. |
| Section 7.8 offscreen/capture example | Uses existing C++ `OffScreen::add_model`, `capture`, and `render`. | Retain | Add to compile-smoke coverage; runtime parity remains a separate test. |
| Section 7.9 low-level example | Fails today because `create_device()` and `RenderOptions` do not exist. | Add prerequisites | Do not present it as buildable until Milestone 1 lands. |
| Section 7.10 Python example | Fails today because Python `OffScreen.capture()`/`FrameImage` do not exist. | Add prerequisites | Do not present it as usable until Milestone 7 lands. |
| Installed-package consumer | `tests/viewer/installed/unified.cpp` still branches on `EASY3D_EXPECT_VULKAN` and backend option types. | Adapt | Add a truly backend-neutral compile-smoke consumer now; simplify the installed consumer after Milestone 1. |
| Runtime tests | Existing tests cover many backend behaviors, but not an unchanged-source public-API compile matrix. | Add | Add compile-smoke coverage for interactive, offscreen, multi-view, custom-drawable, and low-level-device usage without constructing windows/devices. |

### 8.5 Audit result

The existing tree already contains most of the desired application and geometry vocabulary. The critical path is not a rewrite: it is a common device-construction contract, shared command assembly on OpenGL, declarative effects, consistent host access, and deliberate retirement of native handles. Python parity is a distinct later milestone and must not be inferred from C++ progress.

### 8.6 Frozen public-contract decisions

#### Device construction and capabilities

- `DeviceOptions` is the single normal construction options type. It contains `validation`, `sync_validation`, `profiling`, `device_name`, and `use_native_large_points`, all `false`/empty/`true` as shown in section 7.2. The last flag preserves the existing Vulkan comparison/fallback control; backends without that choice accept and ignore it.
- `create_device()` selects the backend configured at build time. There is no runtime backend switch and no client-side backend macro. `RenderDevice::backend()` reports the selected implementation in the final interface.
- Section 7 shows the final 3.0 interface. During 2.8, do not add `backend()`, `capabilities()`, or virtual `cleanup()` to an existing public vtable; these arrive with the coordinated 3.0 virtual-interface change.

#### Compatibility overload resolution

- In 2.8, the existing backend-options constructor keeps its default arguments and symbol. The portable constructor is a C++11 constrained template accepting exactly `DeviceOptions`, with no default trailing argument. Template deduction cannot consume a bare `{}`, so existing `Viewer("title", {}, {})` calls keep their current meaning; new callers pass `DeviceOptions options; Viewer("title", {}, options)` or `DeviceOptions{}`. Apply the same rule to `OffScreen` and `MultiViewer`. In 3.0, remove the legacy constructor and use the ordinary concrete constructor shown in section 7.
- In 2.8, `render(width, height, commands, const RenderOptions&)` is an exact-type constrained template with no default argument and forwards directly to the existing eight-argument positional virtual. Thus three-argument calls keep using the legacy overload, while a bare `{}` still value-initializes the legacy `vec4` parameter instead of attempting template deduction. New callers pass an explicit `RenderOptions` value. In 3.0, remove the positional overload and make the options overload the primary virtual with the default shown in section 7.
- These transitional forms avoid both default-argument and braced-initializer ambiguity without changing 2.8 symbols or virtual tables. Compile-smoke tests cover the current 2.8 call forms; the future overloads get their own ambiguity tests when introduced.

#### Requested options and fallback behavior

- Requested validation, synchronization validation, a named device, or a usable graphics context/device are hard requirements: failure to satisfy them throws. They are not silently ignored.
- MSAA is a quality request: `0`/`1` means single-sample; supported power-of-two requests are honored; unsupported valid power-of-two requests step down to the largest supported count with a warning; invalid counts throw. `supported_sample_counts()` remains the authoritative query.
- `DeviceCapabilities` describes shadow, AO, EDL, supported transparency methods, and native large-point availability for backend capability checks. Low-level `RenderDevice` calls reject unsupported commands/methods explicitly. High-level `set_render_settings()` validates setting values; backend support and fallback selection are resolved by `RenderDevice` at render time with diagnostics. Applications can query supported MSAA counts through the existing non-virtual API; virtual `capabilities()` remains deferred to 3.0.
- Default settings use only the common baseline (`sorted` transparency; optional effects disabled). There is no implicit effect downgrade hidden inside a frame.

#### Presentation and native interop

- The normal public `RenderDevice` is a headless render/capture/selection interface. `present()` moves behind a private presentation interface used by windowed hosts.
- No generic public `Surface`, `PresentationOptions`, or `create_surface_device()` is introduced. Owning a toolkit window, swapchain, and surface lifetime is the host adapter's responsibility.
- `Viewer`, `OffScreen`, and `MultiViewer` remain the supported portable presentation APIs. `OffScreen` never exposes window-only controls publicly.
- Native Vulkan surface integration remains an explicit advanced interop boundary under `easy3d::graphics::backends::vulkan`. Its `VulkanSurface` derives without adding data from the deprecated `easy3d::graphics::VulkanSurface` compatibility base. Keeping the original base as a real type preserves the legacy factory's C++ mangled symbol; a typedef would not. The old factory forwards to the interop factory. The native window remains caller-owned and must outlive the device; the device owns the created Vulkan surface.

#### Ownership and lifetime

- A rendering host owns its `DrawableRenderer`. The bridge may retain immutable CPU geometry/image snapshots keyed by drawable revision, but it does not own models, drawables, cameras, devices, windows, or native GPU resources.
- Scene membership and ownership policy stay with the host's existing `shared_ptr` storage. A retained `DrawSource` snapshot keeps its model/drawable alive through its `shared_ptr` members, but adding/removing scene membership remains a host operation. This is distinct from the bridge's weak scene-object cache.
- The unification work does not change `add_model()`/`add_drawable()` return types. Shared-ownership overloads remain preferred; raw-pointer ownership-transfer overloads are deprecated and removed only in coordination with the separate raw-pointer modernization. Raw pointers used by `delete_model()`, `delete_drawable()`, and other observer APIs are not ownership transfers.
- `device()` returns a borrowed `const RenderDevice&` valid for the host's lifetime. Clients must not destroy it or use it after host destruction.

#### Capture and readback defaults

- Keep three C++ host overloads: `capture()`, `capture(unsigned samples)`, and `capture(unsigned samples, FrameReadback)`. The sample-count overload must not also have a default argument, because that would be ambiguous with `capture()`.
- `capture()` uses the host's effective sample count and full readback. `capture(1)` explicitly requests single-sample color with full readback. `capture(samples, FrameReadback::color)` leaves `depth` and `ids` empty.
- `RenderOptions::readback` defaults to `FrameReadback::color_depth_ids`.
- Successful capture always sets positive `width`/`height` and exactly `width * height * 4` RGBA bytes. Full readback also sets exactly `width * height` depth values and IDs. Color and IDs are top-row-first; depth is in `[0,1]` with clear depth `1`; background IDs are `{UINT32_MAX, UINT32_MAX}`.
- Captured color includes submitted scene and overlay commands. Presented frames and file snapshots additionally include supported presentation effects; raw in-memory `capture()` intentionally remains a scene-command readback without those effects. Depth and picking IDs describe scene geometry and exclude overlays. OpenGL may report `UINT32_MAX` for unavailable primitive provenance, as documented today.
- A default-constructed `FrameImage` is an empty value. A successful capture never returns that empty sentinel; invalid sizes/readback requests throw.

#### Host visibility

| Operation | `Viewer` | `OffScreen` | `MultiViewer` |
|---|---|---|---|
| Scene, camera, picking, overlays, clipping, render settings, capture, `device()` | Public | Public | Public |
| `render(file, ...)` | n/a | Public | n/a |
| Grid assignment/layout/divisions | n/a | n/a | Public |
| `run`, window show/hide/resize/position/title | Public | Protected | Public |
| Native `window()` handle | Public but deprecated in 2.8; protected/private in 3.0 | n/a | n/a |
| `init`, `cleanup`, `pre_draw`, `post_draw`, `draw`, `draw_commands` | Protected customization hooks | Protected customization hooks | Protected customization hooks |

`cleanup()` remains protected and non-virtual in 2.8; adding a virtual cleanup hook is deferred to 3.0. `ViewerBase::window()` and `Viewer::window()` remain public but deprecated in 2.8 and become protected/private in 3.0, giving external native-window callers a deprecation period.

#### Image conventions

- `graphics::Image` and `FrameImage` are canonical top-row-first CPU images. Both adapters map those canonical rows into their backend texture-coordinate convention without changing the portable image data; screen-space `(0,0)` is the top-left corner. Scene vertex texture coordinates retain the conventional bottom-left V origin, while screen-space overlay UVs use the top-left origin. No global image-loader flip state is introduced, and native texture IDs never become part of the portable image contract.
- Legacy `Texture::PixelData` remains bottom-row-first during compatibility; `DrawableRenderer` performs the explicit row conversion at that boundary.

### 8.7 Migration and deprecation policy

#### Release policy

- The 2.8 compatibility baseline is pre-unification `main`, including the intentional 2.7 API renames and ownership moves already present there. APIs removed or renamed by those documented 2.7 changes are not restored as compatibility aliases. One later simplification supersedes the original host-constructor compatibility plan: the temporary `VulkanOptions`/`OpenGLOptions` overloads and `DeviceOptionsTag` dispatch were removed on 2026-11-27 in favor of a single `easy3d::DeviceOptions` parameter on each host. This is an intentional API break for callers that explicitly named the old option types: those type names and the old constructor symbols are not retained, although the host constructors retain their parameter order and defaults.
- **2.8.0 — compatibility release:** add portable replacements and deprecate old APIs. Preserve existing symbols, public class layouts, and virtual tables. New convenience overloads may be non-virtual forwarding wrappers.
- **3.0.0 — removal release:** remove deprecated declarations, definitions, Python bindings, forwarding headers, and install/export entries; make native handles protected/private; land virtual-interface changes that cannot preserve the 2.x vtable.
- A deprecated API must survive at least one complete 2.8.x release series. Removal is allowed in 3.0.0 only after every gate in section 8.8 passes.
- The raw-pointer ownership migration is coordinated with, but not silently folded into, this API work. Its ownership-transfer overloads target 3.0.0 only if the separate modernization is complete; otherwise they remain deprecated until the next coordinated major release.

Because Easy3D requires C++11, deprecations use one project macro rather than C++14 `[[deprecated]]` syntax. Define `EASY3D_DEPRECATED(message)` once in `easy3d/core/types.h`: use `[[deprecated(message)]]` for C++14+, `__declspec(deprecated(message))` on MSVC, `__attribute__((deprecated(message)))` on GCC/Clang, and an empty definition only on compilers with no supported diagnostic.

#### Exact C++ migrations

| Old signature/API | Replacement and compatibility wrapper | Deprecate | Remove |
|---|---|---|---|
| `Viewer(title, WindowOptions, VulkanOptions/OpenGLOptions)` | `Viewer(title, WindowOptions, easy3d::DeviceOptions)` with the same parameter order and defaults. The old backend-option overloads and transitional template were removed on 2026-11-27 rather than carried to 3.0. | Simplified API | Completed in the unification branch |
| `OffScreen(width, height, VulkanOptions/OpenGLOptions)` | `OffScreen(width, height, easy3d::DeviceOptions)`; no separate compatibility overload. | Simplified API | Completed in the unification branch |
| `MultiViewer(rows, cols, title, WindowOptions, VulkanOptions/OpenGLOptions)` | `MultiViewer(rows, cols, title, WindowOptions, easy3d::DeviceOptions)`; no separate compatibility overload. | Simplified API | Completed in the unification branch |
| `RenderDevice::render(width, height, commands, background, samples, transparency, peel_layers, readback)` | `render(width, height, commands, RenderOptions)`; the 2.8 non-virtual options overload is an exact-type constrained template with no default argument and forwards through a private out-of-line bridge to the existing positional virtual, preserving legacy `{}` calls. In 3.0 the options overload becomes primary virtual and the positional overload is removed. | 2.8.0 | Positional overload in 3.0.0 |
| `RenderDevice::present(...)` | No normal public replacement. Windowed hosts present through their private adapter; headless callers use `render()`. | 2.8.0 | 3.0.0 |
| `create_opengl_device()` / `create_vulkan_device(...)` | `create_device(DeviceOptions)`; old factories forward and retain their exact parameter order. | 2.8.0 | 3.0.0 |
| `graphics::VulkanSurface` / `graphics::create_vulkan_window_device(...)` | Advanced native interop moves to `graphics::backends::vulkan`; the original surface remains an ABI-preserving compatibility base and the old factory forwards to the interop factory. | 2.8.0 | 3.0.0 |
| Public `OpenGLDevice` | Use `RenderDevice` returned by `create_device()`. | 2.8.0 | Public class/header removed in 3.0.0 |
| `Drawable::draw(const Camera*)`, `gl_draw()` | `DrawableRenderer::command(...)` or host `draw_commands()`. | 2.8.0 | 3.0.0 |
| `Drawable::vao()`, `vertex_buffer()`, `color_buffer()`, `normal_buffer()`, `texcoord_buffer()`, `element_buffer()` | `Drawable::buffer_data() const` and `DrawableRenderer`; no native-handle replacement. | 2.8.0 | 3.0.0 |
| `const BufferData& Drawable::buffer_data()` non-const member | Add `const BufferData& buffer_data() const`; deprecate the non-const overload and remove it in 3.0. | 2.8.0 | 3.0.0 |
| `Texture::id()`, `bind()`, `release()` | `Texture::pixels()` / `graphics::Image` plus `DrawCommand::texture`. | 2.8.0 | 3.0.0 |
| `State::set_ao_texture(unsigned int)` | `RenderSettings::ambient_occlusion` through `set_render_settings()`. | 2.8.0 | 3.0.0 |
| `Shadow`, `SoftShadow`, `AmbientOcclusion`, `EyeDomeLighting`, `AverageColorBlending`, `DualDepthPeeling`, `PerPixelLinkedListsOIT`, and their backend-facing draw/generate methods | `RenderSettings` compatibility facades; algorithms become private. | 2.8.0 | 3.0.0 |
| `ViewerBase::window()` / `Viewer::window()` public native handle | Protected native interop only; normal clients use portable window methods. | 2.8.0 | Visibility reduced in 3.0.0 |
| Raw-pointer ownership-transfer `add_model(Model*, ...)` / `add_drawable(Drawable*)` | Existing `shared_ptr` overloads; return types remain unchanged in this migration. | 2.8.0 | 3.0.0, coordinated with raw-pointer modernization |
| Public `FramebufferObject`, `ShaderProgram`, `ShaderManager`, `VertexArrayObject`, `opengl::read_color`/`read_depth` (including `_ms` and snapshot variants), `OpenGLTimer`, and `opengl::shape` helpers | `RenderDevice`, `DrawCommand`, shape command builders, and capture/diagnostics. Forwarding headers may remain deprecated in 2.8. | 2.8.0 | 3.0.0 |
| `opengl::OverlayRenderer` | `DrawCommand` overlay lists through `set_overlays()`. | 2.8.0 | 3.0.0 |

`Viewer::cleanup()` remains non-virtual in 2.8. In 3.0, a separately named virtual `on_cleanup()` may provide the symmetric subclass hook: `Viewer::~Viewer()` can call non-virtual `cleanup()`, which invokes `on_cleanup()` while the complete derived object is still alive. Simply making `cleanup()` virtual would not help destructor-time dispatch because the dynamic type has already changed. `RenderDevice::backend()` and `capabilities()` may likewise become virtual in 3.0, not in the 2.8 public vtable. Existing source-level deprecation warnings in 2.8 must not be accompanied by ABI-breaking class changes.

#### Exact Python migrations

| Current Python API | Replacement | Deprecate | Remove |
|---|---|---|---|
| Raw drawable buffer getters | `buffer_data()` exposing vertices/normals/colors/texcoords/indices/source faces/revision. | 2.8.0 | 3.0.0 |
| `draw(camera)`, `gl_draw()`, `vao()` | Add the drawable to a host or use bound `DrawableRenderer`/command APIs; no native equivalent. | 2.8.0 | 3.0.0 |
| `State.set_ao_texture(id)` | `RenderSettings` plus host `set_render_settings()`. | 2.8.0 | 3.0.0 |
| Texture IDs/native texture operations, if exposed by downstream bindings | `graphics.Image` / CPU texture data; Easy3D currently exposes texture loading only through `State.set_texture(file)`. | 2.8.0 | 3.0.0 |
| Missing common host operations | Add `draw_frame`, `show`, `hide`, `device`, clipping, overlays, render settings, capture, and `delete_model`/`delete_drawable` consistently. | n/a | n/a |
| Missing `OffScreen.capture()` / `FrameImage` | Add capture and NumPy-compatible read-only `rgba`, `depth`, and `ids` arrays. | n/a | n/a |
| Flattened constructor keywords | Keep them and add optional `DeviceOptions`; do not break existing positional calls. | n/a | n/a |

Python deprecations use `PyErr_WarnEx(PyExc_DeprecationWarning, message, 1)` while keeping the old callable binding. The warning names the exact replacement. Native-handle bindings are deleted in the same 3.0.0 change that removes their C++ declarations.

`FrameImage` Python arrays have these shapes and dtypes: `rgba` `(height, width, 4)` `uint8`, `depth` `(height, width)` `float32`, and `ids` `(height, width, 2)` `uint32`. Arrays are copies or read-only views with host-independent lifetime; they never expose internal vectors as mutable writable buffers.

### 8.8 Removal gates

A compatibility wrapper may be removed in 3.0.0 only when all of the following are true:

1. Its portable replacement is implemented and documented for both backends.
2. Backend-parity tests cover rendering, updates, picking, capture, overlays, effects, and fallback behavior relevant to that API.
3. All in-tree C++ callers, Python bindings, tutorials, examples, and tests use the replacement or an intentional documented interop boundary.
4. Public-header and installed-package consumers compile without backend macros or native includes.
5. The 2.8 migration guide and release notes name the old signature, exact replacement, behavioral differences, and 3.0 removal.
6. At least one complete 2.8.x deprecation release has shipped.
7. Declaration, definition, binding, forwarding header, CMake source/install entry, and test dependency are removed together.

### 8.9 Milestone 0 validation

Validation date: 2026-11-26.

- Added `easy3d_api_compile_smoke_test` under `tests/viewer/api_smoke/`, covering the current public C++ API for interactive `Viewer`, `OffScreen`, `MultiViewer`, custom drawables plus `DrawableRenderer`, and a mock headless `RenderDevice`. The mock avoids requiring a real GPU and keeps the source backend-neutral.
- The smoke functions are compiled and linked but deliberately not called. CTest verifies the executable starts and exits; it does not claim runtime rendering, installed-package, Python, or future 3.0 API validation.
- Vulkan: built `easy3d_api_compile_smoke_test` in `build/` with `Easy3D_ENABLE_VULKAN=ON`; `viewer.apismoke` passed.
- OpenGL: built the same target in `build-api-smoke-opengl/` with `Easy3D_ENABLE_VULKAN=OFF`; `viewer.apismoke` passed.
- Checked all local Markdown file links in this report and `changes/ToDo.md`; every target exists.
- The Python audit found a pre-existing stale `update_highlight_mask(strengths, dynamic)` binding after the C++ API became one-argument. Python compilation remains a Milestone 7 task; no Python parity is claimed here.

### 8.10 Milestone 1 implementation and validation

Validation date: 2026-11-26.

#### User-facing API

Applications can now create the build-selected headless device, group frame settings, and assemble common drawing commands without backend macros or native includes:

```cpp
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/shape.h>

easy3d::graphics::DeviceOptions device_options;
auto device = easy3d::graphics::create_device(device_options);

easy3d::graphics::RenderOptions render_options;
render_options.background = easy3d::vec4(0, 0, 0, 1);
render_options.samples = 4;

std::vector<easy3d::graphics::Vertex> vertices(3);
vertices[0].position = easy3d::vec3(-1, -1, 0);
vertices[1].position = easy3d::vec3(1, -1, 0);
vertices[2].position = easy3d::vec3(0, 1, 0);
const auto triangle = easy3d::graphics::shape::draw_triangles(vertices);
const auto frame = device->render(64, 64, {triangle}, render_options);
```

`Backend`, `DeviceOptions`, `RenderOptions`, and `DeviceCapabilities` join the existing portable topology, style, transparency, readback, geometry, image, command, frame, and selection types. `draw_triangles()` and `draw_quad_textured()` complement the existing filled/wire quad, screen-line, 3D-line, box, depth, and effect factories. Commands remain ordinary values with explicit public fields.

#### Compatibility and implementation boundaries

- The existing positional `RenderDevice::render()` virtual remains unchanged and deprecated. The exact-type `RenderOptions` template is non-virtual, has no default argument, and routes through a private out-of-line compatibility bridge so inline grouped calls do not reference the deprecated virtual. Legacy three-argument and empty-braces calls retain their overload resolution and behavior.
- `create_device(const DeviceOptions&)` is the normal factory. The OpenGL and Vulkan factories retain their exact signatures as deprecated forwarding wrappers. OpenGL rejects explicitly requested validation, profiling, and physical-device selection instead of silently ignoring them.
- Device name, driver version, sample-count queries, validation diagnostics, dropped-fragment counts, and point-upload counts are available through the existing non-virtual interface. The `DeviceCapabilities` value type lands here; virtual `backend()`, `capabilities()`, and `cleanup()` remain deferred to the coordinated 3.0 vtable change.
- Native Vulkan window integration uses `graphics::backends::vulkan::VulkanSurface`. Its data-free derivation from the original compatibility base preserves the old factory's mangled symbol while giving new interop callers the explicit namespace.
- `graphics::Image` stays top-row-first. Each adapter explicitly maps those rows into its texture-coordinate convention, and screen-space overlay UV `(0,0)` remains the top-left. The Vulkan image-overlay shader compensates for its texture-coordinate origin, while scene-depth/effect targets retain their existing top-left sampling. The RealCamera tutorial now uses the shared textured-quad factory rather than duplicating geometry and projection assembly.
- General `Viewer` rendering is intentionally not routed through the new device contract yet; that integration follows after this contract is stable.

#### Validation coverage

- `devicecontract` is a CPU-only test of value defaults, exact options forwarding, legacy `{}` resolution, and command factories. It does not claim adapter execution.
- `viewer.apismoke` compiles and links the same backend-neutral low-level API in both builds.
- `vulkan.interopsmoke` checks the native surface inheritance and both interop factory signatures, and links the legacy Vulkan window factory symbol.
- `opengl.renderdevice` executes the OpenGL factory and checks option rejection, frame readback, picking, textures, overlays, transparency, and resize behavior.
- `vulkan.picking`, `vulkan.render`, `vulkan.depthoverlay`, and `vulkan.depthtexture` execute the Vulkan adapter, including validation-enabled picking and overlay paths.
- `opengl.realcamera` and `vulkan.realcamera` compare rendered inset pixels against the source image, covering orientation, filtering, resizing, and capture behavior on each backend.
- Vulkan: all eight targeted tests passed. OpenGL: all four targeted tests passed. The portable CPU regression suites passed 5/5 in each build, grouped-options calls passed strict deprecation-warning checks in C++11 and C++14 through `RenderDevice&`, and `git diff --check` passed. The intentionally deprecated concrete `OpenGLDevice` API produces the expected diagnostics.

### 8.11 Milestone 2 implementation and validation

Validation date: 2026-11-27.

Status: complete. The final all-target OpenGL and Vulkan builds succeeded, both complete CTest suites passed serially, and corner-axis/EDL behavior plus MultiViewer live/capture divisions were verified on the macOS HiDPI validation host.

#### User-facing API

Applications upload ordinary drawable data, inspect the same CPU snapshot in either build, and render through portable commands:

```cpp
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_renderer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/render_device.h>

auto points = std::make_shared<easy3d::PointsDrawable>("samples");
points->update_vertex_buffer({easy3d::vec3(0, 0, 0), easy3d::vec3(1, 0, 0)});
points->set_impostor_type(easy3d::PointsDrawable::SPHERE);

const auto &data = points->buffer_data();
easy3d::Camera camera;
easy3d::graphics::DrawableRenderer renderer;
auto command = renderer.command(points, 0, camera, {});
auto device = easy3d::graphics::create_device();
auto frame = device->render(256, 256, {command});
```

`Drawable::BufferData` contains vertices, optional normals/colors/texture coordinates, optional indices, source-face provenance, and a monotonically increasing revision. `buffer_data() const` realizes pending model/callback updates lazily and returns the authoritative CPU geometry in both builds. Explicit vertex uploads mark the geometry ready; a later `update()` deliberately restarts model/callback realization. Explicitly staged colors, normals, texture coordinates, and indices survive that realization, while attributes generated by the model/callback do not become permanent client overrides. Geometry, provenance, and highlight-mask changes advance the revision and invalidate renderer caches. This accessor is not strictly observational: lazy realization mutates the drawable internally, so concurrent access still requires external synchronization.

#### Geometry, cache, and styling behavior

- Both adapters consume `graphics::Geometry`, `graphics::PointData`, and `graphics::Image` values assembled by `DrawableRenderer`. The renderer owns only CPU-side caches keyed by drawable identity and revision; devices, windows, cameras, models, and native GPU resources remain outside its ownership boundary.
- Vertex, normal, color, texture-coordinate, and index uploads, resizing, empty uploads, element-buffer disabling, detachment, clearing, callbacks, and repeated updates share the same CPU realization path. OpenGL uploads remain deferred until native compatibility drawing.
- Point and line commands carry plain/sphere/surfel and plain/cylinder/cone styles, dimensions, color, normals, texture coordinates, primitive IDs, highlight masks, and transforms. Smooth triangle commands preserve portable indices; flat triangle realization expands primitives and derives face normals. Screen-space lines are cached portable ribbon geometry.
- Plain point/line dimensions are logical drawable sizes. Hosts scale them to framebuffer pixels when their camera uses logical screen coordinates; world/view-space impostor radii are not pixel-scaled. `FrameImage` and command viewports use top-left origins, while OpenGL conversion stays inside the adapter.
- `DrawCommand::set_depth_range()`/`depth_range()` map normalized scene depth into a window-space interval. Both adapters validate and apply the same ordered interval in `[0,1]`; a cross-backend test checks actual remapping as well as rejection of reversed, out-of-range, and non-finite values. Endpoints use 15-bit normalized precision stored in existing command padding.
- Legacy `TrianglesDrawable::opacity()` remains effective only in a separate transparency pass, preserving legacy behavior. Ordinary opaque commands retain the drawable's normal alpha rather than implicitly enabling transparency.
- `DrawableRenderer` converts legacy `Texture::PixelData` exactly once from bottom-row-first to top-row-first `graphics::Image`. OpenGL and Vulkan then map portable rows into their respective texture-coordinate conventions; the Vulkan shader compensates only for ordinary image overlays, so portable overlay UVs retain their top-left origin while scene-depth, EDL, AO, and shadow textures continue sampling their native top-left targets without inversion.

#### Application integration and compatibility boundary

- `Viewer`, `MultiViewer`, offscreen hosts, tutorials, and tests assemble portable commands for their ordinary rendering paths. Capture, overlays, point/line sizing, and corner axes use the shared device contract.
- Viewer corner axes use camera-space material lighting and occupy a DPI-scaled, 100-logical-pixel square in the bottom-left corner. This widget placement does not change the top-left origin used by logical coordinates and portable command viewports.
- The supported Mapple renders its ordinary drawables and Viewer overlays through shared command assembly and presents them through `RenderDevice`. Its corner axes use the same camera-space orientation, normal transform, and lighting as Viewer axes. The retired Mapple-Qt is OpenGL-only, is not built or validated, and retains native calls only as a reference compatibility host.
- Mapple's custom drawables, custom `draw()` implementations, AO-texture handoff, and command-translation failures retain centralized native fallback where the shared contract cannot represent them. Flag-only AO drawables without a texture remain portable. `WalkThrough` exposes separate portable and native paths for compatibility hosts. Legacy effect facades remain application-owned compatibility paths after the settings-driven effect migration.
- Native `Drawable::draw()`, `gl_draw()`, `vao()`, raw buffer handle getters, drawable-taking sphere/box helpers, immediate keyframe camera/path drawing, and the concrete public `OpenGLDevice` class are deprecated in 2.8. Python retains the existing native rendering/handle bindings as deprecation-warning compatibility wrappers and also exposes `BufferData`/`buffer_data()`. Existing C++ declarations, virtual signatures, and bindings needed for source compatibility remain until the coordinated 3.0 removal.
- Remaining native immediate drawing is confined to compatibility/effect/picking fallbacks and native shape helpers. It is not a second public rendering path for new portable clients. Public removal remains gated by the release and removal policy in section 8.8.

#### Validation coverage

- `drawablecontract` runs unchanged in both builds and covers CPU realization, stable snapshot identity, standalone client-geometry preservation across `update()`, revision invalidation (including highlight-only changes), provenance, first-use provenance/highlight realization, dynamic resizing, element-buffer disabling, clearing, detachment, callback ordering, explicit-upload precedence, cache reuse, transactional callback rollback, non-mutating public-updater rejection at revision exhaustion, and point/line/triangle/screen-line command state. Primitive IDs remain ordinal through smooth, flat-shaded, and highlighted triangle expansion, including transparent OpenGL triangle splitting.
- `render`, `renderdevice`, device contracts, multiview, snapshot effects, plain-line parity, picking/query paths, selection, screen lines, independent-depth HUDs, and offscreen queries exercise both adapters. `opengl.renderdevice` supplies additional adapter-specific readback, query, presentation, and sorting checks. `draw_commands()` retains its pre-existing virtual extension point on both backends; exact native fallback is limited to unsupported query states and custom draw calls.
- `opengl.render` and `vulkan.render` check color/depth/ID readback, depth ordering, depth-range remapping and validation, image orientation, MSAA IDs, transparency, textures, overlays, and point/line styles available to the shared suite. OpenGL analytic point/line shaders explicitly remap their written fragment depth through `DrawCommand::depth_range()`. Vulkan depth-texture, EDL, AO, and shadow regressions verify that image-overlay compensation does not invert native scene-depth/effect sampling. Backend-specific suites cover camera guides/path, corner axes, and OpenGL effect fallback.
- Deterministic offscreen comparisons against the actual pre-unification baseline `727e163c` covered 20 OpenGL and 24 Vulkan scenes, including native drawable updates/refresh, textured native drawables, MSAA, overlays, transparency, and picking. Depth and object/primitive IDs matched exactly in every scene. RGBA matched exactly in 19/20 OpenGL scenes and 21/24 Vulkan scenes. The sole OpenGL difference was 115 overlay-MSAA pixels with a maximum channel delta of one and no delta greater than one. The three Vulkan differences were direct portable `graphics::Image` scenes intentionally corrected to honor the frozen top-row-first image contract; legacy native textured drawables remained byte-identical on both backends. The comparison also drove restoration of OpenGL's baseline ordinary draw order, polygon-offset factors, and native thin-line rasterization.
- `opengl.corneraxes` and `vulkan.corneraxes` check bright front faces, bottom-left bounds, resize/DPI behavior, MSAA coverage, overlays, visibility, orientation-only transforms, and preserved scene depth/picking IDs. A nearer full-view occluder verifies that the axes retain independent HUD depth instead of being overwritten by the shared 3D-overlay defaults.
- MultiViewer divisions are generated from integer framebuffer boundaries, not rescaled logical-window boundaries. OpenGL resets the internal target viewport and scissor before full-target overlays so a final cell viewport cannot clip portable divisions. File snapshots call the virtual `draw()` hook, preserving subclass custom drawing on both the portable and native paths; a counting subclass guards this behavior. The multiview regression checks every divider centerline pixel in portable and forced-native OpenGL snapshots and in Vulkan, and requires identical portable/native OpenGL divider masks. The forced-native fixture uploads and assigns a custom point drawable to an otherwise empty cell, then verifies its visible color, depth query, and picking source; representative native color probes ensure scene content survives as well as the black dividers. The RenderDevice regression checks that a full-width overlay covers adjacent scene viewports.
- Native OpenGL EDL uses a `GL_DEPTH_COMPONENT32F` projection target compatible with RenderDevice scene depth. Its regression rejects black compositing output as well as checking darkening, preventing an empty depth texture from producing a false pass.
- `PyEasy3D` compiled successfully, and the packaged rendering smoke test passed in both Vulkan-enabled and OpenGL-only builds. Coverage includes the deprecated `draw`, `gl_draw`, `vao`, raw buffer getters, and AO texture wrappers alongside `BufferData`/`buffer_data()` and highlight masks. The borrowed VAO binding uses a non-owning holder, preserving its historical pointer return without requiring the Vulkan-excluded native destructor. OpenGL checks positive vertex/element handles and the returned VAO ID; Vulkan checks zero native handles and the documented unavailability errors. The OpenGL `gl_draw()` wrapper is verified callable but not executed because the packaged Python API does not expose shader binding.
- The supported Mapple built successfully in both backend configurations. Separate visual smoke runs launched `resources/data/sphere.obj` and confirmed intact menus, toolbars, rendering panels, canvas, Models panel, and visible geometry in each window, titled `Mapple - OpenGL - Debug Version` and `Mapple - Vulkan - Debug Version`, respectively. Both runs reported the same 42 vertices and 80 faces. These captures validate application startup and scene presentation, not identical per-pixel styling; snapshot, effects, picking, and high-DPI interaction still require dedicated UI coverage.
- Final post-audit complete serial macOS runs passed OpenGL 125/125 in 111.69 seconds and Vulkan 154/154 in 238.82 seconds. The OpenGL rerun followed correction of a test fixture that had incorrectly applied top-left overlay UVs to a conventional scene texture; the renderer and baseline rendering were unchanged. Focused Vulkan depth-texture, EDL, AO, shadow, depth-overlay, and ordinary image-overlay regressions passed after separating image-overlay UV compensation from native scene-depth/effect sampling. An earlier Vulkan run had only the intermittent `vulkan.crosssection` fixture miss; five immediate isolated repetitions passed, followed by clean complete runs. The cause of that intermittent pick miss is not established here.
- The DPI-2 fixture uses a 602x446 framebuffer. Divider coverage is now checked along every vertical and horizontal centerline, with portable and forced-native OpenGL masks compared pixel by pixel instead of relying on sparse probes or a hard-coded dark-pixel count.
- Strict C++11 and C++14 probes passed with `-Werror=deprecated-declarations`: grouped `RenderOptions` calls through `RenderDevice&` are warning-free in both builds, while intentional calls to deprecated positional/concrete APIs and the deprecated `OpenGLDevice` class produce the expected diagnostics. No warning-suppression pragmas were added.
- ABI/source probes passed in both builds. Complete Clang record-layout comparisons found no removed or moved pre-existing member and no size change in `Drawable`, `Viewer`, `MultiViewer`, or `DrawCommand`. The const and non-const `Drawable::buffer_data()` symbols remain exported, C++14 aggregate initialization and protected mutable compatibility members still compile, and packed depth-range probes preserve the 616-byte command layout.
- `git diff --check` passed.

### 8.12 Milestone 3 implementation and validation

Validation date: 2026-11-27.

Status: complete. The final all-target builds succeeded, the new CPU and interactive regressions passed, and both complete serial CTest suites passed on the final tree. The analytic-clipping deviation listed below was subsequently resolved by section 8.15, and section 8.16 resolved deviations 2–4 by moving settings-driven effect composition into `graphics::RenderDevice`. The capability fallbacks and application-owned legacy compatibility paths that remain are documented below.

#### Declarative effect settings

`easy3d/renderer/render_settings.h` adds the plain value types `ShadowSettings`, `AmbientOcclusionSettings`, `TransparencySettings`, and `RenderSettings`. Defaults match the existing native effects: a 1024 shadow map at 50 scene radii, darkness 0.6, hard shadows, AO radius ratio 0.045 with bias 0.005 and intensity 1, sorted transparency with four layers, disabled optional effects, and no explicit clipping planes.

`validate_render_settings(settings)` rejects non-finite values, a shadow map size that is not a positive power of two or exceeds the host effect integer range, light distances below 1.1, values outside `[0,1]`, non-positive AO radius or negative bias, unknown transparency methods or zero layers, and invalid clipping halfspaces. The capability overload additionally rejects effects and transparency methods that a device does not advertise. Clipping planes are limited to two finite halfspaces with nonzero normals and are normalized by the host setters, so `clipping_planes()` exposes the same plane equations that command assembly uses.

`Viewer` now provides `render_settings()` and `set_render_settings()` in both builds without adding data members or virtual functions. `set_clipping_planes()` updates the `clipping_planes` field instead of a separate backend-only configuration. `MultiViewer` inherits the API, and `OffScreen` re-exposes it through its protected compatibility surface. The default settings reproduce the pre-M3 ordinary frame; live frames and file snapshots apply presentation effects, while in-memory `capture()` remains the raw scene-command readback described in Milestone 4.

#### Backend integration

- Vulkan injects declarative shadows, AO, and EDL as command overlays through the existing shape factories. `Shadow`/`SoftShadow` are used only to compute light-space frame data; legacy `begin()`/`end()` registrations still supply the older compatibility overlay. Transparency method and layer count flow through `RenderDevice::render`/`present`, and one-time warnings report linked-list fragment drops.
- OpenGL reuses the native shadow, AO, EDL, average-color, depth-peeling, and linked-list algorithms from a per-viewer sidecar cache. The sidecar stores only settings and effect objects, preserving the 2.8 `Viewer` layout. Plain sorted transparency uses the OpenGL `RenderDevice`; unsupported drawables fall back to native depth peeling with a warning. Per-pixel linked lists fall back to peeling below OpenGL 4.3.
- Shared command assembly marks scene triangles transparent and applies their opacity when the settings transparency pass is enabled. The OpenGL adapter applies the shared CPU polygon/line/point clip for non-analytic commands and, as completed in section 8.15, shader-side actual-surface clipping for analytic point/line impostors.
- `Shadow` and `SoftShadow` now guard the optional virtual background drawable, so enabling the background without a rendered background plane no longer dereferences a null drawable.

#### Deviations and limitations

1. `TransparencySettings` adds an explicit `enabled` switch. Without it, configuring any unrelated effect would activate a transparency pass over every triangle drawable because `TrianglesDrawable::opacity()` defaults to 0.6 while ordinary opaque rendering has always ignored that value. The switch preserves the default frame and keeps the method/layers fields portable.
2. Resolved by section 8.16: settings-driven shadow/AO/EDL and all four transparency methods now compose through `graphics::RenderDevice` on both backends. The legacy native facades remain application-owned compatibility paths and are not silently treated as settings parity.
3. Resolved by section 8.16: OpenGL composes AO and EDL in the same stream order as Vulkan. The old EDL-precedence warning is retained only for the genuine native fallback path.
4. Resolved by section 8.16 for settings-driven frames: `graphics::effective_clipping_planes()` resolves one plane set before command assembly, and the OpenGL device applies it to scene geometry, transparent/OIT passes, shadow casters, and AO/EDL depth inputs. A genuine native fallback for unsupported drawables or custom draw calls still applies only the process-wide `ClippingPlane` and warns; the clipping-plane guide remains an unclipped, depth-tested overlay.
5. OpenGL analytic sphere/surfel and cylinder/cone clipping was initially a deviation because the proxy shaders could not use the CPU clip. Section 8.15 resolved it with fragment-side actual-surface tests; plain points and lines were already supported. Section 8.16 resolved the remaining effect-composition split described in deviation 4.
6. Below OpenGL 4.3, linked-list transparency falls back to depth peeling. Sorted transparency falls back to native peeling for custom drawables, native AO textures, or adapter rejection. Both fallbacks are reported.
7. In-memory `capture()` remains a raw scene-command frame in both builds. Settings shadows, AO, EDL, and capability-supported transparency methods are applied to live frames and file snapshots; command-level transparent styling and clipping remain visible in capture. Section 8.16 records the presentation-parity validation; the raw in-memory semantics are unchanged.
8. Legacy `Shadow`, `SoftShadow`, `AmbientOcclusion`, `EyeDomeLighting`, and transparency facades remain application-owned compatibility paths and emit precedence warnings where they conflict with settings. Their control-to-settings translation is deferred with the Python and tutorial migration.
9. Vulkan reports dropped linked-list fragments through the device counter and a one-time warning. Section 8.16 also reports drops from the OpenGL `RenderDevice` linked-list path; the application-owned native facade still does not expose an equivalent counter.

#### Validation coverage

- `render_settings_test` (`opengl.rendersettings` and `vulkan.rendersettings`) is a CPU-only contract test for defaults, copy independence, boundary values, every invalid range and representation, clipping normalization, and capability rejection.
- `render_settings_test` under `tests/viewer` (`opengl.rendersettingseffects` and `vulkan.rendersettingseffects`) renders a neutral room scene and compares snapshots for default, hard shadow, soft shadow, scene shadow, AO, EDL, and all four transparency methods. Hard and soft shadows differ, virtual backgrounds are exercised, default settings reproduce the plain frame, and raw in-memory capture excludes presentation-only shadow effects. The clipping fixture verifies normalized plane exposure, triangle/plain-point/plain-line capture, and point picking on both backends.
- The viewer fixture exercises triangle, plain-point, and plain-line clipping through command assembly on both backends. The shared `vulkan.clipping` oracle covers halfspaces, slabs, interpolation, source IDs, whole-primitive policy, analytic surfaces, MSAA, world transforms, and face picking; section 8.15 ports that oracle to `opengl.clipping` and adds the host-query cases on both backends.
- The existing default-rendering, drawable, effect, picking, snapshot, and MultiViewer suites were rerun unchanged; default settings take the legacy/portable path and do not alter ordinary scene output.
- The default, settings-clipping, and sorted-transparency snapshots are byte-identical between the OpenGL and Vulkan runs. Native hard/soft shadow, AO, EDL, average-color, peeling, and linked-list imagery use each backend's established algorithm and are validated by their difference from the plain frame, not by cross-backend pixel equality.
- Final serial macOS results on the Milestone 3 tree: Vulkan 156/156 in 242.22 seconds and OpenGL 127/127 in 136.70 seconds, both with zero failures. The new settings tests passed on both backends, and targeted reruns preceded each complete suite. The final post-follow-up counts are recorded in section 8.16.
- No public class layout or virtual-table entry changed. The new API consists of value types, free validation functions, and non-virtual `Viewer` methods; the OpenGL host stores settings and effect caches privately. `git diff --check` passed.

### 8.13 Milestone 4 implementation and validation

Validation date: 2026-11-27.

Status: complete with the documented backend limitations below. The final all-target builds succeeded, the new CPU and interactive regressions passed, and both complete serial CTest suites passed on the final tree.

#### User-facing API

**`device()` available on both backends.** `Viewer::device()` returns the backend-neutral `graphics::RenderDevice&` on both OpenGL and Vulkan builds (previously Vulkan-only). The OpenGL implementation returns the headless query device created lazily by `query_device()`. OffScreen inherits it through its `using Viewer::device;` declaration, and MultiViewer inherits it from Viewer.

**`cleanup()` remains non-virtual in 2.8.** The symmetric subclass cleanup hook is deferred to 3.0 as required by the frozen vtable policy. Subclasses release their resources from their destructors while the backend is valid and then call `Viewer::cleanup()`.

**`draw_commands()` remains virtual on both backends.** It was already virtual on `main`; Milestone 4 preserves that existing extension point rather than adding a vtable entry.

**One `easy3d::DeviceOptions` constructor for all three hosts.** Viewer, OffScreen, and MultiViewer each take `easy3d::DeviceOptions` directly, with no constrained template, dispatch tag, or deprecated backend-option overload. The type has the Vulkan fields in Vulkan builds and the OpenGL context fields in OpenGL builds, so constructor declarations contain no backend conditional. Existing calls that rely on default options, such as `Viewer("title")`, `Viewer("title", {})`, and `Viewer("title", {}, {})`, remain unchanged. Callers that explicitly named `VulkanOptions` or `OpenGLOptions` must use `DeviceOptions` and the configured backend's field names.

**Host options and low-level device options are distinct.** `easy3d::DeviceOptions` configures Viewer/OffScreen/MultiViewer construction. `graphics::DeviceOptions` remains the backend-neutral value accepted by `graphics::create_device()` and native interop factories. Applications that configure both must construct each type explicitly rather than assuming an implicit conversion.

#### Overlay and capture unification

Overlay submission through `set_overlays(std::vector<graphics::DrawCommand>, bool foreground)` is consistent across `Viewer`, `OffScreen`, and `MultiViewer` — already public on all three hosts before this milestone. Clipping planes via `set_clipping_planes()`/`clipping_planes()` and render settings via `set_render_settings()`/`render_settings()` are likewise consistent across all three hosts.

In-memory `capture()` (with overloads `capture()`, `capture(unsigned samples)`, `capture(unsigned samples, FrameReadback)`) is public on all three hosts and routes through the shared `RenderDevice` contract on both backends. `capture()` uses the host's effective sample count and full readback; `capture(1)` explicitly requests single-sample color; `capture(samples, FrameReadback::color)` skips depth/ID readback. `RenderOptions::readback` defaults to `FrameReadback::color_depth_ids`.

Viewer's direct capture FBO path on OpenGL has been removed: file snapshots and in-memory captures now go through `RenderDevice::render()` via the shared command assembly path. Presented frames and file snapshots use that path with effects, overlays, HUD, and custom drawing; raw in-memory `capture()` intentionally excludes presentation effects. Section 8.16 records the completed parity validation for effect-bearing presented captures.

#### MultiViewer parity

`MultiViewer` inherits `device()`, `draw_commands()`, overlays, clipping, render settings, and capture from Viewer without change. Its grid-specific API (`assign`, `set_layout`, `rows`, `columns`, `set_division_visible`, `division_visible`) remains intact. The virtual `draw_commands()` override assembles every assigned cell's scene commands followed by the grid-division overlay on both backends.

The existing `tests/viewer/multi_viewer_test.cpp` (`opengl.multiview` / `vulkan.multiview`) covers grid-aware parity: assignment of models and drawables to cells, layout changes, division lines and cell rects, coordinate conversion (`world_to_window`/`window_to_world`), picking (`pick`/`point_under_pixel`), snapshots (file output, transparent backgrounds, dividers), MSAA, impostor rendering across cell boundaries, resize, and cross-cell camera interaction.

#### Validation coverage

- `viewer.apismoke` compiles and links the backend-neutral low-level API for interactive Viewer, OffScreen, MultiViewer, custom drawables, and a mock headless RenderDevice in both builds (section 8.9).
- `opengl.multiview` and `vulkan.multiview` run the grid-aware parity test on both backends; both pass.
- Final post-audit complete serial macOS runs passed OpenGL 127/127 in 136.70 seconds and Vulkan 156/156 in 242.22 seconds, both with zero failures.
- No public class layout or virtual-table entry changed. `cleanup()` remains non-virtual in 2.8, and `draw_commands()` retains its pre-existing virtual declaration. `git diff --check` passed.

### 8.14 Milestone 5 implementation and validation

Validation date: 2026-11-27.

Status: complete. The final all-target builds succeeded, and both complete serial CTest suites passed on the final tree. Section 8.15 records the completed follow-up that removed the documented OpenGL analytic-clipping limitation and the two exact fallbacks that depended on it; the suite counts below remain the pre-follow-up Milestone 5 baseline.

#### User-facing API

Picking now routes through the shared `RenderDevice` contract on both backends, with the remaining exact fallbacks confined to unsupported drawable/effect states.

- `Picker` creates its headless query device through the backend-neutral `graphics::create_device()`; no backend factory remains in picker code.
- `Picker::draw_command` assembles backend-neutral draw commands, including the clipping planes reported by `ClippingPlane`, for the sampled drawable.
- `ModelPicker::pick` assembles one command per visible candidate drawable and samples the nearest object ID through the shared pass; the private color-coded native `draw` fallback is removed.
- `SurfaceMeshPicker` face and region picks sample the shared ID/depth pass; vertex picks resolve the closest source vertex of the picked face through the same sampled depth. The private framebuffer-based fallback is removed.
- `PointCloudPicker` single picks sample the shared point-data command for every supported drawable state, including clipped sphere/surfel impostors; region selection uses the shared selection request with clipping planes and model transforms folded into it. Both private framebuffer-based and native analytic-selection fallbacks are removed.

Picker-owned framebuffers (`Picker::fbo_` and `Picker::setup_framebuffer`) are retired. The `Viewer` exact pass creates its context-local framebuffer on demand for states outside the shared command contract.

#### Exact fallbacks retained

1. No `Picker` fallback remains: model, pixel, point, and region queries all use `RenderDevice`. The OpenGL analytic-impostor clipping pass (`selection/selection_pointcloud_single_point_as_sphere_sprite`) was deleted by section 8.15.
2. `Viewer::pick` and `Viewer::read_scene_depth` retain the exact native drawable pass for scenes the shared query cannot serve, including AO textures, non-opaque alpha, unsupported coloring, custom drawables, and custom `Viewer::draw()` overrides. Clipping and standard impostors no longer trigger it.

#### Validation coverage

- `gui.scenepicker` passes on both backends and covers model, pixel, point, and region picks; hidden models and drawables; out-of-bounds picks; translucent, plain, and AO point picks; indexed point picks; manipulator transforms; and clipping for plain points and analytic impostors. Section 8.15 adds a direct picker/command-sample comparison in `clipping_test`; the shared path is established structurally by the deletion of the native picker fallback, not by that behavioral comparison alone.
- Final post-edit complete serial runs passed OpenGL 127/127 in 108.83 seconds and Vulkan 156/156 in 224.96 seconds, both with zero failures, after the final picker unification edits; the focused `gui.scenepicker` regression passed before the complete suites. These are the pre-follow-up Milestone 5 numbers; the final post-follow-up counts and times are in section 8.15.
- No public class layout or virtual-table entry changed in this milestone beyond the already documented Milestone 4 entries. `git diff --check` passed.

### 8.15 Milestone 5 follow-up implementation and validation

Validation date: 2026-11-27.

Status: complete. The OpenGL adapter now serves analytic point/line clipping from the shared `RenderDevice`, and the last backend-specific picker fallback is gone. `Viewer` retains its exact pass only for drawable/effect states outside the shared command contract.

#### Implemented semantics

- `DrawCommand::clipping_planes` remain view-space halfspaces. Analytic sphere/surfel and cylinder/cone fragments test the actual front-surface hit and discard it when a halfspace rejects the hit.
- Plain points test their centers; plain lines and wide plain lines clip their centerlines through `graphics::internal::clip_draw`. This behavior did not change.
- `discard_clipped_primitives` still rejects a primitive when any control point is outside. For analytic draws this is a whole-primitive policy in addition to the per-fragment surface test.
- Surviving pixels keep their source object ID, primitive ID, and surface depth. Clipping discards fragments; it does not renumber primitives or shift depth.
- Multisampled color multiplies coverage by `edge_coverage(distance)` so the cut receives the same ramp as silhouette edges; single-sample depth/ID passes discard hard. The OpenGL loops mirror [points.frag](../resources/shaders/vulkan/points.frag) and [lines.frag](../resources/shaders/vulkan/lines.frag).
- `object_clipping_planes` remains the camera-independent form used by the geometric CPU clip and its cache. Analytic shaders consume the normalized view-space `clipping_planes`; hosts set both forms.

Scope boundary at the time of this follow-up: the native OpenGL effect paths' process-wide `ClippingPlane` behavior from Milestone 3 deviation 4 was unchanged. This work made the portable command, capture, and query paths support analytic clipping; live native shadow/AO/EDL composition with `RenderSettings::clipping_planes` remained a separately documented limitation. Section 8.16 subsequently resolved that split for settings-driven frames by moving effect composition into `graphics::RenderDevice`. Genuine native fallbacks for unsupported drawables or custom draw calls still apply only the process-wide `ClippingPlane` and warn.

#### Adapter and query changes

All adapter edits are in [easy3d/renderer/backends/opengl/render_device.cpp](../easy3d/renderer/backends/opengl/render_device.cpp):

- `validate()` no longer rejects analytic draws that carry clipping planes or the whole-primitive policy. The affine `model_view` requirement and all other validation remain.
- `prepare()` normalizes `clipping_planes` before the overlay skip and clip decision, matching Vulkan's `ClipCache::apply`: at most two finite halfspaces with nonzero normals, with tiny finite normals normalized rather than rejected.
- The embedded point and line fragment shaders declare `clip_planes[2]` and `clip_count`. After computing the view-space `hit`, they apply the Vulkan rule described above before writing depth or IDs. The point shader also discards zero resolved coverage, matching Vulkan; the line shader already did.
- `set_impostor_uniforms()` flattens up to two planes into `float[8]`, sets `clip_count`, and zeros unused entries. Programs without those uniforms ignore the `glGetUniformLocation` result of `-1`.
- `internal::clip_draw()` is unchanged. Analytic draws without the discard policy keep their proxy geometry; with it they receive control-culled proxies. Wide plain lines need no fragment-side plane test.

The OpenGL adapter uses embedded `#version 150` strings and cannot share the Vulkan GLSL files, so the two fragment loops remain separate copies, kept semantically aligned.

Query changes:

- `Viewer::supports_device_query` no longer rejects a scene merely because the process-wide `ClippingPlane` is enabled. Drawable-state gates for coloring, texture availability, opaque alpha, AO textures, and 2D guides remain.
- `Viewer::pick` and `Viewer::read_scene_depth` keep the exact native pass only for those unsupported states. Their comments no longer claim that clipping or standard impostors require it. `Viewer::capture()` inherits the same shared path for supported clipped scenes.
- `PointCloudPicker::pick` no longer has an `#ifndef EASY3D_HAS_VULKAN` analytic-and-clipped selection pass or its native includes. Every single pick uses `draw_command()` plus `render_device().sample_pixel()`; region selection still uses `world_clipping_planes()`.
- No public class layout or virtual-table entry changed. The work is private shader/uniform code, adapter validation/preparation, and fallback deletion.

#### Validation

1. [tests/viewer/clipping_test.cpp](../tests/viewer/clipping_test.cpp) now uses `graphics::DeviceOptions` and `graphics::create_device()` under an alive `OffScreen` host on both backends. Vulkan-only validation flags remain conditional; OpenGL keeps default options. [tests/viewer/CMakeLists.txt](../tests/viewer/CMakeLists.txt) registers `opengl.clipping` in the non-Vulkan branch and retains `vulkan.clipping`.
2. The shared oracle retains halfspace/slab cuts, clipped fan interpolation and colors, original source IDs, whole-primitive policy, plain point/line policy, analytic sphere/surfel/cylinder/cone surface clipping, MSAA coverage and ID/depth invariance, world transforms, and scene picking. New assertions cover more than two planes, zero/non-finite normals, adapter-side normalization of tiny finite normals, process-wide `ClippingPlane` queries through `Viewer::pick` and `read_scene_depth`, a direct `supports_device_query()`/`device_query_commands()` probe proving that clipped supported OpenGL scenes stay on the shared path, a process-wide-plane `Viewer::capture()` case, one unsupported translucent drawable that still requires the exact OpenGL `Viewer` pass, and clipped sphere/surfel `PointCloudPicker` picks compared with a direct `RenderDevice::sample_pixel()` of the same command.
3. Final all-target builds succeeded. Targeted `clipping`, capture, render-settings, and `gui.scenepicker` reruns passed OpenGL 5/5 in 8.43 seconds and Vulkan 5/5 in 8.44 seconds, including both `opengl.clipping` and `vulkan.clipping`.
4. Final complete serial macOS runs passed OpenGL **128/128 in 140.14 seconds** and Vulkan **156/156 in 210.86 seconds**, both with zero failures. One intermediate Vulkan full run had a transient `vulkan.crosssection` failure; the test passed in isolation and in the final clean suite. `git diff --check` passed.

#### Acceptance checklist

- [x] OpenGL analytic sphere/surfel and cylinder/cone draws accept clipping planes and clip their actual surface, with source IDs and depth unchanged.
- [x] Plain points, plain lines, and wide plain lines retain their existing CPU-clip behavior.
- [x] `discard_clipped_primitives` keeps whole-primitive control culling on both backends.
- [x] MSAA cuts show analytic coverage; single-sample depth/ID passes stay hard-edged and consistent with one-sample rendering.
- [x] `PointCloudPicker` has no backend-conditional native selection path.
- [x] `Viewer::supports_device_query` no longer rejects scenes only because clipping is enabled; `Viewer::pick` and `read_scene_depth` use the shared query for clipped supported scenes.
- [x] `opengl.clipping` and `vulkan.clipping` pass, along with the complete suites on both backends.
- [x] The Milestone 3 deviation, the section 8.14 fallback list, the code comments, and the ToDo entries are updated from "planned" to the measured result.

### 8.16 Milestone 5 follow-up 2: unified effect presentation and clipping

Validation date: 2026-11-27.

Status: complete. This is follow-up #2 after the completed analytic-clipping work in section 8.15. It retired Milestone 3 deviations 2–4 and the remaining scope boundary in section 8.15 by moving settings-driven shadow, AO, EDL, transparency, and clipping onto the shared `graphics::RenderDevice` command path on both backends.

#### Objective

Make `graphics::RenderDevice` the single presentation engine for every settings-driven effect on both backends. Vulkan already expresses shadow, AO, EDL, and transparency as command passes; OpenGL currently switches to native effect framebuffers and legacy classes, which cannot consume `RenderSettings::clipping_planes` and cannot compose all effect combinations. The goal is one command stream and one clipping contract, with backend differences confined to private device implementations and reported capabilities.

#### Scope

In scope:

- settings-driven shadows (hard/soft, virtual background, darkness, light range, map size), AO, EDL, all transparency methods and layer settings, clipping planes, and their composition order;
- effective clipping resolution for scene geometry, shadow casters, transparent geometry, and the depth/color inputs sampled by AO and EDL;
- the process-wide `ClippingPlane` guide and its separation from clipped scene content;
- OpenGL `RenderDevice` effect passes, shared command assembly, capability gating, capture/snapshot behavior, and `MultiViewer` cells;
- cross-backend tests and the documentation updates that retire the deviations.

Out of scope:

- legacy `Shadow`, `SoftShadow`, `AmbientOcclusion`, `EyeDomeLighting`, and transparency facades used directly by applications, except that they must keep working and must not silently claim settings parity;
- custom `Drawable` subclasses and custom `Viewer::draw()` overrides that cannot be represented by `DrawCommand`; their exact fallback remains, with an explicit warning if settings effects cannot be honored;
- Python/tutorial migration, which remains in Milestone 7, and deletion of native FBO/shader plumbing, which remains in Milestone 6.

#### Invariants

1. Effective clipping is resolved once, before command assembly: an enabled process-wide `ClippingPlane` overrides `RenderSettings::clipping_planes`; otherwise the normalized settings planes are used. This preserves the current host contract. `object_clipping_planes` remains the separate camera-independent CPU form.
2. Clipped scene commands carry the effective planes, including plain and analytic points, lines, triangles, and transparent commands. Triangle commands marked `shadow_caster` rasterize into the shadow map from the same clipped source geometry; the virtual `shadow_background` receiver and effect overlays are intentionally outside that scene-clip set.
3. AO and EDL sample a depth buffer produced from the clipped scene. A fully cut pixel must not receive AO/EDL darkening or a stale depth contribution.
4. Effect quads, HUD, annotations, and the clipping-plane guide are overlays. They are not clipped by the scene halfspaces. The guide keeps its existing scene-depth-tested overlay behavior; this work must not turn it into a clipped scene primitive.
5. Effect composition follows the Vulkan reference order. OpenGL must not silently choose a different precedence when multiple effects or transparency are enabled; unsupported combinations are either composed through a capability-supported device path or reported by a one-time warning.
6. `capture()` keeps its documented raw-command semantics. Live frames and presented/file snapshots include effects. Any new presented-capture helper must state its effect treatment explicitly.
7. No public class layout or virtual-table entry changes in this follow-up. Capability information is wired through the existing private host/device implementation without introducing the deferred virtual `capabilities()` API.

#### Work packages

1. **Single command and clipping path.** Move effective-plane resolution into one backend-neutral helper used by `draw_commands()` and both backend viewers. Keep the process-wide guide on the overlay path. `apply_settings_effects()` remains the only builder of settings-effect commands.

2. **Validation boundary.** Keep host validation about settings values and device validation about backend support. `Viewer` and `MultiViewer` call the value-only `validate_render_settings(settings)` before any fallback choice; `graphics::RenderDevice` checks actual backend support at render entry and owns the one-time fallback diagnostics. The existing `validate_render_settings(settings, capabilities)` overload remains available to callers that genuinely hold a `DeviceCapabilities` instance, but the viewer no longer fabricates one. No virtual method or public class layout is added.

3. **OpenGL shadow pass.** Let the OpenGL adapter accept triangle `shadow_caster` commands, virtual `shadow_background` receivers, and fullscreen `shadow` commands. Render clipped triangle casters into a cached depth map, keep the background receiver unclipped as today, support hard and soft filtering and virtual backgrounds, and apply the overlay using the frame's resolved depth. Cache keys must include viewport size, map size, softness mode, and sample count.

4. **OpenGL AO and EDL passes.** Accept `ambient_occlusion` and `eye_dome_lighting` commands, render from the clipped scene depth, and support simultaneous AO and EDL in the documented order. Preserve radius/bias/intensity units and projection handling; reject or warn on invalid settings through the existing validation rules.

5. **Transparency composition.** First compose sorted transparency with all effects in the OpenGL device. Then add capability-gated average-color, depth-peeling, and linked-list paths, using the Vulkan semantics as the reference. Below OpenGL 4.3, linked lists continue to fall back explicitly; a fallback must still honor effective clipping and emit the existing one-time diagnostic.

6. **Viewer migration.** In OpenGL builds, route settings-driven frames through the shared host submit path and remove the settings-effect sidecar branch from `Viewer::draw()`. Retain the legacy effect objects only for application-owned compatibility effects and for scenes that genuinely require the exact native pass. Remove the `warn_native_clipping()` warning only when the device path actually supports the requested state.

7. **Framebuffers, readback, and cells.** Verify MSAA resolve, depth/ID readback, viewport/scissor restoration, and resource lifetime for every effect pass. Per-`MultiViewer` cell targets must key on integer cell bounds, not rescaled logical sizes. Picking and depth queries must continue to sample scene depth rather than effect overlays.

#### Validation plan

- Extend `render_settings` and `clipping` regressions on both backends with one-plane, two-plane, tiny-normal, non-finite-normal, cross-section, and process-wide-plane cases for each effect.
- Add an effect-composition matrix covering shadow only, AO only, EDL only, AO+EDL, shadow+AO+EDL, and each effect combined with each capability-supported transparency method. Compare backend results with documented numeric tolerances, not exact GPU equality.
- Assert that cut pixels do not receive shadow/AO/EDL darkening, that triangle shadow casters respect the same planes, that the clipping-plane guide remains visible with its current depth-test contract, and that object/primitive IDs and picked depth are unchanged.
- Exercise plain and analytic points, lines, triangles, transparent triangles, AO textures, custom drawables, and custom `draw()` overrides so that supported and fallback states are explicit.
- Cover live frames, file snapshots, raw `capture()`, `OffScreen`, and every `MultiViewer` cell, including resize and sample-count changes.
- Rerun focused clipping, capture, render-settings, picker, and MultiViewer tests, then complete serial suites in both approved build trees. Record counts and times only after the final source state.

#### Implementation result

- `graphics::effective_clipping_planes()` resolves one plane set before command assembly. A process-wide `ClippingPlane` overrides `RenderSettings::clipping_planes`; otherwise normalized settings planes are used. Scene geometry, transparent commands, triangle shadow casters, and the depth consumed by AO/EDL share that set. Effect quads, the virtual shadow receiver, HUD, annotations, and the clipping-plane guide remain overlays.
- Validation has one clear boundary: `Viewer`, `MultiViewer`, and the OpenGL viewer state validate settings *values* before any fallback choice; `graphics::RenderDevice` validates backend *support* and resolves fallbacks at render entry. No virtual method or public class layout changed. `Viewer::append_settings_effects()` and the validation helper are protected and nonvirtual. The unused host capability descriptor was removed rather than being made artificially “accurate”.
- `OpenGLDevice::Impl` owns all new effect shaders, framebuffers, textures, and pass state. The device applies Vulkan precedence (`shadow > ambient_occlusion > eye_dome_lighting > scene_depth_texture`) and frame order: shadow map, OIT prepass, optional ID/depth prepass, AO compute, color scene with composite/overlays, optional capture ID pass. Clipping of analytic impostors and shadow casters uses the existing shared uniforms; effect overlays are unclipped.
- OpenGL linked lists below 4.3 fall back to depth peeling. The portable device falls back from average-color blending without floating-point color blending to sorted transparency; the legacy native viewer uses its existing final peeling fallback. The exact one-time diagnostics for those paths, settings/legacy shadow precedence, native transparency conflicts, native `MultiViewer` fallback, AO textures, custom types, and sorted fallback are preserved.
- `capture()` remains a raw scene-command readback: effects are excluded from in-memory captures but included in live frames and file snapshots. Raw capture also restores the drawable alpha that presentation transparency replaces with `TrianglesDrawable::opacity()`. `MultiViewer` keeps the stable model/standalone capture IDs used by `pick()` and builds a parallel source-mapped command list so alpha restoration does not depend on source-query renumbering or command order.

#### Validation

1. [tests/viewer/render_settings_test.cpp](../tests/viewer/render_settings_test.cpp) covers hard, soft, and scene-only settings shadows, AO, EDL, AO+EDL, all four transparency methods, and one combined shadow+AO+EDL+peeling frame on both backends. It also asserts that setters still reject invalid values while accepting every known transparency method. File snapshots differ from the plain frame; in-memory `capture()` stays byte-identical to the raw reference after every presentation effect.
2. [tests/viewer/effects_presentation_test.cpp](../tests/viewer/effects_presentation_test.cpp) is a new shared device-path oracle for both backends. It checks stream-ordered effect composition, AO+EDL, shadow+AO+EDL, EDL and shadow over peeled transparency, clipped cut pixels for AO/EDL/shadow casters/transparent geometry, unchanged picking IDs and depth, and per-cell OIT viewports for sorted, average, peeling, and linked lists. Both runs report zero backend validation errors.
3. [tests/viewer/opengl_render_device_test.cpp](../tests/viewer/opengl_render_device_test.cpp) now asserts that average-color transparency falls back to sorted transparency when floating-point color blending is unavailable. The existing clipping, capture, snapshot, picker, and MultiViewer regressions pass with the new command path.
4. All targets rebuilt successfully in both approved trees. Focused runs of `effectpresentation`, `rendersettingseffects`, `snapshoteffects`, `renderdevice`, `rendersettings`, `drawablecontract`, `clipping`, `multiview`, and `capture` passed OpenGL 9/9 in 14.20 seconds and Vulkan 8/8 in 11.31 seconds.
5. Final serial desktop CTest on the final source state: OpenGL **129/129 in 154.35 seconds** and Vulkan **157/157 in 256.90 seconds**, both with zero failures and `CTEST_PARALLEL_LEVEL=1`. `git diff --check` passed.

#### Acceptance checklist

- [x] Settings-driven shadow, AO, EDL, transparency, and clipping use one command/presentation path on both backends.
- [x] Effective planes are applied to visible geometry, triangle shadow casters, and AO/EDL depth inputs; overlay geometry and the clipping-plane guide are not scene-clipped, and guide depth testing is unchanged.
- [x] AO+EDL and effect+transparency combinations compose on OpenGL without a silent precedence warning where the device advertises support.
- [x] Unsupported combinations are diagnosed once by `RenderDevice` before fallback; legacy facades and exact fallbacks remain documented and working.
- [x] `capture()`, presented snapshots, picking, depth reads, and `MultiViewer` cells preserve their documented semantics.
- [x] Milestone 3 deviations 2–4 and the section 8.15 scope boundary are updated from "limitation" to the measured result, with both complete suites passing.

#### Residual coverage gaps

- Diagnostics are preserved in code and exercised indirectly by fallback behavior, but no test captures the exact one-time log strings.
- The new effect matrix does not include AO textures, custom `Drawable` subclasses, or custom `Viewer::draw()` overrides with settings effects; those states still take their exact native fallbacks and are covered only by the earlier fallback-specific regressions.
- `OffScreen` capture/snapshot, resize, and sample-count changes are covered by the existing device and viewer suites, but not repeated for every settings-effect combination.
- `MultiViewer` cells are covered by the default, clipping, and snapshot regressions; they are not measured with a full numeric effect matrix in every cell.
- There is no host-side low-capability simulation: by design the viewer accepts all known settings methods and the device resolves actual support. Backend fallbacks are exercised at the device level.
- Cross-backend effect images are compared by difference-from-plain plus ID/depth invariants, not by exact or tolerance-bounded pixel equality, because the two backends intentionally keep their established shadow/AO/EDL/OIT algorithms.

### 8.17 Independent API-unification audit and proofreading

Audit date: 2026-11-27. The complete branch was reviewed against the pre-unification
`main` commit `2924ec3fa9b736785e26a23fafc751571c614981`, the unification plan, the
active and historical change records, and the final C++/CMake/tests/documentation
diff. The intentional 2.7 renames and Mapple ownership changes recorded in
`ae919b42`, `e5c99b29`, and `f02c0083` were treated as fixed inputs, not reverted.
`Viewer::draw_commands()` was already virtual in the compatibility baseline; no
new vtable slot was introduced for it.

The review corrected genuine deficiencies while preserving documented behavior:

- Restored the `Drawable` storage arrangement and used existing padding for its
  cache flags. `DrawCommand` remains standard-layout and 616 bytes on the validated
  64-bit host; normalized depth-range endpoints occupy tail padding as two
  `std::uint16_t` values with an invalid sentinel. `Viewer::GraphicsResources`
  reuses the former point-selector pointer slot for the renderer/query device.
- Kept legacy backend-options constructors and default arguments source-compatible,
  marked them with `EASY3D_DEPRECATED(message)`, and delegated through non-deprecated
  tagged constructors where applicable. OpenGL accepts and ignores only the native
  large-point preference; it rejects requested validation, synchronization
  validation, profiling, or device-name selection instead of silently dropping a
  hard requirement. No compiler-conditional warning suppression was added.
- Preserved a caller's native CGL/WGL/GLX/EGL current-context state across temporary
  query and snapshot operations, including another current context owned by a
  separate GLFW static copy. The Windows unchanged check compares both HGLRC and
  HDC; EGL restoration retains separate read and draw surfaces. Framebuffer,
  viewport, projection, clear-color, and camera-signal state are restored on all
  tested paths.
- Fixed raw MultiViewer capture to retain stable model/standalone IDs while a
  parallel source-mapped command list recovers original drawable alpha. Raw capture
  continues to exclude presentation effects; presented frames and file snapshots
  include them. Overlay/effect stream order is shared by both adapters.
- Matched native OpenGL transparency capability fallback to the portable device:
  linked lists require OpenGL 4.3, and average-color blending requires
  floating-point color blending. Same-viewport linked-list capacity changes now
  invalidate and recreate the native node pool. Portable average-color fallback is
  sorted transparency; the legacy native viewer retains its peeling fallback.
- Fixed highlight-texture deletion when a mask is cleared, background depth-query
  miss reporting, native custom-drawable query dispatch, and complete geometry,
  image, and screen-line cache clearing.
- Made `KeyframeInterpolator::replace_keyframes()` and Mapple generation/undo
  replacement atomic, preserved loaded-style coincident/nonmonotone paths, and
  made periodic interpolation explicit in provenance snapshots. `add_keyframe()`
  still rejects coincident positions and nonmonotone times.
- Corrected installed consumers, test registrations, stale comments, and
  MultiViewer/fallback/capture documentation. Fresh OpenGL installation exposed a
  header-name collision: the flattened public-header install replaced the shared
  `renderer/render_device.h` with `renderer/backends/opengl/render_device.h`. The
  deprecated OpenGL adapter now installs in its backend subdirectory, preserving
  the shared contract header. Added cross-context, layout, keyframe-replacement,
  transparency, and cache regressions. The legacy manipulated shadow-caster
  omission was confirmed in the compatibility baseline and left unchanged rather
  than misclassified as a unification regression.

Validation on the final source state:

- Both approved all-target builds succeeded.
- Focused query/capture/transparency/effects/offscreen/MultiViewer regressions passed
  **9/9 Vulkan** and **9/9 OpenGL**.
- Complete serial CTest passed **Vulkan 157/157 in 251.98 seconds** and
  **OpenGL 130/130 in 114.57 seconds**, with `CTEST_PARALLEL_LEVEL=1` and zero
  failures. One intermediate OpenGL run aborted in `opengl.input` after the test
  printed its PASS line; it passed immediately in isolation and in the clean
  complete rerun, matching the documented focus/load flake. No source change was
  made for that non-reproducing teardown crash. `git diff --check` passed.
- Fresh isolated Vulkan and OpenGL Release packages were built and installed, and
  the installed offscreen, unified, and public-viewer consumers were compiled and
  run against those packages.

The public-header search still finds `EASY3D_HAS_VULKAN` in 15 headers. Many
occurrences are private storage or native compatibility fallbacks retained by the
2.8 policy; client-visible differences also remain in the legacy effect facades,
the backend-dependent fields of `easy3d::DeviceOptions`, `Viewer::set_samples()`. The Milestone 5
header-audit item therefore remains open rather than being closed by the runtime tests.

Remaining gates are explicit: this is local macOS/MoltenVK evidence, not a Linux,
Windows, or hosted-CI result. `.github/workflows/tests.txt` is not an active workflow,
so the current branch has no automated test-workflow validation; CI was left
untouched as requested. The Milestone 5 public-header `EASY3D_HAS_VULKAN` audit
item and Milestones 6 and 7 remain open, and the root package version is still
`2.7.0` although the compatibility policy targets 2.8; release bookkeeping
is not changed by this audit. No new interactive Mapple visual comparison is
claimed by this pass.

### 8.18 Simplified host constructors

Validation date: 2026-11-28.

The compatibility-preserving host-constructor design recorded in sections 8.7 and 8.13 was
superseded to keep the public signatures simple:

- `Viewer`, `OffScreen`, and `MultiViewer` now take `easy3d::DeviceOptions` directly. Its fields
  describe the configured backend: Vulkan device/validation settings or OpenGL context requests.
  Their parameter order and default arguments are unchanged.
- The `VulkanOptions` and `OpenGLOptions` names, deprecated compatibility constructors, constrained
  templates, `detail::make_device_options()` helper, and `DeviceOptionsTag` dispatch were removed.
  Default construction such as `Viewer("title")` and `Viewer("title", {}, {})` is unchanged;
  callers that explicitly named an old option type must migrate to `DeviceOptions`; the old
  type names and constructor symbols are not retained as binary-compatibility aliases.
- `graphics::DeviceOptions` remains the backend-neutral low-level render-device option type.
  Host and device options no longer convert implicitly, and code configuring both creates each
  value explicitly.
- In-tree applications, tutorials, tests, installed consumers, and API-smoke programs were migrated
  to the simplified host API. OpenGL host-option rejection tests were removed because Vulkan-only
  diagnostics fields no longer exist in the OpenGL host type; the equivalent low-level device
  rejection tests remain in `opengl_render_device_test`.

This intentionally revises the earlier 2.8 host-constructor compatibility exception described in
section 8.7; it does not change rendering behavior or the low-level device factory contract.

Both all-target backend builds succeeded. Complete serial CTest passed **Vulkan 157/157 in
247.69 seconds** and **OpenGL 130/130 in 153.36 seconds**, with zero failures. Fresh isolated
Release packages were rebuilt and installed, and all five installed consumer executables
(three Vulkan, two OpenGL) passed. `git diff --check` passed.

### 8.19 Milestone 6 preparation: gated retirement audit and private OpenGL sources

Before applying any Milestone 6 removal, the current tree was audited against the section 8.7
release policy and section 8.8 removal gates. The audit covered installed headers, Python
bindings and tests, tutorials, applications, tests, live OpenGL plumbing, and the documented
overlay/capture parity boundary. The result is that **no OpenGL-facing API is
`safe-to-delete now`**:

- The root package is still `2.7.0`, no complete `2.8.x` deprecation release has shipped, the
  migration guide and downstream removal notes are Milestone 7 work. CI is now active via
  `.github/workflows/tests.yml` (converted from the inactive `tests.txt`).
- Python bindings/tests still exercise native drawable buffers, `vao()`, `draw()`, `gl_draw()`,
  and `State::set_ao_texture()`. Tutorials 501–505 and Mapple still use the legacy effect
  facades. `EASY3D_HAS_VULKAN` is still exported to installed consumers and remains visible in
  installed and test headers. These are mandated consumer-migration gates, not incidental
  leftovers.
- `FramebufferObject`, shader plumbing, VAO storage, texture realization, native shape drawing,
  and `OverlayRenderer` still support live OpenGL presentation, effects, overlays, and exact
  fallbacks. Backend parity tests cover public overlay and capture behavior, but the OpenGL
  device and Viewer still own native overlay resources.
- The duplicated `Viewer::draw_corner_axes()` implementations preserve distinct shared-command
  and native/fallback presentation paths; unifying them belongs near the end of retirement.
- `scan_visible_points()` was deleted (Vulkan-only, no production callers). In 2.8.x a
  `scan_visible_points()` tutorial demonstration will live in `Tutorial_405_VirtualScanner`,
  using public `Viewer`/`OffScreen` capture APIs rather than a `Viewer` member.
- During the preparation pass, the duplicated render-settings accessors were consolidated. The
  previously duplicated `Viewer::set_clipping_planes()`, `Viewer::clipping_planes()`,
  `Viewer::render_settings()`, and `Viewer::set_render_settings()` implementations in the Vulkan
  and OpenGL backend branches were replaced by backend-neutral implementations backed by private
  state accessors (`backend_render_settings()`, `backend_clipping_planes()`,
  `backend_set_render_settings()`, `backend_set_clipping_planes()`). Validation, clipping-plane
  normalization, and the store sequence now live in one place; the backends retain only their
  private state holders (`Viewer::Impl` for Vulkan, the per-viewer OpenGL state for OpenGL).
  No public API, class layout, or vtable changed.

- The first hosted run of the newly enabled `.github/workflows/tests.yml` (2026-11-28) then failed,
  and the fixes below came out of it:

  - Windows: `tests/viewer/effects_presentation_test.cpp` built overlay locals named `far` and
    `near`, which MSVC reserves as FAR/NEAR pointer keywords, so the Windows OpenGL/Vulkan test
    compile failed. The locals are now `far_overlay` and `near_overlay`.
  - Ubuntu: `easy3d_viewer_query_test` (OpenGL builds) referenced the EGL entry points while the
    OpenGL backend selected GLX, so linking failed with `undefined reference to eglGetCurrentContext`.
    The native-context probe now selects the backend through the same `EASY3D_OPENGL_X11` /
    `EASY3D_OPENGL_WAYLAND` macros that `OpenGLContextScope` uses in
    `easy3d/viewer/backends/glfw_runtime.cpp`, and the test links EGL only when Wayland is selected.
    The macOS Vulkan device-hang failures did not reproduce on the local macOS/MoltenVK rerun and
    are recorded as hosted-runner flakiness pending a fresh hosted run.
  - The display-requiring OpenGL renderer tests (`opengl.render`, `opengl.transparency`) were
    missing the CTest `desktop` label, so the display-less macOS OpenGL job and the Ubuntu no-display
    run still attempted windowed GL context creation and failed (`NSGL` on macOS, missing `DISPLAY`
    on Ubuntu). Both tests now carry `LABELS desktop`, matching the other windowed OpenGL tests.
  - The Windows build then failed in `tests/renderer` with MSVC error C2872: the local
    `DeviceCapabilities` (via `using namespace easy3d::graphics;`) collided with a WinGDI symbol
    (`DeviceCapabilitiesA`) from the Windows SDK headers, leaving `capabilities` unresolvable.
    The tests now qualify the type (`graphics::DeviceCapabilities`).

Accordingly, no deprecated declaration, definition, binding, forwarding header, install entry,
or test was deleted, and no public class layout or vtable was changed. The safe preparatory step
was to make the remaining native OpenGL implementation files private to the backend source tree:

- `framebuffer_object.cpp`, `opengl.cpp`, `opengl_error.cpp`, `opengl_util.cpp`,
  `opengl_timer.cpp`, `read_pixel.cpp`, `shader_manager.cpp`, `shader_program.cpp`, and
  `vertex_array_object.cpp` moved from `easy3d/renderer/` to
  `easy3d/renderer/backends/opengl/`.
- The OpenGL source list in `easy3d/renderer/CMakeLists.txt` was updated to the new paths.
- The installed forwarding headers and exported symbols remain unchanged, so existing clients
  keep the same 2.8 compatibility surface. Texture realization and immediate-mode shape
  implementation were already private.
- Historical path references in `changes/vulkan.md` were updated to the new source location.

Milestone 6 therefore remains open with removal gated on the full section 8.8 checklist. Each
future removal must repeat the consumer audit immediately before deletion.

The reconfigured all-target builds succeeded in both approved trees. Serial CTest passed
**OpenGL 130/130 in 160.86 seconds** (`build-opengl`) and **Vulkan 157/157 in 256.18 seconds**
(`build`), including both Python rendering tests. Staged installed-package consumers were also
built and run: two OpenGL executables and three Vulkan executables passed, including the backend
component checks. Vulkan scan tests (`vulkan.scan`, `vulkan.scanviewer`) pass. This is local
macOS/MoltenVK evidence; it does not establish Linux/Windows or hosted-CI behavior.
`git diff --check` passed.
