/** \cond LICENSE
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
 \endcond */

#include <easy3d/viewer/viewer.h>

// Standard headers used by the backend-independent section below.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <unordered_map>


// Headers shared by both backends.
#include <easy3d/core/model.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/frustum.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/shadow.h>
#include <easy3d/renderer/soft_shadow.h>
#include <easy3d/renderer/ambient_occlusion.h>
#include <easy3d/renderer/eye_dome_lighting.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/point_cloud_io_ptx.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/poly_mesh_io.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/setting.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/resource.h>
#include <easy3d/renderer/text_layout.h>
#include <easy3d/renderer/drawable_renderer.h>
#include <easy3d/renderer/render_settings.h>
#include <easy3d/renderer/backends/clipping.h>
#include <easy3d/util/timer.h>

#ifdef EASY3D_HAS_VULKAN
// vulkan.h must precede glfw3.h (first pulled in by viewer_loop.h): GLFW declares
// glfwCreateWindowSurface only when VK_VERSION_1_0 is defined.
#include <vulkan/vulkan.h>
#endif
#include <easy3d/viewer/backends/glfw_runtime.h>
#include <easy3d/viewer/backends/viewer_loop.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h> // for glfw functions

#ifdef EASY3D_HAS_VULKAN
#include <easy3d/renderer/transparency.h>
#include <easy3d/renderer/eye_dome_lighting.h>
#include <easy3d/renderer/ambient_occlusion.h>
#include <easy3d/renderer/soft_shadow.h>
#include <easy3d/renderer/backends/vulkan/clipping_plane.h>
#include <easy3d/renderer/backends/vulkan/surface.h>
#else
#include <easy3d/viewer/multi_viewer.h>
#include <easy3d/renderer/opengl.h> // for gl functions
#include <easy3d/renderer/backends/opengl/overlay_renderer.h>
#include <easy3d/renderer/backends/opengl/query_state.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/soft_shadow.h>
#include <easy3d/renderer/ambient_occlusion.h>
#include <easy3d/renderer/eye_dome_lighting.h>
#include <easy3d/renderer/average_color_blending.h>
#include <easy3d/renderer/dual_depth_peeling.h>
#include <easy3d/renderer/per_pixel_linked_lists_oit.h>
#include <easy3d/renderer/opengl_util.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/util/string.h>
#include <easy3d/core/matrix.h>
#endif


namespace easy3d {
#ifndef EASY3D_HAS_VULKAN
    struct Viewer::GraphicsResources {
        std::unique_ptr<graphics::DrawableRenderer> drawable_renderer;
        std::unique_ptr<graphics::RenderDevice> query_device;
    };

    Viewer::GraphicsResources &Viewer::graphics_resources() const {
        if (!graphics_resources_)
            graphics_resources_.reset(new GraphicsResources);
        return *graphics_resources_;
    }
#endif

    namespace {
        // Light-space data for a declarative shadow frame. Computing it here keeps
        // the settings path independent of backend-specific legacy effect classes.
        struct SettingsShadowFrame {
            mat4 shadow_matrix;
            std::vector<vec3> background_corners;
            vec4 background_color = vec4(0);
            unsigned shadow_map_size = 1024;
            float darkness = 0;
            float softness = 0;
            float light_near = 1;
            float light_far = 2;
        };

        bool compute_settings_shadow_frame(const Camera &camera, const ShadowSettings &settings,
                                           SettingsShadowFrame &data) {
            if (!settings.enabled)
                return false;

            Frustum camera_frustum(Frustum::PERSPECTIVE);
            camera_frustum.set_perspective(float(camera.field_of_view()) + 0.2f,
                                           float(camera.aspect_ratio()),
                                           float(camera.z_near()), float(camera.z_far()));
            const vec3 &position = camera.position();
            camera_frustum.orient(position, position + camera.view_direction(), camera.up_vector());

            Frustum light_frustum(Frustum::PERSPECTIVE);
            const vec3 &center = camera.scene_center();
            const mat3 transform = transform::normal_matrix(inverse(camera.model_view_matrix()));
            vec3 direction = normalize(transform * vec3(-vec3(setting::light_position)));
            const float radius = camera.scene_radius();
            const vec3 light_position = center - direction * radius * std::max(1.1f, settings.light_distance);
            light_frustum.orient(light_position, center, transform * vec3(0, 1, 0));

            float max_z = -std::numeric_limits<float>::max();
            float min_z = std::numeric_limits<float>::max();
            const mat4 light_view = light_frustum.view_matrix();
            const vec4 center_in_light = light_view * vec4(center, 1.0f);
            max_z = std::max(max_z, center_in_light.z + radius);
            min_z = std::min(min_z, center_in_light.z - radius);
            if (!settings.virtual_background) {
                for (const auto &point : camera_frustum.vertices()) {
                    const vec4 projected = light_view * vec4(point, 1.0f);
                    min_z = std::min(min_z, projected.z);
                }
            }
            float near_distance = -max_z;
            float far_distance = -min_z;
            far_distance = std::min(far_distance, near_distance + radius * 10.0f);

            float max_x = -std::numeric_limits<float>::max();
            float max_y = -std::numeric_limits<float>::max();
            light_frustum.set_ortho(-1.0f, 1.0f, -1.0f, 1.0f, near_distance, far_distance);
            const mat4 initial_shadow_matrix = light_frustum.projection_matrix() * light_view;
            const vec4 center_projected = initial_shadow_matrix * vec4(center, 1.0f);
            max_x = std::max(max_x, center_projected.x + radius);
            max_y = std::max(max_y, center_projected.y + radius);
            near_distance = std::max(near_distance, 0.001f);
            light_frustum.set_frustum(max_x * 2.0f, max_y * 2.0f, near_distance, far_distance);

            if (settings.virtual_background) {
                const vec3 normal = normalize(transform * vec3(setting::light_position));
                const float offset = radius * 0.002f;
                const vec3 background_center = light_frustum.far_center() + normal * offset;
                const vec3 up = light_frustum.up_vector();
                const vec3 dx = normalize(cross(up, normal)) * light_frustum.far_width();
                const vec3 dy = normalize(up) * light_frustum.far_width();
                data.background_corners = {background_center - dx - dy, background_center + dx - dy,
                                           background_center + dx + dy, background_center - dx + dy};
            } else
                data.background_corners.clear();

            const mat4 clip_to_texture(
                vec4(0.5f, 0.0f, 0.0f, 0.0f),
                vec4(0.0f, 0.5f, 0.0f, 0.0f),
                vec4(0.0f, 0.0f, 0.5f, 0.0f),
                vec4(0.5f, 0.5f, 0.5f, 1.0f)
            );
            data.shadow_matrix = clip_to_texture * light_frustum.projection_matrix() * light_view;
            data.background_color = settings.virtual_background_color;
            data.shadow_map_size = settings.map_size;
            data.darkness = settings.darkness;
            data.softness = settings.softness;
            data.light_near = light_frustum.near_distance();
            data.light_far = light_frustum.far_distance();
            return true;
        }

        // This is the only builder of settings-driven effect commands. Both viewers
        // submit the result through graphics::RenderDevice; capture() never calls it.
        void build_settings_effects(std::vector<graphics::DrawCommand> &commands, const Camera &cam,
                                     int width, int height, const RenderSettings &settings) {
            if (width <= 0 || height <= 0)
                return;
            if (settings.shadow.enabled) {
                SettingsShadowFrame data;
                if (compute_settings_shadow_frame(cam, settings.shadow, data)) {
                    const mat4 clip_to_shadow = data.shadow_matrix * inverse(cam.model_view_projection_matrix());
                    for (auto &command : commands) {
                        if (command.overlay || !command.geometry ||
                            command.geometry->topology() != graphics::Topology::triangles)
                            continue;
                        command.shadow_caster = true;
                        command.shadow_matrix = clip_to_shadow;
                        command.shadow_map_size = data.shadow_map_size;
                    }
                    if (!data.background_corners.empty()) {
                        std::vector<graphics::Vertex> vertices(4);
                        for (unsigned i = 0; i < 4; ++i)
                            vertices[i].position = data.background_corners[i];
                        graphics::DrawCommand background;
                        background.geometry = std::make_shared<graphics::Geometry>(
                            std::move(vertices), std::vector<uint32_t>{0, 1, 2, 0, 2, 3});
                        background.color = data.background_color;
                        background.object_id = UINT32_MAX;
                        background.model_view_projection = cam.model_view_projection_matrix();
                        background.shadow_background = true;
                        background.shadow_matrix = clip_to_shadow;
                        background.shadow_map_size = data.shadow_map_size;
                        commands.push_back(std::move(background));
                    }
                    commands.insert(commands.begin(),
                                    graphics::shape::draw_shadow(unsigned(width), unsigned(height), clip_to_shadow,
                                                                 data.darkness, data.softness, data.light_near,
                                                                 data.light_far, data.shadow_map_size));
                }
            }
            if (settings.ambient_occlusion.enabled) {
                const auto &ao = settings.ambient_occlusion;
                const float radius = std::max(cam.scene_radius() * ao.radius_ratio, 1e-4f);
                commands.insert(commands.begin(),
                                graphics::shape::draw_ambient_occlusion(unsigned(width), unsigned(height), radius,
                                                                        ao.bias, ao.intensity, cam.projection_matrix()));
            }
            if (settings.eye_dome_lighting) {
                float z_near, z_far;
                if (cam.type() == Camera::PERSPECTIVE) {
                    z_near = cam.z_near();
                    z_far = cam.z_far();
                } else {
                    const float radius = cam.scene_radius();
                    z_near = 2.0f * radius;
                    z_far = 10.0f * radius;
                }
                commands.insert(commands.begin(),
                                graphics::shape::draw_eye_dome_lighting(unsigned(width), unsigned(height), z_near,
                                                                        z_far, 2.0f * cam.scene_radius()));
            }
        }

#ifdef EASY3D_HAS_VULKAN
        // The Vulkan facade publishes a complete frame for application-owned shadows.
        // The OpenGL facade keeps its effect objects private, so there an active
        // legacy shadow uses the exact native fallback instead.
        template <typename Source>
        void append_legacy_shadow(std::vector<graphics::DrawCommand> &commands,
                                  const std::vector<Source> &sources, const Camera &cam,
                                  int width, int height, Shadow *active_shadow) {
            if (!active_shadow || width <= 0 || height <= 0)
                return;
            Shadow::FrameData data;
            if (!Shadow::frame_data(&cam, data))
                return;
            const mat4 clip_to_shadow = data.shadow_matrix * inverse(cam.model_view_projection_matrix());
            const auto &surfaces = active_shadow->surfaces();
            for (size_t i = 0; i < sources.size() && i < commands.size(); ++i) {
                auto *triangles = dynamic_cast<TrianglesDrawable *>(sources[i].drawable.get());
                if (!triangles || std::find(surfaces.begin(), surfaces.end(), triangles) == surfaces.end())
                    continue;
                if (!commands[i].geometry || commands[i].geometry->topology() != graphics::Topology::triangles)
                    continue;
                commands[i].shadow_caster = true;
                commands[i].shadow_matrix = clip_to_shadow;
                commands[i].shadow_map_size = data.shadow_map_size;
            }
            if (!data.background_corners.empty()) {
                std::vector<graphics::Vertex> vertices(4);
                for (unsigned i = 0; i < 4; ++i)
                    vertices[i].position = data.background_corners[i];
                graphics::DrawCommand background;
                background.geometry = std::make_shared<graphics::Geometry>(
                    std::move(vertices), std::vector<uint32_t>{0, 1, 2, 0, 2, 3});
                background.color = data.background_color;
                background.object_id = UINT32_MAX;
                background.model_view_projection = cam.model_view_projection_matrix();
                background.shadow_background = true;
                background.shadow_matrix = clip_to_shadow;
                background.shadow_map_size = data.shadow_map_size;
                commands.push_back(std::move(background));
            }
            commands.insert(commands.begin(),
                            graphics::shape::draw_shadow(unsigned(width), unsigned(height), clip_to_shadow,
                                                         data.darkness, data.softness, data.light_near,
                                                         data.light_far, data.shadow_map_size));
        }
#endif

        void append_legacy_ao(std::vector<graphics::DrawCommand> &commands, const Camera &cam,
                              int width, int height) {
#ifdef EASY3D_HAS_VULKAN
            float radius_ratio, bias, intensity;
            mat4 projection;
            if (width <= 0 || height <= 0 ||
                !AmbientOcclusion::frame_params(&cam, radius_ratio, bias, intensity, projection))
                return;
            const float radius = std::max(cam.scene_radius() * radius_ratio, 1e-4f);
            commands.insert(commands.begin(), graphics::shape::draw_ambient_occlusion(
                                                  unsigned(width), unsigned(height), radius, bias, intensity, projection));
#else
            (void)commands;
            (void)cam;
            (void)width;
            (void)height;
#endif
        }

        void append_legacy_edl(std::vector<graphics::DrawCommand> &commands, const Camera &cam,
                               int width, int height) {
#ifdef EASY3D_HAS_VULKAN
            float z_near, z_far, scene_size;
            if (width <= 0 || height <= 0 ||
                !EyeDomeLighting::frame_params(&cam, z_near, z_far, scene_size))
                return;
            commands.insert(commands.begin(), graphics::shape::draw_eye_dome_lighting(
                                                  unsigned(width), unsigned(height), z_near, z_far, scene_size));
#else
            (void)commands;
            (void)cam;
            (void)width;
            (void)height;
#endif
        }
    }

    void Viewer::validate_render_settings_for_presentation() const {
        validate_render_settings(render_settings());
    }

    void Viewer::strip_settings_transparency(std::vector<graphics::DrawCommand> &commands,
                                             const std::vector<DrawSource> &sources,
                                             const std::vector<graphics::DrawCommand> &source_commands) const {
        if (!render_settings().transparency.enabled)
            return;
        for (size_t i = 0; i < commands.size(); ++i) {
            auto &command = commands[i];
            if (command.overlay || !command.geometry ||
                command.geometry->topology() != graphics::Topology::triangles)
                continue;
            command.transparent = false;
            // Source-mapped commands use compact IDs, while raw capture keeps
            // model/standalone IDs. Match by command position, not by those IDs.
            if (i >= source_commands.size())
                continue;
            const auto id = source_commands[i].object_id;
            if (id < sources.size() && sources[id].drawable)
                command.color.w = sources[id].drawable->color().w;
        }
    }

#ifndef EASY3D_HAS_VULKAN
    // 2.8 keeps Viewer's public layout unchanged; this scratch state is owned by
    // the implementation and erased by cleanup/destruction.
    struct ViewerOpenGLState {
        bool snapshot_active = false;
        int snapshot_width = 0, snapshot_height = 0;
        unsigned snapshot_samples = 1;
        vec4 snapshot_background = vec4(0);
        vec2 pivot_pixel = vec2(0);
        std::chrono::steady_clock::time_point pivot_expires;
        std::vector<graphics::DrawCommand> corner_axes_commands;
        RenderSettings render_settings;
        // Native effect objects remain for exact fallbacks; their framebuffers are
        // context resources, so the cache is released before the context is destroyed.
        std::unique_ptr<Shadow> settings_shadow;
        const Camera *settings_shadow_camera = nullptr;
        std::unique_ptr<AmbientOcclusion> settings_ao;
        std::unique_ptr<EyeDomeLighting> settings_edl;
        std::unique_ptr<AverageColorBlending> settings_average;
        std::unique_ptr<DualDepthPeeling> settings_peeling;
        std::unique_ptr<PerPixelLinkedListsOIT> settings_linked;
    };
    std::unordered_map<const Viewer *, ViewerOpenGLState> &opengl_viewer_states() {
        static std::unordered_map<const Viewer *, ViewerOpenGLState> states;
        return states;
    }
    ViewerOpenGLState &opengl_viewer_state(const Viewer *viewer) {
        return opengl_viewer_states()[viewer];
    }
    void clear_opengl_effect_cache(ViewerOpenGLState &state) {
        state.settings_shadow.reset();
        state.settings_shadow_camera = nullptr;
        state.settings_ao.reset();
        state.settings_edl.reset();
        state.settings_average.reset();
        state.settings_peeling.reset();
        state.settings_linked.reset();
    }
    void configure_opengl_snapshot(const Viewer *viewer, int width, int height,
                                   unsigned samples, const vec4 &background) {
        auto &state = opengl_viewer_state(viewer);
        state.snapshot_active = true;
        state.snapshot_width = width;
        state.snapshot_height = height;
        state.snapshot_samples = samples;
        state.snapshot_background = background;
    }
    void finish_opengl_snapshot(const Viewer *viewer) {
        opengl_viewer_state(viewer).snapshot_active = false;
    }
#endif

    void Viewer::ensure_text_layout_engine() const {
        if (fonts_tried_)
            return;
        fonts_tried_ = true;
        try {
            text_layout_.reset(new graphics::TextLayoutEngine);
            text_layout_->add_font(resource::directory() + "/fonts/en_Earth-Normal.ttf");
            text_layout_->add_font(resource::directory() + "/fonts/en_Roboto-Medium.ttf");
        } catch (const std::exception &e) {
            text_layout_.reset();
            LOG(WARNING) << "Viewer HUD font unavailable: " << e.what();
        }
    }

