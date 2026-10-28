// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_406_CollisionDetection/viewer.h>
#include <easy3d/algo/collider.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/util/initializer.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <thread>
using namespace easy3d;
void require(bool ok,const char* text) {if (!ok) throw std::runtime_error(text);}
bool same(const mat4& a,const mat4& b) {for (unsigned i=0;i<16;++i) if (a[i]!=b[i]) return false;return true;}
struct CollisionViewer : TutorialCollisionDetection {
    using TutorialCollisionDetection::TutorialCollisionDetection;
    using TutorialCollisionDetection::initialize_collision;
    using TutorialCollisionDetection::detect;
    using TutorialCollisionDetection::mouse_press_event;
    using TutorialCollisionDetection::mouse_drag_event;
    using TutorialCollisionDetection::mouse_release_event;
    bool has_usage() const {return manual_.find("Collision Viewer usage")!=std::string::npos && hint_.find("pause/resume")!=std::string::npos;}
};
void triangle(SurfaceMesh& mesh,vec3 a,vec3 b,vec3 c) {
    auto x=mesh.add_vertex(a),y=mesh.add_vertex(b),z=mesh.add_vertex(c);mesh.add_triangle(x,y,z);
}
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"collision colors changed depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"collision colors changed IDs");
}
void check_colors(const SurfaceMesh& a,const SurfaceMesh& b,const Collider& reference,bool expect_contact) {
    const auto pairs=reference.detect(a.manipulator()->matrix(),b.manipulator()->matrix());
    require(!expect_contact || !pairs.empty(),"crossing fixture did not collide");
    std::vector<bool> hit0(a.n_faces()),hit1(b.n_faces());
    for (const auto& pair:pairs) {hit0[pair.first.idx()]=true;hit1[pair.second.idx()]=true;}
    const auto colors0=a.get_face_property<vec3>("face_color"),colors1=b.get_face_property<vec3>("face_color");
    for (auto f:a.faces()) require(colors0[f]==(hit0[f.idx()]?vec3(1,0,0):vec3(.8f,1,.8f)),"first mesh collision coloring mismatch");
    for (auto f:b.faces()) require(colors1[f]==(hit1[f.idx()]?vec3(1,0,0):vec3(.8f,.8f,1)),"second mesh collision coloring mismatch");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        VulkanViewerOptions options;options.visible=false;options.width=480;options.height=360;
        options.device.validation=options.device.synchronization_validation=argc>1;
        auto first=std::make_shared<SurfaceMesh>(),second=std::make_shared<SurfaceMesh>();
        triangle(*first,vec3(-1,-1,0),vec3(1,-1,0),vec3(0,1,0));
        triangle(*first,vec3(3,-1,0),vec3(5,-1,0),vec3(4,1,0));
        triangle(*second,vec3(-.7f,-.7f,-1),vec3(.7f,-.7f,1),vec3(0,.7f,0));
        triangle(*second,vec3(6,-1,-1),vec3(7,-1,1),vec3(6.5f,1,0));
        const auto local=first->points();
        {
            CollisionViewer v("Collision regression",options);
            require(v.models().size()==2 && v.is_animating() && v.has_usage(),"default collision scene/usage missing");
            require(v.snapshot("vulkan-collision-default.png",1,1),"default collision snapshot failed");
            v.clear_scene();v.add_model(first);v.add_model(second);v.initialize_collision();
            v.camera()->setViewDirection(vec3(0,0,-1));v.camera()->setUpVector(vec3(0,1,0));
            v.camera()->setType(Camera::ORTHOGRAPHIC);v.fit_screen();
            for (const auto& m:v.models()) m->renderer()->get_triangles_drawable("faces")->set_lighting(false);
            auto key=glfwSetKeyCallback(v.window(),nullptr);glfwSetKeyCallback(v.window(),key);
            auto press=[&](int code,int mods=0){key(v.window(),code,0,GLFW_PRESS,mods);};
            press(GLFW_KEY_SPACE,GLFW_MOD_SHIFT);require(v.is_animating(),"modified Space toggled animation");
            press(GLFW_KEY_SPACE);require(!v.is_animating(),"Space did not pause collision animation");
            Collider reference(first.get(),second.get());check_colors(*first,*second,reference,true);
            for (unsigned samples:{1u,4u}) {
                if (!(v.device().supported_sample_counts()&samples)) continue;v.set_samples(samples);
                auto c0=first->get_face_property<vec3>("face_color"),c1=second->get_face_property<vec3>("face_color");
                c0.vector().assign(first->n_faces(),vec3(.8f,1,.8f));c1.vector().assign(second->n_faces(),vec3(.8f,.8f,1));
                first->renderer()->update();second->renderer()->update();const auto plain=v.capture();
                v.detect();const auto marked=v.capture();same_picking(plain,marked);
                require(plain.rgba!=marked.rgba,"collision face colors did not reach GPU rendering");
                check_colors(*first,*second,reference,true);
                bool checked=false;
                for (size_t i=0;i<marked.ids.size();++i) if (marked.ids[i].object==0 && marked.ids[i].primitive==0) {
                    const auto pick=v.pick(int(i%marked.width),int(i/marked.width));
                    require(pick.model==first && pick.face_index==0 && pick.depth==marked.depth[i],
                            "collision view lost source face/depth provenance");checked=true;break;
                }
                require(checked,"collision fixture has no visible first face to pick");
            }
            const auto initial=first->manipulator()->frame()->matrix();
            first->manipulator()->frame()->translate(vec3(100,0,0));v.detect();
            require(reference.detect(first->manipulator()->matrix(),second->manipulator()->matrix()).empty(),"separated meshes still intersect");
            check_colors(*first,*second,reference,false);
            first->manipulator()->frame()->setFromMatrix(initial);v.detect();check_colors(*first,*second,reference,true);
            const auto camera=v.camera()->modelViewProjectionMatrix();
            v.mouse_press_event(100,100,GLFW_MOUSE_BUTTON_RIGHT,GLFW_MOD_ALT);
            auto pose=first->manipulator()->matrix();
            v.mouse_drag_event(130,110,30,10,GLFW_MOUSE_BUTTON_RIGHT,GLFW_MOD_ALT);
            v.mouse_release_event(130,110,GLFW_MOUSE_BUTTON_RIGHT,GLFW_MOD_ALT);
            require(!same(pose,first->manipulator()->matrix()) && same(camera,v.camera()->modelViewProjectionMatrix()),"paused Alt-right did not manipulate only first model");
            check_colors(*first,*second,reference,false);
            v.mouse_press_event(100,100,GLFW_MOUSE_BUTTON_LEFT,GLFW_MOD_ALT);pose=first->manipulator()->matrix();
            v.mouse_drag_event(120,120,20,20,GLFW_MOUSE_BUTTON_LEFT,GLFW_MOD_ALT);
            v.mouse_release_event(120,120,GLFW_MOUSE_BUTTON_LEFT,GLFW_MOD_ALT);
            require(!same(pose,first->manipulator()->matrix()),"paused Alt-left did not rotate mesh");
            check_colors(*first,*second,reference,false);require(first->points()==local,"collision animation edited local positions");
            auto focus=glfwSetWindowFocusCallback(v.window(),nullptr);glfwSetWindowFocusCallback(v.window(),focus);
            v.mouse_press_event(100,100,GLFW_MOUSE_BUTTON_RIGHT,GLFW_MOD_ALT);pose=first->manipulator()->matrix();
            focus(v.window(),GLFW_FALSE);v.mouse_drag_event(130,110,30,10,GLFW_MOUSE_BUTTON_RIGHT,GLFW_MOD_ALT);
            require(same(pose,first->manipulator()->matrix()),"focus cancellation left collision drag active");
            press(GLFW_KEY_SPACE);require(v.is_animating(),"Space did not resume collision animation");
            std::this_thread::sleep_for(std::chrono::milliseconds(70));v.capture();
            require(same(pose,first->manipulator()->matrix()),"capture advanced collision animation");
            require(v.draw_frame() && !same(pose,first->manipulator()->matrix()),"UI frame did not advance due collision tick");
            check_colors(*first,*second,reference,false);
            press(GLFW_KEY_F2);require(!v.is_animating(),"F2 did not pause shared animation state");
            v.mouse_press_event(100,100,GLFW_MOUSE_BUTTON_RIGHT,GLFW_MOD_ALT);pose=first->manipulator()->matrix();
            v.resize(510,380);VulkanViewer::poll_events();v.draw_frame();
            v.mouse_drag_event(130,110,30,10,GLFW_MOUSE_BUTTON_RIGHT,GLFW_MOD_ALT);
            require(same(pose,first->manipulator()->matrix()),"resize did not cancel collision drag");
            require(v.snapshot("vulkan-collision.png",1,1),"collision snapshot failed");
            press(GLFW_KEY_SPACE);require(v.is_animating(),"restart before deletion failed");
            require(v.delete_model(first.get()) && !v.is_animating(),"deletion did not retire collision pair");
            const auto retired=second->get_face_property<vec3>("face_color");
            for (auto f:second->faces()) require(retired[f]==vec3(.8f,.8f,1),"retired pair left stale red collision faces");
            v.detect();v.draw_frame();v.mouse_drag_event(130,110,30,10,GLFW_MOUSE_BUTTON_RIGHT,GLFW_MOD_ALT);
            require(same(pose,first->manipulator()->matrix()),"removed mesh was still animated");
            v.clear_scene();require(v.models().empty() && !v.is_animating(),"clear did not stop collision animation");
            bool invalid=false;try {v.initialize_collision();} catch(const std::invalid_argument&) {invalid=true;}
            require(invalid,"empty collision scene accepted");
            require(v.device().validation_errors()==0,"collision validation errors");
        }
        first->manipulator()->frame()->translate(vec3(.1f,0,0)); // retained mesh has no viewer callback
        std::cout<<"PASS: collision face pairs/colors, transformed picking/depth/MSAA, UI animation, gestures and lifetime\n";
    } catch (const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
