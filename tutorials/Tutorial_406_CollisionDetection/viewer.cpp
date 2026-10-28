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

namespace {
const char* const usage_manual = "--------------------- Collision Viewer usage ----------------------\n"
            "On start, you will see an animation with highlighted intersecting  \n"
            "faces. Press 'space' to pause/resume the animation.                \n"
            "When the animation is paused, you can manipulate the bunny model   \n"
            "(using your mouse) to perform real-time collection detection:      \n"
            "    - Alt + left button: rotate bunny model                        \n"
            "    - Alt + right button: translate bunny model                    \n"
            "------------------------------------------------------------------ \n";
const char* const usage_hint = "Press 'space' to pause/resume animation\n"
            "When paused:\n"
            "  - Alt + left button: rotate bunny model\n"
            "  - Alt + right button: translate bunny model";
}


#if !EASY3D_TUTORIAL_USES_VULKAN
#include <mutex>
#endif
#include <algorithm>
#include <stdexcept>

#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/resource.h>
#include <easy3d/algo/collider.h>


using namespace easy3d;

// \cond

#if EASY3D_TUTORIAL_USES_VULKAN
TutorialCollisionDetection::TutorialCollisionDetection(const std::string& title,const VulkanViewerOptions& options)
    : TutorialViewer(title,options),model0_color_(.8f,1,.8f),model1_color_(.8f,.8f,1) {
    manual_=usage_manual;hint_=usage_hint;
    add_model(resource::directory()+"/data/bunny.ply");
    add_model(resource::directory()+"/data/mannequin.ply");
    animation_func_=[this](VulkanViewer*) {advance();return true;};
    initialize_collision();
}
TutorialCollisionDetection::~TutorialCollisionDetection() {
    set_animation(false);animation_func_={};end_model_drag();collider_.reset();
}
bool TutorialCollisionDetection::collision_models_valid() const {
    if (!collider_ || models().size()!=2) return false;
    for (unsigned i=0;i<2;++i) {
        const auto model=collision_models_[i].lock();
        if (!model || !model->renderer() || !model->manipulator() ||
            std::find(models().begin(),models().end(),model)==models().end()) return false;
    }
    return true;
}
void TutorialCollisionDetection::initialize_collision() {
    retire_collision();
    if (models().size()!=2) throw std::invalid_argument("collision example requires two triangle meshes");
    auto first=std::dynamic_pointer_cast<SurfaceMesh>(models()[0]);
    auto second=std::dynamic_pointer_cast<SurfaceMesh>(models()[1]);
    if (!first || !second || first->n_faces()==0 || second->n_faces()==0 ||
        !first->is_triangle_mesh() || !second->is_triangle_mesh())
        throw std::invalid_argument("collision example requires two nonempty triangle meshes");
    for (const auto& mesh:{first,second}) if (!mesh->manipulator())
        mesh->set_manipulator(std::make_shared<Manipulator>(mesh.get()));
    first->add_face_property<vec3>("face_color",model0_color_);
    second->add_face_property<vec3>("face_color",model1_color_);
    for (const auto& mesh:{first,second})
        mesh->renderer()->get_triangles_drawable("faces")->set_coloring(State::COLOR_PROPERTY,State::FACE,"face_color");
    collider_.reset(new Collider(first.get(),second.get()));
    collision_models_[0]=first;collision_models_[1]=second;
    angle_=0;detect();next_tick_=std::chrono::steady_clock::now()+std::chrono::milliseconds(50);
    set_animation(true);
}
void TutorialCollisionDetection::end_model_drag() {
    if (auto model=drag_model_.lock()) if (model->manipulator()) model->manipulator()->frame()->action_end();
    drag_model_.reset();
}
void TutorialCollisionDetection::retire_collision() {
    set_animation(false);end_model_drag();
    // Retired pairs must not leave red contacts on a remaining/retained mesh.
    for (unsigned i=0;i<2;++i) if (const auto mesh=collision_models_[i].lock()) {
        auto colors=mesh->get_face_property<vec3>("face_color");
        if (colors) {
            colors.vector().assign(mesh->n_faces(),i==0?model0_color_:model1_color_);
            if (mesh->renderer()) mesh->renderer()->update();
        }
    }
    collider_.reset();collision_models_[0].reset();collision_models_[1].reset();
}
void TutorialCollisionDetection::advance() {
    if (!collision_models_valid()) {retire_collision();return;}
    const auto now=std::chrono::steady_clock::now();
    if (now<next_tick_) return;
    next_tick_=now+std::chrono::milliseconds(50); // no catch-up burst after a pause/minimize
    const auto mesh=collision_models_[0].lock();
    angle_+=.0001f;
    const auto pose=mat4::rotation(vec3(0,0,1),angle_)*mesh->manipulator()->frame()->matrix();
    mesh->manipulator()->frame()->setFromMatrix(pose);detect();
}
void TutorialCollisionDetection::detect() {
    if (!collision_models_valid()) {retire_collision();return;}
    const auto first=collision_models_[0].lock(),second=collision_models_[1].lock();
    const auto pairs=collider_->detect(first->manipulator()->matrix(),second->manipulator()->matrix());
    auto colors0=first->get_face_property<vec3>("face_color"),colors1=second->get_face_property<vec3>("face_color");
    colors0.vector().assign(first->n_faces(),model0_color_);colors1.vector().assign(second->n_faces(),model1_color_);
    for (const auto& pair:pairs) {colors0[pair.first]=vec3(1,0,0);colors1[pair.second]=vec3(1,0,0);}
    first->renderer()->update();second->renderer()->update();update();
}
bool TutorialCollisionDetection::key_press_event(int key,int modifiers) {
    if ((key==KEY_SPACE || key==KEY_F2) && modifiers==0) {
        end_model_drag();
        if (is_animating()) set_animation(false);
        else if (collision_models_valid()) {
            next_tick_=std::chrono::steady_clock::now()+std::chrono::milliseconds(50);set_animation(true);
        }
        return true;
    }
    return TutorialViewer::key_press_event(key,modifiers);
}
bool TutorialCollisionDetection::mouse_press_event(int x,int y,int button,int modifiers) {
    end_model_drag();
    if (modifiers==MODIF_ALT && !is_animating() && collision_models_valid() &&
        (button==BUTTON_LEFT || button==BUTTON_RIGHT)) {
        const auto mesh=collision_models_[0].lock();mesh->manipulator()->frame()->action_start();drag_model_=mesh;return true;
    }
    return TutorialViewer::mouse_press_event(x,y,button,modifiers);
}
bool TutorialCollisionDetection::mouse_drag_event(int x,int y,int dx,int dy,int button,int modifiers) {
    const auto mesh=drag_model_.lock();
    if (modifiers==MODIF_ALT && !is_animating() && collision_models_valid() && mesh) {
        if (button==BUTTON_LEFT) mesh->manipulator()->frame()->action_rotate(x,y,dx,dy,camera(),ManipulatedFrame::NONE);
        else if (button==BUTTON_RIGHT) mesh->manipulator()->frame()->action_translate(x,y,dx,dy,camera(),ManipulatedFrame::NONE);
        else return false;
        detect();return true;
    }
    return TutorialViewer::mouse_drag_event(x,y,dx,dy,button,modifiers);
}
bool TutorialCollisionDetection::mouse_release_event(int x,int y,int button,int modifiers) {
    end_model_drag();return TutorialViewer::mouse_release_event(x,y,button,modifiers);
}
bool TutorialCollisionDetection::focus_event(bool focused) {if (!focused) end_model_drag();return false;}
void TutorialCollisionDetection::post_resize(int,int) {end_model_drag();}
bool TutorialCollisionDetection::delete_model(Model* model) {
    if (collision_models_[0].lock().get()==model || collision_models_[1].lock().get()==model) {
        retire_collision();
    }
    return TutorialViewer::delete_model(model);
}
void TutorialCollisionDetection::clear_scene() {
    retire_collision();
    TutorialViewer::clear_scene();
}
#else
TutorialCollisionDetection::TutorialCollisionDetection(const std::string &title)
        : Viewer(title)
        , model0_color_(0.8f, 1.0f, 0.8f)
        , model1_color_(0.8f, 0.8f, 1.0f)
{
    auto m0 = add_model(resource::directory() + "/data/bunny.ply");
    auto m1 = add_model(resource::directory() + "/data/mannequin.ply");
    auto mesh0 = dynamic_cast<SurfaceMesh*>(m0);
    auto mesh1 = dynamic_cast<SurfaceMesh*>(m1);
    if (mesh0 && mesh1) {
        // use the manipulator to transform the first model (visualization only and geometry is not changed)
        mesh0->set_manipulator(std::make_shared<Manipulator>(mesh0));

        // the first mesh is colored by a "face_color" property
        mesh0->add_face_property<vec3>("face_color", model0_color_);
        mesh0->renderer()->get_triangles_drawable("faces")->set_coloring(easy3d::State::COLOR_PROPERTY, easy3d::State::FACE, "face_color");

        // the second mesh is also colored by a "face_color" property
        mesh1->add_face_property<vec3>("face_color", model1_color_);
        mesh1->renderer()->get_triangles_drawable("faces")->set_coloring(easy3d::State::COLOR_PROPERTY, easy3d::State::FACE, "face_color");

        collider_ = new Collider(mesh0, mesh1);
        // detect and show the result when the viewer is visible
        detect();

        timer_.set_interval(50, [this]()->void {
            static std::mutex mutex; // ensures the previous changes are completed
            std::lock_guard<std::mutex> guard(mutex);
            static float angle = 0.0f;
            angle += 0.0001f;
            auto T = mat4::rotation(vec3(0, 0, 1), angle) * models_[0]->manipulator()->frame()->matrix();
            models_[0]->manipulator()->frame()->setFromMatrix(T);
            detect();
        });
    }
    else
        LOG(WARNING) << "not all the two meshes have been loaded";

    manual_ = usage_manual;

    hint_ = usage_hint;
}


