// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/vulkan_offscreen.h>
#include <easy3d/core/model.h>
#include <easy3d/util/setting.h>
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
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/backends/vulkan/drawable.h>
#include <easy3d/renderer/backends/vulkan/clipping_plane.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#include <algorithm>
#include <cmath>
#include <limits>
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
    vec4 background = setting::background_color;
    std::vector<std::shared_ptr<Model>> models;
    int model_idx = -1;
    std::vector<std::shared_ptr<Drawable>> drawables;
    std::vector<graphics::DrawCommand> overlays, viewer_overlays, viewer_3d_overlays, viewer_annotations;
    std::vector<vec4> clipping_planes;
    mutable std::vector<vec4> shared_clipping_planes;
    mutable vulkan::ClippingPlaneRenderer clipping_plane_renderer;
    vulkan::DrawableRenderer drawable_renderer;

    const std::vector<vec4>& active_clipping_planes() const {
        const auto& plane = *ClippingPlane::instance();
        if (!plane.is_enabled()) return clipping_planes;
        shared_clipping_planes = vulkan::ClippingPlaneRenderer::planes(plane);
        return shared_clipping_planes;
    }

    struct PickSource { std::shared_ptr<Model> model; std::shared_ptr<Drawable> drawable; };
    std::vector<graphics::DrawCommand> commands(const Camera& cam, std::vector<PickSource>* sources = nullptr) const {
        drawable_renderer.prune();
        std::vector<graphics::DrawCommand> commands;
        const auto& planes = active_clipping_planes();
        auto append=[&](const std::shared_ptr<Drawable>& d,uint32_t id,const std::shared_ptr<Model>& model,bool selected) {
            if (!d->is_visible()) return;
            auto command=drawable_renderer.command(d,id,cam,planes,selected);
            if (sources && !command.overlay) {
                if (sources->size()>=UINT32_MAX) throw std::length_error("too many picking sources");
                id=uint32_t(sources->size()); sources->push_back({model,d});
                command.object_id=id;
            }
            commands.push_back(std::move(command));
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
        const auto guide = clipping_plane_renderer.draw(*ClippingPlane::instance(),cam);
        commands.insert(commands.end(),guide.begin(),guide.end());
        commands.insert(commands.end(),viewer_annotations.begin(),viewer_annotations.end());
        commands.insert(commands.end(),overlays.begin(),overlays.end());
        commands.insert(commands.end(),viewer_3d_overlays.begin(),viewer_3d_overlays.end());
        commands.insert(commands.end(),viewer_overlays.begin(),viewer_overlays.end());
        return commands;
    }
    graphics::FrameImage capture(const Camera& cam, const vec4& bg, unsigned samples = 1) const {
        return device->render(uint32_t(cam.screenWidth()), uint32_t(cam.screenHeight()), commands(cam), bg, samples);
    }
};
VulkanOffScreen::VulkanOffScreen(int width, int height, const graphics::DeviceOptions& options)
    : VulkanOffScreen(width, height, graphics::create_vulkan_device(options)) {
    camera()->setType(Camera::PERSPECTIVE);
    camera()->setUpVector(vec3(0, 0, 1));
    camera()->setViewDirection(vec3(-1, 0, 0));
    camera()->showEntireScene();
}
VulkanOffScreen::VulkanOffScreen(int width, int height, std::unique_ptr<graphics::RenderDevice> device) : impl_(new Impl) {
    if (!device) throw std::invalid_argument("scene requires a graphics device");
    resize(width, height);
    impl_->device = std::move(device);

    VLOG(1) << "Renderer backend: Vulkan";
    VLOG(1) << "Vulkan device: " << impl_->device->name();
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
    model->set_renderer(renderer);
    impl_->models.push_back(model);
    impl_->model_idx = int(impl_->models.size())-1;
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
    values.erase(found); impl_->model_idx = int(impl_->models.size())-1; impl_->drawable_renderer.clear(); return true;
}
const std::vector<std::shared_ptr<Model>>& VulkanOffScreen::models() const { return impl_->models; }
Model* VulkanOffScreen::current_model() const {
    return impl_->model_idx < 0 || size_t(impl_->model_idx) >= impl_->models.size() ? nullptr : impl_->models[impl_->model_idx].get();
}
void VulkanOffScreen::set_current_model_index(int index) {
    if (index < -1 || (index >= 0 && size_t(index) >= impl_->models.size()))
        throw std::out_of_range("invalid current model index");
    impl_->model_idx = index;
}
Drawable* VulkanOffScreen::add_drawable(std::shared_ptr<Drawable> drawable) {
    if (!drawable) return nullptr;
    for (const auto& d : impl_->drawables) if (d == drawable) return d.get();
    drawable->enable_cpu_buffers(); impl_->drawables.push_back(drawable); return drawable.get();
}
bool VulkanOffScreen::delete_drawable(Drawable* drawable) {
    auto& values = impl_->drawables;
    auto found = std::find_if(values.begin(), values.end(), [drawable](const std::shared_ptr<Drawable>& p) { return p.get() == drawable; });
    if (found == values.end()) return false;
    values.erase(found); impl_->drawable_renderer.clear(); return true;
}
const std::vector<std::shared_ptr<Drawable>>& VulkanOffScreen::drawables() const { return impl_->drawables; }
void VulkanOffScreen::clear_scene() {
    impl_->models.clear(); impl_->model_idx = -1; impl_->drawables.clear(); impl_->drawable_renderer.clear(); impl_->overlays.clear(); impl_->viewer_overlays.clear(); impl_->viewer_3d_overlays.clear(); impl_->viewer_annotations.clear();
}
void VulkanOffScreen::set_overlays(std::vector<graphics::DrawCommand> commands) {
    for (auto& command : commands) command.overlay = true;
    impl_->overlays = std::move(commands);
}
void VulkanOffScreen::set_viewer_overlays(std::vector<graphics::DrawCommand> commands) const {
    for (auto& command : commands) { command.overlay = true; command.overlay_depth_test = command.overlay_independent_depth = false; }
    impl_->viewer_overlays = std::move(commands);
}
void VulkanOffScreen::set_viewer_3d_overlays(std::vector<graphics::DrawCommand> commands) const {
    for (auto& command : commands) { command.overlay = command.overlay_independent_depth = true; command.overlay_depth_test = false; }
    impl_->viewer_3d_overlays = std::move(commands);
}
void VulkanOffScreen::set_viewer_annotations(std::vector<graphics::DrawCommand> commands) const {
    for (auto& command : commands) { command.overlay = command.overlay_depth_test = true; command.overlay_independent_depth = false; }
    impl_->viewer_annotations = std::move(commands);
}
void VulkanOffScreen::set_clipping_planes(const std::vector<vec4>& planes) {
    impl_->clipping_planes=graphics::normalize_clipping_planes(planes);
}
const std::vector<vec4>& VulkanOffScreen::clipping_planes() const { return impl_->active_clipping_planes(); }
void VulkanOffScreen::fit_screen(const Model* selected) {
    Box3 box;
    if (selected) box.grow(selected->bounding_box());
    else {
        for (const auto& model : impl_->models) box.grow(model->bounding_box());
        for (const auto& drawable : impl_->drawables)
            if (!dynamic_cast<LinesDrawable2D*>(drawable.get())) box.grow(drawable->bounding_box());
    }
    if (box.is_valid()) {
        camera()->setSceneBoundingBox(box.min_point(), box.max_point()); camera()->showEntireScene();
        ClippingPlane::instance()->fit_scene(camera()->sceneCenter(),camera()->sceneRadius());
    }
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
bool VulkanOffScreen::read_scene_depth(int x,int y,const Camera& snapshot,float& depth) const {
    const auto frame=impl_->capture(snapshot,background_color(),1);
    const auto pixel=size_t(y)*frame.width+size_t(x);
    if (frame.ids[pixel].object==UINT32_MAX) return false;
    depth=frame.depth[pixel];return true;
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
            return impl_->drawable_renderer.command(d,0,*camera(),impl_->clipping_planes,model->renderer()->is_selected()).geometry->point_data();
    for (const auto& d:impl_->drawables) if (d.get()==drawable)
        return impl_->drawable_renderer.command(d,0,*camera(),impl_->clipping_planes).geometry->point_data();
    throw std::invalid_argument("point drawable does not belong to this viewer");
}
bool VulkanOffScreen::render(const std::string& filename, float scaling, int samples, int bg, bool expand) const {
    try {
        if (samples <= 0) throw std::invalid_argument("sample count must be positive");
        if (!std::isfinite(scaling) || scaling <= 0 || double(width()) * scaling > std::numeric_limits<int>::max() ||
            double(height()) * scaling > std::numeric_limits<int>::max() || bg < 0 || bg > 2)
            throw std::invalid_argument("invalid snapshot scaling/background");
        Camera copy(*camera());
        const int w = int(std::ceil(width() * double(scaling))), h = int(std::ceil(height() * double(scaling)));
        if (w <= 0 || h <= 0) throw std::invalid_argument("scaled snapshot dimensions are zero");
        copy.setScreenWidthAndHeight(w, h);
        copy.projectionMatrix(); // resolve resize invalidation before overriding the matrix
        // Match snapshot.cpp: preserve one source frustum dimension, expanding
        // or cropping the other to accommodate the rounded output aspect ratio.
        const double aspect = double(width()) / height(), output_aspect = double(w) / h;
        const double near = camera()->zNear(), far = camera()->zFar();
        float x, y;
        if (camera()->type() == Camera::PERSPECTIVE) {
            y = float(near * std::tan(camera()->fieldOfView() * .5));
            x = float(aspect * y);
        } else camera()->getOrthoWidthHeight(x, y);
        if ((expand && output_aspect > aspect) || (!expand && output_aspect < aspect))
            x = float(output_aspect * y);
        else y = float(x / output_aspect);
        copy.set_projection_matrix(camera()->type() == Camera::PERSPECTIVE
            ? transform::frustum(-x, x, -y, y, float(near), float(far))
            : transform::ortho(-x, x, -y, y, float(near), float(far)));
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
