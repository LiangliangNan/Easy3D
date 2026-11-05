/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/

#ifdef EASY3D_HAS_VULKAN
// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/core/model.h>
#include <easy3d/util/setting.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/renderer/transparency.h>
#include <easy3d/renderer/eye_dome_lighting.h>
#include <easy3d/renderer/ambient_occlusion.h>
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
struct Viewer::Impl {
    std::unique_ptr<graphics::RenderDevice> device;
    Viewer& owner;
    explicit Impl(Viewer& viewer) : owner(viewer) {}
    std::vector<graphics::DrawCommand> overlays, viewer_overlays, viewer_3d_overlays, viewer_annotations;
    bool overlays_foreground = false;
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

    std::vector<graphics::DrawCommand> commands(const Camera& cam, int width, int height,
                                                std::vector<DrawSource>* sources = nullptr, bool include_hud = true) const {
        auto commands = owner.draw_commands(cam,width,height,sources);
        // Recorded overlay viewports use the host framebuffer's pixels. Keep
        // their NDC transforms and scale the clip rectangles for file output.
        const int fw=owner.framebuffer_width(), fh=owner.framebuffer_height();
        const auto append_overlays=[&](const std::vector<graphics::DrawCommand>& source) {
            for (auto command:source) {
                auto& v=command.viewport;
                if (v.z>0 && v.w>0 && fw>0 && fh>0 && (width!=fw || height!=fh)) {
                    const int x=int(std::lround(double(v.x)*width/fw));
                    const int y=int(std::lround(double(v.y)*height/fh));
                    const int right=int(std::lround(double(v.x+v.z)*width/fw));
                    const int bottom=int(std::lround(double(v.y+v.w)*height/fh));
                    if (right<=x || bottom<=y) continue;
                    v=ivec4(x,y,right-x,bottom-y);
                }
                commands.push_back(std::move(command));
            }
        };
        append_overlays(viewer_annotations);
        if (!overlays_foreground) append_overlays(overlays);
        if (include_hud) {
            append_overlays(viewer_3d_overlays);
            append_overlays(viewer_overlays);
        }
        if (overlays_foreground) append_overlays(overlays);
        return commands;
    }
    graphics::FrameImage capture(const Camera& cam, const vec4& bg, unsigned samples = 1,
                                 int width = 0, int height = 0, bool include_hud = true) const {
        if (!width) width = cam.screenWidth(); if (!height) height = cam.screenHeight();
        return device->render(uint32_t(width), uint32_t(height), commands(cam,width,height,nullptr,include_hud), bg, samples);
    }
};
std::vector<graphics::DrawCommand> Viewer::draw_commands(const Camera& cam, int, int,
                                                       std::vector<DrawSource>* sources, bool include_hidden_models) const {
        impl_->drawable_renderer.prune();
        std::vector<graphics::DrawCommand> commands;
        const auto& planes = impl_->active_clipping_planes();
        auto append=[&](const std::shared_ptr<Drawable>& d,uint32_t id,const std::shared_ptr<Model>& model,bool selected) {
            if (!d->is_visible()) return;
            auto command=impl_->drawable_renderer.command(d,id,cam,planes,selected);
            if (sources) {
                if (sources->size()>=UINT32_MAX) throw std::length_error("too many picking sources");
                id=uint32_t(sources->size()); sources->push_back({model,d});
                command.object_id=id;
            }
            commands.push_back(std::move(command));
        };
        for (size_t i = 0; i < models_.size(); ++i) {
            const auto* renderer = models_[i]->renderer();
            if (!renderer || (!include_hidden_models && !renderer->is_visible())) continue;
            for (const auto& d : renderer->triangles_drawables())
                append(d,uint32_t(i),models_[i],renderer->is_selected());
            for (const auto& d : renderer->lines_drawables())
                append(d,uint32_t(i),models_[i],renderer->is_selected());
            for (const auto& d : renderer->points_drawables())
                append(d,uint32_t(i),models_[i],renderer->is_selected());
        }
        for (size_t i = 0; i < drawables_.size(); ++i)
            append(drawables_[i],uint32_t(models_.size()+i),nullptr,false);
        const auto guide = impl_->clipping_plane_renderer.draw(*ClippingPlane::instance(),cam);
        for (auto command : guide) {
            command.object_id = UINT32_MAX; commands.push_back(std::move(command));
        }
        return commands;
}
Viewer::Viewer(int width, int height, const VulkanOptions& options)
    : window_(nullptr), width_(width), height_(height), camera_(new Camera),
      samples_(1), background_color_(setting::background_color), pressed_key_(-1),
      show_frame_rate_(false), show_camera_path_(false), show_easy3d_logo_(false), model_idx_(-1),
      impl_(new Impl(*this)) {
    resize_render_target(width, height);
    impl_->device = graphics::create_vulkan_device(options.validation, options.sync_validation, options.device_name);
    camera()->setType(Camera::PERSPECTIVE);
    camera()->setUpVector(vec3(0,0,1)); camera()->setViewDirection(vec3(-1,0,0)); camera()->showEntireScene();
    // Transparency effects only know the camera. Rendering and presentation stay
    // in the viewer, as in Viewer::draw().
    Transparency::set_frame_renderer(camera(), [this](const std::vector<TrianglesDrawable*>& surfaces, int method) {
        draw_transparency(surfaces, method);
    });
}
void Viewer::release_device() { impl_->device.reset(); }
void Viewer::resize_render_target(int w, int h) {
    if (w <= 0 || h <= 0) throw std::invalid_argument("offscreen dimensions must be positive");
    render_width_ = w; render_height_ = h;
    camera()->setScreenWidthAndHeight(w, h);
    if (!window_) { width_ = w; height_ = h; }

}
Model* Viewer::add_model(std::shared_ptr<Model> model, bool create) {
    return add_model(std::move(model),create,true);
}
Model* Viewer::add_model(std::shared_ptr<Model> model, bool create, bool fit_scene) {
    if (!model || model->empty()) return nullptr;
    for (const auto& current : models_) if (model == current) return current.get();
    // Construct first: unsupported default coloring fails without replacing the model's existing renderer.
    auto renderer = std::make_shared<Renderer>(model.get(), create);
    model->set_renderer(renderer);
    models_.push_back(model);
    model_idx_ = int(models_.size())-1;
    if (fit_scene) fit_screen();
    return model.get();
}
Model* Viewer::add_model(const std::string& filename, bool create) {
    const auto ext = file_system::extension(filename, true);
    std::shared_ptr<Model> model;
    if (ext == "ply" && io::PlyReader::num_instances(filename, "face") > 0)
        model.reset(SurfaceMeshIO::load(filename));
    else if (ext == "ply" && io::PlyReader::num_instances(filename, "edge") > 0)
        model.reset(GraphIO::load(filename));
    else if (ext == "obj" || ext == "off" || ext == "stl" || ext == "sm" || ext == "geojson" || ext == "trilist")
        model.reset(SurfaceMeshIO::load(filename));
    else if (ext == "plm" || ext == "pm" || ext == "mesh") model.reset(PolyMeshIO::load(filename));
    else if (ext == "ptx") throw std::logic_error("multi-scan PTX loading is not yet ported to Viewer");
    else model.reset(PointCloudIO::load(filename));
    if (model) model->set_name(filename);
    return add_model(model, create);
}
Model* Viewer::add_model(Model* model, bool create) {
    if (!model) return nullptr;
    // Do not manufacture a second shared_ptr control block for an owned model.
    for (const auto& current : models_) if (current.get() == model) return model;
    return add_model(std::shared_ptr<Model>(model), create);
}
bool Viewer::delete_model(Model* model) {
    auto& values = models_;
    auto found = std::find_if(values.begin(), values.end(), [model](const std::shared_ptr<Model>& p) { return p.get() == model; });
    if (found == values.end()) return false;
    values.erase(found); model_idx_ = int(models_.size())-1; impl_->drawable_renderer.clear(); return true;
}
Model* Viewer::current_model() const {
    return model_idx_ < 0 || size_t(model_idx_) >= models_.size() ? nullptr : models_[model_idx_].get();
}
Drawable* Viewer::add_drawable(std::shared_ptr<Drawable> drawable) {
    if (!drawable) return nullptr;
    for (const auto& d : drawables_) if (d == drawable) return d.get();
    drawables_.push_back(drawable); return drawable.get();
}
bool Viewer::delete_drawable(Drawable* drawable) {
    auto& values = drawables_;
    auto found = std::find_if(values.begin(), values.end(), [drawable](const std::shared_ptr<Drawable>& p) { return p.get() == drawable; });
    if (found == values.end()) return false;
    values.erase(found); impl_->drawable_renderer.clear(); return true;
}
void Viewer::clear_scene() {
    models_.clear(); model_idx_ = -1; drawables_.clear(); impl_->drawable_renderer.clear(); impl_->overlays.clear(); impl_->viewer_overlays.clear(); impl_->viewer_3d_overlays.clear(); impl_->viewer_annotations.clear();
    if (window_) { cancel_interaction(); update_camera_path_overlays(); }
}
void Viewer::set_overlays(std::vector<graphics::DrawCommand> commands, bool foreground) {
    for (auto& command : commands) command.overlay = true;
    impl_->overlays = std::move(commands);
    impl_->overlays_foreground = foreground;
}
void Viewer::set_viewer_overlays(std::vector<graphics::DrawCommand> commands) const {
    for (auto& command : commands) { command.overlay = true; command.overlay_depth_test = command.overlay_independent_depth = false; }
    impl_->viewer_overlays = std::move(commands);
}
void Viewer::set_viewer_3d_overlays(std::vector<graphics::DrawCommand> commands) const {
    for (auto& command : commands) { command.overlay = command.overlay_independent_depth = true; command.overlay_depth_test = false; }
    impl_->viewer_3d_overlays = std::move(commands);
}
void Viewer::set_viewer_annotations(std::vector<graphics::DrawCommand> commands) const {
    for (auto& command : commands) { command.overlay = command.overlay_depth_test = true; command.overlay_independent_depth = false; }
    impl_->viewer_annotations = std::move(commands);
}
void Viewer::set_clipping_planes(const std::vector<vec4>& planes) {
    impl_->clipping_planes=graphics::normalize_clipping_planes(planes);
}
const std::vector<vec4>& Viewer::clipping_planes() const { return impl_->active_clipping_planes(); }
void Viewer::fit_screen(const Model* selected) {
    Box3 box;
    if (selected) box.grow(selected->bounding_box());
    else {
        for (const auto& model : models_) box.grow(model->bounding_box());
        for (const auto& drawable : drawables_)
            if (!dynamic_cast<LinesDrawable2D*>(drawable.get())) box.grow(drawable->bounding_box());
    }
    if (box.is_valid()) {
        camera()->setSceneBoundingBox(box.min_point(), box.max_point()); camera()->showEntireScene();
        ClippingPlane::instance()->fit_scene(camera()->sceneCenter(),camera()->sceneRadius());
    }
}
std::vector<vec3> Viewer::scan_visible_points(unsigned samples) const {
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
Viewer::PickResult Viewer::pick(int x,int y) const {
    PickResult result;
    if (x<0 || y<0 || x>=framebuffer_width() || y>=framebuffer_height()) return result;
    // Keep commands and source mapping together for this exact synchronous draw.
    std::vector<DrawSource> sources;
    const int width = framebuffer_width(), height = framebuffer_height();
    const auto commands = draw_commands(*camera(), width, height, &sources);
    const auto sample = impl_->device->sample_pixel(width, height, commands, x, y);
    const auto id = sample.id;
    if (id.object >= sources.size() || sample.depth >= 1) return result;
    result.model=sources[id.object].model; result.drawable=sources[id.object].drawable;
    result.primitive=id.primitive; result.depth=sample.depth;
    result.position=unproject_pixel(x,y,result.depth);
    const auto& faces=result.drawable->cpu_buffers().source_faces;
    auto* mesh=dynamic_cast<SurfaceMesh*>(result.model.get());
    if (mesh && id.primitive<faces.size()) {
        const SurfaceMesh::Face f(faces[id.primitive]);
        if (mesh->is_valid(f) && !mesh->is_deleted(f)) result.face_index=f.idx();
    }
    return result;
}
bool Viewer::read_scene_depth(int x,int y,const Camera& snapshot,float& depth) const {
    const int width = snapshot.screenWidth(), height = snapshot.screenHeight();
    const auto commands = draw_commands(snapshot, width, height);
    const auto sample = impl_->device->sample_pixel(width, height, commands, x, y);
    depth = sample.depth;
    return sample.id.object != UINT32_MAX;
}
const graphics::RenderDevice& Viewer::device() const { return *impl_->device; }
std::vector<std::uint32_t> Viewer::select_points(const graphics::PointSelectionRequest& request) const {
    return impl_->device->select_points(request);
}
std::shared_ptr<const graphics::PointData> Viewer::point_resource(Drawable* drawable) const {
    if (!drawable || drawable->type()!=Drawable::DT_POINTS) throw std::invalid_argument("selection requires a point drawable");
    for (const auto& model:models_) if (model->renderer())
        for (const auto& d:model->renderer()->points_drawables()) if (d.get()==drawable)
            return impl_->drawable_renderer.command(d,0,*camera(),impl_->clipping_planes,model->renderer()->is_selected()).geometry->point_data();
    for (const auto& d:drawables_) if (d.get()==drawable)
        return impl_->drawable_renderer.command(d,0,*camera(),impl_->clipping_planes).geometry->point_data();
    throw std::invalid_argument("point drawable does not belong to this viewer");
}
bool Viewer::render(const std::string& filename, float scaling, int samples, int bg, bool expand) const {
    try {
        if (samples <= 0) throw std::invalid_argument("sample count must be positive");
        if (!std::isfinite(scaling) || scaling <= 0 || double(framebuffer_width()) * scaling > std::numeric_limits<int>::max() ||
            double(framebuffer_height()) * scaling > std::numeric_limits<int>::max() || bg < 0 || bg > 2)
            throw std::invalid_argument("invalid snapshot scaling/background");
        Camera copy(*camera());
        const int w = int(std::ceil(framebuffer_width() * double(scaling))), h = int(std::ceil(framebuffer_height() * double(scaling)));
        if (w <= 0 || h <= 0) throw std::invalid_argument("scaled snapshot dimensions are zero");
        copy.setScreenWidthAndHeight(w, h);
        copy.projectionMatrix(); // resolve resize invalidation before overriding the matrix
        // Match snapshot.cpp: preserve one source frustum dimension, expanding
        // or cropping the other to accommodate the rounded output aspect ratio.
        const double aspect = double(framebuffer_width()) / framebuffer_height(), output_aspect = double(w) / h;
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
        auto frame = impl_->capture(copy, bg == 0 ? background_color() : bg == 1 ? vec4(1, 1, 1, 1) : vec4(0, 0, 0, 0), unsigned(samples), w, h, snapshot_hud_);
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

// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/viewer/backends/glfw_runtime.h>
#include <easy3d/renderer/backends/vulkan/surface.h>
#include <easy3d/viewer/backends/viewer_loop.h>
#include <easy3d/renderer/text_layout.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/backends/vulkan/key_frame_interpolator.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/file_system.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/graph.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/graph_io.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <exception>
#include <stdexcept>

namespace easy3d {
unsigned Viewer::validation_errors() const { return device().validation_errors(); }
void Viewer::init() {}
void Viewer::pre_draw() {}
void Viewer::post_draw() {}
bool Viewer::draw() {
    const int width = framebuffer_width(), height = framebuffer_height();
    auto commands = impl_->commands(*camera(), width, height);
    float radius, bias;
    mat4 projection;
    if (AmbientOcclusion::frame_params(camera(), radius, bias, projection))
        commands.insert(commands.begin(),
                        graphics::shape::draw_ambient_occlusion(unsigned(width), unsigned(height),
                                                                radius, bias, projection));
    float z_near, z_far, scene_size;
    if (EyeDomeLighting::frame_params(camera(), z_near, z_far, scene_size))
        commands.insert(commands.begin(),
                        graphics::shape::draw_eye_dome_lighting(unsigned(width), unsigned(height),
                                                                z_near, z_far, scene_size));
    return impl_->device->present(uint32_t(width), uint32_t(height), commands,
                                  background_color(), unsigned(samples()));
}
void Viewer::draw_transparency(const std::vector<TrianglesDrawable*>& surfaces, int) {
    const int width = framebuffer_width(), height = framebuffer_height();
    std::vector<DrawSource> sources;
    auto commands = impl_->commands(*camera(), width, height, &sources);
    for (size_t i = 0; i < sources.size() && i < commands.size(); ++i) {
        auto* triangles = dynamic_cast<TrianglesDrawable*>(sources[i].drawable.get());
        if (!triangles || std::find(surfaces.begin(), surfaces.end(), triangles) == surfaces.end()) continue;
        if (commands[i].geometry && commands[i].geometry->topology() == graphics::Topology::triangles) {
            commands[i].transparent = true;
            commands[i].color.w = triangles->opacity();
        }
    }
    impl_->device->present(uint32_t(width), uint32_t(height), commands, background_color(), unsigned(samples()));
}
bool Viewer::mouse_free_move_event(int, int, int, int, int) { return false; }
bool Viewer::char_input_event(unsigned int) { return false; }
bool Viewer::focus_event(bool) { return false; }
void Viewer::exit() { request_close(); }
void Viewer::cleanup() { release_device(); }
namespace {
const char* const default_manual =
    "--------------------------------------------------------------\n"
    " Easy3D Viewer Usage (Shortcuts might be overridden):         \n"
    "--------------------------------------------------------------\n"
    "  F1:                      Show help                                          \n"
    "  F2:                      Toggle animation (if available)                    \n"
    "  Ctrl + 'o':              Open file                                          \n"
    "  Ctrl + 's':              Save file                                          \n"
    "  Fn + Delete:             Delete current model                               \n"
    "  '<' or '>':              Switch between models                              \n"
    "  's':                     Take a snapshot                                    \n"
    "  'p':                     Switch between perspective and orthographic projections\n"
    "  Ctrl + 'f':              Toggle frame rate display                          \n"
    "  'a':                     Toggle axes visibility                             \n"
    "  Alt + 'c':               Toggle clipping plane                              \n"
    "  Alt + 'v':               Toggle clipping plane visibility                   \n"
    "  Alt + 'x':               Toggle cross-section                               \n"
    "  Alt + Left drag:         Rotate the clipping plane                          \n"
    "  Alt + Right drag:        Move the clipping plane along its normal           \n"
    "  Left drag:               Rotate the camera                                  \n"
    "  Right drag:              Move the camera                                    \n"
    "  'x' + Left drag:         Rotate the camera around the horizontal axis       \n"
    "  'x' + Right drag:        Move the camera along the horizontal axis          \n"
    "  'y' + Left drag:         Rotate the camera around the vertical axis         \n"
    "  'y' + Right drag:        Move the camera along the vertical axis            \n"
    "  'o' + Left drag:         Rotate the camera around the ortho-screen axis     \n"
    "  Middle button or Wheel:  Zoom in/out                                        \n"
    "  Ctrl + '+'/'-':          Zoom in/out                                        \n"
    "  Left/Right Arrow:        Turn camera left/right                             \n"
    "  Ctrl + Left/Right Arrow: Move camera left/right                             \n"
    "  Up/Down Arrow:           Move camera forward/backward                       \n"
    "  Ctrl + Up/Down Arrow:    Move camera up/down                                \n"
    "  Ctrl + 'c':              Copy current view status to clipboard              \n"
    "  Ctrl + 'v':              Restore view status from clipboard                 \n"
    "  Alt + 'k':               Add key frame to the camera path                   \n"
    "  Alt + 'd':               Delete the camera path                             \n"
    "  Ctrl + 'k':              Play/pause the camera path                         \n"
    "  Ctrl + 't':              Toggle camera path visibility                      \n"
    "  'f':                     Fit screen (all models)                            \n"
    "  'c':                     Fit screen (current model only)                    \n"
    "  'z' + Left click:        Zoom to target point on model                      \n"
    "  'z' + Right click:       Zoom to fit screen                                 \n"
    "  Shift + Left click:      Define a target point on model                     \n"
    "  Shift + Right click:     Undefine the target point (if any) on model        \n"
    "  Ctrl + Left drag:        Zoom to fit region                                 \n"
    "  '-'/'=':                 Decrease/Increase point size                       \n"
    "  '['/']':                 Decrease/Increase line width                       \n"
    "  'b':                     Toggle borders visibility                          \n"
    "  'e':                     Toggle edges visibility                            \n"
    "  'v':                     Toggle vertices visibility                         \n"
    "  'l':                     Toggle locked vertices visibility                  \n"
    "  'm':                     Toggle smooth shading (for SurfaceMesh)            \n"
    "  'd':                     Print model properties                             \n"
    "  Space:                   Align camera with world axes                       \n"
    "  Escape / Alt + F4:       Close viewer                                       \n"
    "-------------------------------------------------------------------";

}
struct Viewer::Window {
    std::shared_ptr<detail::GlfwRuntime> runtime;
    GLFWwindow* handle = nullptr;
    std::string title;
    int width, height;
    unsigned samples;
    int button = -1, last_x = 0, last_y = 0;
    int last_logical_x = 0, last_logical_y = 0;
    double wheel_x = 0, wheel_y = 0;
    std::exception_ptr error;
    Window(const std::string& text, int w, int h, int sample_count, bool full_screen, bool visible, bool resizable)
        : title(text), width(w), height(h), samples(sample_count ? sample_count : 1) {   // 0: no MSAA, as in OpenGL
        if (w <= 0 || h <= 0)
            throw std::invalid_argument("window dimensions must be positive");
        if (sample_count < 0 || sample_count > 64 || (sample_count && (sample_count & (sample_count - 1))))
            throw std::invalid_argument("invalid viewer sample count");
        runtime = detail::GlfwRuntime::acquire(true);
        if (!glfwVulkanSupported()) throw std::runtime_error("GLFW could not find Vulkan loader/driver support");
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
        GLFWmonitor* monitor = nullptr;
        if (full_screen) {
            monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = monitor ? glfwGetVideoMode(monitor) : nullptr;
            if (!mode) throw std::runtime_error("no monitor available for a full-screen window");
            width = mode->width; height = mode->height;
        }
        handle = glfwCreateWindow(width, height, title.c_str(), monitor, nullptr);
        if (!handle) throw std::runtime_error("could not create a GLFW Vulkan window");
    }
    ~Window() { if (handle) glfwDestroyWindow(handle); }
    std::unique_ptr<graphics::RenderDevice> device(const VulkanOptions& options) {
        uint32_t count = 0;
        const char** required = glfwGetRequiredInstanceExtensions(&count);
        if (!required || !count) throw std::runtime_error("GLFW has no Vulkan surface extensions for this platform");
        graphics::VulkanSurface surface;surface.loader=runtime->vulkan_loader();
        for (uint32_t i = 0; i < count; ++i) surface.instance_extensions.emplace_back(required[i]);
        surface.create = [this](VkInstance instance, VkSurfaceKHR* result) {
            return glfwCreateWindowSurface(instance, handle, nullptr, result);
        };
        return graphics::create_vulkan_window_device(surface, options.validation, options.sync_validation, options.device_name);
    }
    void position(double px, double py, int& x, int& y) const {
        int ww = 0, wh = 0, fw = 0, fh = 0;
        glfwGetWindowSize(handle, &ww, &wh); glfwGetFramebufferSize(handle, &fw, &fh);
        x = ww > 0 ? int(std::lround(px * fw / ww)) : 0;
        y = wh > 0 ? int(std::lround(py * fh / wh)) : 0;
    }
    void position(int& x, int& y) const {
        double px = 0, py = 0;
        glfwGetCursorPos(handle, &px, &py); position(px, py, x, y);
    }
    int modifiers() const { return detail::GlfwRuntime::modifiers(handle); }
    template<class Function> static void dispatch(GLFWwindow* handle, const Function& call) noexcept {
        auto* viewer = static_cast<Viewer*>(glfwGetWindowUserPointer(handle));
        if (!viewer) return;
        try { call(*viewer); }
        catch (...) { viewer->window_host_->error = std::current_exception(); glfwSetWindowShouldClose(handle, GLFW_TRUE); }
    }
    void callbacks(Viewer* viewer) {
        glfwSetWindowUserPointer(handle, viewer);
        glfwSetWindowRefreshCallback(handle, [](GLFWwindow* w) { dispatch(w, [](Viewer& v) { v.update(); }); });
        glfwSetCursorPosCallback(handle, [](GLFWwindow* w, double px, double py) { dispatch(w, [&](Viewer& v) {
            int x, y; auto& host = *v.window_host_; host.position(px, py, x, y);
            const int dx = x - host.last_x, dy = y - host.last_y;
            host.last_x = x; host.last_y = y;
            const int wx = int(px), wy = int(py), ldx = wx - host.last_logical_x, ldy = wy - host.last_logical_y;
            host.last_logical_x = wx; host.last_logical_y = wy;
            const int mods = host.modifiers();
            if (host.button >= 0) {
                if (!v.dispatch_mouse_drag(wx, wy, ldx, ldy, host.button, mods)) v.mouse_drag_event(x, y, dx, dy, host.button, mods);
            } else if (!v.dispatch_mouse_move(wx, wy, ldx, ldy, mods)) v.mouse_free_move_event(x, y, dx, dy, mods);
        }); });
        glfwSetMouseButtonCallback(handle, [](GLFWwindow* w, int button, int action, int mods) { dispatch(w, [&](Viewer& v) {
            int x, y; auto& host = *v.window_host_;
            double px, py; glfwGetCursorPos(w, &px, &py); host.position(px, py, x, y);
            host.last_x = x; host.last_y = y; host.last_logical_x = int(px); host.last_logical_y = int(py);
            if (action == GLFW_PRESS) {
                host.button = button;
                if (!v.dispatch_mouse_press(int(px), int(py), button, mods)) v.mouse_press_event(x, y, button, mods);
            } else {
                if (!v.dispatch_mouse_release(int(px), int(py), button, mods)) v.mouse_release_event(x, y, button, mods);
                else {
                    v.pressed_button_ = -1; v.zoom_region_ = false;
                    v.camera()->frame()->action_end();
                    ClippingPlane::instance()->manipulator()->frame()->action_end();
                    v.update_interaction_overlays(true);
                }
                if (host.button == button) host.button = -1;
            }
        }); });
        glfwSetScrollCallback(handle, [](GLFWwindow* w, double dx, double dy) { dispatch(w, [&](Viewer& v) {
            auto& host = *v.window_host_; host.wheel_x += dx; host.wheel_y += dy;
            const int sx = int(host.wheel_x), sy = int(host.wheel_y);
            host.wheel_x -= sx; host.wheel_y -= sy;
            int x, y; double px, py; glfwGetCursorPos(w, &px, &py); host.position(px, py, x, y);
            if ((sx || sy) && !v.dispatch_mouse_scroll(int(px), int(py), sx, sy)) v.mouse_scroll_event(x, y, sx, sy);
        }); });
        glfwSetKeyCallback(handle, [](GLFWwindow* w, int key, int, int action, int mods) { dispatch(w, [&](Viewer& v) {
            if (action == GLFW_RELEASE) {
                if (!v.dispatch_key_release(key, mods)) v.key_release_event(key, mods);
                else v.pressed_key_ = -1;
            } else if (!v.dispatch_key_press(key, mods)) v.key_press_event(key, mods);
        }); });
        glfwSetCharCallback(handle, [](GLFWwindow* w, unsigned int code) { dispatch(w, [&](Viewer& v) { if (!v.dispatch_character(code)) v.char_input_event(code); }); });
        glfwSetDropCallback(handle, [](GLFWwindow* w, int count, const char** files) { dispatch(w, [&](Viewer& v) {
            std::vector<std::string> names;
            for (int i = 0; i < count; ++i) names.emplace_back(files[i]);
            if (!v.dispatch_drop(names)) v.drop_event(names);
        }); });
        glfwSetWindowFocusCallback(handle, [](GLFWwindow* w, int focused) { dispatch(w, [&](Viewer& v) {
            if (!focused) {
                v.window_host_->button = -1; v.window_host_->wheel_x = v.window_host_->wheel_y = 0; v.cancel_interaction();
            }
            if (!v.dispatch_focus(focused != 0)) v.focus_event(focused != 0);
        }); });
        glfwSetFramebufferSizeCallback(handle, [](GLFWwindow* w, int width, int height) { dispatch(w, [&](Viewer& v) {
            glfwGetWindowSize(w, &v.width_, &v.height_);
            if (width > 0 && height > 0) { v.resize_render_target(width, height); v.cancel_interaction(); v.post_resize(v.width_, v.height_); }
            v.notify_resize();
        }); });
    }
};
Viewer::Viewer(const std::string& title, int width, int height, int samples,
                           bool full_screen, bool visible, bool resizable, const VulkanOptions& vk_options)
    : Viewer(std::make_shared<Window>(title, width, height, samples, full_screen, visible, resizable), vk_options) {}
Viewer::Viewer(std::shared_ptr<Window> host, const VulkanOptions& options)
    : window_(host->handle), width_(host->width), height_(host->height), title_(host->title),
      camera_(new Camera), samples_(1), background_color_(setting::background_color), pressed_key_(-1),
      show_frame_rate_(false), show_camera_path_(false), show_easy3d_logo_(true), model_idx_(-1),
      window_host_(std::move(host)), impl_(new Impl(*this)) {
    try {
        impl_->device = window_host_->device(options);
        manual_ = default_manual;
        kfi_.reset(new KeyFrameInterpolator(camera()->frame()));
        camera_path_renderer_.reset(new vulkan::KeyFrameInterpolatorRenderer);
        {   // The default of 4 samples must not fail on a device without 4x MSAA: step a valid
            // power of two down to a supported count with a warning (as OpenGL does); invalid
            // counts are still rejected by set_samples().
            unsigned s = window_host_->samples;
            const unsigned supported = device().supported_sample_counts();
            if (s > 1 && s <= 64 && !(s & (s - 1)))
                while (s > 1 && !(supported & s)) s >>= 1;
            if (s != window_host_->samples) {
                LOG(WARNING) << "MSAA with " << window_host_->samples << " samples is not supported, using " << s;
                window_host_->samples = s;
            }
        }
        set_samples(window_host_->samples);
        window_host_->callbacks(this);
        int w, h; framebuffer_size(w, h);
        if (w > 0 && h > 0) resize_render_target(w, h);
        camera()->setUpVector(vec3(0,0,1)); camera()->setViewDirection(vec3(-1,0,0));
        camera()->showEntireScene();
        // Transparency effects only know the camera, so route their frames back to
        // this viewer for rendering and presentation.
        Transparency::set_frame_renderer(camera(), [this](const std::vector<TrianglesDrawable*>& surfaces, int method) {
            draw_transparency(surfaces, method);
        });
        VLOG(1) << "Vulkan device: " << impl_->device->name() <<  ", Samples: " << window_host_->samples;
    } catch (...) {
        // A failed derived constructor destroys members before base classes.
        // Release the Vulkan surface before member-owned native-window cleanup.
        glfwSetWindowUserPointer(window_host_->handle, nullptr);
        Transparency::set_frame_renderer(camera(), nullptr);
        release_device();
        throw;
    }
}
Viewer::~Viewer() {
    if (kfi_) kfi_->stop_interpolation();
    // Base classes are destroyed after members. Release the surface/device now,
    // while the member-owned GLFW window (and its Metal layer on macOS) still live.
    if (window_) glfwSetWindowUserPointer(window_, nullptr);
    Transparency::set_frame_renderer(camera(), nullptr);
    release_device();
}
void Viewer::poll_events() { ViewerBase::poll_events(); }
void Viewer::request_close() { glfwSetWindowShouldClose(window_host_->handle, GLFW_TRUE); }
bool Viewer::should_close() const { return glfwWindowShouldClose(window_host_->handle) != 0; }
void Viewer::show() { glfwShowWindow(window_host_->handle); }
void Viewer::hide() { glfwHideWindow(window_host_->handle); }
void Viewer::resize(int w, int h) {
    if (w <= 0 || h <= 0) throw std::invalid_argument("window dimensions must be positive");
    glfwSetWindowSize(window_host_->handle, w, h);
}
void Viewer::framebuffer_size(int& w, int& h) const {
    if (window_) glfwGetFramebufferSize(window_, &w, &h);
    else { w = camera()->screenWidth(); h = camera()->screenHeight(); }
}
int Viewer::framebuffer_width() const { int w, h; framebuffer_size(w, h); return w; }
int Viewer::framebuffer_height() const { int w, h; framebuffer_size(w, h); return h; }
void Viewer::viewer_size(int& w, int& h) const {
    if (window_) glfwGetWindowSize(window_, &w, &h); else { w = width_; h = height_; }
}
void Viewer::set_position(int x, int y) { glfwSetWindowPos(window_host_->handle, x, y); }
float Viewer::dpi_scaling() const {
    if (!window_) return 1.0f;
    int w=0,h=0;glfwGetWindowSize(window(),&w,&h);
    return w>0 ? float(framebuffer_width())/w : 1.0f;
}
void Viewer::set_title(const std::string& text) { title_ = text; window_host_->title = text; glfwSetWindowTitle(window_host_->handle, text.c_str()); }
void Viewer::update() const { glfwPostEmptyEvent(); }
void Viewer::set_animation(bool enabled) { animation_enabled_ = enabled; update(); }
bool Viewer::is_animating() const { return animation_enabled_ && bool(animation_func_); }
void Viewer::set_samples(unsigned samples) {
    if (!samples || samples > 64 || (samples & (samples - 1)) || !(device().supported_sample_counts() & samples))
        throw std::invalid_argument("unsupported Vulkan viewer sample count");
    window_host_->samples = samples; samples_ = int(samples);
}
bool Viewer::draw_frame() {
    if (!window_) return false;
    if (window_host_->error) std::rethrow_exception(window_host_->error);
    if (should_close() || glfwGetWindowAttrib(window_host_->handle, GLFW_ICONIFIED)) return false;
    int w, h; framebuffer_size(w, h);
    if (w <= 0 || h <= 0) return false;
    if (render_width_ != w || render_height_ != h) { resize_render_target(w, h); cancel_interaction(); viewer_size(width_, height_); post_resize(width_, height_); notify_resize(); }
    if (!initialized_) { init(); initialized_ = true; }
    if (is_animating()) {
        animation_func_(this);
        if (window_host_->error) std::rethrow_exception(window_host_->error);
        if (should_close()) return false;
    }
    advance_camera_path();
    print_usage();
    notify_pre_draw();
    if (window_host_->error) std::rethrow_exception(window_host_->error);
    if (should_close()) return false;
    pre_draw();
    update_camera_path_overlays();
    update_interaction_overlays();
    const bool drawn = draw();
    if (drawn) {update_frame_rate();post_draw();notify_post_draw();}
    if (window_host_->error) std::rethrow_exception(window_host_->error);
    return drawn;
}

void Viewer::update_camera_path_overlays() const {
    std::vector<graphics::DrawCommand> commands;
    if (show_camera_path_ && !kfi_->is_interpolation_started()) {
        auto path=camera_path_renderer_->draw_path(*kfi_,*camera());
        auto cameras=camera_path_renderer_->draw_cameras(*kfi_,*camera(),camera()->sceneRadius()*.05f);
        if (path.geometry) commands.push_back(path);
        if (cameras.geometry) commands.push_back(cameras);
    }
    set_viewer_annotations(std::move(commands));
}
void Viewer::restore_scene_bounds() {
    Box3 box;
    for (const auto& model:models()) box.grow(model->bounding_box());
    for (const auto& drawable:drawables())
        if (!dynamic_cast<LinesDrawable2D*>(drawable.get())) box.grow(drawable->bounding_box());
    if (box.is_valid()) camera()->setSceneBoundingBox(box.min_point(),box.max_point());
}
void Viewer::update_frame_rate() {
    if (!show_frame_rate_) return;
    ++frame_rate_frames_;
    const auto now=std::chrono::steady_clock::now();
    const double elapsed=std::chrono::duration<double>(now-frame_rate_start_).count();
    if (elapsed<1.0) return;
    std::ostringstream out;
    out << "fps: " << std::fixed << std::setprecision(0) << frame_rate_frames_/elapsed
        << " (" << std::setprecision(1) << 1000.0*elapsed/frame_rate_frames_ << " ms/frame)";
    framerate_=out.str();frame_rate_frames_=0;frame_rate_start_=now;
}
void Viewer::ensure_text_renderer() const {
    if (fonts_tried_) return;
    fonts_tried_=true;
    try {
        texter_.reset(new graphics::TextLayoutEngine);
        texter_->add_font(resource::directory()+"/fonts/en_Earth-Normal.ttf");
        texter_->add_font(resource::directory()+"/fonts/en_Roboto-Medium.ttf");
    } catch (const std::exception& e) {
        texter_.reset();LOG(WARNING) << "Viewer HUD font unavailable: " << e.what();
    }
}
void Viewer::draw_corner_axes() const {
    if (!show_corner_axes_) { set_viewer_3d_overlays({}); return; }
    if (!corner_axes_geometry_) {
        // Same cylinders, cones, sphere and RGB colors as Viewer.
        std::vector<vec3> points,normals,colors;
        for (const auto& axis : {vec3(1,0,0),vec3(0,1,0),vec3(0,0,1)}) {
            shape::create_cylinder(.03f,10,vec3(0),.5f*axis,axis,points,normals,colors);
            shape::create_cone(.06f,20,.5f*axis,.7f*axis,axis,points,normals,colors);
        }
        shape::create_sphere(vec3(0),.06f,20,20,vec3(0,1,1),points,normals,colors);
        std::vector<graphics::Vertex> vertices(points.size());
        for (size_t i=0;i<points.size();++i) {
            vertices[i].position=points[i]; vertices[i].normal=normals[i]; vertices[i].color=vec4(colors[i],1);
        }
        corner_axes_geometry_=std::make_shared<graphics::Geometry>(std::move(vertices));
    }
    const float dpi=dpi_scaling();
    const float sx=100*dpi/framebuffer_width(),sy=100*dpi/framebuffer_height();
    const mat4 view=camera()->orientation().inverse().matrix();
    const mat4 viewport=mat4::translation(vec3(-1+sx,-1+sy,0))*mat4::scale(sx,sy,1,1);
    graphics::DrawCommand axes; axes.geometry=corner_axes_geometry_;
    axes.model_view_projection=viewport*transform::ortho(-1,1,-1,1,-1,1)*view;
    axes.normal_matrix=mat3(view); axes.lighting=true;
    set_viewer_3d_overlays({axes});
}
void Viewer::update_interaction_overlays(bool force) const {
    draw_corner_axes();
    const bool pivot = show_pivot_point_ && std::chrono::steady_clock::now()<pivot_expires_;
    const float dpi=dpi_scaling();
    const bool viewport_changed=hint_dpi_!=dpi || framebuffer_width()!=hint_width_ || framebuffer_height()!=hint_height_ || rendered_show_logo_!=show_easy3d_logo_;
    const bool hint_changed=viewport_changed || rendered_hint_!=hint_ || rendered_show_frame_rate_!=show_frame_rate_;
    const std::string frame_rate_text=show_frame_rate_?framerate_:std::string();
    const bool frame_rate_changed=viewport_changed || rendered_frame_rate_!=frame_rate_text;
    if (!force && !hint_changed && !frame_rate_changed && pivot==pivot_hint_visible_) return;
    if (show_easy3d_logo_ || !hint_.empty() || !frame_rate_text.empty()) ensure_text_renderer();
    if (viewport_changed || force) {
        logo_commands_.clear();
        if (texter_ && show_easy3d_logo_) {
            graphics::TextOptions options;options.size=15*dpi;options.x=20*dpi;options.y=22.5f*dpi;
            logo_commands_=texter_->layout("Easy3D",framebuffer_width(),framebuffer_height(),options).commands;
        }
        rendered_show_logo_=show_easy3d_logo_;
    }
    if (hint_changed) {
        hint_commands_.clear();
        if (texter_ && !hint_.empty()) {
            graphics::TextOptions options;
            options.font=1;options.size=15*dpi;options.x=35*dpi;options.y=22.5f*(1+int(show_frame_rate_)+int(show_easy3d_logo_))*dpi;
            options.color=vec4(0,0,1,1);
            hint_commands_=texter_->layout(hint_,framebuffer_width(),framebuffer_height(),options).commands;
        }
        rendered_hint_=hint_;hint_dpi_=dpi;rendered_show_frame_rate_=show_frame_rate_;
    }
    if (frame_rate_changed) {
        frame_rate_commands_.clear();
        if (texter_ && !frame_rate_text.empty()) {
            graphics::TextOptions options;options.font=1;options.size=15*dpi;options.x=20*dpi;options.y=22.5f*(1+int(show_easy3d_logo_))*dpi;
            frame_rate_commands_=texter_->layout(frame_rate_text,framebuffer_width(),framebuffer_height(),options).commands;
        }
        rendered_frame_rate_=frame_rate_text;
    }
    std::vector<graphics::DrawCommand> commands=logo_commands_;
    commands.insert(commands.end(),frame_rate_commands_.begin(),frame_rate_commands_.end());
    commands.insert(commands.end(),hint_commands_.begin(),hint_commands_.end());
    if (pivot) {
        const float x=pivot_pixel_.x,y=pivot_pixel_.y,size=10*dpi;
        commands.push_back(graphics::shape::draw_lines_2d({vec2(x-size,y),vec2(x+size,y),
                         vec2(x,y-size),vec2(x,y+size)},vec4(0,0,0,1),framebuffer_width(),framebuffer_height(),dpi));
    }
    if (zoom_region_ && (mouse_current_x_!=mouse_pressed_x_ || mouse_current_y_!=mouse_pressed_y_)) {
        const Rect rect{float(mouse_pressed_x_),float(mouse_current_x_),float(mouse_pressed_y_),float(mouse_current_y_)};
        commands.push_back(graphics::shape::draw_quad_filled(rect,vec4(0,0,1,.2f),framebuffer_width(),framebuffer_height()));
        commands.push_back(graphics::shape::draw_quad_wire(rect,vec4(0,0,1,1),framebuffer_width(),framebuffer_height(),dpi));
    }
    set_viewer_overlays(std::move(commands));
    pivot_hint_visible_=pivot; hint_width_=framebuffer_width(); hint_height_=framebuffer_height();
}
void Viewer::cancel_interaction() {
    pressed_key_ = pressed_button_ = -1;
    show_pivot_point_ = zoom_region_ = false;
    camera()->frame()->action_end();
    ClippingPlane::instance()->manipulator()->frame()->action_end();
    update_interaction_overlays(true);
}
Drawable* Viewer::add_drawable(Drawable* drawable) { return add_drawable(std::shared_ptr<Drawable>(drawable)); }
graphics::FrameImage Viewer::capture() const { return capture(samples()); }
graphics::FrameImage Viewer::capture(unsigned sample_count) const {
    if (!initialized_) {
        auto* host = const_cast<Viewer*>(this); host->init(); host->initialized_ = true;
    }
    if (window_) { update_camera_path_overlays(); update_interaction_overlays(); }
    return impl_->capture(*camera(), background_color(), sample_count, framebuffer_width(), framebuffer_height(), snapshot_hud_);
}
int Viewer::run(bool see_all) {
    try {
        if (!initialized_) { init(); initialized_ = true; }
        if (see_all) fit_screen();
        show();
        return run_frames();
    } catch (const std::exception& e) {
        LOG(ERROR) << "Vulkan viewer failed: " << e.what();
        return 1;
    }
}
int Viewer::run_frames(unsigned max_frames) {
    try {
        if (window_host_->error) std::rethrow_exception(window_host_->error);
        return detail::run_viewer_frames(*this, max_frames, [this] {
            return is_animating() || show_frame_rate_ || kfi_->is_interpolation_started();
        });
    } catch (const std::exception& e) { LOG(ERROR) << "Vulkan viewer failed: " << e.what(); return 1; }
}
bool Viewer::mouse_press_event(int x, int y, int button, int modifiers) {
    mouse_pressed_x_ = mouse_current_x_ = x; mouse_pressed_y_ = mouse_current_y_ = y; pressed_button_ = button;
    zoom_region_ = button == BUTTON_LEFT && modifiers == MODIF_CTRL;
    camera()->frame()->action_start();
    if (modifiers == MODIF_ALT && ClippingPlane::instance()->is_enabled())
        ClippingPlane::instance()->manipulator()->frame()->action_start();
    if (pressed_key_ == GLFW_KEY_Z || modifiers == MODIF_SHIFT) {
        if (button == BUTTON_LEFT) {
            bool found = false;
            const auto hit = pick(x, y); found = bool(hit);
            const auto point = hit.position;
            camera()->setPivotPoint(found ? point : camera()->sceneCenter());
            show_pivot_point_ = found;
            if (found) {
                pivot_pixel_ = vec2(float(x),float(y));
                pivot_expires_ = std::chrono::steady_clock::now()+std::chrono::seconds(10);
            }
            if (found && pressed_key_ == GLFW_KEY_Z && modifiers == 0) {
                // Apply the legacy zoom target directly on the UI thread.
                camera()->setPosition(0.1f*camera()->position()+0.9f*point);
                camera()->lookAt(point);
            }
        } else if (button == BUTTON_RIGHT) {
            camera()->setPivotPoint(camera()->sceneCenter());
            show_pivot_point_ = false;
            if (pressed_key_ == GLFW_KEY_Z && modifiers == 0) camera()->showEntireScene();
        }
    }
    update_interaction_overlays(true);
    return false;
}
bool Viewer::mouse_release_event(int x, int y, int button, int modifiers) {
    if (pressed_button_ == button && button == BUTTON_LEFT && modifiers == MODIF_CTRL &&
        x != mouse_pressed_x_ && y != mouse_pressed_y_)
        camera()->fitScreenRegion(std::min(x,mouse_pressed_x_)-interaction_origin_.x,std::min(y,mouse_pressed_y_)-interaction_origin_.y,
                                  std::max(x,mouse_pressed_x_)-interaction_origin_.x,std::max(y,mouse_pressed_y_)-interaction_origin_.y);
    camera()->frame()->action_end();
    ClippingPlane::instance()->manipulator()->frame()->action_end();
    pressed_button_ = -1;
    zoom_region_ = false;
    update_interaction_overlays(true);
    return false;
}
bool Viewer::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
    mouse_current_x_ = x; mouse_current_y_ = y;
    zoom_region_ = pressed_button_ == BUTTON_LEFT && modifiers == MODIF_CTRL;
    update_interaction_overlays(true);
    // Region zoom and clipping/tool manipulation must not move the camera.
    if (modifiers == MODIF_CTRL) return false;
    if (modifiers == MODIF_ALT) {
        if (ClippingPlane::instance()->is_enabled()) {
            auto* frame = ClippingPlane::instance()->manipulator()->frame();
            if (button == BUTTON_LEFT) frame->action_rotate(x-interaction_origin_.x,y-interaction_origin_.y,dx,dy,camera(),ManipulatedFrame::NONE);
            else if (button == BUTTON_RIGHT) frame->action_translate(x-interaction_origin_.x,y-interaction_origin_.y,dx,dy,camera(),ManipulatedFrame::NONE);
        }
        return false;
    }
    auto axis = ManipulatedFrame::NONE;
    if (pressed_key_ == GLFW_KEY_X) axis = ManipulatedFrame::HORIZONTAL;
    else if (pressed_key_ == GLFW_KEY_Y) axis = ManipulatedFrame::VERTICAL;
    else if (pressed_key_ == GLFW_KEY_O) axis = ManipulatedFrame::ORTHOGONAL;
    if (button == BUTTON_LEFT) camera()->frame()->action_rotate(x-interaction_origin_.x,y-interaction_origin_.y,dx,dy,camera(),axis);
    else if (button == BUTTON_RIGHT) camera()->frame()->action_translate(x-interaction_origin_.x,y-interaction_origin_.y,dx,dy,camera(),axis);
    else if (button == BUTTON_MIDDLE && std::abs(dy)>=1) camera()->frame()->action_zoom(dy>0?1:-1,camera());
    return false;
}
bool Viewer::mouse_scroll_event(int, int, int, int dy) { camera()->frame()->action_zoom(dy, camera()); return false; }
bool Viewer::key_press_event(int key, int modifiers) {
    const auto command = commands_.find(Key(key));
    if (command != commands_.end()) {
        const auto binding = command->second.find(Modifier(modifiers));
        if (binding != command->second.end()) return binding->second.first(this,binding->second.second);
    }
    if ((key == GLFW_KEY_ESCAPE && modifiers == 0) || (key == GLFW_KEY_F4 && modifiers == MODIF_ALT)) exit();
    else if (key == GLFW_KEY_F1 && modifiers == 0) {
        if (!manual_.empty()) std::cout << manual_ << '\n';
    } else if (key == GLFW_KEY_F2 && modifiers == 0) set_animation(!animation_enabled_);
    else if ((key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) && modifiers == 0)
        camera()->frame()->action_turn((key==GLFW_KEY_LEFT?1.0f:-1.0f)*float(M_PI/180.0),camera());
    else if ((key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) && modifiers == 0) {
        const float step=.05f*camera()->sceneRadius();
        camera()->frame()->translate(camera()->frame()->inverseTransformOf(vec3(0,0,key==GLFW_KEY_UP?-step:step)));
    } else if (modifiers == MODIF_CTRL && key >= GLFW_KEY_RIGHT && key <= GLFW_KEY_UP) {
        const float step=.05f*camera()->sceneRadius();
        const vec3 direction=key==GLFW_KEY_LEFT?vec3(-step,0,0):key==GLFW_KEY_RIGHT?vec3(step,0,0):
                             key==GLFW_KEY_UP?vec3(0,step,0):vec3(0,-step,0);
        camera()->frame()->translate(camera()->frame()->inverseTransformOf(direction));
    } else if (modifiers == MODIF_CTRL && (key == GLFW_KEY_MINUS || key == GLFW_KEY_EQUAL))
        camera()->frame()->action_zoom(key==GLFW_KEY_EQUAL?1:-1,camera());
    else if (key == GLFW_KEY_K && modifiers == MODIF_ALT) {
        if (kfi_->add_keyframe(*camera()->frame())) LOG(INFO) << "key frame added to camera path";
    } else if (key == GLFW_KEY_D && modifiers == MODIF_ALT) {
        kfi_->delete_path();
        camera_path_renderer_.reset(new vulkan::KeyFrameInterpolatorRenderer);
        set_viewer_annotations({});restore_scene_bounds();update();
    } else if (key == GLFW_KEY_K && modifiers == MODIF_CTRL) {
        if (kfi_->is_interpolation_started()) kfi_->stop_interpolation();
        else {kfi_->start_manual_interpolation();next_camera_path_frame_=std::chrono::steady_clock::now();}
        update();
    } else if (key == GLFW_KEY_T && modifiers == MODIF_CTRL) {
        show_camera_path_=!show_camera_path_;
        if (show_camera_path_) {
            float radius=camera()->sceneRadius();
            for (size_t i=0;i<kfi_->number_of_keyframes();++i)
                radius=std::max(radius,distance(camera()->sceneCenter(),kfi_->keyframe_position(i)));
            camera()->setSceneRadius(radius);
        } else restore_scene_bounds();
        update();
    }
    else if (key == GLFW_KEY_C && modifiers == MODIF_CTRL) copy_view();
    else if (key == GLFW_KEY_V && modifiers == MODIF_CTRL) paste_view();
    else if (key == GLFW_KEY_O && modifiers == MODIF_CTRL) open();
    else if (key == GLFW_KEY_S && modifiers == MODIF_CTRL) save();
    else if (key == GLFW_KEY_S && modifiers == 0) snapshot();
    else if (key == GLFW_KEY_F && modifiers == MODIF_CTRL) {
        show_frame_rate_=!show_frame_rate_;
        if (show_frame_rate_) {
            frame_rate_start_=std::chrono::steady_clock::now();frame_rate_frames_=0;
            framerate_="fps: -- (-- ms/frame)";
        }
        update();
    }
    else if (key == GLFW_KEY_F && modifiers == 0) fit_screen();
    else if (key == GLFW_KEY_C && modifiers == 0) { if (current_model()) fit_screen(current_model()); }
    else if (key == GLFW_KEY_P && modifiers == 0)
        camera()->setType(camera()->type()==Camera::PERSPECTIVE?Camera::ORTHOGRAPHIC:Camera::PERSPECTIVE);
    else if (key == GLFW_KEY_SPACE && modifiers == 0) {
        Frame frame; frame.setTranslation(camera()->pivotPoint()); camera()->frame()->alignWithFrame(&frame,true);
    } else if (key == GLFW_KEY_M && modifiers == 0) {
        if (auto* mesh=dynamic_cast<SurfaceMesh*>(current_model()))
            for (const auto& d:mesh->renderer()->triangles_drawables()) d->set_smooth_shading(!d->smooth_shading());
    } else if (modifiers == 0 && (key==GLFW_KEY_LEFT_BRACKET || key==GLFW_KEY_RIGHT_BRACKET)) {
        for (const auto& m:models()) if (m->renderer()) for (const auto& d:m->renderer()->lines_drawables())
            d->set_line_width(std::max(1.0f,d->line_width()+(key==GLFW_KEY_RIGHT_BRACKET?1:-1)));
    } else if (modifiers == 0 && (key==GLFW_KEY_MINUS || key==GLFW_KEY_EQUAL)) {
        for (const auto& m:models()) if (m->renderer()) for (const auto& d:m->renderer()->points_drawables())
            d->set_point_size(std::max(1.0f,d->point_size()+(key==GLFW_KEY_EQUAL?1:-1)));
    } else if (modifiers == 0 && (key==GLFW_KEY_COMMA || key==GLFW_KEY_PERIOD)) {
        if (!models().empty()) {
            auto i=std::find_if(models().begin(),models().end(),[&](const std::shared_ptr<Model>& m){return m.get()==current_model();});
            const int n=int(models().size()), index=i==models().end()?n-1:int(i-models().begin());
            model_idx_ = (index+n+(key==GLFW_KEY_PERIOD?1:-1))%n; fit_screen(current_model());
        }
    } else if (key==GLFW_KEY_DELETE && modifiers==0) { if (current_model()) delete_model(current_model()); }
    else if (modifiers==0 && current_model() && current_model()->renderer() &&
             (key==GLFW_KEY_E || key==GLFW_KEY_V || key==GLFW_KEY_B || key==GLFW_KEY_L)) {
        auto* renderer=current_model()->renderer();
        Drawable* d=nullptr;
        if (key==GLFW_KEY_E) d=renderer->get_lines_drawable("edges");
        else if (key==GLFW_KEY_V) d=renderer->get_points_drawable("vertices");
        else if (dynamic_cast<SurfaceMesh*>(current_model())) {
            if (key==GLFW_KEY_B) d=renderer->get_lines_drawable("borders");
            else d=renderer->get_points_drawable("locks");
        }
        if (d) d->set_visible(!d->is_visible());
    } else if (key==GLFW_KEY_D && modifiers==0) {
        if (current_model()) current_model()->property_stats(std::cout);
    } else if (modifiers==MODIF_ALT && key==GLFW_KEY_C) {
        auto* plane = ClippingPlane::instance();
        plane->set_enabled(!plane->is_enabled());
        plane->fit_scene(camera()->sceneCenter(),camera()->sceneRadius());
    } else if (modifiers==MODIF_ALT && key==GLFW_KEY_V) {
        auto* plane = ClippingPlane::instance();
        if (plane->is_enabled()) plane->set_visible(!plane->is_visible());
    } else if (modifiers==MODIF_ALT && key==GLFW_KEY_X) {
        auto* plane = ClippingPlane::instance();
        if (plane->is_enabled()) plane->set_cross_section(!plane->cross_section());
    } else if (modifiers==0 && key==GLFW_KEY_A) {
        show_corner_axes_=!show_corner_axes_;
    } else if (modifiers==0 && key==GLFW_KEY_R) {
        LOG(WARNING) << "This viewer shortcut is not yet available in the Vulkan backend";
    } else pressed_key_=key;
    return false;
}
bool Viewer::key_release_event(int key, int) {
    pressed_key_=-1;
    if (key==GLFW_KEY_LEFT_CONTROL || key==GLFW_KEY_RIGHT_CONTROL ||
        key==GLFW_KEY_LEFT_SUPER || key==GLFW_KEY_RIGHT_SUPER) {
        zoom_region_=false; update_interaction_overlays(true);
    }
    return false;
}
void Viewer::set_usage(const std::string& manual, const std::string& screen_hint) {
    if (manual!="default") manual_=manual;
    hint_=screen_hint;
    update();
}
void Viewer::print_usage() {
    if (usage_printed_) return;
    usage_printed_=true;
    if (!manual_.empty()) std::cout << manual_ << std::endl;
}
void Viewer::copy_view() {
    const auto p=camera()->position(); const auto q=camera()->orientation();
    std::ostringstream out; out << std::setprecision(9) << p.x << ' ' << p.y << ' ' << p.z;
    for (unsigned i=0;i<4;++i) out << ' ' << q[i];
    glfwSetClipboardString(window(),out.str().c_str());
}
void Viewer::paste_view() {
    const char* text=glfwGetClipboardString(window());
    if (!text) return;
    std::istringstream in(text); vec3 p; quat q;
    for (unsigned i=0;i<3;++i) if (!(in>>p[i]) || !std::isfinite(p[i])) return;
    float norm=0;
    for (unsigned i=0;i<4;++i) { if (!(in>>q[i]) || !std::isfinite(q[i])) return; norm+=q[i]*q[i]; }
    std::string extra; if ((in>>extra) || !std::isfinite(norm) || norm<1e-12f) return;
    q.normalize(); camera()->setPosition(p); camera()->setOrientation(q); update();
}
    bool Viewer::open() {
        const std::string title("Please choose a file");
        const std::string &default_path = resource::directory() + "/data/";
        const std::vector<std::string> &filters = {
                "Surface Mesh (*.obj *.ply *.off *.stl *.sm *.geojson *.trilist)", "*.obj *.ply *.off *.stl *.sm *.geojson *.trilist",
                "Point Cloud (*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg *.ptx)", "*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg *.ptx",
                "Polyhedral Mesh (*.plm *.pm *.mesh)", "*.plm *.pm *.mesh",
                "Graph (*.ply)", "*.ply",
                "All Files (*.*)", "*"
        };
        const std::vector<std::string> &file_names = dialog::open(title, default_path, filters, true);

        int count = 0;
        for (const auto &file_name : file_names) {
            if (add_model(file_name))
                ++count;
        }

        if (count > 0) {
            fit_screen();
            return true;
        }
        return false;
    }


    bool Viewer::save() const {
        Model* m = current_model();
        if (!m) {
            LOG(ERROR) << "no model exists";
            return false;
        }

        const std::string &title = "Please choose a file name";
        const std::vector<std::string> &filters = {
                "Surface Mesh (*.obj *.ply *.off *.stl *.sm)", "*.obj *.ply *.off *.stl *.sm",
                "Point Cloud (*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg)",
                "*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg",
                "Polyhedral Mesh (*.plm *.pm *.mesh)", "*.plm *.pm *.mesh",
                "Graph (*.ply)", "*.ply",
                "All Files (*.*)", "*"
        };

        std::string default_file_name = m->name();
        if (file_system::extension(default_file_name).empty()) // no extension?
            default_file_name += ".ply"; // default to ply

        const bool warn_overwrite = true;
        const std::string &file_name = dialog::save(title, default_file_name, filters, warn_overwrite);
        if (file_name.empty())
            return false;

        bool saved = false;
        if (dynamic_cast<const PointCloud *>(m))
            saved = PointCloudIO::save(file_name, dynamic_cast<const PointCloud *>(m));
        else if (dynamic_cast<const SurfaceMesh *>(m))
            saved = SurfaceMeshIO::save(file_name, dynamic_cast<const SurfaceMesh *>(m));
        else if (dynamic_cast<const Graph *>(m))
            saved = GraphIO::save(file_name, dynamic_cast<const Graph *>(m));

        if (saved)
            LOG(INFO) << "file successfully saved";

        return saved;
    }


    bool Viewer::snapshot() const {
        const std::string &title = "Please choose a file name";
        std::string default_file_name("untitled.png");
        if (current_model())
            default_file_name = file_system::replace_extension(current_model()->name(), "png");
        const std::vector<std::string> &filters = {
                "Image Files (*.png *.jpg *.bmp *.tga)", "*.png *.jpg *.bmp *.tga",
                "All Files (*.*)", "*"
        };

        const bool warn_overwrite = true;
        const std::string &file_name = dialog::save(title, default_file_name, filters, warn_overwrite);
        if (file_name.empty())
            return false;

        const std::string &ext = file_system::extension(file_name, true);
        if (ext != "png" && ext != "jpg" && ext != "bmp" && ext != "tga") {
            LOG(ERROR) << "snapshot format must be png, jpg, bmp, or tga";
            return false;
        }

        return snapshot(file_name, 1.0f, int(samples()));
    }



bool Viewer::drop_event(const std::vector<std::string>& names) {
    unsigned count=0;
    for (const auto& file : names) if (add_model(file)) ++count;
    if (count) fit_screen();
    return count!=0;
}
}

#else // OpenGL

#include <easy3d/renderer/backends/opengl/overlay_renderer.h>
#include <easy3d/renderer/backends/opengl/point_selection.h>
#include <easy3d/renderer/backends/opengl/query_state.h>

#include <cmath>
#include <limits>

#include <easy3d/viewer/viewer.h>
#include <easy3d/viewer/backends/glfw_runtime.h>
#include <easy3d/viewer/backends/viewer_loop.h>

#include <iostream>

#include <easy3d/renderer/opengl.h>				// for gl functions
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>  // for glfw functions

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/opengl_util.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/text_renderer.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/poly_mesh_io.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/fileio/point_cloud_io_ptx.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/timer.h>
#include <easy3d/util/string.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/setting.h>

#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/core/model.h>
#include <stdexcept>

#include <easy3d/core/matrix.h>
#include <easy3d/fileio/image_io.h>

namespace easy3d {
unsigned Viewer::validation_errors() const { return 0; }
void Viewer::viewer_size(int& w, int& h) const { w = width_; h = height_; }
float Viewer::dpi_scaling() const { return dpi_scaling_; }




    Viewer::Viewer(
            const std::string &title /* = "Easy3D Viewer" */,
            int width /* = 800 */,
            int height /* = 600 */,
            int samples /* = 1 */,
            bool full_screen /* = false */,
            bool visible /* = true */,
            bool resizable /* = true */,
            const OpenGLOptions &gl_options /* = {} */
    )
        : window_(nullptr)
        , should_exit_(false)
        , dpi_scaling_(1.0)
        , title_(title)
        , is_animating_(false)
        , samples_(0)
        , full_screen_(full_screen)
        , visible_(visible)
        , background_color_(setting::background_color)
        , process_events_(true)
        , show_easy3d_logo_(true)
        , pressed_button_(-1)
        , modifiers_(-1)
        , drag_active_(false)
        , mouse_current_x_(0)
        , mouse_current_y_(0)
        , mouse_pressed_x_(0)
        , mouse_pressed_y_(0)
        , pressed_key_(-1)
        , show_pivot_point_(false)
        , show_frame_rate_(false)
        , show_camera_path_(false)
        , model_idx_(-1)
    {
        if (width <= 0 || height <= 0)
            throw std::invalid_argument("window dimensions must be positive");
        if (samples < 0 || samples > 64 || (samples && (samples & (samples - 1))))
            throw std::invalid_argument("invalid viewer sample count");
        // Avoid locale-related number parsing issues.
        setlocale(LC_NUMERIC, "C");

        animation_func_ = nullptr;

        // Initialize logging (if it has not been initialized yet)
        if (!logging::is_initialized())
            logging::initialize(false, true, true, false, "default", 9);

        // create and setup window
        window_ = create_window(title, int(samples), gl_options.major, gl_options.minor, full_screen, resizable,
                                gl_options.depth_bits, gl_options.stencil_bits, int(width), int(height));
        try {
        setup_callbacks(window_);

        // create and set up the camera
        camera_ = std::unique_ptr<Camera>(new Camera);
        camera_->setScreenWidthAndHeight(width_, height_);
        camera_->setType(Camera::PERSPECTIVE);
        camera_->setUpVector(vec3(0, 0, 1)); // Z pointing up
        camera_->setViewDirection(vec3(-1, 0, 0)); // X pointing out
        camera_->showEntireScene();

        easy3d::connect(&camera_->frame_modified, this, &Viewer::update);

        kfi_ = std::unique_ptr<KeyFrameInterpolator>(new KeyFrameInterpolator(camera_->frame()));
        easy3d::connect(&kfi_->interpolation_stopped, this, &Viewer::update);

        snprintf(framerate_, sizeof(framerate_), "fps: ?? (?? ms/frame)");

        /* Poll for events once before starting a potentially lengthy loading process.*/
        glfwPollEvents();

        manual_ =
            "--------------------------------------------------------------\n"
            " Easy3D Viewer Usage (Shortcuts might be overridden):         \n"
            "--------------------------------------------------------------\n"
            "  F1:                      Show help                          \n"
            "  F2:                      Toggle animation (if available)    \n"
            "  Ctrl + 'o':              Open file                          \n"
            "  Ctrl + 's':              Save file                          \n"
            "  Fn + Delete:             Delete current model               \n"
            "  '<' or '>':              Switch between models              \n"
            "  's':                     Take a snapshot                    \n"
            "  'p':                     Switch between perspective and orthographic projections \n"
            "  'a':                     Toggle axes visibility                           \n"
            "  Ctrl + 'f':              Toggle frame rate display                        \n"
            "  Left drag:               Rotate the camera                                \n"
            "  Right drag:              Move the camera                                  \n"
            "  'x' + Left drag:         Rotate the camera around the horizontal axis     \n"
            "  'x' + Right drag:        Move the camera along the horizontal axis        \n"
            "  'y' + Left drag:         Rotate the camera around the vertical axis       \n"
            "  'y' + Right drag:        Move the camera along the vertical axis          \n"
            "  'o' + Left drag:         Rotate the camera around the ortho-screen axis   \n"
            "  Middle button or Wheel:  Zoom in/out                                      \n"
            "  Ctrl + '+'/'-':          Zoom in/out                                      \n"
            "  Left/Right Arrow:        Turn camera left/right                  \n"
            "  Ctrl + Left/Right Arrow: Move camera left/right                  \n"
            "  Up/Down Arrow:           Move camera forward/backward            \n"
            "  Ctrl + Up/Down Arrow:    Move camera up/down                     \n"
            "  Ctrl + 'c':              Copy current view status to clipboard   \n"
            "  Ctrl + 'v':              Restore view status from clipboard      \n"
            "  Alt + 'c':               Toggle clipping plane                   \n"
            "  Alt + 'v':               Toggle clipping plane visibility        \n"
            "  Alt + 'x':               Toggle cross-section                    \n"
            "  Alt + 'k':               Add key frame to the camera path        \n"
            "  Alt + 'd':               Delete the camera path                  \n"
            "  Ctrl + 'k':              Play the camera path                    \n"
            "  Ctrl + 't':              Toggle camera path visibility           \n"
            "  'f':                     Fit screen (all models)                 \n"
            "  'c':                     Fit screen (current model only)         \n"
            "  'z' + Left click:        Zoom to target point on model           \n"
            "  'z' + Right click:       Zoom to fit screen                      \n"
            "  Shift + Left click:      Define a target point on model          \n"
            "  Shift + Right click:     Undefine the target point (if any) on model\n"
            "  Ctrl + Left drag:        Zoom to fit region                         \n"
            "  '-'/'=':                 Decrease/Increase point size               \n"
            "  '{'/'}':                 Decrease/Increase line width               \n"
            "  'b':                     Toggle borders visibility                  \n"
            "  'e':                     Toggle edges visibility                    \n"
            "  'v':                     Toggle vertices visibility                 \n"
            "  'm':                     Toggle smooth shading (for SurfaceMesh)    \n"
            "  'd':                     Print model info (drawables, properties)   \n"
            "-------------------------------------------------------------------";
        } catch (...) { cleanup(); throw; }
    }


    GLFWwindow *Viewer::create_window(
            const std::string &title,
            int samples,
            int gl_major,
            int gl_minor,
            bool full_screen,
            bool resizable,
            int depth_bits,
            int stencil_bits,
            int width,
            int height) {
        if (samples == 1) samples = 0; // One sample means no MSAA in the unified API.
        runtime_ = detail::GlfwRuntime::acquire();

        // Reset the hints, allowing viewers to have different hints.
        glfwDefaultWindowHints();

        glfwWindowHint(GLFW_SAMPLES, samples);

        glfwWindowHint(GLFW_STENCIL_BITS, stencil_bits);
        glfwWindowHint(GLFW_DEPTH_BITS, depth_bits);

        /* Request a forward compatible OpenGL glMajor.glMinor core profile context.
           Default value is an OpenGL 3.2 core profile context. */
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_minor);

#ifdef __APPLE__
        // The only OpenGL 3.x and 4.x contexts currently supported by macOS are
        // forward-compatible, core profile contexts.
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
        if (gl_major >= 3) {
            if (gl_minor >= 2)
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	// 3.2+ only
            if (gl_minor >= 0)
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);			// 3.0+ only
        }
#endif

        // make the whole window transparent
        //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

        glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, resizable ? GL_TRUE : GL_FALSE);

#ifndef NDEBUG
        const std::string title_str = title + " - Debug Version";
#else
        const std::string title_str = title;
#endif

        GLFWwindow *window = nullptr;
        if (full_screen) {
            GLFWmonitor *monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            width = mode->width;
            height = mode->height;
            window = glfwCreateWindow(width, height, title_str.c_str(), monitor, runtime_->opengl_share_window());
        } else {
            window = glfwCreateWindow(width, height, title_str.c_str(), nullptr, runtime_->opengl_share_window());
        }

        if (!window) {
            std::string error_message = "could not create an OpenGL "
                                        + std::to_string(gl_major) + "." + std::to_string(gl_minor) + " context!";
            LOG(ERROR) << error_message;
            throw std::runtime_error(error_message);
        }
        const auto previous_context = glfwGetCurrentContext();
        auto close_failed_window = [previous_context](GLFWwindow* failed) {
            glfwDestroyWindow(failed);
            glfwMakeContextCurrent(previous_context);
        };
        std::unique_ptr<GLFWwindow, decltype(close_failed_window)> guard(window, close_failed_window);
        glfwSetWindowUserPointer(window, this);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        // Enable vsync
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        // Load OpenGL and its extensions
        if (!OpenglUtil::init()) {
            LOG(ERROR) << "failed to load OpenGL and its extensions!";
            throw std::runtime_error("failed to load OpenGL and its extensions!");
        }

#ifndef NDEBUG
        opengl::setup_gl_debug_callback();
#endif

        VLOG(1) << "OpenGL vendor: " << glGetString(GL_VENDOR);
        VLOG(1) << "OpenGL renderer: " << glGetString(GL_RENDERER);
        VLOG(1) << "OpenGL version requested: " << gl_major << "." << gl_minor;
        VLOG(1) << "OpenGL version received: " << glGetString(GL_VERSION);
        VLOG(1) << "GLSL version received: " << glGetString(GL_SHADING_LANGUAGE_VERSION);

        glGetIntegerv(GL_SAMPLES, &samples_);
        int max_num = 0;
        glGetIntegerv(GL_MAX_SAMPLES, &max_num);

        // warn the user if the requests were not satisfied
        if (samples > 0 && samples_ != samples) {
            if (samples_ == 0)
                LOG(WARNING) << "MSAA is not available (" << samples << " samples requested)";
            else
                LOG(WARNING) << "MSAA is available with " << samples_ << " samples (" << samples
                             << " requested but max support is " << max_num << ")";
        } else
            VLOG(1) << "samples received: " << samples_ << " (" << samples << " requested, max support is "
                      << max_num << ")";

        // the request viewer size might not be satisfied, thus we query the actual size
        glfwGetWindowSize(window, &width_, &height_);
        VLOG(1) << "viewer size (width/height): " << width_ << "/" << height_;

        float xscale(0), yscale(0);
        glfwGetWindowContentScale(window, &xscale, &yscale);
        LOG_IF(xscale != yscale, ERROR) << "window content scale in X and Y directions not identical (X: " << xscale << ", Y: " << yscale << ")";
        dpi_scaling_ = static_cast<float>(xscale + yscale) * 0.5f;
        VLOG(1) << "DPI scaling: " << dpi_scaling();

        glfwGetFramebufferSize(window, &framebuffer_width_, &framebuffer_height_);
        runtime_->attach_opengl_window(window);
        return guard.release();
    }


    template<class Callback>
    void Viewer::dispatch_callback(GLFWwindow* window, const Callback& callback) noexcept {
        auto* viewer = static_cast<Viewer*>(glfwGetWindowUserPointer(window));
        if (!viewer) return;
        try {
            glfwMakeContextCurrent(window);
            callback(*viewer);
            if (viewer->callback_error_) glfwSetWindowShouldClose(window, GLFW_TRUE);
        } catch (...) {
            if (!viewer->callback_error_) viewer->callback_error_ = std::current_exception();
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    void Viewer::setup_callbacks(GLFWwindow *window) {
        glfwSetWindowRefreshCallback(window, [](GLFWwindow* win) {
            dispatch_callback(win, [](Viewer& v) { v.update(); });
        });
        glfwSetCursorPosCallback(window, [](GLFWwindow* win, double x, double y) {
            dispatch_callback(win, [=](Viewer& v) {
                if (!v.process_events_) return;
                const int wx = int(x), wy = int(y);
                const int dx = wx - v.callback_mouse_x_, dy = wy - v.callback_mouse_y_;
                v.callback_mouse_x_ = wx; v.callback_mouse_y_ = wy;
                const int modifiers = detail::GlfwRuntime::modifiers(win);
                const bool handled = v.drag_active_
                    ? v.dispatch_mouse_drag(wx, wy, dx, dy, v.pressed_button_, modifiers)
                    : v.dispatch_mouse_move(wx, wy, dx, dy, modifiers);
                if (handled) { v.mouse_current_x_ = wx; v.mouse_current_y_ = wy; return; }
                const int w = v.width(), h = v.height();
                if (x >= 0 && x <= w && y >= 0 && y <= h) v.callback_event_cursor_pos(x, y);
                else if (v.drag_active_) glfwSetCursorPos(win, std::max(0.0, std::min(double(w), x)),
                                                               std::max(0.0, std::min(double(h), y)));
            });
        });
        glfwSetMouseButtonCallback(window, [](GLFWwindow* win, int button, int action, int modifiers) {
            dispatch_callback(win, [=](Viewer& v) {
                if (!v.process_events_) return;
                double x, y; glfwGetCursorPos(win, &x, &y);
                v.mouse_current_x_ = v.callback_mouse_x_ = int(x);
                v.mouse_current_y_ = v.callback_mouse_y_ = int(y);
                v.callback_event_mouse_button(button, action, modifiers);
            });
        });
        glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int, int action, int mods) {
            dispatch_callback(win, [=](Viewer& v) {
                if (v.process_events_) v.callback_event_keyboard(key, action, mods);
            });
        });
        glfwSetCharCallback(window, [](GLFWwindow* win, unsigned int codepoint) {
            dispatch_callback(win, [=](Viewer& v) {
                if (v.process_events_) v.callback_event_character(codepoint);
            });
        });
        glfwSetDropCallback(window, [](GLFWwindow* win, int count, const char** filenames) {
            dispatch_callback(win, [=](Viewer& v) {
                if (v.process_events_) v.callback_event_drop(count, filenames);
            });
        });
        glfwSetScrollCallback(window, [](GLFWwindow* win, double dx, double dy) {
            dispatch_callback(win, [=](Viewer& v) {
                if (!v.process_events_) return;
                double x, y; glfwGetCursorPos(win, &x, &y);
                v.mouse_current_x_ = int(x); v.mouse_current_y_ = int(y);
                v.callback_event_scroll(dx, dy);
            });
        });
        // Camera/input coordinates are logical window units in this OpenGL host.
        glfwSetWindowSizeCallback(window, [](GLFWwindow* win, int w, int h) {
            dispatch_callback(win, [=](Viewer& v) {
                if (v.process_events_) v.callback_event_resize(w, h);
            });
        });
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int, int) {
            dispatch_callback(win, [=](Viewer& v) {
                if (!v.process_events_) return;
                int w, h; glfwGetWindowSize(win, &w, &h); v.callback_event_resize(w, h);
            });
        });
        glfwSetWindowFocusCallback(window, [](GLFWwindow* win, int focused) {
            dispatch_callback(win, [=](Viewer& v) {
                if (!focused) {
                    v.drag_active_ = false; v.pressed_button_ = v.pressed_key_ = -1; v.modifiers_ = 0;
                    v.scroll_remainder_x_ = v.scroll_remainder_y_ = 0;
                    v.camera_->frame()->action_end();
                    ClippingPlane::instance()->manipulator()->frame()->action_end();
                }
                if (!v.dispatch_focus(focused != 0)) v.focus_event(focused != 0);
            });
        });
        glfwSetWindowCloseCallback(window, [](GLFWwindow* win) {
            dispatch_callback(win, [](Viewer& v) { v.request_close(); });
        });
    }


    bool Viewer::callback_event_cursor_pos(double x, double y) {
        int px = static_cast<int>(x);
        int py = static_cast<int>(y);
        try {
            int dx = px - mouse_current_x_;
            int dy = py - mouse_current_y_;
            mouse_current_x_ = px;
            mouse_current_y_ = py;
            if (drag_active_)
                return mouse_drag_event(px, py, dx, dy, pressed_button_, modifiers_);
            else
                return mouse_free_move_event(px, py, dx, dy, modifiers_);
        }
        catch (const std::exception &e) {
            if (!callback_error_) callback_error_ = std::current_exception();
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    bool Viewer::callback_event_mouse_button(int button, int action, int modifiers) {
        try {
            if (action == GLFW_PRESS) {
                drag_active_ = true;
                pressed_button_ = button;
                modifiers_ = modifiers;
                mouse_pressed_x_ = mouse_current_x_;
                mouse_pressed_y_ = mouse_current_y_;
                if (dispatch_mouse_press(mouse_current_x_, mouse_current_y_, button, modifiers)) return true;
                return mouse_press_event(mouse_current_x_, mouse_current_y_, button, modifiers);
            } else if (action == GLFW_RELEASE) {
                drag_active_ = false;
                if (dispatch_mouse_release(mouse_current_x_, mouse_current_y_, button, modifiers)) {
                    pressed_button_ = -1; camera_->frame()->action_end();
                    ClippingPlane::instance()->manipulator()->frame()->action_end();
                    return true;
                }
                return mouse_release_event(mouse_current_x_, mouse_current_y_, button, modifiers);
            } else {
                drag_active_ = false;
                LOG(INFO) << "GLFW_REPEAT? Seems never happen";
                return false;
            }
        }
        catch (const std::exception &e) {
            if (!callback_error_) callback_error_ = std::current_exception();
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    bool Viewer::callback_event_keyboard(int key, int action, int modifiers) {
        try {
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                if (dispatch_key_press(key, modifiers)) return true;
                return key_press_event(key, modifiers);
            } else {
                if (dispatch_key_release(key, modifiers)) { pressed_key_ = -1; return true; }
                return key_release_event(key, modifiers);
            }
        }
        catch (const std::exception &e) {
            if (!callback_error_) callback_error_ = std::current_exception();
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    bool Viewer::callback_event_character(unsigned int codepoint) {
        try {
            if (dispatch_character(codepoint)) return true;
            return char_input_event(codepoint);
        }
        catch (const std::exception &e) {
            if (!callback_error_) callback_error_ = std::current_exception();
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    bool Viewer::callback_event_drop(int count, const char **filenames) {
        try {
            std::vector<std::string> arg(count);
            for (int i = 0; i < count; ++i)
                arg[i] = filenames[i];
            if (dispatch_drop(arg)) return true;
            return drop_event(arg);
        }
        catch (const std::exception &e) {
            if (!callback_error_) callback_error_ = std::current_exception();
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    bool Viewer::callback_event_scroll(double dx, double dy) {
        try {
            scroll_remainder_x_ += dx; scroll_remainder_y_ += dy;
            const int sx = int(scroll_remainder_x_), sy = int(scroll_remainder_y_);
            scroll_remainder_x_ -= sx; scroll_remainder_y_ -= sy;
            if (!sx && !sy) return false;
            if (dispatch_mouse_scroll(mouse_current_x_, mouse_current_y_, sx, sy)) return true;
            return mouse_scroll_event(mouse_current_x_, mouse_current_y_, sx, sy);
        }
        catch (const std::exception &e) {
            if (!callback_error_) callback_error_ = std::current_exception();
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    void Viewer::callback_event_resize(int w, int h) {
        try {
            int fw, fh; glfwGetFramebufferSize(window_, &fw, &fh);
            const bool changed = w != width_ || h != height_ || fw != framebuffer_width_ || fh != framebuffer_height_;
            width_ = w; height_ = h; framebuffer_width_ = fw; framebuffer_height_ = fh;
            if (w > 0 && h > 0 && (camera_->screenWidth() != w || camera_->screenHeight() != h))
                camera_->setScreenWidthAndHeight(w, h);
            glViewport(0, 0, fw, fh);
            if (changed) post_resize(w, h);
            notify_resize();
        } catch (const std::exception& e) {
            if (!callback_error_) callback_error_ = std::current_exception();
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
        }
    }


    bool Viewer::focus_event(bool focused) {
        if (focused) {
            // ...
        }
        return false;
    }


    Viewer::~Viewer() {
        cleanup();

        LOG(INFO) << "viewer terminated. Bye!";
    }

    void Viewer::clear_scene() {
        glfwMakeContextCurrent(window_);
        models_.clear();
        drawables_.clear();
        if(overlay_renderer_) overlay_renderer_->set({},false);
    }

    void Viewer::cleanup() {
        // viewer may have already been destroyed by the user
        if (!window_)
            return;

        const auto previous_context = glfwGetCurrentContext();
        glfwMakeContextCurrent(window_);
        glfwSetWindowUserPointer(window_, nullptr);
        if (kfi_) kfi_->stop_interpolation();
        kfi_.reset();
        camera_.reset();
        drawable_axes_.reset();
        drawable_pivot_.reset();
        texter_.reset();
        overlay_renderer_.reset();
        point_selector_.reset();

        clear_scene();

        if (runtime_->last_opengl_window()) {
            ShaderManager::terminate();
            TextureManager::terminate();
        }

        const auto closing = window_;
        runtime_->detach_opengl_window(window_);
        glfwDestroyWindow(window_);
        window_ = nullptr;
        if (previous_context && previous_context != closing)
            glfwMakeContextCurrent(previous_context);
        runtime_.reset();
    }


    void Viewer::set_title(const std::string &title) {
        if (title != title_) {
            glfwSetWindowTitle(window_, title.c_str());
            title_ = title;
        }
    }


    void Viewer::set_position(int xpos, int ypos) {
        glfwSetWindowPos(window_, xpos, ypos);
    }


    void Viewer::resize(int w, int h) {
#ifndef __APPLE__
        w = static_cast<int>(w * dpi_scaling());
        h = static_cast<int>(h * dpi_scaling());
#endif
        glfwSetWindowSize(window_, w, h);
    }


	int Viewer::framebuffer_width() const {
		int w, h;
		glfwGetFramebufferSize(window_, &w, &h);
		return w;
	}


	int Viewer::framebuffer_height() const {
		int w, h;
		glfwGetFramebufferSize(window_, &w, &h);
		return h;
	}


	void Viewer::framebuffer_size(int& w, int& h) const {
		glfwGetFramebufferSize(window_, &w, &h);
	}


    void Viewer::update() const {
        glfwPostEmptyEvent();
    }


    bool Viewer::mouse_press_event(int x, int y, int button, int modifiers) {
        camera_->frame()->action_start();

        if (pressed_key_ == GLFW_KEY_Z || modifiers == GLFW_MOD_SHIFT) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                bool found = false;
                const vec3 &p = point_under_pixel(x, y, found);
                if (found) {
                    camera_->setPivotPoint(p);
                    // We draw the pivot mark in the screen space.
                    // A 3D version is implemented in Tutorial_204_Viewer_Qt
                    if (!drawable_pivot_)
                        drawable_pivot_ = std::unique_ptr<LinesDrawable2D>(new LinesDrawable2D("pivot"));
                    const float size = static_cast<float>(10 * dpi_scaling());
                    const std::vector<vec2> points = { vec2(x - size, y), vec2(x + size, y),
                                                       vec2(x, y - size), vec2(x, y + size) };
                    drawable_pivot_->update_vertex_buffer(points, width(), height(), true);

                    // show, but hide the visual hint of pivot point after \p delay milliseconds.
                    show_pivot_point_ = true;
                    Timer<>::single_shot(10000, [&]() {
                        show_pivot_point_ = false;
                        update();
                    });

                    if (pressed_key_ == GLFW_KEY_Z &&  modifiers == 0) {
#if 1   // with animation
                        camera()->interpolateToLookAt(p);
#else   // without animation
                        const float coef = 0.1f;
                        const vec3& pos = coef * camera()->frame()->position() + (1.0f - coef)*p;
                        const quat& ori = camera()->frame()->orientation();
                        camera_->frame()->setPositionAndOrientation(pos, ori);
                        camera_->lookAt(p);
#endif
                    }
                } else {
                    camera_->setPivotPoint(camera_->sceneCenter());
                    show_pivot_point_ = false;
                }
            } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                camera_->setPivotPoint(camera_->sceneCenter());
                show_pivot_point_ = false;
                if (pressed_key_ == GLFW_KEY_Z &&  modifiers == 0) {
#if 1   // with animation
                    camera()->interpolateToFitScene();
#else   // without animation
                    camera()->showEntireScene();
#endif
                }
            }
        }

        return false;
    }


    bool Viewer::mouse_release_event(int x, int y, int button, int modifiers) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && modifiers == MODIF_CTRL) { // ZOOM_ON_REGION
            int xmin = std::min(mouse_pressed_x_, x);
            int xmax = std::max(mouse_pressed_x_, x);
            int ymin = std::min(mouse_pressed_y_, y);
            int ymax = std::max(mouse_pressed_y_, y);
            camera_->fitScreenRegion(xmin, ymin, xmax, ymax);
        } else
            camera_->frame()->action_end();

        pressed_button_ = -1;
        return false;
    }


    bool Viewer::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
        // control modifier is reserved for zooming on region
        if (modifiers == MODIF_CTRL)
            return false;

        if (modifiers == MODIF_ALT) {
            // the clipping plane can be manipulated only when it is enabled
            if (ClippingPlane::instance()->is_enabled()) {
                ManipulatedFrame *frame = ClippingPlane::instance()->manipulator()->frame();
                if (pressed_button_ == BUTTON_LEFT)
                    frame->action_rotate(x, y, dx, dy, camera(), ManipulatedFrame::NONE);
                else if (pressed_button_ == BUTTON_RIGHT)
                    frame->action_translate(x, y, dx, dy, camera(), ManipulatedFrame::NONE);
            }
        }
        else { // axis-constrained rotation of a model
            auto axis = ManipulatedFrame::NONE;
            if (pressed_key_ == GLFW_KEY_X) axis = ManipulatedFrame::HORIZONTAL;
            else if (pressed_key_ == GLFW_KEY_Y) axis = ManipulatedFrame::VERTICAL;
            else if (pressed_key_ == GLFW_KEY_O) axis = ManipulatedFrame::ORTHOGONAL;
            switch (button) {
                case GLFW_MOUSE_BUTTON_LEFT:
                    camera_->frame()->action_rotate(x, y, dx, dy, camera(), axis);
                    break;
                case GLFW_MOUSE_BUTTON_RIGHT:
                    camera_->frame()->action_translate(x, y, dx, dy, camera(), axis);
                    break;
                case GLFW_MOUSE_BUTTON_MIDDLE:
                    if (std::abs(dy) >= 1)
                        camera_->frame()->action_zoom(dy > 0 ? 1 : -1, camera());
                    break;
                default:
                    break;
            }
        }

        return false;
    }


    bool Viewer::mouse_free_move_event(int x, int y, int dx, int dy, int modifiers) {
        (void) x;
        (void) y;
        (void) dx;
        (void) dy;
        (void) modifiers;
        // highlight geometry primitives here
        return false;
    }


    bool Viewer::mouse_scroll_event(int x, int y, int dx, int dy) {
        (void) x;
        (void) y;
        (void) dx;
        camera_->frame()->action_zoom(dy, camera());
        return false;
    }


    Model* Viewer::current_model() const {
        if (models_.empty())
            return nullptr;
        if (model_idx_ < models_.size())
            return models_[model_idx_].get();
        return nullptr;
    }

    bool Viewer::key_press_event(int key, int modifiers) {
        auto mod_funcs = commands_.find(Key(key));
        if (mod_funcs != commands_.end()) {
            auto func_model = mod_funcs->second.find(Modifier(modifiers));
            if (func_model != mod_funcs->second.end()) {
                auto func = func_model->second.first;
                auto model = func_model->second.second;
                return func(this, model);
            }
        }

        if (key == GLFW_KEY_F1 && modifiers == 0)
            std::cout << manual_ << std::endl;
        else if (key == GLFW_KEY_F2 && modifiers == 0) {
            is_animating_ = !is_animating_;
            if (is_animating_ && !animation_func_)
                LOG(WARNING) << "animation is enabled but the animation function (i.e., animation_func_) is missing";
        } else if (key == GLFW_KEY_LEFT && modifiers == 0) {
            auto angle = static_cast<float>(1 * M_PI / 180.0); // turn left, 1 degrees each step
            camera_->frame()->action_turn(angle, camera());
        } else if (key == GLFW_KEY_RIGHT && modifiers == 0) {
            auto angle = static_cast<float>(1 * M_PI / 180.0); // turn right, 1 degrees each step
            camera_->frame()->action_turn(-angle, camera());
        } else if (key == GLFW_KEY_UP && modifiers == 0) {    // move camera forward
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, 0.0, -step)));
        } else if (key == GLFW_KEY_DOWN && modifiers == 0) {// move camera backward
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, 0.0, step)));
        } else if (key == GLFW_KEY_LEFT && modifiers == MODIF_CTRL) {    // move camera left
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(-step, 0.0, 0.0)));
        } else if (key == GLFW_KEY_RIGHT && modifiers == MODIF_CTRL) {    // move camera right
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(step, 0.0, 0.0)));
        } else if (key == GLFW_KEY_UP && modifiers == MODIF_CTRL) {    // move camera up
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, step, 0.0)));
        } else if (key == GLFW_KEY_DOWN && modifiers == MODIF_CTRL) {    // move camera down
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, -step, 0.0)));
        } else if (key == GLFW_KEY_C && modifiers == MODIF_CTRL) {    // copy camera
            copy_view();
        } else if (key == GLFW_KEY_V && modifiers == MODIF_CTRL) {    // copy camera
            paste_view();
        } else if (key == GLFW_KEY_C && modifiers == MODIF_ALT) {     // toggle clipping plane
            ClippingPlane::instance()->set_enabled(!ClippingPlane::instance()->is_enabled());
            ClippingPlane::instance()->fit_scene(camera()->sceneCenter(), camera()->sceneRadius());
        } else if (key == GLFW_KEY_V && modifiers == MODIF_ALT) {     // toggle clipping plane visibility
            if (ClippingPlane::instance()->is_enabled())
                ClippingPlane::instance()->set_visible(!ClippingPlane::instance()->is_visible());
        } else if (key == GLFW_KEY_X && modifiers == MODIF_ALT) {
            if (ClippingPlane::instance()->is_enabled())
                ClippingPlane::instance()->set_cross_section(!ClippingPlane::instance()->cross_section());
        }
        else if (key == GLFW_KEY_A && modifiers == 0) {
            if (drawable_axes_)
                drawable_axes_->set_visible(!drawable_axes_->is_visible());
        }
        else if (key == GLFW_KEY_F && modifiers == MODIF_CTRL)
            show_frame_rate_ = !show_frame_rate_;
        else if (key == GLFW_KEY_C && modifiers == 0) {
            if (current_model())
                fit_screen(current_model());
        } else if (key == GLFW_KEY_F && modifiers == 0) {
            fit_screen();
        } else if (key == GLFW_KEY_M && modifiers == 0) {
#if 0
            // NOTE: switching on/off MSAA in this way will affect all viewers because OpenGL
            //       is a state machine. For multi-window applications, you have to call
            //		 glDisable()/glEnable() before the individual draw functions.
            if (samples_ > 0) {
                if (glIsEnabled(GL_MULTISAMPLE)) {
                    glDisable(GL_MULTISAMPLE);
                    LOG(INFO) << title_ + ": MSAA disabled";
                }
                else {
                    glEnable(GL_MULTISAMPLE);
                    LOG(INFO) << title_ + ": MSAA enabled";
                }
            }
#else
            if (dynamic_cast<SurfaceMesh *>(current_model())) {
                auto drawables = current_model()->renderer()->triangles_drawables();
                for (auto d : drawables)
                    d->set_smooth_shading(!d->smooth_shading());
            }
#endif
        } else if (key == GLFW_KEY_P && modifiers == 0) {
            if (camera_->type() == Camera::PERSPECTIVE)
                camera_->setType(Camera::ORTHOGRAPHIC);
            else
                camera_->setType(Camera::PERSPECTIVE);
        } else if (key == GLFW_KEY_SPACE && modifiers == 0) {
            // Aligns camera
            Frame frame;
            frame.setTranslation(camera_->pivotPoint());
            camera_->frame()->alignWithFrame(&frame, true);

            // Aligns frame
            //if (manipulatedFrame())
            //	manipulatedFrame()->alignWithFrame(camera_->frame());
        } else if (key == GLFW_KEY_O && modifiers == MODIF_CTRL)
            open();
        else if (key == GLFW_KEY_S && modifiers == MODIF_CTRL)
            save();

        else if (key == GLFW_KEY_MINUS && modifiers == MODIF_CTRL)
            camera_->frame()->action_zoom(-1, camera());
        else if (key == GLFW_KEY_EQUAL && modifiers == MODIF_CTRL)
            camera_->frame()->action_zoom(1, camera());

        else if (key == GLFW_KEY_K && modifiers == GLFW_MOD_ALT) { // add key frame
            easy3d::Frame *frame = camera()->frame();
            kfi_->add_keyframe(*frame);
            LOG(INFO) << "key frame added to camera path";
        } else if (key == GLFW_KEY_D && modifiers == GLFW_MOD_ALT) { // delete path
            kfi_->delete_path();
            // update scene bounding box
            Box3 box;
            for (auto m : models_)
                box.grow(m->bounding_box());
            camera_->setSceneBoundingBox(box.min_point(), box.max_point());
            LOG(INFO) << "camera path deleted";
        } else if (key == GLFW_KEY_K && modifiers == MODIF_CTRL) { // play the path
            if (kfi_->is_interpolation_started())
                kfi_->stop_interpolation();
            else {
                kfi_->start_manual_interpolation();
                next_camera_path_frame_ = std::chrono::steady_clock::now();
            }
        } else if (key == GLFW_KEY_T && modifiers == MODIF_CTRL) {
            show_camera_path_ = !show_camera_path_;
            if (show_camera_path_) {
                const std::size_t count = kfi_->number_of_keyframes();
                float radius = camera_->sceneRadius();
                for (std::size_t i=0; i<count; ++i) {
                    radius = std::max( radius,
                            distance(camera_->sceneCenter(), kfi_->keyframe(i).position())
                    );
                }
                camera_->setSceneRadius(radius);
            }
            else {
                Box3 box;
                for (auto m : models_)
                    box.grow(m->bounding_box());
                camera_->setSceneBoundingBox(box.min_point(), box.max_point());
            }
        } else if (key == GLFW_KEY_LEFT_BRACKET && modifiers == 0) {
            for (auto m : models_) {
                for (auto d : m->renderer()->lines_drawables()) {
                    float size = d->line_width() - 1.0f;
                    if (size < 1)
                        size = 1;
                    d->set_line_width(size);
                }
            }
        }
        else if (key == GLFW_KEY_RIGHT_BRACKET && modifiers == 0) {
            for (auto m : models_) {
                for (auto d : m->renderer()->lines_drawables()) {
                    float size = d->line_width() + 1.0f;
                    d->set_line_width(size);
                }
            }
        } else if (key == GLFW_KEY_MINUS && modifiers == 0) {
            for (auto m : models_) {
                for (auto d : m->renderer()->points_drawables()) {
                    float size = d->point_size() - 1.0f;
                    if (size < 1)
                        size = 1;
                    d->set_point_size(size);
                }
            }
        } else if (key == GLFW_KEY_EQUAL && modifiers == 0) {
            for (auto m : models_) {
                for (auto d : m->renderer()->points_drawables()) {
                    float size = d->point_size() + 1.0f;
                    d->set_point_size(size);
                }
            }
        } else if (key == GLFW_KEY_COMMA && modifiers == 0) {
            if (models_.empty())
                model_idx_ = -1;
            else
                model_idx_ = int((model_idx_ - 1 + models_.size()) % models_.size());
            if (model_idx_ >= 0) {
                fit_screen(models_[model_idx_].get());
                std::cout << "current model: " << model_idx_ << ", " << models_[model_idx_]->name() << std::endl;
            }
        } else if (key == GLFW_KEY_PERIOD && modifiers == 0) {
            if (models_.empty())
                model_idx_ = -1;
            else
                model_idx_ = int((model_idx_ + 1) % models_.size());
            if (model_idx_ >= 0) {
                fit_screen(models_[model_idx_].get());
                std::cout << "current model: " << model_idx_ << ", " << models_[model_idx_]->name() << std::endl;
            }
        } else if (key == GLFW_KEY_DELETE && modifiers == 0) {
            if (current_model())
                delete_model(current_model());
        } else if (key == GLFW_KEY_E && modifiers == 0) {
            if (current_model()) {
                auto drawable = current_model()->renderer()->get_lines_drawable("edges");
                if (drawable)
					drawable->set_visible(!drawable->is_visible());
            }
        } else if (key == GLFW_KEY_V && modifiers == 0) {
            if (current_model()) {
                auto drawable = current_model()->renderer()->get_points_drawable("vertices");
                if (drawable)
					drawable->set_visible(!drawable->is_visible());
            }
        } else if (key == GLFW_KEY_B && modifiers == 0) {
            auto mesh = dynamic_cast<SurfaceMesh *>(current_model());
            if (mesh) {
                auto drawable = mesh->renderer()->get_lines_drawable("borders");
                if (drawable)
                    drawable->set_visible(!drawable->is_visible());
            }
        } else if (key == GLFW_KEY_L && modifiers == 0) { // locked vertices
            auto mesh = dynamic_cast<SurfaceMesh *>(current_model());
            if (mesh) {
                auto drawable = mesh->renderer()->get_points_drawable("locks");
                if (drawable)
                    drawable->set_visible(!drawable->is_visible());
            }
        } else if (key == GLFW_KEY_D && modifiers == 0) {
            if (current_model()) {
                auto &output = std::cout;

                output << "----------- " << file_system::simple_name(current_model()->name()) << " -----------\n";
                if (dynamic_cast<SurfaceMesh *>(current_model())) {
                    auto model = dynamic_cast<SurfaceMesh *>(current_model());
                    output << "model is a surface mesh. #face: " << std::to_string(model->n_faces())
                           << ", #vertex: " + std::to_string(model->n_vertices())
                           << ", #edge: " + std::to_string(model->n_edges()) << std::endl;
                } else if (dynamic_cast<PointCloud *>(current_model())) {
                    auto model = dynamic_cast<PointCloud *>(current_model());
                    output << "model is a point cloud. #vertex: " + std::to_string(model->n_vertices()) << std::endl;
                } else if (dynamic_cast<Graph *>(current_model())) {
                    auto model = dynamic_cast<Graph *>(current_model());
                    output << "model is a graph. #vertex: " + std::to_string(model->n_vertices())
                           << ", #edge: " + std::to_string(model->n_edges()) << std::endl;
                }

                if (!current_model()->renderer()->points_drawables().empty()) {
                    output << "points drawables:\n";
                    for (auto d : current_model()->renderer()->points_drawables())
                        d->buffer_stats(output);
                }
                if (!current_model()->renderer()->lines_drawables().empty()) {
                    output << "lines drawables:\n";
                    for (auto d : current_model()->renderer()->lines_drawables())
                        d->buffer_stats(output);
                }
                if (!current_model()->renderer()->triangles_drawables().empty()) {
                    output << "triangles drawables:\n";
                    for (auto d : current_model()->renderer()->triangles_drawables())
                        d->buffer_stats(output);
                }

                current_model()->property_stats(output);
            }
        } else if (key == GLFW_KEY_R && modifiers == 0) {
            // Reload the shader(s) - useful for writing/debugging shader code.
            ShaderManager::reload();
        } else if (key == GLFW_KEY_S && modifiers == 0) {
            snapshot();
        } else if (key == GLFW_KEY_F4 && modifiers == GLFW_MOD_ALT) {
            glfwSetWindowShouldClose(window_, true);
        }
        else
            pressed_key_ = key;

        return false;
    }


    bool Viewer::key_release_event(int key, int modifiers) {
        (void) key;
        (void) modifiers;
        pressed_key_ = -1;
        return false;
    }


    bool Viewer::char_input_event(unsigned int codepoint) {
        (void) codepoint;
        //switch (codepoint) {
        //case '-':	break;
        //case 'c':	break;
        //case 'C':	break;
        //case '0':	break;
        //default:
        //	return false;
        //}

        return false;
    }


    bool Viewer::drop_event(const std::vector<std::string> &filenames) {
        int count = 0;
        for (auto &name : filenames) {
            if (add_model(name))
                ++count;
        }

        if (count > 0) {
            model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current
            fit_screen();
            return true;
        }
        return false;
    }


    void Viewer::set_usage(const std::string& manual, const std::string& screen_hint)
    {
        if (manual != "default")
            manual_ = manual;
        hint_ = screen_hint;
        usage_printed_ = false;
    }


    void Viewer::poll_events() { ViewerBase::poll_events(); }
    void Viewer::request_close() { glfwSetWindowShouldClose(window_, GLFW_TRUE); update(); }
    bool Viewer::should_close() const { return should_exit_ || glfwWindowShouldClose(window_) != 0; }
    void Viewer::show() { glfwShowWindow(window_); }
    void Viewer::hide() { glfwHideWindow(window_); }

    void Viewer::print_usage() {
        if (usage_printed_) return;
        usage_printed_ = true;
        if (!manual_.empty()) std::cout << manual_ << std::endl;
    }

    void Viewer::update_frame_rate() {
        if (!show_frame_rate_) return;
        const double now = glfwGetTime();
        if (!frame_rate_start_) frame_rate_start_ = now;
        ++frame_rate_frames_;
        const double elapsed = now - frame_rate_start_;
        if (elapsed >= 1.0) {
            snprintf(framerate_, sizeof(framerate_), "fps: %2.0f (%4.1f ms/frame)",
                     double(frame_rate_frames_) / elapsed, 1000.0 * elapsed / frame_rate_frames_);
            frame_rate_frames_ = 0;
            frame_rate_start_ = now;
        }
    }

    bool Viewer::draw_frame() {
        if (callback_error_) std::rethrow_exception(callback_error_);
        if (should_close() || glfwGetWindowAttrib(window_, GLFW_ICONIFIED)) return false;
        int w = 0, h = 0;
        framebuffer_size(w, h);
        if (w <= 0 || h <= 0) return false;
        glfwMakeContextCurrent(window_);
        if (!initialized_) { init(); initialized_ = true; }
        print_usage();
        if (is_animating()) {
            animation_func_(this);
            if (callback_error_) std::rethrow_exception(callback_error_);
            if (should_close()) return false;
        }
        advance_camera_path();
        notify_pre_draw();
        if (callback_error_) std::rethrow_exception(callback_error_);
        if (should_close()) return false;
        pre_draw();
        const bool drawn = draw();
        if (drawn) { update_frame_rate(); post_draw(); }
        glfwSwapBuffers(window_);
        notify_post_draw();
        if (callback_error_) std::rethrow_exception(callback_error_);
        return true;
    }

    int Viewer::run(bool see_all) {
        try {
            if (callback_error_) std::rethrow_exception(callback_error_);
            if (!initialized_) { init(); initialized_ = true; }
            if (see_all) fit_screen();
            if (visible_) show();
            return run_frames();
        } catch (const std::exception& e) {
            LOG(ERROR) << "Viewer failed: " << e.what();
            return EXIT_FAILURE;
        }
    }

    int Viewer::run_frames(unsigned max_frames) {
        try {
            if (callback_error_) std::rethrow_exception(callback_error_);
            return detail::run_viewer_frames(*this, max_frames, [this] {
                return is_animating() || show_frame_rate_ || kfi_->is_interpolation_started();
            });
        } catch (const std::exception& e) {
            LOG(ERROR) << "Viewer failed: " << e.what();
            return EXIT_FAILURE;
        }
    }

    void Viewer::exit() { request_close(); }


    void Viewer::init() {
        glfwMakeContextCurrent(window_);
        //// back face culling?
        //glEnable(GL_CULL_FACE);
        //glCullFace(GL_BACK); // culls only the back faces

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glClearDepth(1.0f);
        glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);

        // camera is manipulated by the mouse, working in the screen coordinate system
        // (different from the viewport or framebuffer size, which are in pixel coordinates)
        if (camera_->screenWidth() != width_ || camera_->screenHeight() != height_)
            camera_->setScreenWidthAndHeight(width_, height_);
        int fw, fh;
        glfwGetFramebufferSize(window_, &fw, &fh);
        glViewport(0, 0, fw, fh);

        // create TextRenderer renderer and load default fonts
        texter_ = std::unique_ptr<TextRenderer>(new TextRenderer(dpi_scaling()));
        texter_->add_font(resource::directory() + "/fonts/en_Earth-Normal.ttf");
        texter_->add_font(resource::directory() + "/fonts/en_Roboto-Medium.ttf");

        initialized_ = true;
        print_usage();
    }


    void Viewer::set_animation(bool b) {
        is_animating_ = b;
        update();
    }


    bool Viewer::is_animating() const {
        return is_animating_ && animation_func_;
    }


    Model* Viewer::add_model(const std::string &file_path, bool create_default_drawables) {
        glfwMakeContextCurrent(window_);
        const std::string file_name = file_system::convert_to_native_style(file_path);
        for (auto m : models_) {
            if (m->name() == file_name) {
                LOG(WARNING) << "model has already been added to the viewer: " << file_name;
                return m.get();
            }
        }

        const std::string &ext = file_system::extension(file_name, true);
        bool is_ply_mesh = false;
        if (ext == "ply")
            is_ply_mesh = (io::PlyReader::num_instances(file_name, "face") > 0);

        Model* model = nullptr;
        if ((ext == "ply" && is_ply_mesh) || ext == "obj" || ext == "off" || ext == "stl" || ext == "sm" || ext == "geojson" || ext == "trilist") { // mesh
            model = SurfaceMeshIO::load(file_name);
        }
        else if (ext == "ply" && io::PlyReader::num_instances(file_name, "edge") > 0) {
            model = GraphIO::load(file_name);
        }
        else if (ext == "plm" || ext == "pm" || ext == "mesh") {
            model = PolyMeshIO::load(file_name);
        }
        else { // point cloud
            if (ext == "ptx") {
                io::PointCloudIO_ptx serializer(file_name);
                PointCloud* cloud = nullptr;
                while ((cloud = serializer.load_next())) {
                    model = add_model(std::shared_ptr<PointCloud>(cloud), create_default_drawables);
                    update();
                }
                return model;   // returns the last cloud in the file.
            } else
                model = PointCloudIO::load(file_name);
        }

        if (model) {
            model->set_name(file_name);
            add_model(std::shared_ptr<Model>(model), create_default_drawables);
        }
        return model;
    }


    Model* Viewer::add_model(std::shared_ptr<Model> model, bool create) {
        glfwMakeContextCurrent(window_);
        if (!model) {
            LOG(WARNING) << "model is nullptr";
            return nullptr;
        }
        for (auto m : models_) {
            if (model == m) {
                LOG(WARNING) << "model has already been added to the viewer: " << m->name();
                return nullptr;
            }
        }

        model->set_renderer(std::make_shared<Renderer>(model.get(), create));

        int pre_idx = model_idx_;
        models_.push_back(model);
        model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current

        if (model_idx_ != pre_idx) {
            if (model_idx_ >= 0)
                LOG(INFO) << "current model: " << model_idx_ << ", " << models_[model_idx_]->name();
        }
        return model.get();
    }


    Model* Viewer::add_model(Model* model, bool create_default_drawables) {
        return add_model(std::shared_ptr<Model>(model), create_default_drawables);
    }


    bool Viewer::delete_model(Model *model) {
        glfwMakeContextCurrent(window_);
        if (!model) {
            LOG(WARNING) << "model is nullptr";
            return false;
        }

        for (auto it = models_.begin(); it != models_.end(); ++it) {
            if (it->get() == model) {
                int pre_idx = model_idx_;
                const std::string name = model->name();
                models_.erase(it);
                model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current
                LOG(INFO) << "model deleted: " << name;

                if (model_idx_ != pre_idx) {
                    if (model_idx_ >= 0)
                        LOG(INFO) << "current model: " << model_idx_ << ", " << models_[model_idx_]->name();
                }
                return true;
            }
        }

        // if the model was not found
        LOG(WARNING) << "no such model: " << model->name();
        return false;
    }


    Drawable* Viewer::add_drawable(std::shared_ptr<Drawable> drawable) {
        if (!drawable) {
            LOG(WARNING) << "drawable is nullptr.";
            return nullptr;
        }
        for (auto d : drawables_) {
            if (drawable == d) {
                LOG(WARNING) << "drawable has already been added to the viewer.";
                return drawable.get();
            }
        }

        drawables_.push_back(drawable);
        return drawable.get();
    }


    Drawable* Viewer::add_drawable(Drawable* drawable) {
        return add_drawable(std::shared_ptr<Drawable>(drawable));
    }


    bool Viewer::delete_drawable(Drawable *drawable) {
        glfwMakeContextCurrent(window_);
        if (!drawable) {
            LOG(WARNING) << "drawable is nullptr";
            return false;
        }

        for (auto it = drawables_.begin(); it != drawables_.end(); ++it) {
            if (it->get() == drawable) {
                drawables_.erase(it);
                return true;
            }
        }

        // if the drawable was not found
        LOG(WARNING) << "no such drawable: " << drawable->name();
        return false;
    }


    void Viewer::fit_screen(const easy3d::Model *model) {
        if (!model && models_.empty() && drawables_.empty()) {
            camera_->showEntireScene();
            return;
        }

        auto visual_box = [](const Model* m) -> Box3 {
            Box3 box = m->bounding_box();
            for (auto d : m->renderer()->points_drawables()) box.grow(d->bounding_box());
            for (auto d : m->renderer()->lines_drawables()) box.grow(d->bounding_box());
            for (auto d : m->renderer()->triangles_drawables()) box.grow(d->bounding_box());
            return box;
        };

        Box3 box;
        if (model)
            box = visual_box(model);
        else {
            for (auto m : models_)
                box.grow(visual_box(m.get()));
            for (auto d : drawables_)
                box.grow(d->bounding_box());
        }

        if (box.is_valid()) {
            camera_->setSceneBoundingBox(box.min_point(), box.max_point());
            camera_->showEntireScene();
            ClippingPlane::instance()->fit_scene(camera()->sceneCenter(), camera()->sceneRadius());
            update();
        }
    }


    bool Viewer::open() {
        const std::string title("Please choose a file");
        const std::string &default_path = resource::directory() + "/data/";
        const std::vector<std::string> &filters = {
                "Surface Mesh (*.obj *.ply *.off *.stl *.sm *.geojson *.trilist)", "*.obj *.ply *.off *.stl *.sm *.geojson *.trilist",
                "Point Cloud (*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg *.ptx)", "*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg *.ptx",
                "Polyhedral Mesh (*.plm *.pm *.mesh)", "*.plm *.pm *.mesh",
                "Graph (*.ply)", "*.ply",
                "All Files (*.*)", "*"
        };
        const std::vector<std::string> &file_names = dialog::open(title, default_path, filters, true);

        int count = 0;
        for (const auto &file_name : file_names) {
            if (add_model(file_name))
                ++count;
        }

        if (count > 0) {
            fit_screen();
            return true;
        }
        return false;
    }


    bool Viewer::save() const {
        Model* m = current_model();
        if (!m) {
            LOG(ERROR) << "no model exists";
            return false;
        }

        const std::string &title = "Please choose a file name";
        const std::vector<std::string> &filters = {
                "Surface Mesh (*.obj *.ply *.off *.stl *.sm)", "*.obj *.ply *.off *.stl *.sm",
                "Point Cloud (*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg)",
                "*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg",
                "Polyhedral Mesh (*.plm *.pm *.mesh)", "*.plm *.pm *.mesh",
                "Graph (*.ply)", "*.ply",
                "All Files (*.*)", "*"
        };

        std::string default_file_name = m->name();
        if (file_system::extension(default_file_name).empty()) // no extension?
            default_file_name += ".ply"; // default to ply

        const bool warn_overwrite = true;
        const std::string &file_name = dialog::save(title, default_file_name, filters, warn_overwrite);
        if (file_name.empty())
            return false;

        bool saved = false;
        if (dynamic_cast<const PointCloud *>(m))
            saved = PointCloudIO::save(file_name, dynamic_cast<const PointCloud *>(m));
        else if (dynamic_cast<const SurfaceMesh *>(m))
            saved = SurfaceMeshIO::save(file_name, dynamic_cast<const SurfaceMesh *>(m));
        else if (dynamic_cast<const Graph *>(m))
            saved = GraphIO::save(file_name, dynamic_cast<const Graph *>(m));

        if (saved)
            LOG(INFO) << "file successfully saved";

        return saved;
    }


    bool Viewer::snapshot() const {
        const std::string &title = "Please choose a file name";
        std::string default_file_name("untitled.png");
        if (current_model())
            default_file_name = file_system::replace_extension(current_model()->name(), "png");
        const std::vector<std::string> &filters = {
                "Image Files (*.png *.jpg *.bmp *.tga)", "*.png *.jpg *.bmp *.tga",
                "All Files (*.*)", "*"
        };

        const bool warn_overwrite = true;
        const std::string &file_name = dialog::save(title, default_file_name, filters, warn_overwrite);
        if (file_name.empty())
            return false;

        const std::string &ext = file_system::extension(file_name, true);
        if (ext != "png" && ext != "jpg" && ext != "bmp" && ext != "tga") {
            LOG(ERROR) << "snapshot format must be png, jpg, bmp, or tga";
            return false;
        }

        return snapshot(file_name, 1.0f, samples_);
    }


    void Viewer::draw_corner_axes() const {
        ShaderProgram *program = ShaderManager::get_program("surface/surface");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                    ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                    ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord"),
                    ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"),
                    ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal")
            };
            program = ShaderManager::create_program_from_files("surface/surface", attributes);
        }
        if (!program)
            return;

        if (!drawable_axes_) {
            float base = 0.5f;   // the cylinder length, relative to the allowed region
            float head = 0.2f;   // the cone length, relative to the allowed region
            std::vector<vec3> points, normals, colors;
            shape::create_cylinder(0.03, 10, vec3(0, 0, 0), vec3(base, 0, 0), vec3(1, 0, 0), points, normals, colors);
            shape::create_cylinder(0.03, 10, vec3(0, 0, 0), vec3(0, base, 0), vec3(0, 1, 0), points, normals, colors);
            shape::create_cylinder(0.03, 10, vec3(0, 0, 0), vec3(0, 0, base), vec3(0, 0, 1), points, normals, colors);
            shape::create_cone(0.06, 20, vec3(base, 0, 0), vec3(base + head, 0, 0), vec3(1, 0, 0), points, normals,
                                 colors);
            shape::create_cone(0.06, 20, vec3(0, base, 0), vec3(0, base + head, 0), vec3(0, 1, 0), points, normals,
                                 colors);
            shape::create_cone(0.06, 20, vec3(0, 0, base), vec3(0, 0, base + head), vec3(0, 0, 1), points, normals,
                                 colors);
            shape::create_sphere(vec3(0, 0, 0), 0.06, 20, 20, vec3(0, 1, 1), points, normals, colors);
            const_cast<Viewer*>(this)->drawable_axes_ = std::unique_ptr<TrianglesDrawable>(new TrianglesDrawable("corner_axes"));
            drawable_axes_->update_vertex_buffer(points);
            drawable_axes_->update_normal_buffer(normals);
            drawable_axes_->update_color_buffer(colors);
            drawable_axes_->set_property_coloring(State::VERTEX);
        }
        if (!drawable_axes_->is_visible())
            return;

        // The viewport is changed to fit the lower left corner.
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        static int corner_frame_size = static_cast<int>(100 * dpi_scaling());
        glViewport(0, 0, corner_frame_size, corner_frame_size);

        // To make the axis appear over other objects: reserve a tiny bit of the
        // front depth range. NOTE: do remember to restore it later.
        glDepthRange(0, 0.01f);

        const mat4 &proj = transform::ortho(-1, 1, -1, 1, -1, 1);
        const mat4 &view = camera_->orientation().inverse().matrix();
        const mat4 &MVP = proj * view;

        // camera position is defined in world coordinate system.
        const vec3 &wCamPos = camera_->position();
        // it can also be computed as follows:
        //const vec3& wCamPos = invMV * vec4(0, 0, 0, 1);
        const mat4 &MV = camera_->modelViewMatrix();
        const vec4 &wLightPos = inverse(MV) * setting::light_position;

        program->bind();
        program->set_uniform("MVP", MVP)
                ->set_uniform("MANIP", mat4::identity())
                ->set_uniform("NORMAL", mat3::identity())   // needs be padded when using uniform blocks
                ->set_uniform("lighting", true)
                ->set_uniform("two_sides_lighting", false)
                ->set_uniform("smooth_shading", true)
                ->set_uniform("wLightPos", wLightPos)
                ->set_uniform("wCamPos", wCamPos)
                ->set_uniform("ssaoEnabled", false)
                ->set_uniform("per_vertex_color", true)
                ->set_uniform("distinct_back_color", false)
                ->set_block_uniform("Material", "ambient", setting::material_ambient)
                ->set_block_uniform("Material", "specular", setting::material_specular)
                ->set_block_uniform("Material", "shininess", &setting::material_shininess)
                ->set_uniform("highlight", false)
                ->set_uniform("clippingPlaneEnabled", false)
                ->set_uniform("selected", false)
                ->set_uniform("highlight_color", setting::highlight_color)
                ->set_uniform("use_texture", false);
        drawable_axes_->gl_draw();
        program->release();

        // restore the viewport
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glDepthRange(0.0f, 1.0f);
    }


    void Viewer::pre_draw() {
        glfwMakeContextCurrent(window_);
        glClearColor(background_color_[0], background_color_[1], background_color_[2], 1.0f);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }


    void Viewer::post_draw() {
        if (texter_ && texter_->num_fonts() >=2) {
            const float font_size = 15.0f;
            float x = 20.0f * dpi_scaling();
            float y = 0.0f;
            // draw the Easy3D logo and GPU time
            if (show_easy3d_logo_) {
                y += font_size * 1.5f * dpi_scaling();
                texter_->draw("Easy3D", x, y, font_size, 0);
            }

            // draw the framerate
            if (show_frame_rate_) {
                y += font_size * 1.5f * dpi_scaling();
                texter_->draw(framerate_, x, y, font_size, 1);
            }

            if (!hint_.empty()) {
                x += font_size * dpi_scaling();
                y += font_size * 1.5f * dpi_scaling();
                texter_->draw(hint_, x, y, font_size, TextRenderer::ALIGN_LEFT, 1, vec3(0, 0, 1));
            }
        }

        // shown only when it is not animating
        if (show_camera_path_ && !kfi_->is_interpolation_started()) {
            kfi_->draw_path(camera(), 2.0f);
            kfi_->draw_cameras(camera(), camera()->sceneRadius() * 0.05f);
        }

        if (show_pivot_point_)
            drawable_pivot_->draw(camera());

        // ------------- draw the picking region with transparency  ---------------

        if (pressed_button_ == GLFW_MOUSE_BUTTON_LEFT && modifiers_ == MODIF_CTRL) {
            const Rect rect(
                static_cast<float>(mouse_pressed_x_),
                static_cast<float>(mouse_current_x_),
                static_cast<float>(mouse_pressed_y_),
                static_cast<float>(mouse_current_y_)
            );
            if (rect.width() > 0 || rect.height() > 0) {
                // draw the boundary of the rect
                shape::draw_quad_wire(rect, vec4(0.0f, 0.0f, 1.0f, 1.0f), width(), height(), -1.0f);
                // draw the transparent face
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                shape::draw_quad_filled(rect, vec4(0.0f, 0.0f, 1.0f, 0.2f), width(), height(), -0.9f);
                glDisable(GL_BLEND);
            }
        }

        // ------- draw the axes indicating the orientation of the model  ----------

        draw_corner_axes();
        draw_overlays(true);
    }


    void Viewer::draw_face_labels(Model *model, TextRenderer *texter, int font_id, const vec3 &color) const {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        const auto &points = mesh->points();
        for (auto f : mesh->faces()) {
            int count = 0;
            vec3 c(0, 0, 0);
            for (auto v : mesh->vertices(f)) {
                c += points[v.idx()];
                ++count;
            }
            c /= count;

            const vec3 p = camera()->projectedCoordinatesOf(c);
            const float x = p.x * dpi_scaling_;
            const float y = (static_cast<float>(height()) - 1.0f - p.y) * dpi_scaling_;

            float depth = 1.0f;
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            easy3d_debug_log_gl_error
            glReadPixels(static_cast<int>(x), static_cast<int>(y), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
            easy3d_debug_log_gl_error
            if (depth < 1.0 && p.z < 1.0 && std::abs(depth - p.z) < 0.001)
                texter->draw(std::to_string(f.idx()), x, p.y * dpi_scaling_, 16, font_id, color);
        }
    }


    void Viewer::draw_vertex_labels(Model *model, TextRenderer *texter, int font_id, const vec3 &color) const {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        const auto &points = mesh->points();
        for (std::size_t id = 0; id < points.size(); ++id) {
            const vec3& v = points[id];
            const vec3 p = camera()->projectedCoordinatesOf(v);
            const float x = p.x * dpi_scaling_;
            const float y = (static_cast<float>(height()) - 1.0f - p.y) * dpi_scaling_;

            float depth = 1.0f;
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            easy3d_debug_log_gl_error
            glReadPixels(static_cast<int>(x), static_cast<int>(y), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
            easy3d_debug_log_gl_error
            if (depth < 1.0 && p.z < 1.0 && std::abs(depth - p.z) < 0.001)
                texter->draw(std::to_string(id), x, p.y * dpi_scaling_, 16, font_id, color);
        }
    }


    void Viewer::copy_view() {
        const vec3 pos = camera()->position();
        const quat q = camera()->orientation();

        std::stringstream stream;
        stream << pos[0] << ' '
                << pos[1] << ' '
                << pos[2] << ' '
                << q[0] << ' '
                << q[1] << ' '
                << q[2] << ' '
                << q[3];
        glfwSetClipboardString(window_, stream.str().c_str());
    }


    void Viewer::paste_view() {
        // get the camera parameters from clipboard string
        const std::string str = glfwGetClipboardString(window_);
        std::vector<std::string> list;
        string::split(str, ' ', list, true);
        if (list.size() != 7) {
            LOG(WARNING) << "camera not available in clipboard";
            return;
        }

        vec3 pos;
        for (int i = 0; i < 3; ++i)
            pos[i] = std::stof(list[i]);
        quat orient;
        for (int i = 0; i < 4; ++i)
            orient[i] = std::stof(list[i + 3]);

        //////////////////////////////////////////////////////////////////////////
        // change view directly
        // 	camera()->setPosition(pos);
        // 	camera()->setOrientation(orient);
        // I prefer to make animation
        Frame new_frame;
        new_frame.setPosition(pos);
        new_frame.setOrientation(orient);
        const float duration = 0.5f;
        camera()->interpolateTo(&new_frame, duration);

        update();
    }


    bool Viewer::draw() {
#if 0
        // Example code: split the view to have different rendering styles for the same object/scene.
        glEnable(GL_SCISSOR_TEST);
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glScissor(viewport[0], viewport[1], viewport[2] * 0.5f, viewport[3]);
        for (const auto m : models_) {
            for (auto d : m->renderer()->triangles_drawables())
                d->draw(camera());
        }
        glScissor(viewport[2] * 0.5f, viewport[1], viewport[2] * 0.5f, viewport[3]);
        for (const auto m : models_) {
            for (auto d : m->renderer()->lines_drawables())
                d->draw(camera());
        }
        glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);

#else

        for (const auto m : models_) {
            if (!m->renderer()->is_visible())
                continue;

            // Let's check if edges and surfaces are both shown. If true, we
            // make the depth coordinates of the surface smaller, so that displaying
            // the mesh and the surface together does not cause Z-fighting.
            std::size_t count = 0;
            for (auto d : m->renderer()->lines_drawables()) {
                if (d->is_visible()) {
                    d->draw(camera());
                    easy3d_debug_log_gl_error
                    ++count;
                }
            }

            for (auto d : m->renderer()->points_drawables()) {
                if (d->is_visible())
                    d->draw(camera());
                easy3d_debug_log_gl_error
            }

            if (count > 0) {
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(0.5f, -0.0001f);
            }
            for (auto d : m->renderer()->triangles_drawables()) {
                if (d->is_visible())
                    d->draw(camera());
                easy3d_debug_log_gl_error
            }
            if (count > 0)
                glDisable(GL_POLYGON_OFFSET_FILL);
        }

        for (auto d : drawables_) {
            if (d->is_visible())
                d->draw(camera());
        }

#if 0 // draw face labels and vertex labels
        if (dynamic_cast<SurfaceMesh *>(current_model()) && texter_ && texter_->num_fonts() > 1) {
            draw_face_labels(current_model(), texter_, 1, vec3(0, 0, 1));
            draw_vertex_labels(current_model(), texter_, 1, vec3(0, 1, 0));
        }
#endif
#endif

        // draw the clipping plane (not in postDraw so it can be effective when taking snapshots)
        ClippingPlane::instance()->draw(camera()); easy3d_debug_log_gl_error
        draw_overlays(false);
        return true;
    }

}

