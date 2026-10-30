// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/viewer/multi_viewer.h>
#include <easy3d/viewer/opengl_offscreen.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#include <type_traits>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
namespace {
void require(bool value,const char* message) { if(!value)throw std::runtime_error(message); }
std::shared_ptr<SurfaceMesh> triangle() {
    auto mesh=std::make_shared<SurfaceMesh>();
    auto a=mesh->add_vertex(vec3(0,-1,-1)),b=mesh->add_vertex(vec3(0,1,-1)),c=mesh->add_vertex(vec3(0,0,1));
    mesh->add_triangle(a,b,c);return mesh;
}
void red(const std::shared_ptr<SurfaceMesh>& mesh) {
    auto* faces=mesh->renderer()->get_triangles_drawable("faces");
    faces->set_lighting(false);faces->set_uniform_coloring(vec4(.8f,.2f,.1f,1));
}
void check_file(const char* path,int width,int height) {
    int w,h,c;std::vector<unsigned char> image;
    require(ImageIO::load(path,image,w,h,c,4,false) && w==width && h==height,"offscreen pixel dimensions changed");
    const size_t center=(size_t(h/2)*w+w/2)*4;
    require(std::abs(int(image[center])-204)<=2 && std::abs(int(image[center+1])-51)<=2,"concrete offscreen lost scene/material/context");
}
}
int main() {
    static_assert(std::is_same<Viewer,OpenGLViewer>::value,"Viewer alias must match CMake backend");
    static_assert(std::is_same<MultiViewer,OpenGLMultiViewer>::value,"MultiViewer alias must match CMake backend");
    try {
        initialize(false,false,false);
        OpenGLOffScreen surviving(103,79);auto mesh=triangle();surviving.add_model(mesh);red(mesh);
        // Another offscreen host changes the current context and is destroyed.
        { OpenGLOffScreen other(81,61);auto second=triangle();other.add_model(second);red(second);
          require(other.render("concrete-other.png",1,1,0),"second offscreen failed"); }
        for(int samples:{1,4}) {
            require(surviving.render("concrete-offscreen.png",1,samples,0),"surviving offscreen failed");
            check_file("concrete-offscreen.png",103,79);
        }
        surviving.resize(117,91);require(surviving.render("concrete-resized.png",1,4,0),"offscreen resize failed");
        check_file("concrete-resized.png",117,91);
        require(!surviving.render("invalid.png",1,3,0),"invalid MSAA accepted");
        bool rejected=false;try { OpenGLOffScreen invalid(0,1); }catch(const std::invalid_argument&) { rejected=true; }
        require(rejected,"invalid dimensions accepted");
        OpenGLMultiViewer grid(1,2,"Concrete multi-view");grid.hide();grid.resize(202,102);
        auto shared=triangle();grid.add_model(shared);red(shared);grid.assign(0,0,shared.get());grid.assign(0,1,shared.get());
        grid.set_division_visible(false);grid.fit_screen();require(grid.draw_frame(),"renamed multi-view did not render");
        require(grid.rows()==1 && grid.columns()==2 && !grid.division_visible(),"multi-view layout API changed");
        require(static_cast<ViewerBase&>(grid).snapshot("concrete-grid.png",1,4,0),"multi-view snapshot failed");
        int w,h,c;std::vector<unsigned char> image;
        require(ImageIO::load("concrete-grid.png",image,w,h,c,4,false),"multi-view image missing");
        for(int x:{w/4,3*w/4}) {
            const size_t p=(size_t(h/2)*w+x)*4;
            require(std::abs(int(image[p])-204)<=2 && std::abs(int(image[p+1])-51)<=2,"assigned multi-view cell did not render");
        }
        require(grid.models().size()==1,"multi-view duplicated source ownership");
        std::cout<<"Explicit OpenGL counterparts, aliases, shared context survival and grid rendering passed\n";
        return 0;
    }catch(const std::exception& error) { std::cerr<<error.what()<<'\n';return 1; }
}