    bool Viewer::update_hud_text(std::vector<graphics::DrawCommand> &commands) const {
        const float dpi = dpi_scaling();
        const bool viewport_changed = hint_dpi_ != dpi || framebuffer_width() != hint_width_ ||
                                      framebuffer_height() != hint_height_ || rendered_show_logo_ != show_easy3d_logo_;
        const bool hint_changed =
            viewport_changed || rendered_hint_ != hint_ || rendered_show_frame_rate_ != show_frame_rate_;
        const std::string frame_rate_text = show_frame_rate_ ? framerate_ : std::string();
        const bool frame_rate_changed = viewport_changed || rendered_frame_rate_ != frame_rate_text;
        const bool changed = hint_changed || frame_rate_changed;

        if (changed) {
            if (show_easy3d_logo_ || !hint_.empty() || !frame_rate_text.empty())
                ensure_text_layout_engine();
            if (viewport_changed) {
                logo_commands_.clear();
                if (text_layout_ && show_easy3d_logo_) {
                    graphics::TextOptions options;
                    options.size = 15 * dpi;
                    options.x = 20 * dpi;
                    options.y = 22.5f * dpi;
                    logo_commands_ =
                        text_layout_->layout("Easy3D", framebuffer_width(), framebuffer_height(), options).commands;
                }
                rendered_show_logo_ = show_easy3d_logo_;
            }
            if (hint_changed) {
                hint_commands_.clear();
                if (text_layout_ && !hint_.empty()) {
                    graphics::TextOptions options;
                    options.font = 1;
                    options.size = 15 * dpi;
                    options.x = 35 * dpi;
                    options.y = 22.5f * (1 + int(show_frame_rate_) + int(show_easy3d_logo_)) * dpi;
                    options.color = vec4(0, 0, 1, 1);
                    hint_commands_ =
                        text_layout_->layout(hint_, framebuffer_width(), framebuffer_height(), options).commands;
                }
                rendered_hint_ = hint_;
                rendered_show_frame_rate_ = show_frame_rate_;
            }
            if (frame_rate_changed) {
                frame_rate_commands_.clear();
                if (text_layout_ && !frame_rate_text.empty()) {
                    graphics::TextOptions options;
                    options.font = 1;
                    options.size = 15 * dpi;
                    options.x = 20 * dpi;
                    options.y = 22.5f * (1 + int(show_easy3d_logo_)) * dpi;
                    frame_rate_commands_ =
                        text_layout_->layout(frame_rate_text, framebuffer_width(), framebuffer_height(), options).commands;
                }
                rendered_frame_rate_ = frame_rate_text;
            }
            hint_width_ = framebuffer_width();
            hint_height_ = framebuffer_height();
            hint_dpi_ = dpi;
        }

        commands = logo_commands_;
        commands.insert(commands.end(), frame_rate_commands_.begin(), frame_rate_commands_.end());
        commands.insert(commands.end(), hint_commands_.begin(), hint_commands_.end());
        return changed;
    }
} // namespace easy3d

#ifdef EASY3D_HAS_VULKAN
namespace easy3d {
    struct Viewer::Impl {
        std::unique_ptr<graphics::RenderDevice> device;
        Viewer &owner;
        explicit Impl(Viewer &viewer) : owner(viewer) {}
        std::vector<graphics::DrawCommand> overlays, viewer_overlays, viewer_3d_overlays, viewer_annotations;
        bool overlays_foreground = false;
        RenderSettings render_settings;
        std::vector<vec4> clipping_planes;
        mutable std::vector<vec4> shared_clipping_planes;
        mutable vulkan::ClippingPlaneRenderer clipping_plane_renderer;
        graphics::DrawableRenderer drawable_renderer;

        // While active, draw() renders the file-snapshot frame into an offscreen
        // image instead of presenting it; snapshot_frame() owns the state.
        struct SnapshotFrame {
            bool active = false;
            int width = 0, height = 0;
            unsigned samples = 1;
            vec4 background{};
            graphics::FrameImage image;
        } snapshot;

        void frame_size(int &width, int &height) const {
            if (snapshot.active) {
                width = snapshot.width;
                height = snapshot.height;
            } else
                owner.framebuffer_size(width, height);
        }

        void report_dropped_fragments(graphics::TransparencyMethod method) const {
            if (method == graphics::TransparencyMethod::linked_lists && device && device->oit_dropped_fragments() > 0)
                LOG_N_TIMES(1, WARNING) << "per-pixel linked lists dropped "
                                        << device->oit_dropped_fragments()
                                        << " transparent fragments; increase RenderSettings::transparency.layers";
        }

        bool submit(std::vector<graphics::DrawCommand> commands,
                    graphics::TransparencyMethod method = graphics::TransparencyMethod::sorted,
                    unsigned peel_layers = 0) {
            int width = 0, height = 0;
            frame_size(width, height);
            if (width <= 0 || height <= 0)
                return false;
            if (!snapshot.active) {
                const bool presented = device->present(unsigned(width), unsigned(height), commands, owner.background_color(),
                                                       unsigned(owner.samples()), method, peel_layers);
                report_dropped_fragments(method);
                return presented;
            }
            // File snapshots keep only the color image, so skip the depth/ID readback.
            graphics::RenderOptions options;
            options.background = snapshot.background;
            options.samples = snapshot.samples;
            options.transparency = method;
            options.peel_layers = peel_layers;
            options.readback = graphics::FrameReadback::color;
            snapshot.image = device->render(unsigned(width), unsigned(height), commands, options);
            report_dropped_fragments(method);
            return true;
        }

        const std::vector<vec4> &active_clipping_planes() const {
            shared_clipping_planes = graphics::effective_clipping_planes(clipping_planes,
                                                                        *ClippingPlane::instance());
            return shared_clipping_planes;
        }

        // Inject the effect overlays declared in RenderSettings. Legacy facades keep
        // their own path in Viewer::draw(); capture() stays a raw scene-command
        // readback and does not receive these presentation overlays.
        void apply_settings_effects(std::vector<graphics::DrawCommand> &commands, const Camera &cam,
                                    int width, int height) const {
            if (commands.empty())
                return;
            build_settings_effects(commands, cam, width, height, render_settings);
        }

        graphics::FrameImage capture(const Camera &cam,
                                     const vec4 &bg,
                                     unsigned samples = 1,
                                     int width = 0,
                                     int height = 0,
                                     bool include_hud = true,
                                     graphics::FrameReadback readback = graphics::FrameReadback::color_depth_ids) const {
            if (!width)
                width = cam.screen_width();
            if (!height)
                height = cam.screen_height();
            graphics::RenderOptions options;
            options.background = bg;
            options.samples = samples;
            options.readback = readback;
            auto commands = owner.frame_commands(cam, width, height, nullptr, include_hud);
            // MultiViewer::draw_commands() remaps IDs when sources are requested;
            // capture() must keep the model/standalone IDs used by pick(). Recover
            // drawable colors from a parallel source-mapped command list without
            // changing the IDs in the captured command list.
            std::vector<DrawSource> sources;
            std::vector<graphics::DrawCommand> source_commands;
            if (owner.render_settings().transparency.enabled) {
                if (const auto *multi = dynamic_cast<const MultiViewer *>(&owner))
                    source_commands = multi->draw_commands(cam, width, height, &sources);
                else
                    source_commands = owner.draw_commands(cam, width, height, &sources);
            }
            owner.strip_settings_transparency(commands, sources, source_commands);
            return device->render(uint32_t(width), uint32_t(height), std::move(commands), options);
        }
    };

    void Viewer::append_settings_effects(std::vector<graphics::DrawCommand> &commands,
                                         const Camera &camera, int width, int height) const {
        if (commands.empty())
            return;
        validate_render_settings(impl_->render_settings);
        build_settings_effects(commands, camera, width, height, impl_->render_settings);
    }

    namespace {
        const char *const default_manual =
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
            "  'd':                     Print model information                            \n"
            "  Space:                   Align camera with world axes                       \n"
            "  Escape / Alt + F4:       Close viewer                                       \n"
            "-------------------------------------------------------------------";

    }
    struct Viewer::Window {
        std::shared_ptr<detail::GlfwRuntime> runtime;
        GLFWwindow *handle = nullptr;
        std::string title;
        int width, height;
        unsigned samples;
        // Vulkan expresses "no MSAA" as VK_SAMPLE_COUNT_1_BIT, which is what this field
        // holds: the constructor's 0 (the OpenGL spelling of no multisampling) and a
        // request of 1 both normalize to the single sample count. The OpenGL backend
        // needs the opposite translation, done in Viewer::create_window().
        Window(const std::string &text, int w, int h, int sample_count, bool full_screen, bool visible, bool resizable)
            : title(text), width(w), height(h), samples(sample_count > 1 ? unsigned(sample_count) : 1u) {
            if (w <= 0 || h <= 0)
                throw std::invalid_argument("window dimensions must be positive");
            if (sample_count < 0 || sample_count > 64 || (sample_count && (sample_count & (sample_count - 1))))
                throw std::invalid_argument("invalid viewer sample count");
            runtime = detail::GlfwRuntime::acquire(true);
            if (!glfwVulkanSupported())
                throw std::runtime_error("GLFW could not find Vulkan loader/driver support");
            glfwDefaultWindowHints();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);
            glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
            GLFWmonitor *monitor = nullptr;
            if (full_screen) {
                monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode *mode = monitor ? glfwGetVideoMode(monitor) : nullptr;
                if (!mode)
                    throw std::runtime_error("no monitor available for a full-screen window");
                width = mode->width;
                height = mode->height;
            }
#ifndef NDEBUG
            const std::string title_str = title + " - Debug Version";
#else
            const std::string title_str = title;
#endif
            handle = glfwCreateWindow(width, height, title_str.c_str(), monitor, nullptr);
            if (!handle)
                throw std::runtime_error("could not create a GLFW Vulkan window");
        }
        ~Window() {
            if (handle)
                glfwDestroyWindow(handle);
        }
        std::unique_ptr<graphics::RenderDevice> device(const DeviceOptions &options) {
            uint32_t count = 0;
            const char **required = glfwGetRequiredInstanceExtensions(&count);
            if (!required || !count)
                throw std::runtime_error("GLFW has no Vulkan surface extensions for this platform");
            graphics::backends::vulkan::VulkanSurface surface;
            surface.loader = runtime->vulkan_loader();
            for (uint32_t i = 0; i < count; ++i)
                surface.instance_extensions.emplace_back(required[i]);
            surface.create = [this](VkInstance instance, VkSurfaceKHR *result) {
                return glfwCreateWindowSurface(instance, handle, nullptr, result);
            };
            graphics::DeviceOptions device_options;
            device_options.validation = options.validation;
            device_options.sync_validation = options.sync_validation;
            device_options.profiling = options.profiling;
            device_options.use_native_large_points = options.use_native_large_points;
            device_options.device_name = options.device_name;
            return graphics::backends::vulkan::create_vulkan_window_device(surface, device_options);
        }
        int modifiers() const {
            return detail::GlfwRuntime::modifiers(handle);
        }
        template <class Function> static void dispatch(GLFWwindow *handle, const Function &call) noexcept {
            auto *viewer = static_cast<Viewer *>(glfwGetWindowUserPointer(handle));
            if (!viewer)
                return;
            try {
                call(*viewer);
            } catch (...) {
                viewer->callback_error_ = std::current_exception();
                glfwSetWindowShouldClose(handle, GLFW_TRUE);
            }
        }
        void callbacks(Viewer *viewer) {
            glfwSetWindowUserPointer(handle, viewer);
            glfwSetWindowRefreshCallback(handle, [](GLFWwindow *w) { dispatch(w, [](Viewer &v) { v.update(); }); });
            // Native events use logical window coordinates, identical to OpenGL; the
            // shared ViewerBase routers own all input state and event ordering.
            glfwSetCursorPosCallback(handle, [](GLFWwindow *w, double px, double py) {
                dispatch(w, [&](Viewer &v) {
                    v.handle_cursor_pos(int(px), int(py), detail::GlfwRuntime::modifiers(w));
                });
            });
            glfwSetMouseButtonCallback(handle, [](GLFWwindow *w, int button, int action, int mods) {
                dispatch(w, [&](Viewer &v) {
                    double px, py;
                    glfwGetCursorPos(w, &px, &py);
                    v.handle_mouse_button(int(px), int(py), button, action, mods);
                });
            });
            glfwSetScrollCallback(handle, [](GLFWwindow *w, double dx, double dy) {
                dispatch(w, [&](Viewer &v) {
                    double px, py;
                    glfwGetCursorPos(w, &px, &py);
                    v.handle_scroll(int(px), int(py), dx, dy);
                });
            });
            glfwSetKeyCallback(handle, [](GLFWwindow *w, int key, int, int action, int mods) {
                dispatch(w, [&](Viewer &v) { v.handle_key(key, action, mods); });
            });
            glfwSetCharCallback(handle, [](GLFWwindow *w, unsigned int code) {
                dispatch(w, [&](Viewer &v) { v.handle_character(code); });
            });
            glfwSetDropCallback(handle, [](GLFWwindow *w, int count, const char **files) {
                dispatch(w, [&](Viewer &v) {
                    std::vector<std::string> names;
                    for (int i = 0; i < count; ++i)
                        names.emplace_back(files[i]);
                    v.handle_drop(names);
                });
            });
            glfwSetWindowFocusCallback(handle, [](GLFWwindow *w, int focused) {
                dispatch(w, [&](Viewer &v) { v.handle_focus(focused != 0); });
            });
            glfwSetFramebufferSizeCallback(handle, [](GLFWwindow *w, int width, int height) {
                dispatch(w, [&](Viewer &v) {
                    glfwGetWindowSize(w, &v.width_, &v.height_);
                    if (width > 0 && height > 0) {
                        v.resize_render_target(width, height);
                        v.cancel_interaction();
                    }
                    v.notify_resize();
                });
            });
        }
    };
} // namespace easy3d
#endif


//============================================================================
// Backend-independent implementation.
//
// The frame loop, window/event plumbing, HUD bookkeeping, model/drawable
// management and file IO behave identically on OpenGL and Vulkan. Only the
// few genuinely backend-specific steps (context/device binding, presentation,
// overlay encoding and the pivot mark) are guarded inline below. Rendering,
// picking, capture and window creation live in the backend sections that follow.
//============================================================================
namespace easy3d {
    unsigned Viewer::validation_errors() const {
#ifdef EASY3D_HAS_VULKAN
        return device().validation_errors();
#else
        return 0;
#endif
    }

    void Viewer::poll_events() {
        ViewerBase::poll_events();
    }

    void Viewer::request_close() {
        if (window_)
            glfwSetWindowShouldClose(window_, GLFW_TRUE);
        update();
    }

    bool Viewer::should_close() const {
        return window_ && glfwWindowShouldClose(window_) != 0;
    }

    void Viewer::show() {
        if (window_)
            glfwShowWindow(window_);
    }

    void Viewer::hide() {
        if (window_)
            glfwHideWindow(window_);
    }

    void Viewer::exit() {
        request_close();
    }

    int Viewer::supported_sample_count(int requested) const {
        // One rule for every sample-count entry point: a request of 1 - and the
        // constructors' 0, which means the same - needs no device to answer for, and
        // anything above it becomes the largest count this backend can render with.
        // The ladder is walked instead of testing the request against the mask, whose
        // flags hold numeric counts: a request of 3 would "match" the flags of 1 and 2.
        if (requested <= 1)
            return 1;
        const unsigned available = supported_sample_mask();
        for (unsigned candidate : {64u, 32u, 16u, 8u, 4u, 2u})
            if (unsigned(requested) >= candidate && (available & candidate)) {
                if (int(candidate) != requested)
                    LOG(WARNING) << "sample count " << requested << " is not supported, using " << candidate;
                return int(candidate);
            }
        LOG(WARNING) << "sample count " << requested << " is not supported, using 1 (no multisampling)";
        return 1;
    }

    void Viewer::update() const {
        glfwPostEmptyEvent();
    }

    void Viewer::resize(int w, int h) {
        if (w <= 0 || h <= 0)
            throw std::invalid_argument("window dimensions must be positive");
        if (!window_)
            return;
        glfwSetWindowSize(window_, w, h);
        // Window systems may apply the request asynchronously (X11 with a window
        // manager), so a frame rendered immediately after this call could use the
        // old size. Wait a bounded time for the platform to report the request.
        for (int i = 0; i < 20; ++i) {
            int ww, wh;
            glfwGetWindowSize(window_, &ww, &wh);
            if (ww == w && wh == h)
                break;
            glfwWaitEventsTimeout(0.01);
        }
    }

    void Viewer::framebuffer_size(int &w, int &h) const {
        if (window_)
            glfwGetFramebufferSize(window_, &w, &h);
        else {
            w = camera()->screen_width();
            h = camera()->screen_height();
        }
    }

    void Viewer::viewer_size(int &w, int &h) const {
        if (window_)
            glfwGetWindowSize(window_, &w, &h);
        else {
            w = width_;
            h = height_;
        }
    }

    void Viewer::position(int &pos_x, int &pos_y) const {
        if (window_)
            glfwGetWindowPos(window_, &pos_x, &pos_y);
        else
            throw std::runtime_error("viewer window doesn't exist");
    }

    void Viewer::notify_resize() {
        int w, h, fw, fh;
        viewer_size(w, h);
        framebuffer_size(fw, fh);
        if (w == resize_width_ && h == resize_height_ && fw == resize_fb_width_ && fh == resize_fb_height_)
            return;
        resize_width_ = w;
        resize_height_ = h;
        resize_fb_width_ = fw;
        resize_fb_height_ = fh;
        post_resize(w, h);
    }

    void Viewer::set_position(int x, int y) {
        if (window_)
            glfwSetWindowPos(window_, x, y);
    }

    void Viewer::set_title(const std::string &text) {
        if (text == title_)
            return;
        title_ = text;
        if (window_)
            glfwSetWindowTitle(window_, text.c_str());
#ifdef EASY3D_HAS_VULKAN
        if (window_host_)
            window_host_->title = text;
#endif
    }

    void Viewer::set_animation(bool enabled) {
        animation_enabled_ = enabled;
        update();
    }

    bool Viewer::is_animating() const {
        return animation_enabled_ && bool(animation_func_);
    }

    void Viewer::set_usage(const std::string &manual, const std::string &screen_hint) {
        if (manual != "default") {
            manual_ = manual;
            usage_printed_ = false;
        }
        hint_ = screen_hint;
        update();
    }

    void Viewer::print_usage() {
        if (usage_printed_)
            return;
        if (!manual_.empty()) {
            std::cout << manual_ << std::endl;
            usage_printed_ = true;
        }
    }

