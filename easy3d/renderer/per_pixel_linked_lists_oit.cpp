/** \cond LICENSE
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 \endcond */

/**
 * @file per_pixel_linked_lists_oit.cpp
 * @brief Implementation of per-pixel linked lists order-independent transparency.
 *
 * @details This file implements the PerPixelLinkedListsOIT class, which provides perfect
 *      order-independent transparency using per-pixel linked lists. The implementation
 *      follows the algorithm described in:
 *
 *      "Order-Independent Transparency with Per-Pixel Linked Lists"
 *      by Jason C. Yang, Justin Hensley, Holger Grün, Nicolas Thibieroz
 *      NVIDIA Corporation, 2010
 *
 *      **Implementation Details:**
 *
 *      The algorithm uses three GPU buffers:
 *
 *      1. **Head Buffer (SSBO):** An array of int32 values, one per pixel. Each value stores
 *         the index of the first node in that pixel's linked list, or -1 if the list is empty.
 *         Layout: `layout(std430, binding=0) buffer HeadBuffer { int head[]; };`
 *
 *      2. **Node Buffer (SSBO):** A pool of fragment nodes. Each node is 32 bytes (std430):
 *         - vec4 color (16 bytes): premultiplied RGBA color
 *         - float depth (4 bytes): fragment depth value
 *         - int next (4 bytes): index of next node in list, or -1 if end
 *         - (8 bytes implicit padding to align the struct to 16 bytes)
 *         Layout: `layout(std430, binding=1) buffer NodeBuffer { FragmentNode nodes[]; };`
 *         where `struct FragmentNode { vec4 color; float depth; int next; };`
 *
 *      3. **Counter Buffer (Atomic Counter):** A single uint32 value used to allocate node
 *         indices atomically. Reset to 0 at the start of each frame.
 *         Layout: `layout(binding=2) uniform atomic_uint node_counter;`
 *
 *      **Build Pass (per_pixel_linked_lists_oit_build.frag):**
 *
 *      For each transparent fragment:
 *      1. Compute the pixel coordinates: `ivec2 pixel = ivec2(gl_FragCoord.xy);`
 *      2. Compute the pixel index: `int pixel_index = pixel.y * width + pixel.x;`
 *      3. Allocate a node index atomically: `uint node_index = atomicCounterIncrement(node_counter);`
 *      4. Check for overflow: if `node_index >= max_nodes`, discard the fragment.
 *      5. Write node data:
 *         ```glsl
 *         nodes[node_index].color = vec4(color.rgb * alpha, alpha);  // premultiply
 *         nodes[node_index].depth = gl_FragCoord.z;
 *         ```
 *      6. Insert at head of list using atomic compare-and-swap:
 *         ```glsl
 *         int old_head;
 *         do {
 *             old_head = head[pixel_index];
 *             nodes[node_index].next = old_head;
 *         } while (atomicCompSwap(head[pixel_index], old_head, int(node_index)) != old_head);
 *         ```
 *         This loop ensures thread-safe insertion: if another fragment modified the head
 *         between reading it and writing, we retry with the new head value.
 *
 *      **Resolve Pass (per_pixel_linked_lists_oit_resolve.frag):**
 *
 *      For each pixel (fullscreen quad):
 *      1. Read the head index: `int node_index = head[pixel_index];`
 *      2. If `node_index == -1`, the list is empty; output the background color.
 *      3. Traverse the linked list, collecting all fragments into a local array:
 *         ```glsl
 *         FragmentNode fragments[MAX_FRAGMENTS];
 *         int count = 0;
 *         while (node_index != -1 && count < MAX_FRAGMENTS) {
 *             fragments[count++] = nodes[node_index];
 *             node_index = nodes[node_index].next;
 *         }
 *         ```
 *      4. Sort the fragments by depth (front-to-back, i.e., ascending depth):
 *         ```glsl
 *         // Simple insertion sort (efficient for small N)
 *         for (int i = 1; i < count; ++i) {
 *             FragmentNode key = fragments[i];
 *             int j = i - 1;
 *             while (j >= 0 && fragments[j].depth > key.depth) {
 *                 fragments[j + 1] = fragments[j];
 *                 --j;
 *             }
 *             fragments[j + 1] = key;
 *         }
 *         ```
 *         Insertion sort is O(N²) but efficient for small N (≤ 16). For larger N, a more
 *         efficient sort (e.g., quicksort) would be needed, but typical scenes have few
 *         fragments per pixel.
 *
 *      5. Blend the sorted fragments front-to-back using standard alpha compositing:
 *         ```glsl
 *         vec4 result = background_color;
 *         for (int i = 0; i < count; ++i) {
 *             vec4 frag = fragments[i];
 *             // Premultiplied alpha blending: result = frag + result * (1 - frag.a)
 *             result.rgb = frag.rgb + result.rgb * (1.0 - frag.a);
 *             result.a = frag.a + result.a * (1.0 - frag.a);
 *         }
 *         fragOutput = result;
 *         ```
 *         Note: The node colors are stored premultiplied, so we use premultiplied alpha
 *         blending. This is mathematically equivalent to standard alpha blending but
 *         avoids an extra multiplication per fragment.
 *
 *      **Memory Management:**
 *
 *      The node pool size is determined by `max_fragments_` (fragments per pixel) and the
 *      viewport size:
 *      ```cpp
 *      int max_nodes = width * height * max_fragments_;
 *      ```
 *      For a 1920×1080 viewport with max_fragments_ = 8:
 *      - max_nodes = 16,588,800 nodes
 *      - Node buffer size = 16,588,800 × 32 bytes = 505 MB
 *
 *      If the node pool overflows (more fragments than allocated nodes), excess fragments
 *      are silently discarded in the build pass. This can cause visual artifacts (missing
 *      transparency) in very complex scenes. To avoid this, increase `max_fragments_` or
 *      reduce the viewport size.
 *
 *      **Performance Optimization Notes:**
 *
 *      1. **Atomic Contention:** The build pass uses atomic operations for node allocation
 *         and list insertion. High fragment density causes contention, reducing performance.
 *         Mitigation: Use a larger node pool to reduce contention, or tile the viewport and
 *         use per-tile counters (not implemented here for simplicity).
 *
 *      2. **Memory Bandwidth:** The resolve pass reads linked list nodes from global memory.
 *         This is bandwidth-limited. Mitigation: Use a compute shader with shared memory for
 *         sorting (not implemented here; fragment shader is simpler and sufficient for
 *         offline rendering).
 *
 *      3. **Sorting Efficiency:** Insertion sort is used for simplicity. For scenes with many
 *         fragments per pixel (> 16), a more efficient sort (e.g., bitonic sort in a compute
 *         shader) would improve performance. For typical scenes (≤ 8 fragments/pixel),
 *         insertion sort is fast enough.
 *
 *      4. **Overflow Handling:** The current implementation silently discards overflow
 *         fragments. A more robust implementation could:
 *         - Use a secondary overflow buffer
 *         - Render a warning when overflow occurs
 *         - Dynamically resize the node pool (not possible with fixed-size SSBOs)
 *
 *      **OpenGL Version Requirements:**
 *
 *      This implementation requires OpenGL 4.3+ for:
 *      - Shader Storage Buffer Objects (SSBOs): `layout(std430, binding=N) buffer`
 *      - Atomic counter operations: `atomicCounterIncrement`, `atomicCompSwap`
 *      - Image load/store (not used here, but related feature)
 *
 *      OpenGL 3.2 (used by other parts of Easy3D) does not support SSBOs. If the OpenGL
 *      context is < 4.3, this class will fail to initialize. In that case, fall back to
 *      DualDepthPeeling or AverageColorBlending.
 */

