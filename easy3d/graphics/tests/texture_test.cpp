// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/vulkan_offscreen.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>
using namespace easy3d;
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
template<class Function> void rejects(Function f) {
    bool threw = false;
    try { f(); } catch (const std::exception&) { threw = true; }
    require(threw,"invalid texture/scalar input was accepted");
}
size_t pixel(const VulkanOffScreen& renderer, const graphics::FrameImage& frame, float x, float y) {
    const auto p = renderer.camera()->projectedCoordinatesOf(vec3(x,y,0));
    require(p.x >= 0 && p.x < frame.width && p.y >= 0 && p.y < frame.height,"texture probe is offscreen");
    return size_t(p.y)*frame.width+size_t(p.x);
}
void probe_color(const VulkanOffScreen& renderer, const graphics::FrameImage& frame, float x, float y,
           unsigned r, unsigned g, unsigned b) {
    const auto i = pixel(renderer,frame,x,y);
    require(frame.ids[i].object != UINT32_MAX,"texture probe missed geometry");
    require(std::abs(int(frame.rgba[4*i])-int(r)) <= 2 && std::abs(int(frame.rgba[4*i+1])-int(g)) <= 2 &&
            std::abs(int(frame.rgba[4*i+2])-int(b)) <= 2,"texture/scalar probe color mismatch");
}
void tests(const graphics::DeviceOptions& options) {
    // Public texture data uses bottom-first rows, including file-decoded data.
    std::vector<unsigned char> rgba = {255,0,0,255, 0,255,0,255, 0,0,255,255, 255,255,0,255};
    std::unique_ptr<Texture> texture(Texture::create_cpu(rgba,2,2,4,Texture::CLAMP_TO_EDGE,Texture::NEAREST));
    rgba[0] = 0;
    require(texture->id() == 0 && texture->pixels()->bytes[0] == 255,"CPU texture called GL or borrowed mutable pixels");
    rejects([&] { texture->bind(); });
    rejects([] { std::unique_ptr<Texture> invalid(Texture::create_cpu({},-1,2,4)); });
    require(ImageIO::save("vulkan-texture-source.png",texture->pixels()->bytes,2,2,4,true),"could not write texture fixture");
    auto* cached = TextureManager::request_cpu("vulkan-texture-source.png",Texture::CLAMP_TO_EDGE,Texture::NEAREST);
    auto* repeat = TextureManager::request_cpu("vulkan-texture-source.png",Texture::REPEAT,Texture::NEAREST);
    auto* linear = TextureManager::request_cpu("vulkan-texture-source.png",Texture::CLAMP_TO_EDGE,Texture::LINEAR);
    require(cached && repeat && linear && cached != repeat && cached != linear,"sampler variants shared a texture cache entry");
    require(cached->pixels()->bytes == texture->pixels()->bytes,"file texture flipped rows or changed channels");
    require(cached == TextureManager::request_cpu("vulkan-texture-source.png",Texture::CLAMP_TO_EDGE,Texture::NEAREST),
            "CPU texture cache did not reuse a matching request");
    TextureManager::release(cached);
    std::vector<Texture*> concurrent(4,nullptr);
    std::vector<std::thread> workers;
    for (size_t i=0;i<concurrent.size();++i)
        workers.emplace_back([&,i] { concurrent[i]=TextureManager::request_cpu("vulkan-texture-source.png",Texture::CLAMP_TO_EDGE,Texture::NEAREST); });
    for (auto& worker:workers) worker.join();
    cached=concurrent[0];
    for (auto* result:concurrent) require(result && result==cached,"concurrent CPU cache requests created distinct textures");
    VulkanOffScreen renderer(129,97,options);
    renderer.camera()->setType(Camera::ORTHOGRAPHIC);
    renderer.camera()->setViewDirection(vec3(0,0,-1));
    renderer.camera()->setUpVector(vec3(0,1,0));
    auto quad = std::make_shared<TrianglesDrawable>();
    renderer.add_drawable(quad);
    quad->update_vertex_buffer({vec3(-1,-1,0),vec3(1,-1,0),vec3(1,1,0),vec3(-1,1,0)});
    quad->update_texcoord_buffer({vec2(0,0),vec2(1,0),vec2(1,1),vec2(0,1)});
    quad->update_element_buffer(std::vector<std::vector<unsigned>>{{0,1,2},{0,2,3}});
    quad->set_lighting(false); quad->set_opacity(1);
    quad->set_texture_coloring(State::VERTEX,"v:texcoord",texture.get());
    renderer.fit_screen();
    const auto image = renderer.capture();
    probe_color(renderer,image,-0.6f,-0.4f,255,0,0); probe_color(renderer,image,0.6f,-0.4f,0,255,0);
    probe_color(renderer,image,-0.6f,0.4f,0,0,255); probe_color(renderer,image,0.6f,0.4f,255,255,0);
    quad->set_texture(cached);
    require(renderer.capture().rgba == image.rgba,"file and raw texture orientation differ");
    quad->set_texture(repeat); quad->set_texture_repeat(2);
    auto repeated = renderer.capture();
    probe_color(renderer,repeated,-0.6f,-0.7f,255,0,0); probe_color(renderer,repeated,0.4f,-0.7f,255,0,0);
    quad->set_texture_repeat(1); quad->set_texture_fractional_repeat(100);
    require(renderer.capture().rgba == repeated.rgba,"fractional texture repeat changed its unit convention");
    quad->set_texture_fractional_repeat(0); quad->set_texture(linear);
    auto filtered = renderer.capture();
    const auto center = pixel(renderer,filtered,0,0);
    require(filtered.rgba[4*center] > 90 && filtered.rgba[4*center] < 170 &&
            filtered.rgba[4*center+1] > 90 && filtered.rgba[4*center+1] < 170,
            "linear filtering did not blend neighboring texels");
    quad->set_uniform_coloring(vec4(1,0,1,1));
    probe_color(renderer,renderer.capture(),0,0,255,0,255);
    quad->set_texture_coloring(State::VERTEX,"v:texcoord",cached);
    require(renderer.capture().rgba == image.rgba,"switching coloring modes retained stale texture state");
    if (renderer.device().supported_sample_counts() & 4) {
        const auto aa = renderer.capture(4);
        for (size_t i = 0; i < image.ids.size(); ++i)
            require(aa.ids[i].object == image.ids[i].object && aa.ids[i].primitive == image.ids[i].primitive &&
                    std::abs(aa.depth[i]-image.depth[i]) < 1e-6f,"texturing/MSAA changed picking IDs/depth");
    }
    // Existing default selection now recognizes texture-coordinate properties.
    renderer.clear_scene();
    auto mesh = std::make_shared<SurfaceMesh>();
    auto a=mesh->add_vertex(vec3(-1,-1,0)), b=mesh->add_vertex(vec3(1,-1,0));
    auto c=mesh->add_vertex(vec3(1,1,0)), d=mesh->add_vertex(vec3(-1,1,0));
    auto f0=mesh->add_triangle(a,b,c), f1=mesh->add_triangle(a,c,d);
    auto uv=mesh->add_vertex_property<vec2>("v:texcoord");
    uv[a]=vec2(0,0); uv[b]=vec2(1,0); uv[c]=vec2(1,1); uv[d]=vec2(0,1);
    renderer.add_model(mesh);
    auto* faces=mesh->renderer()->get_triangles_drawable("faces");
    require(faces->coloring_method() == State::TEXTURED && faces->texture() && faces->texture()->id() == 0,
            "CPU model defaults did not select a texture without GL calls");
    faces->set_texture(cached); faces->set_texture_repeat(1); faces->set_lighting(false);
    renderer.fit_screen();
    probe_color(renderer,renderer.capture(),-0.6f,0.4f,0,0,255);
    auto halfedge_uv=mesh->add_halfedge_property<vec2>("h:texcoord");
    for (auto h:mesh->halfedges())
        halfedge_uv[h]=mesh->face(h)==f1 ? vec2(0.1f,0.1f) : uv[mesh->target(h)];
    faces->set_texture_coloring(State::HALFEDGE,"h:texcoord",cached); mesh->renderer()->update();
    const auto seam=renderer.capture();
    probe_color(renderer,seam,-0.6f,0.4f,255,0,0); probe_color(renderer,seam,0.6f,-0.4f,0,255,0);
    // Face fields and constant scalar fields use an explicit two-color palette.
    std::unique_ptr<Texture> palette(Texture::create_cpu({0,0,255,255, 255,0,0,255},2,1,4,Texture::CLAMP_TO_EDGE,Texture::LINEAR));
    auto scalar=mesh->add_face_property<float>("f:scalar"); scalar[f0]=0; scalar[f1]=1;
    faces->set_scalar_coloring(State::FACE,"f:scalar",palette.get(),0,0); mesh->renderer()->update();
    const auto field=renderer.capture();
    probe_color(renderer,field,0.6f,-0.4f,0,0,255); probe_color(renderer,field,-0.6f,0.4f,255,0,0);
    scalar[f0]=scalar[f1]=5; mesh->renderer()->update();
    const auto constant=renderer.capture();
    probe_color(renderer,constant,0.6f,-0.4f,128,0,128); probe_color(renderer,constant,-0.6f,0.4f,128,0,128);
    for (size_t i=0;i<field.ids.size();++i)
        require(field.ids[i].object==constant.ids[i].object && field.ids[i].primitive==constant.ids[i].primitive &&
                std::abs(field.depth[i]-constant.depth[i])<1e-6f,"scalar updates changed picking/depth");
    auto vertex_scalar=mesh->add_vertex_property<float>("v:scalar");
    for (auto v:mesh->vertices()) vertex_scalar[v]=mesh->position(v).x;
    faces->set_scalar_coloring(State::VERTEX,"v:scalar",palette.get(),0,0); mesh->renderer()->update();
    auto varying=renderer.capture();
    probe_color(renderer,varying,-0.7f,0.2f,0,0,255); probe_color(renderer,varying,0.7f,0.2f,255,0,0);
    faces->set_scalar_coloring(State::VERTEX,"v:scalar",palette.get(),-0.1f,0); mesh->renderer()->update();
    rejects([&] { renderer.capture(); });
    faces->set_scalar_coloring(State::VERTEX,"v:scalar",palette.get(),0,0);
    vertex_scalar[a]=std::numeric_limits<float>::quiet_NaN(); mesh->renderer()->update();
    rejects([&] { renderer.capture(); });
    vertex_scalar[a]=-1; mesh->renderer()->update(); renderer.capture();
    // Default colormap creation works without a GL context as in Tutorial 303.
    faces->set_scalar_coloring(State::VERTEX,"v:scalar",nullptr,0,0); mesh->renderer()->update();
    require(faces->texture() && faces->texture()->id()==0,"implicit scalar palette called OpenGL");
    renderer.capture();
    // CPU image caches may outlive the GPU binding. Replacing one drawable's
    // texture must retire old GPU descriptors even while all CPU snapshots live.
    renderer.clear_scene(); renderer.resize(16,16); renderer.add_drawable(quad); renderer.fit_screen();
    std::vector<std::unique_ptr<Texture>> snapshots;
    for (unsigned i=0;i<1030;++i) {
        std::vector<unsigned char> stripe(4,0); stripe[i%2]=stripe[3]=255;
        snapshots.emplace_back(Texture::create_cpu(stripe,1,1,4));
        quad->set_texture(snapshots.back().get());
        const auto frame=renderer.capture();
        require(frame.ids[8*16+8].object != UINT32_MAX && frame.rgba[(8*16+8)*4+i%2]==255,
                "replacing CPU texture snapshots exhausted descriptors or retained stale data");
    }
    quad->set_texture(cached);
    TextureManager::release(linear); TextureManager::release(repeat);
    require(renderer.device().validation_errors()==0,"texture/scalar rendering reported validation errors");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        graphics::DeviceOptions options;
        options.validation=options.synchronization_validation=argc>1;
        tests(options);
        TextureManager::terminate();
        std::cout << "PASS: CPU textures, orientation, file/cache/samplers, repeat/filter, scalar fields, MSAA and picking\n";
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        TextureManager::terminate();
        return 1;
    }
}
