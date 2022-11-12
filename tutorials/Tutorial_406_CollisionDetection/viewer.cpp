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
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/resources.h>
#include <easy3d/util/file_system.h>
#include <easy3d/algo/collider.h>

using namespace easy3d;


CollisionViewer::CollisionViewer(const std::string &title)
        : Viewer(title)
        , model0_color_(0.9f, 1.0f, 0.9f)
        , model1_color_(0.9f, 0.9f, 1.0f)
{
    auto m0 = add_model(resource::directory() + "/data/bunny.ply");
    auto m1 = add_model(resource::directory() + "/data/mannequin.ply");
    auto mesh0 = dynamic_cast<SurfaceMesh*>(m0);
    auto mesh1 = dynamic_cast<SurfaceMesh*>(m1);
    if (mesh0 && mesh1) {
        // use the manipulator to transform the first model (for visualization purpose, but its geometry is not changed)
        mesh0->set_manipulator(new Manipulator(mesh0));

        mesh0->add_face_property<vec3>("face_color", model0_color_);
        mesh0->renderer()->get_triangles_drawable("faces")->set_coloring(easy3d::State::COLOR_PROPERTY, easy3d::State::FACE, "face_color");

        mesh1->add_face_property<vec3>("face_color", model1_color_);
        mesh1->renderer()->get_triangles_drawable("faces")->set_coloring(easy3d::State::COLOR_PROPERTY, easy3d::State::FACE, "face_color");

        collider_ = new Collider(mesh0, mesh1);
    }
    else
        LOG(WARNING) << "not two meshes have been loaded";

    timer_.set_interval(20, this, &CollisionViewer::detect);
}


CollisionViewer::~CollisionViewer() {
    timer_.stop();
    delete collider_;
}


std::string CollisionViewer::usage() const {
    return ("------------ Collision Viewer usage ------------\n"
            "Press the 'space' key to start/pause animation. \n"
            "------------------------------------------------\n");
}


void CollisionViewer::detect() {
    if (!collider_ || models().size() < 2)
        return;

    static float angle = 0.0f;
    angle += 0.01f;

    models_[0]->manipulator()->frame()->setFromMatrix(mat4::rotation(vec3(0, 0, 1), angle));
    auto pairs = collider_->detect(models_[0]->manipulator()->matrix(), mat4::identity());
    std::cout << pairs.size() << " pairs of intersecting faces" << std::endl;

    // mark the intersecting faces red
    auto mesh0_colors = dynamic_cast<SurfaceMesh*>(models_[0])->get_face_property<vec3>("face_color");
    auto mesh1_colors = dynamic_cast<SurfaceMesh*>(models_[1])->get_face_property<vec3>("face_color");
    mesh0_colors.vector().assign(dynamic_cast<SurfaceMesh*>(models_[0])->n_faces(), model0_color_);
    mesh1_colors.vector().assign(dynamic_cast<SurfaceMesh*>(models_[1])->n_faces(), model1_color_);
    for (const auto& pair : pairs) {
        mesh0_colors[pair.first] = vec3(1.0f, 0.0f, 0.0f);
        mesh1_colors[pair.second] = vec3(1.0f, 0.0f, 0.0f);
    }
    models_[0]->renderer()->update();
    models_[1]->renderer()->update();

    update();
}
