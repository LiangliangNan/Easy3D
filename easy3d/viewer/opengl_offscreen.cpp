// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/opengl_offscreen.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/opengl.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <limits>
#include <stdexcept>
namespace easy3d {
OpenGLOffScreen::OpenGLOffScreen(int w, int h)
    : OpenGLViewer("Easy3D OffScreen Renderer",4,3,2,false,true,24,8,
                   w>0?w:throw std::invalid_argument("offscreen width must be positive"),
                   h>0?h:throw std::invalid_argument("offscreen height must be positive")) {
    manual_.clear();width_=w;height_=h;dpi_scaling_=1;init();resize(w,h);
}
OpenGLOffScreen::~OpenGLOffScreen() { activate(); }
void OpenGLOffScreen::activate() const {
    glfwMakeContextCurrent(window_);glViewport(0,0,width_,height_);
}
void OpenGLOffScreen::framebuffer_size(int& w,int& h) const { w=width_;h=height_; }
void OpenGLOffScreen::resize(int w,int h) {
    if(w<=0 || h<=0)throw std::invalid_argument("offscreen dimensions must be positive");
    activate();OpenGLViewer::resize(w,h);
    width_=w;height_=h;dpi_scaling_=1;camera()->setScreenWidthAndHeight(w,h);
}
bool OpenGLOffScreen::render(const std::string& file,float scale,int samples,int bg,bool expand) const {
    if(!std::isfinite(scale) || scale<=0 || double(width())*scale>std::numeric_limits<int>::max() ||
       double(height())*scale>std::numeric_limits<int>::max() || samples<=0 || samples>64 ||
       (samples&(samples-1)) || bg<0 || bg>2)return false;
    activate();return snapshot(file,scale,samples==1?0:samples,bg,expand);
}
Model* OpenGLOffScreen::add_model(const std::string& file,bool create) {
    auto* model=OpenGLViewer::add_model(file,create);if(model)fit_screen();return model;
}
Model* OpenGLOffScreen::add_model(std::shared_ptr<Model> model,bool create) {
    auto* result=OpenGLViewer::add_model(std::move(model),create);if(result)fit_screen();return result;
}
Drawable* OpenGLOffScreen::add_drawable(std::shared_ptr<Drawable> drawable) {
    activate();return OpenGLViewer::add_drawable(std::move(drawable));
}
}
