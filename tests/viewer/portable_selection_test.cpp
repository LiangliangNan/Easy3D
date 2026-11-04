// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/core/point_cloud.h>
#include <tutorials/Tutorial_403_PointSelection/viewer.h>
#include <limits>
#include <cmath>
#ifndef EASY3D_HAS_VULKAN
#include <easy3d/renderer/opengl.h>
#endif
using namespace easy3d;
using test::require;
struct SelectionView:TutorialPointSelection {
    using TutorialPointSelection::TutorialPointSelection;
    using TutorialPointSelection::mouse_press_event;using TutorialPointSelection::mouse_release_event;
    using TutorialPointSelection::mouse_drag_event;using TutorialPointSelection::key_press_event;
    using TutorialPointSelection::focus_event;
};
int main(int argc,char**) {return test::run("portable GPU selection and shared point storage",[&]{
#ifdef EASY3D_HAS_VULKAN
    VulkanOptions options;options.validation=options.sync_validation=argc>1;
#else
    OpenGLOptions options;
#endif
    std::shared_ptr<const graphics::PointData> surviving;
    {
    Viewer v("GPU selection",161,121,4,false,false,true,options);v.set_usage("");
    auto points=std::make_shared<PointsDrawable>("shared positions");
    std::vector<vec3> positions(70);
    for(size_t i=0;i<positions.size();++i) positions[i]=vec3(i%2? .5f:-.5f,0,0);
    positions[3]=vec3(0,2,0);positions[5]=vec3(0,0,2);
#ifndef EASY3D_HAS_VULKAN
    positions[3]=vec3(std::numeric_limits<float>::quiet_NaN(),0,0);
#endif
    points->update_vertex_buffer(positions);points->set_lighting(false);v.add_drawable(points);
    graphics::PointSelectionRequest request;request.width=200;request.height=100;
    request.polygon={{0,0},{100,0},{100,100},{0,100}};request.points=v.point_resource(points.get());
    require(request.points->source_count()==70,"point source count changed");
    const auto original=request.points;
#ifndef EASY3D_HAS_VULKAN
    require(original->positions().empty(),"OpenGL point resource duplicated CPU positions");
#endif
    auto words=v.select_points(request);
    require(words==std::vector<uint32_t>({0x55555555,0x55555555,0x15}),"GPU polygon test/packed word ownership differs");
    request.words=words;request.deselect=true;require(v.select_points(request)==std::vector<uint32_t>(3,0),"GPU deselection failed");
    request.deselect=false;request.words={0,0,0xffffffc0};words=v.select_points(request);
    require(words.back()==0xffffffd5,"GPU selection changed padding bits");
    request.words.clear();request.local_clipping_planes={vec4(1,0,0,0)};
    require(v.select_points(request)==std::vector<uint32_t>(3,0),"local clipping was ignored");request.local_clipping_planes.clear();
    request.polygon={{0,0},{200,0},{200,100},{0,100}};
    words=v.select_points(request);require((words[0]&(1u<<3))==0 && (words[0]&(1u<<5))==0,"nonfinite/out-of-frustum points were selected");
    request.polygon={{0,0},{100,0}};require(v.select_points(request)==std::vector<uint32_t>(3,0),"degenerate polygon selected points");
    request.polygon={{0,0},{200,0},{200,100},{0,100}};
    request.words={0};bool invalid=false;try{v.select_points(request);}catch(const std::invalid_argument&){invalid=true;}
    require(invalid,"invalid bitset size was accepted");request.words.clear();
    request.polygon[0].x=std::numeric_limits<float>::infinity();invalid=false;
    try{v.select_points(request);}catch(const std::invalid_argument&){invalid=true;}
    require(invalid,"nonfinite polygon was accepted");request.polygon[0].x=0;
    request.local_clipping_planes={vec4(0)};invalid=false;
    try{v.select_points(request);}catch(const std::invalid_argument&){invalid=true;}
    require(invalid,"zero-normal clipping plane was accepted");request.local_clipping_planes.clear();
    points->update_color_buffer(std::vector<vec3>(70,vec3(1,0,0)));
    require(v.point_resource(points.get())==original,"attribute update replaced the point resource");
    points->update_vertex_buffer(std::vector<vec3>(70,vec3(2,0,0)));
    const auto replacement=v.point_resource(points.get());require(replacement!=original,"geometry update reused stale point storage");
    request.points=replacement;require(v.select_points(request)==std::vector<uint32_t>(3,0),"replacement geometry did not reach selection");
    request.points=original;require(v.select_points(request)==words,"retained GPU point snapshot changed after VBO replacement");
    v.delete_drawable(points.get());points.reset();require(v.select_points(request)==words,"drawable deletion invalidated retained GPU snapshot");
    surviving=original;
    auto indexed=std::make_shared<PointsDrawable>("indexed points");indexed->update_vertex_buffer({vec3(0),vec3(.2f,0,0),vec3(-.2f,0,0)});
    indexed->update_element_buffer(std::vector<unsigned>{2,2,0});v.add_drawable(indexed);
    request.points=v.point_resource(indexed.get());require(v.select_points(request)==std::vector<uint32_t>{5},"indexed source IDs/duplicates changed selection slots");
    indexed->disable_element_buffer();request.points=v.point_resource(indexed.get());require(v.select_points(request)==std::vector<uint32_t>{7},"disabling indices retained stale membership");
#ifndef EASY3D_HAS_VULKAN
    GLuint feedback_object=0;
    if(GLEW_VERSION_4_0 || GLEW_ARB_transform_feedback2){glGenTransformFeedbacks(1,&feedback_object);glBindTransformFeedback(GL_TRANSFORM_FEEDBACK,feedback_object);}
    GLuint tf=0,textures[3]={};glGenBuffers(1,&tf);glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER,tf);glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,64,nullptr,GL_STATIC_DRAW);
    glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER,0,tf,16,32);
    glGenTextures(3,textures);for(unsigned i=0;i<3;++i){glActiveTexture(GL_TEXTURE0+i);glBindTexture(GL_TEXTURE_BUFFER,textures[i]);}
    glActiveTexture(GL_TEXTURE3);glEnable(GL_RASTERIZER_DISCARD);v.select_points(request);
    GLint active=0,index=0;GLint64 start=0,size=0;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);glGetIntegeri_v(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING,0,&index);
    glGetInteger64i_v(GL_TRANSFORM_FEEDBACK_BUFFER_START,0,&start);glGetInteger64i_v(GL_TRANSFORM_FEEDBACK_BUFFER_SIZE,0,&size);
    require(active==GL_TEXTURE3 && GLuint(index)==tf && start==16 && size==32 && glIsEnabled(GL_RASTERIZER_DISCARD),"selection leaked TF/active texture/rasterizer state");
    for(unsigned i=0;i<3;++i){GLint texture=0;glActiveTexture(GL_TEXTURE0+i);glGetIntegerv(GL_TEXTURE_BINDING_BUFFER,&texture);require(GLuint(texture)==textures[i],"selection leaked texture buffer binding");glBindTexture(GL_TEXTURE_BUFFER,0);}
    if(feedback_object){GLint bound=0;glGetIntegerv(GL_TRANSFORM_FEEDBACK_BINDING,&bound);require(GLuint(bound)==feedback_object,"selection replaced caller transform feedback object");}
    glDeleteTextures(3,textures);glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER,0,0);glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER,0);glDeleteBuffers(1,&tf);glDisable(GL_RASTERIZER_DISCARD);
    if(feedback_object){glBindTransformFeedback(GL_TRANSFORM_FEEDBACK,0);glDeleteTransformFeedbacks(1,&feedback_object);}
    require(glGetError()==GL_NO_ERROR,"selection generated OpenGL errors");
