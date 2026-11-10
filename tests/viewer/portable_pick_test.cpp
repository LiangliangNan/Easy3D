// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <tutorials/Tutorial_401_ModelPicker/viewer.h>
#include <tutorials/Tutorial_402_FacePicker/viewer.h>
#include <tutorials/Tutorial_405_ObjectManipulation/viewer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/fileio/image_io.h>
#include <cmath>
#ifndef EASY3D_HAS_VULKAN
#include <easy3d/renderer/opengl.h>
#endif
using namespace easy3d;
using test::require;
std::shared_ptr<SurfaceMesh> quad(float z) {
    auto m=std::make_shared<SurfaceMesh>();
    auto a=m->add_vertex(vec3(-1,-1,z)),b=m->add_vertex(vec3(1,-1,z));
    auto c=m->add_vertex(vec3(1,1,z)),d=m->add_vertex(vec3(-1,1,z));m->add_quad(a,b,c,d);return m;
}
void faces_only(Model* model) {
    for(const auto& d:model->renderer()->points_drawables()) d->set_visible(false);
    for(const auto& d:model->renderer()->lines_drawables()) d->set_visible(false);
    for(const auto& d:model->renderer()->triangles_drawables()) d->set_lighting(false);
}
void view(Viewer& v) {
    v.camera()->setType(Camera::ORTHOGRAPHIC);v.camera()->setUpVector(vec3(0,1,0));
    v.camera()->setViewDirection(vec3(0,0,-1));v.fit_screen();
}
Viewer::PickResult hit(Viewer& v,const vec3& point) {
    const auto p=v.world_to_window(point);
    return v.pick(int(p.x),int(p.y));
}
void exercise(Viewer& v) {
    auto distant=quad(0),close=quad(.5f);v.add_model(distant);v.add_model(close);
    faces_only(distant.get());faces_only(close.get());view(v);
    const auto matrix=v.camera()->modelViewProjectionMatrix();
#ifndef EASY3D_HAS_VULKAN
    GLuint copy_buffer;glGenBuffers(1,&copy_buffer);glBindBuffer(GL_COPY_READ_BUFFER,copy_buffer);
    glBufferData(GL_COPY_READ_BUFFER,16,nullptr,GL_STATIC_DRAW);
    GLuint texture;glGenTextures(1,&texture);glActiveTexture(GL_TEXTURE3);glBindTexture(GL_TEXTURE_2D,texture);
    glViewport(3,4,31,27);glDepthFunc(GL_GREATER);glColorMask(GL_FALSE,GL_TRUE,GL_FALSE,GL_TRUE);
#endif
    auto result=hit(v,vec3(-.6f,.2f,.5f));
#ifndef EASY3D_HAS_VULKAN
    GLint viewport[4],binding=0,depth_func=0,active=0,bound_texture=0;GLboolean mask[4];
    glGetIntegerv(GL_ACTIVE_TEXTURE,&active);glGetIntegerv(GL_TEXTURE_BINDING_2D,&bound_texture);
    glGetIntegerv(GL_VIEWPORT,viewport);glGetIntegerv(GL_COPY_READ_BUFFER_BINDING,&binding);
    glGetIntegerv(GL_DEPTH_FUNC,&depth_func);glGetBooleanv(GL_COLOR_WRITEMASK,mask);
    require(viewport[0]==3 && viewport[1]==4 && viewport[2]==31 && viewport[3]==27 &&
            active==GL_TEXTURE3 && GLuint(bound_texture)==texture && GLuint(binding)==copy_buffer && depth_func==GL_GREATER && !mask[0] && mask[1] && !mask[2] && mask[3],
            "picking leaked OpenGL state");
    glBindBuffer(GL_COPY_READ_BUFFER,0);glDeleteBuffers(1,&copy_buffer);
    glBindTexture(GL_TEXTURE_2D,0);glDeleteTextures(1,&texture);glActiveTexture(GL_TEXTURE0);
#endif
    require(result.model==close,"picking followed model insertion order instead of depth");
    require(result.drawable.get()==close->renderer()->get_triangles_drawable("faces"),"picked drawable differs");
    require(result.depth>=0 && result.depth<1 && std::abs(result.position.z-.5f)<2e-4f,"picked depth/world point differs");
    require(result.face_index==0 && result.primitive<2,"polygon triangle provenance missing");
    {
        const auto logical=v.world_to_window(vec3(-.6f,.2f,.5f));
        const int qx=int(logical.x),qy=int(logical.y);
        bool direct_found=false,base_found=false;
        const auto direct=v.point_under_pixel(qx,qy,direct_found);
        const ViewerBase& base=v;
        const auto inherited=base.point_under_pixel(qx,qy,base_found);
        const auto query_pick=v.pick(qx,qy);
        require(direct_found && base_found && query_pick && (direct-inherited).length()<1e-6f &&
                (direct-query_pick.position).length()<2e-4f,"inherited logical surface query differs from logical pick");
        bool outside=true;v.point_under_pixel(v.width(),0,outside);
        require(!outside,"logical outside pixel was accepted");
    }
    const auto projected=v.world_to_window(result.position);
    require(std::abs(projected.z-result.depth)<2e-5f,"returned world point does not reproduce depth");
    for(unsigned i=0;i<16;++i) require(std::abs(matrix[i]-v.camera()->modelViewProjectionMatrix()[i])<1e-6f,"picking changed camera");
    require(!v.pick(-1,0) && !v.pick(v.width(),0),"out-of-bounds pick succeeded");
    close->renderer()->set_visible(false);require(hit(v,vec3(-.6f,.2f,0)).model==distant,"hidden model occluded picking");
    auto* plane=ClippingPlane::instance();plane->fit_scene(vec3(0,0,-.25f),2);plane->set_enabled(true);
    require(!hit(v,vec3(-.6f,.2f,0)),"clipped surface was picked");plane->set_enabled(false);
    close->renderer()->set_visible(true);
    close->set_manipulator(std::make_shared<Manipulator>(close.get()));
    close->manipulator()->frame()->translate(vec3(.2f,0,.2f));
    const vec3 moved=(close->manipulator()->matrix()*vec4(-.6f,.2f,.5f,1)).xyz();
    result=hit(v,moved);require(result.model==close && std::abs(result.position.z-moved.z)<2e-4f,"picking ignored manipulation");
    std::weak_ptr<Drawable> retained=result.drawable;std::weak_ptr<Model> retained_model=close;
    v.clear_scene();close.reset();distant.reset();
    require(!retained.expired() && !retained_model.expired(),"pick did not retain source ownership");
    result={};require(retained.expired() && retained_model.expired(),"pick ownership leaked sources");
    auto points=std::make_shared<PointsDrawable>("standalone spheres");v.add_drawable(points);
    points->update_vertex_buffer({vec3(0)});points->set_point_size(30);points->set_impostor_type(PointsDrawable::SPHERE);
    points->set_lighting(false);v.camera()->setSceneBoundingBox(vec3(-1),vec3(1));v.camera()->showEntireScene();
    result=hit(v,vec3(0));
    require(result && !result.model && result.drawable==points,"standalone point drawable was not picked");
    require(result.depth<1 && result.position.z>0,"sphere pick did not use analytic front-surface depth");
    v.clear_scene();result={};points.reset();
    auto lines=std::make_shared<LinesDrawable>("standalone line");v.add_drawable(lines);
    lines->update_vertex_buffer({vec3(-.8f,0,0),vec3(.8f,0,0)});lines->set_lighting(false);
    const auto pixel=v.world_to_window(vec3(0));
    const int lx=int(pixel.x),ly=int(pixel.y);
    for(int dy=-1;dy<=1 && !result;++dy) result=v.pick(lx,ly+dy);
    require(result && result.drawable==lines && !result.model,"standalone line was not picked");
    v.clear_scene();result={};lines.reset();
    auto hud=std::make_shared<LinesDrawable2D>("HUD");v.add_drawable(hud);
    hud->update_vertex_buffer({vec2(0,0),vec2(float(v.framebuffer_width()),float(v.framebuffer_height()))},v.framebuffer_width(),v.framebuffer_height());
    require(!hit(v,vec3(0)),"HUD produced a scene pick");v.clear_scene();
}
void sphere_snapshots(Viewer& v) {
    auto points=std::make_shared<PointsDrawable>("snapshot sphere");
    points->set_point_size(40);points->set_impostor_type(PointsDrawable::SPHERE);
    points->set_lighting(false);points->update_texcoord_buffer({vec2(.5f)});v.add_drawable(points);
    std::unique_ptr<Texture> texture(Texture::create({255,0,0,255},1,1,4));
    require(bool(texture),"sphere texture creation failed");
    for (auto projection:{Camera::PERSPECTIVE,Camera::ORTHOGRAPHIC}) {
        v.camera()->setType(projection);v.camera()->setSceneBoundingBox(vec3(-1),vec3(1));v.camera()->showEntireScene();
        for (bool textured:{false,true}) {
            const vec3 center=textured?vec3(.7f,.3f,0):vec3(0);
            points->update_vertex_buffer({center});
            if (textured) points->set_texture_coloring(State::VERTEX,"v:texcoord",texture.get());
            else points->set_uniform_coloring(vec4(1,0,0,1));
            const float radius=points->point_size()*v.camera()->pixelGLRatio(v.camera()->pivotPoint())*.5f;
            const auto inv=inverse(v.camera()->modelViewProjectionMatrix());
            for (float scale:{1.f,1.5f}) {
                const std::string file="sphere-"+std::to_string(int(projection))+"-"+std::to_string(textured)+
                    "-"+std::to_string(v.samples())+"-"+std::to_string(scale)+".png";
                require(v.snapshot(file,scale,v.samples(),1),"sphere snapshot failed");
                std::vector<unsigned char> rgba;int w=0,h=0,channels=0;
                require(ImageIO::load(file,rgba,w,h,channels,4,false),"sphere snapshot could not be decoded");
                unsigned interior=0,mismatches=0;
                for (int y=0;y<h;++y) for (int x=0;x<w;++x) {
                    const vec4 near=inv*vec4(2*(x+.5f)/w-1,1-2*(y+.5f)/h,-1,1);
                    const vec4 far=inv*vec4(2*(x+.5f)/w-1,1-2*(y+.5f)/h,1,1);
                    const vec3 origin=near.xyz()/near.w,direction=normalize(far.xyz()/far.w-origin);
                    const vec3 offset=center-origin,perpendicular=cross(offset,direction);
                    const float discriminant=radius*radius-dot(perpendicular,perpendicular);
                    if (std::abs(discriminant)<radius*radius*.05f) continue;
                    const bool expected=dot(offset,direction)>0 && discriminant>0;
                    const auto* pixel=rgba.data()+4*(size_t(y)*w+x);
                    const bool observed=pixel[0]>pixel[1]+32 && pixel[0]>pixel[2]+32;
                    interior+=expected;mismatches+=expected!=observed;
                }
                if (interior<=100 || mismatches>5)
                    throw std::runtime_error(file+": sphere silhouette differs from ray/sphere oracle (interior="+
                                             std::to_string(interior)+", mismatches="+std::to_string(mismatches)+")");
            }
            const auto window=v.world_to_window(center);
            const int x=int(window.x),y=int(window.y);
            const auto picked=v.pick(x,y);const auto captured=v.capture();
            const int px=int(double(x)*captured.width/v.width()),py=int(double(y)*captured.height/v.height());
            require(picked && std::abs(picked.depth-captured.depth[size_t(py)*captured.width+px])<1e-5f,
                    "sphere capture and logical picking use different radii");
            require(std::abs((picked.position-center).norm()-radius)<1e-3f,"sphere pick depth disagrees with its world radius");
        }
    }
    v.clear_scene();
}
struct ModelPickerView:TutorialModelPicker {using TutorialModelPicker::TutorialModelPicker;using TutorialModelPicker::mouse_press_event;};
struct FacePickerView:TutorialFacePicker {using TutorialFacePicker::TutorialFacePicker;using TutorialFacePicker::mouse_press_event;};
struct ManipulationView:TutorialObjectManipulation {
    using TutorialObjectManipulation::TutorialObjectManipulation;
    using TutorialObjectManipulation::mouse_press_event;
    using TutorialObjectManipulation::mouse_release_event;
    using TutorialObjectManipulation::mouse_drag_event;
    using TutorialObjectManipulation::focus_event;
    using TutorialObjectManipulation::post_resize;
};
void tutorials() {
    {
        ManipulationView v("Portable Tutorial 405");auto m=quad(0);v.add_model(m);faces_only(m.get());view(v);
        const auto p=v.camera()->projectedCoordinatesOf(vec3(-.6f,.2f,0));
        const int x=int(p.x),y=int(p.y);
        v.mouse_press_event(x,y,Viewer::BUTTON_LEFT,0);
        v.mouse_release_event(x,y,Viewer::BUTTON_LEFT,0);
        require(m->renderer()->is_selected() && m->manipulator(),"Tutorial 405 selection missed at native DPI");
        const auto camera=v.camera()->modelViewProjectionMatrix();
        const auto before=m->manipulator()->matrix();
        v.mouse_press_event(x,y,Viewer::BUTTON_RIGHT,Viewer::MODIF_ALT);
        v.mouse_drag_event(x+20,y+10,20,10,Viewer::BUTTON_RIGHT,Viewer::MODIF_ALT);
        v.mouse_release_event(x+20,y+10,Viewer::BUTTON_RIGHT,Viewer::MODIF_ALT);
        const auto translated=m->manipulator()->matrix();
        require((translated*vec4(0,0,0,1)-before*vec4(0,0,0,1)).length()>1e-4f,"Tutorial 405 did not translate model");
        for(unsigned i=0;i<16;++i) require(camera[i]==v.camera()->modelViewProjectionMatrix()[i],"object drag moved camera");
        v.mouse_press_event(x,y,Viewer::BUTTON_RIGHT,Viewer::MODIF_ALT);v.focus_event(false);
        v.mouse_drag_event(x+20,y+10,20,10,Viewer::BUTTON_RIGHT,Viewer::MODIF_ALT);
        for(unsigned i=0;i<16;++i) require(translated[i]==m->manipulator()->matrix()[i],"focus loss did not cancel model drag");
        v.mouse_release_event(x,y,Viewer::BUTTON_RIGHT,Viewer::MODIF_ALT);
        v.mouse_press_event(x,y,Viewer::BUTTON_RIGHT,Viewer::MODIF_ALT);v.post_resize(v.width(),v.height());
        v.mouse_drag_event(x+20,y+10,20,10,Viewer::BUTTON_RIGHT,Viewer::MODIF_ALT);
        for(unsigned i=0;i<16;++i) require(translated[i]==m->manipulator()->matrix()[i],"resize did not cancel model drag");
        v.mouse_press_event(x,y,Viewer::BUTTON_RIGHT,Viewer::MODIF_ALT);v.delete_model(m.get());
        v.mouse_drag_event(x+20,y+10,20,10,Viewer::BUTTON_RIGHT,Viewer::MODIF_ALT);
        for(unsigned i=0;i<16;++i) require(translated[i]==m->manipulator()->matrix()[i],"deleted model was manipulated");
    }

    {
        ModelPickerView v("Portable Tutorial 401");auto m=quad(0);v.add_model(m);faces_only(m.get());view(v);
        const auto p=v.camera()->projectedCoordinatesOf(vec3(-.6f,.2f,0));
        v.mouse_press_event(int(p.x),int(p.y),Viewer::BUTTON_LEFT,0);
        require(m->renderer()->is_selected(),"Tutorial 401 click missed at native DPI");
        v.mouse_press_event(int(p.x),int(p.y),Viewer::BUTTON_RIGHT,0);
        require(m->renderer()->is_selected(),"right click toggled selection");
        v.mouse_press_event(int(p.x),int(p.y),Viewer::BUTTON_LEFT,0);
        require(!m->renderer()->is_selected(),"second click did not unselect model");
    }
    {
        FacePickerView v("Portable Tutorial 402");auto m=quad(0);v.Viewer::add_model(m);faces_only(m.get());view(v);
        const auto p=v.camera()->projectedCoordinatesOf(vec3(-.6f,.2f,0));
        v.mouse_press_event(int(p.x),int(p.y),Viewer::BUTTON_LEFT,0);
        auto* faces=m->renderer()->get_triangles_drawable("faces");
        require(faces->highlight() && faces->highlight_range()==std::make_pair(0,1),"Tutorial 402 DPI conversion lost polygon highlight");
    }
}
int main(int argc,char**) {return test::run("portable picking depth, provenance, ownership, styles and tutorial input",[&] {
#ifdef EASY3D_HAS_VULKAN
    VulkanOptions options;options.validation=options.sync_validation=argc>1;
#else
    OpenGLOptions options;
#endif
    for(int samples:{1,4}) {
        Viewer v("Portable pick",241,181,samples,false,false,true,options);sphere_snapshots(v);exercise(v);
        require(v.validation_errors()==0,"picking validation errors");
    }
    tutorials();
});}
