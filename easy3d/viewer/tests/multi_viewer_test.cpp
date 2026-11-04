// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <applications/FigureMaker/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <type_traits>
using namespace easy3d;
namespace {
void require(bool ok,const char* message) {if(!ok) throw std::runtime_error(message);}
struct Grid : MultiViewer {
    using MultiViewer::MultiViewer;
    using MultiViewer::mouse_press_event;
    using MultiViewer::mouse_drag_event;
    using MultiViewer::mouse_release_event;
    ivec4 bounds(int row,int col) const {
        const int w=framebuffer_width(),h=framebuffer_height();
        int x=int(int64_t(col)*w/columns()),y=int(int64_t(row)*h/rows());
        return ivec4(x,y,int(int64_t(col+1)*w/columns())-x,int(int64_t(row+1)*h/rows())-y);
    }
};
std::shared_ptr<SurfaceMesh> triangle(const char* name) {
    auto m=std::make_shared<SurfaceMesh>();m->set_name(name);
    auto a=m->add_vertex(vec3(0,-1,-1)),b=m->add_vertex(vec3(0,1,-1)),c=m->add_vertex(vec3(0,0,1));
    m->add_triangle(a,b,c);return m;
}
void material(Model* m,const vec4& color) {
    auto* r=m->renderer();for(auto d:r->lines_drawables()) d->set_visible(false);for(auto d:r->points_drawables()) d->set_visible(false);
    auto* d=r->get_triangles_drawable("faces");d->set_lighting(false);d->set_uniform_coloring(color);
}
void frame(Grid& grid) {
    for(int i=0;i<100;++i) {Viewer::poll_events();if(grid.draw_frame()) return;glfwWaitEventsTimeout(.01);}
    throw std::runtime_error("multi-view frame did not present");
}
void color_at(const std::vector<unsigned char>& rgba,int width,int x,int y,const vec4& expected) {
    const auto p=4*(size_t(y)*width+x);
    for(unsigned c=0;c<3;++c) require(std::abs(int(rgba.at(p+c))-int(std::lround(expected[c]*255)))<=3,"wrong cell image or lost material");
}
void snapshot(Grid& grid,const char* file,float scale) {
    const auto matrix=grid.camera()->modelViewProjectionMatrix();
    require(grid.snapshot(file,scale,4,0),"multi-view file snapshot failed");
    for(unsigned i=0;i<16;++i) require(std::abs(grid.camera()->modelViewProjectionMatrix()[i]-matrix[i])<1e-6f,"snapshot changed the shared camera");
    std::vector<unsigned char> rgba;int w,h,c;require(ImageIO::load(file,rgba,w,h,c,4,false),"snapshot was not readable");
    require(w==int(std::ceil(grid.framebuffer_width()*double(scale))) && h==int(std::ceil(grid.framebuffer_height()*double(scale))),"snapshot dimensions incorrect");
    color_at(rgba,w,w/6,h/4,vec4(1,0,0,1));color_at(rgba,w,w/2,h/4,vec4(0,0,1,1));
    color_at(rgba,w,5*w/6,h/4,vec4(1));color_at(rgba,w,w/6,3*h/4,vec4(1,0,0,1));
}
}
int main(int argc,char**) {
    static_assert(std::is_base_of<Viewer,MultiViewer>::value,"multi-view uses the same Viewer");
    try {
        initialize(false,false,false);
#ifdef EASY3D_HAS_VULKAN
        VulkanOptions options;options.validation=options.synchronization_validation=argc>1;
        Grid grid(2,3,"Shared multi-view test",301,223,1,false,false,true,options);
#else
        Grid grid(2,3,"Shared multi-view test",301,223,1,false,false,true);
#endif
        grid.set_background_color(vec4(1));grid.set_division_visible(false);
        auto red=triangle("red"),blue=triangle("blue");grid.add_model(red);grid.add_model(blue);
        material(red.get(),vec4(1,0,0,1));material(blue.get(),vec4(0,0,1,1));
        grid.assign(0,0,red.get());grid.assign(0,1,blue.get());grid.assign(1,0,red->renderer()->get_triangles_drawable("faces"));
        auto point=std::make_shared<PointsDrawable>("sphere");grid.add_drawable(point);point->update_vertex_buffer({vec3(0)});point->set_impostor_type(PointsDrawable::SPHERE);point->set_point_size(23);point->set_lighting(false);point->set_uniform_coloring(vec4(0,1,0,1));grid.assign(1,1,point.get());
        auto line=std::make_shared<LinesDrawable>("cylinder");grid.add_drawable(line);line->update_vertex_buffer({vec3(0,-.5f,0),vec3(0,.5f,0)});line->set_impostor_type(LinesDrawable::CYLINDER);line->set_line_width(13);line->set_lighting(false);line->set_uniform_coloring(vec4(1,0,1,1));grid.assign(1,2,line.get());
        grid.fit_screen();frame(grid);snapshot(grid,"multiview.png",1);snapshot(grid,"multiview-scaled.png",1.25f);
        require(grid.snapshot("multiview-transparent.png",1,4,2,false),"transparent/cropped grid snapshot failed");
        std::vector<unsigned char> transparent;int tw,th,tc;
        require(ImageIO::load("multiview-transparent.png",transparent,tw,th,tc,4,false) && transparent[3]==0,"grid snapshot lost transparent background");
        unsigned edges=0;
        for(int y=0;y<th/2;++y) for(int x=0;x<tw/3;++x) {
            const auto p=4*(size_t(y)*tw+x);const auto alpha=transparent[p+3];
            if(alpha && alpha<255 && transparent[p]>0) {++edges;require(transparent[p]>250 && transparent[p+1]<3 && transparent[p+2]<3,"MSAA snapshot edge was saved as premultiplied alpha");}
        }
        require(edges>0,"transparent snapshot did not exercise multisample edges");
        require(grid.models().size()==2 && grid.drawables().size()==2,"assignment duplicated ownership");
        ViewerBase& api=grid;
        require((api.unproject_point(api.project_point(vec3(0)))-vec3(0)).norm()<1e-4f,"shared multi-view coordinate round trip failed");
        for(int row=0;row<2;++row) for(int col=0;col<3;++col) {
            const auto vp=grid.bounds(row,col);int x=int(double(vp.x+vp.z/2)*grid.width()/grid.framebuffer_width()),y=int(double(vp.y+vp.w/2)*grid.height()/grid.framebuffer_height());
            bool found=false;const auto p=api.point_under_pixel(x,y,found);
            require(found!=(row==0 && col==2),"portable query included an unassigned cell or missed assigned geometry");
            if(found && !(row==1 && col>=1)) require(std::abs(p.x)<1e-4f,"portable cell query unprojected through the whole canvas");
            bool direct=false;const auto q=grid.point_under_pixel(x,y,direct);require(direct==found && (!found || (p-q).norm()<1e-3f),"direct and portable cell queries disagree");
#ifdef EASY3D_HAS_VULKAN
            auto hit=grid.pick(vp.x+vp.z/2,vp.y+vp.w/2);require(bool(hit)==found,"cell picking lost the source ID");
            if(hit) require(hit.drawable && (hit.position-p).norm()<.03f,"pixel picking used another cell's inverse projection");
#endif
        }
        const auto projection=grid.camera()->projectionMatrix(),view=grid.camera()->modelViewMatrix();
        auto custom=projection;custom(0,2)+=.12f;grid.camera()->set_projection_matrix(custom);
        auto shifted=view;shifted(0,3)+=.1f;grid.camera()->set_modelview_matrix(shifted);
        const auto custom_point=api.unproject_point(api.project_point(vec3(0)));
        require(custom_point.norm()<1e-4f,"custom multi-view matrices were lost in coordinate conversion");
        bool custom_hit=false;const auto custom_surface=api.point_under_pixel(grid.width()/6,grid.height()/4,custom_hit);
        require(custom_hit && std::abs(custom_surface.x)<1e-4f,"cell depth query ignored custom camera matrices");
        grid.camera()->set_projection_matrix(projection);grid.camera()->set_modelview_matrix(view);
        red->renderer()->set_visible(false);frame(grid);
        bool found=false;grid.point_under_pixel(grid.width()/6,grid.height()/4,found);require(!found,"hidden model rendered in its assigned cell");
        grid.point_under_pixel(grid.width()/6,3*grid.height()/4,found);require(found,"explicit drawable assignment inherited parent-model visibility");red->renderer()->set_visible(true);
#ifdef EASY3D_HAS_VULKAN
        auto before=grid.capture(1);if(grid.device().supported_sample_counts()&4) {
            auto msaa=grid.capture(4);require(before.depth==msaa.depth && before.ids.size()==msaa.ids.size(),"MSAA changed per-cell scene depth");
            for(size_t i=0;i<before.ids.size();++i) require(before.ids[i].object==msaa.ids[i].object && before.ids[i].primitive==msaa.ids[i].primitive,"MSAA changed cell source IDs");
        }
        grid.set_division_visible(true);auto divider=grid.capture(1);require(divider.depth==before.depth && divider.rgba!=before.rgba,"divider changed depth or did not draw");
        for(size_t i=0;i<before.ids.size();++i) require(before.ids[i].object==divider.ids[i].object,"divider changed source IDs");grid.set_division_visible(false);
        point->set_point_size(300);auto large=grid.capture();const auto empty=grid.bounds(0,2);
        for(int y=empty.y;y<empty.y+empty.w;++y) for(int x=empty.x;x<empty.x+empty.z;++x)
            require(large.ids[size_t(y)*large.width+x].object==UINT32_MAX,"large point escaped its cell scissor");
        point->set_point_size(23);
        const auto uploads=grid.device().point_upload_count();grid.assign(0,2,point.get());grid.capture();require(grid.device().point_upload_count()==uploads,"sharing a point drawable across cells uploaded positions again");
#endif
        bool rejected=false;try{grid.set_layout(0,2);}catch(const std::invalid_argument&){rejected=true;}require(rejected && grid.rows()==2,"invalid layout modified the grid");
        grid.resize(317,237);frame(grid);
        auto position=grid.camera()->position();auto orientation=grid.camera()->orientation();auto pivot=grid.camera()->pivotPoint();vec3 delta;
        for(int i=0;i<2;++i) {
            grid.camera()->setPosition(position);grid.camera()->setOrientation(orientation);grid.camera()->frame()->setPivotPoint(pivot);
#ifdef EASY3D_HAS_VULKAN
            auto vp=grid.bounds(i,i);const int x=vp.x+vp.z/2,y=vp.y+vp.w/2,dx=10;
#else
            const int x=(i*2+1)*grid.width()/6,y=(i*2+1)*grid.height()/4,dx=10;
#endif
            grid.mouse_press_event(x,y,Viewer::BUTTON_RIGHT,0);grid.mouse_drag_event(x+dx,y,dx,0,Viewer::BUTTON_RIGHT,0);grid.mouse_release_event(x+dx,y,Viewer::BUTTON_RIGHT,0);
            const auto projected=api.project_point(grid.camera()->sceneCenter());
            require((api.unproject_point(projected)-grid.camera()->sceneCenter()).norm()<1e-4f,"active-cell coordinate round trip failed");
            if(i==0) delta=grid.camera()->position()-position;else require((grid.camera()->position()-position-delta).norm()<1e-5f,"camera pan depends on the cell's canvas offset");
        }
        require(grid.delete_model(red.get()),"model removal failed");frame(grid);grid.point_under_pixel(grid.width()/6,3*grid.height()/4,found);require(!found,"deleted model retained an assigned drawable");
        require(grid.delete_drawable(point.get()),"drawable removal failed");grid.set_layout(3,2);frame(grid);require(grid.models().size()==1,"layout changes altered ownership");grid.clear_scene();frame(grid);
        grid.point_under_pixel(grid.width()/4,grid.height()/6,found);require(!found && grid.models().empty() && grid.drawables().empty(),"cleared assignments rendered old geometry");
#ifdef EASY3D_HAS_VULKAN
        require(grid.device().validation_errors()==0,"multi-view Vulkan validation error");
#endif
        // Exercise FigureMaker's original file-loading/layout overrides without changing the application.
        for(const char* file:{"figure-a.off","figure-b.off"}) {
            std::ofstream out(file);out<<"OFF\n3 1 0\n0 -1 -1\n0 1 -1\n0 0 1\n3 0 1 2\n";
            require(bool(out),"cannot write FigureMaker fixture");
        }
        {
            FigureMaker app(1,1,"FigureMaker migration test");app.hide();app.set_background_color(vec4(1));app.set_division_visible(false);
            Viewer& editor=app;auto* a=editor.add_model("figure-a.off");auto* b=editor.add_model("figure-b.off");
            require(a && b && app.columns()==2 && app.models().size()==2,"FigureMaker did not grow/assign its grid");
            material(a,vec4(1,0,0,1));material(b,vec4(0,0,1,1));app.fit_screen();
            require(app.run_frames(2)==0 && app.snapshot("figure-grid.png",1,4,0),"FigureMaker grid snapshot failed");
            std::vector<unsigned char> pixels;int w,h,c;require(ImageIO::load("figure-grid.png",pixels,w,h,c,4,false),"FigureMaker snapshot unreadable");
            color_at(pixels,w,w/4,h/2,vec4(1,0,0,1));color_at(pixels,w,3*w/4,h/2,vec4(0,0,1,1));
            require(editor.delete_model(a) && app.columns()==1 && app.models().size()==1,"FigureMaker did not shrink/reassign its grid");
            app.fit_screen();require(app.run_frames(1)==0 && app.snapshot("figure-shrunk.png",1,1,0),"FigureMaker snapshot failed after removal");
            require(ImageIO::load("figure-shrunk.png",pixels,w,h,c,4,false),"FigureMaker shrunk snapshot unreadable");color_at(pixels,w,w/2,h/2,vec4(0,0,1,1));
        }
        frame(grid); // Destroying a second viewer must leave the original device/context alive.
        std::cout<<"PASS: shared multi-view layout, source ownership, assignment, cell queries, MSAA, impostors, dividers, resize, interaction and snapshots\n";
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