    void Viewer::update_frame_rate() {
        if (!show_frame_rate_)
            return;
        ++frame_rate_frames_;
        const auto now = std::chrono::steady_clock::now();
        const double elapsed = std::chrono::duration<double>(now - frame_rate_start_).count();
        if (elapsed < 1.0)
            return;
        std::ostringstream out;
        out << "fps: " << std::fixed << std::setprecision(0) << frame_rate_frames_ / elapsed << " ("
            << std::setprecision(1) << 1000.0 * elapsed / frame_rate_frames_ << " ms/frame)";
        framerate_ = out.str();
        frame_rate_frames_ = 0;
        frame_rate_start_ = now;
    }

    void Viewer::restore_scene_bounds() {
        Box3 box;
        for (const auto &model : models())
            box.grow(model->bounding_box());
        for (const auto &drawable : drawables())
            if (!dynamic_cast<LinesDrawable2D *>(drawable.get()))
                box.grow(drawable->bounding_box());
        if (box.is_valid())
            camera()->set_scene_bounding_box(box.min_point(), box.max_point());
    }

    bool Viewer::draw_frame() {
#ifdef EASY3D_HAS_VULKAN
        if (!window_)
            return false;
#endif
        if (callback_error_)
            std::rethrow_exception(callback_error_);
        if (should_close() || glfwGetWindowAttrib(window_, GLFW_ICONIFIED))
            return false;
        int w, h;
        framebuffer_size(w, h);
        if (w <= 0 || h <= 0)
            return false;
#ifdef EASY3D_HAS_VULKAN
        // Hidden windows may not receive size callbacks (e.g., X11 never reports
        // ConfigureNotify for unmapped windows), so poll both the logical and the
        // framebuffer size. The logical size can also lag the framebuffer by one
        // window-system round trip, so a framebuffer-only change must not cache a
        // stale logical size: the next frame then reports the settled value.
        int lw = 0, lh = 0;
        viewer_size(lw, lh);
        if (lw != width_ || lh != height_ || framebuffer_width_ != w || framebuffer_height_ != h) {
            resize_render_target(w, h);
            cancel_interaction();
            width_ = lw;
            height_ = lh;
            notify_resize();
        }
#else
        glfwMakeContextCurrent(window_);
        // Hidden windows may not receive size callbacks (e.g., X11 never reports
        // ConfigureNotify for unmapped windows), so poll like the Vulkan path.
        int lw = 0, lh = 0;
        viewer_size(lw, lh);
        if (lw != width_ || lh != height_ || w != framebuffer_width_ || h != framebuffer_height_)
            callback_event_resize(lw, lh);
#endif
        if (!initialized_) {
            init();
            initialized_ = true;
        }
        if (is_animating()) {
            animation_func_(this);
            if (callback_error_)
                std::rethrow_exception(callback_error_);
            if (should_close())
                return false;
        }
        print_usage();
        if (callback_error_)
            std::rethrow_exception(callback_error_);
        if (should_close())
            return false;
        pre_draw();
#ifdef EASY3D_HAS_VULKAN
        update_interaction_overlays();
#endif
        draw();
        update_frame_rate();
        post_draw();
#ifndef EASY3D_HAS_VULKAN
        glfwSwapBuffers(window_);
#endif
        if (callback_error_)
            std::rethrow_exception(callback_error_);
        return true;
    }

    int Viewer::run(bool see_all) {
        try {
            if (callback_error_)
                std::rethrow_exception(callback_error_);
            if (!initialized_) {
                init();
                initialized_ = true;
            }
            if (see_all)
                fit_screen();
            if (visible_)
                show();
            return run_frames();
        } catch (const std::exception &e) {
            LOG(ERROR) << "Viewer failed: " << e.what();
            return EXIT_FAILURE;
        }
    }

    int Viewer::run_frames(unsigned max_frames) {
        try {
            if (callback_error_)
                std::rethrow_exception(callback_error_);
            return detail::run_viewer_frames(*this, max_frames, [this] {
                return is_animating() || show_frame_rate_;
            });
        } catch (const std::exception &e) {
            LOG(ERROR) << "Viewer failed: " << e.what();
            return EXIT_FAILURE;
        }
    }

    //------------------------------------------------------------------------
    // Model and drawable management
    //------------------------------------------------------------------------
    Model *Viewer::current_model() const {
        return model_idx_ < 0 || size_t(model_idx_) >= models_.size() ? nullptr : models_[model_idx_].get();
    }

    bool Viewer::set_current_model(Model *model) {
        if (!model)
            return false;
        const auto pos = std::find_if(models_.begin(), models_.end(),
                                      [model](const std::shared_ptr<Model> &m) { return m.get() == model; });
        if (pos == models_.end())
            return false;
        model_idx_ = int(pos - models_.begin());
        return true;
    }

    Model *Viewer::add_model(std::shared_ptr<Model> model, bool create, bool fit_scene) {
#ifndef EASY3D_HAS_VULKAN
        glfwMakeContextCurrent(window_);
#endif
        if (!model || model->empty())
            return nullptr;
        for (const auto &current : models_)
            if (model == current)
                return current.get();
        // Construct first: unsupported default coloring fails without replacing the model's existing renderer.
        auto renderer = std::make_shared<Renderer>(model.get(), create);
        model->set_renderer(renderer);
        models_.push_back(model);
        model_idx_ = int(models_.size()) - 1;
        if (fit_scene)
            fit_screen();
        return model.get();
    }

    Model *Viewer::add_model(Model *model, bool create, bool fit_scene) {
        if (!model)
            return nullptr;
        // Do not manufacture a second shared_ptr control block for an owned model.
        for (const auto &current : models_)
            if (current.get() == model)
                return model;
        return add_model(std::shared_ptr<Model>(model), create, fit_scene);
    }

    Model *Viewer::add_model(const std::string &file_path, bool create_drawables, bool fit_scene) {
#ifndef EASY3D_HAS_VULKAN
        glfwMakeContextCurrent(window_);
#endif
        const std::string file_name = file_system::convert_to_native_style(file_path);
        for (const auto &m : models_) {
            if (m->name() == file_name) {
                LOG(WARNING) << "model has already been added to the viewer: " << file_name;
                return m.get();
            }
        }
        const auto ext = file_system::extension(file_name, true);
        std::shared_ptr<Model> model;
        if (ext == "ply" && io::PlyReader::num_instances(file_name, "face") > 0)
            model.reset(SurfaceMeshIO::load(file_name));
        else if (ext == "ply" && io::PlyReader::num_instances(file_name, "edge") > 0)
            model.reset(GraphIO::load(file_name));
        else if (ext == "obj" || ext == "off" || ext == "stl" || ext == "sm" || ext == "geojson" || ext == "trilist")
            model.reset(SurfaceMeshIO::load(file_name));
        else if (ext == "plm" || ext == "pm" || ext == "mesh")
            model.reset(PolyMeshIO::load(file_name));
        else if (ext == "ptx") {
            // A PTX file may store several scans; each becomes its own model.
            io::PointCloudIO_ptx serializer(file_name);
            PointCloud *cloud = nullptr;
            Model *last = nullptr;
            while ((cloud = serializer.load_next())) {
                last = add_model(std::shared_ptr<PointCloud>(cloud), create_drawables, fit_scene);
                update();
            }
            return last; // the last cloud in the file
        } else
            model.reset(PointCloudIO::load(file_name));
        if (model) {
            model->set_name(file_name);
            return add_model(model, create_drawables, fit_scene);
        }
        return nullptr;
    }

    bool Viewer::delete_model(Model *model) {
#ifndef EASY3D_HAS_VULKAN
        glfwMakeContextCurrent(window_);
#endif
        auto &values = models_;
        auto found = std::find_if(values.begin(), values.end(),
                                  [model](const std::shared_ptr<Model> &p) { return p.get() == model; });
        if (found == values.end())
            return false;
        // Shortcuts carry a borrowed model pointer: drop them before the model can be destroyed.
        unbind_shortcut(model);
        values.erase(found);
        model_idx_ = int(models_.size()) - 1;
#ifdef EASY3D_HAS_VULKAN
        impl_->drawable_renderer.clear();
#endif
        return true;
    }

    Drawable *Viewer::add_drawable(std::shared_ptr<Drawable> drawable) {
        if (!drawable)
            return nullptr;
        for (const auto &d : drawables_)
            if (d == drawable)
                return d.get();
        drawables_.push_back(drawable);
        return drawable.get();
    }

    Drawable *Viewer::add_drawable(Drawable *drawable) {
        return add_drawable(std::shared_ptr<Drawable>(drawable));
    }

    bool Viewer::delete_drawable(Drawable *drawable) {
#ifndef EASY3D_HAS_VULKAN
        glfwMakeContextCurrent(window_);
#endif
        auto &values = drawables_;
        auto found = std::find_if(values.begin(), values.end(),
                                  [drawable](const std::shared_ptr<Drawable> &p) { return p.get() == drawable; });
        if (found == values.end())
            return false;
        values.erase(found);
#ifdef EASY3D_HAS_VULKAN
        impl_->drawable_renderer.clear();
#endif
        return true;
    }

    void Viewer::clear_scene() {
#ifndef EASY3D_HAS_VULKAN
        glfwMakeContextCurrent(window_);
#endif
        // Model-bound shortcuts would dangle once the models are gone; model-free actions survive.
        for (const auto& model : models_)
            unbind_shortcut(model.get());
        models_.clear();
        model_idx_ = -1;
        drawables_.clear();
#ifdef EASY3D_HAS_VULKAN
        impl_->drawable_renderer.clear();
        impl_->overlays.clear();
        impl_->viewer_overlays.clear();
        impl_->viewer_3d_overlays.clear();
        impl_->viewer_annotations.clear();
        if (window_) {
            cancel_interaction();
            update_interaction_overlays();
        }
#else
        if (overlay_renderer_)
            overlay_renderer_->set({}, false);
#endif
    }

    void Viewer::fit_screen(const Model *selected) {
        Box3 box;
        if (selected)
            box.grow(selected->bounding_box());
        else {
            for (const auto &model : models_)
                box.grow(model->bounding_box());
            for (const auto &drawable : drawables_)
                if (!dynamic_cast<LinesDrawable2D *>(drawable.get()))
                    box.grow(drawable->bounding_box());
        }
        if (box.is_valid()) {
            camera()->set_scene_bounding_box(box.min_point(), box.max_point());
            camera()->show_entire_scene();
            ClippingPlane::instance()->fit_scene(camera()->scene_center(), camera()->scene_radius());
        }
    }

