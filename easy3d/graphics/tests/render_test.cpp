// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/graphics/render_device.h>
#include <cmath>
#include <iostream>
#include <limits>
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
void surfels(RenderDevice& device) {
    Vertex vertex;
    vertex.position = vec3(0);
    vertex.normal = vec3(1, 0, 1);
    DrawCommand disc;
    disc.point_style = PointStyle::surfel;
    disc.sphere_radius = 0.6f;
    disc.object_id = 64;
    disc.color = vec4(1, 0, 0, 1);
    auto geometry = [&] {
        disc.geometry = std::make_shared<Geometry>(std::vector<Vertex>{vertex},
                std::vector<uint32_t>{0}, Topology::points, std::vector<uint32_t>{123});
    };
    geometry();
    disc.model_view(2,3) = 0.2f;
    auto image = device.render(65, 65, {disc}, vec4(0));
    require(image.ids[32*65+32].primitive == 123, "surfel lost explicit primitive ID");
    require(std::abs(image.depth[32*65+32] - 0.6f) < 1e-5f, "surfel center plane depth mismatch");
    require(std::abs(image.depth[32*65+40] - (0.6f-8.0f/65)) < 1e-5f, "tilted surfel depth is flat");
    DrawCommand plane;
    plane.geometry = triangle(0.1f, vec4(0, 1, 0, 1)); plane.object_id = 19;
    for (bool reverse : {false, true}) {
        auto overlap = device.render(65, 65, reverse ? std::vector<DrawCommand>{plane, disc} :
                                                       std::vector<DrawCommand>{disc, plane});
        require(overlap.ids[32*65+32].object == 19 && overlap.ids[32*65+40].object == 64,
                "surfel/mesh depth ordering mismatch");
    }
    // Off-axis perspective, with a nonuniform model scale. Inverse-transpose
    // transforms (4,0,1) to (2,0,2), so the expected plane normal is (1,0,1).
    vertex.normal = vec3(4, 0, 1); geometry();
    disc.model_view(0,0) = 2; disc.model_view(2,2) = 0.5f;
    disc.model_view(0,3) = 0.45f; disc.model_view(1,3) = -0.15f; disc.model_view(2,3) = -3;
    disc.sphere_radius = 0.8f;
    disc.projection = mat4(0.0f);
    disc.projection(0,0) = 1.3f; disc.projection(1,1) = 1.6f;
    disc.projection(2,2) = -11.0f/9; disc.projection(2,3) = -20.0f/9; disc.projection(3,2) = -1;
    image = device.render(93, 71, {disc}, vec4(0));
    unsigned hits = 0;
    const vec3 center(0.45f, -0.15f, -3), normal = normalize(vec3(1, 0, 1));
    for (unsigned y = 0; y < 71; ++y) for (unsigned x = 0; x < 93; ++x) {
        const vec3 ray = normalize(vec3((2*(x+0.5f)/93-1)/1.3f, (1-2*(y+0.5f)/71)/1.6f, -1));
        const float t = dot(center, normal)/dot(ray, normal);
        const vec3 hit = t*ray, delta = hit-center;
        const float radial = dot(delta, delta);
        const size_t pixel = y*93+x;
        if (t > 0 && radial < 0.64f-1e-5f) {
            ++hits;
            const vec4 clip = disc.projection*vec4(hit, 1);
            require(image.ids[pixel].object == 64 && image.ids[pixel].primitive == 123,
                    "perspective surfel silhouette/normal transform mismatch");
            require(std::abs(image.depth[pixel] - (clip.z/clip.w+1)*0.5f) < 2e-5f,
                    "perspective surfel ray depth mismatch");
        } else if (t <= 0 || radial > 0.64f+1e-5f)
            require(image.ids[pixel].object == UINT32_MAX, "surfel proxy leaked outside disc");
    }
    require(hits > 100, "surfel oracle did not cover enough pixels");
    if (device.supported_sample_counts(PointStyle::surfel) & 4) {
        auto aa = device.render(93, 71, {disc}, vec4(0), 4);
        unsigned edges = 0;
        for (size_t i = 0; i < image.ids.size(); ++i) {
            require(aa.ids[i].object == image.ids[i].object && aa.ids[i].primitive == image.ids[i].primitive &&
                    std::abs(aa.depth[i]-image.depth[i]) < 1e-5f, "surfel MSAA changed center picking/depth");
            edges += aa.rgba[4*i] > 0 && aa.rgba[4*i] < 255;
        }
        require(edges > 0, "surfel has no antialiased silhouette");
    }
    vertex.normal = -vertex.normal; geometry();
    const auto back = device.render(93, 71, {disc});
    for (size_t i = 0; i < image.ids.size(); ++i)
        require(back.ids[i].object == image.ids[i].object && std::abs(back.depth[i]-image.depth[i]) < 1e-5f,
                "surfel back face changed silhouette/depth");
    disc.model_view(2,3) = 3; // disc center on/behind the eye plane
    auto hidden = device.render(65, 65, {disc});
    for (auto id : hidden.ids) require(id.object == UINT32_MAX, "surfel behind eye was visible");
    // Near/far clipping, including a partially clipped tilted disc.
    disc.model_view = mat4::identity(); disc.projection = mat4::identity();
    vertex.normal = vec3(1, 0, 1); geometry(); disc.sphere_radius = 0.5f;
    disc.model_view(2,3) = -0.9f;
    auto clipped = device.render(65, 65, {disc});
    require(clipped.ids[32*65+25].object == 64 && clipped.ids[32*65+39].object == UINT32_MAX,
            "surfel near clipping discarded the visible portion or leaked depth");
    disc.model_view(2,3) = 0.9f;
    clipped = device.render(65, 65, {disc});
    require(clipped.ids[32*65+39].object == 64 && clipped.ids[32*65+25].object == UINT32_MAX,
            "surfel far clipping mismatch");
    disc.model_view = mat4::identity();
    vertex.normal = vec3(1, 0, 0); geometry();
    clipped = device.render(65, 65, {disc});
    for (auto id : clipped.ids) require(id.object == UINT32_MAX, "edge-on surfel has visible area");
    vertex.normal = vec3(0); geometry();
    rejects([&] { device.render(32, 32, {disc}); });
    vertex.normal = vec3(0, 0, 1); geometry(); disc.model_view(0,0) = 0;
    rejects([&] { device.render(32, 32, {disc}); });
    disc.model_view = mat4::identity(); disc.model_view(3,0) = 1;
    rejects([&] { device.render(32, 32, {disc}); });
    disc.model_view = mat4::identity(); disc.projection = mat4(0.0f);
    rejects([&] { device.render(32, 32, {disc}); });
}
void line_impostors(RenderDevice& device) {
    DrawCommand line;
    line.object_id = 41; line.line_radius = 0.3f;
    line.color = vec4(1, 0, 0, 1);
    std::vector<Vertex> vertices(2);
    vertices[0].position = vec3(-0.6f, 0, 0);
    vertices[1].position = vec3(0.6f, 0, 0);
    auto geometry = [&](std::vector<uint32_t> indices = std::vector<uint32_t>{0, 1}) {
        line.geometry = std::make_shared<Geometry>(vertices, indices, Topology::lines, std::vector<uint32_t>{201});
    };
    geometry();
    for (auto style : {LineStyle::cylinder, LineStyle::cone}) {
        line.line_style = style;
        const auto single = device.render(65, 65, {line}, vec4(0));
        const float expected = style == LineStyle::cylinder ? 0.35f : 0.425f;
        require(single.ids[32*65+32].object == 41 && single.ids[32*65+32].primitive == 201,
                "line impostor lost indexed primitive ID");
        require(std::abs(single.depth[32*65+32]-expected) < 1e-5f, "line impostor side depth mismatch");
        require(single.ids[20*65+32].object == UINT32_MAX, "line impostor proxy leaked");
        DrawCommand plane;
        plane.geometry = triangle(style == LineStyle::cylinder ? -0.2f : -0.1f, vec4(0, 1, 0, 1));
        plane.object_id = 9;
        for (bool reverse : {false, true}) {
            const auto overlap = device.render(65, 65, reverse ? std::vector<DrawCommand>{plane, line} :
                                                               std::vector<DrawCommand>{line, plane});
            const unsigned rim = style == LineStyle::cylinder ? 40 : 36;
            require(overlap.ids[32*65+32].object == 41 && overlap.ids[rim*65+32].object == 9,
                    "line/mesh surface occlusion depends on proxy or draw order");
        }
        if (device.supported_sample_counts(style) & 4) {
            const auto aa = device.render(65, 65, {line}, vec4(0), 4);
            unsigned edges = 0;
            for (size_t pixel = 0; pixel < aa.ids.size(); ++pixel) {
                require(aa.ids[pixel].object == single.ids[pixel].object &&
                        aa.ids[pixel].primitive == single.ids[pixel].primitive &&
                        std::abs(aa.depth[pixel]-single.depth[pixel]) < 1e-5f,
                        "line MSAA changed center-sample picking/depth");
                edges += aa.rgba[4*pixel] > 0 && aa.rgba[4*pixel] < 255;
            }
            require(edges > 0, "line impostor MSAA has no partial coverage");
        }
    }
    // Clipping follows the actual front surface, preserving only the visible
    // portion of a solid, without substituting its back surface or adding caps.
    for (auto style : {LineStyle::cylinder, LineStyle::cone}) {
        line.line_style = style;
        const float center_z = style == LineStyle::cylinder ? -0.8f : -0.9f;
        vertices[0].position.z = vertices[1].position.z = center_z; geometry();
        auto clipped = device.render(65, 65, {line});
        const unsigned rim = style == LineStyle::cylinder ? 40 : 36;
        require(clipped.ids[32*65+32].object == UINT32_MAX && clipped.ids[rim*65+32].object == 41,
                "line near clipping leaked a back surface or discarded the visible rim");
        vertices[0].position.z = vertices[1].position.z = style == LineStyle::cylinder ? 1.2f : 1.1f; geometry();
        clipped = device.render(65, 65, {line});
        require(clipped.ids[32*65+32].object == 41 && clipped.ids[rim*65+32].object == UINT32_MAX,
                "line far clipping mismatch");
    }
    // Axis-parallel rays hit flat caps, including the cone's base.
    vertices[0].position = vec3(0, 0, -0.5f); vertices[1].position = vec3(0, 0, 0.5f); geometry();
    for (auto style : {LineStyle::cylinder, LineStyle::cone}) {
        line.line_style = style;
        auto caps = device.render(65, 65, {line});
        require(std::abs(caps.depth[32*65+32]-0.25f) < 1e-5f, "axial ray missed base cap");
        geometry({1, 0});
        caps = device.render(65, 65, {line});
        require(std::abs(caps.depth[32*65+32]-0.25f) < 1e-5f, "reversed line missed end cap/cone tip");
        if (style == LineStyle::cone)
            require(caps.depth[32*65+39] > 0.5f, "cone did not taper toward its second endpoint");
        geometry();
    }
    // Endpoint colors interpolate along the surface axis, independent of proxy triangles.
    vertices[0].position = vec3(-0.6f, 0, 0); vertices[1].position = vec3(0.6f, 0, 0);
    vertices[0].color = vec4(1, 0, 0, 1); vertices[1].color = vec4(0, 0, 1, 1); geometry();
    line.line_style = LineStyle::cylinder; line.color = vec4(1);
    auto colored = device.render(65, 65, {line});
    require(std::abs(int(colored.rgba[(32*65+32)*4])-128) <= 1 &&
            std::abs(int(colored.rgba[(32*65+32)*4+2])-128) <= 1, "line endpoint color interpolation mismatch");
    line.lighting = true; line.sphere_light = vec3(0, 0, 1);
    auto lit = device.render(65, 65, {line});
    require(lit.rgba[(40*65+32)*4] < colored.rgba[(40*65+32)*4], "cylinder lighting ignores surface normal");
    line.lighting = false;
    for (auto& v : vertices) v.color = vec4(1);
    vertices[0].texcoord = vec2(0, 0); vertices[1].texcoord = vec2(1, 0); geometry();
    line.texture = std::make_shared<Image>(2, 1, std::vector<unsigned char>{255,0,0,255, 0,255,0,255});
    auto textured = device.render(65, 65, {line});
    require(textured.rgba[(32*65+20)*4] == 255 && textured.rgba[(32*65+44)*4+1] == 255,
            "line texture/axial coordinate mismatch");
    line.texture.reset(); line.color = vec4(1, 0, 0, 1);
    // Different styles sharing a geometry must retain independent dynamic uniforms.
    line.model_view(1,3) = -0.45f; line.line_radius = 0.12f;
    DrawCommand other = line;
    other.model_view(1,3) = 0.45f; other.line_style = LineStyle::cone;
    other.line_radius = 0.2f; other.object_id = 77;
    Vertex point; point.position = vec3(0);
    DrawCommand sphere;
    sphere.geometry = std::make_shared<Geometry>(std::vector<Vertex>{point}, std::vector<uint32_t>{}, Topology::points);
    sphere.point_style = PointStyle::sphere; sphere.sphere_radius = 0.12f; sphere.object_id = 78;
    const auto mixed = device.render(80, 64, {line, sphere, other});
    require(mixed.ids[46*80+40].object == 41 && mixed.ids[18*80+40].object == 77 && mixed.ids[32*80+40].object == 78,
            "mixed point/line impostors shared draw uniforms or geometry bindings");
    line.model_view = mat4::identity(); line.line_radius = 0.3f;
    // Perspective oracle: an axis-aligned finite solid, solved in double precision
    // directly in camera coordinates, independently of the shader's local basis.
    vertices[0].position = vec3(-0.6f, 0.05f, -3); vertices[1].position = vec3(0.6f, 0.05f, -3); geometry();
    line.projection = mat4(0.0f);
    line.projection(0,0) = 1.3f; line.projection(1,1) = 1.6f;
    line.projection(2,2) = -11.0f/9; line.projection(2,3) = -20.0f/9; line.projection(3,2) = -1;
    for (auto style : {LineStyle::cylinder, LineStyle::cone}) {
        line.line_style = style;
        const auto image = device.render(93, 71, {line}, vec4(0));
        unsigned hits = 0;
        for (unsigned y = 0; y < 71; ++y) for (unsigned x = 0; x < 93; ++x) {
            const double rx = (2*(x+0.5)/93-1)/1.3, ry = (1-2*(y+0.5)/71)/1.6;
            const bool cone = style == LineStyle::cone;
            const double a = ry*ry+1-(cone ? rx*rx/16 : 0);
            const double b = -0.1*ry-6+(cone ? 0.075*rx : 0);
            const double c = 9.0025-(cone ? 0.0225 : 0.09);
            const double discriminant = b*b-4*a*c;
            double nearest = 1e30;
            auto candidate = [&](double t) {
                const double px = rx*t;
                if (px >= -0.6 && px <= 0.6) nearest = std::min(nearest, t);
            };
            if (discriminant > 0) {
                candidate((-b-std::sqrt(discriminant))/(2*a));
                candidate((-b+std::sqrt(discriminant))/(2*a));
            }
            if (std::abs(rx) > 1e-12) for (double cap : {-0.6, 0.6}) {
                if (cone && cap > 0) continue;
                const double t = cap/rx, py = ry*t-0.05, pz = 3-t;
                if (py*py+pz*pz <= 0.09) nearest = std::min(nearest, t);
            }
            const size_t pixel = y*93+x;
            if (nearest > 1 && nearest < 10) {
                ++hits;
                require(image.ids[pixel].object == 41 && image.ids[pixel].primitive == 201,
                        "perspective line silhouette/ID mismatch");
                const double depth = 0.5*((11.0*nearest/9-20.0/9)/nearest+1);
                require(std::abs(image.depth[pixel]-depth) < 2e-5, "perspective line oracle depth mismatch");
            } else require(image.ids[pixel].object == UINT32_MAX, "perspective line proxy leaked");
        }
        require(hits > 50, "line perspective oracle did not cover enough pixels");
        line.model_view(2,3) = 4;
        auto behind = device.render(65, 65, {line});
        for (auto id : behind.ids) require(id.object == UINT32_MAX, "line behind eye was visible");
        line.model_view(2,3) = -9;
        auto distant = device.render(65, 65, {line});
        for (auto id : distant.ids) require(id.object == UINT32_MAX, "line beyond far plane was visible");
        line.model_view = mat4::identity();
    }
    line.projection = mat4::identity(); line.line_radius = 0.15f;
    // Multiple indexed segments exercise endpoint-instance strides and original IDs.
    vertices.resize(4);
    vertices[0].position = vec3(-0.6f, -0.4f, 0); vertices[1].position = vec3(0.6f, -0.4f, 0);
    vertices[2].position = vec3(-0.6f, 0.4f, 0); vertices[3].position = vec3(0.6f, 0.4f, 0);
    line.geometry = std::make_shared<Geometry>(vertices, std::vector<uint32_t>{2,3,0,1}, Topology::lines,
                                               std::vector<uint32_t>{501,502});
    auto indexed = device.render(65, 65, {line});
    require(indexed.ids[19*65+32].primitive == 501 && indexed.ids[45*65+32].primitive == 502,
            "line endpoint instance packing changed primitive IDs");
    vertices.resize(2); vertices[0].position = vertices[1].position; geometry();
    auto degenerate = device.render(65, 65, {line});
    for (auto id : degenerate.ids) require(id.object == UINT32_MAX, "collapsed line impostor was visible");
    line.line_radius = 0; rejects([&] { device.render(32, 32, {line}); });
    line.line_radius = 1; line.projection = mat4(0.0f); rejects([&] { device.render(32, 32, {line}); });
    line.projection = mat4::identity(); line.model_view(3,0) = 1;
    rejects([&] { device.render(32, 32, {line}); });
}
void wide_lines(RenderDevice& device) {
    std::vector<Vertex> vertices(2);
    vertices[0].position = vec3(-0.75f, 0, 0); vertices[1].position = vec3(0.75f, 0, 0);
    DrawCommand line;
    line.line_width = 12; line.object_id = 91; line.color = vec4(1, 0, 0, 1);
    auto geometry = [&](bool reverse = false) {
        line.geometry = std::make_shared<Geometry>(vertices, reverse ? std::vector<uint32_t>{1,0} :
                std::vector<uint32_t>{0,1}, Topology::lines, std::vector<uint32_t>{321});
    };
    auto count = [](const FrameImage& frame) {
        unsigned hits = 0;
        for (auto id : frame.ids) hits += id.object == 91;
        return hits;
    };
    auto msaa = [&](unsigned width, unsigned height, const FrameImage& single) {
        if (!(device.supported_sample_counts(LineStyle::plain) & 4)) return;
        const auto aa = device.render(width, height, {line}, vec4(0), 4);
        unsigned edges = 0;
        for (size_t i = 0; i < aa.ids.size(); ++i) {
            require(aa.ids[i].object == single.ids[i].object && aa.ids[i].primitive == single.ids[i].primitive &&
                    std::abs(aa.depth[i]-single.depth[i]) < 1e-5f, "wide line MSAA changed center picking/depth");
            edges += aa.rgba[4*i] > 0 && aa.rgba[4*i] < 255;
        }
        require(edges > 0, "wide line MSAA produced no partial edge coverage");
    };
    geometry();
    auto horizontal = device.render(64, 48, {line});
    require(count(horizontal) == 48*12, "wide horizontal line has incorrect pixel width or butt caps");
    line.line_width = 512;
    require(count(device.render(64,48,{line})) == 48*48, "portable line width was clamped to a native-device limit");
    line.line_width = 0.25f;
    require(count(device.render(65,65,{line})) > 0, "subpixel wide line was rejected or clamped away");
    vertices[0].position.y = vertices[1].position.y = 1.05f; geometry(); line.line_width = 12;
    auto outside = device.render(64,64,{line});
    require(outside.ids[1*64+32].object == 91 && outside.ids[8*64+32].object == UINT32_MAX,
            "wide line was clipped by its offscreen centerline instead of its ribbon");
    vertices[0].position.y = vertices[1].position.y = 0; geometry();
    line.line_width = 12.5f;
    msaa(64, 48, device.render(64, 48, {line}, vec4(0)));
    line.line_width = 12;
    vertices[0].position = vec3(0, -0.75f, 0); vertices[1].position = vec3(0, 0.75f, 0); geometry();
    require(count(device.render(64, 48, {line})) == 36*12, "wide vertical line has incorrect pixel width");
    // Independent screen-space rectangle/depth oracle for a diagonal ribbon.
    vertices[0].position = vec3(-0.65f, -0.55f, -0.6f);
    vertices[1].position = vec3(0.65f, 0.55f, 0.6f); geometry(); line.line_width = 7.5f;
    auto diagonal = device.render(93, 71, {line}, vec4(0));
    const double ax = 0.175*93, ay = 0.775*71, dx = 0.65*93, dy = -0.55*71;
    const double length_squared = dx*dx+dy*dy;
    unsigned expected_hits = 0;
    for (unsigned y = 0; y < 71; ++y) for (unsigned x = 0; x < 93; ++x) {
        const double px = x+0.5-ax, py = y+0.5-ay;
        const double along = (px*dx+py*dy)/length_squared;
        const double across = std::abs(px*dy-py*dx)/std::sqrt(length_squared);
        const size_t pixel = y*93+x;
        if (along > 1e-5 && along < 1-1e-5 && across < 3.75-1e-5) {
            ++expected_hits;
            require(diagonal.ids[pixel].object == 91 && diagonal.ids[pixel].primitive == 321,
                    "wide line screen-space silhouette/ID mismatch");
            // Rasterizer subpixel quantization slightly perturbs the triangle's
            // interpolation plane; center picking must still match across MSAA.
            require(std::abs(diagonal.depth[pixel]-(0.2+0.6*along)) < 5e-5,
                    "wide line interpolated depth mismatch");
        } else if (along < -1e-5 || along > 1+1e-5 || across > 3.75+1e-5)
            require(diagonal.ids[pixel].object == UINT32_MAX, "wide line escaped its butt-capped rectangle");
    }
    require(expected_hits > 100, "wide-line oracle did not exercise enough pixels");
    msaa(93, 71, diagonal);
    geometry(true);
    auto reversed = device.render(93, 71, {line});
    for (size_t i = 0; i < diagonal.ids.size(); ++i)
        require(reversed.ids[i].object == diagonal.ids[i].object && std::abs(reversed.depth[i]-diagonal.depth[i]) < 5e-5f,
                "reversing a wide segment changed coverage/depth");
    vertices[0].position = vec3(-0.75f, 0, -0.6f); vertices[1].position = vec3(0.75f, 0, 0.6f); geometry();
    DrawCommand plane; plane.geometry = triangle(0, vec4(0,1,0,1)); plane.object_id = 19;
    for (bool reverse : {false, true}) {
        const auto overlap = device.render(65, 65, reverse ? std::vector<DrawCommand>{plane,line} :
                                                           std::vector<DrawCommand>{line,plane});
        require(overlap.ids[32*65+24].object == 91 && overlap.ids[32*65+40].object == 19,
                "wide line/mesh depth occlusion depends on draw order");
    }
    // Near/far clipping happens before width expansion, with no offscreen spikes.
    vertices[0].position.z = -2; vertices[1].position.z = 0; geometry();
    auto clipped = device.render(65, 65, {line});
    require(clipped.ids[32*65+24].object == UINT32_MAX && clipped.ids[32*65+40].object == 91,
            "wide line near clipping mismatch");
    vertices[0].color = vec4(1,0,0,1); vertices[1].color = vec4(0,1,0,1); geometry();
    line.color = vec4(1);
    const auto clipped_color = device.render(65,65,{line});
    require(std::abs(int(clipped_color.rgba[(32*65+40)*4])-86) <= 2 &&
            std::abs(int(clipped_color.rgba[(32*65+40)*4+1])-169) <= 2,
            "near clipping lost the original endpoint color parameters");
    vertices[0].position.z = 0; vertices[1].position.z = 2; geometry();
    clipped = device.render(65, 65, {line});
    require(clipped.ids[32*65+24].object == 91 && clipped.ids[32*65+40].object == UINT32_MAX,
            "wide line far clipping mismatch");
    // Perspective-correct color/UV interpolation with unequal endpoint depths.
    vertices[0].position = vec3(-0.8f, 0, -2); vertices[1].position = vec3(1.2f, 0, -6);
    vertices[0].color = vec4(1,0,0,1); vertices[1].color = vec4(0,1,0,1);
    vertices[0].texcoord = vec2(0,0); vertices[1].texcoord = vec2(1,0); geometry();
    line.model_view_projection = mat4(0.0f);
    line.model_view_projection(0,0) = 1.3f; line.model_view_projection(1,1) = 1.6f;
    line.model_view_projection(2,2) = -11.0f/9; line.model_view_projection(2,3) = -20.0f/9;
    line.model_view_projection(3,2) = -1;
    line.color = vec4(1);
    auto perspective = device.render(93, 71, {line}, vec4(0));
    for (unsigned x = 24; x < 58; ++x) {
        const double t = (x+0.5-0.24*93)/(0.39*93);
        const double fraction = (t/6)/((1-t)/2+t/6);
        const size_t pixel = 35*93+x;
        require(perspective.ids[pixel].object == 91, "perspective wide ribbon is missing");
        require(std::abs(perspective.rgba[4*pixel]-255*(1-fraction)) < 2 &&
                std::abs(perspective.rgba[4*pixel+1]-255*fraction) < 2,
                "wide-line endpoint colors are not perspective correct");
        require(std::abs(perspective.depth[pixel]-(5.0/9+10.0/27*t)) < 5e-5,
                "wide-line perspective depth mismatch");
    }
    // Use solid red for a strict silhouette-coverage MSAA test.
    vertices[0].color = vertices[1].color = vec4(1,0,0,1); geometry();
    msaa(93, 71, device.render(93, 71, {line}, vec4(0)));
    vertices[0].color = vertices[1].color = vec4(1); geometry();
    line.texture = std::make_shared<Image>(2,1,std::vector<unsigned char>{255,0,0,255, 0,255,0,255});
    auto textured = device.render(93, 71, {line});
    require(textured.rgba[(35*93+30)*4] == 255 && textured.rgba[(35*93+56)*4+1] == 255,
            "wide-line texture coordinate interpolation mismatch");
    line.texture.reset();
    line.lighting = true;
    line.normal_matrix(2,2) = -1;
    auto lit = device.render(93, 71, {line});
    require(lit.rgba[(35*93+30)*4] > 200 && lit.rgba[(35*93+30)*4] < 240,
            "wide-line lighting differs from native line lighting");
    line.lighting = false; line.color = vec4(1,0,0,1);
    vertices[0].position = vec3(-0.8f,0,1); vertices[1].position = vec3(0.8f,0,-3); geometry();
    auto eye_crossing = device.render(93,71,{line});
    require(count(eye_crossing) > 30 && eye_crossing.ids[35*93+30].object == UINT32_MAX &&
            eye_crossing.ids[35*93+55].object == 91, "wide line crossing the eye plane was not clipped safely");
    vertices[0].position.z = 2; vertices[1].position.z = 3; geometry();
    require(count(device.render(93,71,{line})) == 0, "wide line behind the eye was visible");
    vertices[0].position = vec3(0,0,-2); vertices[1].position = vec3(0,0,-6); geometry();
    require(count(device.render(93,71,{line})) == 0, "projectively collapsed wide segment has area");
    // Shared endpoint geometry with two widths, then a resize/sample-count switch.
    line.model_view_projection = mat4::identity(); line.line_width = 4;
    vertices[0].position = vec3(-0.75f,0,0); vertices[1].position = vec3(0.75f,0,0); geometry();
    DrawCommand other = line; other.line_width = 12; other.object_id = 92;
    other.model_view_projection(1,3) = 0.5f;
    auto pair = device.render(80,64,{line,other});
    require(pair.ids[32*80+40].object == 91 && pair.ids[16*80+40].object == 92 &&
            pair.ids[22*80+40].object == UINT32_MAX, "wide-line draw widths shared uniform state");
    line.line_width = 5.5f;
    msaa(97,53,device.render(97,53,{line},vec4(0)));
    line.line_width = 0; rejects([&] { device.render(32,32,{line}); });
    line.line_width = std::numeric_limits<float>::infinity(); rejects([&] { device.render(32,32,{line}); });
}
void texture_samplers(RenderDevice& device) {
    auto vertices = triangle(0,vec4(1))->vertices();
    DrawCommand draw; draw.object_id = 61;
    auto geometry = [&](float u) {
        for (auto& vertex : vertices) vertex.texcoord = vec2(u,0.5f);
        draw.geometry = std::make_shared<Geometry>(vertices);
    };
    const std::vector<unsigned char> pixels = {255,0,0,255, 0,255,0,255};
    Sampler sampler;
    geometry(1.25f);
    draw.texture = std::make_shared<Image>(2,1,pixels,sampler);
    auto clamp = device.render(32,32,{draw});
    const size_t center = 16*32+16;
    require(clamp.rgba[4*center] == 0 && clamp.rgba[4*center+1] == 255,"clamp-to-edge sampler mismatch");
    sampler.wrap = TextureWrap::repeat;
    draw.texture = std::make_shared<Image>(2,1,pixels,sampler);
    auto repeated = device.render(32,32,{draw});
    require(repeated.rgba[4*center] == 255 && repeated.rgba[4*center+1] == 0,"repeat sampler mismatch");
    geometry(0.5f); sampler.wrap = TextureWrap::clamp_to_edge; sampler.filter = TextureFilter::linear;
    draw.texture = std::make_shared<Image>(2,1,pixels,sampler);
    auto linear = device.render(32,32,{draw});
    require(std::abs(int(linear.rgba[4*center])-128) <= 1 && std::abs(int(linear.rgba[4*center+1])-128) <= 1,
            "linear sampler did not blend its neighboring texels");
    require(linear.ids[center].object == 61 && repeated.ids[center].object == clamp.ids[center].object &&
            linear.depth[center] == clamp.depth[center],"sampler selection changed picking/depth");
    sampler.wrap = static_cast<TextureWrap>(42);
    rejects([&] { Image invalid(2,1,pixels,sampler); });
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
        surfels(*device);
        line_impostors(*device);
        wide_lines(*device);
        texture_samplers(*device);
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
        std::cout << "PASS: color, indexed geometry, depth, IDs, orientation, texture, updates, resize, independent devices, large points, sphere/surfel impostors, MSAA, clipping, cylinders, cones, wide lines, empty scene\n";
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << std::endl;
        return 1;
    }
}
