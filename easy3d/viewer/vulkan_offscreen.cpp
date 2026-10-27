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
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <stdexcept>

namespace easy3d {
struct VulkanOffScreen::Impl {
    std::unique_ptr<graphics::RenderDevice> device;
    std::unique_ptr<Camera> camera{new Camera};
    int width = 0, height = 0;
    vec4 background = vec4(1, 1, 1, 1);
    std::vector<std::shared_ptr<Model>> models;
    std::vector<std::shared_ptr<Drawable>> drawables;
    struct Cached {
        std::weak_ptr<Drawable> source;
        std::size_t revision = 0;
        bool use_colors = false;
        bool flat_shading = false;
        std::shared_ptr<const graphics::Geometry> geometry;
    };
    mutable std::map<const Drawable*, Cached> cache;

    graphics::DrawCommand command(const std::shared_ptr<Drawable>& drawable, uint32_t id, const Camera& cam) const {
        auto& d = *drawable;
        if (!d.has_cpu_buffers())
            throw std::logic_error("drawable must use CPU buffers before Vulkan rendering");
        if (d.coloring_method() != State::UNIFORM_COLOR && d.coloring_method() != State::COLOR_PROPERTY)
            throw std::logic_error("Vulkan scene adapter currently supports uniform/property coloring; use graphics::Image for native textures");
        if (d.color().w != 1 || d.is_ssao_enabled() || d.highlight() || d.is_selected())
            throw std::logic_error("opacity, SSAO and selection highlighting are not yet ported");
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
        const auto* triangles = dynamic_cast<const TrianglesDrawable*>(&d);
        const bool flat_shading = triangles && !triangles->smooth_shading();
        if (entry.source.lock() != drawable || entry.revision != buffers.revision || !entry.geometry ||
            entry.use_colors != use_colors || entry.flat_shading != flat_shading) {
            const auto n = buffers.vertices.size();
            if ((!buffers.colors.empty() && buffers.colors.size() != n) ||
                (!buffers.normals.empty() && buffers.normals.size() != n))
                throw std::logic_error("drawable attribute count does not match vertices");
            std::vector<graphics::Vertex> vertices(n);
            for (size_t i = 0; i < n; ++i) {
                vertices[i].position = buffers.vertices[i];
                if (!buffers.normals.empty()) vertices[i].normal = buffers.normals[i];
                if (use_colors && !buffers.colors.empty()) vertices[i].color = vec4(buffers.colors[i], 1);
            }
            const auto topology = d.type() == Drawable::DT_TRIANGLES ? graphics::Topology::triangles :
                                  d.type() == Drawable::DT_LINES ? graphics::Topology::lines : graphics::Topology::points;
            auto geometry = std::make_shared<graphics::Geometry>(vertices, buffers.indices, topology);
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
            entry.source = drawable; entry.revision = buffers.revision; entry.geometry = geometry;
            entry.use_colors = use_colors; entry.flat_shading = flat_shading;
        }
        graphics::DrawCommand result;
        result.geometry = entry.geometry;
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
        if (d.lighting()) result.normal_matrix = transform::normal_matrix(d.manipulated_matrix());
        result.color = d.coloring_method() == State::UNIFORM_COLOR ? d.color() : vec4(1, 1, 1, 1);
        result.object_id = id;
        // Basic world-space diffuse lighting; full material parity is pending.
        result.lighting = d.lighting();
        return result;
    }
    std::vector<graphics::DrawCommand> commands(const Camera& cam) const {
        for (auto it = cache.begin(); it != cache.end(); ) {
            if (it->second.source.expired()) it = cache.erase(it); else ++it;
        }
        std::vector<graphics::DrawCommand> commands;
        for (size_t i = 0; i < models.size(); ++i) {
            const auto* renderer = models[i]->renderer();
            if (!renderer || !renderer->is_visible()) continue;
            if (renderer->is_selected()) throw std::logic_error("model selection highlighting is not yet ported");
            for (const auto& d : renderer->triangles_drawables())
                if (d->is_visible()) commands.push_back(command(d, uint32_t(i), cam));
            for (const auto& d : renderer->lines_drawables())
                if (d->is_visible()) commands.push_back(command(d, uint32_t(i), cam));
            for (const auto& d : renderer->points_drawables())
                if (d->is_visible()) commands.push_back(command(d, uint32_t(i), cam));
        }
        for (size_t i = 0; i < drawables.size(); ++i)
            if (drawables[i]->is_visible()) commands.push_back(command(drawables[i], uint32_t(models.size() + i), cam));
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
    if (!model || model->empty()) return nullptr;
    for (const auto& current : impl_->models) if (model == current) return current.get();
    // Construct first: unsupported default coloring fails without replacing the model's existing renderer.
    auto renderer = std::make_shared<Renderer>(model.get(), create, true);
    // Portable initial defaults. Callers can request other sizes/styles and receive an explicit error at capture.
    for (const auto& d : renderer->points_drawables()) { d->set_impostor_type(PointsDrawable::PLAIN); d->set_point_size(1); }
    for (const auto& d : renderer->lines_drawables()) { d->set_impostor_type(LinesDrawable::PLAIN); d->set_line_width(1); }
    model->set_renderer(renderer);
    impl_->models.push_back(model);
    fit_screen();
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
void VulkanOffScreen::clear_scene() { impl_->models.clear(); impl_->drawables.clear(); impl_->cache.clear(); }
void VulkanOffScreen::fit_screen(const Model* selected) {
    Box3 box;
    if (selected) box.grow(selected->bounding_box());
    else {
        for (const auto& model : impl_->models) box.grow(model->bounding_box());
        for (const auto& drawable : impl_->drawables) box.grow(drawable->bounding_box());
    }
    if (box.is_valid()) { camera()->setSceneBoundingBox(box.min_point(), box.max_point()); camera()->showEntireScene(); }
}
graphics::FrameImage VulkanOffScreen::capture(unsigned samples) const { return impl_->capture(*camera(), background_color(), samples); }
const graphics::RenderDevice& VulkanOffScreen::device() const { return *impl_->device; }
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
        if (bg == 2 && samples > 1) {
            // Opaque fragments resolved against transparent black produce
            // coverage-premultiplied edges. Image files store straight alpha.
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