    //------------------------------------------------------------------------
    // File IO
    //------------------------------------------------------------------------
    bool Viewer::open() {
        const std::string title("Please choose a file");
        const std::string &default_path = resource::directory() + "/data/";
        const std::vector<std::string> &filters = {
            "Surface Mesh (*.obj *.ply *.off *.stl *.sm *.geojson *.trilist)",
            "*.obj *.ply *.off *.stl *.sm *.geojson *.trilist",
            "Point Cloud (*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg *.ptx)",
            "*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg *.ptx",
            "Polyhedral Mesh (*.plm *.pm *.mesh)",
            "*.plm *.pm *.mesh",
            "Graph (*.ply)",
            "*.ply",
            "All Files (*.*)",
            "*"};
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
        Model *m = current_model();
        if (!m) {
            LOG(ERROR) << "no model exists";
            return false;
        }

        const std::string &title = "Please choose a file name";
        const std::vector<std::string> &filters = {"Surface Mesh (*.obj *.ply *.off *.stl *.sm)",
                                                   "*.obj *.ply *.off *.stl *.sm",
                                                   "Point Cloud (*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg)",
                                                   "*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg",
                                                   "Polyhedral Mesh (*.plm *.pm *.mesh)",
                                                   "*.plm *.pm *.mesh",
                                                   "Graph (*.ply)",
                                                   "*.ply",
                                                   "All Files (*.*)",
                                                   "*"};

        std::string default_file_name = m->name();
        if (file_system::extension(default_file_name).empty()) // no extension?
            default_file_name += ".ply";                       // default to ply

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
        const std::vector<std::string> &filters = {"Image Files (*.png *.jpg *.bmp *.tga)", "*.png *.jpg *.bmp *.tga",
                                                   "All Files (*.*)", "*"};

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

    void Viewer::copy_view() {
        const auto p = camera()->position();
        const auto q = camera()->orientation();
        std::ostringstream out;
        out << std::setprecision(9) << p.x << ' ' << p.y << ' ' << p.z;
        for (unsigned i = 0; i < 4; ++i)
            out << ' ' << q[i];
        glfwSetClipboardString(window(), out.str().c_str());
    }

    void Viewer::paste_view() {
        const char *text = glfwGetClipboardString(window());
        if (!text)
            return;
        std::istringstream in(text);
        vec3 p;
        quat q;
        for (unsigned i = 0; i < 3; ++i)
            if (!(in >> p[i]) || !std::isfinite(p[i]))
                return;
        float norm = 0;
        for (unsigned i = 0; i < 4; ++i) {
            if (!(in >> q[i]) || !std::isfinite(q[i]))
                return;
            norm += q[i] * q[i];
        }
        std::string extra;
        if ((in >> extra) || !std::isfinite(norm) || norm < 1e-12f)
            return;
        q.normalize();
        // I prefer to animate the view change rather than snap to it.
        Frame new_frame;
        new_frame.set_position(p);
        new_frame.set_orientation(q);
        const float duration = 0.5f;
        camera()->interpolate_to(&new_frame, duration);
        update();
    }

    //------------------------------------------------------------------------
    // Event handlers (logical window coordinates on both backends)
    //------------------------------------------------------------------------
    bool Viewer::mouse_free_move_event(int, int, int, int, int) {
        // highlight geometry primitives here
        return false;
    }

    bool Viewer::char_input_event(unsigned int) {
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

    bool Viewer::focus_event(bool) {
        return false;
    }

    void Viewer::end_gesture() {
        zoom_region_ = false;
        camera()->frame()->action_end();
        ClippingPlane::instance()->manipulator()->frame()->action_end();
#ifdef EASY3D_HAS_VULKAN
        update_interaction_overlays(true);
#endif
    }

    void Viewer::cancel_interaction() {
        input_.reset();
        show_pivot_point_ = false;
        end_gesture();
    }

    bool Viewer::mouse_scroll_event(int, int, int, int dy) {
        camera()->frame()->action_zoom(dy, camera());
        return false;
    }

    bool Viewer::mouse_press_event(int x, int y, int button, int modifiers) {
        // The router already recorded these; assigning again keeps direct calls (for
        // example from tests or subclasses) consistent.
        input_.press_x = input_.x = x;
        input_.press_y = input_.y = y;
        input_.button = button;
        input_.press_modifiers = modifiers;
        zoom_region_ = button == BUTTON_LEFT && modifiers == MODIF_CTRL;
        camera()->frame()->action_start();
        if (modifiers == MODIF_ALT && ClippingPlane::instance()->is_enabled())
            ClippingPlane::instance()->manipulator()->frame()->action_start();
        if (input_.key == KEY_Z || modifiers == MODIF_SHIFT) {
            if (button == BUTTON_LEFT) {
                bool found = false;
                const vec3 point = point_under_pixel(x, y, found);
                camera()->set_pivot_point(found ? point : camera()->scene_center());
                show_pivot_point_ = found;
                if (found) {
#ifdef EASY3D_HAS_VULKAN
                    pivot_pixel_ = vec2(float(x), float(y));
                    pivot_expires_ = std::chrono::steady_clock::now() + std::chrono::seconds(10);
#else
                    auto &pivot = opengl_viewer_state(this);
                    pivot.pivot_pixel = vec2(float(x), float(y));
                    pivot.pivot_expires = std::chrono::steady_clock::now() + std::chrono::seconds(10);
#endif
                    Timer<>::single_shot(10000, [&]() {
                        show_pivot_point_ = false;
                        update();
                    });
                    if (input_.key == KEY_Z && modifiers == 0) {
#if 1 // with animation  Do NOT delete!!!
                        camera()->interpolate_to_look_at(point);
#else // without animation  Do NOT delete!!!
                        // Apply the legacy zoom target directly on the UI thread.
                        camera()->set_position(0.1f * camera()->position() + 0.9f * point);
                        camera()->look_at(point);
#endif
                    }
                }
            } else if (button == BUTTON_RIGHT) {
                camera()->set_pivot_point(camera()->scene_center());
                show_pivot_point_ = false;
                if (input_.key == KEY_Z && modifiers == 0) {
#if 1 // with animation  Do NOT delete!!!
                    camera()->interpolate_to_fit_scene();
#else // without animation  Do NOT delete!!!
                    camera()->show_entire_scene();
#endif
                }
            }
        }
#ifdef EASY3D_HAS_VULKAN
        update_interaction_overlays(true);
#endif
        return false;
    }

    bool Viewer::mouse_release_event(int x, int y, int button, int modifiers) {
        if (input_.button == button && button == BUTTON_LEFT && modifiers == MODIF_CTRL && x != input_.press_x &&
            y != input_.press_y) {
            const ivec2 origin = interaction_origin();
            camera()->fit_screen_region(std::min(x, input_.press_x) - origin.x,
                                      std::min(y, input_.press_y) - origin.y,
                                      std::max(x, input_.press_x) - origin.x,
                                      std::max(y, input_.press_y) - origin.y);
        }
        input_.button = -1;
        input_.press_modifiers = 0;
        end_gesture();
        return false;
    }

    bool Viewer::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
        input_.x = x;
        input_.y = y;
        zoom_region_ = input_.button == BUTTON_LEFT && modifiers == MODIF_CTRL;
#ifdef EASY3D_HAS_VULKAN
        update_interaction_overlays(true);
#endif
        // Region zoom and clipping/tool manipulation must not move the camera.
        if (modifiers == MODIF_CTRL)
            return false;
        // Camera actions are cell-local in multi-view layouts; overlays stay absolute.
        const ivec2 origin = interaction_origin();
        const int cx = x - origin.x, cy = y - origin.y;
        if (modifiers == MODIF_ALT) {
            // the clipping plane can be manipulated only when it is enabled
            if (ClippingPlane::instance()->is_enabled()) {
                auto *frame = ClippingPlane::instance()->manipulator()->frame();
                if (button == BUTTON_LEFT)
                    frame->action_rotate(cx, cy, dx, dy, camera(), ManipulatedFrame::NONE);
                else if (button == BUTTON_RIGHT)
                    frame->action_translate(cx, cy, dx, dy, camera(), ManipulatedFrame::NONE);
            }
            return false;
        }
        // axis-constrained rotation/translation of the camera
        auto axis = ManipulatedFrame::NONE;
        if (input_.key == KEY_X)
            axis = ManipulatedFrame::HORIZONTAL;
        else if (input_.key == KEY_Y)
            axis = ManipulatedFrame::VERTICAL;
        else if (input_.key == KEY_O)
            axis = ManipulatedFrame::ORTHOGONAL;
        if (button == BUTTON_LEFT)
            camera()->frame()->action_rotate(cx, cy, dx, dy, camera(), axis);
        else if (button == BUTTON_RIGHT)
            camera()->frame()->action_translate(cx, cy, dx, dy, camera(), axis);
        else if (button == BUTTON_MIDDLE && std::abs(dy) >= 1)
            camera()->frame()->action_zoom(dy > 0 ? 1 : -1, camera());
        return false;
    }

    bool Viewer::key_release_event(int key, int) {
        input_.key = -1;
        if (key == KEY_LEFT_CONTROL || key == KEY_RIGHT_CONTROL || key == KEY_LEFT_SUPER ||
            key == KEY_RIGHT_SUPER) {
            zoom_region_ = false;
#ifdef EASY3D_HAS_VULKAN
            update_interaction_overlays(true);
#endif
        }
        return false;
    }

    bool Viewer::drop_event(const std::vector<std::string> &names) {
        unsigned count = 0;
        for (const auto &file : names)
            if (add_model(file))
                ++count;
        if (count)
            fit_screen();
        return count != 0;
    }

    bool Viewer::key_press_event(int key, int modifiers) {
        const auto command = commands_.find(Key(key));
        if (command != commands_.end()) {
            const auto binding = command->second.find(Modifier(modifiers));
            if (binding != command->second.end()) {
                // The callable may unbind itself or delete its model (which prunes the binding),
                // so keep a copy of the bound callable alive for the duration of the call.
                const Function func = binding->second.first;
                return func(this, binding->second.second);
            }
        }
        if ((key == KEY_ESCAPE && modifiers == 0) || (key == KEY_F4 && modifiers == MODIF_ALT))
            exit();
        else if (key == KEY_F1 && modifiers == 0) {
            if (!manual_.empty())
                std::cout << manual_ << '\n';
        } else if (key == KEY_F2 && modifiers == 0)
            set_animation(!animation_enabled_);
        else if ((key == KEY_LEFT || key == KEY_RIGHT) && modifiers == 0)
            // turn left/right, 1 degree each step
            camera()->frame()->action_turn((key == KEY_LEFT ? 1.0f : -1.0f) * float(M_PI / 180.0), camera());
        else if ((key == KEY_UP || key == KEY_DOWN) && modifiers == 0) {
            // move camera forward/backward
            const float step = .05f * camera()->scene_radius();
            camera()->frame()->translate(
                camera()->frame()->inverse_transform_of(vec3(0, 0, key == KEY_UP ? -step : step)));
        } else if (modifiers == MODIF_CTRL && key >= KEY_RIGHT && key <= KEY_UP) {
            // move camera left/right/up/down
            const float step = .05f * camera()->scene_radius();
            const vec3 direction = key == KEY_LEFT    ? vec3(-step, 0, 0)
                                   : key == KEY_RIGHT ? vec3(step, 0, 0)
                                   : key == KEY_UP    ? vec3(0, step, 0)
                                                           : vec3(0, -step, 0);
            camera()->frame()->translate(camera()->frame()->inverse_transform_of(direction));
        } else if (modifiers == MODIF_CTRL && (key == KEY_MINUS || key == KEY_EQUAL))
            camera()->frame()->action_zoom(key == KEY_EQUAL ? 1 : -1, camera());
        else if (key == KEY_C && modifiers == MODIF_CTRL)
            copy_view(); // copy camera
        else if (key == KEY_V && modifiers == MODIF_CTRL)
            paste_view(); // paste camera
        else if (key == KEY_O && modifiers == MODIF_CTRL)
            open();
        else if (key == KEY_S && modifiers == MODIF_CTRL)
            save();
        else if (key == KEY_S && modifiers == 0)
            snapshot();
        else if (key == KEY_F && modifiers == MODIF_CTRL) {
            show_frame_rate_ = !show_frame_rate_;
            if (show_frame_rate_) {
                frame_rate_start_ = std::chrono::steady_clock::now();
                frame_rate_frames_ = 0;
                framerate_ = "fps: -- (-- ms/frame)";
            }
            update();
        } else if (key == KEY_F && modifiers == 0)
            fit_screen();
        else if (key == KEY_C && modifiers == 0) {
            if (current_model())
                fit_screen(current_model());
        } else if (key == KEY_P && modifiers == 0)
            camera()->set_type(camera()->type() == Camera::PERSPECTIVE ? Camera::ORTHOGRAPHIC : Camera::PERSPECTIVE);
        else if (key == KEY_SPACE && modifiers == 0) {
            // Aligns camera
            Frame frame;
            frame.set_translation(camera()->pivot_point());
            camera()->frame()->align_with_frame(&frame, true);
        } else if (key == KEY_M && modifiers == 0) {
            if (auto *mesh = dynamic_cast<SurfaceMesh *>(current_model()))
                for (const auto &d : mesh->renderer()->triangles_drawables())
                    d->set_smooth_shading(!d->smooth_shading());
        } else if (modifiers == 0 && (key == KEY_LEFT_BRACKET || key == KEY_RIGHT_BRACKET)) {
            for (const auto &m : models())
                if (m->renderer())
                    for (const auto &d : m->renderer()->lines_drawables())
                        d->set_line_width(std::max(1.0f, d->line_width() + (key == KEY_RIGHT_BRACKET ? 1 : -1)));
        } else if (modifiers == 0 && (key == KEY_MINUS || key == KEY_EQUAL)) {
            for (const auto &m : models())
                if (m->renderer())
                    for (const auto &d : m->renderer()->points_drawables())
                        d->set_point_size(std::max(1.0f, d->point_size() + (key == KEY_EQUAL ? 1 : -1)));
        } else if (modifiers == 0 && (key == KEY_COMMA || key == KEY_PERIOD)) {
            if (!models().empty()) {
                auto i = std::find_if(models().begin(), models().end(),
                                      [&](const std::shared_ptr<Model> &m) { return m.get() == current_model(); });
                const int n = int(models().size()), index = i == models().end() ? n - 1 : int(i - models().begin());
                model_idx_ = (index + n + (key == KEY_PERIOD ? 1 : -1)) % n;
                fit_screen(current_model());
            }
        } else if (key == KEY_DELETE && modifiers == 0) {
            if (current_model())
                delete_model(current_model());
        } else if (modifiers == 0 && current_model() && current_model()->renderer() &&
                   (key == KEY_E || key == KEY_V || key == KEY_B || key == KEY_L)) {
            auto *renderer = current_model()->renderer();
            Drawable *d = nullptr;
            if (key == KEY_E)
                d = renderer->get_lines_drawable("edges");
            else if (key == KEY_V)
                d = renderer->get_points_drawable("vertices");
            else if (dynamic_cast<SurfaceMesh *>(current_model())) {
                if (key == KEY_B)
                    d = renderer->get_lines_drawable("borders");
                else
                    d = renderer->get_points_drawable("locks"); // locked vertices
            }
            if (d)
                d->set_visible(!d->is_visible());
        } else if (key == KEY_D && modifiers == 0) {
            auto model = current_model();
            if (model) {
                std::ostream& output = std::cout;
                output << "----------- " << file_system::simple_name(model->name()) << " -----------\n";
                if (dynamic_cast<SurfaceMesh*>(model)) {
                    auto m = dynamic_cast<SurfaceMesh*>(model);
                    output << "model is a surface mesh. #face: " << std::to_string(m->n_faces())
                              << ", #vertex: " + std::to_string(m->n_vertices())
                              << ", #edge: " + std::to_string(m->n_edges()) << std::endl;
                }
                else if (dynamic_cast<PointCloud*>(model)) {
                    auto m = dynamic_cast<PointCloud*>(model);
                    output << "model is a point cloud. #vertex: " + std::to_string(m->n_vertices()) << std::endl;
                }
                else if (dynamic_cast<Graph*>(model)) {
                    auto m = dynamic_cast<Graph*>(model);
                    output << "model is a graph. #vertex: " + std::to_string(m->n_vertices())
                              << ", #edge: " + std::to_string(m->n_edges()) << std::endl;
                }

                model->property_stats(output);
            }
        } else if (modifiers == MODIF_ALT && key == KEY_C) {
            // toggle clipping plane
            auto *plane = ClippingPlane::instance();
            plane->set_enabled(!plane->is_enabled());
            plane->fit_scene(camera()->scene_center(), camera()->scene_radius());
        } else if (modifiers == MODIF_ALT && key == KEY_V) {
            // toggle clipping plane visibility
            auto *plane = ClippingPlane::instance();
            if (plane->is_enabled())
                plane->set_visible(!plane->is_visible());
        } else if (modifiers == MODIF_ALT && key == KEY_X) {
            auto *plane = ClippingPlane::instance();
            if (plane->is_enabled())
                plane->set_cross_section(!plane->cross_section());
        } else if (modifiers == 0 && key == KEY_A) {
            show_corner_axes_ = !show_corner_axes_;
        } else if (modifiers == 0 && key == KEY_R) {
#ifdef EASY3D_HAS_VULKAN
            LOG(WARNING) << "This viewer shortcut is not yet available in the Vulkan backend";
#else
            // Reload the shader(s) - useful for writing/debugging shader code.
            ShaderManager::reload();
#endif
        } else
            input_.key = key;
        return false;
    }
} // namespace easy3d


namespace easy3d {
    std::vector<graphics::DrawCommand> Viewer::draw_commands(
        const Camera &cam, int width, int, std::vector<DrawSource> *sources, bool include_hidden_models) const {
#ifdef EASY3D_HAS_VULKAN
        impl_->drawable_renderer.prune();
#else
        auto &renderer = graphics_resources().drawable_renderer;
        if (!renderer)
            renderer.reset(new graphics::DrawableRenderer);
        renderer->prune();
#endif
        std::vector<graphics::DrawCommand> commands;
        const auto &planes = clipping_planes();
        // Drawable point/line sizes are logical pixels; the device rasterizes plain
        // points/lines in framebuffer pixels. Scale by the target-to-camera ratio,
        // not dpi_scaling(): it is the DPI scale only when the target is the window
        // framebuffer (live frames, picking, depth reads). A file snapshot captures
        // at output size while the camera keeps its logical size, so the ratio is
        // output/logical and keeps plain sizes in proportion; a MultiViewer cell's
        // integer cell_bounds make it target-cell/logical-cell, approximately the
        // DPI scale. Impostor radii are world-space and already carry the logical
        // sizing through world_pixel_ratio().
        const float pixel_scale = cam.screen_width() > 0 ? float(width) / float(cam.screen_width()) : 1.0f;
        auto append = [&](const std::shared_ptr<Drawable> &d, uint32_t id, const std::shared_ptr<Model> &model,
                          bool selected, bool depth_bias = false) {
            if (!d->is_visible())
                return;
#ifdef EASY3D_HAS_VULKAN
            auto command = impl_->drawable_renderer.command(d, id, cam, planes, selected);
#else
            auto command = graphics_resources().drawable_renderer->command(d, id, cam, planes, selected);
#endif
            command.point_size *= pixel_scale;
            command.line_width *= pixel_scale;
#ifndef EASY3D_HAS_VULKAN
            if (command.geometry->topology() == graphics::Topology::lines &&
                command.line_style == graphics::LineStyle::plain && command.line_width <= 1) {
                command.line_width = 1;
                command.line_screen_space_width = true;
            }
#endif
            command.depth_bias = depth_bias;
            if (render_settings().transparency.enabled && !command.overlay && command.geometry &&
                command.geometry->topology() == graphics::Topology::triangles) {
                if (const auto *triangles = dynamic_cast<const TrianglesDrawable *>(d.get())) {
                    command.transparent = true;
                    command.color.w = triangles->opacity();
                }
            }
            if (sources) {
                if (sources->size() >= UINT32_MAX)
                    throw std::length_error("too many picking sources");
                id = uint32_t(sources->size());
                sources->push_back({model, d});
                command.object_id = id;
            }
            commands.push_back(std::move(command));
        };
        for (size_t i = 0; i < models_.size(); ++i) {
            const auto *renderer = models_[i]->renderer();
            if (!renderer || (!include_hidden_models && !renderer->is_visible()))
                continue;
            // Match the OpenGL viewer: surfaces are offset only while this model
            // also draws visible lines, so coplanar edges do not z-fight.
            bool has_visible_lines = false;
            for (const auto &d : renderer->lines_drawables())
                if (d->is_visible())
                    has_visible_lines = true;
            #ifdef EASY3D_HAS_VULKAN
            for (const auto &d : renderer->triangles_drawables())
                append(d, uint32_t(i), models_[i], renderer->is_selected(), has_visible_lines);
            for (const auto &d : renderer->lines_drawables())
                append(d, uint32_t(i), models_[i], renderer->is_selected());
            for (const auto &d : renderer->points_drawables())
                append(d, uint32_t(i), models_[i], renderer->is_selected());
#else
            for (const auto &d : renderer->lines_drawables())
                append(d, uint32_t(i), models_[i], renderer->is_selected());
            for (const auto &d : renderer->points_drawables())
                append(d, uint32_t(i), models_[i], renderer->is_selected());
            for (const auto &d : renderer->triangles_drawables())
                append(d, uint32_t(i), models_[i], renderer->is_selected(), has_visible_lines);
#endif
        }
        for (size_t i = 0; i < drawables_.size(); ++i)
            append(drawables_[i], uint32_t(models_.size() + i), nullptr, false);
#ifdef EASY3D_HAS_VULKAN
        const auto guide = impl_->clipping_plane_renderer.draw(*ClippingPlane::instance(), cam);
        for (auto command : guide) {
            command.object_id = UINT32_MAX;
            commands.push_back(std::move(command));
        }
#endif
        return commands;
    }
} // namespace easy3d

#ifdef EASY3D_HAS_VULKAN
namespace easy3d {
    std::vector<graphics::DrawCommand> Viewer::frame_commands(
        const Camera &cam, int width, int height, std::vector<DrawSource> *sources, bool include_hud) const {
        auto commands = draw_commands(cam, width, height, sources);
        // Recorded overlay viewports use the host framebuffer's pixels. Keep
        // their NDC transforms and scale the clip rectangles for file output.
        const int fw = framebuffer_width(), fh = framebuffer_height();
        const auto append_overlays = [&](const std::vector<graphics::DrawCommand> &source) {
            for (auto command : source) {
                auto &v = command.viewport;
                if (v.z > 0 && v.w > 0 && fw > 0 && fh > 0 && (width != fw || height != fh)) {
                    const int x = int(std::lround(double(v.x) * width / fw));
                    const int y = int(std::lround(double(v.y) * height / fh));
                    const int right = int(std::lround(double(v.x + v.z) * width / fw));
                    const int bottom = int(std::lround(double(v.y + v.w) * height / fh));
                    if (right <= x || bottom <= y)
                        continue;
                    v = ivec4(x, y, right - x, bottom - y);
                }
                commands.push_back(std::move(command));
            }
        };
        append_overlays(impl_->viewer_annotations);
        if (!impl_->overlays_foreground)
            append_overlays(impl_->overlays);
        if (include_hud) {
            append_overlays(impl_->viewer_3d_overlays);
            append_overlays(impl_->viewer_overlays);
        }
        if (impl_->overlays_foreground)
            append_overlays(impl_->overlays);
        return commands;
    }
    Viewer::Viewer(int width, int height, const DeviceOptions &options)
        : window_(nullptr), width_(width), height_(height), camera_(new Camera), samples_(1),
          background_color_(setting::background_color), show_frame_rate_(false),
          show_easy3d_logo_(false), model_idx_(-1), impl_(new Impl(*this)) {
        resize_render_target(width, height);
        graphics::DeviceOptions device_options;
        device_options.validation = options.validation;
        device_options.sync_validation = options.sync_validation;
        device_options.profiling = options.profiling;
        device_options.use_native_large_points = options.use_native_large_points;
        device_options.device_name = options.device_name;
        impl_->device = graphics::create_device(device_options);
        camera()->set_type(Camera::PERSPECTIVE);
        camera()->set_up_vector(vec3(0, 0, 1));
        camera()->set_view_direction(vec3(-1, 0, 0));
        camera()->show_entire_scene();
        // Transparency effects only know the camera. Rendering and presentation stay
        // in the viewer, as in Viewer::draw().
        Transparency::set_frame_renderer(camera(), [this](const std::vector<TrianglesDrawable *> &surfaces, int method,
                                                          int peels) { draw_transparency(surfaces, method, peels); });
    }
    void Viewer::release_device() {
        impl_->device.reset();
    }
    void Viewer::resize_render_target(int w, int h) {
        if (w <= 0 || h <= 0)
            throw std::invalid_argument("offscreen dimensions must be positive");
        framebuffer_width_ = w;
        framebuffer_height_ = h;
        if (!window_) {
            width_ = w;
            height_ = h;
        }
        // The camera always carries logical window dimensions (framebuffer pixels
        // are GPU-only). For an offscreen canvas logical units equal pixels.
        int cw, ch;
        viewer_size(cw, ch);
        camera()->set_screen_width_and_height(cw, ch);
    }
    void Viewer::set_overlays(std::vector<graphics::DrawCommand> commands, bool foreground) {
        for (auto &command : commands)
            command.overlay = true;
        impl_->overlays = std::move(commands);
        impl_->overlays_foreground = foreground;
    }
    void Viewer::set_viewer_overlays(std::vector<graphics::DrawCommand> commands) const {
        for (auto &command : commands) {
            command.overlay = true;
            command.overlay_depth_test = command.overlay_independent_depth = false;
        }
        impl_->viewer_overlays = std::move(commands);
    }
    void Viewer::set_viewer_3d_overlays(std::vector<graphics::DrawCommand> commands) const {
        for (auto &command : commands)
            command.overlay = true;
        impl_->viewer_3d_overlays = std::move(commands);
    }
    void Viewer::set_viewer_annotations(std::vector<graphics::DrawCommand> commands) const {
        for (auto &command : commands) {
            command.overlay = command.overlay_depth_test = true;
            command.overlay_independent_depth = false;
        }
        impl_->viewer_annotations = std::move(commands);
    }
    void Viewer::set_clipping_planes(const std::vector<vec4> &planes) {
        RenderSettings settings = impl_->render_settings;
        settings.clipping_planes = graphics::normalize_clipping_planes(planes);
        validate_render_settings(settings);
        impl_->clipping_planes = settings.clipping_planes;
        impl_->render_settings.clipping_planes = std::move(settings.clipping_planes);
    }
    const std::vector<vec4> &Viewer::clipping_planes() const {
        return impl_->active_clipping_planes();
    }
    const RenderSettings &Viewer::render_settings() const {
        return impl_->render_settings;
    }
    void Viewer::set_render_settings(const RenderSettings &settings) {
        validate_render_settings(settings);
        impl_->render_settings = settings;
        impl_->render_settings.clipping_planes = graphics::normalize_clipping_planes(settings.clipping_planes);
        impl_->clipping_planes = impl_->render_settings.clipping_planes;
    }
    std::vector<vec3> Viewer::scan_visible_points(unsigned samples) const {
        if (!samples || samples > 64 || (samples & (samples - 1))) // (samples & (samples - 1)) is a bitwise trick to check whether samples is a power of two
            throw std::invalid_argument("invalid capture sample count");
        Camera snapshot(*camera());
        // Camera's copy constructor regenerates matrices from its parameters. Keep
        // effective matrices too, including externally calibrated/custom cameras.
        snapshot.frame()->set_pivot_point(camera()->pivot_point()); // do not re-adjust the copied ortho coefficient
        snapshot.set_projection_matrix(camera()->projection_matrix());
        snapshot.set_model_view_matrix(camera()->model_view_matrix());
        const auto inv = transform::checked_inverse(snapshot.model_view_projection_matrix());
        const auto frame = impl_->capture(snapshot, background_color(), unsigned(supported_sample_count(int(samples))),
                                          framebuffer_width(), framebuffer_height());
        std::vector<vec3> points;
        points.reserve(std::count_if(frame.depth.begin(), frame.depth.end(), [](float d) { return d >= 0 && d < 1; }));
        for (unsigned y = 0; y < frame.height; ++y)
            for (unsigned x = 0; x < frame.width; ++x) {
                const size_t pixel = size_t(y) * frame.width + x;
                const float d = frame.depth[pixel];
                if (frame.ids[pixel].object == UINT32_MAX || !std::isfinite(d) || d < 0 || d >= 1)
                    continue;
                const int viewport[] = {0, 0, int(frame.width), int(frame.height)};
                points.push_back(transform::unproject_inverse(vec3(x + .5f, y + .5f, d), inv, viewport, false));
            }
        return points;
    }
    Viewer::PickResult Viewer::pick(int x, int y) const {
        PickResult result;
        int px, py;
        if (!window_pixel(x, y, px, py))
            return result;
        int fw, fh;
        framebuffer_size(fw, fh);
        // One backend draw re-renders the visible scene with object/primitive IDs and
        // depth. Sampling the cursor pixel therefore names the nearest drawable (and
        // its triangle for meshes) without any CPU ray casting.
        // Keep commands and source mapping together for this exact synchronous draw.
        // This whole-scene query is intentionally not delegated to the Picker classes:
        // they operate on one caller-supplied model with a borrowed camera and own
        // their offscreen resources, while the viewer already owns every model,
        // drawable and the device, so it can answer in a single submission.
        std::vector<DrawSource> sources;
        const auto commands = draw_commands(*camera(), fw, fh, &sources);
        const auto sample = impl_->device->sample_pixel(fw, fh, commands, px, py);
        const auto id = sample.id;
        if (id.object >= sources.size() || sample.depth >= 1)
            return result;
        result.model = sources[id.object].model;
        result.drawable = sources[id.object].drawable;
        result.primitive = id.primitive;
        result.depth = sample.depth;
        result.position = pixel_to_world(px, py, result.depth);
        const auto &faces = result.drawable->cpu_buffers().source_faces;
        auto *mesh = dynamic_cast<SurfaceMesh *>(result.model.get());
        if (mesh && id.primitive < faces.size()) {
            const SurfaceMesh::Face f(faces[id.primitive]);
            if (mesh->is_valid(f) && !mesh->is_deleted(f))
                result.face_index = f.idx();
        }
        return result;
    }
    bool Viewer::read_scene_depth(int x, int y, const Camera &snapshot, float &depth) const {
        const int width = framebuffer_width(), height = framebuffer_height();
        const auto commands = draw_commands(snapshot, width, height);
        const auto sample = impl_->device->sample_pixel(width, height, commands, x, y);
        depth = sample.depth;
        return sample.id.object != UINT32_MAX;
    }
    const graphics::RenderDevice &Viewer::device() const {
        return *impl_->device;
    }
    std::vector<std::uint32_t> Viewer::select_points(const graphics::PointSelectionRequest &request) const {
        return impl_->device->select_points(request);
    }
    std::shared_ptr<const graphics::PointData> Viewer::point_resource(Drawable *drawable) const {
        if (!drawable || drawable->type() != Drawable::DT_POINTS)
            throw std::invalid_argument("selection requires a point drawable");
        for (const auto &model : models_)
            if (model->renderer())
                for (const auto &d : model->renderer()->points_drawables())
                    if (d.get() == drawable)
                        return impl_->drawable_renderer
                            .command(d, 0, *camera(), clipping_planes(), model->renderer()->is_selected())
                            .geometry->point_data();
        for (const auto &d : drawables_)
            if (d.get() == drawable)
                return impl_->drawable_renderer.command(d, 0, *camera(), clipping_planes()).geometry->point_data();
        throw std::invalid_argument("point drawable does not belong to this viewer");
    }
    void Viewer::init() {}
    void Viewer::pre_draw() {}
    void Viewer::post_draw() {}
    void Viewer::draw() const {
        int width = 0, height = 0;
        impl_->frame_size(width, height);
        const auto &settings = render_settings();
        Shadow *active_shadow = Shadow::active(camera());
        const bool legacy_shadow = !settings.shadow.enabled && active_shadow;
        std::vector<DrawSource> sources;
        auto commands =
            frame_commands(*camera(), width, height, legacy_shadow ? &sources : nullptr, !impl_->snapshot.active);
        impl_->apply_settings_effects(commands, *camera(), width, height);
        if (legacy_shadow)
            append_legacy_shadow(commands, sources, *camera(), width, height, active_shadow);
        if (!settings.ambient_occlusion.enabled)
            append_legacy_ao(commands, *camera(), width, height);
        if (!settings.eye_dome_lighting)
            append_legacy_edl(commands, *camera(), width, height);
        if (settings.transparency.enabled)
            impl_->submit(std::move(commands), settings.transparency.method, settings.transparency.layers);
        else
            impl_->submit(std::move(commands));
    }
    void Viewer::draw_transparency(const std::vector<TrianglesDrawable *> &surfaces, int method, int peels) {
        int width = 0, height = 0;
        impl_->frame_size(width, height);
        std::vector<DrawSource> sources;
        auto commands = frame_commands(*camera(), width, height, &sources, !impl_->snapshot.active);
        for (size_t i = 0; i < sources.size() && i < commands.size(); ++i) {
            auto *triangles = dynamic_cast<TrianglesDrawable *>(sources[i].drawable.get());
            if (!triangles || std::find(surfaces.begin(), surfaces.end(), triangles) == surfaces.end())
                continue;
            if (commands[i].geometry && commands[i].geometry->topology() == graphics::Topology::triangles) {
                commands[i].transparent = true;
                commands[i].color.w = triangles->opacity();
            }
        }
        const auto transparency = method == 1   ? graphics::TransparencyMethod::average
                                  : method == 2 ? graphics::TransparencyMethod::peeling
                                  : method == 3 ? graphics::TransparencyMethod::linked_lists
                                                : graphics::TransparencyMethod::sorted;
        impl_->submit(std::move(commands), transparency, peels > 0 ? unsigned(peels) : 0u);
    }
    bool Viewer::snapshot_frame(int width,
                                int height,
                                const mat4 &projection,
                                const vec4 &background,
                                unsigned samples,
                                graphics::FrameImage &image) {
        auto &frame = impl_->snapshot;
        // Keep the camera's logical screen dimensions: only the projection changes
        // for the output frustum. draw_commands() then scales plain point/line sizes
        // by output/logical and impostors keep their relative size, as on OpenGL.
        const mat4 source_projection = camera()->projection_matrix();
        struct Restore {
            Viewer &self;
            mat4 projection;
            ~Restore() {
                self.impl_->snapshot.active = false;
                self.camera()->set_projection_matrix(projection);
            }
        } restore{*this, source_projection};
        frame.width = width;
        frame.height = height;
        frame.samples = samples;
        frame.background = background;
        frame.image = graphics::FrameImage();
        frame.active = true;
        camera()->set_projection_matrix(projection);
        draw();
        if (frame.image.rgba.size() != size_t(width) * size_t(height) * 4)
            throw std::runtime_error("the captured frame does not match the requested size");
        image = std::move(frame.image);
        return true;
    }
    void Viewer::cleanup() {
        release_device();
    }
    Viewer::Viewer(const std::string &title,
                   const WindowOptions &window,
                   const DeviceOptions &options)
        : Viewer(std::make_shared<Window>(title, window.width, window.height, window.samples, window.full_screen,
                                          window.visible, window.resizable), options) {
    }
    Viewer::Viewer(std::shared_ptr<Window> host, const DeviceOptions &options)
        : window_(host->handle), width_(host->width), height_(host->height), title_(host->title), camera_(new Camera),
          samples_(1), background_color_(setting::background_color), show_frame_rate_(false),
          show_easy3d_logo_(true), model_idx_(-1), window_host_(std::move(host)),
          impl_(new Impl(*this)) {
        try {
            // Initialize logging (if it has not been initialized yet)
            if (!logging::is_initialized())
                logging::initialize(false, true, true, false, "default", 9);

            impl_->device = window_host_->device(options);
            manual_ = default_manual;
            { // The default of 4 samples must not fail on a device without 4x MSAA: a valid
                // power of two steps down to a supported count with a warning (as OpenGL's
                // pixel-format chooser does). Malformed counts are left for set_samples() to
                // reject, and a count of 1 - like the constructor's 0 alias - is this
                // backend's "no MSAA", already supported by definition.
                const unsigned requested = window_host_->samples;
                if (requested > 1 && requested <= 64 && !(requested & (requested - 1)))
                    window_host_->samples = unsigned(supported_sample_count(int(requested)));
            }
            set_samples(window_host_->samples);
            window_host_->callbacks(this);
            int w, h;
            framebuffer_size(w, h);
            if (w > 0 && h > 0)
                resize_render_target(w, h);
            camera()->set_up_vector(vec3(0, 0, 1));
            camera()->set_view_direction(vec3(-1, 0, 0));
            camera()->show_entire_scene();
            // Transparency effects only know the camera, so route their frames back to
            // this viewer for rendering and presentation.
            Transparency::set_frame_renderer(camera(),
                                             [this](const std::vector<TrianglesDrawable *> &surfaces, int method,
                                                    int peels) { draw_transparency(surfaces, method, peels); });
            VLOG(1) << "Vulkan device: " << impl_->device->name() << ", driver: "
                    << impl_->device->driver_version() << ", Samples: " << window_host_->samples;
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
        // Base classes are destroyed after members. Release the surface/device now,
        // while the member-owned GLFW window (and its Metal layer on macOS) still live.
        if (window_)
            glfwSetWindowUserPointer(window_, nullptr);
        Transparency::set_frame_renderer(camera(), nullptr);
        release_device();
        LOG(INFO) << "viewer terminated. Bye!";
    }
    float Viewer::dpi_scaling() const {
        if (!window_)
            return 1.0f;
        int w = 0, h = 0;
        glfwGetWindowSize(window(), &w, &h);
        return w > 0 ? float(framebuffer_width()) / w : 1.0f;
    }
    unsigned Viewer::supported_sample_mask() const {
        // The device's color MSAA counts; 1 is always present, because Vulkan has no
        // zero sample count and VK_SAMPLE_COUNT_1_BIT is what disables multisampling.
        return impl_->device ? device().supported_sample_counts() : 1u;
    }
    void Viewer::set_samples(unsigned samples) {
        // 1 means "no MSAA" (this backend has no zero sample count) and 0 is the
        // constructor's alias, so neither is accepted here; the same goes for a count
        // that is not a power of two or above 64. A well-formed count the device cannot
        // do steps down to the largest it can, like the constructor does, and samples()
        // then reports the count actually in use.
        if (!samples || samples > 64 || (samples & (samples - 1))) // (samples & (samples - 1)) is a bitwise trick to check whether samples is a power of two
            throw std::invalid_argument("Vulkan viewer sample count must be a power of two from 1 to 64");
        const unsigned count = unsigned(supported_sample_count(int(samples)));
        window_host_->samples = count;
        samples_ = int(count);
    }
    void Viewer::draw_corner_axes() const {
        if (!show_corner_axes_) {
            set_viewer_3d_overlays({});
            return;
        }
        if (!corner_axes_geometry_) {
            // Same cylinders, cones, sphere and RGB colors as the OpenGL corner axes.
            std::vector<vec3> points, normals, colors;
            for (const auto &axis : {vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)}) {
                shape::create_cylinder(.03f, 10, vec3(0), .5f * axis, axis, points, normals, colors);
                shape::create_cone(.06f, 20, .5f * axis, .7f * axis, axis, points, normals, colors);
            }
            shape::create_sphere(vec3(0), .06f, 20, 20, vec3(0, 1, 1), points, normals, colors);
            std::vector<graphics::Vertex> vertices(points.size());
            for (size_t i = 0; i < points.size(); ++i) {
                vertices[i].position = points[i];
                vertices[i].normal = normals[i];
                vertices[i].color = vec4(colors[i], 1);
            }
            corner_axes_geometry_ = std::make_shared<graphics::Geometry>(std::move(vertices));
        }
        const mat4 view = camera()->orientation().inverse().matrix();
        graphics::DrawCommand axes;
        axes.geometry = corner_axes_geometry_;
        axes.model_view_projection = transform::ortho(-1, 1, -1, 1, -1, 1) * view;
        axes.normal_matrix = mat3(view);
        axes.lighting = true;
        axes.material_lighting = true;
        axes.lighting_two_sides = false;
        axes.ambient = setting::material_ambient;
        axes.specular = setting::material_specular;
        axes.shininess = setting::material_shininess;
        axes.model = view;
        axes.eye = vec3(0);
        axes.light = vec3(setting::light_position);
        axes.object_id = UINT32_MAX;
        axes.set_depth_range(vec2(0.0f, 0.01f));
        axes.overlay = true;
        axes.overlay_depth_test = false;
        axes.overlay_independent_depth = true;
        const int size = std::max(1, std::min({static_cast<int>(100 * dpi_scaling()),
                                              framebuffer_width(), framebuffer_height()}));
        axes.viewport = ivec4(0, framebuffer_height() - size, size, size);
        set_viewer_3d_overlays({axes});
    }