TutorialCollisionDetection::~TutorialCollisionDetection() {
    timer_.stop();
    delete collider_;
}


bool TutorialCollisionDetection::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
    if (collider_ && models().size() == 2 && modifiers == MODIF_ALT && timer_.is_paused()) {
        auto manipulator = models_[0]->manipulator();
        if (button == BUTTON_LEFT)
            manipulator->frame()->action_rotate(x, y, dx, dy, camera(), ManipulatedFrame::NONE);
        else if (button == BUTTON_RIGHT)
            manipulator->frame()->action_translate(x, y, dx, dy, camera(), ManipulatedFrame::NONE);
        else
            return false;

        detect();
        return true;
    }
    else
        return Viewer::mouse_drag_event(x, y, dx, dy, button, modifiers);
}


bool TutorialCollisionDetection::key_press_event(int key, int modifiers) {
    if (key == KEY_SPACE) {
        if (timer_.is_paused())
            timer_.resume();
        else
            timer_.pause();
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}


void TutorialCollisionDetection::detect() {
    const auto pairs = collider_->detect(models_[0]->manipulator()->matrix(), mat4::identity());
    //std::cout << pairs.size() << " pairs of intersecting faces" << std::endl;

    // mark the intersecting faces red
    // Note: The following code is for visualizing the intersecting faces and is not optimized.
    //      Ideas for better performance:
    //          (1) update only the color buffer;
    //          (2) use a shader storage buffer to transfer the *status* of the faces to the fragment shader.
    auto mesh0 = dynamic_cast<SurfaceMesh *>(models_[0].get());
    auto mesh1 = dynamic_cast<SurfaceMesh *>(models_[1].get());
    auto mesh0_colors = mesh0->get_face_property<vec3>("face_color");
    auto mesh1_colors = mesh1->get_face_property<vec3>("face_color");
    mesh0_colors.vector().assign(mesh0->n_faces(), model0_color_);
    mesh1_colors.vector().assign(mesh1->n_faces(), model1_color_);
    for (const auto &pair : pairs) {
        mesh0_colors[pair.first] = vec3(1.0f, 0.0f, 0.0f);
        mesh1_colors[pair.second] = vec3(1.0f, 0.0f, 0.0f);
    }
    models_[0]->renderer()->update();
    models_[1]->renderer()->update();

    update();
}

// \endcond
#endif // EASY3D_TUTORIAL_USES_VULKAN
