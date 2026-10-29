// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/backends/viewer.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/core/model.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <stdexcept>
namespace easy3d {
namespace detail {
std::unique_ptr<ViewerBase> create_opengl_viewer(const std::string &title, const ViewerOptions &o) {
    if (o.validation || o.synchronization_validation)
        throw BackendUnavailable("OpenGL viewer does not support the requested validation mode");
    std::unique_ptr<ViewerBase> host(
        new Viewer(title, o.samples == 1 ? 0 : int(o.samples), 3, 2, false, o.resizable, 24, 8, o.width, o.height));
    if (host->samples() != o.samples)
        throw BackendUnavailable("OpenGL viewer does not support the requested MSAA sample count");
    return host;
}
} // namespace detail
} // namespace easy3d

namespace easy3d {
namespace {
struct QueryContext {
    GLFWwindow *previous;
    explicit QueryContext(GLFWwindow *window) : previous(glfwGetCurrentContext()) { glfwMakeContextCurrent(window); }
    ~QueryContext() { glfwMakeContextCurrent(previous); }
};
// Isolate the readback from HUD state, offscreen bindings and caller pixel-pack
// buffers. No default framebuffer contents, camera or drawable settings change.
struct QueryState {
    static const unsigned flag_count = 16;
    const GLenum flags[flag_count] = {GL_DEPTH_TEST,
                                      GL_BLEND,
                                      GL_SCISSOR_TEST,
                                      GL_STENCIL_TEST,
                                      GL_CULL_FACE,
                                      GL_RASTERIZER_DISCARD,
                                      GL_POLYGON_OFFSET_FILL,
                                      GL_POLYGON_OFFSET_LINE,
                                      GL_POLYGON_OFFSET_POINT,
                                      GL_PROGRAM_POINT_SIZE,
                                      GL_DEPTH_CLAMP,
                                      GL_PRIMITIVE_RESTART,
                                      GL_POLYGON_SMOOTH,
                                      GL_SAMPLE_ALPHA_TO_COVERAGE,
                                      GL_SAMPLE_COVERAGE,
                                      GL_SAMPLE_MASK};
    GLboolean enabled[flag_count], depth_mask, color_mask[4];
    GLint viewport[4], draw_fbo, read_fbo, renderbuffer, program, vao, array, element, active, uniform, pack_buffer,
        depth_func;
    GLint blend[6], textures[2][2], polygon_mode[2];
    std::vector<GLboolean> clip_distances;
    static const unsigned pack_count = 8;
    const GLenum pack_names[pack_count] = {GL_PACK_ALIGNMENT,  GL_PACK_ROW_LENGTH,   GL_PACK_SKIP_PIXELS,
                                           GL_PACK_SKIP_ROWS,  GL_PACK_IMAGE_HEIGHT, GL_PACK_SKIP_IMAGES,
                                           GL_PACK_SWAP_BYTES, GL_PACK_LSB_FIRST};
    GLint pack[pack_count];
    GLdouble clear_depth, depth_range[2];
    GLfloat point_size, line_width;
    struct UniformBinding {
        GLint name;
        GLint64 start, size;
    };
    std::vector<UniformBinding> blocks;
    QueryState() {
        for (unsigned i = 0; i < flag_count; ++i)
            enabled[i] = glIsEnabled(flags[i]);
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_fbo);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_fbo);
        glGetIntegerv(GL_RENDERBUFFER_BINDING, &renderbuffer);
        glGetIntegerv(GL_CURRENT_PROGRAM, &program);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &array);
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &element);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
        glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &uniform);
        glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &pack_buffer);
        glGetIntegerv(GL_DEPTH_FUNC, &depth_func);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depth_mask);
        glGetBooleanv(GL_COLOR_WRITEMASK, color_mask);
        glGetDoublev(GL_DEPTH_CLEAR_VALUE, &clear_depth);
        glGetDoublev(GL_DEPTH_RANGE, depth_range);
        glGetIntegerv(GL_POLYGON_MODE, polygon_mode);
        GLint clip_count = 0;
        glGetIntegerv(GL_MAX_CLIP_DISTANCES, &clip_count);
        clip_distances.resize(size_t(clip_count));
        for (unsigned i = 0; i < clip_distances.size(); ++i)
            clip_distances[i] = glIsEnabled(GL_CLIP_DISTANCE0 + i);
        glGetFloatv(GL_POINT_SIZE, &point_size);
        glGetFloatv(GL_LINE_WIDTH, &line_width);
        const GLenum blend_names[6] = {GL_BLEND_SRC_RGB,   GL_BLEND_DST_RGB,      GL_BLEND_SRC_ALPHA,
                                       GL_BLEND_DST_ALPHA, GL_BLEND_EQUATION_RGB, GL_BLEND_EQUATION_ALPHA};
        for (unsigned i = 0; i < 6; ++i)
            glGetIntegerv(blend_names[i], &blend[i]);
        for (unsigned i = 0; i < pack_count; ++i)
            glGetIntegerv(pack_names[i], &pack[i]);
        for (unsigned unit = 0; unit < 2; ++unit) {
            glActiveTexture(GL_TEXTURE0 + unit);
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &textures[unit][0]);
            glGetIntegerv(GL_TEXTURE_BINDING_BUFFER, &textures[unit][1]);
        }
        glActiveTexture(active);
        GLint count = 0;
        glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &count);
        blocks.resize(size_t(count));
        for (GLuint i = 0; i < blocks.size(); ++i) {
            glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, i, &blocks[i].name);
            glGetInteger64i_v(GL_UNIFORM_BUFFER_START, i, &blocks[i].start);
            glGetInteger64i_v(GL_UNIFORM_BUFFER_SIZE, i, &blocks[i].size);
        }
    }
    ~QueryState() {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_fbo);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, read_fbo);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glUseProgram(program);
        glBindVertexArray(vao);
        if (vao)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element);
        glBindBuffer(GL_ARRAY_BUFFER, array);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pack_buffer);
        for (GLuint i = 0; i < blocks.size(); ++i) {
            const auto &b = blocks[i];
            if (b.name && b.size > 0)
                glBindBufferRange(GL_UNIFORM_BUFFER, i, b.name, b.start, b.size);
            else
                glBindBufferBase(GL_UNIFORM_BUFFER, i, b.name);
        }
        glBindBuffer(GL_UNIFORM_BUFFER, uniform);
        for (unsigned unit = 0; unit < 2; ++unit) {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, textures[unit][0]);
            glBindTexture(GL_TEXTURE_BUFFER, textures[unit][1]);
        }
        glActiveTexture(active);
        for (unsigned i = 0; i < pack_count; ++i)
            glPixelStorei(pack_names[i], pack[i]);
        glDepthFunc(depth_func);
        glDepthMask(depth_mask);
        glColorMask(color_mask[0], color_mask[1], color_mask[2], color_mask[3]);
        glClearDepth(clear_depth);
        glDepthRange(depth_range[0], depth_range[1]);
        glPointSize(point_size);
        glLineWidth(line_width);
        glPolygonMode(GL_FRONT_AND_BACK, polygon_mode[0]);
        for (unsigned i = 0; i < clip_distances.size(); ++i) {
            if (clip_distances[i])
                glEnable(GL_CLIP_DISTANCE0 + i);
            else
                glDisable(GL_CLIP_DISTANCE0 + i);
        }
        glBlendFuncSeparate(blend[0], blend[1], blend[2], blend[3]);
        glBlendEquationSeparate(blend[4], blend[5]);
        for (unsigned i = 0; i < flag_count; ++i) {
            if (enabled[i])
                glEnable(flags[i]);
            else
                glDisable(flags[i]);
        }
    }
    void prepare(int w, int h) {
        for (unsigned i = 0; i < flag_count; ++i)
            glDisable(flags[i]);
        for (unsigned i = 0; i < clip_distances.size(); ++i)
            glDisable(GL_CLIP_DISTANCE0 + i);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDepthRange(0, 1);
        glClearDepth(1);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glViewport(0, 0, w, h);
        glActiveTexture(GL_TEXTURE0);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        for (unsigned i = 0; i < pack_count; ++i)
            glPixelStorei(pack_names[i], i == 0 ? 1 : 0);
    }
};
void query_draw(const Drawable *drawable, const Camera &camera) {
    if (!drawable || !drawable->is_visible() || dynamic_cast<const LinesDrawable2D *>(drawable))
        return;
    // Standard drawable rendering only, matching the native scene encoder.
    // Custom Viewer::draw() and application frame hooks do not run here.
    if (auto *d = dynamic_cast<const TrianglesDrawable *>(drawable))
        d->TrianglesDrawable::draw(&camera);
    else if (auto *d = dynamic_cast<const LinesDrawable *>(drawable))
        d->LinesDrawable::draw(&camera);
    else if (auto *d = dynamic_cast<const PointsDrawable *>(drawable))
        d->PointsDrawable::draw(&camera);
    else
        throw std::invalid_argument("unsupported drawable in scene depth query");
}
} // namespace
bool Viewer::read_scene_depth(int x, int y, const Camera &snapshot, float &depth) const {
    QueryContext context(window_);
    QueryState state;
    state.prepare(snapshot.screenWidth(), snapshot.screenHeight());
    FramebufferObject target(snapshot.screenWidth(), snapshot.screenHeight(), 0);
    if (!target.add_color_buffer() || !target.add_depth_buffer(GL_DEPTH_COMPONENT32F) || !target.is_valid())
        throw std::runtime_error("cannot create single-sample scene depth framebuffer");
    if (!target.bind())
        throw std::runtime_error("cannot bind scene depth framebuffer");
    glClear(GL_DEPTH_BUFFER_BIT);
    for (const auto &model : models_) {
        const auto *renderer = model->renderer();
        if (!renderer || !renderer->is_visible())
            continue;
        for (const auto &d : renderer->triangles_drawables())
            query_draw(d.get(), snapshot);
        for (const auto &d : renderer->lines_drawables())
            query_draw(d.get(), snapshot);
        for (const auto &d : renderer->points_drawables())
            query_draw(d.get(), snapshot);
    }
    for (const auto &drawable : drawables_)
        query_draw(drawable.get(), snapshot);
    glReadPixels(x, snapshot.screenHeight() - 1 - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    return true;
}
} // namespace easy3d