    void Viewer::update_interaction_overlays(bool force) const {
        draw_corner_axes();
        const bool pivot = show_pivot_point_ && std::chrono::steady_clock::now() < pivot_expires_;
        const float dpi = dpi_scaling();
        std::vector<graphics::DrawCommand> commands;
        const bool text_changed = update_hud_text(commands);
        if (!force && !text_changed && pivot == pivot_hint_visible_)
            return;
        if (pivot) {
            // The pivot cross is ten logical pixels long and one logical pixel wide;
            // both backends draw it in framebuffer pixels, so the arm is 10 * dpi and
            // the ribbon width is dpi (one logical pixel; a fixed width of one would be
            // a thinner device-pixel hairline on HiDPI). Screen-space lines are
            // corner-based (texel (i, j) spans [i, i+1) x [j, j+1)), so center the
            // ribbon on the marked pixel's center instead of its corner.
            const float x = (pivot_pixel_.x + 0.5f) * dpi, y = (pivot_pixel_.y + 0.5f) * dpi, size = 10.0f * dpi;
            commands.push_back(graphics::shape::draw_lines_2d(
                {vec2(x - size, y), vec2(x + size, y), vec2(x, y - size), vec2(x, y + size)}, vec4(0, 0, 0, 1),
                framebuffer_width(), framebuffer_height(), dpi));
        }
        if (zoom_region_ && (input_.x != input_.press_x || input_.y != input_.press_y)) {
            // Mouse coordinates are logical; scale into the framebuffer overlay space.
            // The one-logical-pixel wire is centered on its endpoints, so place them on
            // the boundary texels' centers: a wire centered on a texel boundary covers
            // only half of each neighbour and never resolves to a solid color under
            // MSAA, and lavapipe's fill rule can drop it entirely. Same convention as
            // the pivot cross above.
            const Rect rect{(input_.press_x + 0.5f) * dpi, (input_.x + 0.5f) * dpi,
                            (input_.press_y + 0.5f) * dpi, (input_.y + 0.5f) * dpi};
            commands.push_back(
                graphics::shape::draw_quad_filled(rect, vec4(0, 0, 1, .2f), framebuffer_width(), framebuffer_height()));
            commands.push_back(graphics::shape::draw_quad_wire(rect, vec4(0, 0, 1, 1), framebuffer_width(),
                                                               framebuffer_height(), dpi));
        }
        set_viewer_overlays(std::move(commands));
        pivot_hint_visible_ = pivot;
    }
    graphics::FrameImage Viewer::capture() const {
        return capture(samples());
    }
    graphics::FrameImage Viewer::capture(unsigned sample_count) const {
        return capture(sample_count, graphics::FrameReadback::color_depth_ids);
    }
    graphics::FrameImage Viewer::capture(unsigned sample_count, graphics::FrameReadback readback) const {
        if (!initialized_) {
            auto *host = const_cast<Viewer *>(this);
            host->init();
            host->initialized_ = true;
        }
        if (window_) {
            update_interaction_overlays();
        }
        // A count that is not a power of two from 1 to 64 is the caller's mistake and
        // throws, as on OpenGL; a well-formed count the device cannot do steps down to
        // the largest it can, because the capture itself is not a quality debate.
        if (!sample_count || sample_count > 64 || (sample_count & (sample_count - 1)))
            throw std::invalid_argument("invalid capture sample count");
        return impl_->capture(*camera(), background_color(), unsigned(supported_sample_count(int(sample_count))),
                              framebuffer_width(), framebuffer_height(),
                              snapshot_hud_, readback);
    }
} // namespace easy3d
#else // OpenGL
namespace easy3d {
    float Viewer::dpi_scaling() const {
        return dpi_scaling_;
    }
    Viewer::Viewer(const std::string &title,
                   const WindowOptions &window,
                   const DeviceOptions &options)
        : window_(nullptr), dpi_scaling_(1.0), title_(title), samples_(0), // Liangliang: actual samples received known only after window creation
          visible_(window.visible), background_color_(setting::background_color),
          show_easy3d_logo_(true), show_pivot_point_(false), show_frame_rate_(false),
          model_idx_(-1) {
        if (window.width <= 0 || window.height <= 0)
            throw std::invalid_argument("window dimensions must be positive");
        const int samples = window.samples;
        if (samples < 0 || samples > 64 || (samples && (samples & (samples - 1)))) // (samples & (samples - 1)) is a bitwise trick to check whether samples is a power of two
            throw std::invalid_argument("invalid viewer sample count");
        // Avoid locale-related number parsing issues.
        setlocale(LC_NUMERIC, "C");

        animation_func_ = nullptr;

        // Initialize logging (if it has not been initialized yet)
        if (!logging::is_initialized())
            logging::initialize(false, true, true, false, "default", 9);

        // create and setup window
        window_ = create_window(title, window, options);
        try {
            setup_callbacks(window_);

            // create and set up the camera
            camera_ = std::unique_ptr<Camera>(new Camera);
            camera_->set_screen_width_and_height(width_, height_);
            camera_->set_type(Camera::PERSPECTIVE);
            camera_->set_up_vector(vec3(0, 0, 1));       // Z pointing up
            camera_->set_view_direction(vec3(-1, 0, 0)); // X pointing out
            camera_->show_entire_scene();

            easy3d::connect(&camera_->frame_modified, this, &Viewer::update);

            /* Poll for events once before starting a potentially lengthy loading process.*/
            glfwPollEvents();

            manual_ = "--------------------------------------------------------------\n"
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
                      "  'f':                     Fit screen (all models)                 \n"
                      "  'c':                     Fit screen (current model only)         \n"
                      "  'z' + Left click:        Zoom to target point on model           \n"
                      "  'z' + Right click:       Zoom to fit screen                      \n"
                      "  Shift + Left click:      Define a target point on model          \n"
                      "  Shift + Right click:     Undefine the target point (if any) on model\n"
                      "  Ctrl + Left drag:        Zoom to fit region                         \n"
                      "  '-'/'=':                 Decrease/Increase point size               \n"
                      "  '['/']':                 Decrease/Increase line width               \n"
                      "  'b':                     Toggle borders visibility                  \n"
                      "  'e':                     Toggle edges visibility                    \n"
                      "  'v':                     Toggle vertices visibility                 \n"
                      "  'm':                     Toggle smooth shading (for SurfaceMesh)    \n"
                      "  'd':                     Print model info (drawables, properties)   \n"
                      "-------------------------------------------------------------------";
        } catch (...) {
            cleanup();
            throw;
        }
    }
    GLFWwindow *Viewer::create_window(const std::string &title,
                                      const WindowOptions &window,
                                      const DeviceOptions &options) {
        const int samples = window.samples;
        const int gl_major = options.major;
        const int gl_minor = options.minor;
        int width = window.width;
        int height = window.height;

        // The only place the unified sample count is translated into the OpenGL dialect.
        // GLFW_SAMPLES is a *minimum*, and any positive value asks the native pixel-format
        // chooser for a real multisample buffer (a request of 1 measured a 2x context on
        // macOS), while 0 asks for none -- so "one sample means no MSAA" is a zero-sample
        // hint here. The count actually obtained is read back from GL_SAMPLES below.
        const int msaa_hint = samples > 1 ? samples : 0;

        runtime_ = detail::GlfwRuntime::acquire();

        // Reset the hints, allowing viewers to have different hints.
        glfwDefaultWindowHints();

        glfwWindowHint(GLFW_SAMPLES, msaa_hint);

        glfwWindowHint(GLFW_STENCIL_BITS, options.stencil_bits);
        glfwWindowHint(GLFW_DEPTH_BITS, options.depth_bits);

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
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
            if (gl_minor >= 0)
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
        }
#endif

