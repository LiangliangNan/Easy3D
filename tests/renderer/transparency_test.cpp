// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/renderer/render_device.h>
#include <cmath>
#include <sstream>
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
void probe(const FrameImage& f,unsigned x,unsigned y,const int* rgb,const char* label="") {
    const auto i=(size_t(y)*f.width+x)*4;
    for (unsigned c=0;c<3;++c) if (std::abs(int(f.rgba[i+c])-rgb[c])>2) {
        std::ostringstream message;
        message << "transparent blend probe mismatch (" << label << ") at " << x << ',' << y << ": got ("
                << int(f.rgba[i]) << ',' << int(f.rgba[i+1]) << ',' << int(f.rgba[i+2])
                << ") expected (" << rgb[0] << ',' << rgb[1] << ',' << rgb[2] << ')';
        throw std::runtime_error(message.str());
    }
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
        // Order-independent transparency: the average-color and depth-peeling
        // methods accumulate instead of sorting, so they differ from each other
        // and from the depth-sorted pass while staying order independent.
        auto opaque_yellow=yellow; opaque_yellow.transparent=true;
        for (unsigned count:{1u,4u}) {
            if (!(device->supported_sample_counts()&count)) continue;
            const auto sorted=device->render(65,65,{green,blue},vec4(0),count);
            probe(sorted,32,32,expected,"oit sorted");

            const auto averaged=device->render(65,65,{green,blue},vec4(0),count,TransparencyMethod::average,4);
            // Mean color (0,.5,.5), mean alpha .5, n=2: T=.25, so (0,.375,.375).
            const int average_expected[]={0,96,96};
            probe(averaged,32,32,average_expected,"average two");
            require(averaged.rgba!=sorted.rgba,"average transparency matched the sorted pass");
            const auto averaged_reversed=device->render(65,65,{blue,green},vec4(0),count,TransparencyMethod::average,4);
            require(averaged.rgba==averaged_reversed.rgba,"average transparency was not order independent");

            // One layer keeps only the nearest surface; two layers blend it over.
            const auto peeled_near=device->render(65,65,{green,blue},vec4(0),count,TransparencyMethod::peeling,1);
            const int near_expected[]={0,0,128};
            probe(peeled_near,32,32,near_expected,"peel one");
            const auto peeled=device->render(65,65,{green,blue},vec4(0),count,TransparencyMethod::peeling,2);
            const int peeled_expected[]={0,128,64};
            probe(peeled,32,32,peeled_expected,"peel two");
            require(peeled.rgba!=averaged.rgba,"depth peeling matched average transparency");
            const auto peeled_reversed=device->render(65,65,{blue,green},vec4(0),count,TransparencyMethod::peeling,2);
            require(peeled.rgba==peeled_reversed.rgba,"depth peeling was not order independent");

            // The farthest opaque layer truncates the accumulation; asking for
            // every layer (0) reproduces the sorted result.
            const auto peeled_capped=device->render(65,65,{green,blue,opaque_yellow},vec4(0),count,
                                                    TransparencyMethod::peeling,2);
            probe(peeled_capped,32,32,peeled_expected,"peel capped");
            const auto peeled_all=device->render(65,65,{green,blue,opaque_yellow},vec4(0),count,
                                                 TransparencyMethod::peeling,0);
            const int opaque_expected[]={255,255,0};
            probe(peeled_all,32,32,opaque_expected,"peel all");
            require(peeled_capped.rgba!=peeled_all.rgba,"peel layer limit was ignored");

            const auto averaged_three=device->render(65,65,{green,blue,opaque_yellow},vec4(0),count,
                                                     TransparencyMethod::average,0);
            // Mean color (.5,.75,.25), mean alpha 2/3, n=3.
            const int average_three_expected[]={123,184,61};
            probe(averaged_three,32,32,average_three_expected,"average three");

            // Picking keeps the nearest transparent surface for both methods.
            for (const FrameImage* image:{&averaged,&peeled}) {
                require(image->ids[32*65+32].object==31 && image->ids[32*65+32].primitive==0,
                        "order-independent transparency changed picking IDs");
                require(std::abs(image->depth[32*65+32]-.25f)<1e-5f,
                        "order-independent transparency changed picking depth");
            }
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
        std::cout << "PASS: depth-sorted straight-alpha scene triangles, opaque occlusion, nearest IDs/depth, order independence, pixel queries, MSAA, average-color and depth-peel OIT\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
