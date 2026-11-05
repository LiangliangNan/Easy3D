// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/renderer/render_device.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
using namespace easy3d::graphics;
void require(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
DrawCommand rectangle(float x0,float y0,float x1,float y1,float z,const vec4& color,bool transparent=false) {
    std::vector<Vertex> v(4);
    v[0].position=vec3(x0,y0,z); v[1].position=vec3(x1,y0,z); v[2].position=vec3(x1,y1,z); v[3].position=vec3(x0,y1,z);
    DrawCommand d; d.geometry=std::make_shared<Geometry>(v,std::vector<uint32_t>{0,1,2,0,2,3}); d.color=color;
    d.transparent=transparent; return d;
}
void probe(const FrameImage& f,unsigned x,unsigned y,const int* rgb) {
    const auto i=(size_t(y)*f.width+x)*4;
    for (unsigned c=0;c<3;++c) require(std::abs(int(f.rgba[i+c])-rgb[c])<=2,"transparent blend probe mismatch");
}
void same_ids(const FrameImage& a,const FrameImage& b,const char* message) {
    require(a.ids.size()==b.ids.size(),message);
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,message);
}
int main(int argc,char**) {
    try {
        const bool validation = argc > 1;
        auto device=create_vulkan_device(validation, validation);
        // Identity MVP: clip depth is 0.5*(z+1), so larger z is farther.
        auto green=rectangle(-1,-1,1,1,.5f,vec4(0,1,0,.5f),true); green.object_id=21;
        auto blue=rectangle(-1,-1,1,1,-.5f,vec4(0,0,1,.5f),true); blue.object_id=31;
        auto red=rectangle(-1,-1,1,1,-.9f,vec4(1,0,0,1)); red.object_id=41;
        auto yellow=rectangle(-1,-1,1,1,.9f,vec4(1,1,0,1)); yellow.object_id=51;
        const int expected[]={0,64,128};
        for (unsigned count:{1u,4u}) {
            if (!(device->supported_sample_counts()&count)) continue;
            const auto ordered=device->render(65,65,{green,blue},vec4(0),count);
            probe(ordered,32,32,expected);
            require(ordered.ids[32*65+32].object==31 && ordered.ids[32*65+32].primitive==0,
                    "transparent capture ID did not keep the nearest surface");
            require(std::abs(ordered.depth[32*65+32]-.25f)<1e-5f,"transparent capture depth did not keep the nearest surface");
            const auto reversed=device->render(65,65,{blue,green},vec4(0),count);
            require(ordered.rgba==reversed.rgba,"transparent draws were not sorted far to near");
            same_ids(ordered,reversed,"transparent draw order changed picking IDs");

            const auto occluded=device->render(65,65,{green,red,blue},vec4(0),count);
            const int only_red[]={255,0,0};
            probe(occluded,32,32,only_red);
            require(occluded.ids[32*65+32].object==41,"opaque surface did not occlude transparent surfaces");

            const auto behind=device->render(65,65,{blue,yellow,green},vec4(0),count);
            // Green 0.5 over yellow, then blue 0.5: (0.25, 0.5, 0.5).
            const int blended[]={64,128,128};
            probe(behind,32,32,blended);
            require(behind.ids[32*65+32].object==31,"transparent surface in front of opaque lost its picking ID");

            auto solid=blue; solid.color.w=1;
            const auto opaque=device->render(65,65,{solid,green},vec4(0),count);
            const int only_blue[]={0,0,255};
            probe(opaque,32,32,only_blue);
            require(opaque.ids[32*65+32].object==31,"alpha-one transparent draw lost its picking ID");

            const auto sample=device->sample_pixel(65,65,{blue,green},32,32);
            require(sample.id.object==31 && sample.id.primitive==0,"transparent pixel query returned the wrong object");
            require(std::abs(sample.depth-.25f)<1e-5f,"transparent pixel query returned the wrong depth");

            // One drawable with the near quad listed before the far quad. Sorting
            // only drawables would blend the near surface alone; triangles inside
            // the drawable must also be ordered far to near.
            const float corners[4][2]={{-1,-1},{1,-1},{1,1},{-1,1}};
            std::vector<Vertex> layered_vertices(8);
            for (unsigned i=0;i<4;++i) {
                layered_vertices[i].position=vec3(corners[i][0],corners[i][1],-.5f);
                layered_vertices[i].color=vec4(0,0,1,1);
                layered_vertices[4+i].position=vec3(corners[i][0],corners[i][1],.5f);
                layered_vertices[4+i].color=vec4(0,1,0,1);
            }
            DrawCommand layered;
            layered.geometry=std::make_shared<Geometry>(layered_vertices,
                std::vector<uint32_t>{0,1,2,0,2,3,4,5,6,4,6,7});
            layered.color=vec4(1,1,1,.5f); layered.transparent=true; layered.object_id=61;
            const auto layered_image=device->render(65,65,{layered},vec4(0),count);
            probe(layered_image,32,32,expected);
            require(layered_image.ids[32*65+32].object==61 && layered_image.ids[32*65+32].primitive==0,
                    "per-triangle sorting changed transparent picking IDs");
            require(std::abs(layered_image.depth[32*65+32]-.25f)<1e-5f,
                    "per-triangle sorting lost the nearest transparent depth");
        }
        bool rejected=false;
        auto line=rectangle(-1,-1,1,1,0,vec4(0,1,0,.5f),true);
        line.geometry=std::make_shared<Geometry>(std::vector<Vertex>(2),std::vector<uint32_t>{0,1},Topology::lines);
        try { device->render(65,65,{line}); } catch (const std::invalid_argument&) { rejected=true; }
        require(rejected,"transparent flag was accepted on line geometry");
        auto cover=green; cover.overlay=true; rejected=false;
        try { device->render(65,65,{cover}); } catch (const std::invalid_argument&) { rejected=true; }
        require(rejected,"transparent flag was accepted on an overlay");
        auto bad_alpha=green; bad_alpha.color.w=1.2f; rejected=false;
        try { device->render(65,65,{bad_alpha}); } catch (const std::invalid_argument&) { rejected=true; }
        require(rejected,"out-of-range transparent alpha was accepted");
        require(device->validation_errors()==0,"transparency validation errors");
        std::cout << "PASS: depth-sorted straight-alpha scene triangles, opaque occlusion, nearest IDs/depth, order independence, pixel queries, MSAA\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
