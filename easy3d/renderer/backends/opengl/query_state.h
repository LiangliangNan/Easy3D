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


#ifndef EASY3D_RENDERER_BACKENDS_OPENGL_QUERY_STATE_H
#define EASY3D_RENDERER_BACKENDS_OPENGL_QUERY_STATE_H

#include <easy3d/renderer/opengl.h>
#include <vector>
namespace easy3d {
    namespace opengl {
        // Preserve caller state around scene queries and annotation rendering.
        // The caller chooses the render target; camera/drawable settings are untouched.
        struct QueryState {
            static const unsigned flag_count = 20;
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
                                              GL_SAMPLE_MASK,
                                              GL_FRAMEBUFFER_SRGB,
                                              GL_DITHER,
                                              GL_COLOR_LOGIC_OP,
                                              GL_MULTISAMPLE};
            GLboolean enabled[flag_count], depth_mask, color_mask[4];
            GLint viewport[4], draw_fbo, read_fbo, renderbuffer, program, vao, array, element, active, uniform,
                pack_buffer, depth_func, unpack_buffer;
            GLint blend[6], textures[2][2], polygon_mode[2], front_face;
            std::vector<GLboolean> clip_distances;
            static const unsigned pack_count = 8;
            const GLenum pack_names[pack_count] = {GL_PACK_ALIGNMENT,  GL_PACK_ROW_LENGTH,   GL_PACK_SKIP_PIXELS,
                                                   GL_PACK_SKIP_ROWS,  GL_PACK_IMAGE_HEIGHT, GL_PACK_SKIP_IMAGES,
                                                   GL_PACK_SWAP_BYTES, GL_PACK_LSB_FIRST};
            GLint pack[pack_count], unpack[pack_count];
            const GLenum unpack_names[pack_count] = {
                GL_UNPACK_ALIGNMENT,    GL_UNPACK_ROW_LENGTH,  GL_UNPACK_SKIP_PIXELS, GL_UNPACK_SKIP_ROWS,
                GL_UNPACK_IMAGE_HEIGHT, GL_UNPACK_SKIP_IMAGES, GL_UNPACK_SWAP_BYTES,  GL_UNPACK_LSB_FIRST};
            GLdouble clear_depth, depth_range[2];
            GLfloat point_size, line_width, clear_color[4];
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
                glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &unpack_buffer);
                glGetBooleanv(GL_DEPTH_WRITEMASK, &depth_mask);
                glGetBooleanv(GL_COLOR_WRITEMASK, color_mask);
                glGetDoublev(GL_DEPTH_CLEAR_VALUE, &clear_depth);
                glGetDoublev(GL_DEPTH_RANGE, depth_range);
                glGetIntegerv(GL_POLYGON_MODE, polygon_mode);
                glGetIntegerv(GL_FRONT_FACE, &front_face);
                GLint clip_count = 0;
                glGetIntegerv(GL_MAX_CLIP_DISTANCES, &clip_count);
                clip_distances.resize(size_t(clip_count));
                for (unsigned i = 0; i < clip_distances.size(); ++i)
                    clip_distances[i] = glIsEnabled(GL_CLIP_DISTANCE0 + i);
                glGetFloatv(GL_COLOR_CLEAR_VALUE, clear_color);
                glGetFloatv(GL_POINT_SIZE, &point_size);
                glGetFloatv(GL_LINE_WIDTH, &line_width);
                const GLenum blend_names[6] = {GL_BLEND_SRC_RGB,   GL_BLEND_DST_RGB,      GL_BLEND_SRC_ALPHA,
                                               GL_BLEND_DST_ALPHA, GL_BLEND_EQUATION_RGB, GL_BLEND_EQUATION_ALPHA};
                for (unsigned i = 0; i < 6; ++i)
                    glGetIntegerv(blend_names[i], &blend[i]);
                for (unsigned i = 0; i < pack_count; ++i)
                    glGetIntegerv(pack_names[i], &pack[i]);
                for (unsigned i = 0; i < pack_count; ++i)
                    glGetIntegerv(unpack_names[i], &unpack[i]);
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
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpack_buffer);
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
                for (unsigned i = 0; i < pack_count; ++i)
                    glPixelStorei(unpack_names[i], unpack[i]);
                glDepthFunc(depth_func);
                glDepthMask(depth_mask);
                glColorMask(color_mask[0], color_mask[1], color_mask[2], color_mask[3]);
                glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
                glClearDepth(clear_depth);
                glDepthRange(depth_range[0], depth_range[1]);
                glPointSize(point_size);
                glLineWidth(line_width);
                glPolygonMode(GL_FRONT_AND_BACK, polygon_mode[0]);
                glFrontFace(front_face);
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
                glFrontFace(GL_CCW);
                glEnable(GL_MULTISAMPLE);
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);
                glDepthMask(GL_TRUE);
                glDepthRange(0, 1);
                glClearDepth(1);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                glViewport(0, 0, w, h);
                glActiveTexture(GL_TEXTURE0);
                glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                for (unsigned i = 0; i < pack_count; ++i)
                    glPixelStorei(pack_names[i], i == 0 ? 1 : 0);
                for (unsigned i = 0; i < pack_count; ++i)
                    glPixelStorei(unpack_names[i], i == 0 ? 1 : 0);
            }
        };
    } // namespace opengl
} // namespace easy3d
#endif
