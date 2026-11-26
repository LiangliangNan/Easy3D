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

The non-Vulkan renderer source list is defined in [easy3d/renderer/CMakeLists.txt:89](easy3d/renderer/CMakeLists.txt#L89). The caller inventory counted actual calls and excluded comments, documentation, declarations, and `#if 0` examples.

Terms used in this report:

- **Unused in-tree:** no caller in Easy3D production code, tests, applications, tutorials, or bindings outside the symbol's own definition.
- **Python-only:** no C++ caller, but exposed through Python bindings.
- **Tests-only:** no production caller, but exercised by tests.
- **Required:** used by live rendering, exact fallback behavior, effects, or the new shared-device adapter.

“Unused in-tree” does not prove that downstream users do not depend on an API. The public include tree is installed by [CMakeLists.txt:357](CMakeLists.txt#L357).

## Current architectural boundary

The shared backend-neutral pieces are:

- `graphics::RenderDevice`: offscreen rendering, capture metadata, picking, readback, and selection.
- `graphics::DrawCommand`: backend-neutral draw description.
- `graphics::DrawableRenderer`: converts a `Drawable` into shared commands.
- `graphics::OpenGLDevice`: headless OpenGL realization borrowing the caller's current context.

The OpenGL `Viewer` still renders its live scene through native drawable implementations. The decisive dependency is [easy3d/viewer/viewer.cpp:2485](easy3d/viewer/viewer.cpp#L2485), which calls:

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
| `easy3d::opengl::read_pixel` | No callers outside `read_pixel.{h,cpp}`; no tests or active Python bindings. This includes single-pixel and buffer `read_color`/`read_depth`, MSAA variants, and color/depth snapshot functions. See [easy3d/renderer/read_pixel.h:52](easy3d/renderer/read_pixel.h#L52). | Remove declarations, definitions, and CMake entries if public API breaks are acceptable. Otherwise deprecate first. |
| `easy3d::OpenGLTimer` | No callers outside its own header/source; no tests or bindings. See [easy3d/renderer/opengl_timer.h:53](easy3d/renderer/opengl_timer.h#L53). | Same treatment as `read_pixel`. |

Both are listed in the OpenGL-only headers and sources at [easy3d/renderer/CMakeLists.txt:92](easy3d/renderer/CMakeLists.txt#L92). Removing them requires no rendering adaptation.

### 1.2 Public members with no active in-tree callers

The following are cleanup candidates. Public API policy still applies.

| Component | Unused surface | Notes |
|---|---|---|
| `ShaderManager` | `create_program_from_codes()`, `all_programs()`, and the individual-file `create_program_from_files()` overload | The base-name creation overload, `get_program()`, `reload()`, and `terminate()` remain used. See [easy3d/renderer/shader_manager.h:82](easy3d/renderer/shader_manager.h#L82). |
| `ShaderProgram` | `set_verbose()`, `program_output()`, `is_program_valid()`, `load_binary()`, `save_binary()` | These belong to diagnostics/binary-program support with no external callers. `set_program_output()` remains used by `ShaderManager`. See [easy3d/renderer/shader_program.h:128](easy3d/renderer/shader_program.h#L128). |
| `VertexArrayObject` | `is_supported()`, `create_storage_buffer()`, `update_storage_buffer()`, `map_buffer()`, `unmap_buffer()` | Core array/element buffer creation and `get_buffer_data()` remain required. See [easy3d/renderer/vertex_array_object.h:58](easy3d/renderer/vertex_array_object.h#L58). |
| `FramebufferObject` | `is_supported()`, `attach_color_texture()`, `copy_color_to_texture()`, `copy_depth_to_texture()`, `is_bound()`, `num_color_attachments()`, `has_color_attachment()`, `has_depth_attachment()`, `depth_bits()`, `has_stencil()`, `texture_target()`, `has_color_texture()`, `has_depth_texture()`, `deactivate_read_buffer()`, and all `blit_framebuffer()` overloads | `attach_depth_texture()` is used by `OpenGLDevice`; draw/read-buffer activation and color readback remain used. See [easy3d/renderer/framebuffer_object.h:128](easy3d/renderer/framebuffer_object.h#L128). |
| `FramebufferObject` depth helpers | Buffer `read_depth()` overloads and `snapshot_depth()` | The vector `read_depth()` overload is tests-only; the pointer overload has no caller. `snapshot_depth()` appears only in disabled code. Single-pixel `read_depth()` remains production-used. |
| `FramebufferObject` diagnostics | `print_attachments()`, `print_draw_buffers()`, `print_read_buffer()` | Referenced only from disabled debug code. |
| `OpenglUtil` | Profile, vendor, renderer, version, extension, GLSL/GLEW, sample, viewport, GPU-memory, output-stream, and `get_*_info` queries | Only `init()`, `is_supported()`, and `has_extension()` have production callers. See [easy3d/renderer/opengl_util.h:47](easy3d/renderer/opengl_util.h#L47). |
| OpenGL error helpers | `check_gl_error()`, `check_frame_buffer_error()`, `easy3d_gl_error`, and `easy3d_frame_buffer_error` | The logging variants remain used throughout production code. See [easy3d/renderer/opengl_error.h:38](easy3d/renderer/opengl_error.h#L38). |
| Native `shape` functions | Textured `draw_quad_filled()` and textured `draw_full_screen_quad()` | The bound-shader fullscreen-quad overload is heavily used. See [easy3d/renderer/shape.h:84](easy3d/renderer/shape.h#L84). |
| `Texture` | `bind()` and `release()` | Tests-only; production rendering uses texture IDs through shader binding. Public API nonetheless. See [easy3d/renderer/texture.h:119](easy3d/renderer/texture.h#L119). |

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

The bindings are at [python/bindings/easy3d/renderer/drawables.cpp:194](python/bindings/easy3d/renderer/drawables.cpp#L194) and [python/bindings/easy3d/renderer/drawables.cpp:279](python/bindings/easy3d/renderer/drawables.cpp#L279).

`Drawable::vertex_buffer()`, `color_buffer()`, and `normal_buffer()` also have production callers in effects, native shape drawing, and Mapple-Qt. They are not deletion candidates today.

Recommended treatment for Python-only methods:

1. Introduce backend-neutral predicates or resource queries.
2. Deprecate the raw GL accessors.
3. Remove the Python bindings only in a coordinated API release.

## 3. Components that remain required

| Component | Production dependencies | Conclusion |
|---|---|---|
| `FramebufferObject` | Viewer capture and fallbacks; `MultiViewer`; GUI pickers; AO, EDL, shadows, transparency; `OverlayRenderer`; `OpenGLDevice` | Required. The adapter itself allocates targets with it at [easy3d/renderer/backends/opengl/render_device.cpp:342](easy3d/renderer/backends/opengl/render_device.cpp#L342). |
| `ShaderProgram` / `ShaderManager` | Native drawables, corner axes, picking fallbacks, overlays, and effects | Required. Viewer uses the manager directly at [easy3d/viewer/viewer.cpp:2335](easy3d/viewer/viewer.cpp#L2335). |
| `Texture` / `TextureManager` | Native textured/scalar-field rendering; material handling; shared command extraction; Mapple UI and tutorials | Required. The shared device consumes CPU pixel data but does not replace live texture realization. |
| `VertexArrayObject` | Native buffer creation, element buffers, `Drawable::buffer_data()` readback, screen-space lines, shape drawing | Required. |
| Native `Drawable` storage | Live rendering and CPU snapshots used by `DrawableRenderer`/`OpenGLDevice` | Required. |
| `ClippingPlane` | Viewer controls, native shader setup, clipping visualization, pickers, effects, Mapple, tutorials | Required. See [easy3d/viewer/viewer.cpp:2535](easy3d/viewer/viewer.cpp#L2535). |
| Shadow/AO/EDL/transparency | Viewer integration, Mapple, and rendering tutorials | Required as application/library functionality, even where base Viewer does not call every effect directly. |
| `opengl::OverlayRenderer` | HUD text and client overlays, including depth-tested overlays | Required today. See [easy3d/viewer/viewer.cpp:2423](easy3d/viewer/viewer.cpp#L2423). |
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

This is currently blocked by the headless contract: `OpenGLDevice::present()` throws at [easy3d/renderer/backends/opengl/render_device.cpp:950](easy3d/renderer/backends/opengl/render_device.cpp#L950).

**Assessment:** coherent future migration, but not a simple caller replacement.

### 4.2 Native picking and depth fallbacks

The old FBO/shader fallback blocks and `Picker::fbo_` can be removed incrementally as shared-device support expands to:

- Enabled clipping.
- AO and non-opaque drawables.
- Unsupported coloring or unavailable CPU texture data.
- Point/line impostors.
- Non-native line widths.
- `MultiViewer`.

The capability gate is visible at [easy3d/viewer/viewer.cpp:2567](easy3d/viewer/viewer.cpp#L2567).

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
