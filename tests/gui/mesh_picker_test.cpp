// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/gui/picker_surface_mesh.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
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
SurfaceMesh::Face face(SurfaceMesh& mesh,const std::vector<vec3>& points) {
    std::vector<SurfaceMesh::Vertex> vertices;for(const auto& p:points)vertices.push_back(mesh.add_vertex(p));
    auto result=mesh.add_face(vertices);require(result.is_valid(),"cannot create mesh fixture");return result;
}
void math_tests() {
    Camera camera;camera.set_screen_width_and_height(400,300);camera.set_view_direction(vec3(0,0,-1));camera.set_up_vector(vec3(0,1,0));
    camera.set_type(Camera::ORTHOGRAPHIC);camera.set_scene_bounding_box(vec3(-2),vec3(2));camera.show_entire_scene();
    static_assert(!std::is_copy_constructible<Picker>::value,"pickers own resources and must not be copied");
    SurfaceMeshPicker picker(&camera);
    int px=0,py=0;picker.screen_to_opengl(0,0,px,py,800,600);require(px==0 && py==599,"DPI top-left conversion differs");
    picker.screen_to_opengl(399,299,px,py,800,600);require(px==798 && py==1,"DPI bottom-right conversion differs");
    picker.screen_to_opengl(-1,-1,px,py,800,600);require(px==-2 && py==601,"outside conversion clamped into the viewport");
    bool invalid=false;try{Picker bad(nullptr);}catch(const std::invalid_argument&){invalid=true;}require(invalid,"null camera was accepted");

    SurfaceMesh mesh;
    const auto removed=face(mesh,{vec3(-1,-1,0),vec3(1,-1,0),vec3(0,1,0)});
    const auto rear=face(mesh,{vec3(-1,-1,-.5f),vec3(1,-1,-.5f),vec3(0,1,-.5f)});
    const auto front=face(mesh,{vec3(-1,-1,0),vec3(1,-1,0),vec3(0,1,0)});
    mesh.delete_face(removed);
    const auto center=picker.camera()->projected_coordinates_of(vec3(0));const int x=int(center.x),y=int(center.y);
    // Face picking needs a rendered face drawable; a detached mesh cannot be picked.
    require(!picker.pick_face(&mesh,x,y).is_valid() && !picker.picked_face().is_valid(),
            "mesh without a drawable was picked");
    require(picker.picked_point(&mesh,front,x,y)==vec3(0),"invalid pick exposed a surface point");
    require(!picker.pick_face(&mesh,-1,y).is_valid(),"outside query retained a stale face");
    require(!picker.pick_face(nullptr,x,y).is_valid(),"null mesh retained a stale face");

    // Region selection stays geometric and needs no render context.
    Polygon2 region;region.push_back(vec2(0,0));region.push_back(vec2(399,0));region.push_back(vec2(399,299));region.push_back(vec2(0,299));
    const auto all=picker.pick_faces(&mesh,region);require(all==std::vector<SurfaceMesh::Face>({rear,front}),"through-region selection lost deleted source slots");
    require(picker.pick_faces(&mesh,Rect(399,0,299,0))==all,"rectangle and polygon region semantics differ");
    SurfaceMesh other;
    face(other,{vec3(8,8,0),vec3(9,8,0),vec3(8,9,0)});face(other,{vec3(-8,-8,0),vec3(-9,-8,0),vec3(-8,-9,0)});
    const auto other_face=face(other,{vec3(-1,-1,0),vec3(1,-1,0),vec3(0,1,0)});
    Polygon2 boundary;for(auto vertex:other.vertices(other_face)) boundary.push_back(vec2(picker.camera()->projected_coordinates_of(other.position(vertex))));
    require(picker.pick_faces(&other,boundary)==std::vector<SurfaceMesh::Face>({other_face}),"polygon boundary vertices were excluded");
    require(picker.pick_faces(&mesh,Polygon2()).empty(),"empty polygon selected faces");
    Polygon2 degenerate;degenerate.push_back(vec2(0));degenerate.push_back(vec2(1));degenerate.push_back(vec2(2));
    require(picker.pick_faces(&mesh,degenerate).empty(),"degenerate region selected faces");
    region[0].x=std::numeric_limits<float>::quiet_NaN();require(picker.pick_faces(&mesh,region).empty(),"nonfinite region selected faces");
    // Completely beyond the camera's far plane must not be selected in projection.
    SurfaceMesh far_plane;face(far_plane,{vec3(-1,-1,-10000),vec3(1,-1,-10000),vec3(0,1,-10000)});
    require(picker.pick_faces(&far_plane,Rect(0,399,0,299)).empty(),"region selection ignored the camera frustum");
    // Region selection honors clipping and the model transform.
    auto* clipping=ClippingPlane::instance();clipping->fit_scene(vec3(0,0,-.1f),1);clipping->set_enabled(true);clipping->set_cross_section(false);
    require(picker.pick_faces(&mesh,Rect(0,399,0,299))==std::vector<SurfaceMesh::Face>({rear}),"region selection ignored clipping");
    clipping->set_cross_section(true);clipping->set_cross_section_width(.01f);
    require(picker.pick_faces(&mesh,Rect(0,399,0,299)).empty(),"region selection ignored cross-section thickness");
    clipping->set_enabled(false);clipping->set_cross_section(false);
    mesh.set_manipulator(std::make_shared<Manipulator>(&mesh));mesh.manipulator()->frame()->translate(vec3(100,0,0));
    require(picker.pick_faces(&mesh,Rect(0,399,0,299)).empty(),"region selection ignored the model transform");
}
void rendered_tests() {
    Viewer viewer("Mesh picker",161,121,1,false,false);
    viewer.set_usage("");
    auto mesh=std::make_shared<SurfaceMesh>();
    const auto dead=face(*mesh,{vec3(-1,-1,.5f),vec3(1,-1,.5f),vec3(0,1,.5f)});
    const auto front=face(*mesh,{vec3(-1,-1,0),vec3(1,-1,0),vec3(0,1,0)});
    const auto rear=face(*mesh,{vec3(-1,-1,-.5f),vec3(1,-1,-.5f),vec3(0,1,-.5f)});
    mesh->delete_face(dead);
    auto warped=std::make_shared<SurfaceMesh>();
    const auto warped_face=face(*warped,{vec3(-1,-1,0),vec3(1,-1,0),vec3(1,1,.8f),vec3(-1,1,0)});
    auto concave=std::make_shared<SurfaceMesh>();
    const auto polygon=face(*concave,{vec3(-1,-1,0),vec3(1,-1,0),vec3(1,1,0),vec3(.2f,1,0),
                                       vec3(.2f,0,0),vec3(-.2f,0,0),vec3(-.2f,1,0),vec3(-1,1,0)});
    viewer.add_model(mesh);viewer.add_model(warped);viewer.add_model(concave);
    auto* camera=viewer.camera();
    camera->set_view_direction(vec3(0,0,-1));camera->set_up_vector(vec3(0,1,0));
    camera->set_type(Camera::PERSPECTIVE);viewer.fit_screen();camera->set_type(Camera::ORTHOGRAPHIC);
    require(viewer.run_frames(2)==0,"mesh picker viewer frame failed");

    SurfaceMeshPicker picker(camera);
    const auto pixel=picker.camera()->projected_coordinates_of(vec3(0));
    const int x=int(pixel.x),y=int(pixel.y);
    // The deleted front-most face keeps its source slot but must never be picked.
    require(picker.pick_face(mesh.get(),x,y)==front,"rasterized pick lost the nearest live face");
    require(picker.picked_face()==front,"picked face query lost its result");
    require(distance(picker.picked_point(mesh.get(),front,x,y),vec3(0))<.03f,
            "rasterized pick point differs from the visible surface");
    require(picker.picked_point(mesh.get(),rear,x,y)==vec3(0),"picked point ignored the picked face");
    require(picker.picked_point(mesh.get(),front,x+3,y)==vec3(0),"picked point ignored the picked pixel");

    // Visibility of the model and of its face drawable gates picking.
    mesh->renderer()->set_visible(false);
    require(!picker.pick_face(mesh.get(),x,y).is_valid(),"hidden model was picked");
    mesh->renderer()->set_visible(true);
    auto* drawable=mesh->renderer()->get_triangles_drawable("faces");
    require(drawable!=nullptr,"surface mesh lost its faces drawable");
    drawable->set_visible(false);
    require(!picker.pick_face(mesh.get(),x,y).is_valid(),"hidden face drawable was picked");
    drawable->set_visible(true);

    // Rasterized picking follows the drawn drawable.
    auto moved=mesh->points();
    for(auto vertex:mesh->vertices(front)) moved[vertex.idx()].z=5;
    drawable->update_vertex_buffer(moved);
    require(picker.pick_face(mesh.get(),x,y)==rear,"rasterized pick ignored the drawn drawable");
    require(picker.picked_point(mesh.get(),rear,x,y).z<-.4f,"rasterized pick point is not on the drawn face");
    drawable->update_vertex_buffer(mesh->points());
    require(picker.pick_face(mesh.get(),x,y)==front,"restored drawable was not pickable");

    // Vertex/edge snapping remains geometric on the picked face.
    const auto first=*mesh->vertices(front).begin();
    const auto vertex_pixel=picker.camera()->projected_coordinates_of(mesh->position(first));
    require(picker.pick_vertex(mesh.get(),front,int(vertex_pixel.x)+1,int(vertex_pixel.y)-1)==first,
            "vertex snapping differs");
    auto halfedge=mesh->halfedge(front);
    const auto midpoint=(mesh->position(mesh->source(halfedge))+mesh->position(mesh->target(halfedge)))*.5f;
    const auto edge_pixel=picker.camera()->projected_coordinates_of(midpoint);
    require(picker.pick_edge(mesh.get(),front,int(edge_pixel.x),int(edge_pixel.y)).is_valid(),
            "edge sensitivity failed");

    // The transform of the picked model is applied by the raster pass.
    mesh->set_manipulator(std::make_shared<Manipulator>(mesh.get()));
    mesh->manipulator()->frame()->translate(vec3(.4f,.25f,.3f));
    const auto translated=picker.camera()->projected_coordinates_of(vec3(.4f,.25f,.3f));
    const int tx=int(translated.x),ty=int(translated.y);
    require(picker.pick_face(mesh.get(),tx,ty)==front,"model transform was ignored");
    require(std::abs(picker.picked_point(mesh.get(),front,tx,ty).z-.3f)<1e-4f,
            "picked point is not world space after manipulation");

    // Clipping removes the front surface; a cross-section removes the remainder.
    auto* clipping=ClippingPlane::instance();
    clipping->fit_scene(vec3(.4f,.25f,.2f),1);clipping->set_enabled(true);clipping->set_cross_section(false);
    require(picker.pick_face(mesh.get(),tx,ty)==rear,"clipping was ignored by rasterized picking");
    clipping->set_cross_section(true);clipping->set_cross_section_width(.01f);
    require(!picker.pick_face(mesh.get(),tx,ty).is_valid(),"cross-section thickness was ignored");
    clipping->set_enabled(false);clipping->set_cross_section(false);

    // Perspective projection and rotation keep the raster path aligned with the camera.
    mesh->manipulator()->frame()->rotate(quat(vec3(0,1,0),.6f));
    camera->set_type(Camera::PERSPECTIVE);camera->show_entire_scene();
    const mat4 transform=mesh->manipulator()->matrix();
    const auto rotated=picker.camera()->projected_coordinates_of(transform*vec3(0));
    require(picker.pick_face(mesh.get(),int(rotated.x),int(rotated.y))==front,
            "perspective/rotated mesh was missed");
    const auto rotated_vertex=picker.camera()->projected_coordinates_of(transform*mesh->position(first));
    require(picker.pick_vertex(mesh.get(),front,int(rotated_vertex.x),int(rotated_vertex.y))==first,
            "vertex hit did not follow model rotation");
    mesh->manipulator()->frame()->setOrientation(quat());mesh->manipulator()->frame()->setPosition(vec3(0));
    camera->set_type(Camera::ORTHOGRAPHIC);viewer.fit_screen();

    // Nonplanar and concave polygons use the same tessellation as rendering.
    const auto corner=picker.camera()->projected_coordinates_of(vec3(.9f,-.9f,0));
    require(picker.pick_face(warped.get(),int(corner.x),int(corner.y))==warped_face,
            "nonplanar polygon was missed");
    require(picker.picked_point(warped.get(),warped_face,int(corner.x),int(corner.y)).z<.12f,
            "picked point used a fitted plane instead of the tessellated surface");
    const auto inside=picker.camera()->projected_coordinates_of(vec3(.7f,.5f,0)),notch=picker.camera()->projected_coordinates_of(vec3(0,.5f,0));
    require(picker.pick_face(concave.get(),int(inside.x),int(inside.y))==polygon,
            "concave face interior was missed");
    require(!picker.pick_face(concave.get(),int(notch.x),int(notch.y)).is_valid(),
            "concave notch was incorrectly filled");
    require(!picker.picked_face().is_valid(),"failed pick retained a stale face");

    SurfaceMesh detached;
    const auto detached_face=face(detached,{vec3(-1,-1,0),vec3(1,-1,0),vec3(0,1,0)});
    require(!picker.pick_face(&detached,x,y).is_valid(),"detached mesh was picked");
    Polygon2 detached_polygon;
    for(auto vertex:detached.vertices(detached_face)) detached_polygon.push_back(vec2(picker.camera()->projected_coordinates_of(detached.position(vertex))));
    require(picker.pick_faces(&detached,detached_polygon)==std::vector<SurfaceMesh::Face>({detached_face}),
            "detached region selection failed");

#ifndef EASY3D_HAS_VULKAN
    // Each picker owns a context-local framebuffer and restores the caller state.
    glfwMakeContextCurrent(viewer.window());
    GLuint pack=0;glGenBuffers(1,&pack);glBindBuffer(GL_PIXEL_PACK_BUFFER,pack);
    glBufferData(GL_PIXEL_PACK_BUFFER,128,nullptr,GL_STATIC_DRAW);glPixelStorei(GL_PACK_ROW_LENGTH,7);glPixelStorei(GL_PACK_SKIP_ROWS,2);
    glViewport(3,4,viewer.framebuffer_width()-6,viewer.framebuffer_height()-8);glScissor(0,0,1,1);
    glEnable(GL_SCISSOR_TEST);glEnable(GL_BLEND);glEnable(GL_COLOR_LOGIC_OP);glLogicOp(GL_XOR);glEnable(GL_RASTERIZER_DISCARD);
    glDepthMask(GL_FALSE);glDepthFunc(GL_GREATER);glColorMask(GL_FALSE,GL_TRUE,GL_FALSE,GL_TRUE);glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    glClearColor(.1f,.2f,.3f,.4f);glActiveTexture(GL_TEXTURE3);
    const auto state_pixel=picker.camera()->projected_coordinates_of(vec3(0));
    require(picker.pick_face(mesh.get(),int(state_pixel.x),int(state_pixel.y))==front,"caller render state changed mesh picking");
    GLint vp[4],bound=0,row=0,skip=0,active=0,depth=0,mode[2];GLboolean mask[4],depth_mask=GL_TRUE;GLfloat clear[4];
    glGetIntegerv(GL_VIEWPORT,vp);glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING,&bound);glGetIntegerv(GL_PACK_ROW_LENGTH,&row);
    glGetIntegerv(GL_PACK_SKIP_ROWS,&skip);glGetIntegerv(GL_ACTIVE_TEXTURE,&active);glGetIntegerv(GL_DEPTH_FUNC,&depth);glGetIntegerv(GL_POLYGON_MODE,mode);
    glGetBooleanv(GL_COLOR_WRITEMASK,mask);glGetBooleanv(GL_DEPTH_WRITEMASK,&depth_mask);glGetFloatv(GL_COLOR_CLEAR_VALUE,clear);
    require(vp[0]==3 && vp[1]==4 && vp[2]==viewer.framebuffer_width()-6 && vp[3]==viewer.framebuffer_height()-8 &&
        GLuint(bound)==pack && row==7 && skip==2 && active==GL_TEXTURE3 && depth==GL_GREATER && mode[0]==GL_LINE &&
        !depth_mask && !mask[0] && mask[1] && !mask[2] && mask[3] && std::abs(clear[0]-.1f)<1e-6f && std::abs(clear[3]-.4f)<1e-6f &&
        glIsEnabled(GL_SCISSOR_TEST) && glIsEnabled(GL_BLEND) && glIsEnabled(GL_COLOR_LOGIC_OP) && glIsEnabled(GL_RASTERIZER_DISCARD),
        "mesh query leaked OpenGL state");
    glBindBuffer(GL_PIXEL_PACK_BUFFER,0);glDeleteBuffers(1,&pack);glPixelStorei(GL_PACK_ROW_LENGTH,0);glPixelStorei(GL_PACK_SKIP_ROWS,0);
    glDisable(GL_SCISSOR_TEST);glDisable(GL_BLEND);glDisable(GL_COLOR_LOGIC_OP);glDisable(GL_RASTERIZER_DISCARD);glDepthMask(GL_TRUE);glDepthFunc(GL_LESS);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);glActiveTexture(GL_TEXTURE0);

    {
        Viewer other("Mesh picker B",171,131,1,false,false);
        other.set_usage("");
        auto second=std::make_shared<SurfaceMesh>();
        const auto second_face=face(*second,{vec3(-1,-1,0),vec3(1,-1,0),vec3(0,1,0)});
        other.add_model(second);
        other.camera()->set_view_direction(vec3(0,0,-1));other.camera()->set_up_vector(vec3(0,1,0));
        other.camera()->set_type(Camera::ORTHOGRAPHIC);other.fit_screen();
        require(other.run_frames(1)==0,"second picker viewer frame failed");
        SurfaceMeshPicker other_picker(other.camera());
        glfwMakeContextCurrent(other.window());
        const auto other_pixel=other_picker.camera()->projected_coordinates_of(vec3(0));
        require(other_picker.pick_face(second.get(),int(other_pixel.x),int(other_pixel.y))==second_face,
                "picker framebuffer crossed contexts");
        glfwMakeContextCurrent(viewer.window());
        require(picker.pick_face(mesh.get(),int(state_pixel.x),int(state_pixel.y))==front,
                "picker did not survive a context switch");
    }
    glfwMakeContextCurrent(viewer.window());
    require(glGetError()==GL_NO_ERROR,"mesh picking generated OpenGL errors");
#endif
}
int main(int argc,char**) {return test::run("GUI mesh picking and geometric regions",[&]{
    if(argc>1){rendered_tests();return;}
    math_tests();
});}
