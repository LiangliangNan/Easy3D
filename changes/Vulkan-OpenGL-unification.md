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

    graphics::FrameImage capture(
        unsigned samples = 1,
        graphics::FrameReadback readback =
            graphics::FrameReadback::color_depth_ids) const;

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

    graphics::FrameImage capture(
        unsigned samples = 1,
        graphics::FrameReadback readback =
            graphics::FrameReadback::color_depth_ids) const;

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

class RenderDevice {
public:
    virtual ~RenderDevice() = default;

    virtual Backend backend() const = 0;
    virtual const std::string& name() const = 0;
    virtual const std::string& driver_version() const = 0;

    virtual std::uint32_t supported_sample_counts(
        PointStyle style = PointStyle::plain) const = 0;
    virtual std::uint32_t supported_sample_counts(LineStyle style) const = 0;

    virtual FrameImage render(
        std::uint32_t width,
        std::uint32_t height,
        const std::vector<DrawCommand>& commands,
        const RenderOptions& options = {}) = 0;

    virtual bool present(
        std::uint32_t width,
        std::uint32_t height,
        const std::vector<DrawCommand>& commands,
        const PresentationOptions& options = {}) = 0;

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
    virtual std::uint32_t oit_dropped_fragments() const = 0;
};
```

Factories become backend-neutral:

```cpp
std::unique_ptr<RenderDevice> create_device(
    const DeviceOptions& options = {});

std::unique_ptr<RenderDevice> create_surface_device(
    const Surface& surface,
    const DeviceOptions& options = {});
```

`create_opengl_device()` and `create_vulkan_device()` may remain temporarily as deprecated wrappers. Public concrete classes such as `OpenGLDevice` and `VulkanDevice` are unnecessary; callers program against `RenderDevice`.

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
| `read_pixel`, `OpenGLTimer` | `RenderDevice::render()` and diagnostics |
| Backend-specific public option structs | Common `DeviceOptions` |
| Backend-specific device factories | `create_device()` / `create_surface_device()` |

### 7.12 Compatibility and migration policy

The final API should be reached without forcing every downstream application to migrate at once:

1. Add the portable replacement alongside the existing API.
2. Route both backends through the replacement.
3. Verify image, picking, capture, overlay, and interaction parity.
4. Deprecate native public APIs with documented replacements.
5. Remove native declarations, definitions, bindings, and build entries in a coordinated release.
6. Keep a compatibility release only where downstream C++ or Python compatibility justifies it.

The essential outcome is **a smaller backend-facing API and a familiar application API**. OpenGL and Vulkan should differ only in private implementations, performance characteristics, and reported capabilities—not in public classes, native handles, or client-side `#ifdef`s.