#ifdef EASY3D_HAS_VULKAN
// The Vulkan bridge is header-only (see per_pixel_linked_lists_oit.h): draw()
// routes to the render device via Transparency::render_frame(method=3), which
// builds the per-pixel fragment lists in storage buffers and resolves them.
#else

#include <easy3d/renderer/per_pixel_linked_lists_oit.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/setting.h>


namespace easy3d {


PerPixelLinkedListsOIT::PerPixelLinkedListsOIT(Camera* cam)
    : camera_(cam)
    , max_fragments_(8)
    , head_buffer_(0)
    , node_buffer_(0)
    , counter_buffer_(0)
    , fbo_(nullptr)
    , width_(0)
    , height_(0)
    , node_count_(0)
{
}


PerPixelLinkedListsOIT::~PerPixelLinkedListsOIT()
{
    clear();
}


void PerPixelLinkedListsOIT::draw(const std::vector<TrianglesDrawable*>& surfaces)
{
    if (!camera_)
        return;

    int w = camera_->screen_width();
    int h = camera_->screen_height();
    if (w <= 0 || h <= 0)
        return;

    // Check OpenGL version (require 4.3+ for SSBOs and atomic operations).
    // macOS caps at OpenGL 4.1, so this feature is not available on macOS OpenGL.
    // Users on macOS should use the Vulkan backend (via MoltenVK) or DualDepthPeeling.
    const char* version_str = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    if (!version_str) {
        LOG(ERROR) << "PerPixelLinkedListsOIT: cannot query OpenGL version";
        return;
    }
    int major = 0, minor = 0;
    sscanf(version_str, "%d.%d", &major, &minor);
    if (major < 4 || (major == 4 && minor < 3)) {
        LOG_N_TIMES(3, ERROR) << "PerPixelLinkedListsOIT: requires OpenGL 4.3+, current version is "
                   << major << "." << minor << ". "
#ifdef __APPLE__
                   << "macOS OpenGL is limited to 4.1. Use the Vulkan backend (MoltenVK) "
                   << "or DualDepthPeeling for transparency.";
#else
                   << "Update your OpenGL driver or use DualDepthPeeling.";
#endif

        return;
    }

    if (w != width_ || h != height_)
        init(w, h);

    // Build pass: render transparent fragments into per-pixel linked lists.
    build_lists(surfaces);

    // Resolve pass: sort and blend the linked lists to produce the final image.
    resolve();
}


void PerPixelLinkedListsOIT::init(int w, int h)
{
    if (head_buffer_ || node_buffer_ || counter_buffer_ || fbo_)
        clear();

    width_ = w;
    height_ = h;
    node_count_ = width_ * height_ * max_fragments_;

    // std430 struct stride: vec4(16) + float(4) + int(4) + padding(8) = 32 bytes.
    const GLsizeiptr node_buffer_size = static_cast<GLsizeiptr>(node_count_) * 32;

    LOG(INFO) << "PerPixelLinkedListsOIT::init: viewport " << width_ << "x" << height_
              << ", max_nodes=" << node_count_ << " ("
              << static_cast<double>(node_buffer_size) / (1024.0 * 1024.0) << " MB)";

    // Head buffer (SSBO): one int32 per pixel, initialized to -1 (empty list).
    glGenBuffers(1, &head_buffer_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, head_buffer_);
    {
        std::vector<int> head_data(width_ * height_, -1);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
                     static_cast<GLsizeiptr>(head_data.size() * sizeof(int)),
                     head_data.data(), GL_DYNAMIC_DRAW);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    easy3d_debug_log_gl_error

    // Node buffer (SSBO): pool of FragmentNode structs (32 bytes each).
    glGenBuffers(1, &node_buffer_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, node_buffer_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, node_buffer_size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    easy3d_debug_log_gl_error

    // Atomic counter buffer: single uint32, initialized to 0.
    glGenBuffers(1, &counter_buffer_);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, counter_buffer_);
    {
        unsigned int zero = 0;
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(unsigned int), &zero, GL_DYNAMIC_DRAW);
    }
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    easy3d_debug_log_gl_error

