// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace easy3d;
void require(bool ok,const char* text) {if (!ok) throw std::runtime_error(text);}
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"screen lines changed scene depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"screen lines changed source IDs");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        graphics::DeviceOptions options;options.validation=options.synchronization_validation=argc>1;
        VulkanOffScreen v(161,127,options);v.set_background_color(vec4(0));
        auto lines=std::make_shared<LinesDrawable2D>("screen polyline");v.add_drawable(lines);
        const std::vector<vec2> points{vec2(10,15),vec2(130,15),vec2(130,100)};
        lines->update_vertex_buffer(points,161,127);lines->update_element_buffer(std::vector<unsigned>{0,1,1,2});
        lines->set_uniform_coloring(vec4(1,0,0,1));
        const auto center=v.camera()->sceneCenter();const float radius=v.camera()->sceneRadius();v.fit_screen();
        require(v.camera()->sceneCenter()==center && v.camera()->sceneRadius()==radius,"2D bounds changed camera fit");
        for (unsigned count:{1u,4u}) {
            if (!(v.device().supported_sample_counts()&count)) continue;
            const auto image=v.capture(count);unsigned colored=0,partial=0;
            for (unsigned y=0;y<image.height;++y) for (unsigned x=0;x<image.width;++x) {
                const auto i=size_t(y)*image.width+x;
                require(image.depth[i]==1 && image.ids[i].object==UINT32_MAX,"2D line became a world surface");
                if (!image.rgba[4*i+3]) continue;
                require(x>=9 && x<=131 && y>=14 && y<=101,"screen-space coordinates mapped incorrectly");
                ++colored;partial+=image.rgba[4*i+3]<255;
            }
            require(colored>100,"indexed 2D polyline did not render");if (count==4) require(partial>30,"2D ribbons lack MSAA coverage");
            v.camera()->setPosition(v.camera()->position()+vec3(2,3,4));
            v.camera()->setOrientation(v.camera()->orientation()*quat(vec3(0,1,0),.3f));v.camera()->setType(Camera::ORTHOGRAPHIC);
            v.set_clipping_planes({vec4(1,0,0,-10000)});
            require(v.capture(count).rgba==image.rgba,"camera/clipping changed screen-space polyline");
            require(!v.pick(60,15) && v.scan_visible_points(count).empty(),"screen line participated in picking/scanning");
            lines->set_visible(false);const auto hidden=v.capture(count);same_picking(image,hidden);
            for (auto c:hidden.rgba) require(c==0,"hidden screen lines left stale pixels");
            lines->set_visible(true);lines->set_uniform_coloring(vec4(0,1,0,.5f));
            require(v.capture(count).rgba!=image.rgba,"screen line color update was stale");
            lines->set_uniform_coloring(vec4(1,0,0,1));require(v.capture(count).rgba==image.rgba,"screen line color restore failed");
        }
        auto coverage=[](const graphics::FrameImage& image) {
            size_t result=0;for(size_t i=3;i<image.rgba.size();i+=4) result+=image.rgba[i]!=0;return result;
        };
        const auto narrow=v.capture();const auto geometry_revision=lines->cpu_buffers().revision;
        lines->set_line_width(7);const auto wide=v.capture();same_picking(narrow,wide);
        require(coverage(wide)>coverage(narrow)*3,"line width did not expand indexed ribbons");
        require(lines->cpu_buffers().revision==geometry_revision,"width change reuploaded positions");
        for(float invalid:{0.0f,-1.0f,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}) {
            bool rejected=false;try {lines->set_line_width(invalid);}catch(const std::invalid_argument&) {rejected=true;}
            require(rejected && lines->line_width()==7,"invalid line width changed state");
        }
        lines->set_line_width(2.5f);require(coverage(v.capture())>coverage(narrow),"fractional width ignored");
        lines->set_line_width(1);require(v.capture().rgba==narrow.rgba,"line width restore left stale cache");
        const auto stable=v.capture();const auto revision=lines->cpu_buffers().revision;
        for (int width:{0,-1}) {
            bool rejected=false;try {lines->update_vertex_buffer(points,width,127);} catch(const std::invalid_argument&) {rejected=true;}
            require(rejected && lines->cpu_buffers().revision==revision,"invalid viewport changed 2D buffer");
        }
        bool rejected=false;try {lines->update_vertex_buffer({vec2(std::numeric_limits<float>::quiet_NaN(),1)},161,127);}
        catch(const std::invalid_argument&) {rejected=true;}
        require(rejected && v.capture().rgba==stable.rgba,"invalid coordinate changed 2D rendering");
        lines->update_element_buffer(std::vector<unsigned>{0,99});rejected=false;
        try {v.capture();} catch(const std::out_of_range&) {rejected=true;}
        require(rejected,"invalid screen index accepted");
        lines->update_element_buffer(std::vector<unsigned>{0});rejected=false;
        try {v.capture();} catch(const std::invalid_argument&) {rejected=true;}
        require(rejected,"odd screen endpoint count accepted");
        lines->update_element_buffer(std::vector<unsigned>{0,1,1,2});
        v.resize(251,173);const auto resized=v.capture();unsigned covered=0;
        for (unsigned x=30;x<180;++x) for (unsigned y=19;y<23;++y)
            covered+=resized.rgba[(size_t(y)*resized.width+x)*4+3]>0;
        require(covered>100,"resize did not preserve normalized line placement");
        lines->update_vertex_buffer({},251,173);lines->disable_element_buffer();
        const auto cleared=v.capture();for (auto c:cleared.rgba) require(c==0,"empty screen drawable retained geometry");
        v.set_clipping_planes({});v.camera()->setUpVector(vec3(0,1,0));v.camera()->setViewDirection(vec3(0,0,-1));
        auto* model=v.add_model(resource::directory()+"/data/sphere.obj");require(model,"world fixture missing");
        auto world=std::make_shared<LinesDrawable>("world line");v.add_drawable(world);
        world->update_vertex_buffer({vec3(30,-5,0),vec3(30,5,0)});world->set_line_width(3);v.fit_screen();
        const float scene_radius=v.camera()->sceneRadius();const auto scene_center=v.camera()->sceneCenter();
        lines->update_vertex_buffer({vec2(1e6f,1e6f),vec2(2e6f,2e6f)},251,173);v.fit_screen();
        require(v.camera()->sceneRadius()==scene_radius && v.camera()->sceneCenter()==scene_center,"2D coordinates polluted world bounds");
        lines->update_vertex_buffer({vec2(0,86.5f),vec2(251,86.5f)},251,173);
        lines->set_visible(false);const auto bare=v.capture();lines->set_visible(true);const auto overlay=v.capture();same_picking(bare,overlay);
        require(bare.rgba!=overlay.rgba,"2D line did not overlay opaque geometry");
        const auto pixel=v.camera()->projectedCoordinatesOf(vec3(30,0,0));const auto hit=v.pick(int(pixel.x),int(pixel.y));
        if (!hit || hit.drawable.get()!=world.get())
            std::cerr<<"world-line probe "<<pixel<<" hit "<<(hit ? hit.drawable->name() : "background")<<'\n';
        require(hit && hit.drawable.get()==world.get(),"2D drawable corrupted later 3D picking provenance");
        require(v.device().validation_errors()==0,"screen-line validation errors");
        std::cout<<"PASS: native indexed 2D lines, camera/clipping independence, MSAA, resize, validation, world bounds and preserved depth/picking\n";
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
