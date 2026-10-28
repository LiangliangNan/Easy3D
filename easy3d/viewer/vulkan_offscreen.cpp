// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/vulkan_offscreen.h>
#include <easy3d/core/model.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/poly_mesh_io.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/setting.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <stdexcept>

namespace easy3d {
namespace {
vec3 world_pixel(int x,int y,float depth,int width,int height,const mat4& inverse_mvp) {
    const vec4 ndc(2*(x+.5f)/width-1,1-2*(y+.5f)/height,2*depth-1,1);
    const auto world=inverse_mvp*ndc;
    if (!std::isfinite(world.w) || world.w==0) throw std::invalid_argument("invalid camera unprojection");
    const vec3 point(world.x/world.w,world.y/world.w,world.z/world.w);
    for (unsigned i=0;i<3;++i) if (!std::isfinite(point[i])) throw std::invalid_argument("non-finite world point");
    return point;
}
}
struct VulkanOffScreen::Impl {
    std::unique_ptr<graphics::RenderDevice> device;
    std::unique_ptr<Camera> camera{new Camera};
    int width = 0, height = 0;
    vec4 background = vec4(1, 1, 1, 1);
    std::vector<std::shared_ptr<Model>> models;
    std::vector<std::shared_ptr<Drawable>> drawables;
    std::vector<graphics::DrawCommand> overlays;
    std::vector<vec4> clipping_planes;
    struct Cached {
        std::weak_ptr<Drawable> source;
        std::size_t revision = 0;
        bool use_colors = false;
        bool flat_shading = false;
        bool textured = false;
        bool selected = false, highlighted = false;
        std::pair<int,int> highlight_range = {-1,-1};
        vec4 highlight_color = vec4(0);
        float repeat = 1;
        std::shared_ptr<const graphics::Geometry> geometry;
        std::shared_ptr<const graphics::PointData> points;
        std::shared_ptr<const graphics::Image> texture_image;
    };
    mutable std::map<const Drawable*, Cached> cache;

    struct CachedImage {
        std::weak_ptr<const Texture::PixelData> source;
        std::weak_ptr<const graphics::Image> image;
    };
    mutable std::map<const Texture::PixelData*, CachedImage> images;
    std::shared_ptr<const graphics::Image> image(const Texture* texture) const {
        if (!texture || !texture->pixels()) throw std::logic_error("texture coloring requires CPU image data");
        const auto& pixels = texture->pixels();
        auto& cached = images[pixels.get()];
        if (cached.source.lock() == pixels) {
            auto existing = cached.image.lock();
            if (existing) return existing;
        }
        const uint64_t count = uint64_t(pixels->width)*pixels->height;
        if (count > std::numeric_limits<size_t>::max()/4) throw std::invalid_argument("texture is too large for RGBA8");
        std::vector<unsigned char> rgba(size_t(count)*4);
        for (int y = 0; y < pixels->height; ++y) for (int x = 0; x < pixels->width; ++x) {
            const size_t source = (size_t(pixels->height-1-y)*pixels->width+x)*pixels->channels;
            const size_t target = (size_t(y)*pixels->width+x)*4;
            rgba[target] = pixels->bytes[source];
            rgba[target+1] = pixels->channels > 1 ? pixels->bytes[source+1] : 0;
            rgba[target+2] = pixels->channels > 2 ? pixels->bytes[source+2] : 0;
            rgba[target+3] = pixels->channels > 3 ? pixels->bytes[source+3] : 255;
        }
        graphics::Sampler sampler;
        sampler.wrap = texture->wrap_mode() == Texture::REPEAT ? graphics::TextureWrap::repeat : graphics::TextureWrap::clamp_to_edge;
        sampler.filter = texture->filter_mode() == Texture::LINEAR ? graphics::TextureFilter::linear : graphics::TextureFilter::nearest;
        auto result = std::make_shared<graphics::Image>(pixels->width,pixels->height,std::move(rgba),sampler);
        cached.source = pixels; cached.image = result;
        return result;
    }

