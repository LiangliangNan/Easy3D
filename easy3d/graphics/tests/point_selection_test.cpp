// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/graphics/render_device.h>
#include <iostream>
#include <algorithm>
#include <limits>
#include <stdexcept>
using namespace easy3d;
using namespace easy3d::graphics;
void require(bool ok,const char* message) { if (!ok) throw std::runtime_error(message); }
bool bit(const std::vector<uint32_t>& words,size_t i) { return (words[i>>5]>>(i&31))&1u; }
int main(int argc,char**) {
    try {
        DeviceOptions options; options.validation=options.synchronization_validation=argc>1;
        bool bad_source=false;
        try { PointData invalid({vec3(0)},{32},32); } catch(const std::invalid_argument&) { bad_source=true; }
        require(bad_source,"out-of-range source bit index accepted");
        auto device=create_vulkan_device(options);
        PointSelectionRequest r; r.width=r.height=1000;
        r.polygon={{100,100},{900,100},{900,300},{300,300},{300,900},{100,900}};
        const unsigned count=100003;
        std::vector<vec3> positions;
        for (unsigned i=0;i<count;++i) {
            const float x=float(i%1000),y=float((i*37)%1000);
            positions.emplace_back(x/500-1,1-y/500,float(i%3)*.2f);
        }
        r.points=std::make_shared<PointData>(std::move(positions));
        r.words.resize((count+31)/32,0x55555555u);
        auto selected=device->select_points(r);
        const auto initial_uploads=device->point_upload_count();
        require(initial_uploads==1,"initial point resource was not uploaded exactly once");
        for (unsigned i=0;i<count;++i) {
            const unsigned x=i%1000,y=(i*37)%1000;
            const bool inside=x>=100 && x<=900 && y>=100 && y<=900 && (x<=300 || y<=300);
            require(bit(selected,i)==(inside || bit(r.words,i)),"GPU concave selection or shared-word atomicOr mismatch");
        }
        const uint32_t padding=~((1u<<(count%32))-1);
        require((selected.back()&padding)==(r.words.back()&padding),"selection changed padding bits");
        r.words=selected; r.deselect=true;
        auto deselected=device->select_points(r);
        for (unsigned i=0;i<count;++i) {
            const unsigned x=i%1000,y=(i*37)%1000;
            const bool inside=x>=100 && x<=900 && y>=100 && y<=900 && (x<=300 || y<=300);
            require(bit(deselected,i)==(!inside && bit(selected,i)),"GPU atomicAnd deselection mismatch");
        }
        std::reverse(r.polygon.begin(),r.polygon.end()); r.deselect=false; r.words=deselected;
        require(device->select_points(r)==selected,"polygon orientation changed selection");
        require(device->point_upload_count()==initial_uploads,"repeated selection re-uploaded positions");
        // Indexed/repeated positions use original source slots, independently of
        // generated picking primitive IDs. Rendering consumes the same buffer.
        std::vector<Vertex> v(3);
        v[0].position=vec3(-.5f,0,0); v[1].position=vec3(0,.5f,0); v[2].position=vec3(.5f,0,0);
        auto geometry=std::make_shared<Geometry>(v,std::vector<uint32_t>{2,0,2},Topology::points);
        DrawCommand draw; draw.geometry=geometry; draw.point_size=9;
        const auto before=device->point_upload_count();
        auto image=device->render(100,100,{draw});
        PointSelectionRequest shared; shared.points=geometry->point_data(); shared.width=shared.height=100;
        shared.polygon={{0,0},{100,0},{100,100},{0,100}};
        require(device->select_points(shared)[0]==5,"indexed point source mapping was lost");
        require(device->point_upload_count()==before+1,"selection duplicated the rendered position buffer");
        v[0].color=vec4(1,0,0,1); v[2].color=vec4(0,1,0,1);
        draw.geometry=std::make_shared<Geometry>(v,std::vector<uint32_t>{2,0,2},Topology::points,
                                               std::vector<uint32_t>{},geometry->point_data());
        require(device->render(100,100,{draw}).rgba!=image.rgba,"split point attribute colors did not update");
        draw.clipping_planes={vec4(1,0,0,0)};
        device->render(100,100,{draw});
        require(device->point_upload_count()==before+1,"color/clipping changes re-uploaded point positions");
        // A selection-first resource is also reused by subsequent rendering.
        auto sparse=std::make_shared<PointData>(std::vector<vec3>{vec3(-.5f,0,0),vec3(.5f,0,0)},
                                              std::vector<uint32_t>{63,2},65);
        shared.points=sparse; const auto sparse_before=device->point_upload_count();
        auto sparse_words=device->select_points(shared);
        require(sparse_words.size()==3 && sparse_words[0]==4 && sparse_words[1]==0x80000000u && sparse_words[2]==0,
                "source IDs across bit words were not preserved");
        std::vector<Vertex> sparse_vertices{v[0],v[2]};
        draw.geometry=std::make_shared<Geometry>(sparse_vertices,std::vector<uint32_t>{},Topology::points,
                                               std::vector<uint32_t>{},sparse);
        draw.clipping_planes.clear(); device->render(100,100,{draw});
        require(device->point_upload_count()==sparse_before+1,"rendering duplicated a selection-first point resource");
        // Every invocation targets a bit in the same small set of words.
        PointSelectionRequest all; all.width=all.height=100;
        all.polygon={{0,0},{100,0},{100,100},{0,100}};
        all.points=std::make_shared<PointData>(std::vector<vec3>(1025,vec3(0)));
        auto words=device->select_points(all);
        for (unsigned i=0;i<1025;++i) require(bit(words,i),"concurrent same-word updates lost bits");
        all.words=words; all.deselect=true;
        for (auto word:device->select_points(all)) require(word==0,"concurrent deselection lost bits");
        // Bow-tie even-odd fill, edge inclusion, frustum and invalid point slots.
        PointSelectionRequest bow; bow.width=bow.height=100;
        bow.polygon={{10,10},{90,90},{10,90},{90,10}};
        std::vector<vec3> bow_positions;
        auto add=[&](float x,float y,float z=0) { bow_positions.emplace_back(x/50-1,1-y/50,z); };
        add(50,20); add(50,80); add(20,50); add(80,50); add(50,50); add(50,20,2);
        bow_positions.emplace_back(std::numeric_limits<float>::quiet_NaN(),0,0);
        bow.points=std::make_shared<PointData>(std::move(bow_positions));
        auto b=device->select_points(bow);
        require(b[0]==0x13,"self-intersection, edge, frustum or deleted-slot handling mismatch");
        bow.local_clipping_planes={vec4(0,1,0,0)};
        require(device->select_points(bow)[0]==0x11,"local clipping halfspace was ignored");
        bow.local_clipping_planes.clear(); bow.model_view_projection(3,3)=-1;
        require(device->select_points(bow)[0]==0,"points behind the eye were selected");
        bow.model_view_projection=mat4::identity(); bow.model_view_projection(0,3)=.8f;
        require(device->select_points(bow)[0]!=b[0],"model/camera transform was ignored");
        // Degenerate/empty requests are no-ops; invalid requests fail before GPU work.
        all.polygon={{0,0},{1,1},{2,2}}; all.words=words;
        require(device->select_points(all)==words,"degenerate polygon changed selection");
        all.polygon.clear(); require(device->select_points(all)==words,"empty polygon changed selection");
        all.width=0; bool rejected=false;
        try { device->select_points(all); } catch(const std::invalid_argument&) { rejected=true; }
        require(rejected,"zero viewport accepted"); all.width=100;
        all.words={1}; rejected=false;
        try { device->select_points(all); } catch(const std::invalid_argument&) { rejected=true; }
        require(rejected,"bad bitset size accepted");
        all.words.clear(); all.polygon={{0,0},{100,0},{100,100}};
        all.polygon[0].x=std::numeric_limits<float>::infinity(); rejected=false;
        try { device->select_points(all); } catch(const std::invalid_argument&) { rejected=true; }
        require(rejected,"nonfinite polygon accepted");
        // Dispatches do not invalidate render targets, readback or another device.
        const auto baseline=device->render(31,27,{});
        device->select_points(r);
        require(device->render(31,27,{}).rgba==baseline.rgba,"compute changed rendering state");
        auto second=create_vulkan_device(options);
        require(second->select_points(r)==selected,"independent device selection mismatch");
        require(device->validation_errors()==0 && second->validation_errors()==0,"selection validation errors");
        std::cout << "PASS: shared vertex/SSBO point resources, single position uploads, indexed/source IDs, attribute/clipping reuse, GPU lasso atomics, validation and device isolation\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