#endif
    require(v.validation_errors()==0,"GPU selection validation errors");
    }
    surviving.reset(); // Safe even after the owning device/context is gone.
    SelectionView tutorial("Shared selection tutorial");tutorial.resize(201,151);
    auto cloud=std::make_shared<PointCloud>();const auto dead=cloud->add_vertex(vec3(0));
    const auto left=cloud->add_vertex(vec3(-.5f,0,0)),right=cloud->add_vertex(vec3(.5f,0,0));
    const auto behind=cloud->add_vertex(vec3(-.5f,0,-.2f));cloud->delete_vertex(dead);tutorial.add_model(cloud);
    tutorial.camera()->setViewDirection(vec3(0,0,-1));tutorial.camera()->setUpVector(vec3(0,1,0));tutorial.camera()->setType(Camera::ORTHOGRAPHIC);tutorial.fit_screen();
    require(tutorial.run_frames(2)==0,"tutorial frame failed");
    const auto p=tutorial.camera()->projectedCoordinatesOf(cloud->position(left));
    auto gesture=[&](int button){int x=int(p.x),y=int(p.y);tutorial.mouse_press_event(x-15,y-15,button,Viewer::MODIF_SHIFT);
        tutorial.mouse_drag_event(x+15,y-15,30,0,button,0);tutorial.mouse_drag_event(x+15,y+15,0,30,button,0);
        tutorial.mouse_drag_event(x-15,y+15,-30,0,button,0);tutorial.mouse_release_event(x-15,y+15,button,0);};
    const auto tutorial_resource=tutorial.point_resource(cloud->renderer()->get_points_drawable("vertices"));
    auto selected=cloud->vertex_property<bool>("v:select");gesture(Viewer::BUTTON_LEFT);
    require(selected[left] && selected[behind] && !selected[right] && !selected[dead],"shared tutorial did not select live/occluded original slots at native DPI");
    gesture(Viewer::BUTTON_RIGHT);require(!selected[left] && !selected[behind],"shared tutorial did not deselect");
    require(tutorial.point_resource(cloud->renderer()->get_points_drawable("vertices"))==tutorial_resource,"selection colors replaced shared point positions");
    tutorial.key_press_event(Viewer::KEY_R,0);
    tutorial.mouse_press_event(int(p.x)-15,int(p.y)-15,Viewer::BUTTON_LEFT,Viewer::MODIF_SHIFT);
    tutorial.mouse_release_event(int(p.x)+15,int(p.y)+15,Viewer::BUTTON_LEFT,0);
    require(selected[left] && selected[behind] && !selected[right],"shared rectangle did not select through the cloud");
    tutorial.mouse_press_event(int(p.x)-15,int(p.y)-15,Viewer::BUTTON_RIGHT,Viewer::MODIF_SHIFT);
    tutorial.mouse_release_event(int(p.x)+15,int(p.y)+15,Viewer::BUTTON_RIGHT,0);tutorial.key_press_event(Viewer::KEY_L,0);
    tutorial.mouse_press_event(int(p.x)-15,int(p.y)-15,Viewer::BUTTON_LEFT,Viewer::MODIF_SHIFT);
    tutorial.key_press_event(Viewer::KEY_ESCAPE,0);tutorial.mouse_release_event(int(p.x)+15,int(p.y)+15,Viewer::BUTTON_LEFT,0);
    require(!selected[left] && !tutorial.should_close(),"Escape failed to cancel selection");
    tutorial.mouse_press_event(int(p.x)-15,int(p.y)-15,Viewer::BUTTON_LEFT,Viewer::MODIF_SHIFT);tutorial.focus_event(false);
    tutorial.mouse_release_event(int(p.x)+15,int(p.y)+15,Viewer::BUTTON_LEFT,0);require(!selected[left],"focus cancellation selected points");
    require(tutorial.run_frames(2)==0 && tutorial.validation_errors()==0,"shared tutorial final frame/validation failed");
});}
