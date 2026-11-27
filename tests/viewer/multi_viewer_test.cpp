// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <tests/viewer/test_window.h>   // window_options()
#include <applications/FigureMaker/viewer.h>
#include <easy3d/renderer/render_settings.h>
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
    using Viewer::mouse_drag_event;
    using Viewer::mouse_release_event;
    // The cell rects MultiViewer built, not a re-derivation of them: a change to
    // update_division() has to show up here. views_ stores Vulkan rects in top-left
    // framebuffer pixels and OpenGL ones bottom-left, so cell() is raw and bounds()
    // hands out the top-left convention the pixel math below expects.
    using MultiViewer::views_;
    const ivec4& cell(int row,int col) const { return views_[row][col].viewport; }
    ivec4 bounds(int row,int col) const {
        const auto& stored=cell(row,col);
#ifdef EASY3D_HAS_VULKAN
        return stored;
#else
        return ivec4(stored.x,framebuffer_height()-stored.y-stored.w,stored.z,stored.w);
#endif
    }
    // The exact integer partition of the framebuffer that the cells must form. Stated
    // from the framebuffer extent only: scaling a logical-window rect by dpi_scaling()
    // - the double scaling of issue #179 - is what this test must reject.
    ivec4 partition(int row,int col) const {
        const int w=framebuffer_width(),h=framebuffer_height();
        const int x=col*w/columns(),y=row*h/rows();
        return ivec4(x,y,(col+1)*w/columns()-x,(row+1)*h/rows()-y);
    }
    using MultiViewer::active_row_;
    using MultiViewer::active_col_;
    mutable int draw_calls = 0;
    int custom_draw_calls() const { return draw_calls; }
protected:
    void draw() const override {
        ++draw_calls;
#ifdef EASY3D_HAS_VULKAN
        Viewer::draw();
#else
        MultiViewer::draw();
#endif
    }
