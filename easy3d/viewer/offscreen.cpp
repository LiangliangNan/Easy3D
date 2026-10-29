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

#include <easy3d/viewer/offscreen.h>
#include <easy3d/viewer/backends/offscreen.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/drawable.h>
#include <easy3d/util/logging.h>
#include <cmath>
#include <limits>
namespace easy3d {
struct OffScreen::Impl {
    std::unique_ptr<detail::OffScreenBackend> host;
    Backend selected=Backend::Automatic;
    std::string fallback;
    void create(Backend backend,int w,int h,const OffScreenOptions& options) {
        switch(backend) {
        case Backend::Vulkan:
#ifdef EASY3D_HAS_VULKAN
            host=detail::create_vulkan_offscreen(w,h,options);break;
#else
            throw BackendUnavailable("Vulkan backend was not built");
#endif
        case Backend::OpenGL:
#ifndef EASY3D_NO_OPENGL
            host=detail::create_opengl_offscreen(w,h,options);break;
#else
            throw BackendUnavailable("OpenGL backend was not built");
#endif
        default:throw std::invalid_argument("invalid offscreen backend");
        }
        selected=backend;
    }
};
OffScreen::OffScreen(int w,int h,const OffScreenOptions& options) : impl_(new Impl) {
    if(w<=0 || h<=0) throw std::invalid_argument("offscreen dimensions must be positive");
    if(options.backend==Backend::Automatic) {
        try {impl_->create(Backend::Vulkan,w,h,options);}
        catch(const BackendUnavailable& e) {
            impl_->fallback=e.what();
            try {impl_->create(Backend::OpenGL,w,h,options);}
            catch(const BackendUnavailable& gl) {
                throw BackendUnavailable("No offscreen backend available. Vulkan: "+impl_->fallback+"; OpenGL: "+gl.what());
            }
        }
    } else impl_->create(options.backend,w,h,options);
    auto* cam=camera();cam->setType(Camera::PERSPECTIVE);
    cam->setUpVector(vec3(0,0,1));cam->setViewDirection(vec3(-1,0,0));cam->showEntireScene();
    LOG(INFO)<<"OffScreen backend: "<<(backend()==Backend::Vulkan?"Vulkan":"OpenGL")
             <<(impl_->fallback.empty()?"":"; Vulkan unavailable: "+impl_->fallback);
}
OffScreen::~OffScreen() = default;
Backend OffScreen::backend() const {return impl_->selected;}
unsigned OffScreen::validation_errors() const {return impl_->host->validation_errors();}
const std::string& OffScreen::fallback_reason() const {return impl_->fallback;}
Camera* OffScreen::camera() {return impl_->host->camera();}
const Camera* OffScreen::camera() const {return impl_->host->camera();}
int OffScreen::width() const {return impl_->host->width();}
int OffScreen::height() const {return impl_->host->height();}
void OffScreen::resize(int w,int h) {
    if(w<=0 || h<=0) throw std::invalid_argument("offscreen dimensions must be positive");
    impl_->host->resize(w,h);
}
void OffScreen::set_background_color(const vec4& color) {
    for(unsigned i=0;i<4;++i) if(!std::isfinite(color[i])) throw std::invalid_argument("background color must be finite");
    impl_->host->set_background_color(color);
}
const vec4& OffScreen::background_color() const {return impl_->host->background_color();}
bool OffScreen::render(const std::string& f,float scale,int samples,int bg,bool expand) const {
    if(!std::isfinite(scale) || scale<=0 || double(width())*scale>std::numeric_limits<int>::max() || double(height())*scale>std::numeric_limits<int>::max() || samples<=0 || (samples&(samples-1)) || samples>64 || bg<0 || bg>2) return false;
    return impl_->host->render(f,scale,samples,bg,expand);
}
Model* OffScreen::add_model(const std::string& f,bool create) {return impl_->host->add_model(f,create);}
Model* OffScreen::add_model(std::shared_ptr<Model> m,bool create) {return impl_->host->add_model(std::move(m),create);}
bool OffScreen::delete_model(Model* m) {return impl_->host->delete_model(m);}
const std::vector<std::shared_ptr<Model>>& OffScreen::models() const {return impl_->host->models();}
Model* OffScreen::current_model() const {return impl_->host->current_model();}
Drawable* OffScreen::add_drawable(std::shared_ptr<Drawable> d) {return impl_->host->add_drawable(std::move(d));}
bool OffScreen::delete_drawable(Drawable* d) {return impl_->host->delete_drawable(d);}
const std::vector<std::shared_ptr<Drawable>>& OffScreen::drawables() const {return impl_->host->drawables();}
void OffScreen::clear_scene() {impl_->host->clear_scene();}
void OffScreen::fit_screen(const Model* m) {impl_->host->fit_screen(m);}
}
