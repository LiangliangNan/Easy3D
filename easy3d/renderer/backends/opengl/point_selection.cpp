// SPDX-License-Identifier: GPL-3.0-or-later
#include "point_selection.h"
#include <easy3d/renderer/drawable.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/util/resource.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
namespace easy3d { namespace opengl {
namespace {
// Selection must also work inside a caller's rendering scope. Save the bindings
// not covered by Viewer's ordinary scene-query state (notably indexed TF state).
struct State {
    GLint active, textures[3], texture_buffer, feedback, indexed, feedback_object=0;
    bool feedback_objects=GLEW_VERSION_4_0 || GLEW_ARB_transform_feedback2;
    GLint64 start,size;
    bool discard;
    State() {
        if(feedback_objects) {
            glGetIntegerv(GL_TRANSFORM_FEEDBACK_BINDING,&feedback_object);
            GLint running=0;glGetIntegerv(GL_TRANSFORM_FEEDBACK_ACTIVE,&running);
            if(running) throw std::logic_error("selection cannot interrupt active transform feedback");
        }
        glGetIntegerv(GL_ACTIVE_TEXTURE,&active);
        glGetIntegerv(GL_TEXTURE_BUFFER_BINDING,&texture_buffer);
        glGetIntegerv(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING,&feedback);
        glGetIntegeri_v(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING,0,&indexed);
        glGetInteger64i_v(GL_TRANSFORM_FEEDBACK_BUFFER_START,0,&start);
        glGetInteger64i_v(GL_TRANSFORM_FEEDBACK_BUFFER_SIZE,0,&size);
        discard=glIsEnabled(GL_RASTERIZER_DISCARD);
        for(unsigned i=0;i<3;++i) {glActiveTexture(GL_TEXTURE0+i);glGetIntegerv(GL_TEXTURE_BINDING_BUFFER,&textures[i]);}
    }
    ~State() {
        for(unsigned i=0;i<3;++i) {glActiveTexture(GL_TEXTURE0+i);glBindTexture(GL_TEXTURE_BUFFER,textures[i]);}
        glActiveTexture(active);glBindBuffer(GL_TEXTURE_BUFFER,texture_buffer);
        if(feedback_objects) glBindTransformFeedback(GL_TRANSFORM_FEEDBACK,feedback_object);
        if(indexed && size) glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER,0,indexed,start,size);
        else glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER,0,indexed);
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER,feedback);
        if(discard) glEnable(GL_RASTERIZER_DISCARD);else glDisable(GL_RASTERIZER_DISCARD);
    }
};
void texture_buffer(unsigned& texture,unsigned buffer,GLenum format,unsigned unit) {
    glGenTextures(1,&texture);glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(GL_TEXTURE_BUFFER,texture);glTexBuffer(GL_TEXTURE_BUFFER,format,buffer);
}
}
struct PointSelector::Points:graphics::PointData {
    Points(uint32_t count):PointData({}, {}, count) {}
    std::weak_ptr<Drawable> owner;
    unsigned long long revision=0;
    GLuint positions=0,mask=0,mask_texture=0;
    void release() {
        glDeleteTextures(1,&positions);glDeleteTextures(1,&mask_texture);glDeleteBuffers(1,&mask);
        positions=mask=mask_texture=0;
    }
};
PointSelector::~PointSelector() {
    for(auto& p:points_) p->release();
    if(feedback_) glDeleteTransformFeedbacks(1,&feedback_);
    glDeleteVertexArrays(1,&vao_);glDeleteBuffers(1,&input_);glDeleteBuffers(1,&output_);
    glDeleteBuffers(1,&polygon_);glDeleteTextures(1,&polygon_texture_);
}
std::shared_ptr<const graphics::PointData> PointSelector::points(const std::shared_ptr<Drawable>& drawable,unsigned long long revision) {
    State state;
    for(auto i=points_.begin();i!=points_.end();) {
        if((*i)->owner.lock()==drawable && (*i)->revision==revision) return *i;
        if(i->use_count()==1) {(*i)->release();i=points_.erase(i);} else ++i;
    }
    GLint limit=0;glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE,&limit);
    if(drawable->num_vertices()>size_t(limit)/3 || drawable->num_vertices()>size_t(INT32_MAX))
        throw std::invalid_argument("point positions exceed OpenGL texture-buffer range");
    auto result=std::make_shared<Points>(uint32_t(drawable->num_vertices()));
    result->owner=drawable;result->revision=revision;
    if(!result->source_count()) {points_.push_back(result);return result;}
    std::vector<uint32_t> mask((size_t(result->source_count())+31)/32,~0u);
    if(drawable->element_buffer()) {
        std::fill(mask.begin(),mask.end(),0);
        std::vector<uint32_t> ids(drawable->num_indices());
        glBindBuffer(GL_TEXTURE_BUFFER,drawable->element_buffer());
        glGetBufferSubData(GL_TEXTURE_BUFFER,0,ids.size()*sizeof(uint32_t),ids.data());
        for(auto id:ids) {
            if(id>=result->source_count()) throw std::invalid_argument("point element index is out of range");
            mask[id>>5]|=1u<<(id&31);
        }
    }
    // A texture-buffer view retains the VBO data store after later buffer-name
    // deletion/replacement. There is no position upload, CPU readback or second store.
    texture_buffer(result->positions,drawable->vertex_buffer(),GL_R32F,0);
    glGenBuffers(1,&result->mask);glBindBuffer(GL_TEXTURE_BUFFER,result->mask);
    glBufferData(GL_TEXTURE_BUFFER,mask.size()*sizeof(uint32_t),mask.data(),GL_STATIC_DRAW);
    texture_buffer(result->mask_texture,result->mask,GL_R32UI,2);
    points_.push_back(result);
    return result;
}
std::vector<uint32_t> PointSelector::select(const graphics::PointSelectionRequest& request) {
    State state;
    auto source=std::dynamic_pointer_cast<const Points>(request.points);
    if(!source || std::find(points_.begin(),points_.end(),source)==points_.end())
        throw std::invalid_argument("OpenGL selection requires this viewer's point resource");
    if(!request.width || !request.height) throw std::invalid_argument("selection requires positive framebuffer dimensions");
    const size_t count=(size_t(source->source_count())+31)/32;
    auto words=request.words;
    if(words.empty()) words.resize(count,0);
    else if(words.size()!=count) throw std::invalid_argument("selection word count does not match points");
    for(unsigned i=0;i<16;++i) if(!std::isfinite(request.model_view_projection[i])) throw std::invalid_argument("nonfinite selection transform");
    for(const auto& p:request.polygon) if(!std::isfinite(p.x) || !std::isfinite(p.y)) throw std::invalid_argument("nonfinite selection polygon");
    const auto planes=graphics::normalize_clipping_planes(request.local_clipping_planes);
    bool area=false;
    for(size_t i=1;i+1<request.polygon.size();++i) {
        auto a=request.polygon[i]-request.polygon[0],b=request.polygon[i+1]-request.polygon[0];
        area=area || std::abs(double(a.x)*b.y-double(a.y)*b.x)>1e-8;
    }
    if(!source->source_count() || !area) return words;
    GLint limit=0;glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE,&limit);
    if(request.polygon.size()>size_t(limit)) throw std::invalid_argument("selection polygon exceeds OpenGL texture-buffer range");
    if(!shader_) {
        std::unique_ptr<ShaderProgram> shader(new ShaderProgram("point selection"));
        if(!shader->load_shader_from_file(ShaderProgram::VERTEX,resource::directory()+"/shaders/selection/selection_pointcloud_words.vert"))
            throw std::runtime_error("cannot compile point selection shader");
        const char* varying="new_word";glTransformFeedbackVaryings(shader->get_program(),1,&varying,GL_INTERLEAVED_ATTRIBS);
        glBindAttribLocation(shader->get_program(),0,"old_word");
        if(!shader->link_program()) throw std::runtime_error("cannot link point selection shader");
        shader_=std::move(shader);
        if(state.feedback_objects) glGenTransformFeedbacks(1,&feedback_);
        glGenVertexArrays(1,&vao_);glGenBuffers(1,&input_);glGenBuffers(1,&output_);glGenBuffers(1,&polygon_);
        glBindBuffer(GL_TEXTURE_BUFFER,polygon_);glBufferData(GL_TEXTURE_BUFFER,sizeof(vec2),nullptr,GL_STREAM_DRAW);
        texture_buffer(polygon_texture_,polygon_,GL_RG32F,1);
    }
    const GLuint program=shader_->get_program();glUseProgram(program);
    glBindVertexArray(vao_);glBindBuffer(GL_ARRAY_BUFFER,input_);
    glBufferData(GL_ARRAY_BUFFER,words.size()*sizeof(uint32_t),words.data(),GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);glVertexAttribIPointer(0,1,GL_UNSIGNED_INT,0,nullptr);
    if(feedback_) glBindTransformFeedback(GL_TRANSFORM_FEEDBACK,feedback_);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER,output_);glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,words.size()*sizeof(uint32_t),nullptr,GL_STREAM_READ);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER,0,output_);
    glBindBuffer(GL_TEXTURE_BUFFER,polygon_);glBufferData(GL_TEXTURE_BUFFER,request.polygon.size()*sizeof(vec2),request.polygon.data(),GL_STREAM_DRAW);
    const GLuint textures[]={source->positions,polygon_texture_,source->mask_texture};
    const char* names[]={"positions","polygon","active_words"};
    for(unsigned i=0;i<3;++i) {glActiveTexture(GL_TEXTURE0+i);glBindTexture(GL_TEXTURE_BUFFER,textures[i]);glUniform1i(glGetUniformLocation(program,names[i]),i);}
    glUniformMatrix4fv(glGetUniformLocation(program,"mvp"),1,GL_FALSE,request.model_view_projection);
    glUniform2f(glGetUniformLocation(program,"dimensions"),float(request.width),float(request.height));
    glUniform1i(glGetUniformLocation(program,"source_count"),source->source_count());
    glUniform1i(glGetUniformLocation(program,"polygon_count"),request.polygon.size());
    glUniform1i(glGetUniformLocation(program,"deselect"),request.deselect);
    glUniform1i(glGetUniformLocation(program,"plane_count"),planes.size());
    if(!planes.empty()) glUniform4fv(glGetUniformLocation(program,"planes"),planes.size(),planes.front());
    glEnable(GL_RASTERIZER_DISCARD);glBeginTransformFeedback(GL_POINTS);glDrawArrays(GL_POINTS,0,words.size());glEndTransformFeedback();
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER,0,words.size()*sizeof(uint32_t),words.data());
    return words;
}
}}
