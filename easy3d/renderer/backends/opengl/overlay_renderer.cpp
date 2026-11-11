// SPDX-License-Identifier: GPL-3.0-or-later
/********************************************************************
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
 ********************************************************************/
#include "overlay_renderer.h"
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/shader_program.h>
#include <stdexcept>
#include <cstddef>
#include <cmath>
namespace easy3d {
    namespace opengl {
        OverlayRenderer::~OverlayRenderer() {
            clear();
        }
        void OverlayRenderer::clear() {
            for (auto &d : draws_) {
                glDeleteVertexArrays(1, &d.vao);
                glDeleteBuffers(1, &d.vertices);
                glDeleteBuffers(1, &d.indices);
                glDeleteTextures(1, &d.texture);
            }
            draws_.clear();
        }
        void OverlayRenderer::set(std::vector<graphics::DrawCommand> commands, bool foreground) {
            for (const auto &c : commands)
                if (c.geometry &&
                    (c.geometry->topology() != graphics::Topology::triangles ||
                     (c.overlay_independent_depth && c.overlay_depth_test) ||
                     (c.scene_depth_texture && (c.texture || c.overlay_depth_test || c.overlay_independent_depth))))
                    throw std::invalid_argument("annotations require triangles and compatible depth modes");
            bool same = commands.size() == draws_.size();
            for (size_t i = 0; same && i < commands.size(); ++i)
                same = commands[i].geometry == draws_[i].command.geometry &&
                       commands[i].texture == draws_[i].command.texture;
            foreground_ = foreground;
            if (same) {
                for (size_t i = 0; i < commands.size(); ++i)
                    draws_[i].command = std::move(commands[i]);
                return;
            }
            clear();
            for (auto &c : commands) {
                Draw d;
                d.command = std::move(c);
                draws_.push_back(std::move(d));
            }
        }
        void OverlayRenderer::draw(
            bool foreground, int sw, int sh, const ivec4 &tile, const std::function<void(int, int)> &draw_depth) {
            if (foreground != foreground_ || draws_.empty())
                return;
            if (!shader_) {
                shader_.reset(new ShaderProgram("viewer annotations"));
                auto &shader = *shader_;
                if (!shader.load_shader_from_code(ShaderProgram::VERTEX,
                                                  R"(#version 150
in vec3 position; in vec3 normal; in vec4 color; in vec2 uv;
uniform mat4 mvp, model; uniform mat3 normal_matrix; uniform bool lighting, material_lighting;
out vec4 tint; out vec2 texcoord; out vec3 world_normal, world_position;
void main() {
        gl_Position=mvp*vec4(position,1); tint=color; texcoord=uv;
        vec3 n=normal_matrix*normal; world_normal=n; world_position=(model*vec4(position,1)).xyz;
        if (lighting && !material_lighting && dot(n,n)>0)
            tint.rgb*=0.2+0.8*abs(dot(normalize(n),normalize(vec3(0.3,0.5,1))));
})") || !shader.load_shader_from_code(ShaderProgram::FRAGMENT,
                                      R"(#version 150
in vec4 tint; in vec2 texcoord; in vec3 world_normal, world_position;
uniform bool lighting, material_lighting, two_sides, distinct_back;
uniform vec3 eye, light; uniform vec4 ambient, specular, back_color; uniform float shininess;
uniform vec4 color; uniform sampler2D image; uniform bool depth_image;
out vec4 fragment;
void main() {
    fragment=depth_image?
        vec4(vec3(texture(image,vec2(texcoord.x,1-texcoord.y)).r),1):tint*color*texture(image,texcoord);
    if (lighting && material_lighting) {
        vec3 n=normalize(world_normal), view_dir=normalize(eye-world_position), l=normalize(light);
        if (distinct_back && !gl_FrontFacing) fragment.rgb=back_color.rgb;
        float df=dot(l,n); df=two_sides?abs(df):max(df,0);
        float sf=0;
        if (df>0) {sf=dot(normalize(l+view_dir),n);sf=two_sides?abs(sf):max(sf,0);sf=pow(sf,shininess);}
        fragment.rgb=fragment.rgb*df+specular.rgb*sf+ambient.rgb;
    }
})"))
                    throw std::runtime_error("cannot compile annotation shader");
                shader.set_attrib_name(ShaderProgram::POSITION, "position");
                shader.set_attrib_name(ShaderProgram::COLOR, "color");
                shader.set_attrib_name(ShaderProgram::NORMAL, "normal");
                shader.set_attrib_name(ShaderProgram::TEXCOORD, "uv");
                if (!shader.link_program())
                    throw std::runtime_error("cannot link annotation shader");
            }
            const auto program_ = shader_->get_program();
            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);
            const int width = tile.z > 0 ? tile.z : viewport[2], height = tile.w > 0 ? tile.w : viewport[3];
            bool needs_depth = false;
            for (const auto &d : draws_)
                needs_depth |= d.command.geometry && d.command.scene_depth_texture;
            if (needs_depth) {
                GLint draw_fbo = 0, read_fbo = 0;
                glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_fbo);
                glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_fbo);
                if (!depth_ || depth_->width() != width || depth_->height() != height) {
                    depth_.reset(new FramebufferObject(width, height, 0));
                    if (!depth_->add_depth_texture(GL_DEPTH_COMPONENT32F, GL_NEAREST, GL_NONE))
                        throw std::runtime_error("cannot allocate annotation scene depth");
                    depth_->deactivate_draw_buffers();
                }
                if (!depth_->bind())
                    throw std::runtime_error("cannot bind annotation scene depth");
                glDepthMask(GL_TRUE);
                glClearDepth(1);
                glClear(GL_DEPTH_BUFFER_BIT);
                draw_depth(width, height);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_fbo);
                glBindFramebuffer(GL_READ_FRAMEBUFFER, read_fbo);
                glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
            }
            glUseProgram(program_);
            glUniform1i(glGetUniformLocation(program_, "image"), 0);
            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
            glDepthMask(GL_FALSE);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glActiveTexture(GL_TEXTURE0);
            GLint sampler = 0;
            const bool sampler_objects = GLEW_VERSION_3_3 || GLEW_ARB_sampler_objects;
            if (sampler_objects) {
                glGetIntegerv(GL_SAMPLER_BINDING, &sampler);
                glBindSampler(0, 0);
            }
            GLint draw_fbo = 0, read_fbo = 0, samples = 0;
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_fbo);
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_fbo);
            // GL_SAMPLES reports the window visual on some drivers (e.g., Mesa llvmpipe)
            // even with a multisample framebuffer bound, so ask the attached renderbuffer.
            if (draw_fbo) {
                GLint type = 0, buffer = 0, bound = 0;
                glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                      GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
                glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                      GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &buffer);
                if (type == GL_RENDERBUFFER) {
                    glGetIntegerv(GL_RENDERBUFFER_BINDING, &bound);
                    glBindRenderbuffer(GL_RENDERBUFFER, buffer);
                    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples);
                    glBindRenderbuffer(GL_RENDERBUFFER, bound);
                }
            }
            else
                glGetIntegerv(GL_SAMPLES, &samples);
            bool independent = false;
            for (const auto &d : draws_)
                independent |= d.command.geometry && d.command.overlay_independent_depth;
            for (unsigned phase = 0; phase < 3; ++phase) {
                if (phase == 1 && independent) {
                    // Copy color sample-for-sample, then use private cleared depth. Copying
                    // color back preserves coverage/blending correlations with scene MSAA.
                    if (!hud_ || hud_->width() != viewport[2] || hud_->height() != viewport[3] ||
                        hud_->samples() != samples) {
                        hud_.reset(new FramebufferObject(viewport[2], viewport[3], samples));
                        if (!hud_->add_color_buffer() || !hud_->add_depth_buffer(GL_DEPTH_COMPONENT32F) ||
                            hud_->samples() != samples)
                            throw std::runtime_error("cannot allocate independent HUD target");
                    }
                    if (!hud_->bind())
                        throw std::runtime_error("cannot bind independent HUD target");
                    glDisable(GL_SCISSOR_TEST);
                    glDepthMask(GL_TRUE);
                    glClearDepth(1);
                    glClear(GL_DEPTH_BUFFER_BIT);
                    glBindFramebuffer(GL_READ_FRAMEBUFFER, draw_fbo);
                    GLint read_buffer = 0, draw_buffer = 0;
                    glGetIntegerv(GL_READ_BUFFER, &read_buffer);
                    // The caller can draw to a different color attachment than it reads.
                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_fbo);
                    glGetIntegerv(GL_DRAW_BUFFER0, &draw_buffer);
                    glReadBuffer(draw_buffer);
                    hud_->bind(GL_DRAW_FRAMEBUFFER);
                    glBlitFramebuffer(viewport[0], viewport[1], viewport[0] + viewport[2], viewport[1] + viewport[3], 0,
                                      0, viewport[2], viewport[3], GL_COLOR_BUFFER_BIT, GL_NEAREST);
                    glReadBuffer(read_buffer);
                }
                for (auto &d : draws_) {
                    const auto &c = d.command;
                    if (!c.geometry || unsigned(c.overlay_depth_test          ? 0
                                                : c.overlay_independent_depth ? 1
                                                                              : 2) != phase)
                        continue;
                    if (!d.vao) {
                        glGenVertexArrays(1, &d.vao);
                        glBindVertexArray(d.vao);
                        glGenBuffers(1, &d.vertices);
                        glBindBuffer(GL_ARRAY_BUFFER, d.vertices);
                        const auto &v = c.geometry->vertices();
                        glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(graphics::Vertex), v.data(), GL_STATIC_DRAW);
                        const size_t offsets[] = {
                            offsetof(graphics::Vertex, position), offsetof(graphics::Vertex, color),
                            offsetof(graphics::Vertex, texcoord), offsetof(graphics::Vertex, normal)};
                        const int sizes[] = {3, 4, 2, 3};
                        const unsigned slots[] = {ShaderProgram::POSITION, ShaderProgram::COLOR,
                                                  ShaderProgram::TEXCOORD, ShaderProgram::NORMAL};
                        for (unsigned i = 0; i < 4; ++i) {
                            glEnableVertexAttribArray(slots[i]);
                            glVertexAttribPointer(slots[i], sizes[i], GL_FLOAT, GL_FALSE, sizeof(graphics::Vertex),
                                                  reinterpret_cast<void *>(offsets[i]));
                        }
                        if (!c.geometry->indices().empty()) {
                            glGenBuffers(1, &d.indices);
                            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d.indices);
                            const auto &ids = c.geometry->indices();
                            glBufferData(GL_ELEMENT_ARRAY_BUFFER, ids.size() * sizeof(uint32_t), ids.data(),
                                         GL_STATIC_DRAW);
                        }
                        glGenTextures(1, &d.texture);
                        glBindTexture(GL_TEXTURE_2D, d.texture);
                        GLint unpack_buffer = 0;
                        glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &unpack_buffer);
                        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                        const GLenum names[] = {GL_UNPACK_ALIGNMENT, GL_UNPACK_ROW_LENGTH, GL_UNPACK_SKIP_ROWS,
                                                GL_UNPACK_SKIP_PIXELS};
                        GLint saved[4];
                        for (unsigned i = 0; i < 4; ++i) {
                            glGetIntegerv(names[i], &saved[i]);
                            glPixelStorei(names[i], i ? 0 : 1);
                        }
                        const unsigned char white[] = {255, 255, 255, 255};
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, c.texture ? c.texture->width() : 1,
                                     c.texture ? c.texture->height() : 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                                     c.texture ? c.texture->rgba().data() : white);
                        const auto sampler = c.texture ? c.texture->sampler() : graphics::Sampler{};
                        const GLint filter = sampler.filter == graphics::TextureFilter::linear ? GL_LINEAR : GL_NEAREST;
                        const GLint wrap = sampler.wrap == graphics::TextureWrap::repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
                        for (unsigned i = 0; i < 4; ++i)
                            glPixelStorei(names[i], saved[i]);
                        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpack_buffer);
                    }
                    auto vp = c.viewport;
                    if (vp == ivec4(0))
                        vp = ivec4(0, 0, width, height);
                    else {
                        const int x = int(std::lround(double(vp.x) * width / sw)),
                                  y = int(std::lround(double(vp.y) * height / sh));
                        const int right = int(std::lround(double(vp.x + vp.z) * width / sw)),
                                  bottom = int(std::lround(double(vp.y + vp.w) * height / sh));
                        vp = ivec4(x, y, right - x, bottom - y);
                    }
                    if (vp.z <= 0 || vp.w <= 0)
                        continue;
                    const int origin_x = phase == 1 ? 0 : viewport[0], origin_y = phase == 1 ? 0 : viewport[1];
                    glViewport(origin_x + vp.x - tile.x, origin_y + viewport[3] - vp.y - vp.w + tile.y, vp.z, vp.w);
                    glEnable(GL_SCISSOR_TEST);
                    glScissor(origin_x + vp.x - tile.x, origin_y + viewport[3] - vp.y - vp.w + tile.y, vp.z, vp.w);
                    glDepthMask(phase == 1 ? GL_TRUE : GL_FALSE);
                    if (phase < 2) {
                        glEnable(GL_DEPTH_TEST);
                        glDepthFunc(phase == 1 ? GL_LESS : GL_LEQUAL);
                    } else
                        glDisable(GL_DEPTH_TEST);
                    glUniform1i(glGetUniformLocation(program_, "lighting"), phase == 1 && c.lighting);
                    glUniformMatrix3fv(glGetUniformLocation(program_, "normal_matrix"), 1, GL_FALSE, c.normal_matrix);
                    glUniform1i(glGetUniformLocation(program_, "material_lighting"), c.material_lighting);
                    glUniform1i(glGetUniformLocation(program_, "two_sides"), c.lighting_two_sides);
                    glUniform1i(glGetUniformLocation(program_, "distinct_back"), c.distinct_back_color);
                    glUniformMatrix4fv(glGetUniformLocation(program_, "model"), 1, GL_FALSE, c.model);
                    glUniform3fv(glGetUniformLocation(program_, "eye"), 1, c.eye);
                    glUniform3fv(glGetUniformLocation(program_, "light"), 1, c.light);
                    glUniform4fv(glGetUniformLocation(program_, "ambient"), 1, c.ambient);
                    glUniform4fv(glGetUniformLocation(program_, "specular"), 1, c.specular);
                    glUniform4fv(glGetUniformLocation(program_, "back_color"), 1, c.back_color);
                    glUniform1f(glGetUniformLocation(program_, "shininess"), c.shininess);
                    glUniformMatrix4fv(glGetUniformLocation(program_, "mvp"), 1, GL_FALSE, c.model_view_projection);
                    glUniform4fv(glGetUniformLocation(program_, "color"), 1, c.color);
                    glBindVertexArray(d.vao);
                    glUniform1i(glGetUniformLocation(program_, "depth_image"), c.scene_depth_texture);
                    glBindTexture(GL_TEXTURE_2D, c.scene_depth_texture ? depth_->depth_texture() : d.texture);
                    if (d.indices)
                        glDrawElements(GL_TRIANGLES, c.geometry->indices().size(), GL_UNSIGNED_INT, nullptr);
                    else
                        glDrawArrays(GL_TRIANGLES, 0, c.geometry->vertices().size());
                }
                if (phase == 1 && independent) {
                    glDisable(GL_SCISSOR_TEST);
                    hud_->bind(GL_READ_FRAMEBUFFER);
                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_fbo);
                    glBlitFramebuffer(0, 0, viewport[2], viewport[3], viewport[0], viewport[1],
                                      viewport[0] + viewport[2], viewport[1] + viewport[3], GL_COLOR_BUFFER_BIT,
                                      GL_NEAREST);
                    glBindFramebuffer(GL_READ_FRAMEBUFFER, read_fbo);
                    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
                    glDepthMask(GL_FALSE);
                }
            }
            if (sampler_objects)
                glBindSampler(0, sampler);
        }
    } // namespace opengl
} // namespace easy3d