    // FBO for the build pass. The build shader writes fragment data to SSBOs;
    // the color attachment receives a dummy output to keep the FBO complete.
    fbo_ = new FramebufferObject(width_, height_, 0);
    fbo_->add_color_buffer();
    fbo_->add_depth_buffer();
    easy3d_debug_log_gl_error
    easy3d_debug_log_frame_buffer_error
}


void PerPixelLinkedListsOIT::build_lists(const std::vector<TrianglesDrawable*>& surfaces)
{
    if (!fbo_)
        return;

    fbo_->bind();
    easy3d_debug_log_gl_error

    // Clear the head buffer to -1 using glClearBufferData (GL 4.3+, much faster
    // than uploading from CPU for large viewports).
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, head_buffer_);
    {
        GLint clear_value = -1;
        glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32I, GL_RED_INTEGER, GL_INT, &clear_value);
    }
    easy3d_debug_log_gl_error

    // Reset the atomic counter to 0.
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, counter_buffer_);
    {
        GLuint zero = 0;
        glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);
    }
    easy3d_debug_log_gl_error

    // Retrieve or create the build shader program.
    static const std::string build_shader_name = "transparency/per_pixel_linked_lists_oit_build";
    ShaderProgram* program = ShaderManager::get_program(build_shader_name);
    if (!program) {
        std::vector<ShaderProgram::Attribute> attributes = {
            ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
            ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"),
            ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"),
            ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord")
        };
        program = ShaderManager::create_program_from_files(build_shader_name, attributes);
    }
    if (!program) {
        LOG(ERROR) << "PerPixelLinkedListsOIT: failed to create build shader";
        fbo_->release();
        return;
    }

    // Camera and lighting (shared across all drawables).
    const mat4& MVP = camera_->model_view_projection_matrix();
    const vec3& wCamPos = camera_->position();
    const mat4& MV = camera_->model_view_matrix();
    const vec4& wLightPos = inverse(MV) * setting::light_position;

    program->bind();
    program->set_uniform("wLightPos", wLightPos);
    program->set_uniform("wCamPos", wCamPos);
    program->set_uniform("viewport_width", width_);
    program->set_uniform("viewport_height", height_);
    program->set_uniform("max_nodes", node_count_);
    easy3d_debug_log_gl_error

    // Bind SSBOs and atomic counter. Binding points must match the shader layouts:
    //   SSBO binding 0: head buffer
    //   SSBO binding 1: node buffer
    //   Atomic counter binding 0: node counter
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, head_buffer_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, node_buffer_);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, counter_buffer_);
    easy3d_debug_log_gl_error

    // Depth test disabled: we capture all transparent fragments regardless of
    // depth ordering. Blending disabled: fragment data goes to SSBOs, not the
    // framebuffer color. Color mask disabled to skip the dummy framebuffer write.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    easy3d_debug_log_gl_error

    for (auto* d : surfaces) {
        if (!d || !d->is_visible())
            continue;

        const mat4 MANIP = d->manipulated_matrix();
        const mat3 NORMAL = transform::normal_matrix(MANIP);

        program->set_uniform("MANIP", MANIP);
        program->set_uniform("MVP_MANIP", MVP * MANIP);
        program->set_uniform("NORMAL", NORMAL);
        program->set_uniform("Alpha", d->opacity());
        program->set_uniform("lighting", d->lighting());
        program->set_uniform("smooth_shading", d->smooth_shading());
        program->set_uniform("two_sides_lighting", d->lighting_two_sides());
        program->set_uniform("distinct_back_color", d->distinct_back_color());
        program->set_uniform("backside_color", d->back_color());
        program->set_block_uniform("Material", "ambient", d->material().ambient);
        program->set_block_uniform("Material", "specular", d->material().specular);
        program->set_block_uniform("Material", "shininess", &d->material().shininess);
        program->set_uniform("per_vertex_color",
                             d->coloring_method() != State::UNIFORM_COLOR && d->color_buffer());
        program->set_uniform("default_color", d->color());
        program->set_uniform("selected", d->is_selected());
        program->set_uniform("highlight_color", setting::highlight_color);

        ClippingPlane::instance()->set_program(program);
        ClippingPlane::instance()->set_discard_primitives(program, d->plane_clip_discard_primitive());

        bool use_texture = (d->texture() &&
                            (d->coloring_method() == State::SCALAR_FIELD ||
                             d->coloring_method() == State::TEXTURED));
        program->set_uniform("use_texture", use_texture);
        if (use_texture) {
            program->bind_texture("textureID", d->texture()->id(), 0);
            program->set_uniform("texture_repeat", d->texture_repeat());
            program->set_uniform("fractional_repeat", d->texture_fractional_repeat());
        }
        easy3d_debug_log_gl_error

        d->gl_draw();
        easy3d_debug_log_gl_error

        if (use_texture)
            program->release_texture();
    }

    // Restore state.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    program->release();
    fbo_->release();
    easy3d_debug_log_gl_error

    // Read back the atomic counter to check for pool overflow.
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, counter_buffer_);
    auto* counter_ptr = reinterpret_cast<unsigned int*>(
        glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_ONLY));
    if (counter_ptr) {
        unsigned int allocated = *counter_ptr;
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
        if (allocated >= static_cast<unsigned int>(node_count_)) {
            LOG(WARNING) << "PerPixelLinkedListsOIT: node pool overflow! "
                         << allocated << " nodes allocated, "
                         << node_count_ << " available. "
                         << "Increase max_fragments_per_pixel or reduce viewport size.";
        }
        LOG(INFO) << "PerPixelLinkedListsOIT: allocated " << allocated << " nodes ("
                  << (100.0f * allocated / node_count_) << "% of pool)";
    }
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    easy3d_debug_log_gl_error
}


