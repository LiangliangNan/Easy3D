// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/util/initializer.h>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
void require(bool ok,const char* message) {if(!ok) throw std::runtime_error(message);}
void create_curves_drawables(Viewer&);
struct TestViewer : Viewer {
    using Viewer::Viewer;
    using Viewer::key_press_event;
    const std::string& hint() const {return hint_;}
};
int main() {
    try {
        initialize(false,false,false);
        TestViewer viewer("OpenGL screen line widths");
        create_curves_drawables(viewer);
        require(viewer.hint().find("line width")!=std::string::npos,"OpenGL tutorial hint missing");
        for(const auto& d:viewer.drawables()) require(static_cast<LinesDrawable2D*>(d.get())->line_width()==3,"tutorial default width mismatch");
        viewer.key_press_event(Viewer::KEY_RIGHT_BRACKET,0);
        for(const auto& d:viewer.drawables()) require(static_cast<LinesDrawable2D*>(d.get())->line_width()==4,"OpenGL tutorial width shortcut failed");
        viewer.key_press_event(Viewer::KEY_0,0);
        for(const auto& d:viewer.drawables()) require(static_cast<LinesDrawable2D*>(d.get())->line_width()==3,"OpenGL tutorial width reset failed");
        while(glGetError()!=GL_NO_ERROR) {}
        GLuint fbo,color,depth;
        glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);
        glGenTextures(1,&color);glBindTexture(GL_TEXTURE_2D,color);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,161,127,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,color,0);
        glGenRenderbuffers(1,&depth);glBindRenderbuffer(GL_RENDERBUFFER,depth);
        glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,161,127);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);
        require(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE,"incomplete fixture framebuffer");
        glViewport(0,0,161,127);
        LinesDrawable2D lines;
        lines.update_vertex_buffer({vec2(10,63.5f),vec2(150,63.5f)},161,127);
        lines.update_element_buffer(std::vector<unsigned>{0,1});
        lines.set_uniform_coloring(vec4(1,0,0,1));
        auto capture=[&]() {
            glDepthMask(GL_TRUE);glClearDepth(.4);glClearColor(0,0,0,0);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);glEnable(GL_CLIP_DISTANCE0);
            glDisable(GL_BLEND);glActiveTexture(GL_TEXTURE3);
            lines.draw(nullptr);
            require(glIsEnabled(GL_DEPTH_TEST) && glIsEnabled(GL_CULL_FACE) && glIsEnabled(GL_CLIP_DISTANCE0)
                    && !glIsEnabled(GL_BLEND),"line draw leaked enable state");
            GLint active;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);require(active==GL_TEXTURE3,"line draw leaked active texture");
            GLboolean mask;glGetBooleanv(GL_DEPTH_WRITEMASK,&mask);require(mask,"line draw leaked depth mask");
            std::vector<unsigned char> rgba(161*127*4);glReadPixels(0,0,161,127,GL_RGBA,GL_UNSIGNED_BYTE,rgba.data());
            std::vector<float> z(161*127);glReadPixels(0,0,161,127,GL_DEPTH_COMPONENT,GL_FLOAT,z.data());
            for(float value:z) require(std::abs(value-.4f)<1e-5f,"screen line wrote scene depth");
            require(glGetError()==GL_NO_ERROR,"OpenGL screen line error");
            size_t covered=0;for(size_t i=3;i<rgba.size();i+=4) covered+=rgba[i]>0;
            return covered;
        };
        const auto narrow=capture();require(narrow>100,"one-pixel line missing");
        lines.set_line_width(7);const auto wide=capture();require(wide>narrow*5,"indexed width ignored");
        lines.disable_element_buffer();require(capture()==wide,"non-indexed width differs");
        lines.set_line_width(2.5f);const auto fractional=capture();require(fractional>narrow && fractional<wide,"fractional width ignored");
        lines.set_line_width(1);require(capture()==narrow,"width restore failed");
        glDisable(GL_CLIP_DISTANCE0);glBindFramebuffer(GL_FRAMEBUFFER,0);
        glDeleteFramebuffers(1,&fbo);glDeleteTextures(1,&color);glDeleteRenderbuffers(1,&depth);
        std::cout<<"PASS: OpenGL indexed/non-indexed portable widths, fractional width, preserved depth and GL state\n";
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