    graphics::DrawCommand command(const std::shared_ptr<Drawable>& drawable, uint32_t id, const Camera& cam, bool model_selected = false) const {
        auto& d = *drawable;
        if (!d.has_cpu_buffers())
            throw std::logic_error("drawable must use CPU buffers before Vulkan rendering");
        if (d.coloring_method() != State::UNIFORM_COLOR && d.coloring_method() != State::COLOR_PROPERTY &&
            d.coloring_method() != State::TEXTURED && d.coloring_method() != State::SCALAR_FIELD)
            throw std::logic_error("unknown drawable coloring method");
        if (d.color().w != 1 || d.is_ssao_enabled())
            throw std::logic_error("opacity and SSAO are not yet ported");
        if (auto* p = dynamic_cast<PointsDrawable*>(&d)) {
            if (p->impostor_type() != PointsDrawable::PLAIN && p->impostor_type() != PointsDrawable::SPHERE &&
                p->impostor_type() != PointsDrawable::SURFEL)
                throw std::logic_error("unknown point impostor type");
        }
        if (auto* l = dynamic_cast<LinesDrawable*>(&d)) {
            if (l->impostor_type() != LinesDrawable::PLAIN && l->impostor_type() != LinesDrawable::CYLINDER &&
                l->impostor_type() != LinesDrawable::CONE)
                throw std::logic_error("unknown line impostor type");
            if (!std::isfinite(l->line_width()) || l->line_width() <= 0)
                throw std::invalid_argument("line width must be finite and positive");
        }
        // As with the legacy opaque pass, TrianglesDrawable::opacity() is only
        // meaningful in a separate transparency pass (not implemented here).
        const auto& buffers = d.cpu_buffers();
        if (auto* p = dynamic_cast<PointsDrawable*>(&d))
            if (p->impostor_type() == PointsDrawable::SURFEL && !buffers.vertices.empty() && buffers.normals.empty())
                throw std::logic_error("surfel rendering requires point normals");
        auto& entry = cache[&d];
        const bool use_colors = d.coloring_method() == State::COLOR_PROPERTY;
        const bool textured = d.coloring_method() == State::TEXTURED || d.coloring_method() == State::SCALAR_FIELD;
        const float repeat = d.texture_repeat()+d.texture_fractional_repeat()/100.0f;
        if (textured && !std::isfinite(repeat)) throw std::invalid_argument("texture repeat must be finite");
        const auto* triangles = dynamic_cast<const TrianglesDrawable*>(&d);
        const bool flat_shading = triangles && !triangles->smooth_shading();
        const bool selected = model_selected || d.is_selected();
        const bool highlighted = d.highlight();
        if (entry.source.lock() != drawable || entry.revision != buffers.revision || !entry.geometry ||
            entry.use_colors != use_colors || entry.flat_shading != flat_shading ||
            entry.textured != textured || (textured && entry.repeat != repeat) ||
            entry.selected != selected || entry.highlighted != highlighted ||
            entry.highlight_range != d.highlight_range() || entry.highlight_color != setting::highlight_color) {
            const auto n = buffers.vertices.size();
            if ((!buffers.colors.empty() && buffers.colors.size() != n) ||
                (!buffers.normals.empty() && buffers.normals.size() != n))
                throw std::logic_error("drawable attribute count does not match vertices");
            if (textured && buffers.texcoords.size() != n)
                throw std::logic_error("texture coordinate count does not match vertices");
            std::vector<graphics::Vertex> vertices(n);
            for (size_t i = 0; i < n; ++i) {
                vertices[i].position = buffers.vertices[i];
                if (!buffers.normals.empty()) vertices[i].normal = buffers.normals[i];
                if (use_colors && !buffers.colors.empty()) vertices[i].color = vec4(buffers.colors[i], 1);
                if (textured) vertices[i].texcoord = vec2(buffers.texcoords[i].x*repeat,1-buffers.texcoords[i].y*repeat);
            }
            const auto topology = d.type() == Drawable::DT_TRIANGLES ? graphics::Topology::triangles :
                                  d.type() == Drawable::DT_LINES ? graphics::Topology::lines : graphics::Topology::points;
            if (topology==graphics::Topology::points) {
                const auto count=buffers.indices.empty()?n:buffers.indices.size();
                if (n>UINT32_MAX) throw std::invalid_argument("too many point source slots");
                bool unchanged=entry.points && entry.points->positions().size()==count && entry.points->source_count()==n;
                for (size_t i=0;i<count;++i) {
                    const auto index=buffers.indices.empty()?uint32_t(i):buffers.indices[i];
                    if (index>=n) throw std::out_of_range("point drawable index out of range");
                    if (unchanged && (entry.points->positions()[i]!=buffers.vertices[index] || entry.points->source_ids()[i]!=index))
                        unchanged=false;
                }
                if (!unchanged) {
                    std::vector<vec3> positions; std::vector<uint32_t> ids;
                    positions.reserve(count); ids.reserve(count);
                    for (size_t i=0;i<count;++i) {
                        const auto index=buffers.indices.empty()?uint32_t(i):buffers.indices[i];
                        positions.push_back(buffers.vertices[index]); ids.push_back(index);
                    }
                    entry.points=std::make_shared<graphics::PointData>(std::move(positions),std::move(ids),uint32_t(n));
                }
            } else entry.points.reset();
            auto geometry = std::make_shared<graphics::Geometry>(vertices, buffers.indices, topology,std::vector<uint32_t>{},entry.points);
            if (flat_shading) {
                // Geometry has already validated the indices and triangle count.
                // Give every corner its triangle normal while preserving colors
                // and primitive order, including indexed/shared-vertex inputs.
                const auto& indices = geometry->indices();
                const size_t count = indices.empty() ? vertices.size() : indices.size();
                std::vector<graphics::Vertex> flat(count);
                for (size_t i = 0; i < count; i += 3) {
                    for (size_t j = 0; j < 3; ++j)
                        flat[i+j] = vertices[indices.empty() ? i+j : indices[i+j]];
                    const vec3 normal = cross(flat[i+1].position-flat[i].position,
                                              flat[i+2].position-flat[i].position);
                    for (size_t j = 0; j < 3; ++j) flat[i+j].normal = normal;
                }
                geometry = std::make_shared<graphics::Geometry>(std::move(flat), std::vector<uint32_t>{}, topology);
            }
            if (selected || highlighted) {
                const auto& source=geometry->vertices(); const auto& indices=geometry->indices();
                const auto count=indices.empty() ? source.size() : indices.size();
                const unsigned stride=topology==graphics::Topology::triangles ? 3 : topology==graphics::Topology::lines ? 2 : 1;
                std::vector<graphics::Vertex> marked(count);
                const auto range=d.highlight_range();
                for (size_t i=0;i<count;++i) {
                    marked[i]=source[indices.empty() ? i : indices[i]];
                    float strength=selected ? 0.6f : 0;
                    if (highlighted && range.first>=0 && i/stride>=size_t(range.first) &&
                        range.second>=range.first && i/stride<=size_t(range.second)) strength=0.8f+0.2f*strength;
                    marked[i].highlight=vec4(setting::highlight_color.x,setting::highlight_color.y,setting::highlight_color.z,strength);
                }
                geometry=std::make_shared<graphics::Geometry>(std::move(marked),std::vector<uint32_t>{},topology,std::vector<uint32_t>{},entry.points);
            }
            entry.selected=selected; entry.highlighted=highlighted;
            entry.highlight_range=d.highlight_range(); entry.highlight_color=setting::highlight_color;
            entry.source = drawable; entry.revision = buffers.revision; entry.geometry = geometry;
            entry.use_colors = use_colors; entry.flat_shading = flat_shading;
            entry.textured = textured; entry.repeat = repeat;
        }
        graphics::DrawCommand result;
        result.geometry = entry.geometry;
        entry.texture_image = textured ? image(d.texture()) : nullptr;
        result.texture = entry.texture_image;
        if (auto* p = dynamic_cast<PointsDrawable*>(&d)) {
            result.point_size = p->point_size();
            result.point_style = p->impostor_type() == PointsDrawable::SPHERE ? graphics::PointStyle::sphere :
                                 p->impostor_type() == PointsDrawable::SURFEL ? graphics::PointStyle::surfel : graphics::PointStyle::plain;
            result.sphere_radius = p->point_size() * cam.pixelGLRatio(cam.pivotPoint()) * 0.5f;
            result.model_view = cam.modelViewMatrix() * d.manipulated_matrix();
            result.projection = cam.projectionMatrix();
            result.sphere_light = mat3(cam.modelViewMatrix()) * vec3(0.3f, 0.5f, 1);
        }
        if (auto* l = dynamic_cast<LinesDrawable*>(&d)) {
            result.line_style = l->impostor_type() == LinesDrawable::CYLINDER ? graphics::LineStyle::cylinder :
                                l->impostor_type() == LinesDrawable::CONE ? graphics::LineStyle::cone : graphics::LineStyle::plain;
            result.line_width = l->line_width();
            result.line_radius = l->line_width() * cam.pixelGLRatio(cam.pivotPoint()) * 0.5f;
            result.model_view = cam.modelViewMatrix() * d.manipulated_matrix();
            result.projection = cam.projectionMatrix();
            result.sphere_light = mat3(cam.modelViewMatrix()) * vec3(0.3f, 0.5f, 1);
        }
        result.model_view_projection = cam.modelViewProjectionMatrix() * d.manipulated_matrix();
        if (!clipping_planes.empty()) {
            result.model_view=cam.modelViewMatrix()*d.manipulated_matrix();
            const auto plane_transform=transpose(inverse(cam.modelViewMatrix()));
            for (const auto& plane:clipping_planes) result.clipping_planes.push_back(plane_transform*plane);
            result.discard_clipped_primitives=d.plane_clip_discard_primitive();
        }
        if (d.lighting()) result.normal_matrix = transform::normal_matrix(d.manipulated_matrix());
        result.color = d.coloring_method() == State::UNIFORM_COLOR ? d.color() : vec4(1, 1, 1, 1);
        result.object_id = id;
        // Basic world-space diffuse lighting; full material parity is pending.
        result.lighting = d.lighting();
        return result;
    }
    struct PickSource { std::shared_ptr<Model> model; std::shared_ptr<Drawable> drawable; };
    std::vector<graphics::DrawCommand> commands(const Camera& cam, std::vector<PickSource>* sources = nullptr) const {
        for (auto it = cache.begin(); it != cache.end(); ) {
            if (it->second.source.expired()) it = cache.erase(it); else ++it;
        }
        for (auto it = images.begin(); it != images.end(); ) {
            if (it->second.source.expired() || it->second.image.expired()) it = images.erase(it); else ++it;
        }
        std::vector<graphics::DrawCommand> commands;
        auto append=[&](const std::shared_ptr<Drawable>& d,uint32_t id,const std::shared_ptr<Model>& model,bool selected) {
            if (!d->is_visible()) return;
            if (sources) {
                if (sources->size()>=UINT32_MAX) throw std::length_error("too many picking sources");
                id=uint32_t(sources->size()); sources->push_back({model,d});
            }
            commands.push_back(command(d,id,cam,selected));
        };
        for (size_t i = 0; i < models.size(); ++i) {
            const auto* renderer = models[i]->renderer();
            if (!renderer || !renderer->is_visible()) continue;
            for (const auto& d : renderer->triangles_drawables())
                append(d,uint32_t(i),models[i],renderer->is_selected());
            for (const auto& d : renderer->lines_drawables())
                append(d,uint32_t(i),models[i],renderer->is_selected());
            for (const auto& d : renderer->points_drawables())
                append(d,uint32_t(i),models[i],renderer->is_selected());
        }
        for (size_t i = 0; i < drawables.size(); ++i)
            append(drawables[i],uint32_t(models.size()+i),nullptr,false);
        commands.insert(commands.end(),overlays.begin(),overlays.end());
        return commands;
    }
    graphics::FrameImage capture(const Camera& cam, const vec4& bg, unsigned samples = 1) const {
        return device->render(uint32_t(cam.screenWidth()), uint32_t(cam.screenHeight()), commands(cam), bg, samples);
    }
};
VulkanOffScreen::VulkanOffScreen(int width, int height, const graphics::DeviceOptions& options)
    : VulkanOffScreen(width, height, graphics::create_vulkan_device(options)) {}
VulkanOffScreen::VulkanOffScreen(int width, int height, std::unique_ptr<graphics::RenderDevice> device) : impl_(new Impl) {
    if (!device) throw std::invalid_argument("scene requires a graphics device");
    resize(width, height);
    impl_->device = std::move(device);
}
bool VulkanOffScreen::present(unsigned samples) {
    return impl_->device->present(uint32_t(width()), uint32_t(height()), impl_->commands(*camera()), background_color(), samples);
}
void VulkanOffScreen::release_device() { impl_->device.reset(); }
VulkanOffScreen::~VulkanOffScreen() = default;
Camera* VulkanOffScreen::camera() { return impl_->camera.get(); }
const Camera* VulkanOffScreen::camera() const { return impl_->camera.get(); }
int VulkanOffScreen::width() const { return impl_->width; }
int VulkanOffScreen::height() const { return impl_->height; }
void VulkanOffScreen::resize(int w, int h) {
    if (w <= 0 || h <= 0) throw std::invalid_argument("offscreen dimensions must be positive");
    impl_->width = w; impl_->height = h; camera()->setScreenWidthAndHeight(w, h);
}
void VulkanOffScreen::set_background_color(const vec4& color) { impl_->background = color; }
const vec4& VulkanOffScreen::background_color() const { return impl_->background; }
Model* VulkanOffScreen::add_model(std::shared_ptr<Model> model, bool create) {
    return add_model(std::move(model),create,true);
}
Model* VulkanOffScreen::add_model(std::shared_ptr<Model> model, bool create, bool fit_scene) {
    if (!model || model->empty()) return nullptr;
    for (const auto& current : impl_->models) if (model == current) return current.get();
    // Construct first: unsupported default coloring fails without replacing the model's existing renderer.
    auto renderer = std::make_shared<Renderer>(model.get(), create, true);
    // Portable initial defaults. Callers can request other sizes/styles and receive an explicit error at capture.
    for (const auto& d : renderer->points_drawables()) { d->set_impostor_type(PointsDrawable::PLAIN); d->set_point_size(1); }
    for (const auto& d : renderer->lines_drawables()) { d->set_impostor_type(LinesDrawable::PLAIN); d->set_line_width(1); }
    model->set_renderer(renderer);
    impl_->models.push_back(model);
    if (fit_scene) fit_screen();
    return model.get();
}
Model* VulkanOffScreen::add_model(const std::string& filename, bool create) {
    const auto ext = file_system::extension(filename, true);
    std::shared_ptr<Model> model;
    if (ext == "ply" && io::PlyReader::num_instances(filename, "face") > 0)
        model.reset(SurfaceMeshIO::load(filename));
    else if (ext == "ply" && io::PlyReader::num_instances(filename, "edge") > 0)
        model.reset(GraphIO::load(filename));
    else if (ext == "obj" || ext == "off" || ext == "stl" || ext == "sm" || ext == "geojson" || ext == "trilist")
        model.reset(SurfaceMeshIO::load(filename));
    else if (ext == "plm" || ext == "pm" || ext == "mesh") model.reset(PolyMeshIO::load(filename));
    else if (ext == "ptx") throw std::logic_error("multi-scan PTX loading is not yet ported to VulkanOffScreen");
    else model.reset(PointCloudIO::load(filename));
    if (model) model->set_name(filename);
    return add_model(model, create);
}
Model* VulkanOffScreen::add_model(Model* model, bool create) {
    if (!model) return nullptr;
    // Do not manufacture a second shared_ptr control block for an owned model.
    for (const auto& current : impl_->models) if (current.get() == model) return model;
    return add_model(std::shared_ptr<Model>(model), create);
}
bool VulkanOffScreen::delete_model(Model* model) {
    auto& values = impl_->models;
    auto found = std::find_if(values.begin(), values.end(), [model](const std::shared_ptr<Model>& p) { return p.get() == model; });
    if (found == values.end()) return false;
    values.erase(found); impl_->cache.clear(); return true;
}
const std::vector<std::shared_ptr<Model>>& VulkanOffScreen::models() const { return impl_->models; }
Model* VulkanOffScreen::current_model() const { return impl_->models.empty() ? nullptr : impl_->models.back().get(); }
Drawable* VulkanOffScreen::add_drawable(std::shared_ptr<Drawable> drawable) {
    if (!drawable) return nullptr;
    for (const auto& d : impl_->drawables) if (d == drawable) return d.get();
    drawable->enable_cpu_buffers(); impl_->drawables.push_back(drawable); return drawable.get();
}
bool VulkanOffScreen::delete_drawable(Drawable* drawable) {
    auto& values = impl_->drawables;
    auto found = std::find_if(values.begin(), values.end(), [drawable](const std::shared_ptr<Drawable>& p) { return p.get() == drawable; });
    if (found == values.end()) return false;
    values.erase(found); impl_->cache.clear(); return true;
}
const std::vector<std::shared_ptr<Drawable>>& VulkanOffScreen::drawables() const { return impl_->drawables; }
void VulkanOffScreen::clear_scene() {
    impl_->models.clear(); impl_->drawables.clear(); impl_->cache.clear(); impl_->overlays.clear();
}
void VulkanOffScreen::set_overlays(std::vector<graphics::DrawCommand> commands) {
    for (auto& command : commands) command.overlay = true;
    impl_->overlays = std::move(commands);
}
void VulkanOffScreen::set_clipping_planes(const std::vector<vec4>& planes) {
    impl_->clipping_planes=graphics::normalize_clipping_planes(planes);
}
const std::vector<vec4>& VulkanOffScreen::clipping_planes() const { return impl_->clipping_planes; }
void VulkanOffScreen::fit_screen(const Model* selected) {
    Box3 box;
    if (selected) box.grow(selected->bounding_box());
    else {
        for (const auto& model : impl_->models) box.grow(model->bounding_box());
        for (const auto& drawable : impl_->drawables) box.grow(drawable->bounding_box());
    }
    if (box.is_valid()) { camera()->setSceneBoundingBox(box.min_point(), box.max_point()); camera()->showEntireScene(); }
}
vec3 VulkanOffScreen::unproject_pixel(int x,int y,float depth) const {
    if (x<0 || y<0 || x>=width() || y>=height() || !std::isfinite(depth) || depth<0 || depth>1)
        throw std::invalid_argument("invalid framebuffer pixel/depth for unprojection");
    return world_pixel(x,y,depth,width(),height(),inverse(camera()->modelViewProjectionMatrix()));
}
std::vector<vec3> VulkanOffScreen::scan_visible_points(unsigned samples) const {
    Camera snapshot(*camera());
    // Camera's copy constructor regenerates matrices from its parameters. Keep
    // effective matrices too, including externally calibrated/custom cameras.
    snapshot.frame()->setPivotPoint(camera()->pivotPoint()); // do not re-adjust the copied ortho coefficient
    snapshot.set_projection_matrix(camera()->projectionMatrix());
    snapshot.set_modelview_matrix(camera()->modelViewMatrix());
    const auto inv=inverse(snapshot.modelViewProjectionMatrix());
    for (unsigned i=0;i<16;++i) if (!std::isfinite(inv[i])) throw std::invalid_argument("invalid scan camera inverse");
    const auto frame=impl_->capture(snapshot,background_color(),samples);
    std::vector<vec3> points;
    points.reserve(std::count_if(frame.depth.begin(),frame.depth.end(),[](float d) { return d>=0 && d<1; }));
    for (unsigned y=0;y<frame.height;++y) for (unsigned x=0;x<frame.width;++x) {
        const size_t pixel=size_t(y)*frame.width+x;
        const float d=frame.depth[pixel];
        if (frame.ids[pixel].object==UINT32_MAX || !std::isfinite(d) || d<0 || d>=1) continue;
        points.push_back(world_pixel(int(x),int(y),d,int(frame.width),int(frame.height),inv));
    }
    return points;
}
VulkanOffScreen::PickResult VulkanOffScreen::pick(int x,int y) const {
    PickResult result;
    if (x<0 || y<0 || x>=width() || y>=height()) return result;
    // Keep commands and source mapping together for this exact synchronous draw.
    std::vector<Impl::PickSource> sources;
    const auto commands=impl_->commands(*camera(),&sources);
    const auto frame=impl_->device->render(width(),height(),commands,background_color(),1);
    const auto pixel=size_t(y)*frame.width+x;
    const auto id=frame.ids[pixel];
    if (id.object==UINT32_MAX || frame.depth[pixel]>=1 || id.object>=sources.size()) return result;
    result.model=sources[id.object].model; result.drawable=sources[id.object].drawable;
    result.primitive=id.primitive; result.depth=frame.depth[pixel];
    result.position=unproject_pixel(x,y,result.depth);
    const auto& faces=result.drawable->cpu_buffers().source_faces;
    auto* mesh=dynamic_cast<SurfaceMesh*>(result.model.get());
    if (mesh && id.primitive<faces.size()) {
        const SurfaceMesh::Face f(faces[id.primitive]);
        if (mesh->is_valid(f) && !mesh->is_deleted(f)) result.face_index=f.idx();
    }
    return result;
}
graphics::FrameImage VulkanOffScreen::capture(unsigned samples) const { return impl_->capture(*camera(), background_color(), samples); }
const graphics::RenderDevice& VulkanOffScreen::device() const { return *impl_->device; }
std::vector<std::uint32_t> VulkanOffScreen::select_points(const graphics::PointSelectionRequest& request) const {
    return impl_->device->select_points(request);
}
std::shared_ptr<const graphics::PointData> VulkanOffScreen::point_resource(Drawable* drawable) const {
    if (!drawable || drawable->type()!=Drawable::DT_POINTS) throw std::invalid_argument("selection requires a point drawable");
    for (const auto& model:impl_->models) if (model->renderer())
        for (const auto& d:model->renderer()->points_drawables()) if (d.get()==drawable)
            return impl_->command(d,0,*camera(),model->renderer()->is_selected()).geometry->point_data();
    for (const auto& d:impl_->drawables) if (d.get()==drawable)
        return impl_->command(d,0,*camera()).geometry->point_data();
    throw std::invalid_argument("point drawable does not belong to this viewer");
}
bool VulkanOffScreen::render(const std::string& filename, float scaling, int samples, int bg, bool expand) const {
    try {
        if (samples <= 0) throw std::invalid_argument("sample count must be positive");
        if (!expand) throw std::invalid_argument("VulkanOffScreen currently requires expand=true");
        if (!std::isfinite(scaling) || scaling <= 0 || double(width()) * scaling > std::numeric_limits<int>::max() ||
            double(height()) * scaling > std::numeric_limits<int>::max() || bg < 0 || bg > 2)
            throw std::invalid_argument("invalid snapshot scaling/background");
        Camera copy(*camera());
        const int w = int(width() * double(scaling)), h = int(height() * double(scaling));
        if (w <= 0 || h <= 0) throw std::invalid_argument("scaled snapshot dimensions are zero");
        copy.setScreenWidthAndHeight(w, h);
        auto frame = impl_->capture(copy, bg == 0 ? background_color() : bg == 1 ? vec4(1, 1, 1, 1) : vec4(0, 0, 0, 0), unsigned(samples));
        if (bg == 2) {
            // Alpha overlays and MSAA coverage against transparent black produce
            // premultiplied colors, including single-sample text. PNG stores straight alpha.
            for (size_t i = 0; i < frame.rgba.size(); i += 4) {
                const unsigned alpha = frame.rgba[i + 3];
                if (alpha && alpha < 255)
                    for (unsigned c = 0; c < 3; ++c)
                        frame.rgba[i + c] = static_cast<unsigned char>(std::min(255u,
                            (unsigned(frame.rgba[i + c]) * 255 + alpha / 2) / alpha));
            }
        }
        return ImageIO::save(filename, frame.rgba, w, h, 4, false);
    } catch (const std::exception& e) {
        LOG(ERROR) << "Vulkan snapshot failed: " << e.what(); return false;
    }
}
}
