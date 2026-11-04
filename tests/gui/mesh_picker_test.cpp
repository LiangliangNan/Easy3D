// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/gui/picker_surface_mesh.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/viewer/viewer.h>
#include <cmath>
#include <limits>
#include <type_traits>
#ifndef EASY3D_HAS_VULKAN
#include <easy3d/renderer/opengl.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#endif
using namespace easy3d;
using test::require;
struct GeometricPicker:SurfaceMeshPicker {
    explicit GeometricPicker(const Camera* c):SurfaceMeshPicker(c){use_gpu_if_supported_=false;}
};
SurfaceMesh::Face face(SurfaceMesh& mesh,const std::vector<vec3>& points) {
    std::vector<SurfaceMesh::Vertex> vertices;for(const auto& p:points)vertices.push_back(mesh.add_vertex(p));
    auto result=mesh.add_face(vertices);require(result.is_valid(),"cannot create mesh fixture");return result;
}
void math_tests() {
    Camera camera;camera.setScreenWidthAndHeight(400,300);camera.setViewDirection(vec3(0,0,-1));camera.setUpVector(vec3(0,1,0));
    camera.setType(Camera::ORTHOGRAPHIC);camera.setSceneBoundingBox(vec3(-2),vec3(2));camera.showEntireScene();
    static_assert(!std::is_copy_constructible<Picker>::value,"pickers own resources and must not be copied");
    GeometricPicker picker(&camera);
    int px=0,py=0;picker.screen_to_opengl(0,0,px,py,800,600);require(px==0 && py==599,"DPI top-left conversion differs");
    picker.screen_to_opengl(399,299,px,py,800,600);require(px==798 && py==1,"DPI bottom-right conversion differs");
    picker.screen_to_opengl(-1,-1,px,py,800,600);require(px==-2 && py==601,"outside conversion clamped into the viewport");
    bool invalid=false;try{Picker bad(nullptr);}catch(const std::invalid_argument&){invalid=true;}require(invalid,"null camera was accepted");
    SurfaceMesh mesh;
    const auto removed=face(mesh,{vec3(-1,-1,0),vec3(1,-1,0),vec3(0,1,0)});
    const auto rear=face(mesh,{vec3(-1,-1,-.5f),vec3(1,-1,-.5f),vec3(0,1,-.5f)});
    const auto front=face(mesh,{vec3(-1,-1,0),vec3(1,-1,0),vec3(0,1,0)});
    mesh.delete_face(removed);
    const auto center=picker.project(vec3(0));const int x=int(center.x),y=int(center.y);
    require(picker.pick_face(&mesh,x,y)==front,"nearest live face did not preserve original slot");
    const auto hit=picker.picked_point(&mesh,front,x,y);
    require(distance(hit,vec3(0))<1e-4f,"picked point differs from geometric ray intersection");
    require(picker.picked_face()==front,"picked face query lost its result");
    picker.set_resolution(20);
    const auto first=*mesh.vertices(front).begin();const auto vertex_pixel=picker.project(mesh.position(first));
    require(picker.pick_vertex(&mesh,front,int(vertex_pixel.x)+1,int(vertex_pixel.y)-1)==first,"vertex resolution/nearest projected vertex differs");
    auto halfedge=mesh.halfedge(front);
    const auto midpoint=(mesh.position(mesh.source(halfedge))+mesh.position(mesh.target(halfedge)))*.5f;
    const auto edge_pixel=picker.project(midpoint);
    require(picker.pick_edge(&mesh,front,int(edge_pixel.x),int(edge_pixel.y)).is_valid(),"edge sensitivity failed");
    SurfaceMesh other;
    face(other,{vec3(8,8,0),vec3(9,8,0),vec3(8,9,0)});face(other,{vec3(-8,-8,0),vec3(-9,-8,0),vec3(-8,-9,0)});
    const auto other_face=face(other,{vec3(-1,-1,0),vec3(1,-1,0),vec3(0,1,0)});
    require(!picker.pick_vertex(&other,other_face,x,y).is_valid(),"face result leaked across models");
    require(!picker.pick_face(&mesh,-1,y).is_valid() && !picker.picked_face().is_valid(),"outside query retained stale face");
    require(!picker.pick_face(nullptr,x,y).is_valid(),"null mesh retained stale face");
    mesh.set_manipulator(std::make_shared<Manipulator>(&mesh));mesh.manipulator()->frame()->translate(vec3(.4f,.25f,.3f));
    const auto moved=picker.project(vec3(.4f,.25f,.3f));
    require(picker.pick_face(&mesh,int(moved.x),int(moved.y))==front,"model transform was ignored");
    const auto moved_hit=picker.picked_point(&mesh,front,int(moved.x),int(moved.y));
    require(std::abs(moved_hit.z-.3f)<1e-5f,"picked point is not world space after manipulation");
    auto* clipping=ClippingPlane::instance();clipping->fit_scene(vec3(0,0,-.1f),1);clipping->set_enabled(true);clipping->set_cross_section(false);
    // Default normal +Z keeps the negative-normal side: front is clipped, rear survives.
    require(picker.pick_face(&mesh,int(moved.x),int(moved.y))==rear,"geometric pick ignored clipping");
    require(picker.pick_faces(&mesh,Rect(0,399,0,299))==std::vector<SurfaceMesh::Face>({rear}),"region selection ignored clipping");
    clipping->set_cross_section(true);clipping->set_cross_section_width(.01f);
    require(!picker.pick_face(&mesh,int(moved.x),int(moved.y)).is_valid(),"geometric pick ignored cross-section thickness");
    clipping->set_enabled(false);clipping->set_cross_section(false);
    mesh.manipulator()->frame()->rotate(quat(vec3(0,1,0),.6f));
    camera.setType(Camera::PERSPECTIVE);camera.showEntireScene();
    const auto rotated=picker.project(vec3(.4f,.25f,.3f));
    require(picker.pick_face(&mesh,int(rotated.x),int(rotated.y))==front,"perspective/rotated mesh was missed");
    const mat4 transform=mesh.manipulator()->matrix();
    const auto rotated_vertex=picker.project(transform*mesh.position(first));
    require(picker.pick_vertex(&mesh,front,int(rotated_vertex.x),int(rotated_vertex.y))==first,"vertex hit did not follow model rotation");
    camera.setType(Camera::ORTHOGRAPHIC);camera.showEntireScene();mesh.manipulator()->frame()->setOrientation(quat());
    SurfaceMesh warped;
    const auto warped_face=face(warped,{vec3(-1,-1,0),vec3(1,-1,0),vec3(1,1,.8f),vec3(-1,1,0)});
    const auto corner=picker.project(vec3(.9f,-.9f,0));
    require(picker.pick_face(&warped,int(corner.x),int(corner.y))==warped_face,"nonplanar polygon was missed");
    const auto warped_hit=picker.picked_point(&warped,warped_face,int(corner.x),int(corner.y));
    require(warped_hit.z<.12f,"picked point used a fitted plane instead of the tessellated surface");
    SurfaceMesh concave;
    const auto polygon=face(concave,{vec3(-1,-1,0),vec3(1,-1,0),vec3(1,1,0),vec3(.2f,1,0),vec3(.2f,0,0),vec3(-.2f,0,0),vec3(-.2f,1,0),vec3(-1,1,0)});
    const auto inside=picker.project(vec3(.7f,.5f,0)),notch=picker.project(vec3(0,.5f,0));
    require(picker.pick_face(&concave,int(inside.x),int(inside.y))==polygon,"concave face interior was missed");
    require(!picker.pick_face(&concave,int(notch.x),int(notch.y)).is_valid(),"concave notch was incorrectly filled");
    Polygon2 region;region.push_back(vec2(0,0));region.push_back(vec2(399,0));region.push_back(vec2(399,299));region.push_back(vec2(0,299));
    const auto all=picker.pick_faces(&mesh,region);require(all==std::vector<SurfaceMesh::Face>({rear,front}),"through-region selection lost deleted source slots");
    require(picker.pick_faces(&mesh,Rect(399,0,299,0))==all,"rectangle and polygon region semantics differ");
    Polygon2 boundary;for(auto vertex:other.vertices(other_face)) boundary.push_back(vec2(picker.project(other.position(vertex))));
    require(picker.pick_faces(&other,boundary)==std::vector<SurfaceMesh::Face>({other_face}),"polygon boundary vertices were excluded");
    require(picker.pick_faces(&mesh,Polygon2()).empty(),"empty polygon selected faces");
    Polygon2 degenerate;degenerate.push_back(vec2(0));degenerate.push_back(vec2(1));degenerate.push_back(vec2(2));
    require(picker.pick_faces(&mesh,degenerate).empty(),"degenerate region selected faces");
    region[0].x=std::numeric_limits<float>::quiet_NaN();require(picker.pick_faces(&mesh,region).empty(),"nonfinite region selected faces");
    // Completely beyond the camera's far plane must not be selected in projection.
    SurfaceMesh far;face(far,{vec3(-1,-1,-10000),vec3(1,-1,-10000),vec3(0,1,-10000)});
    require(picker.pick_faces(&far,Rect(0,399,0,299)).empty(),"region selection ignored the camera frustum");
    require(!picker.pick_face(&far,x,y).is_valid(),"ray selection extended beyond the camera frustum");
    mesh.delete_face(front);require(!picker.pick_face(&mesh,int(moved.x),int(moved.y)).is_valid() || picker.picked_face()==rear,"deleted face was selected");
}
#ifndef EASY3D_HAS_VULKAN
void gpu_tests() {
    Viewer a("Mesh picker A",161,121,4,false,false,true),b("Mesh picker B",171,131,4,false,false,true);
    auto first=std::make_shared<SurfaceMesh>(),second=std::make_shared<SurfaceMesh>();
    auto dead=face(*first,{vec3(-1,-1,0),vec3(1,-1,0),vec3(0,1,0)});
    auto live=face(*first,{vec3(-1,-1,0),vec3(1,-1,0),vec3(0,1,0)});first->delete_face(dead);
    auto other=face(*second,{vec3(-1,-1,0),vec3(1,-1,0),vec3(0,1,0)});
    a.add_model(first);b.add_model(second);
    for(auto* v:{&a,&b}) {v->set_usage("");v->camera()->setViewDirection(vec3(0,0,-1));v->camera()->setUpVector(vec3(0,1,0));v->fit_screen();require(v->run_frames(1)==0,"picker viewer frame failed");}
    auto pa=std::unique_ptr<SurfaceMeshPicker>(new SurfaceMeshPicker(a.camera()));
    auto pb=std::unique_ptr<SurfaceMeshPicker>(new SurfaceMeshPicker(b.camera()));
    auto pick=[&](Viewer& v,SurfaceMeshPicker& p,SurfaceMesh* m,SurfaceMesh::Face f) {
        glfwMakeContextCurrent(v.window());glViewport(0,0,v.framebuffer_width(),v.framebuffer_height());
        const auto pixel=p.project(vec3(0));require(p.pick_face(m,int(pixel.x),int(pixel.y))==f,"native picker lost source face or crossed framebuffer contexts");
        require(distance(p.picked_point(m,f,int(pixel.x),int(pixel.y)),vec3(0))<.03f,"native picked point differs from geometry");
    };
    pick(a,*pa,first.get(),live);pick(b,*pb,second.get(),other);pick(a,*pa,first.get(),live);
    // Queries must ignore caller render state and pixel-pack buffers, then restore it.
    glfwMakeContextCurrent(a.window());GLuint pack=0;glGenBuffers(1,&pack);glBindBuffer(GL_PIXEL_PACK_BUFFER,pack);
    glBufferData(GL_PIXEL_PACK_BUFFER,128,nullptr,GL_STATIC_DRAW);glPixelStorei(GL_PACK_ROW_LENGTH,7);glPixelStorei(GL_PACK_SKIP_ROWS,2);
    glViewport(3,4,a.framebuffer_width()-6,a.framebuffer_height()-8);glScissor(0,0,1,1);
    glEnable(GL_SCISSOR_TEST);glEnable(GL_BLEND);glEnable(GL_COLOR_LOGIC_OP);glLogicOp(GL_XOR);glEnable(GL_RASTERIZER_DISCARD);
    glDepthMask(GL_FALSE);glDepthFunc(GL_GREATER);glColorMask(GL_FALSE,GL_TRUE,GL_FALSE,GL_TRUE);glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    glClearColor(.1f,.2f,.3f,.4f);glActiveTexture(GL_TEXTURE3);
    const auto center=pa->project(vec3(0));require(pa->pick_face(first.get(),int(center.x),int(center.y))==live,"caller render state changed mesh picking");
    GLint vp[4],bound=0,row=0,skip=0,active=0,depth=0,mode[2];GLboolean mask[4],depth_mask=GL_TRUE;GLfloat clear[4];
    glGetIntegerv(GL_VIEWPORT,vp);glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING,&bound);glGetIntegerv(GL_PACK_ROW_LENGTH,&row);
    glGetIntegerv(GL_PACK_SKIP_ROWS,&skip);glGetIntegerv(GL_ACTIVE_TEXTURE,&active);glGetIntegerv(GL_DEPTH_FUNC,&depth);glGetIntegerv(GL_POLYGON_MODE,mode);
    glGetBooleanv(GL_COLOR_WRITEMASK,mask);glGetBooleanv(GL_DEPTH_WRITEMASK,&depth_mask);glGetFloatv(GL_COLOR_CLEAR_VALUE,clear);
    require(vp[0]==3 && vp[1]==4 && vp[2]==a.framebuffer_width()-6 && vp[3]==a.framebuffer_height()-8 && GLuint(bound)==pack && row==7 && skip==2 && active==GL_TEXTURE3 && depth==GL_GREATER && mode[0]==GL_LINE &&
        !depth_mask && !mask[0] && mask[1] && !mask[2] && mask[3] && std::abs(clear[0]-.1f)<1e-6f && std::abs(clear[3]-.4f)<1e-6f &&
        glIsEnabled(GL_SCISSOR_TEST) && glIsEnabled(GL_BLEND) && glIsEnabled(GL_COLOR_LOGIC_OP) && glIsEnabled(GL_RASTERIZER_DISCARD),"mesh query leaked OpenGL state");
    glBindBuffer(GL_PIXEL_PACK_BUFFER,0);glDeleteBuffers(1,&pack);glPixelStorei(GL_PACK_ROW_LENGTH,0);glPixelStorei(GL_PACK_SKIP_ROWS,0);
    glDisable(GL_SCISSOR_TEST);glDisable(GL_BLEND);glDisable(GL_COLOR_LOGIC_OP);glDisable(GL_RASTERIZER_DISCARD);glDepthMask(GL_TRUE);glDepthFunc(GL_LESS);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);glActiveTexture(GL_TEXTURE0);
    glfwMakeContextCurrent(b.window());pb.reset();glfwMakeContextCurrent(a.window());pa.reset();
    require(glGetError()==GL_NO_ERROR,"native mesh picker generated OpenGL errors");
}
#endif
int main(int argc,char**) {return test::run("GUI mesh picking and geometric regions",[&]{
#ifndef EASY3D_HAS_VULKAN
    if(argc>1){gpu_tests();return;}
#endif
    math_tests();
});}
