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

#include "text_overlay.h"

#include <easy3d/renderer/text_layout.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/util/logging.h>

#include <cstddef>
#include <stdexcept>

using namespace easy3d;


TextOverlay::TextOverlay() : engine_(new graphics::TextLayoutEngine) {}

TextOverlay::~TextOverlay() = default;


bool TextOverlay::add_font(const std::string &file) {
    try {
        engine_->add_font(file);
        return true;
    } catch (const std::exception &e) {
        LOG(ERROR) << "failed loading font: " << e.what();
        return false;
    }
}


std::size_t TextOverlay::num_fonts() const {
    return engine_->font_names().size();
}


void TextOverlay::draw(const std::string &text, float x, float y, float font_size, std::size_t font_id,
                       const vec3 &color) const {
    if (text.empty() || font_id >= num_fonts())
        return;

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    graphics::TextOptions options;
    options.font = font_id;
    options.size = font_size;
    options.x = x;
    options.y = y;
    options.color = vec4(color, 1.0f);
    const auto commands = engine_->layout(text, viewport[2], viewport[3], options).commands;
    if (commands.empty())
        return;

    if (!shader_) {
        std::unique_ptr<ShaderProgram> shader(new ShaderProgram("text overlay"));
        const bool ok =
            shader->load_shader_from_code(ShaderProgram::VERTEX, R"(#version 150
in vec3 position; in vec4 color; in vec2 uv;
uniform mat4 mvp;
out vec4 tint; out vec2 texcoord;
void main() { gl_Position = mvp * vec4(position, 1); tint = color; texcoord = uv; })") &&
            shader->load_shader_from_code(ShaderProgram::FRAGMENT, R"(#version 150
in vec4 tint; in vec2 texcoord;
uniform vec4 color; uniform sampler2D image;
out vec4 fragment;
void main() { fragment = tint * color * texture(image, texcoord); })");
        shader->set_attrib_name(ShaderProgram::POSITION, "position");
        shader->set_attrib_name(ShaderProgram::COLOR, "color");
        shader->set_attrib_name(ShaderProgram::TEXCOORD, "uv");
        if (!ok || !shader->link_program()) {
            LOG_N_TIMES(3, ERROR) << "failed creating the text shader. " << COUNTER;
            return;
        }
        shader_ = std::move(shader);
    }

    const GLuint program = shader_->get_program();
    const GLboolean blend = glIsEnabled(GL_BLEND), depth_test = glIsEnabled(GL_DEPTH_TEST),
                    cull_face = glIsEnabled(GL_CULL_FACE);
    GLboolean depth_mask = GL_TRUE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depth_mask);
    GLint blend_src_rgb, blend_dst_rgb, blend_src_alpha, blend_dst_alpha, active_texture, bound_texture;
    glGetIntegerv(GL_BLEND_SRC_RGB, &blend_src_rgb);
    glGetIntegerv(GL_BLEND_DST_RGB, &blend_dst_rgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &blend_src_alpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &blend_dst_alpha);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound_texture);

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "image"), 0);

    GLint pixel_alignment = 4;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &pixel_alignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (const auto &c : commands) {
        if (!c.geometry || !c.texture)
            continue;
        const auto &vertices = c.geometry->vertices();
        const auto &indices = c.geometry->indices();

        GLuint vao = 0, vbo = 0, ibo = 0, texture = 0;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(graphics::Vertex), vertices.data(), GL_STREAM_DRAW);
        const std::size_t offsets[] = {offsetof(graphics::Vertex, position), offsetof(graphics::Vertex, color),
                                       offsetof(graphics::Vertex, texcoord)};
        const int sizes[] = {3, 4, 2};
        const unsigned slots[] = {ShaderProgram::POSITION, ShaderProgram::COLOR, ShaderProgram::TEXCOORD};
        for (int i = 0; i < 3; ++i) {
            glEnableVertexAttribArray(slots[i]);
            glVertexAttribPointer(slots[i], sizes[i], GL_FLOAT, GL_FALSE, sizeof(graphics::Vertex),
                                  reinterpret_cast<void *>(offsets[i]));
        }
        if (!indices.empty()) {
            glGenBuffers(1, &ibo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(std::uint32_t), indices.data(),
                         GL_STREAM_DRAW);
        }

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, c.texture->width(), c.texture->height(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, c.texture->rgba().data());
        const GLint filter = c.texture->sampler().filter == graphics::TextureFilter::linear ? GL_LINEAR : GL_NEAREST;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glUniformMatrix4fv(glGetUniformLocation(program, "mvp"), 1, GL_FALSE, c.model_view_projection);
        glUniform4fv(glGetUniformLocation(program, "color"), 1, c.color);
        if (ibo)
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
        else
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));

        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ibo);
        glDeleteTextures(1, &texture);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, pixel_alignment);
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(bound_texture));
    glActiveTexture(static_cast<GLenum>(active_texture));
    glUseProgram(0);
    glBlendFuncSeparate(blend_src_rgb, blend_dst_rgb, blend_src_alpha, blend_dst_alpha);
    glDepthMask(depth_mask);
    if (cull_face) glEnable(GL_CULL_FACE);
    if (depth_test) glEnable(GL_DEPTH_TEST);
    if (!blend) glDisable(GL_BLEND);
}