        // make the whole window transparent
        //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

        glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, window.resizable ? GL_TRUE : GL_FALSE);

#ifndef NDEBUG
        const std::string title_str = title + " - Debug Version";
#else
        const std::string title_str = title;
#endif

        GLFWwindow *glfw_window = nullptr;
        if (window.full_screen) {
            GLFWmonitor *monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            width = mode->width;
            height = mode->height;
            glfw_window = glfwCreateWindow(width, height, title_str.c_str(), monitor, runtime_->opengl_share_window());
        } else {
            glfw_window = glfwCreateWindow(width, height, title_str.c_str(), nullptr, runtime_->opengl_share_window());
        }

        if (!glfw_window) {
            std::string error_message =
                "could not create an OpenGL " + std::to_string(gl_major) + "." + std::to_string(gl_minor) + " context!";
            LOG(ERROR) << error_message;
            throw std::runtime_error(error_message);
        }
        const auto previous_context = glfwGetCurrentContext();
        auto close_failed_window = [previous_context](GLFWwindow *failed) {
            glfwDestroyWindow(failed);
            glfwMakeContextCurrent(previous_context);
        };
        std::unique_ptr<GLFWwindow, decltype(close_failed_window)> guard(glfw_window, close_failed_window);
        glfwSetWindowUserPointer(glfw_window, this);
        glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        // Enable vsync
        glfwMakeContextCurrent(glfw_window);
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

        // GL_SAMPLES reports the OpenGL dialect (0 for a plain window), and samples_
        // keeps that value; the public samples() normalizes it to the unified 1.
        glGetIntegerv(GL_SAMPLES, &samples_);
        int max_num = 0;
        glGetIntegerv(GL_MAX_SAMPLES, &max_num);

        // warn the user if the requests were not satisfied. msaa_hint == 0 is the
        // request for no MSAA, which is satisfied by the 0 samples received.
        if (msaa_hint > 0 && samples_ != msaa_hint) {
            if (samples_ == 0)
                LOG(WARNING) << "MSAA not available (samples received: " << samples_ << ", requested: " << samples << ", max support: " << max_num << ")";
            else
                LOG(WARNING) << "MSAA with " << samples_ << " samples (requested: " << samples << ", max support: " << max_num << ")";
        } else
            VLOG(1) << "samples received: " << samples_ << " (requested: " << samples << ", max support: " << max_num << ")";

        // the request viewer size might not be satisfied, thus we query the actual size
        glfwGetWindowSize(glfw_window, &width_, &height_);
        VLOG(1) << "viewer size (width/height): " << width_ << "/" << height_;

        float xscale(0), yscale(0);
        glfwGetWindowContentScale(glfw_window, &xscale, &yscale);
        LOG_IF(xscale != yscale, ERROR) << "glfw_window content scale in X and Y directions not identical (X: " << xscale
                                        << ", Y: " << yscale << ")";
        dpi_scaling_ = static_cast<float>(xscale + yscale) * 0.5f;
        VLOG(1) << "DPI scaling: " << dpi_scaling();

