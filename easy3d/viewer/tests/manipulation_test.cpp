// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_405_ObjectManipulation/viewer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/backends/vulkan/shape.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
void require(bool ok,const char* text) {if (!ok) throw std::runtime_error(text);}
bool same_matrix(const mat4& a,const mat4& b) {for (unsigned i=0;i<16;++i) if (a[i]!=b[i]) return false;return true;}
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"guide changed scene depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"guide changed picking IDs");
}
struct ManipulationViewer : TutorialObjectManipulation {
    using TutorialObjectManipulation::TutorialObjectManipulation;
    using TutorialObjectManipulation::mouse_drag_event;
    using TutorialObjectManipulation::mouse_press_event;
    using TutorialObjectManipulation::mouse_release_event;
    bool has_usage() const {
        return manual_.find("Manipulator Viewer usage")!=std::string::npos && hint_.find("Alt + left button")!=std::string::npos;
    }
};
std::shared_ptr<SurfaceMesh> quad(float center,float z=0,float half=.8f) {
    auto mesh=std::make_shared<SurfaceMesh>();
    auto a=mesh->add_vertex(vec3(center-half,-half,z)),b=mesh->add_vertex(vec3(center+half,-half,z));
    auto c=mesh->add_vertex(vec3(center+half,half,z)),d=mesh->add_vertex(vec3(center-half,half,z));
    mesh->add_quad(a,b,c,d);return mesh;
}
void frame(VulkanViewer& v) {
    for (unsigned i=0;i<100;++i) {VulkanViewer::poll_events();if (v.draw_frame()) return;glfwWaitEventsTimeout(.01);}
    throw std::runtime_error("manipulation frame did not present");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        auto retained=quad(-1.2f);
        VulkanViewerOptions options;options.visible=false;options.width=520;options.height=360;
        options.device.validation=options.device.synchronization_validation=argc>1;
        {
            ManipulationViewer v("Object manipulation",options);
            require(v.has_usage(),"OpenGL counterpart manual/hint missing");
            v.add_model(retained);auto other=quad(1.2f);v.add_model(other);
            for (const auto& m:v.models()) m->renderer()->get_triangles_drawable("faces")->set_lighting(false);
            v.camera()->setType(Camera::ORTHOGRAPHIC);v.fit_screen();frame(v);
            vec2 mouse_pixel(0);
            auto mouse=[&](GLFWwindow*,int button,int action,int modifiers) {
                if (action==GLFW_PRESS) v.mouse_press_event(int(mouse_pixel.x),int(mouse_pixel.y),button,modifiers);
                else v.mouse_release_event(int(mouse_pixel.x),int(mouse_pixel.y),button,modifiers);
            };
            auto key=glfwSetKeyCallback(v.window(),nullptr);glfwSetKeyCallback(v.window(),key);
            auto focus=glfwSetWindowFocusCallback(v.window(),nullptr);glfwSetWindowFocusCallback(v.window(),focus);
            auto position=[&](const vec3& world) {
                const auto p=v.camera()->projectedCoordinatesOf(world);mouse_pixel=vec2(p.x,p.y);return p;
            };
            auto click=[&](const vec3& world) {position(world);mouse(v.window(),GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,0);
                mouse(v.window(),GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,0);frame(v);};
            const auto baseline=v.capture();
            const auto fixture=v.camera()->projectedCoordinatesOf(vec3(-1.2f,0,0));
            require(v.pick(int(fixture.x),int(fixture.y)).model==retained,"initial transformed fixture pick failed");
            click(vec3(-1.2f,0,0));
            require(retained->renderer()->is_selected() && retained->manipulator(),"click did not select/create manipulator");
            require(!other->renderer()->is_selected(),"selection affected second model");
            same_picking(baseline,v.capture());
            const auto points=retained->points();const auto camera=v.camera()->modelViewProjectionMatrix();
            for (unsigned samples:{1u,4u}) {
                if (!(v.device().supported_sample_counts()&samples)) continue;v.set_samples(samples);frame(v);
                const auto guide=v.capture();v.set_overlays({});const auto plain=v.capture();same_picking(guide,plain);
                require(guide.rgba!=plain.rgba,"RGB manipulation frame not displayed");frame(v);
            }
            auto p=position(vec3(-1.2f,0,0));mouse(v.window(),GLFW_MOUSE_BUTTON_RIGHT,GLFW_PRESS,GLFW_MOD_ALT);
            key(v.window(),GLFW_KEY_X,0,GLFW_PRESS,0);
            auto transform=retained->manipulator()->matrix();
            v.mouse_drag_event(int(p.x),int(p.y)+20,0,20,GLFW_MOUSE_BUTTON_RIGHT,GLFW_MOD_ALT);
            require(same_matrix(transform,retained->manipulator()->matrix()),"X constraint allowed vertical model translation");
            key(v.window(),GLFW_KEY_X,0,GLFW_RELEASE,0);
            v.mouse_drag_event(int(p.x)+25,int(p.y)+15,25,15,GLFW_MOUSE_BUTTON_RIGHT,GLFW_MOD_ALT);
            mouse(v.window(),GLFW_MOUSE_BUTTON_RIGHT,GLFW_RELEASE,GLFW_MOD_ALT);frame(v);
            require(!same_matrix(transform,retained->manipulator()->matrix()),"Alt-right did not translate model");
            require(same_matrix(camera,v.camera()->modelViewProjectionMatrix()),"object drag moved camera");
            require(retained->points()==points,"model manipulation rewrote local geometry");
            const auto moved=retained->manipulator()->matrix()*vec4(-1.2f,0,0,1);
            const auto projected=v.camera()->projectedCoordinatesOf(moved.xyz());
            require(v.pick(int(projected.x),int(projected.y)).model==retained,"picking did not follow transformed model");
            p=position(moved.xyz());mouse(v.window(),GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,GLFW_MOD_ALT);
            transform=retained->manipulator()->matrix();
            v.mouse_drag_event(int(p.x)+30,int(p.y)+20,30,20,GLFW_MOUSE_BUTTON_LEFT,GLFW_MOD_ALT);
            mouse(v.window(),GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,GLFW_MOD_ALT);frame(v);
            require(!same_matrix(transform,retained->manipulator()->matrix()),"Alt-left did not rotate model");
            require(same_matrix(camera,v.camera()->modelViewProjectionMatrix()),"object rotation moved camera");
            require(!other->manipulator(),"drag transformed unselected model");
            require(v.snapshot("vulkan-object-manipulation.png",1,1),"manipulation snapshot failed");
            // A nearer opaque model must hide the frame, without changing its IDs.
            auto foreground=quad(0,2,5);v.add_model(foreground,true,false);
            auto* faces=foreground->renderer()->get_triangles_drawable("faces");faces->set_uniform_coloring(vec4(.2f,.4f,.6f,1));faces->set_lighting(false);
            frame(v);const auto obscured=v.capture();v.set_overlays({});const auto no_guide=v.capture();
            same_picking(obscured,no_guide);require(obscured.rgba==no_guide.rgba,"manipulator frame showed through nearer surface");
            v.delete_model(foreground.get());frame(v);
            // Focus cancellation and deletion during a drag must not leave borrowed pointers.
            position(moved.xyz());mouse(v.window(),GLFW_MOUSE_BUTTON_RIGHT,GLFW_PRESS,GLFW_MOD_ALT);
            focus(v.window(),GLFW_FALSE);transform=retained->manipulator()->matrix();
            v.mouse_drag_event(100,100,20,20,GLFW_MOUSE_BUTTON_RIGHT,GLFW_MOD_ALT);
            require(same_matrix(transform,retained->manipulator()->matrix()),"focus loss left model drag active");
            mouse(v.window(),GLFW_MOUSE_BUTTON_RIGHT,GLFW_RELEASE,GLFW_MOD_ALT);
            position(moved.xyz());mouse(v.window(),GLFW_MOUSE_BUTTON_RIGHT,GLFW_PRESS,GLFW_MOD_ALT);
            require(v.delete_model(retained.get()),"selected model deletion failed");
            v.mouse_drag_event(100,100,20,20,GLFW_MOUSE_BUTTON_RIGHT,GLFW_MOD_ALT);frame(v);
            require(same_matrix(transform,retained->manipulator()->matrix()),"deleted model was still manipulated");
            mouse(v.window(),GLFW_MOUSE_BUTTON_RIGHT,GLFW_RELEASE,GLFW_MOD_ALT);
            click(vec3(1.2f,0,0));require(other->renderer()->is_selected(),"selection did not recover after deletion");
            click(vec3(1.2f,0,0));require(!other->renderer()->is_selected(),"second click did not unpick model");
            v.resize(560,390);frame(v);click(vec3(1.2f,0,0));require(other->renderer()->is_selected(),"resized picking failed");
            position(vec3(1.2f,0,0));mouse(v.window(),GLFW_MOUSE_BUTTON_RIGHT,GLFW_PRESS,GLFW_MOD_ALT);
            transform=other->manipulator()->matrix();v.resize(580,410);frame(v);
            v.mouse_drag_event(100,100,30,30,GLFW_MOUSE_BUTTON_RIGHT,GLFW_MOD_ALT);
            require(same_matrix(transform,other->manipulator()->matrix()),"resize left stale model drag active");
            mouse(v.window(),GLFW_MOUSE_BUTTON_RIGHT,GLFW_RELEASE,GLFW_MOD_ALT);
            v.clear_scene();frame(v);require(v.models().empty(),"clear failed");
            require(v.device().validation_errors()==0,"manipulation validation errors");
        }
        retained->manipulator()->frame()->translate(vec3(.1f,0,0)); // no callback into the destroyed viewer
        // Frustum/eye-crossing and fully clipped box guides produce finite vertices.
        mat4 projection;projection.load_identity();projection(3,2)=1;projection(3,3)=.2f;
        const auto clipped=vulkan::shape::draw_box_wire(projection,320,240,1,true);
        for (const auto& vertex:clipped.geometry->vertices()) for (unsigned c=0;c<3;++c)
            require(std::isfinite(vertex.position[c]),"eye-crossing guide produced non-finite coordinates");
        auto outside=mat4::translation(vec3(10,0,0));
        require(vulkan::shape::draw_box_wire(outside,320,240).geometry->vertices().empty(),"offscreen guide was not clipped");
        std::cout<<"PASS: native object selection, shared transforms, constraints, depth/IDs/MSAA, occlusion and lifetime\n";
    } catch (const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
