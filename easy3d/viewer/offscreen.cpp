// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_HAS_VULKAN   // the Vulkan counterpart is not folded in yet (work item U1)

#include <easy3d/viewer/offscreen.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/opengl.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <limits>
#include <stdexcept>
namespace easy3d {
OffScreen::OffScreen(unsigned int w, unsigned int h, const OpenGLOptions& gl_options)
    : Viewer("Easy3D OffScreen Renderer",
                   w?w:throw std::invalid_argument("offscreen width must be positive"),
                   h?h:throw std::invalid_argument("offscreen height must be positive"),
                   4,false,false,true,gl_options) {
    manual_.clear();width_=int(w);height_=int(h);dpi_scaling_=1;init();resize(int(w),int(h));
}
OffScreen::~OffScreen() { activate(); }
void OffScreen::activate() const {
    glfwMakeContextCurrent(window_);glViewport(0,0,width_,height_);
}
void OffScreen::framebuffer_size(int& w,int& h) const { w=width_;h=height_; }
void OffScreen::resize(int w,int h) {
    if(w<=0 || h<=0)throw std::invalid_argument("offscreen dimensions must be positive");
    activate();Viewer::resize(w,h);
    width_=w;height_=h;dpi_scaling_=1;camera()->setScreenWidthAndHeight(w,h);
}
bool OffScreen::render(const std::string& file,float scale,int samples,int bg,bool expand) const {
    if(!std::isfinite(scale) || scale<=0 || double(width())*scale>std::numeric_limits<int>::max() ||
       double(height())*scale>std::numeric_limits<int>::max() || samples<=0 || samples>64 ||
       (samples&(samples-1)) || bg<0 || bg>2)return false;
    activate();return snapshot(file,scale,samples==1?0:samples,bg,expand);
}
Model* OffScreen::add_model(const std::string& file,bool create) {
    auto* model=Viewer::add_model(file,create);if(model)fit_screen();return model;
}
Model* OffScreen::add_model(std::shared_ptr<Model> model,bool create) {
    auto* result=Viewer::add_model(std::move(model),create);if(result)fit_screen();return result;
}
Drawable* OffScreen::add_drawable(std::shared_ptr<Drawable> drawable) {
    activate();return Viewer::add_drawable(std::move(drawable));
}
}

#endif   // !EASY3D_HAS_VULKAN
