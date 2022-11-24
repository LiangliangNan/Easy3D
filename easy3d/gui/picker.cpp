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
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl_error.h>


namespace easy3d {


    FramebufferObject *Picker::fbo_ = nullptr;


    Picker::Picker(const Camera *cam)
            : camera_(cam), use_gpu_if_supported_(true) {
    }


    Picker::~Picker() {
        delete fbo_;
        fbo_ = nullptr;
    }


    void Picker::setup_framebuffer(int width, int height) {
        // prepare a frame buffer object (fbo), I will do offscreen rendering to the new fbo
        if (!fbo_) {
            fbo_ = new FramebufferObject(width, height, 0);
            fbo_->add_color_buffer();
            fbo_->add_depth_buffer();
            easy3d_debug_log_gl_error
            easy3d_debug_log_frame_buffer_error

#if 0
            fbo_->print_attachments();
            fbo_->print_draw_buffers();
            fbo_->print_read_buffer();
#endif
        }
        fbo_->ensure_size(width, height);
        easy3d_debug_log_gl_error
        easy3d_debug_log_frame_buffer_error
    }


    void Picker::screen_to_opengl(int x, int y, int &gl_x, int &gl_y, int width, int height) const {
        float dpi_scaling_x = static_cast<float>(width) / static_cast<float>(camera()->screenWidth());
        float dpi_scaling_y = static_cast<float>(height) / static_cast<float>(camera()->screenHeight());

        gl_x = static_cast<int>(dpi_scaling_x * x);
        gl_y = static_cast<int>(dpi_scaling_y * (camera()->screenHeight() - 1 - y));
    }

}