        glfwGetFramebufferSize(glfw_window, &framebuffer_width_, &framebuffer_height_);
        runtime_->attach_opengl_window(glfw_window);
        return guard.release();
    }
    template <class Callback> void Viewer::dispatch_callback(GLFWwindow *window, const Callback &callback) noexcept {
        auto *viewer = static_cast<Viewer *>(glfwGetWindowUserPointer(window));
        if (!viewer)
            return;
        try {
            glfwMakeContextCurrent(window);
            callback(*viewer);
            if (viewer->callback_error_)
                glfwSetWindowShouldClose(window, GLFW_TRUE);
        } catch (...) {
            if (!viewer->callback_error_)
                viewer->callback_error_ = std::current_exception();
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
    void Viewer::setup_callbacks(GLFWwindow *window) {
        glfwSetWindowRefreshCallback(window,
                                     [](GLFWwindow *win) { dispatch_callback(win, [](Viewer &v) { v.update(); }); });
        // Camera/input coordinates are logical window units; the shared ViewerBase
        // routers own all input state and event ordering on both backends.
        glfwSetCursorPosCallback(window, [](GLFWwindow *win, double x, double y) {
            dispatch_callback(win, [&](Viewer &v) {
                v.handle_cursor_pos(int(x), int(y), detail::GlfwRuntime::modifiers(win));
            });
        });
        glfwSetMouseButtonCallback(window, [](GLFWwindow *win, int button, int action, int modifiers) {
            dispatch_callback(win, [&](Viewer &v) {
                double x, y;
                glfwGetCursorPos(win, &x, &y);
                v.handle_mouse_button(int(x), int(y), button, action, modifiers);
            });
        });
        glfwSetKeyCallback(window, [](GLFWwindow *win, int key, int, int action, int mods) {
            dispatch_callback(win, [&](Viewer &v) { v.handle_key(key, action, mods); });
        });
        glfwSetCharCallback(window, [](GLFWwindow *win, unsigned int codepoint) {
            dispatch_callback(win, [&](Viewer &v) { v.handle_character(codepoint); });
        });
        glfwSetDropCallback(window, [](GLFWwindow *win, int count, const char **filenames) {
            dispatch_callback(win, [&](Viewer &v) {
                std::vector<std::string> names(count);
                for (int i = 0; i < count; ++i)
                    names[i] = filenames[i];
                v.handle_drop(names);
            });
        });
        glfwSetScrollCallback(window, [](GLFWwindow *win, double dx, double dy) {
            dispatch_callback(win, [&](Viewer &v) {
                double x, y;
                glfwGetCursorPos(win, &x, &y);
                v.handle_scroll(int(x), int(y), dx, dy);
            });
        });
        glfwSetWindowSizeCallback(window, [](GLFWwindow *win, int w, int h) {
            dispatch_callback(win, [&](Viewer &v) { v.callback_event_resize(w, h); });
        });
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *win, int, int) {
            dispatch_callback(win, [&](Viewer &v) {
                int w, h;
                glfwGetWindowSize(win, &w, &h);
                v.callback_event_resize(w, h);
            });
        });
        glfwSetWindowFocusCallback(window, [](GLFWwindow *win, int focused) {
            dispatch_callback(win, [&](Viewer &v) { v.handle_focus(focused != 0); });
        });
        glfwSetWindowCloseCallback(
            window, [](GLFWwindow *win) { dispatch_callback(win, [](Viewer &v) { v.request_close(); }); });
    }
    void Viewer::callback_event_resize(int w, int h) {
        try {
            int fw, fh;
            glfwGetFramebufferSize(window_, &fw, &fh);
            width_ = w;
            height_ = h;
            framebuffer_width_ = fw;
            framebuffer_height_ = fh;
            if (w > 0 && h > 0 && (camera_->screen_width() != w || camera_->screen_height() != h))
                camera_->set_screen_width_and_height(w, h);
            glViewport(0, 0, fw, fh);
            notify_resize();
        } catch (const std::exception &e) {
            if (!callback_error_)
                callback_error_ = std::current_exception();
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
        }
    }
    Viewer::~Viewer() {
        cleanup();
        LOG(INFO) << "viewer terminated. Bye!";
    }
    void Viewer::cleanup() {
        // viewer may have already been destroyed by the user
        if (!window_) {
#ifndef EASY3D_HAS_VULKAN
            opengl_viewer_states().erase(this);
#endif
            return;
        }

#ifndef EASY3D_HAS_VULKAN
        detail::OpenGLContextScope context_guard(window_);
#else
        const auto previous_context = glfwGetCurrentContext();
        glfwMakeContextCurrent(window_);
#endif
        glfwSetWindowUserPointer(window_, nullptr);
        camera_.reset();
        opengl_viewer_state(this).corner_axes_commands.clear();
        text_layout_.reset();
        hud_overlay_renderer_.reset();
        overlay_renderer_.reset();
        graphics_resources_.reset();

        clear_scene();
        clear_opengl_effect_cache(opengl_viewer_state(this));

        if (runtime_->last_opengl_window()) {
            ShaderManager::terminate();
            TextureManager::terminate();
        }

        const auto closing = window_;
        runtime_->detach_opengl_window(window_);
        glfwDestroyWindow(window_);
        window_ = nullptr;
#ifndef EASY3D_HAS_VULKAN
        context_guard.restore();
#else
        if (previous_context && previous_context != closing)
            glfwMakeContextCurrent(previous_context);
#endif
        runtime_.reset();
#ifndef EASY3D_HAS_VULKAN
        opengl_viewer_states().erase(this);
#endif
    }
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
        if (camera_->screen_width() != width_ || camera_->screen_height() != height_)
            camera_->set_screen_width_and_height(width_, height_);
        int fw, fh;
        glfwGetFramebufferSize(window_, &fw, &fh);
        glViewport(0, 0, fw, fh);

        ensure_text_layout_engine();

        initialized_ = true;
        print_usage();
    }
    void Viewer::draw_corner_axes() const {
        if (!show_corner_axes_)
            return;
        if (opengl_viewer_state(this).corner_axes_commands.empty()) {
            float base = 0.5f; // the cylinder length, relative to the allowed region
            float head = 0.2f; // the cone length, relative to the allowed region
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
            std::vector<graphics::Vertex> vertices(points.size());
            for (size_t i = 0; i < points.size(); ++i) {
                vertices[i].position = points[i];
                vertices[i].normal = normals[i];
                vertices[i].color = vec4(colors[i], 1);
            }
            graphics::DrawCommand axes;
            axes.geometry = std::make_shared<graphics::Geometry>(std::move(vertices));
            axes.lighting = true;
            axes.material_lighting = true;
            axes.lighting_two_sides = false;
            axes.ambient = setting::material_ambient;
            axes.specular = setting::material_specular;
            axes.shininess = setting::material_shininess;
            axes.color = vec4(1);
            axes.object_id = UINT32_MAX;
            opengl_viewer_state(this).corner_axes_commands.push_back(std::move(axes));
        }
        const mat4 &projection = transform::ortho(-1, 1, -1, 1, -1, 1);
        const mat4 &view = camera_->orientation().inverse().matrix();
        for (auto &axes : opengl_viewer_state(this).corner_axes_commands) {
            axes.model_view_projection = projection * view;
            axes.model_view = view;
            axes.model = view;
            axes.normal_matrix = mat3(view);
            axes.projection = projection;
            axes.eye = vec3(0);
            axes.light = vec3(setting::light_position);
            const int size = std::max(1, std::min({static_cast<int>(100 * dpi_scaling()),
                                                  framebuffer_width(), framebuffer_height()}));
            axes.viewport = ivec4(0, framebuffer_height() - size, size, size);
            axes.set_depth_range(vec2(0.0f, 0.01f));
            axes.overlay = true;
            axes.overlay_depth_test = false;
            axes.overlay_independent_depth = true;
        }
    }
    void Viewer::pre_draw() {
        glfwMakeContextCurrent(window_);
        glClearColor(background_color_[0], background_color_[1], background_color_[2], 1.0f);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    void Viewer::post_draw() {
        std::vector<graphics::DrawCommand> commands;
        update_hud_text(commands);
        draw_corner_axes();
        if (show_corner_axes_)
            commands.insert(commands.end(), opengl_viewer_state(this).corner_axes_commands.begin(), opengl_viewer_state(this).corner_axes_commands.end());

        // Shown only when it is not animating. Screen-space coordinates are
        // framebuffer pixels and the cross is centered on the picked pixel.
#ifdef EASY3D_HAS_VULKAN
        const vec2 pivot_pixel = pivot_pixel_;
        const auto pivot_expires = pivot_expires_;
#else
        const auto &pivot_state = opengl_viewer_state(this);
        const vec2 pivot_pixel = pivot_state.pivot_pixel;
        const auto pivot_expires = pivot_state.pivot_expires;
#endif
        if (show_pivot_point_ && std::chrono::steady_clock::now() < pivot_expires) {
            const float dpi = dpi_scaling();
            const float px = (pivot_pixel.x + 0.5f) * dpi, py = (pivot_pixel.y + 0.5f) * dpi, size = 10.0f * dpi;
            commands.push_back(graphics::shape::draw_lines_2d(
                {vec2(px - size, py), vec2(px + size, py), vec2(px, py - size), vec2(px, py + size)},
                vec4(0, 0, 0, 1), framebuffer_width(), framebuffer_height(), dpi));
        }

        if (zoom_region_ && (input_.x != input_.press_x || input_.y != input_.press_y)) {
            const float dpi = dpi_scaling();
            const Rect rect{(input_.press_x + 0.5f) * dpi, (input_.x + 0.5f) * dpi,
                            (input_.press_y + 0.5f) * dpi, (input_.y + 0.5f) * dpi};
            commands.push_back(graphics::shape::draw_quad_filled(rect, vec4(0, 0, 1, .2f),
                                                                 framebuffer_width(), framebuffer_height()));
            commands.push_back(graphics::shape::draw_quad_wire(rect, vec4(0, 0, 1, 1),
                                                               framebuffer_width(), framebuffer_height(), dpi));
        }

        if (!commands.empty()) {
            if (!hud_overlay_renderer_)
                hud_overlay_renderer_.reset(new opengl::OverlayRenderer);
            hud_overlay_renderer_->set(std::move(commands), true);
            opengl::QueryState state;
            state.prepare(framebuffer_width(), framebuffer_height());
            hud_overlay_renderer_->draw(true, framebuffer_width(), framebuffer_height(), overlay_tile_, {});
        } else if (hud_overlay_renderer_) {
            hud_overlay_renderer_->set({}, true);
        }
        draw_overlays(true);
    }
    namespace {
        bool opengl_linked_lists_available() {
            GLint major = 0, minor = 0;
            glGetIntegerv(GL_MAJOR_VERSION, &major);
            glGetIntegerv(GL_MINOR_VERSION, &minor);
            return major > 4 || (major == 4 && minor >= 3);
        }

        bool opengl_float_blending_available() {
            GLint major = 0, minor = 0;
            glGetIntegerv(GL_MAJOR_VERSION, &major);
            glGetIntegerv(GL_MINOR_VERSION, &minor);
            return major > 4 || (major == 4 && minor >= 4) ||
                   glewIsSupported("GL_ARB_color_buffer_float") ||
                   glewIsSupported("GL_EXT_color_buffer_float") ||
                   glewIsSupported("GL_EXT_float_blend");
        }

        Shadow *prepare_settings_shadow(ViewerOpenGLState &state, Camera *camera, const ShadowSettings &settings) {
            const bool soft = settings.softness > 0.0f;
            auto *current = dynamic_cast<SoftShadow *>(state.settings_shadow.get());
            if (!state.settings_shadow || state.settings_shadow_camera != camera ||
                (soft ? current == nullptr : current != nullptr)) {
                if (soft)
                    state.settings_shadow.reset(new SoftShadow(camera));
                else
                    state.settings_shadow.reset(new Shadow(camera));
                state.settings_shadow_camera = camera;
            }
            state.settings_shadow->set_shadow_map_size(int(settings.map_size));
            state.settings_shadow->set_light_distance(settings.light_distance);
            state.settings_shadow->set_darkness(settings.darkness);
            state.settings_shadow->set_virtual_background(settings.virtual_background);
            state.settings_shadow->set_virtual_background_color(settings.virtual_background_color);
            if (soft)
                static_cast<SoftShadow *>(state.settings_shadow.get())->set_softness(settings.softness);
            return state.settings_shadow.get();
        }

        Transparency *prepare_settings_transparency(ViewerOpenGLState &state, Camera *camera,
                                                    graphics::TransparencyMethod method, unsigned layers) {
            switch (method) {
                case graphics::TransparencyMethod::average:
                    if (!state.settings_average)
                        state.settings_average.reset(new AverageColorBlending(camera));
                    return state.settings_average.get();
                case graphics::TransparencyMethod::peeling:
                    if (!state.settings_peeling)
                        state.settings_peeling.reset(new DualDepthPeeling(camera));
                    state.settings_peeling->set_max_peels(int(layers));
                    return state.settings_peeling.get();
                case graphics::TransparencyMethod::linked_lists:
                    if (!state.settings_linked)
                        state.settings_linked.reset(new PerPixelLinkedListsOIT(camera));
                    state.settings_linked->set_max_fragments_per_pixel(int(layers));
                    return state.settings_linked.get();
                case graphics::TransparencyMethod::sorted:
                    break;
            }
            return nullptr;
        }
    } // namespace

    void Viewer::draw() const {
        auto &state = opengl_viewer_state(this);
        const RenderSettings &settings = state.render_settings;
        // Settings values are checked before any fallback choice. Backend support
        // and fallback policy belong to RenderDevice; setters already validate, and
        // repeating it here keeps direct state edits honest too.
        try {
            validate_render_settings(settings);
        } catch (const std::exception &e) {
            LOG_N_TIMES(1, WARNING) << "invalid render settings: " << e.what();
            throw;
        }

        Shadow *active_shadow = Shadow::active(camera());
        if (settings.shadow.enabled && active_shadow && active_shadow != state.settings_shadow.get())
            LOG_N_TIMES(1, WARNING) << "RenderSettings shadow takes precedence over the legacy active shadow";
        GLint viewport[4] = {0, 0, 0, 0};
        glGetIntegerv(GL_VIEWPORT, viewport);
        const int width = viewport[2], height = viewport[3];
        const bool effects_enabled = settings.shadow.enabled || settings.ambient_occlusion.enabled ||
                                     settings.eye_dome_lighting;
        const bool transparency_enabled = settings.transparency.enabled;
        // An application-owned active legacy shadow requires the exact native
        // fallback: Shadow::begin()/end() only bracket the pass, and the composite
        // is produced by Shadow::draw().
        const bool legacy_shadow = !settings.shadow.enabled && active_shadow;

        // Only scenes that cannot be encoded as DrawCommand use the exact native
        // pass. The device owns every settings effect and every transparency method.
        bool portable = width > 0 && height > 0 && !legacy_shadow;
        const auto supported = [&](const std::shared_ptr<Drawable> &d) {
            if (!d)
                return false;
            if (d->is_ao_enabled() && d->has_ao_texture()) {
                LOG_N_TIMES(1, WARNING) << "native drawable fallback: AO on '" << d->name() << "'";
                return false;
            }
            const Drawable &drawable = *d;
            const bool result = typeid(drawable) == typeid(TrianglesDrawable) ||
                                typeid(drawable) == typeid(LinesDrawable) ||
                                typeid(drawable) == typeid(PointsDrawable) ||
                                typeid(drawable) == typeid(LinesDrawable2D);
            if (!result)
                LOG_N_TIMES(1, WARNING) << "native drawable fallback: custom type '" << d->name() << "' ("
                                        << typeid(drawable).name() << ")";
            return result;
        };
        for (const auto &m : models_) {
            if (!m->renderer() || !m->renderer()->is_visible())
                continue;
            for (const auto &d : m->renderer()->triangles_drawables())
                portable = portable && supported(d);
            for (const auto &d : m->renderer()->lines_drawables())
                portable = portable && supported(d);
            for (const auto &d : m->renderer()->points_drawables())
                portable = portable && supported(d);
        }
        for (const auto &d : drawables_)
            portable = portable && supported(d);

        const auto warn_native_clipping = [&]() {
            if (!settings.clipping_planes.empty() && !ClippingPlane::instance()->is_enabled()) {
                LOG_N_TIMES(1, WARNING) << "OpenGL native rendering applies the process-wide clipping plane only; "
                                           "RenderSettings::clipping_planes are ignored";
            }
        };
        const auto submit = [&](std::vector<graphics::DrawCommand> commands,
                                graphics::TransparencyMethod method, unsigned layers) {
            query_device().present(uint32_t(width), uint32_t(height), commands,
                                   state.snapshot_active ? state.snapshot_background : background_color(),
                                   state.snapshot_active ? state.snapshot_samples : unsigned(samples()),
                                   method, layers);
            if (method == graphics::TransparencyMethod::linked_lists &&
                query_device().oit_dropped_fragments() > 0)
                LOG_N_TIMES(1, WARNING) << "per-pixel linked lists dropped "
                                        << query_device().oit_dropped_fragments()
                                        << " transparent fragments; increase RenderSettings::transparency.layers";
        };

        const auto visible_triangles = [&]() {
            std::vector<TrianglesDrawable *> surfaces;
            for (const auto &model : models_) {
                if (!model->renderer()->is_visible())
                    continue;
                for (const auto &d : model->renderer()->triangles_drawables())
                    if (d->is_visible())
                        surfaces.push_back(d.get());
            }
            for (const auto &d : drawables_) {
                if (!d->is_visible())
                    continue;
                if (auto *triangles = dynamic_cast<TrianglesDrawable *>(d.get()))
                    surfaces.push_back(triangles);
            }
            return surfaces;
        };
        const auto native_scene = [&](bool skip_all_triangles, Shadow *skip_shadow) {
            const auto shadow_surface = [&](TrianglesDrawable *d) {
                if (!skip_shadow)
                    return false;
                const auto &surfaces = skip_shadow->surfaces();
                return std::find(surfaces.begin(), surfaces.end(), d) != surfaces.end();
            };
            for (const auto m : models_) {
                if (!m->renderer()->is_visible())
                    continue;
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
                    if (!d->is_visible() || skip_all_triangles || shadow_surface(d.get()))
                        continue;
                    d->draw(camera());
                    easy3d_debug_log_gl_error
                }
                if (count > 0)
                    glDisable(GL_POLYGON_OFFSET_FILL);
            }
            for (auto d : drawables_) {
                if (!d->is_visible())
                    continue;
                if (skip_all_triangles && dynamic_cast<TrianglesDrawable *>(d.get()))
                    continue;
                d->draw(camera());
            }
        };

        if (portable) {
            try {
                auto commands = draw_commands(*camera(), width, height);
                build_settings_effects(commands, *camera(), width, height, settings);
                if (!settings.ambient_occlusion.enabled)
                    append_legacy_ao(commands, *camera(), width, height);
                if (!settings.eye_dome_lighting)
                    append_legacy_edl(commands, *camera(), width, height);
                if (transparency_enabled)
                    submit(std::move(commands), settings.transparency.method, settings.transparency.layers);
                else
                    submit(std::move(commands), graphics::TransparencyMethod::sorted, 4);
                graphics_resources().drawable_renderer->prune();
            } catch (const std::exception &e) {
                LOG_N_TIMES(1, WARNING) << "native drawable fallback: " << e.what();
                portable = false;
            }
        }

        if (!portable) {
            warn_native_clipping();
            if (effects_enabled) {
                if (transparency_enabled)
                    LOG_N_TIMES(1, WARNING) << "OpenGL native effects cannot be composed with the RenderSettings "
                                               "transparency pass; rendering the effects without transparency";
                if (settings.ambient_occlusion.enabled && settings.eye_dome_lighting)
                    LOG_N_TIMES(1, WARNING) << "OpenGL cannot compose ambient occlusion with eye-dome lighting; "
                                               "eye-dome lighting takes precedence";

                Shadow *effect_shadow = nullptr;
                const std::vector<TrianglesDrawable *> *effect_surfaces = nullptr;
                std::vector<TrianglesDrawable *> settings_surfaces;
                if (settings.shadow.enabled) {
                    if (active_shadow && active_shadow != state.settings_shadow.get())
                        LOG_N_TIMES(1, WARNING) << "RenderSettings shadow takes precedence over the legacy active shadow";
                    settings_surfaces = visible_triangles();
                    effect_shadow = prepare_settings_shadow(state, const_cast<Camera *>(camera()), settings.shadow);
                    effect_surfaces = &settings_surfaces;
                } else if (active_shadow) {
                    effect_shadow = active_shadow;
                    effect_surfaces = &active_shadow->surfaces();
                }

                EyeDomeLighting *edl = nullptr;
                AmbientOcclusion *ao = nullptr;
                if (settings.eye_dome_lighting) {
                    if (!state.settings_edl)
                        state.settings_edl.reset(new EyeDomeLighting(const_cast<Camera *>(camera())));
                    edl = state.settings_edl.get();
                    edl->begin();
                } else if (settings.ambient_occlusion.enabled) {
                    if (!state.settings_ao)
                        state.settings_ao.reset(new AmbientOcclusion(const_cast<Camera *>(camera())));
                    ao = state.settings_ao.get();
                    ao->set_radius_ratio(settings.ambient_occlusion.radius_ratio);
                    ao->set_bias(settings.ambient_occlusion.bias);
                    ao->set_intensity(settings.ambient_occlusion.intensity);
                    ao->begin(models_);
                }

                native_scene(settings.shadow.enabled, settings.shadow.enabled ? nullptr : effect_shadow);
                if (effect_shadow && !effect_surfaces->empty())
                    effect_shadow->draw(*effect_surfaces);
                if (edl)
                    edl->end();
                else if (ao)
                    ao->end();
            } else if (transparency_enabled) {
                graphics::TransparencyMethod method = settings.transparency.method;
                if (method == graphics::TransparencyMethod::linked_lists && !opengl_linked_lists_available()) {
                    LOG_N_TIMES(1, WARNING) << "OpenGL per-pixel linked lists require OpenGL 4.3; "
                                               "falling back to depth peeling";
                    method = graphics::TransparencyMethod::peeling;
                } else if (method == graphics::TransparencyMethod::average && !opengl_float_blending_available()) {
                    LOG_N_TIMES(1, WARNING) << "OpenGL average-color blending requires floating-point color blending; "
                                               "falling back to depth peeling";
                    method = graphics::TransparencyMethod::peeling;
                }
                // Every native transparency method bypasses the legacy shadow because
                // native_scene() excludes it, so diagnose that once regardless of method.
                if (active_shadow)
                    LOG_N_TIMES(1, WARNING) << "the legacy active shadow cannot be combined with RenderSettings "
                                               "transparency; ignoring the legacy shadow";
                if (method == graphics::TransparencyMethod::sorted) {
                    LOG_N_TIMES(1, WARNING) << "sorted transparency fallback: depth peeling";
                    method = graphics::TransparencyMethod::peeling;
                }
                const unsigned layers = std::min(settings.transparency.layers,
                                                  unsigned(std::numeric_limits<int>::max()));
                native_scene(true, nullptr);
                if (auto *transparency = prepare_settings_transparency(
                        state, const_cast<Camera *>(camera()), method, layers))
                    transparency->draw(visible_triangles());
            } else {
                // Preserve the exact legacy frame: exclude the shadow caster surfaces
                // from the ordinary scene, then let Shadow::draw() render its own
                // shadow map and composite.
                native_scene(false, active_shadow);
                if (active_shadow)
                    active_shadow->draw(active_shadow->surfaces());
            }
        }


        // draw the clipping plane (not in postDraw so it can be effective when taking snapshots)
        ClippingPlane::instance()->draw(camera());
        easy3d_debug_log_gl_error
        draw_overlays(false);
    }
    void Viewer::append_settings_effects(std::vector<graphics::DrawCommand> &commands,
                                         const Camera &camera, int width, int height) const {
        if (commands.empty())
            return;
        validate_render_settings(opengl_viewer_state(this).render_settings);
        build_settings_effects(commands, camera, width, height, opengl_viewer_state(this).render_settings);
    }

    namespace {
        using QueryContext = detail::OpenGLContextScope;
        using opengl::QueryState;
        void query_draw(const Drawable *drawable, const Camera &camera) {
            if (!drawable || !drawable->is_visible() || dynamic_cast<const LinesDrawable2D *>(drawable))
                return;
            // Unsupported states use the exact native drawable implementation,
            // including application overrides. Custom Viewer::draw() and frame
            // hooks do not run in a scene query.
            drawable->draw(&camera);
        }
    } // namespace

    bool Viewer::supports_device_query(const Camera &camera, int width, int height, bool guides) const {
        if (width <= 0 || height <= 0)
            return false;
        const auto supported = [&](const std::shared_ptr<Drawable> &drawable) {
            if (!drawable || !drawable->is_visible())
                return true;
            if (dynamic_cast<LinesDrawable2D *>(drawable.get())) {
                if (!guides)
                    return true;
                const auto *lines = static_cast<const LinesDrawable2D *>(drawable.get());
                return lines->coloring_method() == State::UNIFORM_COLOR &&
                       std::abs(lines->line_width() * float(width) / float(camera.screen_width()) - 1.0f) < 1e-6f;
            }
            if (drawable->coloring_method() != State::UNIFORM_COLOR &&
                drawable->coloring_method() != State::COLOR_PROPERTY &&
                drawable->coloring_method() != State::TEXTURED &&
                drawable->coloring_method() != State::SCALAR_FIELD)
                return false;
            if (drawable->color().w != 1.0f ||
                (drawable->is_ao_enabled() && drawable->has_ao_texture()))
                return false;
            const bool textured = drawable->coloring_method() == State::TEXTURED ||
                                  drawable->coloring_method() == State::SCALAR_FIELD;
            if (textured && (!drawable->texture() || !drawable->texture()->pixels()))
                return false;
            const Drawable &value = *drawable;
            return typeid(value) == typeid(PointsDrawable) ||
                   typeid(value) == typeid(LinesDrawable) ||
                   typeid(value) == typeid(TrianglesDrawable);
        };
        for (const auto &model : models_) {
            if (!model->renderer() || !model->renderer()->is_visible())
                continue;
            for (const auto &d : model->renderer()->triangles_drawables())
                if (!supported(d))
                    return false;
            for (const auto &d : model->renderer()->lines_drawables())
                if (!supported(d))
                    return false;
            for (const auto &d : model->renderer()->points_drawables())
                if (!supported(d))
                    return false;
        }
        for (const auto &d : drawables_)
            if (!supported(d))
                return false;
        return true;
    }

    void Viewer::prune_drawable_cache() const {
        auto &renderer = graphics_resources().drawable_renderer;
        if (renderer)
            renderer->prune();
    }

    graphics::RenderDevice &Viewer::query_device() const {
        auto &device = graphics_resources().query_device;
        if (!device)
            device = graphics::create_device();
        return *device;
    }

    const graphics::RenderDevice &Viewer::device() const {
        return query_device();
    }

    bool Viewer::device_query_commands(const Camera &camera,
                                       int width,
                                       int height,
                                       bool guides,
                                       std::vector<graphics::DrawCommand> &commands,
                                       std::vector<DrawSource> *sources) const {
        if (!supports_device_query(camera, width, height, guides))
            return false;
        if (const auto *multi_viewer = dynamic_cast<const MultiViewer *>(this))
            commands = multi_viewer->draw_commands(camera, width, height, sources);
        else
            commands = draw_commands(camera, width, height, sources);
        if (!guides) {
            std::vector<DrawSource> visible_sources;
            for (auto &command : commands) {
                if (command.overlay) {
                    if (sources)
                        continue;
                    command.object_id = UINT32_MAX;
                } else if (sources) {
                    const auto source_id = command.object_id;
                    command.object_id = uint32_t(visible_sources.size());
                    visible_sources.push_back(sources->at(source_id));
                }
            }
            if (sources)
                *sources = std::move(visible_sources);
            commands.erase(std::remove_if(commands.begin(), commands.end(), [](const graphics::DrawCommand &command) {
                return command.overlay;
            }), commands.end());
        }
        graphics_resources().drawable_renderer->prune();
        return true;
    }

    const std::vector<vec4> &Viewer::clipping_planes() const {
        selection_planes_ = graphics::effective_clipping_planes(render_settings().clipping_planes,
                                                               *ClippingPlane::instance());
        return selection_planes_;
    }
    void Viewer::set_clipping_planes(const std::vector<vec4> &planes) {
        RenderSettings settings = render_settings();
        settings.clipping_planes = graphics::normalize_clipping_planes(planes);
        validate_render_settings(settings);
        opengl_viewer_state(this).render_settings = std::move(settings);
    }
    const RenderSettings &Viewer::render_settings() const {
        return opengl_viewer_state(this).render_settings;
    }
    void Viewer::set_render_settings(const RenderSettings &settings) {
        validate_render_settings(settings);
        auto normalized = settings;
        normalized.clipping_planes = graphics::normalize_clipping_planes(settings.clipping_planes);
        opengl_viewer_state(this).render_settings = std::move(normalized);
    }
    std::shared_ptr<const graphics::PointData> Viewer::point_resource(Drawable *drawable) const {
        if (!drawable || drawable->type() != Drawable::DT_POINTS)
            throw std::invalid_argument("selection requires a point drawable");
        std::shared_ptr<Drawable> owner;
        for (const auto &model : models_)
            if (model->renderer())
                for (const auto &d : model->renderer()->points_drawables())
                    if (d.get() == drawable)
                        owner = d;
        for (const auto &d : drawables_)
            if (d.get() == drawable)
                owner = d;
        if (!owner)
            throw std::invalid_argument("point drawable does not belong to this viewer");
        QueryContext context(window_);
        auto &renderer = graphics_resources().drawable_renderer;
        if (!renderer)
            renderer.reset(new graphics::DrawableRenderer);
        return renderer->command(owner, 0, *camera(), clipping_planes()).geometry->point_data();
    }
    std::vector<std::uint32_t> Viewer::select_points(const graphics::PointSelectionRequest &request) const {
        QueryContext context(window_);
        return query_device().select_points(request);
    }
    void Viewer::set_overlays(std::vector<graphics::DrawCommand> commands, bool foreground) {
        QueryContext context(window_);
        QueryState state;
        state.prepare(framebuffer_width(), framebuffer_height());
        if (!overlay_renderer_)
            overlay_renderer_.reset(new opengl::OverlayRenderer);
        overlay_renderer_->set(std::move(commands), foreground);
        update();
    }
    void Viewer::draw_overlays(bool foreground) const {
        if (!overlay_renderer_)
            return;
        QueryState state;
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        state.prepare(viewport[2], viewport[3]);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        overlay_renderer_->draw(foreground, framebuffer_width(), framebuffer_height(), overlay_tile_,
                                [&](int width, int height) {
                                    QueryState depth_state;
                                    depth_state.prepare(width, height);
                                    Camera snapshot(*camera());
                                    snapshot.frame()->set_pivot_point(camera()->pivot_point());
                                    snapshot.projection_matrix();
                                    snapshot.model_view_matrix();
                                    mat4 projection = camera()->projection_matrix();
                                    if (overlay_tile_.z > 0) {
                                        mat4 full = mat4::identity();
                                        full(0, 0) = float(viewport[2]) / width;
                                        full(1, 1) = float(viewport[3]) / height;
                                        full(0, 3) = float(2 * overlay_tile_.x + viewport[2]) / width - 1;
                                        full(1, 3) = 1 - float(2 * overlay_tile_.y + viewport[3]) / height;
                                        projection = full * projection;
                                    }
                                    snapshot.set_projection_matrix(projection);
                                    snapshot.set_model_view_matrix(camera()->model_view_matrix());
                                    draw_scene(snapshot);
                                });
    }
    unsigned Viewer::supported_sample_mask() const {
        // Offscreen multisample storage is capped by GL_MAX_SAMPLES, and reading it
        // needs a current context. Without a window only single-sample is promised.
        if (!window_)
            return 1;
        QueryContext context(window_);
        return query_device().supported_sample_counts(graphics::PointStyle::plain);
    }
    graphics::FrameImage Viewer::capture() const {
        return capture(samples());
    }
    graphics::FrameImage Viewer::capture(unsigned count) const {
        return capture(count, graphics::FrameReadback::color_depth_ids);
    }
    graphics::FrameImage Viewer::capture(unsigned count, graphics::FrameReadback readback) const {
        if (!count || count > 64 || (count & (count - 1)))
            throw std::invalid_argument("invalid capture sample count");
        QueryContext context(window_);
        QueryState state;
        GLint extent = 0;
        glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &extent);
        const int w = framebuffer_width(), h = framebuffer_height();
        if (w <= 0 || h <= 0 || w > extent || h > extent)
            throw std::invalid_argument("unsupported capture dimensions");
        // The dimensions are not negotiable but the quality is: a count above
        // GL_MAX_SAMPLES renders at the largest supported one, as on Vulkan.
        count = unsigned(supported_sample_count(int(count)));
        state.prepare(w, h);
        auto *self = const_cast<Viewer *>(this);
        if (!initialized_) {
            self->init();
            self->initialized_ = true;
        }
        Camera snapshot(*camera());
        snapshot.frame()->set_pivot_point(camera()->pivot_point());
        snapshot.projection_matrix();
        snapshot.model_view_matrix();
        snapshot.set_projection_matrix(camera()->projection_matrix());
        snapshot.set_model_view_matrix(camera()->model_view_matrix());
        for (unsigned i = 0; i < 16; ++i)
            if (!std::isfinite(snapshot.model_view_projection_matrix()[i]))
                throw std::invalid_argument("nonfinite capture camera");
        graphics::FrameImage frame;
        frame.width = w;
        frame.height = h;
        std::vector<graphics::DrawCommand> scene_commands;
        const bool device_scene = device_query_commands(snapshot, w, h, false, scene_commands);
        // capture() keeps the stable model/standalone IDs used by pick(). Querying
        // sources would renumber them, so build a parallel source-mapped command
        // list to recover drawable colors, including for MultiViewer hosts.
        std::vector<DrawSource> scene_sources;
        std::vector<graphics::DrawCommand> source_commands;
        if (render_settings().transparency.enabled) {
            if (const auto *multi_viewer = dynamic_cast<const MultiViewer *>(this))
                source_commands = multi_viewer->draw_commands(snapshot, w, h, &scene_sources);
            else
                source_commands = draw_commands(snapshot, w, h, &scene_sources);
        }
        // Presentation transparency is a RenderSettings effect, so it must not
        // alter the raw in-memory capture.
        strip_settings_transparency(scene_commands, scene_sources, source_commands);
        auto color_commands = scene_commands;
        if (device_scene) {
            std::vector<graphics::DrawCommand> all_commands;
            if (const auto *multi_viewer = dynamic_cast<const MultiViewer *>(this))
                all_commands = multi_viewer->draw_commands(snapshot, w, h, nullptr, false);
            else
                all_commands = draw_commands(snapshot, w, h);
            for (const auto &command : all_commands)
                if (command.overlay)
                    color_commands.push_back(command);
        }
        {
            FramebufferObject color(w, h, int(count));
            if (!color.add_color_buffer() || !color.add_depth_buffer(GL_DEPTH_COMPONENT32F) || !color.bind())
                throw std::runtime_error("cannot create capture color target");
            state.prepare(w, h);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            const auto bg = background_color();
            glClearColor(bg.x, bg.y, bg.z, bg.w);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            if (device_scene)
                query_device().present(uint32_t(w), uint32_t(h), color_commands, bg, count,
                                       graphics::TransparencyMethod::sorted, 4);
            else
                draw_scene(snapshot, nullptr, true);
            glViewport(0, 0, w, h);
            draw_overlays(false);
            if (snapshot_hud_)
                self->Viewer::post_draw();
            else
                draw_overlays(true);
            glDisable(GL_SCISSOR_TEST);
            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            if (!color.read_color(0, frame.rgba, GL_RGBA, true))
                throw std::runtime_error("cannot read capture color");
        }
        if (readback == graphics::FrameReadback::color_depth_ids) {
            if (device_scene) {
                auto metadata = query_device().render(
                    uint32_t(w), uint32_t(h), scene_commands, background_color(), 1,
                    graphics::TransparencyMethod::sorted, 4, readback);
                frame.depth = std::move(metadata.depth);
                frame.ids = std::move(metadata.ids);
                for (auto &id : frame.ids)
                    id.primitive = UINT32_MAX;
            } else {
                FramebufferObject depth(w, h, 0);
                if (!depth.add_color_buffer() || !depth.add_depth_buffer(GL_DEPTH_COMPONENT32F) || !depth.bind())
                    throw std::runtime_error("cannot create capture depth target");
                frame.depth.assign(size_t(w) * h, 1);
                frame.ids.assign(size_t(w) * h, {UINT32_MAX, UINT32_MAX});
                std::vector<float> candidate(frame.depth.size());
                const auto append = [&](const std::shared_ptr<Model> &model, const std::shared_ptr<Drawable> &drawable,
                                        uint32_t id) {
                    if (!drawable->is_visible() || dynamic_cast<LinesDrawable2D *>(drawable.get()))
                        return;
                    state.prepare(w, h);
                    glClear(GL_DEPTH_BUFFER_BIT);
                    DrawSource source{model, drawable};
                    draw_scene(snapshot, &source);
                    glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, candidate.data());
                    for (int y = 0; y < h; ++y)
                        for (int x = 0; x < w; ++x) {
                            const auto p = size_t(y) * w + x;
                            const auto d = candidate[size_t(h - 1 - y) * w + x];
                            if (d >= 0 && d < frame.depth[p]) {
                                frame.depth[p] = d;
                                frame.ids[p] = {id, UINT32_MAX};
                            }
                        }
                };
                if (models_.size() + drawables_.size() >= UINT32_MAX)
                    throw std::length_error("too many capture objects");
                for (size_t i = 0; i < models_.size(); ++i) {
                    const auto &model = models_[i];
                    if (!model->renderer())
                        continue;
                    for (const auto &d : model->renderer()->triangles_drawables())
                        append(model, d, uint32_t(i));
                    for (const auto &d : model->renderer()->lines_drawables())
                        append(model, d, uint32_t(i));
                    for (const auto &d : model->renderer()->points_drawables())
                        append(model, d, uint32_t(i));
                }
                for (size_t i = 0; i < drawables_.size(); ++i)
                    append(nullptr, drawables_[i], uint32_t(models_.size() + i));
            }
        }
        return frame;
    }
    void Viewer::prepare_pick_view(int, int, int width, int height) const {
        glDisable(GL_SCISSOR_TEST);
        glViewport(0, 0, width, height);
    }
    Viewer::PickResult Viewer::pick(int x, int y) const {
        PickResult result;
        int px, py;
        if (!window_pixel(x, y, px, py))
            return result;
        int fw, fh;
        framebuffer_size(fw, fh);
        QueryContext context(window_);
        Camera snapshot(*camera());
        snapshot.projection_matrix();
        snapshot.model_view_matrix();
        snapshot.set_projection_matrix(camera()->projection_matrix());
        snapshot.set_model_view_matrix(camera()->model_view_matrix());
        std::vector<graphics::DrawCommand> commands;
        std::vector<DrawSource> sources;
        if (device_query_commands(snapshot, fw, fh, false, commands, &sources)) {
            const auto pixel = query_device().sample_pixel(
                uint32_t(fw), uint32_t(fh), commands, uint32_t(px), uint32_t(py));
            if (pixel.id.object == UINT32_MAX || pixel.id.object >= sources.size())
                return result;
            result.model = sources[pixel.id.object].model;
            result.drawable = sources[pixel.id.object].drawable;
            result.primitive = pixel.id.primitive;
            result.depth = pixel.depth;
        } else {
            // Unsupported drawable and effect states still use the exact native
            // drawable shaders. Draw each candidate separately and retain the nearest.
            QueryState state;
            state.prepare(fw, fh);
            FramebufferObject target(fw, fh, 0);
            if (!target.add_color_buffer() || !target.add_depth_buffer(GL_DEPTH_COMPONENT32F) ||
                !target.is_valid() || !target.bind())
                throw std::runtime_error("cannot create picking framebuffer");
            const auto sample = [&](const std::shared_ptr<Drawable> &drawable,
                                    const std::shared_ptr<Model> &model) {
                if (!drawable->is_visible() || dynamic_cast<LinesDrawable2D *>(drawable.get()))
                    return;
                state.prepare(fw, fh);
                glClear(GL_DEPTH_BUFFER_BIT);
                DrawSource source{model, drawable};
                draw_scene(snapshot, &source);
                float depth = 1;
                glReadPixels(px, fh - 1 - py, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
                if (depth >= 0 && depth < result.depth) {
                    result.model = model;
                    result.drawable = drawable;
                    result.depth = depth;
                }
            };
            for (const auto &model : models_) {
                const auto *renderer = model->renderer();
                if (!renderer)
                    continue;
                for (const auto &d : renderer->triangles_drawables())
                    sample(d, model);
                for (const auto &d : renderer->lines_drawables())
                    sample(d, model);
                for (const auto &d : renderer->points_drawables())
                    sample(d, model);
            }
            for (const auto &d : drawables_)
                sample(d, nullptr);
            if (!result)
                return result;
            if (result.drawable->type() == Drawable::DT_TRIANGLES) {
                const std::string name = "selection/selection_single_primitive";
                auto *program = ShaderManager::get_program(name);
                if (!program)
                    program = ShaderManager::create_program_from_files(name, {{ShaderProgram::POSITION, "vtx_position"}});
                if (!program)
                    throw std::runtime_error("picking requires the selection shader '" + name + "'");
                state.prepare(fw, fh);
                prepare_pick_view(px, py, fw, fh);
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                glClearColor(1, 1, 1, 1);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                program->bind();
                const mat4 MANIP = result.drawable->manipulated_matrix();
                program->set_uniform("MANIP", MANIP)
                    ->set_uniform("MVP_MANIP", snapshot.model_view_projection_matrix() * MANIP);
                ClippingPlane::instance()->set_program(program);
                ClippingPlane::instance()->set_discard_primitives(program, result.drawable->plane_clip_discard_primitive());
                result.drawable->gl_draw();
                program->release();
                unsigned char rgba[4] = {255, 255, 255, 255};
                glReadPixels(px, fh - 1 - py, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
                const int id = color::encode(rgba[0], rgba[1], rgba[2], rgba[3]);
                if (id >= 0)
                    result.primitive = uint32_t(id);
            }
        }
        if (!result)
            return result;
        result.position = pixel_to_world(px, py, result.depth);
        auto *mesh = dynamic_cast<SurfaceMesh *>(result.model.get());
        if (mesh && result.primitive != UINT32_MAX &&
            result.model->renderer()->get_triangles_drawable("faces", false) == result.drawable.get()) {
            const auto ranges = mesh->get_face_property<std::pair<int, int>>("f:triangle_range");
            if (ranges)
                for (auto face : mesh->faces()) {
                    const auto range = ranges[face];
                    if (range.first >= 0 && result.primitive >= uint32_t(range.first) &&
                        result.primitive <= uint32_t(range.second)) {
                        result.face_index = face.idx();
                        break;
                    }
                }
        }
        return result;
    }
    bool Viewer::read_scene_depth(int x, int y, const Camera &snapshot, float &depth) const {
        // x/y are framebuffer pixels; the camera carries logical screen dimensions.
        const int fw = framebuffer_width(), fh = framebuffer_height();
        QueryContext context(window_);
        std::vector<graphics::DrawCommand> commands;
        if (device_query_commands(snapshot, fw, fh, false, commands)) {
            const auto pixel = query_device().sample_pixel(
                uint32_t(fw), uint32_t(fh), commands, uint32_t(x), uint32_t(y));
            depth = pixel.depth;
            return pixel.id.object != UINT32_MAX;
        }
        QueryState state;
        state.prepare(fw, fh);
        FramebufferObject target(fw, fh, 0);
        if (!target.add_color_buffer() || !target.add_depth_buffer(GL_DEPTH_COMPONENT32F) || !target.is_valid())
            throw std::runtime_error("cannot create single-sample scene depth framebuffer");
        if (!target.bind())
            throw std::runtime_error("cannot bind scene depth framebuffer");
        glClear(GL_DEPTH_BUFFER_BIT);
        draw_scene(snapshot);
        glReadPixels(x, fh - 1 - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
        return depth < 1.0f;
    }
    void Viewer::draw_scene(const Camera &snapshot, const DrawSource *source, bool guides) const {
        if (source) {
            if (source->model && (!source->model->renderer() || !source->model->renderer()->is_visible()))
                return;
            query_draw(source->drawable.get(), snapshot);
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
        if (guides) {
            ClippingPlane::instance()->draw(&snapshot);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            for (const auto &d : drawables_)
                if (d->is_visible())
                    if (auto *flat = dynamic_cast<LinesDrawable2D *>(d.get()))
                        flat->draw(&snapshot);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
        }
    }
} // namespace easy3d
#endif // !EASY3D_HAS_VULKAN