public:
    // The same partition of the *logical* window, which is what update_division()
    // calibrates the shared camera with (the other half of issue #179: a camera fed
    // framebuffer pixels at a scale factor of two doubles every pointer delta).
    void active_camera_extent(int& w,int& h) const {
        const int x=active_col_*width()/columns(),y=active_row_*height()/rows();
        w=(active_col_+1)*width()/columns()-x;
        h=(active_row_+1)*height()/rows()-y;
    }
    // pick() takes logical window coordinates; bounds() is in framebuffer pixels.
    Viewer::PickResult pick_cell(const ivec4& vp) const {
        return pick(int(double(vp.x+vp.z/2)*width()/framebuffer_width()),
                    int(double(vp.y+vp.w/2)*height()/framebuffer_height()));
    }
};
class NativeFallbackPoints final : public PointsDrawable {
public:
    using PointsDrawable::PointsDrawable;
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
    for(unsigned c=0;c<3;++c) {
        if(std::abs(int(rgba.at(p+c))-int(std::lround(expected[c]*255)))>3) {
            std::cerr<<"pixel "<<x<<','<<y<<" channel "<<c<<" actual "<<int(rgba.at(p+c))
                     <<" expected "<<int(std::lround(expected[c]*255))<<'\n';
            throw std::runtime_error("wrong cell image or lost material");
        }
    }
}
void require_black_at(const std::vector<unsigned char>& rgba,int width,int x,int y) {
    const auto p=4*(size_t(y)*width+x);
    if(rgba.at(p)>=5 || rgba.at(p+1)>=5 || rgba.at(p+2)>=5)
        throw std::runtime_error("grid division is missing or misaligned at "+std::to_string(x)+","+std::to_string(y));
}
void require_dividers(const std::vector<unsigned char>& rgba,int width,int height) {
    for(int col=1;col<3;++col) {
        const int x=col*width/3;
        for(int y=0;y<height;++y)
            require_black_at(rgba,width,x,y);
    }
    const int y=height/2;
    for(int x=0;x<width;++x)
        require_black_at(rgba,width,x,y);
}
// Reconciles the stored cell rects with the framebuffer partition they are supposed to
// be, at the current window size and grid layout.
void check_cells(const Grid& grid) {
    const int fw=grid.framebuffer_width(),fh=grid.framebuffer_height();
    require(fw>=grid.width() && fh>=grid.height(),"framebuffer is smaller than the logical window");
    long area=0;
    for(int row=0;row<grid.rows();++row) for(int col=0;col<grid.columns();++col) {
        const auto want=grid.partition(row,col),got=grid.cell(row,col);
        require(got.z==want.z && got.w==want.w,"cell size is not the framebuffer partition");
#ifdef EASY3D_HAS_VULKAN
        require(got.x==want.x && got.y==want.y,"Vulkan cell is not stored in top-left framebuffer pixels");
#else
        require(got.x==want.x && got.y==fh-want.y-want.w,"OpenGL cell is not stored in bottom-left framebuffer pixels");
#endif
        require(got.x>=0 && got.y>=0 && got.x+got.z<=fw && got.y+got.w<=fh,"cell escapes the framebuffer");
        area+=long(got.z)*got.w;
    }
    require(area==long(fw)*fh,"cells do not tile the framebuffer exactly");
    int w,h;
    grid.active_camera_extent(w,h);
    require(grid.camera()->screen_width()==w && grid.camera()->screen_height()==h,
            "the shared camera is not calibrated to the active cell's logical extent");
}
void snapshot(Grid& grid,const char* file,float scale) {
    const auto matrix=grid.camera()->model_view_projection_matrix();
    const int draws=grid.custom_draw_calls();
    require(grid.snapshot(file,scale,4,0),"multi-view file snapshot failed");
    require(grid.custom_draw_calls()>draws,"multi-view snapshot bypassed custom draw()");
    for(unsigned i=0;i<16;++i) require(std::abs(grid.camera()->model_view_projection_matrix()[i]-matrix[i])<1e-6f,"snapshot changed the shared camera");
    std::vector<unsigned char> rgba;int w,h,c;require(ImageIO::load(file,rgba,w,h,c,4,false),"snapshot was not readable");
    require(w==int(std::ceil(grid.framebuffer_width()*double(scale))) && h==int(std::ceil(grid.framebuffer_height()*double(scale))),"snapshot dimensions incorrect");
    color_at(rgba,w,w/6,h/4,vec4(1,0,0,1));color_at(rgba,w,w/2,h/4,vec4(0,0,1,1));
    color_at(rgba,w,5*w/6,h/4,vec4(1));color_at(rgba,w,w/6,3*h/4,vec4(1,0,0,1));
    color_at(rgba,w,w/2,3*h/4,vec4(0,1,0,1));color_at(rgba,w,5*w/6,3*h/4,vec4(1,0,1,1));
}
}
int main(int argc,char**) {
    static_assert(std::is_base_of<Viewer,MultiViewer>::value,"multi-view uses the same Viewer");
    try {
        initialize(false,false,false);
#ifdef EASY3D_HAS_VULKAN
        DeviceOptions options;options.validation=options.sync_validation=argc>1;
        Grid grid(2,3,"Shared multi-view test",window_options(301,223,1,false,false,true),options);
#else
        Grid grid(2,3,"Shared multi-view test",window_options(301,223,1,false,false,true));
#endif
        grid.set_background_color(vec4(1));grid.set_division_visible(false);
        auto red=triangle("red"),blue=triangle("blue");grid.add_model(red);grid.add_model(blue);
        material(red.get(),vec4(1,0,0,1));material(blue.get(),vec4(0,0,1,1));
        grid.assign(0,0,red.get());grid.assign(0,1,blue.get());grid.assign(1,0,red->renderer()->get_triangles_drawable("faces"));
        auto point=std::make_shared<PointsDrawable>("sphere");grid.add_drawable(point);point->update_vertex_buffer({vec3(0)});point->set_impostor_type(PointsDrawable::SPHERE);point->set_point_size(23);point->set_lighting(false);point->set_uniform_coloring(vec4(0,1,0,1));grid.assign(1,1,point.get());
        auto line=std::make_shared<LinesDrawable>("cylinder");grid.add_drawable(line);line->update_vertex_buffer({vec3(0,-.5f,0),vec3(0,.5f,0)});line->set_impostor_type(LinesDrawable::CYLINDER);line->set_line_width(13);line->set_lighting(false);line->set_uniform_coloring(vec4(1,0,1,1));grid.assign(1,2,line.get());
        grid.fit_screen();frame(grid);check_cells(grid);
        // Odd logical extents expose logical-to-framebuffer rounding in native divisions.
        require(grid.width()==301 && grid.height()==223,"divider fixture logical extents changed");
        const float scale=grid.dpi_scaling();
        require(grid.framebuffer_width()==int(std::ceil(301*scale)) &&
                grid.framebuffer_height()==int(std::ceil(223*scale)),"divider fixture framebuffer extents disagree with DPI");
        if(scale==2.0f)
            require(grid.framebuffer_width()==602 && grid.framebuffer_height()==446,"DPI-2 divider fixture was not exercised");
        snapshot(grid,"multiview.png",1);snapshot(grid,"multiview-scaled.png",1.25f);
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
        require((api.window_to_world(api.world_to_window(vec3(0)))-vec3(0)).norm()<1e-4f,"shared multi-view coordinate round trip failed");
        for(int row=0;row<2;++row) for(int col=0;col<3;++col) {
            const auto vp=grid.bounds(row,col);int x=int(double(vp.x+vp.z/2)*grid.width()/grid.framebuffer_width()),y=int(double(vp.y+vp.w/2)*grid.height()/grid.framebuffer_height());
            bool found=false;const auto p=api.point_under_pixel(x,y,found);
            require(found!=(row==0 && col==2),"portable query included an unassigned cell or missed assigned geometry");
            if(found && !(row==1 && col>=1)) require(std::abs(p.x)<1e-4f,"portable cell query unprojected through the whole canvas");
            bool direct=false;const auto q=grid.point_under_pixel(x,y,direct);require(direct==found && (!found || (p-q).norm()<1e-3f),"direct and portable cell queries disagree");
            auto hit=grid.pick_cell(vp);require(bool(hit)==found,("cell picking lost the source ID at "+std::to_string(row)+","+std::to_string(col)).c_str());
            if(hit) {
                require(hit.drawable && (hit.position-p).norm()<.03f,"pixel picking used another cell's inverse projection");
                // pick_cell() routes through logical coordinates (x,y); compare against the
                // exact framebuffer pixel that logical pick samples, not the raw cell center.
                const auto exact=grid.pixel_to_world(int(std::floor(double(x)*grid.framebuffer_width()/grid.width())),
                                                     int(std::floor(double(y)*grid.framebuffer_height()/grid.height())),hit.depth);
                require((exact-hit.position).norm()<1e-5f,"pixel unprojection and picking diverged");
                if(row==0) require(hit.model.get()==(col==0?red.get():blue.get()) && hit.face_index==0,"cell source ownership/provenance is wrong");
                if(row==1 && col>0) require(hit.drawable.get()==(col==1?static_cast<Drawable*>(point.get()):static_cast<Drawable*>(line.get())),"impostor source escaped its assigned cell");
            }
        }
        const auto projection=grid.camera()->projection_matrix(),view=grid.camera()->model_view_matrix();
        auto custom=projection;custom(0,2)+=.12f;grid.camera()->set_projection_matrix(custom);
        auto shifted=view;shifted(0,3)+=.1f;grid.camera()->set_model_view_matrix(shifted);
        const auto custom_point=api.window_to_world(api.world_to_window(vec3(0)));
        require(custom_point.norm()<1e-4f,"custom multi-view matrices were lost in coordinate conversion");
        bool custom_hit=false;const auto custom_surface=api.point_under_pixel(grid.width()/6,grid.height()/4,custom_hit);
        require(custom_hit && std::abs(custom_surface.x)<1e-4f,"cell depth query ignored custom camera matrices");
        const auto custom_pick=grid.pick(grid.width()/6,grid.height()/4);
        require(custom_pick.model==red && std::abs(custom_pick.position.x)<1e-4f,"cell source picking ignored custom matrices");
        grid.camera()->set_projection_matrix(projection);grid.camera()->set_model_view_matrix(view);
        red->renderer()->set_visible(false);frame(grid);
        bool found=false;grid.point_under_pixel(grid.width()/6,grid.height()/4,found);require(!found,"hidden model rendered in its assigned cell");
        grid.point_under_pixel(grid.width()/6,3*grid.height()/4,found);require(found,"explicit drawable assignment inherited parent-model visibility");
        auto hidden_cell=grid.bounds(0,0),explicit_cell=grid.bounds(1,0);
        require(!grid.pick_cell(hidden_cell),"hidden model remained pickable in its model-assigned cell");
        const auto explicit_hit=grid.pick_cell(explicit_cell);
        require(explicit_hit.model==red && explicit_hit.face_index==0,"explicit assignment lost a hidden parent's source ownership");
        red->renderer()->set_visible(true);
        auto before=grid.capture(1);
        const auto captured_id=[&](int r,int c) {const auto vp=grid.bounds(r,c);return before.ids[size_t(vp.y+vp.w/2)*before.width+vp.x+vp.z/2].object;};
        require(captured_id(0,0)==0 && captured_id(0,1)==1 && captured_id(1,0)==0 && captured_id(1,1)==2 && captured_id(1,2)==3,"capture IDs do not match model/standalone ownership");
        auto *red_faces=red->renderer()->get_triangles_drawable("faces");
        const auto red_color=red_faces->color();const float red_opacity=red_faces->opacity();
        red_faces->set_uniform_coloring(vec4(1,0,0,.25f));red_faces->set_opacity(.6f);
        const auto raw_opaque=grid.capture(1);
        const auto original_settings=grid.render_settings();auto transparent_settings=original_settings;
        transparent_settings.transparency.enabled=true;
        transparent_settings.transparency.method=graphics::TransparencyMethod::sorted;transparent_settings.transparency.layers=4;
        grid.set_render_settings(transparent_settings);
        const auto raw_transparent=grid.capture(1);
        require(raw_transparent.rgba==raw_opaque.rgba && raw_transparent.depth==raw_opaque.depth &&
                raw_transparent.ids.size()==raw_opaque.ids.size() &&
                std::equal(raw_transparent.ids.begin(),raw_transparent.ids.end(),raw_opaque.ids.begin(),
                           [](const graphics::FrameImage::Id& a,const graphics::FrameImage::Id& b) {
                               return a.object==b.object && a.primitive==b.primitive;
                           }),"raw multi-view capture retained presentation opacity");
        grid.set_render_settings(original_settings);
        red_faces->set_uniform_coloring(red_color);red_faces->set_opacity(red_opacity);
        {
            auto msaa=grid.capture(4);require(before.depth==msaa.depth && before.ids.size()==msaa.ids.size(),"MSAA changed per-cell scene depth");
            for(size_t i=0;i<before.ids.size();++i) require(before.ids[i].object==msaa.ids[i].object && before.ids[i].primitive==msaa.ids[i].primitive,"MSAA changed cell source IDs");
        }
        grid.set_division_visible(true);auto divider=grid.capture(1);require(divider.depth==before.depth && divider.rgba!=before.rgba,"divider changed depth or did not draw");
        for(size_t i=0;i<before.ids.size();++i) require(before.ids[i].object==divider.ids[i].object,"divider changed source IDs");
        require(grid.snapshot("multiview-divider.png",1,4,0),"portable grid divider snapshot failed");
        std::vector<unsigned char> portable;int pw,ph,pc;
        require(ImageIO::load("multiview-divider.png",portable,pw,ph,pc,4,false),"portable divider snapshot unreadable");
        require_dividers(portable,pw,ph);
#ifndef EASY3D_HAS_VULKAN
        auto fallback=std::make_shared<NativeFallbackPoints>("native fallback");grid.add_drawable(fallback);
        fallback->update_vertex_buffer({vec3(0)});fallback->set_point_size(23);fallback->set_lighting(false);fallback->set_uniform_coloring(vec4(1,1,0,1));
        grid.assign(0,2,fallback.get());fallback->set_visible(false);
        require(grid.snapshot("multiview-native-divider.png",1,4,0),"native fallback grid snapshot failed");
        fallback->set_visible(true);
        require(grid.snapshot("multiview-native-fallback.png",1,4,0),"visible native fallback snapshot failed");
        std::vector<unsigned char> visible;int vw,vh,vc;
        require(ImageIO::load("multiview-native-fallback.png",visible,vw,vh,vc,4,false),"visible native fallback snapshot unreadable");
        color_at(visible,vw,5*vw/6,vh/4,vec4(1,1,0,1));
        bool fallback_found=false;
        grid.point_under_pixel(5*grid.width()/6,grid.height()/4,fallback_found);
        require(fallback_found,"native fallback did not answer a MultiViewer depth query");
        const auto fallback_hit=grid.pick_cell(grid.bounds(0,2));
        require(fallback_hit.drawable.get()==fallback.get(),"native fallback picking lost its source drawable");
        fallback->set_visible(false);
        std::vector<unsigned char> native;int nw,nh,nc;
        require(ImageIO::load("multiview-native-divider.png",native,nw,nh,nc,4,false),"native fallback snapshot unreadable");
        require_dividers(native,nw,nh);
        require(nw==pw && nh==ph && native.size()==portable.size(),"native and portable divider extents differ");
        color_at(native,nw,nw/6,nh/4,vec4(1,0,0,1));color_at(native,nw,nw/2,nh/4,vec4(0,0,1,1));
        color_at(native,nw,5*nw/6,nh/4,vec4(1));color_at(native,nw,nw/6,3*nh/4,vec4(1,0,0,1));
        color_at(native,nw,nw/2,3*nh/4,vec4(0,1,0,1));color_at(native,nw,5*nw/6,3*nh/4,vec4(1,0,1,1));
        for(size_t p=0;p<portable.size();p+=4) {
            const bool portable_black=portable[p]<5 && portable[p+1]<5 && portable[p+2]<5;
            const bool native_black=native[p]<5 && native[p+1]<5 && native[p+2]<5;
            require(portable_black==native_black,"native and portable divider masks differ");
        }
        require(grid.delete_drawable(fallback.get()),"native fallback fixture removal failed");
#endif
        grid.set_division_visible(false);
        point->set_point_size(300);auto large=grid.capture();const auto empty=grid.bounds(0,2);
        for(int y=empty.y;y<empty.y+empty.w;++y) for(int x=empty.x;x<empty.x+empty.z;++x)
            require(large.ids[size_t(y)*large.width+x].object==UINT32_MAX,"large point escaped its cell scissor");
        point->set_point_size(23);
#ifdef EASY3D_HAS_VULKAN
        const auto uploads=grid.device().point_upload_count();grid.assign(0,2,point.get());grid.capture();require(grid.device().point_upload_count()==uploads,"sharing a point drawable across cells uploaded positions again");
#endif
        point->set_point_size(300);
        const auto vacant=grid.bounds(0,2);
#ifndef EASY3D_HAS_VULKAN
        // The Vulkan-specific sharing check above assigns this cell explicitly.
        require(!grid.pick_cell(vacant),"large point query escaped its cell scissor");
#endif
        point->set_point_size(23);
        bool rejected=false;try{grid.set_layout(0,2);}catch(const std::invalid_argument&){rejected=true;}require(rejected && grid.rows()==2,"invalid layout modified the grid");
        grid.resize(317,237);frame(grid);
        auto position=grid.camera()->position();auto orientation=grid.camera()->orientation();auto pivot=grid.camera()->pivot_point();vec3 delta;
        const auto reset_camera=[&](){grid.camera()->set_position(position);grid.camera()->set_orientation(orientation);grid.camera()->frame()->set_pivot_point(pivot);};
        const auto pan=[&](int x,int y,int dx){
            grid.mouse_press_event(x,y,Viewer::BUTTON_RIGHT,0);grid.mouse_drag_event(x+dx,y,dx,0,Viewer::BUTTON_RIGHT,0);grid.mouse_release_event(x+dx,y,Viewer::BUTTON_RIGHT,0);
            return grid.camera()->position()-position;
        };
        // Legacy mouse hooks take logical window coordinates on both backends (Batch 46).
        // Odd sizes split 317x237 into 105/106-px columns and 118/119-px rows; perspective
        // pan scales with inverse cell height, so strict parity uses equal-sized cells.
        for(int i=0;i<2;++i) {
            reset_camera();
            const auto moved=pan((2*i+3)*grid.width()/6,grid.height()/4,10);
            const auto projected=api.world_to_window(grid.camera()->scene_center());
            require((api.window_to_world(projected)-grid.camera()->scene_center()).norm()<1e-4f,"active-cell coordinate round trip failed");
            if(i==0) delta=moved;else require((moved-delta).norm()<1e-5f,"camera pan depends on the cell's canvas offset");
        }
        reset_camera();
        const auto taller=pan(grid.width()/2,3*grid.height()/4,10); // cell (1,1) is one row pixel taller
        const int h0=grid.height()/2,h1=grid.height()-h0;
        require((taller*h1-delta*h0).norm()<1e-3f,"camera pan did not follow the active cell's size");
        reset_camera();
        const int reach=grid.width()/3;
        const auto crossed=pan(3*grid.width()/6,grid.height()/4,reach); // drag from cell (0,1) into cell (0,2)
        require((crossed-delta*float(reach)/10).norm()<1e-4f,"cross-cell drag lost the pressed cell's origin");
        require(grid.delete_model(red.get()),"model removal failed");frame(grid);grid.point_under_pixel(grid.width()/6,3*grid.height()/4,found);require(!found,"deleted model retained an assigned drawable");
        require(grid.delete_drawable(point.get()),"drawable removal failed");grid.set_layout(3,2);frame(grid);check_cells(grid);require(grid.models().size()==1,"layout changes altered ownership");grid.clear_scene();frame(grid);
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
