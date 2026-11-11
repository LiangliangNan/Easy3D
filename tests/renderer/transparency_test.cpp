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
        if (a.ids[i].object!=b.ids[i].object || a.ids[i].primitive!=b.ids[i].primitive) {
            std::ostringstream text;
            text << message << ": pixel " << (i%b.width) << ',' << (i/b.width) << " got object "
                 << a.ids[i].object << " primitive " << a.ids[i].primitive << ", expected object "
                 << b.ids[i].object << " primitive " << b.ids[i].primitive;
            throw std::runtime_error(text.str());
        }
}
// The full-screen quads are split along a diagonal through the target center, so
// the sample of pixel (32,32) lies exactly on the shared edge of two coplanar
// triangles at equal depth. Which triangle the rasterizer attributes that sample
// to is not portable, so primitive checks sample a pixel well inside each triangle
// instead. Depth and object IDs are the same at every pixel of a full-screen quad,
// so they stay checked at the center.
constexpr unsigned first_triangle_x=56, first_triangle_y=56;
constexpr unsigned second_triangle_x=8, second_triangle_y=8;
void probe_id(const FrameImage& f,unsigned x,unsigned y,unsigned object,unsigned primitive,const char* label) {
    const auto& id=f.ids[size_t(y)*f.width+x];
    if (id.object!=object || id.primitive!=primitive) {
        std::ostringstream message;
        message << label << " at " << x << ',' << y << ": got object " << id.object << " primitive "
                << id.primitive << ", expected object " << object << " primitive " << primitive;
        throw std::runtime_error(message.str());
    }
}
// Different methods accumulate at different precisions, so an exact peel must
// still be compared with the probe tolerance rather than bit for bit.
void close_rgba(const FrameImage& a,const FrameImage& b,const char* message) {
    require(a.rgba.size()==b.rgba.size(),message);
    for (size_t i=0;i<a.rgba.size();++i)
        require(std::abs(int(a.rgba[i])-int(b.rgba[i]))<=2,message);
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
            require(ordered.ids[32*65+32].object==31,"transparent capture ID did not keep the nearest surface");
            probe_id(ordered,first_triangle_x,first_triangle_y,31,0,"transparent capture first triangle ID");
            probe_id(ordered,second_triangle_x,second_triangle_y,31,1,"transparent capture second triangle ID");
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

            const auto sample=device->sample_pixel(65,65,{blue,green},first_triangle_x,first_triangle_y);
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
            require(layered_image.ids[32*65+32].object==61,
                    "per-triangle sorting changed transparent picking IDs");
            probe_id(layered_image,first_triangle_x,first_triangle_y,61,0,
                     "per-triangle sorting changed transparent picking primitive");
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

            // One layer keeps only the nearest surface; two layers put the far
            // one under it, which is exactly the depth-sorted blend. Blending the
            // peeled layers the other way round would swap the two colors.
            const auto peeled_near=device->render(65,65,{green,blue},vec4(0),count,TransparencyMethod::peeling,1);
            const int near_expected[]={0,0,128};
            probe(peeled_near,32,32,near_expected,"peel one");
            const auto peeled=device->render(65,65,{green,blue},vec4(0),count,TransparencyMethod::peeling,2);
            const int peeled_expected[]={0,64,128};
            probe(peeled,32,32,peeled_expected,"peel two");
            close_rgba(peeled,sorted,"depth peeling did not composite its layers front to back");
            require(peeled.rgba!=averaged.rgba,"depth peeling matched average transparency");
            const auto peeled_reversed=device->render(65,65,{blue,green},vec4(0),count,TransparencyMethod::peeling,2);
            require(peeled.rgba==peeled_reversed.rgba,"depth peeling was not order independent");

            // Stopping after two layers drops the farthest surface; asking for
            // every layer (0) reproduces the sorted blend of all three.
            const auto peeled_capped=device->render(65,65,{green,blue,opaque_yellow},vec4(0),count,
                                                    TransparencyMethod::peeling,2);
            probe(peeled_capped,32,32,peeled_expected,"peel capped");
            const auto sorted_three=device->render(65,65,{green,blue,opaque_yellow},vec4(0),count);
            const auto peeled_all=device->render(65,65,{green,blue,opaque_yellow},vec4(0),count,
                                                 TransparencyMethod::peeling,0);
            // Blue over green over opaque yellow: (.25,.5,.5).
            const int opaque_expected[]={64,128,128};
            probe(peeled_all,32,32,opaque_expected,"peel all");
            close_rgba(peeled_all,sorted_three,"unlimited depth peeling did not reproduce the sorted blend");
            require(peeled_capped.rgba!=peeled_all.rgba,"peel layer limit was ignored");

            const auto averaged_three=device->render(65,65,{green,blue,opaque_yellow},vec4(0),count,
                                                     TransparencyMethod::average,0);
            // Mean color (.5,.75,.25), mean alpha 2/3, n=3.
            const int average_three_expected[]={123,184,61};
            probe(averaged_three,32,32,average_three_expected,"average three");

            // Picking keeps the nearest transparent surface for both methods.
            for (const FrameImage* image:{&averaged,&peeled}) {
                require(image->ids[32*65+32].object==31,
                        "order-independent transparency changed picking IDs");
                probe_id(*image,first_triangle_x,first_triangle_y,31,0,
                         "order-independent transparency changed picking primitive");
                require(std::abs(image->depth[32*65+32]-.25f)<1e-5f,
                        "order-independent transparency changed picking depth");
            }

            // A deep stack of alternating red/blue surfaces, nearest first. With
            // every layer peeled the result must equal the depth-sorted blend, and
            // a truncated peel must equal the sorted blend of the surfaces it kept.
            // Accumulating the layers the wrong way round fails both.
            std::vector<DrawCommand> stack, near_half;
            for (unsigned i=0;i<8;++i) {
                auto layer=rectangle(-1,-1,1,1,-.9f+.25f*float(i),
                                     i%2 ? vec4(1,0,0,.5f) : vec4(0,0,1,.5f),true);
                layer.object_id=100+i;
                stack.push_back(layer);
                if (i<4) near_half.push_back(layer);
            }
            const auto stacked_sorted=device->render(65,65,stack,vec4(0),count);
            const auto stacked_peeled=device->render(65,65,stack,vec4(0),count,TransparencyMethod::peeling,0);
            close_rgba(stacked_peeled,stacked_sorted,"eight-layer peeling did not reproduce the sorted blend");
            const auto stacked_truncated=device->render(65,65,stack,vec4(0),count,TransparencyMethod::peeling,4);
            close_rgba(stacked_truncated,device->render(65,65,near_half,vec4(0),count),
                       "a truncated peel did not keep the nearest layers");
            require(stacked_truncated.rgba!=stacked_peeled.rgba,"the peel layer limit changed nothing");
            const std::vector<DrawCommand> reversed_stack(stack.rbegin(),stack.rend());
            close_rgba(device->render(65,65,reversed_stack,vec4(0),count,TransparencyMethod::peeling,0),
                       stacked_peeled,"eight-layer peeling was not order independent");

            // Per-pixel linked lists store every fragment, so the resolve must
            // reproduce the depth-sorted blend exactly and stay order independent.
            const auto linked=device->render(65,65,{green,blue},vec4(0),count,TransparencyMethod::linked_lists,8);
            probe(linked,32,32,expected,"linked lists two");
            close_rgba(linked,sorted,"linked lists did not reproduce the sorted blend");
            require(linked.rgba!=averaged.rgba,"linked lists matched average transparency");
            const auto linked_reversed=device->render(65,65,{blue,green},vec4(0),count,TransparencyMethod::linked_lists,8);
            require(linked.rgba==linked_reversed.rgba,"linked lists transparency was not order independent");
            require(linked.ids[32*65+32].object==31,"linked lists changed picking IDs");
            probe_id(linked,first_triangle_x,first_triangle_y,31,0,"linked lists changed first triangle picking ID");
            probe_id(linked,second_triangle_x,second_triangle_y,31,1,"linked lists changed second triangle picking ID");
            require(std::abs(linked.depth[32*65+32]-.25f)<1e-5f,"linked lists changed picking depth");
            const auto linked_three=device->render(65,65,{green,blue,opaque_yellow},vec4(0),count,
                                                   TransparencyMethod::linked_lists,8);
            close_rgba(linked_three,sorted_three,"linked lists did not composite over the opaque scene");
            const auto linked_stack=device->render(65,65,stack,vec4(0),count,TransparencyMethod::linked_lists,8);
            close_rgba(linked_stack,stacked_sorted,"eight-layer linked lists did not reproduce the sorted blend");
            close_rgba(device->render(65,65,reversed_stack,vec4(0),count,TransparencyMethod::linked_lists,8),
                       linked_stack,"eight-layer linked lists was not order independent");
        }
        // Linked-list pool overflow is reported rather than silent. A tiny pool
        // exhausted by stacked full-screen surfaces drops the excess fragments and
        // counts them; a sufficient pool and non-linked-list frames report zero.
        {
            std::vector<DrawCommand> many;
            for (unsigned i=0;i<4;++i) {
                auto layer=rectangle(-1,-1,1,1,-.9f+.5f*float(i),vec4(0,0,1,.5f),true);
                layer.object_id=200+i;
                many.push_back(layer);
            }
            // 16x16 pixels x 1 fragment/pixel = 256 nodes; four full-screen surfaces
            // attempt 4 x 256 = 1024 appends, so 768 fragments are dropped.
            device->render(16,16,many,vec4(0),1,TransparencyMethod::linked_lists,1);
            require(device->oit_dropped_fragments()==768,"linked lists did not report the overflowed fragment count");
            // Capacity 4 gives 1024 nodes, exactly enough for all 1024 appends.
            device->render(16,16,many,vec4(0),1,TransparencyMethod::linked_lists,4);
            require(device->oit_dropped_fragments()==0,"linked lists reported overflow for a sufficient pool");
            device->render(16,16,many,vec4(0),1,TransparencyMethod::sorted);
            require(device->oit_dropped_fragments()==0,"a sorted frame did not clear the overflow diagnostic");
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
        std::cout << "PASS: depth-sorted straight-alpha scene triangles, opaque occlusion, nearest IDs/depth, order independence, pixel queries, MSAA, average-color, depth-peel and per-pixel linked lists OIT with overflow diagnostics\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