void PerPixelLinkedListsOIT::resolve()
{
    // The resolve pass renders a fullscreen quad to the currently bound framebuffer
    // (the default framebuffer / screen). It reads the per-pixel linked lists from
    // the SSBOs, sorts fragments by depth, and blends them over the background.

    static const std::string resolve_shader_name = "transparency/per_pixel_linked_lists_oit_resolve";
    ShaderProgram* program = ShaderManager::get_program(resolve_shader_name);
    if (!program) {
        std::vector<ShaderProgram::Attribute> attributes = {
            ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
            ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord")
        };
        program = ShaderManager::create_program_from_files(resolve_shader_name, attributes);
    }
    if (!program) {
        LOG(ERROR) << "PerPixelLinkedListsOIT: failed to create resolve shader";
        return;
    }

    program->bind();

    // Bind SSBOs (same binding points as the build shader).
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, head_buffer_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, node_buffer_);
    easy3d_debug_log_gl_error

    program->set_uniform("viewport_width", width_);
    program->set_uniform("viewport_height", height_);
    // TODO: integrate with the viewer's opaque scene background.
    program->set_uniform("background_color", vec4(1.0f, 1.0f, 1.0f, 1.0f));
    easy3d_debug_log_gl_error

    // Enable depth test and disable blending for the fullscreen quad.
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    shape::draw_full_screen_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, 0.0f);
    easy3d_debug_log_gl_error

    program->release();
}


void PerPixelLinkedListsOIT::clear()
{
    if (head_buffer_) {
        glDeleteBuffers(1, &head_buffer_);
        head_buffer_ = 0;
    }
    if (node_buffer_) {
        glDeleteBuffers(1, &node_buffer_);
        node_buffer_ = 0;
    }
    if (counter_buffer_) {
        glDeleteBuffers(1, &counter_buffer_);
        counter_buffer_ = 0;
    }
    if (fbo_) {
        delete fbo_;
        fbo_ = nullptr;
    }
    width_ = 0;
    height_ = 0;
    node_count_ = 0;
    easy3d_debug_log_gl_error
}


} // namespace easy3d

#endif // EASY3D_HAS_VULKAN
