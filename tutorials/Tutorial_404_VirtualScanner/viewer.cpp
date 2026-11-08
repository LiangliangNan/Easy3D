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
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/clipping_plane.h>
#include <cmath>
#include <easy3d/algo/gaussian_noise.h>

using namespace easy3d;

TutorialVirtualScanner::TutorialVirtualScanner(const std::string& title) : Viewer(title) {
    manual_ = "-------------- Virtual Scanner usage -------------- \n"
            "- change the view using the mouse.\n"
            "- press the 'Space' key to perform scanning. Everything (and only those) visible\n"
            "  will be captured in a point cloud.\n"
            "- press 'n' to toggle Gaussian noise.\n"
            "---------------------------------------------------------- \n";
    hint_ = "press 'Space' to perform scanning\n"
            "press 'n' to toggle Gaussian noise";
    camera()->setUpVector(easy3d::vec3(0,1,0));
}

bool TutorialVirtualScanner::key_press_event(int key,int modifiers) {
    if (modifiers==0 && key==KEY_N) { add_noise_=!add_noise_; return true; }
    if (modifiers==0 && key==KEY_SPACE) {
        int w,h; framebuffer_size(w,h);
        if (w<=0 || h<=0) return true;
        const auto inv=inverse(camera()->modelViewProjectionMatrix());
        const auto frame=capture(samples());
        std::vector<vec3> points;
        for(unsigned y=0;y<frame.height;++y) for(unsigned x=0;x<frame.width;++x) {
            const size_t pixel=size_t(y)*frame.width+x;const float d=frame.depth[pixel];
            if(frame.ids[pixel].object==UINT32_MAX || !std::isfinite(d) || d<0 || d>=1) continue;
            const vec4 p=inv*vec4(2*(float(x)+.5f)/frame.width-1,1-2*(float(y)+.5f)/frame.height,2*d-1,1);
            if(std::isfinite(p.w) && p.w!=0) points.emplace_back(p.x/p.w,p.y/p.w,p.z/p.w);
        }
        if (points.empty()) return true;
        auto cloud=std::make_shared<easy3d::PointCloud>();
        cloud->set_name("Virtual scan");
        for (const auto& p:points) cloud->add_vertex(p);
        if (add_noise_) easy3d::GaussianNoise::apply(cloud.get(),camera()->sceneRadius()*.0001f);
        // Keep the complete acquisition camera state, including its pivot.
        // Model attachment may fit the scene; retain the acquisition camera.
        Camera saved(*camera());saved.frame()->setPivotPoint(camera()->pivotPoint());
        saved.set_projection_matrix(camera()->projectionMatrix());saved.set_modelview_matrix(camera()->modelViewMatrix());
        const auto* plane=ClippingPlane::instance();const auto center=plane->center();const float radius=plane->scene_radius();
        add_model(cloud);
        ClippingPlane::instance()->fit_scene(center,radius);
        *camera()=saved;camera()->frame()->setPivotPoint(saved.pivotPoint());
        camera()->set_projection_matrix(saved.projectionMatrix());camera()->set_modelview_matrix(saved.modelViewMatrix());
        update(); return true;
    }
    return Viewer::key_press_event(key,modifiers);
}
