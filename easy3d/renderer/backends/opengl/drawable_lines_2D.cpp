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

#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/vertex_array_object.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/opengl_error.h>
#include <stdexcept>

namespace easy3d {
    void LinesDrawable2D::opengl_update_vertex_buffer(const std::vector<vec2>& points, bool dynamic) {
        assert(vao_);
        bool success = vao_->create_array_buffer(vertex_buffer_, ShaderProgram::POSITION, points.data(),
                                                 points.size() * sizeof(vec2), 2, dynamic);
        LOG_IF(!success, ERROR) << "failed creating vertex buffer";
        num_vertices_ = success ? points.size() : 0;
    }

    void LinesDrawable2D::opengl_release_line_resources() {
        const GLuint textures[]={position_texture_,index_texture_};glDeleteTextures(2,textures);
        if (line_vao_) glDeleteVertexArrays(1,&line_vao_);
        position_texture_=index_texture_=line_vao_=0;
    }

    void LinesDrawable2D::draw(const Camera*) const {
        if (has_cpu_buffers()) throw std::logic_error("draw(camera) requires the OpenGL backend; submit CPU drawables through VulkanOffScreen");
        if (update_needed_ || vertex_buffer_==0) {
            const_cast<LinesDrawable2D*>(this)->update_buffers_internal();
            const_cast<LinesDrawable2D*>(this)->update_needed_=false;
        }
        const bool indexed=element_buffer_!=0;
        const size_t count=indexed ? num_indices_ : num_vertices_;
        if (!count || !num_vertices_) return;
        if (count%2) throw std::invalid_argument("screen-space lines require endpoint pairs");
        GLint viewport[4];glGetIntegerv(GL_VIEWPORT,viewport);
        if (viewport[2]<=0 || viewport[3]<=0) return;
        GLint maximum=0;glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE,&maximum);
        if (num_vertices_>size_t(maximum) || (indexed && count>size_t(maximum)))
            throw std::length_error("screen-space line buffer exceeds texture-buffer capacity");

        const std::string name="screen_space/screen_space_lines";
        ShaderProgram* program=ShaderManager::get_program(name);
        if (!program) program=ShaderManager::create_program_from_files(name);
        if (!program) return;
        if (!position_texture_) glGenTextures(1,&position_texture_);
        if (!index_texture_) glGenTextures(1,&index_texture_);
        if (!line_vao_) glGenVertexArrays(1,&line_vao_);

        // Reuse the original buffers as vertex-stage texture views. No geometry
        // shader, driver wide-line support or position/index readback is needed.
        GLint active=0,previous_positions=0,previous_indices=0,previous_vao=0;
        glGetIntegerv(GL_ACTIVE_TEXTURE,&active);glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&previous_vao);
        glActiveTexture(GL_TEXTURE0);glGetIntegerv(GL_TEXTURE_BINDING_BUFFER,&previous_positions);
        glActiveTexture(GL_TEXTURE1);glGetIntegerv(GL_TEXTURE_BINDING_BUFFER,&previous_indices);
        const GLboolean depth_test=glIsEnabled(GL_DEPTH_TEST),blend=glIsEnabled(GL_BLEND),cull=glIsEnabled(GL_CULL_FACE);
        const GLboolean clip0=glIsEnabled(GL_CLIP_DISTANCE0),clip1=glIsEnabled(GL_CLIP_DISTANCE1);
        GLboolean depth_write;glGetBooleanv(GL_DEPTH_WRITEMASK,&depth_write);
        GLint src_rgb,dst_rgb,src_alpha,dst_alpha,equation_rgb,equation_alpha;
        glGetIntegerv(GL_BLEND_SRC_RGB,&src_rgb);glGetIntegerv(GL_BLEND_DST_RGB,&dst_rgb);
        glGetIntegerv(GL_BLEND_SRC_ALPHA,&src_alpha);glGetIntegerv(GL_BLEND_DST_ALPHA,&dst_alpha);
        glGetIntegerv(GL_BLEND_EQUATION_RGB,&equation_rgb);glGetIntegerv(GL_BLEND_EQUATION_ALPHA,&equation_alpha);
        glDisable(GL_DEPTH_TEST);glDepthMask(GL_FALSE);glDisable(GL_CULL_FACE);
        glDisable(GL_CLIP_DISTANCE0);glDisable(GL_CLIP_DISTANCE1);
        glEnable(GL_BLEND);glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        program->bind();
        program->bind_texture("positions",position_texture_,0,GL_TEXTURE_BUFFER);
        glTexBuffer(GL_TEXTURE_BUFFER,GL_RG32F,vertex_buffer_);
        program->bind_texture("indices",index_texture_,1,GL_TEXTURE_BUFFER);
        // Bind a complete unused index view for the non-indexed shader branch.
        glTexBuffer(GL_TEXTURE_BUFFER,GL_R32UI,indexed ? element_buffer_ : vertex_buffer_);
        program->set_uniform("indexed",indexed)->set_uniform("viewport",vec2(float(viewport[2]),float(viewport[3])))
            ->set_uniform("line_width",line_width_)->set_uniform("depth",-1.0f)->set_uniform("screen_color",color_);
        glBindVertexArray(line_vao_);glDrawArraysInstanced(GL_TRIANGLES,0,6,GLsizei(count/2));
        glBindVertexArray(GLuint(previous_vao));program->release();
        glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_BUFFER,GLuint(previous_positions));
        glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_BUFFER,GLuint(previous_indices));glActiveTexture(GLenum(active));
        if (depth_test) glEnable(GL_DEPTH_TEST);else glDisable(GL_DEPTH_TEST);glDepthMask(depth_write);
        if (blend) glEnable(GL_BLEND);else glDisable(GL_BLEND);
        if (cull) glEnable(GL_CULL_FACE);else glDisable(GL_CULL_FACE);
        if (clip0) glEnable(GL_CLIP_DISTANCE0);else glDisable(GL_CLIP_DISTANCE0);
        if (clip1) glEnable(GL_CLIP_DISTANCE1);else glDisable(GL_CLIP_DISTANCE1);
        glBlendEquationSeparate(GLenum(equation_rgb),GLenum(equation_alpha));
        glBlendFuncSeparate(GLenum(src_rgb),GLenum(dst_rgb),GLenum(src_alpha),GLenum(dst_alpha));
        easy3d_debug_log_gl_error
    }
}
