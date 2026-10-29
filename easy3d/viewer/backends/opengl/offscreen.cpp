// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/backends/offscreen.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/opengl.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
namespace easy3d { namespace detail {
namespace {
class OpenGLOffScreenHost : public Viewer {
public:
    OpenGLOffScreenHost(int w,int h,const OffScreenOptions&)
        : Viewer("Easy3D OffScreen Renderer",4,3,2,false,true,24,8,w,h) {
        manual_.clear();width_=w;height_=h;dpi_scaling_=1;init();resize(w,h);
    }
    ~OpenGLOffScreenHost() override {activate();}
    void activate() {glfwMakeContextCurrent(window_);dpi_scaling_=1;glViewport(0,0,width_,height_);}
    unsigned validation_errors() const {return 0;}
    void framebuffer_size(int& w,int& h) const override {w=width_;h=height_;}
    bool render(const std::string& f,float scale,int samples,int bg,bool expand) const {
        return snapshot(f,scale,samples==1?0:samples,bg,expand);
    }
    void resize(int w,int h) {
        Viewer::resize(w,h);
        // Offscreen dimensions are pixels, independent of the hidden window's DPI.
        width_=w;height_=h;camera()->setScreenWidthAndHeight(w,h);
    }
    Model* add_model(const std::string& file,bool create) override {
        auto* result=Viewer::add_model(file,create);if(result) fit_screen();return result;
    }
    Model* add_model(std::shared_ptr<Model> model,bool create) {
        auto* result=Viewer::add_model(std::move(model),create);if(result) fit_screen();return result;
    }
};
}
std::unique_ptr<OffScreenBackend> create_opengl_offscreen(int w,int h,const OffScreenOptions& o) {
    if(o.validation || o.synchronization_validation)
        throw BackendUnavailable("OpenGL offscreen does not support the requested validation mode");
    return std::unique_ptr<OffScreenBackend>(new OffScreenAdapter<OpenGLOffScreenHost>(w,h,o));
}
} }
