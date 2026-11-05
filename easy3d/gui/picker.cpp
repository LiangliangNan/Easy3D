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


#include <easy3d/gui/picker.h>

#ifdef EASY3D_HAS_VULKAN
#include <easy3d/renderer/backends/vulkan/drawable.h>
#include <easy3d/renderer/backends/vulkan/clipping_plane.h>
#include <easy3d/renderer/clipping_plane.h>
#else
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl_error.h>
#endif
#include <cmath>
#include <limits>
#include <stdexcept>


namespace easy3d {



    Picker::Picker(const Camera *cam) : camera_(cam) {
        if (!cam) throw std::invalid_argument("picker requires a camera");
    }


    Picker::~Picker() = default;


#ifdef EASY3D_HAS_VULKAN
    graphics::RenderDevice& Picker::render_device() {
        if (!device_)
            device_ = graphics::create_vulkan_device();
        if (drawable_renderer_)
            drawable_renderer_->prune();
        return *device_;
    }

    graphics::DrawCommand Picker::draw_command(const std::shared_ptr<Drawable>& drawable, unsigned id) {
        if (!drawable_renderer_)
            drawable_renderer_.reset(new vulkan::DrawableRenderer);
        return drawable_renderer_->command(drawable, id, *camera(),
            vulkan::ClippingPlaneRenderer::planes(*ClippingPlane::instance()));
    }
#else
    void Picker::setup_framebuffer(int width, int height) {
        // prepare a frame buffer object (fbo), I will do offscreen rendering to the new fbo
        if (!fbo_) {
            fbo_.reset(new FramebufferObject(width, height, 0));
            fbo_->add_color_buffer();
            fbo_->add_depth_buffer();
            easy3d_debug_log_gl_error
            easy3d_debug_log_frame_buffer_error

#if 0 // for debugging only
            fbo_->print_attachments();
            fbo_->print_draw_buffers();
            fbo_->print_read_buffer();
#endif
        }
        fbo_->ensure_size(width, height);
        easy3d_debug_log_gl_error
        easy3d_debug_log_frame_buffer_error
    }


#endif

    void Picker::screen_to_opengl(int x, int y, int &gl_x, int &gl_y, int width, int height) const {
        if(width<=0 || height<=0 || camera()->screenWidth()<=0 || camera()->screenHeight()<=0)
            throw std::invalid_argument("picker viewport dimensions must be positive");
        const double fx=std::floor(double(x)*width/camera()->screenWidth());
        const double fy=height-1-std::floor(double(y)*height/camera()->screenHeight());
        if(fx<std::numeric_limits<int>::min() || fx>std::numeric_limits<int>::max() ||
           fy<std::numeric_limits<int>::min() || fy>std::numeric_limits<int>::max())
            throw std::out_of_range("picker coordinate exceeds integer range");
        gl_x = static_cast<int>(fx);gl_y = static_cast<int>(fy);
    }

}
