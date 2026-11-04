// SPDX-License-Identifier: GPL-3.0-or-later
#include "overlay_renderer.h"
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/shader_program.h>
#include <stdexcept>
#include <cstddef>
#include <cmath>
namespace easy3d { namespace opengl {
OverlayRenderer::~OverlayRenderer() { clear(); }
void OverlayRenderer::clear() {
    for(auto& d:draws_) {
        glDeleteVertexArrays(1,&d.vao);glDeleteBuffers(1,&d.vertices);glDeleteBuffers(1,&d.indices);glDeleteTextures(1,&d.texture);
    }
    draws_.clear();
}
void OverlayRenderer::set(std::vector<graphics::DrawCommand> commands,bool foreground) {
    for(const auto& c:commands) if(c.geometry && (c.geometry->topology()!=graphics::Topology::triangles ||
        c.overlay_independent_depth || c.scene_depth_texture || c.lighting))
        throw std::invalid_argument("OpenGL annotations currently support unlit flat/scene-depth triangles only");
    clear();foreground_=foreground;
    for(auto& c:commands) { Draw d;d.command=std::move(c);draws_.push_back(std::move(d)); }
}
void OverlayRenderer::draw(bool foreground,int sw,int sh,const ivec4& tile) {
    if(foreground!=foreground_ || draws_.empty()) return;
    if(!shader_) {
        shader_.reset(new ShaderProgram("viewer annotations"));
        auto& shader=*shader_;
        if(!shader.load_shader_from_code(ShaderProgram::VERTEX,
            "#version 150\nin vec3 position;in vec4 color;in vec2 uv;uniform mat4 mvp;out vec4 tint;out vec2 texcoord;void main(){gl_Position=mvp*vec4(position,1);tint=color;texcoord=uv;}") ||
           !shader.load_shader_from_code(ShaderProgram::FRAGMENT,
            "#version 150\nin vec4 tint;in vec2 texcoord;uniform vec4 color;uniform sampler2D image;out vec4 fragment;void main(){fragment=tint*color*texture(image,texcoord);}"))
            throw std::runtime_error("cannot compile annotation shader");
        shader.set_attrib_name(ShaderProgram::POSITION,"position");
        shader.set_attrib_name(ShaderProgram::COLOR,"color");
        shader.set_attrib_name(ShaderProgram::TEXCOORD,"uv");
        if(!shader.link_program()) throw std::runtime_error("cannot link annotation shader");
    }
    const auto program_=shader_->get_program();
    GLint viewport[4];glGetIntegerv(GL_VIEWPORT,viewport);
    const int width=tile.z>0?tile.z:viewport[2],height=tile.w>0?tile.w:viewport[3];
    glUseProgram(program_);glUniform1i(glGetUniformLocation(program_,"image"),0);
    glEnable(GL_BLEND);glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);glDepthMask(GL_FALSE);glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    glActiveTexture(GL_TEXTURE0);
    GLint sampler=0;
    const bool sampler_objects=GLEW_VERSION_3_3 || GLEW_ARB_sampler_objects;
    if(sampler_objects) {glGetIntegerv(GL_SAMPLER_BINDING,&sampler);glBindSampler(0,0);}
    for(unsigned phase=0;phase<2;++phase) for(auto& d:draws_) {
        const auto& c=d.command;
        if(!c.geometry || unsigned(!c.overlay_depth_test)!=phase) continue;
        if(!d.vao) {
            glGenVertexArrays(1,&d.vao);glBindVertexArray(d.vao);
            glGenBuffers(1,&d.vertices);glBindBuffer(GL_ARRAY_BUFFER,d.vertices);
            const auto& v=c.geometry->vertices();glBufferData(GL_ARRAY_BUFFER,v.size()*sizeof(graphics::Vertex),v.data(),GL_STATIC_DRAW);
            const size_t offsets[]={offsetof(graphics::Vertex,position),offsetof(graphics::Vertex,color),offsetof(graphics::Vertex,texcoord)};
            const int sizes[]={3,4,2};
            const unsigned slots[]={ShaderProgram::POSITION,ShaderProgram::COLOR,ShaderProgram::TEXCOORD};
            for(unsigned i=0;i<3;++i) {glEnableVertexAttribArray(slots[i]);glVertexAttribPointer(slots[i],sizes[i],GL_FLOAT,GL_FALSE,sizeof(graphics::Vertex),reinterpret_cast<void*>(offsets[i]));}
            if(!c.geometry->indices().empty()) {
                glGenBuffers(1,&d.indices);glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,d.indices);
                const auto& ids=c.geometry->indices();glBufferData(GL_ELEMENT_ARRAY_BUFFER,ids.size()*sizeof(uint32_t),ids.data(),GL_STATIC_DRAW);
            }
            glGenTextures(1,&d.texture);glBindTexture(GL_TEXTURE_2D,d.texture);
            GLint unpack_buffer=0;glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING,&unpack_buffer);glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
            const GLenum names[]={GL_UNPACK_ALIGNMENT,GL_UNPACK_ROW_LENGTH,GL_UNPACK_SKIP_ROWS,GL_UNPACK_SKIP_PIXELS};GLint saved[4];
            for(unsigned i=0;i<4;++i) {glGetIntegerv(names[i],&saved[i]);glPixelStorei(names[i],i?0:1);}
            const unsigned char white[]={255,255,255,255};
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,c.texture?c.texture->width():1,c.texture?c.texture->height():1,0,GL_RGBA,GL_UNSIGNED_BYTE,c.texture?c.texture->rgba().data():white);
            const auto sampler=c.texture?c.texture->sampler():graphics::Sampler{};
            const GLint filter=sampler.filter==graphics::TextureFilter::linear?GL_LINEAR:GL_NEAREST;
            const GLint wrap=sampler.wrap==graphics::TextureWrap::repeat?GL_REPEAT:GL_CLAMP_TO_EDGE;
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,filter);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,filter);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,wrap);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,wrap);
            for(unsigned i=0;i<4;++i) glPixelStorei(names[i],saved[i]);glBindBuffer(GL_PIXEL_UNPACK_BUFFER,unpack_buffer);
        }
        auto vp=c.viewport;
        if(vp==ivec4(0)) vp=ivec4(0,0,width,height);
        else {
            const int x=int(std::lround(double(vp.x)*width/sw)),y=int(std::lround(double(vp.y)*height/sh));
            const int right=int(std::lround(double(vp.x+vp.z)*width/sw)),bottom=int(std::lround(double(vp.y+vp.w)*height/sh));
            vp=ivec4(x,y,right-x,bottom-y);
        }
        if(vp.z<=0 || vp.w<=0) continue;
        glViewport(viewport[0]+vp.x-tile.x,viewport[1]+viewport[3]-vp.y-vp.w+tile.y,vp.z,vp.w);
        glEnable(GL_SCISSOR_TEST);glScissor(viewport[0]+vp.x-tile.x,viewport[1]+viewport[3]-vp.y-vp.w+tile.y,vp.z,vp.w);
        if(c.overlay_depth_test) {glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);} else glDisable(GL_DEPTH_TEST);
        glUniformMatrix4fv(glGetUniformLocation(program_,"mvp"),1,GL_FALSE,c.model_view_projection);
        glUniform4fv(glGetUniformLocation(program_,"color"),1,c.color);glBindVertexArray(d.vao);glBindTexture(GL_TEXTURE_2D,d.texture);
        if(d.indices) glDrawElements(GL_TRIANGLES,c.geometry->indices().size(),GL_UNSIGNED_INT,nullptr);
        else glDrawArrays(GL_TRIANGLES,0,c.geometry->vertices().size());
    }
    if(sampler_objects) glBindSampler(0,sampler);
}
} }