namespace easy3d {
namespace {
struct QueryContext {
    GLFWwindow *previous;
    explicit QueryContext(GLFWwindow *window) : previous(glfwGetCurrentContext()) { glfwMakeContextCurrent(window); }
    ~QueryContext() { glfwMakeContextCurrent(previous); }
};
using opengl::QueryState;
void query_draw(const Drawable *drawable, const Camera &camera) {
    if (!drawable || !drawable->is_visible() || dynamic_cast<const LinesDrawable2D *>(drawable))
        return;
    // Standard drawable rendering only, matching the native scene encoder.
    // Custom Viewer::draw() and application frame hooks do not run here.
    if (auto *d = dynamic_cast<const TrianglesDrawable *>(drawable))
        d->TrianglesDrawable::draw(&camera);
    else if (auto *d = dynamic_cast<const LinesDrawable *>(drawable))
        d->LinesDrawable::draw(&camera);
    else if (auto *d = dynamic_cast<const PointsDrawable *>(drawable))
        d->PointsDrawable::draw(&camera);
    else
        throw std::invalid_argument("unsupported drawable in scene depth query");
}
} // namespace
const std::vector<vec4>& Viewer::clipping_planes() const {
    selection_planes_.clear();
    const auto* clipping=ClippingPlane::instance();
    if(clipping->is_enabled()) {
        const auto n=clipping->normal(),c=clipping->center();
        selection_planes_.push_back(vec4(-n,dot(n,c)));
        if(clipping->cross_section()) selection_planes_.push_back(vec4(n,-dot(n,c)+clipping->cross_section_width()*clipping->scene_radius()));
    }
    selection_planes_=graphics::normalize_clipping_planes(selection_planes_);
    return selection_planes_;
}
std::shared_ptr<const graphics::PointData> Viewer::point_resource(Drawable* drawable) const {
    if(!drawable || drawable->type()!=Drawable::DT_POINTS) throw std::invalid_argument("selection requires a point drawable");
    std::shared_ptr<Drawable> owner;
    for(const auto& model:models_) if(model->renderer())
        for(const auto& d:model->renderer()->points_drawables()) if(d.get()==drawable) owner=d;
    for(const auto& d:drawables_) if(d.get()==drawable) owner=d;
    if(!owner) throw std::invalid_argument("point drawable does not belong to this viewer");
    QueryContext context(window_);
    if(GLEW_VERSION_4_0 || GLEW_ARB_transform_feedback2) {
        GLint running=0;glGetIntegerv(GL_TRANSFORM_FEEDBACK_ACTIVE,&running);
        if(running) throw std::logic_error("selection cannot interrupt active transform feedback");
    }
    QueryState state;state.prepare(framebuffer_width(),framebuffer_height());
    if(drawable->update_needed_ || (!drawable->vertex_buffer_ && (drawable->model_ || drawable->update_func_))) {
        drawable->update_buffers_internal();drawable->update_needed_=false;
    }
    if(!point_selector_) point_selector_.reset(new opengl::PointSelector);
    return point_selector_->points(owner,drawable->point_revision_);
}
std::vector<std::uint32_t> Viewer::select_points(const graphics::PointSelectionRequest& request) const {
    QueryContext context(window_);
    if(GLEW_VERSION_4_0 || GLEW_ARB_transform_feedback2) {
        GLint running=0;glGetIntegerv(GL_TRANSFORM_FEEDBACK_ACTIVE,&running);
        if(running) throw std::logic_error("selection cannot interrupt active transform feedback");
    }
    QueryState state;state.prepare(framebuffer_width(),framebuffer_height());
    if(!point_selector_) point_selector_.reset(new opengl::PointSelector);
    return point_selector_->select(request);
}
void Viewer::set_overlays(std::vector<graphics::DrawCommand> commands,bool foreground) {
    QueryContext context(window_);
    QueryState state;state.prepare(framebuffer_width(),framebuffer_height());
    if(!overlay_renderer_) overlay_renderer_.reset(new opengl::OverlayRenderer);
    overlay_renderer_->set(std::move(commands),foreground);
    update();
}
void Viewer::draw_overlays(bool foreground) const {
    if(!overlay_renderer_) return;
    QueryState state;
    GLint viewport[4];glGetIntegerv(GL_VIEWPORT,viewport);
    state.prepare(viewport[2],viewport[3]);glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
    overlay_renderer_->draw(foreground,framebuffer_width(),framebuffer_height(),overlay_tile_,[&](int width,int height) {
        QueryState depth_state;depth_state.prepare(width,height);
        Camera snapshot(*camera());snapshot.frame()->setPivotPoint(camera()->pivotPoint());
        snapshot.setScreenWidthAndHeight(width,height);snapshot.projectionMatrix();snapshot.modelViewMatrix();
        mat4 projection=camera()->projectionMatrix();
        if(overlay_tile_.z>0) {
            mat4 full=mat4::identity();
            full(0,0)=float(viewport[2])/width;full(1,1)=float(viewport[3])/height;
            full(0,3)=float(2*overlay_tile_.x+viewport[2])/width-1;
            full(1,3)=1-float(2*overlay_tile_.y+viewport[3])/height;
            projection=full*projection;
        }
        snapshot.set_projection_matrix(projection);snapshot.set_modelview_matrix(camera()->modelViewMatrix());
        draw_scene(snapshot);
    });
}
graphics::FrameImage Viewer::capture() const {return capture(samples());}
graphics::FrameImage Viewer::capture(unsigned count) const {
    if(!count || count>64 || (count&(count-1))) throw std::invalid_argument("invalid capture sample count");
    QueryContext context(window_);QueryState state;
    GLint maximum=0,extent=0;glGetIntegerv(GL_MAX_SAMPLES,&maximum);glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE,&extent);
    const int w=framebuffer_width(),h=framebuffer_height();
    if(w<=0 || h<=0 || w>extent || h>extent || (count>1 && count>unsigned(maximum)))
        throw std::invalid_argument("unsupported capture dimensions or samples");
    state.prepare(w,h);
    auto* self=const_cast<Viewer*>(this);
    if(!initialized_) {self->init();self->initialized_=true;}
    Camera snapshot(*camera());snapshot.frame()->setPivotPoint(camera()->pivotPoint());
    snapshot.setScreenWidthAndHeight(w,h);snapshot.projectionMatrix();snapshot.modelViewMatrix();
    snapshot.set_projection_matrix(camera()->projectionMatrix());snapshot.set_modelview_matrix(camera()->modelViewMatrix());
    for(unsigned i=0;i<16;++i) if(!std::isfinite(snapshot.modelViewProjectionMatrix()[i]))
        throw std::invalid_argument("nonfinite capture camera");
    graphics::FrameImage frame;frame.width=w;frame.height=h;
    {
        FramebufferObject color(w,h,count<=1?0:int(count));
        if(!color.add_color_buffer() || !color.add_depth_buffer() || !color.bind()) throw std::runtime_error("cannot create capture color target");
        state.prepare(w,h);glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
        const auto bg=background_color();glClearColor(bg.x,bg.y,bg.z,bg.w);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
        draw_scene(snapshot,nullptr,true);glViewport(0,0,w,h);draw_overlays(false);
        if(snapshot_hud_) self->Viewer::post_draw();else draw_overlays(true);
        glDisable(GL_SCISSOR_TEST);glBindBuffer(GL_PIXEL_PACK_BUFFER,0);
        if(!color.read_color(0,frame.rgba,GL_RGBA,true)) throw std::runtime_error("cannot read capture color");
    }
    FramebufferObject depth(w,h,0);
    if(!depth.add_color_buffer() || !depth.add_depth_buffer(GL_DEPTH_COMPONENT32F) || !depth.bind()) throw std::runtime_error("cannot create capture depth target");
    frame.depth.assign(size_t(w)*h,1);frame.ids.assign(size_t(w)*h,{UINT32_MAX,UINT32_MAX});
    std::vector<float> candidate(frame.depth.size());
    const auto append=[&](const std::shared_ptr<Model>& model,const std::shared_ptr<Drawable>& drawable,uint32_t id) {
        if(!drawable->is_visible() || dynamic_cast<LinesDrawable2D*>(drawable.get())) return;
        state.prepare(w,h);glClear(GL_DEPTH_BUFFER_BIT);
        DrawSource source{model,drawable};draw_scene(snapshot,&source);
        glReadPixels(0,0,w,h,GL_DEPTH_COMPONENT,GL_FLOAT,candidate.data());
        for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
            const auto p=size_t(y)*w+x;const auto d=candidate[size_t(h-1-y)*w+x];
            if(d>=0 && d<frame.depth[p]) {frame.depth[p]=d;frame.ids[p]={id,UINT32_MAX};}
        }
    };
    if(models_.size()+drawables_.size()>=UINT32_MAX) throw std::length_error("too many capture objects");
    for(size_t i=0;i<models_.size();++i) {
        const auto& model=models_[i];if(!model->renderer()) continue;
        for(const auto& d:model->renderer()->triangles_drawables()) append(model,d,uint32_t(i));
        for(const auto& d:model->renderer()->lines_drawables()) append(model,d,uint32_t(i));
        for(const auto& d:model->renderer()->points_drawables()) append(model,d,uint32_t(i));
    }
    for(size_t i=0;i<drawables_.size();++i) append(nullptr,drawables_[i],uint32_t(models_.size()+i));
    return frame;
}
Viewer::PickResult Viewer::pick(int x, int y) const {
    PickResult result;
    const int fw=framebuffer_width(), fh=framebuffer_height();
    if (x<0 || y<0 || x>=fw || y>=fh) return result;
    QueryContext context(window_);
    QueryState state;
    Camera snapshot(*camera());
    snapshot.setScreenWidthAndHeight(fw,fh);
    snapshot.projectionMatrix(); snapshot.modelViewMatrix();
    snapshot.set_projection_matrix(camera()->projectionMatrix());
    snapshot.set_modelview_matrix(camera()->modelViewMatrix());
    state.prepare(fw,fh);
    FramebufferObject target(fw,fh,0);
    if (!target.add_color_buffer() || !target.add_depth_buffer(GL_DEPTH_COMPONENT32F) || !target.is_valid() || !target.bind())
        throw std::runtime_error("cannot create picking framebuffer");
    // Reuse the actual drawable shaders: point impostors, clipping and transforms
    // must agree with scene depth. Keep the renderer's shared ownership intact.
    const auto sample=[&](const std::shared_ptr<Drawable>& drawable,const std::shared_ptr<Model>& model) {
        if (!drawable->is_visible() || dynamic_cast<LinesDrawable2D*>(drawable.get())) return;
        state.prepare(fw,fh);
        glClear(GL_DEPTH_BUFFER_BIT);
        DrawSource source{model,drawable};
        draw_scene(snapshot,&source);
        float depth=1;
        glReadPixels(x,fh-1-y,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&depth);
        if (depth>=0 && depth<result.depth) {
            result.model=model;result.drawable=drawable;result.depth=depth;
        }
    };
    for (const auto& model:models_) {
        const auto* renderer=model->renderer();
        if (!renderer) continue;
        for (const auto& d:renderer->triangles_drawables()) sample(d,model);
        for (const auto& d:renderer->lines_drawables()) sample(d,model);
        for (const auto& d:renderer->points_drawables()) sample(d,model);
    }
    for (const auto& d:drawables_) sample(d,nullptr);
    if (!result) return result;
    const auto world=[&](float depth) {return unproject_pixel(x,y,depth);};
    result.position=world(result.depth);
    if (result.drawable->type()==Drawable::DT_TRIANGLES) {
        // Recover triangle provenance from the winning drawable's existing buffers,
        // without a second, differently clipped ID shader or a persistent CPU copy.
        const auto& d=*result.drawable;
        std::vector<vec3> vertices(d.num_vertices());
        std::vector<unsigned> indices(d.num_indices());
        GLint previous=0;glGetIntegerv(GL_COPY_READ_BUFFER_BINDING,&previous);
        glBindBuffer(GL_COPY_READ_BUFFER,d.vertex_buffer());
        glGetBufferSubData(GL_COPY_READ_BUFFER,0,GLsizeiptr(vertices.size()*sizeof(vec3)),vertices.data());
        if (!indices.empty()) {
            glBindBuffer(GL_COPY_READ_BUFFER,d.element_buffer());
            glGetBufferSubData(GL_COPY_READ_BUFFER,0,GLsizeiptr(indices.size()*sizeof(unsigned)),indices.data());
        }
        glBindBuffer(GL_COPY_READ_BUFFER,GLuint(previous));
        const vec3 ray_origin=world(0), ray_direction=world(.5f)-ray_origin;
        const mat4 model=d.manipulated_matrix();
        float closest=std::numeric_limits<float>::max();
        const size_t count=indices.empty()?vertices.size():indices.size();
        for (size_t i=0;i+2<count;i+=3) {
            vec3 triangle[3];bool valid=true;
            for(unsigned j=0;j<3;++j) {
                const size_t index=indices.empty()?i+j:indices[i+j];
                if (index>=vertices.size()) {valid=false;break;}
                triangle[j]=(model*vec4(vertices[index],1)).xyz();
            }
            if (!valid) continue;
            const vec3 e1=triangle[1]-triangle[0],e2=triangle[2]-triangle[0];
            const vec3 h=cross(ray_direction,e2);const float det=dot(e1,h);
            if (std::abs(det)<=std::numeric_limits<float>::epsilon()*e1.length()*h.length()) continue;
            const vec3 offset=ray_origin-triangle[0];
            const float u=dot(offset,h)/det;
            const vec3 q=cross(offset,e1);
            const float v=dot(ray_direction,q)/det,t=dot(e2,q)/det;
            if (u<0 || v<0 || u+v>1 || t<0) continue;
            const float distance=(ray_origin+t*ray_direction-result.position).length();
            if (distance<closest) {closest=distance;result.primitive=uint32_t(i/3);}
        }
        auto* mesh=dynamic_cast<SurfaceMesh*>(result.model.get());
        if (mesh && result.primitive!=UINT32_MAX &&
            result.model->renderer()->get_triangles_drawable("faces",false)==result.drawable.get()) {
            const auto ranges=mesh->get_face_property<std::pair<int,int>>("f:triangle_range");
            if (ranges) for (auto face:mesh->faces()) {
                const auto range=ranges[face];
                if (range.first>=0 && result.primitive>=uint32_t(range.first) && result.primitive<=uint32_t(range.second)) {
                    result.face_index=face.idx();break;
                }
            }
        }
    }
    return result;
}
bool Viewer::read_scene_depth(int x, int y, const Camera &snapshot, float &depth) const {
    QueryContext context(window_);
    QueryState state;
    state.prepare(snapshot.screenWidth(), snapshot.screenHeight());
    FramebufferObject target(snapshot.screenWidth(), snapshot.screenHeight(), 0);
    if (!target.add_color_buffer() || !target.add_depth_buffer(GL_DEPTH_COMPONENT32F) || !target.is_valid())
        throw std::runtime_error("cannot create single-sample scene depth framebuffer");
    if (!target.bind())
        throw std::runtime_error("cannot bind scene depth framebuffer");
    glClear(GL_DEPTH_BUFFER_BIT);
    draw_scene(snapshot);
    glReadPixels(x, snapshot.screenHeight() - 1 - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    return true;
}
void Viewer::draw_scene(const Camera& snapshot,const DrawSource* source,bool guides) const {
    if(source) {
        if(source->model && (!source->model->renderer() || !source->model->renderer()->is_visible())) return;
        query_draw(source->drawable.get(),snapshot);
        return;
    }
    for (const auto &model : models_) {
        const auto *renderer = model->renderer();
        if (!renderer || !renderer->is_visible())
            continue;
        for (const auto &d : renderer->triangles_drawables())
            query_draw(d.get(), snapshot);
        for (const auto &d : renderer->lines_drawables())
            query_draw(d.get(), snapshot);
        for (const auto &d : renderer->points_drawables())
            query_draw(d.get(), snapshot);
    }
    for (const auto &drawable : drawables_)
        query_draw(drawable.get(), snapshot);
    if(guides) {
        ClippingPlane::instance()->draw(&snapshot);
        glDisable(GL_DEPTH_TEST);glDepthMask(GL_FALSE);
        for(const auto& d:drawables_) if(d->is_visible())
            if(auto* flat=dynamic_cast<LinesDrawable2D*>(d.get())) flat->draw(&snapshot);
        glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);
    }
}
} // namespace easy3d

#endif   // !EASY3D_HAS_VULKAN

namespace easy3d {
vec3 Viewer::unproject_pixel(int x,int y,float depth) const {
    const int fw=framebuffer_width(),fh=framebuffer_height();
    if(x<0 || y<0 || x>=fw || y>=fh || !std::isfinite(depth) || depth<0 || depth>1)
        throw std::invalid_argument("invalid framebuffer pixel/depth for unprojection");
    return unproject_point(vec3(float((x+.5)*width()/fw),float((y+.5)*height()/fh),depth));
}
void Viewer::advance_camera_path() {
    if (!kfi_->is_interpolation_started()) return;
    const auto now=std::chrono::steady_clock::now();
    if (now<next_camera_path_frame_) return;
    kfi_->advance_interpolation();
    next_camera_path_frame_=now+std::chrono::milliseconds(kfi_->interpolation_period());
}
} // namespace easy3d
