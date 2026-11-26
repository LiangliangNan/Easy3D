# Vulkan/OpenGL Unification: OpenGL Renderer Redundancy Audit

**Date:** 2026-11-26  
**Scope:** `easy3d::renderer`, the OpenGL `Viewer`, GUI pickers, effects, Mapple/Mapple-Qt, tutorials, tests, and Python bindings.  
**Status:** Read-only architectural and caller audit. No code was changed.

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

5. **Native picking/depth fallbacks can be retired incrementally**, as shared-device support expands to clipping, AO, transparency, impostors, non-native line widths, and `MultiViewer`. They cannot be removed today without changing behavior.

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

The OpenGL `Viewer` still renders its live scene through native drawable implementations. The decisive dependency is [easy3d/viewer/viewer.cpp:2485](../easy3d/viewer/viewer.cpp#L2485), which calls:

- `LinesDrawable::draw()`.
- `PointsDrawable::draw()`.
- `TrianglesDrawable::draw()`.
- Native shadow and clipping rendering.
- Native overlay rendering.

Consequently, the shared device replaces specific offscreen/query functionality; it does not yet replace the complete OpenGL presentation path.

## 1. Confirmed unused in-tree

### 1.1 Complete units

| Component | Finding | Recommended treatment |
|---|---|---|
| `easy3d::opengl::read_pixel` | No callers outside `read_pixel.{h,cpp}`; no tests or active Python bindings. This includes single-pixel and buffer `read_color`/`read_depth`, MSAA variants, and color/depth snapshot functions. See [easy3d/renderer/read_pixel.h:52](../easy3d/renderer/read_pixel.h#L52). | Remove declarations, definitions, and CMake entries if public API breaks are acceptable. Otherwise deprecate first. |
| `easy3d::OpenGLTimer` | No callers outside its own header/source; no tests or bindings. See [easy3d/renderer/opengl_timer.h:53](../easy3d/renderer/opengl_timer.h#L53). | Same treatment as `read_pixel`. |

Both are listed in the OpenGL-only headers and sources at [easy3d/renderer/CMakeLists.txt:92](../easy3d/renderer/CMakeLists.txt#L92). Removing them requires no rendering adaptation.

### 1.2 Public members with no active in-tree callers

The following are cleanup candidates. Public API policy still applies.

| Component | Unused surface | Notes |
|---|---|---|
| `ShaderManager` | `create_program_from_codes()`, `all_programs()`, and the individual-file `create_program_from_files()` overload | The base-name creation overload, `get_program()`, `reload()`, and `terminate()` remain used. See [easy3d/renderer/shader_manager.h:82](../easy3d/renderer/shader_manager.h#L82). |
| `ShaderProgram` | `set_verbose()`, `program_output()`, `is_program_valid()`, `load_binary()`, `save_binary()` | These belong to diagnostics/binary-program support with no external callers. `set_program_output()` remains used by `ShaderManager`. See [easy3d/renderer/shader_program.h:128](../easy3d/renderer/shader_program.h#L128). |
| `VertexArrayObject` | `is_supported()`, `create_storage_buffer()`, `update_storage_buffer()`, `map_buffer()`, `unmap_buffer()` | Core array/element buffer creation and `get_buffer_data()` remain required. See [easy3d/renderer/vertex_array_object.h:58](../easy3d/renderer/vertex_array_object.h#L58). |
| `FramebufferObject` | `is_supported()`, `attach_color_texture()`, `copy_color_to_texture()`, `copy_depth_to_texture()`, `is_bound()`, `num_color_attachments()`, `has_color_attachment()`, `has_depth_attachment()`, `depth_bits()`, `has_stencil()`, `texture_target()`, `has_color_texture()`, `has_depth_texture()`, `deactivate_read_buffer()`, and all `blit_framebuffer()` overloads | `attach_depth_texture()` is used by `OpenGLDevice`; draw/read-buffer activation and color readback remain used. See [easy3d/renderer/framebuffer_object.h:128](../easy3d/renderer/framebuffer_object.h#L128). |
| `FramebufferObject` depth helpers | Buffer `read_depth()` overloads and `snapshot_depth()` | The vector `read_depth()` overload is tests-only; the pointer overload has no caller. `snapshot_depth()` appears only in disabled code. Single-pixel `read_depth()` remains production-used. |
| `FramebufferObject` diagnostics | `print_attachments()`, `print_draw_buffers()`, `print_read_buffer()` | Referenced only from disabled debug code. |
| `OpenglUtil` | Profile, vendor, renderer, version, extension, GLSL/GLEW, sample, viewport, GPU-memory, output-stream, and `get_*_info` queries | Only `init()`, `is_supported()`, and `has_extension()` have production callers. See [easy3d/renderer/opengl_util.h:47](../easy3d/renderer/opengl_util.h#L47). |
| OpenGL error helpers | `check_gl_error()`, `check_frame_buffer_error()`, `easy3d_gl_error`, and `easy3d_frame_buffer_error` | The logging variants remain used throughout production code. See [easy3d/renderer/opengl_error.h:38](../easy3d/renderer/opengl_error.h#L38). |
| Native `shape` functions | Textured `draw_quad_filled()` and textured `draw_full_screen_quad()` | The bound-shader fullscreen-quad overload is heavily used. See [easy3d/renderer/shape.h:84](../easy3d/renderer/shape.h#L84). |
| `Texture` | `bind()` and `release()` | Tests-only; production rendering uses texture IDs through shader binding. Public API nonetheless. See [easy3d/renderer/texture.h:119](../easy3d/renderer/texture.h#L119). |

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

The bindings are at [python/bindings/easy3d/renderer/drawables.cpp:194](../python/bindings/easy3d/renderer/drawables.cpp#L194) and [python/bindings/easy3d/renderer/drawables.cpp:279](../python/bindings/easy3d/renderer/drawables.cpp#L279).

`Drawable::vertex_buffer()`, `color_buffer()`, and `normal_buffer()` also have production callers in effects, native shape drawing, and Mapple-Qt. They are not deletion candidates today.

Recommended treatment for Python-only methods:

1. Introduce backend-neutral predicates or resource queries.
2. Deprecate the raw GL accessors.
3. Remove the Python bindings only in a coordinated API release.

## 3. Components that remain required

| Component | Production dependencies | Conclusion |
|---|---|---|
| `FramebufferObject` | Viewer capture and fallbacks; `MultiViewer`; GUI pickers; AO, EDL, shadows, transparency; `OverlayRenderer`; `OpenGLDevice` | Required. The adapter itself allocates targets with it at [easy3d/renderer/backends/opengl/render_device.cpp:342](../easy3d/renderer/backends/opengl/render_device.cpp#L342). |
| `ShaderProgram` / `ShaderManager` | Native drawables, corner axes, picking fallbacks, overlays, and effects | Required. Viewer uses the manager directly at [easy3d/viewer/viewer.cpp:2335](../easy3d/viewer/viewer.cpp#L2335). |
| `Texture` / `TextureManager` | Native textured/scalar-field rendering; material handling; shared command extraction; Mapple UI and tutorials | Required. The shared device consumes CPU pixel data but does not replace live texture realization. |
| `VertexArrayObject` | Native buffer creation, element buffers, `Drawable::buffer_data()` readback, screen-space lines, shape drawing | Required. |
| Native `Drawable` storage | Live rendering and CPU snapshots used by `DrawableRenderer`/`OpenGLDevice` | Required. |
| `ClippingPlane` | Viewer controls, native shader setup, clipping visualization, pickers, effects, Mapple, tutorials | Required. See [easy3d/viewer/viewer.cpp:2535](../easy3d/viewer/viewer.cpp#L2535). |
| Shadow/AO/EDL/transparency | Viewer integration, Mapple, and rendering tutorials | Required as application/library functionality, even where base Viewer does not call every effect directly. |
| `opengl::OverlayRenderer` | HUD text and client overlays, including depth-tested overlays | Required today. See [easy3d/viewer/viewer.cpp:2423](../easy3d/viewer/viewer.cpp#L2423). |
| `LinesDrawable2D` | Pivot marks, screen-space guides, `MultiViewer`, portable command extraction, tutorials | Required. |
| Native `shape_draw` | Corner axes, fullscreen effect passes, manipulator visualization, Mapple-Qt tools | Required. |
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

This is currently blocked by the headless contract: `OpenGLDevice::present()` throws at [easy3d/renderer/backends/opengl/render_device.cpp:950](../easy3d/renderer/backends/opengl/render_device.cpp#L950).

**Assessment:** coherent future migration, but not a simple caller replacement.

### 4.2 Native picking and depth fallbacks

The old FBO/shader fallback blocks and `Picker::fbo_` can be removed incrementally as shared-device support expands to:

- Enabled clipping.
- AO and non-opaque drawables.
- Unsupported coloring or unavailable CPU texture data.
- Point/line impostors.
- Non-native line widths.
- `MultiViewer`.

The capability gate is visible at [easy3d/viewer/viewer.cpp:2567](../easy3d/viewer/viewer.cpp#L2567).

**Assessment:** retire individual fallback branches as parity is added. Do not remove the fallback system as one change.

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
- No active Python bindings were found for `FramebufferObject`, `read_pixel`, `ShaderManager`/`ShaderProgram`, VAO methods, `OpenGLTimer`, `OpenglUtil`, native shape functions, `ClippingPlane`, or `OverlayRenderer`.

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

The existing foundation is already portable: see [WindowOptions and Viewer](../easy3d/viewer/viewer.h#L73), [ViewerBase](../easy3d/viewer/viewer_base.h#L77), and [OffScreen](../easy3d/viewer/offscreen.h#L50).

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

The mutually exclusive public `OpenGLOptions` and `VulkanOptions` should be replaced by common diagnostics and device-selection options:

```cpp
struct DeviceOptions {
    bool validation = false;
    bool sync_validation = false;
    bool profiling = false;
    std::string device_name;
    bool use_native_large_points = true;
};
```

OpenGL context-version negotiation and Vulkan instance/device construction become implementation details.

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

The final change is that `device()`, `set_clipping_planes()`, and command assembly stop being Vulkan-only. Their current transitional forms are visible in [Viewer overlays and clipping](../easy3d/viewer/viewer.h#L836), [capture and device access](../easy3d/viewer/viewer.h#L904), and [draw_commands()](../easy3d/viewer/viewer.h#L1220).

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

The backend-specific members currently declared in [Drawable buffer access and rendering](../easy3d/renderer/drawable.h#L220) would be removed:

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

The existing [RenderDevice interface](../easy3d/renderer/render_device.h#L465) and its [command/value types](../easy3d/renderer/render_device.h#L39) are already close to the desired final form.

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
| `WindowOptions::{width, height, samples, full_screen, visible, resizable}` | [viewer.h](../easy3d/viewer/viewer.h#L73) already defines all six fields with the proposed defaults. | Retain | Keep the struct and its current sample-count validation/fallback contract. |
| `DeviceOptions::{validation, sync_validation, profiling, device_name}` | Vulkan has these controls in `VulkanOptions`; OpenGL instead has `OpenGLOptions` for context requests. | Add | Introduce one common struct. OpenGL context negotiation becomes private implementation detail. Preserve the existing Vulkan large-point comparison control in an advanced/common option rather than silently dropping it. |
| `Viewer(title, window, options)` | Constructor selects `VulkanOptions` or `OpenGLOptions` through `EASY3D_HAS_VULKAN` in [viewer.h](../easy3d/viewer/viewer.h#L155). | Adapt | Replace the client-visible conditional type with `DeviceOptions`; retain temporary backend-option overloads during migration. |
| `run`, `run_frames`, `draw_frame`, `show`, `hide`, `resize`, `update`, `camera` | All are available on both backends through `Viewer`/`ViewerBase`; Python omits `draw_frame`, `show`, and `hide`. | Retain C++; add Python parity later | No signature redesign is needed. |
| `add_model(file, create_drawables, fit_scene)` | Available on both backends; Python's file overload omits `fit_scene`. | Retain C++; adapt Python | Add `fit_scene` to Python without changing existing calls. |
| `add_model(shared_ptr<Model>, create_drawables, fit_scene)` | Shared ownership is already supported; raw-pointer overloads transfer ownership. | Adapt | Retain the `shared_ptr` overload. Deprecate raw-pointer ownership transfer and coordinate return-type modernization with the separate raw-pointer TODO. |
| `add_drawable(shared_ptr<Drawable>)` | Shared ownership is already supported; raw-pointer overloads transfer ownership. | Adapt | Retain the `shared_ptr` overload; deprecate raw-pointer ownership transfer and coordinate return types with the model migration. |
| `delete_model`, `delete_drawable`, `clear_scene`, `fit_screen` | Available on all C++ hosts. Python exposes deletion on `OffScreen` but not on `Viewer`/`MultiViewer`. | Retain C++; add Python parity | Raw pointer parameters here are non-owning observers, distinct from the ownership-transfer overloads. |
| `pick`, `set_overlays` | Portable C++ APIs already exist and are used by both hosts. | Retain | Keep command-list overlay submission and `PickResult`. |
| `select_points`, `point_resource` | Both exist, but OpenGL currently requires a viewer-created native point snapshot while Vulkan also accepts portable CPU `PointData`. | Adapt | Keep the methods and make portable `PointData` requests behave consistently on both backends. |
| `scan_visible_points` | Public only in Vulkan builds and omitted from the abbreviated proposal. | Adapt | Retain the useful capture/selection helper and provide equivalent OpenGL behavior before removing the public backend conditional. |
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
| `Drawable::BufferData` fields | [drawable.h](../easy3d/renderer/drawable.h#L70) already has vertices, normals, colors, texcoords, indices, source faces, and revision. | Retain | Keep the existing fields and revision meaning. |
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
| `Topology`, `PointStyle`, `LineStyle`, `TransparencyMethod`, `FrameReadback` | Already defined in [render_device.h](../easy3d/renderer/render_device.h#L39). | Retain | Reuse these enums. |
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
- `DeviceCapabilities` reports shadow, AO, EDL, supported transparency methods, and native large-point availability. Low-level `RenderDevice` calls reject unsupported commands/methods explicitly. High-level `set_render_settings()` also rejects an explicitly enabled unsupported effect instead of silently producing different imagery. Applications can inspect capabilities and choose a supported configuration.
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
- Captured color includes the composited presentation result, including supported overlays/HUD. Depth and picking IDs describe scene geometry and exclude overlays. OpenGL may report `UINT32_MAX` for unavailable primitive provenance, as documented today.
- A default-constructed `FrameImage` is an empty value. A successful capture never returns that empty sentinel; invalid sizes/readback requests throw.

#### Host visibility

| Operation | `Viewer` | `OffScreen` | `MultiViewer` |
|---|---|---|---|
| Scene, camera, picking, overlays, clipping, render settings, capture, `device()` | Public | Public | Public |
| `render(file, ...)` | n/a | Public | n/a |
| Grid assignment/layout/divisions | n/a | n/a | Public |
| `run`, window show/hide/resize/position/title | Public | Protected | Public |
| Native `window()` handle | Protected | Protected | Protected |
| `init`, `cleanup`, `pre_draw`, `post_draw`, `draw`, `draw_commands` | Protected customization hooks | Protected customization hooks | Protected customization hooks |

`cleanup()` becomes protected virtual. `ViewerBase::window()` and `Viewer::window()` become protected, with a deprecation period for external native-window callers.

#### Image conventions

- `graphics::Image` and `FrameImage` are canonical top-row-first CPU images. Both adapters preserve that row order on upload; screen-space `(0,0)` is the top-left corner. No global image-loader flip state is introduced, and native texture IDs never become part of the portable image contract.
- Legacy `Texture::PixelData` remains bottom-row-first during compatibility; `DrawableRenderer` performs the explicit row conversion at that boundary.
- No global image-loader flip state is introduced, and native texture IDs never become part of the portable image contract.

### 8.7 Migration and deprecation policy

#### Release policy

- **2.8.0 — compatibility release:** add portable replacements and deprecate old APIs. Preserve existing symbols, public class layouts, and virtual tables. New convenience overloads may be non-virtual forwarding wrappers.
- **3.0.0 — removal release:** remove deprecated declarations, definitions, Python bindings, forwarding headers, and install/export entries; make native handles protected/private; land virtual-interface changes that cannot preserve the 2.x vtable.
- A deprecated API must survive at least one complete 2.8.x release series. Removal is allowed in 3.0.0 only after every gate in section 8.8 passes.
- The raw-pointer ownership migration is coordinated with, but not silently folded into, this API work. Its ownership-transfer overloads target 3.0.0 only if the separate modernization is complete; otherwise they remain deprecated until the next coordinated major release.

Because Easy3D requires C++11, deprecations use one project macro rather than C++14 `[[deprecated]]` syntax. Define `EASY3D_DEPRECATED(message)` once in `easy3d/core/types.h`: use `[[deprecated(message)]]` for C++14+, `__declspec(deprecated(message))` on MSVC, `__attribute__((deprecated(message)))` on GCC/Clang, and an empty definition only on compilers with no supported diagnostic.

#### Exact C++ migrations

| Old signature/API | Replacement and compatibility wrapper | Deprecate | Remove |
|---|---|---|---|
| `Viewer(title, WindowOptions, VulkanOptions/OpenGLOptions)` | Final `Viewer(title, WindowOptions, DeviceOptions)`; 2.8 adds an exact-`DeviceOptions` constrained-template constructor without trailing defaults, leaving existing calls and symbols unchanged. | 2.8.0 | Legacy constructor and transitional template in 3.0.0; ordinary target constructor then remains. |
| `OffScreen(width, height, VulkanOptions/OpenGLOptions)` | Same transition to `OffScreen(width, height, DeviceOptions)`. | 2.8.0 | 3.0.0 |
| `MultiViewer(rows, cols, title, WindowOptions, VulkanOptions/OpenGLOptions)` | Same transition to the final `DeviceOptions` constructor. | 2.8.0 | 3.0.0 |
| `RenderDevice::render(width, height, commands, background, samples, transparency, peel_layers, readback)` | `render(width, height, commands, RenderOptions)`; the 2.8 non-virtual options overload is an exact-type constrained template with no default argument and forwards directly to the existing positional virtual, preserving legacy `{}` calls. In 3.0 the options overload becomes primary virtual and the positional overload is removed. | 2.8.0 | Positional overload in 3.0.0 |
| `RenderDevice::present(...)` | No normal public replacement. Windowed hosts present through their private adapter; headless callers use `render()`. | 2.8.0 | 3.0.0 |
| `create_opengl_device()` / `create_vulkan_device(...)` | `create_device(DeviceOptions)`; old factories forward and retain their exact parameter order. | 2.8.0 | 3.0.0 |
| `graphics::VulkanSurface` / `graphics::create_vulkan_window_device(...)` | Advanced native interop moves to `graphics::backends::vulkan`; the original surface remains an ABI-preserving compatibility base and the old factory forwards to the interop factory. | 2.8.0 | 3.0.0 |
| Public `OpenGLDevice` | Use `RenderDevice` returned by `create_device()`. | 2.8.0 | Public class/header removed in 3.0.0 |
| `Drawable::draw(const Camera*)`, `gl_draw()` | `DrawableRenderer::command(...)` or host `draw_commands()`. | 2.8.0 | 3.0.0 |
| `Drawable::vao()`, `vertex_buffer()`, `color_buffer()`, `normal_buffer()`, `texcoord_buffer()`, `element_buffer()` | `Drawable::buffer_data()` and `DrawableRenderer`; no native-handle replacement. | 2.8.0 | 3.0.0 |
| `const BufferData& Drawable::buffer_data()` non-const member | Add `const BufferData& buffer_data() const`; deprecate the non-const overload and remove it in 3.0. | 2.8.0 | 3.0.0 |
| `Texture::id()`, `bind()`, `release()` | `Texture::pixels()` / `graphics::Image` plus `DrawCommand::texture`. | 2.8.0 | 3.0.0 |
| `State::set_ao_texture(unsigned int)` | `RenderSettings::ambient_occlusion` through `set_render_settings()`. | 2.8.0 | 3.0.0 |
| `Shadow`, `SoftShadow`, `AmbientOcclusion`, `EyeDomeLighting`, `AverageColorBlending`, `DualDepthPeeling`, `PerPixelLinkedListsOIT`, and their backend-facing draw/generate methods | `RenderSettings` compatibility facades; algorithms become private. | 2.8.0 | 3.0.0 |
| `ViewerBase::window()` / `Viewer::window()` public native handle | Protected native interop only; normal clients use portable window methods. | 2.8.0 | Visibility reduced in 3.0.0 |
| Raw-pointer ownership-transfer `add_model(Model*, ...)` / `add_drawable(Drawable*)` | Existing `shared_ptr` overloads; return types remain unchanged in this migration. | 2.8.0 | 3.0.0, coordinated with raw-pointer modernization |
| Public `FramebufferObject`, `ShaderProgram`, `ShaderManager`, `VertexArrayObject`, `opengl::read_color`/`read_depth` (including `_ms` and snapshot variants), `OpenGLTimer`, and `opengl::shape` helpers | `RenderDevice`, `DrawCommand`, shape command builders, and capture/diagnostics. Forwarding headers may remain deprecated in 2.8. | 2.8.0 | 3.0.0 |
| `opengl::OverlayRenderer` | `DrawCommand` overlay lists through `set_overlays()`. | 2.8.0 | 3.0.0 |

`Viewer::cleanup()` changes from non-virtual protected to virtual protected in 3.0.0. `RenderDevice::backend()` and `capabilities()` are likewise introduced as virtual methods in 3.0.0, not added to the 2.8 public vtable. Existing source-level deprecation warnings in 2.8 must not be accompanied by ABI-breaking class changes.

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

- The existing positional `RenderDevice::render()` virtual remains unchanged and deprecated. The exact-type `RenderOptions` template is non-virtual, has no default argument, and calls the positional virtual directly. Legacy three-argument and empty-braces calls retain their overload resolution and behavior.
- `create_device(const DeviceOptions&)` is the normal factory. The OpenGL and Vulkan factories retain their exact signatures as deprecated forwarding wrappers. OpenGL rejects explicitly requested validation, profiling, and physical-device selection instead of silently ignoring them.
- Device name, driver version, sample-count queries, validation diagnostics, dropped-fragment counts, and point-upload counts are available through the existing non-virtual interface. The `DeviceCapabilities` value type lands here; virtual `backend()`, `capabilities()`, and `cleanup()` remain deferred to the coordinated 3.0 vtable change.
- Native Vulkan window integration uses `graphics::backends::vulkan::VulkanSurface`. Its data-free derivation from the original compatibility base preserves the old factory's mangled symbol while giving new interop callers the explicit namespace.
- `graphics::Image` stays top-row-first. Both adapters upload rows without flipping, and screen-space overlay UV `(0,0)` is the top-left. The RealCamera tutorial now uses the shared textured-quad factory rather than duplicating geometry and projection assembly.
- General `Viewer` rendering is intentionally not routed through the new device contract yet; that integration follows after this contract is stable.

#### Validation coverage

- `devicecontract` is a CPU-only test of value defaults, exact options forwarding, legacy `{}` resolution, and command factories. It does not claim adapter execution.
- `viewer.apismoke` compiles and links the same backend-neutral low-level API in both builds.
- `vulkan.interopsmoke` checks the native surface inheritance and both interop factory signatures, and links the legacy Vulkan window factory symbol.
- `opengl.renderdevice` executes the OpenGL factory and checks option rejection, frame readback, picking, textures, overlays, transparency, and resize behavior.
- `vulkan.picking`, `vulkan.render`, `vulkan.depthoverlay`, and `vulkan.depthtexture` execute the Vulkan adapter, including validation-enabled picking and overlay paths.
- `opengl.realcamera` and `vulkan.realcamera` compare rendered inset pixels against the source image, covering orientation, filtering, resizing, and capture behavior on each backend.
- Vulkan: all eight targeted tests passed. OpenGL: all four targeted tests passed. The portable CPU regression suites passed 5/5 in each build, grouped-options calls passed strict deprecation-warning checks in C++11 and C++14 through both base and concrete OpenGL device types, and `git diff --check` passed.
