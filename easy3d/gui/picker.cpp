/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */


#include <easy3d/gui/picker.h>
#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/framebuffer_object.h>


namespace easy3d {


    FramebufferObject *Picker::fbo_ = 0;


    Picker::Picker(Camera *cam)
            : camera_(cam), use_gpu_if_supported_(true) {
    }


    Picker::~Picker() {
        if (fbo_) {
            delete fbo_;
            fbo_ = 0;
        }
    }


    void Picker::screen_to_opengl(int x, int y, int &gl_x, int &gl_y, int width, int height) const {
        float dpi_scaling = 1.0f;
        if (width > 0)
            dpi_scaling = width / static_cast<float>(camera()->screenWidth());
        else if (height > 0)
            dpi_scaling = height / static_cast<float>(camera()->screenHeight());

        gl_x = static_cast<int>(dpi_scaling * x);
        gl_y = static_cast<int>(dpi_scaling * (camera()->screenHeight() - 1 - y));
    }

}