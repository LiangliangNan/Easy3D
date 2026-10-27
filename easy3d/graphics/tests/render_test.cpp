// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/graphics/render_device.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
using namespace easy3d::graphics;

void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
template<class Function> void rejects(Function f) {
    bool threw = false;
    try { f(); } catch (const std::exception&) { threw = true; }
    require(threw, "invalid input was accepted");
}
std::shared_ptr<const Geometry> triangle(float z, const vec4& color) {
    std::vector<Vertex> v(3);
    v[0].position = vec3(-0.8f, -0.8f, z);
    v[1].position = vec3(0.8f, -0.8f, z);
    v[2].position = vec3(0, 0.8f, z);
    for (auto& p : v) p.color = color;
    return std::make_shared<Geometry>(v, std::vector<uint32_t>{0, 1, 2},
                                     Topology::triangles, std::vector<uint32_t>{173});
}
void point_styles(RenderDevice& device) {
    Vertex v; v.position = vec3(0);
    DrawCommand d;
    d.geometry = std::make_shared<Geometry>(std::vector<Vertex>{v}, std::vector<uint32_t>{0,0},
                                           Topology::points, std::vector<uint32_t>{71,92});
    d.object_id = 33; d.point_size = 12;
    d.color = vec4(1,0,0,1);
    const auto square = device.render(64,48,{d},vec4(0));
    unsigned hits = 0;
    for (size_t i=0;i<square.ids.size();++i) if (square.ids[i].object == 33) {
        ++hits;
        require(square.ids[i].primitive == 71, "expanded indexed point lost original ID");
        require(std::abs(square.depth[i]-0.5f)<1e-6f, "large point depth differs from center");
    }
    require(hits == 144, "large point is not a framebuffer-pixel square");
    auto check_msaa = [&](unsigned w, unsigned h, const FrameImage& single) {
        if (!(device.supported_sample_counts(d.point_style) & 4)) return;
        auto aa = device.render(w,h,{d},vec4(0),4);
        unsigned edges = 0;
        for (size_t i=0;i<aa.ids.size();++i) {
            require(aa.ids[i].object == single.ids[i].object && aa.ids[i].primitive == single.ids[i].primitive,
                    "point/sphere MSAA changed picking IDs");
            require(std::abs(aa.depth[i]-single.depth[i])<1e-5f, "point/sphere MSAA changed picking depth");
            edges += aa.rgba[4*i] > 0 && aa.rgba[4*i] < 255;
        }
        require(edges > 0, "point/sphere has no MSAA coverage");
    };
    d.point_size = 12.5f;
    check_msaa(64,48,device.render(64,48,{d},vec4(0)));
    d.point_style = PointStyle::sphere; d.sphere_radius = 0.35f;
    auto sphere = device.render(65,65,{d},vec4(0));
    require(std::abs(sphere.depth[32*65+32]-0.325f)<1e-5f, "orthographic sphere surface depth is wrong");
    require(sphere.ids[22*65+22].object == UINT32_MAX, "sphere proxy corner was not discarded");
    check_msaa(65,65,sphere);
    d.lighting=true; d.sphere_light=vec3(0,0,1);
    auto lit=device.render(65,65,{d});
    require(lit.rgba[(32*65+32)*4]==255 && lit.rgba[(32*65+41)*4]<220,
            "sphere lighting does not follow its surface normal");
    d.lighting=false; d.color=vec4(1);
    d.texture=std::make_shared<Image>(1,1,std::vector<unsigned char>{0,255,0,255});
    auto textured=device.render(65,65,{d});
    require(textured.rgba[(32*65+32)*4]==0 && textured.rgba[(32*65+32)*4+1]==255,
            "sphere texture descriptor failed");
    d.texture.reset(); d.color=vec4(1,0,0,1);
    // A plane crosses the sphere: its center is in front, its rim behind.
    DrawCommand plane; plane.geometry = triangle(-0.2f,vec4(0,1,0,1)); plane.object_id=19;
    for (bool reverse : {false,true}) {
        auto overlap = device.render(65,65,reverse ? std::vector<DrawCommand>{plane,d} : std::vector<DrawCommand>{d,plane});
        require(overlap.ids[32*65+32].object==33 && overlap.ids[32*65+42].object==19,
                "sphere/mesh depth occlusion depends on proxy/order");
    }
    // Off-axis perspective sphere checked against an independent CPU ray test.
    v.position=vec3(0.6f,0.2f,-3);
    d.geometry=std::make_shared<Geometry>(std::vector<Vertex>{v},std::vector<uint32_t>{},Topology::points,
                                         std::vector<uint32_t>{87});
    d.sphere_radius=0.7f;
    d.projection=mat4(0.0f);
    d.projection(0,0)=1.3f; d.projection(1,1)=1.6f;
    d.projection(2,2)=-11.0f/9; d.projection(2,3)=-20.0f/9; d.projection(3,2)=-1;
    const auto perspective=device.render(93,71,{d},vec4(0));
    unsigned expected_hits=0;
    for (unsigned y=0;y<71;++y) for (unsigned x=0;x<93;++x) {
        vec3 ray=normalize(vec3((2*(x+0.5f)/93-1)/1.3f,(1-2*(y+0.5f)/71)/1.6f,-1));
        const double b=dot(ray,v.position);
        const double disc=b*b-dot(v.position,v.position)+0.49;
        const size_t i=y*93+x;
        if (disc>1e-5) {
            ++expected_hits;
            const vec3 hit=ray*float(b-std::sqrt(disc));
            const vec4 clip=d.projection*vec4(hit,1);
            require(perspective.ids[i].object==33 && perspective.ids[i].primitive==87, "perspective sphere silhouette/ID mismatch");
            require(std::abs(perspective.depth[i]-(clip.z/clip.w+1)*0.5f)<2e-5f,"perspective sphere ray depth mismatch");
        } else if (disc < -1e-5)
            require(perspective.ids[i].object==UINT32_MAX,"perspective sphere proxy leaked");
    }
    require(expected_hits>100,"perspective sphere oracle did not exercise enough pixels");
    check_msaa(93,71,perspective);
    // Camera/model transform of centers and perspective near/far clipping.
    d.model_view(0,3)=-0.6f; d.model_view(1,3)=-0.2f;
    d.model_view(2,3)=4; // entirely behind the eye
    auto clipped=device.render(65,65,{d});
    for (auto id:clipped.ids) require(id.object==UINT32_MAX,"sphere behind eye was visible");
    d.model_view(2,3)=-9; // beyond far plane
    clipped=device.render(65,65,{d});
    for (auto id:clipped.ids) require(id.object==UINT32_MAX,"sphere beyond far plane was visible");
    d.model_view(2,3)=1.4f; // center -1.6, front intersects near plane
    clipped=device.render(65,65,{d});
    require(clipped.ids[32*65+32].object==UINT32_MAX,"sphere front surface escaped near clipping");
    unsigned rim=0;
    for (auto id:clipped.ids) rim+=id.object==33;
    require(rim>0,"near-clipped sphere lost its visible rim");
    // Independent dynamic uniform offsets for commands sharing one geometry,
    // changing styles/counts/extents without replacing immutable point data.
    d.model_view=mat4::identity(); d.projection=mat4::identity();
    v.position=vec3(0);
    d.geometry=std::make_shared<Geometry>(std::vector<Vertex>{v},std::vector<uint32_t>{},Topology::points);
    DrawCommand other=d;
    d.sphere_radius=0.2f; d.model_view(0,3)=-0.5f;
    other.model_view(0,3)=0.5f; other.sphere_radius=0.3f; other.object_id=49;
    auto pair=device.render(64,64,{d,other});
    require(pair.ids[32*64+16].object==33 && pair.ids[32*64+48].object==49,
            "point commands shared uniform state");
    other.point_style=PointStyle::plain; other.point_size=7;
    other.model_view_projection(0,3)=0.5f;
    pair=device.render(80,48,{other,d});
    require(pair.ids[24*80+20].object==33 && pair.ids[24*80+60].object==49,
            "point style/resize changed dynamic draw state");
    d.point_size=0; rejects([&]{device.render(32,32,{d});});
    d.point_size=1; d.projection=mat4(0.0f); rejects([&]{device.render(32,32,{d});});
}
int main(int argc, char**) {
    try {
        DeviceOptions options;
        options.validation = argc > 1;
        options.synchronization_validation = options.validation;
        auto device = create_vulkan_device(options);
        std::cout << "Vulkan device: " << device->name() << std::endl;
        rejects([] { Geometry({}, {0}); });
        rejects([] { Image(0, 1, {}); });
        rejects([] { Geometry(std::vector<Vertex>(3), {}, Topology::triangles, {UINT32_MAX}); });
        rejects([&] { device->render(0, 10, {}); });
        DrawCommand far, near;
        far.geometry = triangle(0.5f, vec4(0, 1, 0, 1)); far.object_id = 9;
        near.geometry = triangle(-0.5f, vec4(1, 0, 0, 1)); near.object_id = 21;
        const auto image = device->render(64, 64, {near, far}, vec4(0, 0, 1, 1));
        const auto center = 32 * 64 + 32;
        require(image.rgba[center * 4] == 255 && image.rgba[center * 4 + 1] == 0, "depth occlusion/color mismatch");
        require(std::abs(image.depth[center] - 0.25f) < 0.00001f, "camera depth conversion mismatch");
        require(image.ids[center].object == 21 && image.ids[center].primitive == 173, "integer picking ID mismatch");
        require(image.ids[0].object == UINT32_MAX && image.ids[0].primitive == UINT32_MAX, "background picking ID mismatch");
        require(image.depth[0] == 1 && image.rgba[2] == 255, "clear/readback mismatch");
        // Y-up input must appear toward the top of the returned image.
        require(image.ids[12 * 64 + 32].object == 21, "image orientation flipped");
        require(image.ids[52 * 64 + 10].object == 21, "triangle base orientation mismatch");
        std::cout << "Supported color sample counts:";
        for (unsigned count = 1; count <= 64; count <<= 1)
            if (device->supported_sample_counts() & count) std::cout << ' ' << count;
        std::cout << '\n';
        rejects([&] { device->render(64, 64, {near}, vec4(0), 0); });
        rejects([&] { device->render(64, 64, {near}, vec4(0), 3); });
        if (device->supported_sample_counts() & 4) {
            auto aa = device->render(64, 64, {near, far}, vec4(0, 0, 1, 1), 4);
            require(aa.ids[center].object == 21 && aa.ids[center].primitive == 173, "MSAA changed center picking IDs");
            unsigned edge_pixels = 0;
            for (size_t i = 0; i < aa.ids.size(); ++i) {
                const auto red = aa.rgba[i * 4], blue = aa.rgba[i * 4 + 2];
                edge_pixels += red > 0 && red < 255 && blue > 0 && blue < 255;
                require(aa.ids[i].object == image.ids[i].object && aa.ids[i].primitive == image.ids[i].primitive,
                        "MSAA changed center-sample primitive IDs");
                require(std::abs(aa.depth[i] - image.depth[i]) < 1e-5f, "MSAA changed center-sample depth");
            }
            require(edge_pixels > 0, "four samples produced no antialiased edges");
            // Exercise pipeline/target caching while switching sample count and extent.
            for (unsigned i = 0; i < 8; ++i) {
                const unsigned w = 63 + i, h = 49 + i;
                const auto frame = device->render(w, h, {near}, vec4(0), i % 2 ? 1 : 4);
                require(frame.ids[(h / 2) * w + w / 2].object == 21, "sample-count/resize transition lost geometry");
            }
        }
        point_styles(*device);
        // Valid descriptor for a real texture, then a null texture on the same draw.
        near.texture = std::make_shared<Image>(1, 1, std::vector<unsigned char>{0, 255, 0, 255});
        near.color = vec4(1, 1, 1, 1);
        near.geometry = triangle(-0.5f, vec4(1, 1, 1, 1));
        auto textured = device->render(64, 64, {near});
        require(textured.rgba[center * 4 + 1] == 255 && textured.rgba[center * 4] == 0, "texture upload/binding mismatch");
        near.texture.reset();
        textured = device->render(64, 64, {near});
        require(textured.rgba[center * 4] == 255 && textured.rgba[center * 4 + 1] == 255, "white fallback descriptor mismatch");
        near.lighting = true;
        near.normal_matrix(0, 0) = 0; near.normal_matrix(0, 2) = 1;
        near.normal_matrix(2, 0) = -1; near.normal_matrix(2, 2) = 0;
        auto lit = device->render(64, 64, {near});
        const float diffuse = 0.2f + 0.8f * 0.3f / std::sqrt(1.34f);
        require(std::abs(float(lit.rgba[center * 4]) - diffuse * 255) < 2, "normal transform/lighting layout mismatch");
        near.lighting = false; near.normal_matrix = mat3::identity();
        // Immutable geometry can be realized on two independently owned devices.
        auto second = create_vulkan_device(options);
        require(second->render(64, 64, {near}).ids[center].object == 21, "cross-device snapshot reuse failed");
        for (unsigned i = 0; i < 20; ++i) {
            near.geometry = triangle(0, vec4(float(i % 2), 0, 1, 1));
            auto resized = device->render(31 + i, 23 + i, {near});
            require(resized.rgba.size() == size_t(31 + i) * (23 + i) * 4, "resize storage mismatch");
            require(resized.ids[(resized.height / 2) * resized.width + resized.width / 2].object == 21, "updated geometry missing");
        }
        std::vector<Vertex> points(1);
        points[0].position = vec3(0, 0, 0);
        near.geometry = std::make_shared<Geometry>(points, std::vector<uint32_t>{}, Topology::points);
        auto primitives = device->render(32, 32, {near});
        bool point = false;
        for (auto id : primitives.ids) point |= id.object == 21;
        require(point, "point pipeline did not draw");
        if (device->supported_sample_counts() & 4) {
            const auto aa = device->render(32, 32, {near}, vec4(1), 4);
            for (size_t i = 0; i < aa.ids.size(); ++i)
                require(aa.ids[i].object == primitives.ids[i].object && std::abs(aa.depth[i] - primitives.depth[i]) < 1e-5f,
                        "point picking changed with MSAA");
        }
        points.push_back(points[0]);
        points[0].position.x = -0.8f; points[1].position.x = 0.8f;
        near.geometry = std::make_shared<Geometry>(points, std::vector<uint32_t>{}, Topology::lines);
        primitives = device->render(32, 32, {near});
        unsigned line = 0;
        for (auto id : primitives.ids) line += id.object == 21;
        require(line >= 20, "line pipeline did not draw");
        if (device->supported_sample_counts() & 4) {
            const auto aa = device->render(32, 32, {near}, vec4(1), 4);
            for (size_t i = 0; i < aa.ids.size(); ++i)
                require(aa.ids[i].object == primitives.ids[i].object && std::abs(aa.depth[i] - primitives.depth[i]) < 1e-5f,
                        "line picking changed with MSAA");
        }
        auto empty = device->render(19, 17, {});
        for (auto id : empty.ids) require(id.object == UINT32_MAX, "stale target contents after clear");
        require(device->validation_errors() == 0 && second->validation_errors() == 0, "Vulkan validation reported errors");
        std::cout << "PASS: color, indexed geometry, depth, IDs, orientation, texture, updates, resize, independent devices, large points, sphere impostors, MSAA, clipping, lines, empty scene\n";
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << std::endl;
        return 1;
    }
}
