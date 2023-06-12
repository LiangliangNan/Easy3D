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

#include "viewer.h"
#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/read_pixel.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/algo/gaussian_noise.h>


using namespace easy3d;

VirtualScanner::VirtualScanner(const std::string &title)
    : Viewer(title)
    , add_noise_(false)
{
    camera()->setUpVector(vec3(0, 1, 0));

    usage_string_ =
            "-------------- Virtual Scanner usage -------------- \n"
            "- change the view using the mouse.\n"
            "- press the 'Space' key to perform scanning. Everything (and only those) visible\n"
            "  will be captured in a point cloud.\n"
            "- press 'n' to toggle Gaussian noise.\n"
            "---------------------------------------------------------- \n";
}


bool VirtualScanner::key_press_event(int key, int modifiers) {
    if (key == KEY_SPACE && modifiers == 0) {
        int fw, fh;
		framebuffer_size(fw, fh);

        FramebufferObject fbo(fw, fh, 0);
        fbo.add_depth_buffer(GL_DEPTH_COMPONENT32F);
        fbo.bind();
        glClearDepth(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);
        draw();
        fbo.release();

        std::vector<float> depths;
        fbo.read_depth(depths, false);

        const mat4& MVP = camera()->modelViewProjectionMatrix();
        const mat4& invMVP = inverse(MVP);
        const int viewport[] = { 0, 0, width_, height_};

        std::vector<vec3> points;
        for (int x=0; x<width_; ++x) {
            for (int y=0; y<height_; ++y) {
                // NOTE: when dealing with OpenGL, we always work in the highdpi screen space
#if defined(__APPLE__)
                const int idx = static_cast<int>(static_cast<float>(y) * dpi_scaling() * static_cast<float>(fw) + static_cast<float>(x) * dpi_scaling());
#else
                const int idx = static_cast<int>(y * fw + x);
#endif
                const float d = depths[idx];
                if (d < 1.0f) {
                    vec3 vs(static_cast<float>(x), static_cast<float>(y), d);
                    vs.x = static_cast<float>(vs.x - static_cast<float>(viewport[0])) / static_cast<float>(viewport[2]) * 2.0f - 1.0f;
                    vs.y = static_cast<float>(vs.y - static_cast<float>(viewport[1])) / static_cast<float>(viewport[3]) * 2.0f - 1.0f;
                    vs.z = vs.z * 2.0f - 1.0f;
                    points.push_back(invMVP * vs);
                }
            }
        }

        if (!points.empty()) {
            auto cloud = new PointCloud;
            for (const auto& p : points)
                cloud->add_vertex(p);

            if (add_noise_) {
                const float ratio = 0.0001f;
                const float sigma = current_model()->bounding_box().radius() * ratio;
                GaussianNoise::apply(cloud, sigma);
                std::cout << "Gaussian noise added (sigma = " << ratio << " * model radius)" << std::endl;
            }
            add_model(cloud);
            update();
        }

        return false;
    }
    else if (key == KEY_N && modifiers == 0) {
        add_noise_ = !add_noise_;
        if (add_noise_)
            std::cout << "add_noise = ON" << std::endl;
        else
            std::cout << "add_noise = OFF" << std::endl;
        return false;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}

