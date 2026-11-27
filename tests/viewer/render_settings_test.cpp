// SPDX-License-Identifier: GPL-3.0-or-later
#include "test_window.h"
#include <easy3d/viewer/viewer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/render_settings.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
using namespace easy3d;

namespace {
    void require(bool value, const char *message) {
        if (!value)
            throw std::runtime_error(message);
    }

    std::string output_path(const std::string &directory, const std::string &file) {
        return directory.empty() || directory == "." ? file : directory + "/" + file;
    }

    std::vector<unsigned char> read_snapshot(const std::string &file, int width, int height) {
        std::vector<unsigned char> data;
        int w = 0, h = 0, channels = 0;
        require(ImageIO::load(file, data, w, h, channels), "snapshot could not be read");
        require(w == width && h == height && channels == 4, "snapshot size or channel count changed");
        return data;
    }

    std::vector<TrianglesDrawable *> triangle_drawables(const Viewer &viewer) {
        std::vector<TrianglesDrawable *> result;
        for (const auto &model : viewer.models()) {
            if (!model->renderer())
                continue;
            for (const auto &d : model->renderer()->triangles_drawables())
                result.push_back(dynamic_cast<TrianglesDrawable *>(d.get()));
        }
        for (const auto &d : viewer.drawables()) {
            if (auto *triangles = dynamic_cast<TrianglesDrawable *>(d.get()))
                result.push_back(triangles);
        }
        return result;
    }

    void neutralize_appearance(const std::vector<TrianglesDrawable *> &drawables) {
        for (auto *d : drawables) {
            if (!d)
                continue;
            d->set_coloring_method(State::UNIFORM_COLOR);
            d->set_color(vec4(0.72f, 0.52f, 0.32f, 1.0f));
            d->set_opacity(0.4f);
        }
    }

    std::shared_ptr<SurfaceMesh> quad() {
        auto mesh = std::make_shared<SurfaceMesh>();
        const auto a = mesh->add_vertex(vec3(-1, -1, 0));
        const auto b = mesh->add_vertex(vec3(1, -1, 0));
        const auto c = mesh->add_vertex(vec3(1, 1, 0));
        const auto d = mesh->add_vertex(vec3(-1, 1, 0));
        mesh->add_quad(a, b, c, d);
        return mesh;
    }

    void prepare_quad(Viewer &viewer, const std::shared_ptr<SurfaceMesh> &mesh) {
        viewer.add_model(mesh);
        for (const auto &d : mesh->renderer()->lines_drawables())
            d->set_visible(false);
        for (const auto &d : mesh->renderer()->points_drawables())
            d->set_visible(false);
        auto *triangles = mesh->renderer()->get_triangles_drawable("faces");
        triangles->set_lighting(false);
        triangles->set_coloring_method(State::UNIFORM_COLOR);
        triangles->set_color(vec4(0.9f, 0.3f, 0.2f, 1.0f));
        viewer.camera()->set_type(Camera::ORTHOGRAPHIC);
        viewer.camera()->set_view_direction(vec3(0, 0, -1));
        viewer.camera()->set_up_vector(vec3(0, 1, 0));
        viewer.fit_screen();
    }

    void clear_settings(Viewer &viewer) {
        viewer.set_render_settings(RenderSettings{});
    }
} // namespace

int main(int argc, char **argv) {
    try {
        bool validation = false;
        std::string output_dir = ".";
        for (int i = 1; i < argc; ++i) {
            const std::string argument = argv[i];
            if (argument == "--validation")
                validation = true;
            else if (argument == "--output-dir" && i + 1 < argc)
                output_dir = argv[++i];
            else
                throw std::invalid_argument("unexpected render-settings test argument: " + argument);
        }
        initialize(false, false, false);
#ifdef EASY3D_HAS_VULKAN
        VulkanOptions options;
        options.validation = options.sync_validation = validation;
#else
        OpenGLOptions options;
        (void)validation;
#endif
        {
            const int width = 320, height = 240;
            Viewer viewer("Render settings effects", window_options(width, height, 1, false, false, true), options);
            require(viewer.add_model(resource::directory() + "/data/room.obj"), "test scene missing");
            viewer.camera()->set_up_vector(vec3(0, 1, 0));
            viewer.camera()->set_view_direction(vec3(0, 0, 1));
            viewer.fit_screen();
            neutralize_appearance(triangle_drawables(viewer));
            const auto fw = viewer.framebuffer_width(), fh = viewer.framebuffer_height();
            require(fw > 0 && fh > 0, "test window has no framebuffer");

            const auto snapshot = [&](const RenderSettings &settings, const std::string &name) {
                viewer.set_render_settings(settings);
                require(viewer.snapshot(output_path(output_dir, name), 1, 1, 1), "snapshot failed");
                return read_snapshot(output_path(output_dir, name), fw, fh);
            };

            const RenderSettings defaults;
            const auto raw_capture = viewer.capture(1);
            const auto plain = snapshot(defaults, "render-settings-plain.png");
            require(snapshot(defaults, "render-settings-defaults.png") == plain,
                    "the default settings changed the rendered scene");

            RenderSettings settings = defaults;
            settings.shadow.enabled = true;
            settings.shadow.virtual_background = true;
            settings.shadow.virtual_background_color = viewer.background_color();
            const auto hard_shadow = snapshot(settings, "render-settings-shadow-hard.png");
            require(hard_shadow != plain, "hard shadows are missing from the frame");
            require(viewer.capture(1).rgba == raw_capture.rgba,
                    "in-memory capture applied presentation-only shadow effects");

            settings.shadow.softness = 0.6f;
            const auto soft_shadow = snapshot(settings, "render-settings-shadow-soft.png");
            require(soft_shadow != plain, "soft shadows are missing from the frame");
            require(soft_shadow != hard_shadow, "softness did not change the shadow rendering");

            settings = defaults;
            settings.shadow.enabled = true;
            settings.shadow.virtual_background = false;
            const auto scene_shadow = snapshot(settings, "render-settings-shadow-scene.png");
            require(scene_shadow != plain, "scene-only shadows are missing from the frame");

            settings = defaults;
            settings.ambient_occlusion.enabled = true;
            settings.ambient_occlusion.intensity = 0.8f;
            require(snapshot(settings, "render-settings-ao.png") != plain,
                    "ambient occlusion is missing from the frame");

            settings = defaults;
            settings.eye_dome_lighting = true;
            require(snapshot(settings, "render-settings-edl.png") != plain,
                    "eye-dome lighting is missing from the frame");

            for (auto method : {graphics::TransparencyMethod::sorted, graphics::TransparencyMethod::average,
                                graphics::TransparencyMethod::peeling, graphics::TransparencyMethod::linked_lists}) {
                settings = defaults;
                settings.transparency.enabled = true;
                settings.transparency.method = method;
                settings.transparency.layers = 6;
                const std::string name = "render-settings-transparency-" + std::to_string(int(method)) + ".png";
                require(snapshot(settings, name) != plain,
                        "the settings transparency pass did not change the opaque frame");
            }
        }
        {
            Viewer viewer("Render settings clipping", window_options(181, 141, 1, false, false, true), options);
            auto mesh = quad();
            prepare_quad(viewer, mesh);
            const auto plain = viewer.capture(1);
            const size_t left = size_t(plain.width / 4) * plain.width + plain.width / 4;
            const size_t right = size_t(plain.height / 2) * plain.width + 3 * plain.width / 4;
            require(plain.ids[left].object != UINT32_MAX && plain.ids[right].object != UINT32_MAX,
                    "the unclipped quad does not cover the test pixels");

            RenderSettings settings;
            settings.clipping_planes = {vec4(2, 0, 0, 0)};
            viewer.set_render_settings(settings);
            require(viewer.clipping_planes().size() == 1 && viewer.clipping_planes()[0].x == 1,
                    "settings clipping planes were not normalized or exposed");
            const auto clipped = viewer.capture(1);
            require(clipped.ids[left].object == UINT32_MAX,
                    "rendered capture retained the clipped half of the quad");
            require(clipped.ids[right].object == 0,
                    "rendered capture lost the surviving half of the quad");
            auto points = std::make_shared<PointsDrawable>("clipped points");
            points->update_vertex_buffer({vec3(-0.5f, 0, 0.2f), vec3(0.5f, 0, 0.2f)});
            points->set_point_size(21);
            points->set_impostor_type(PointsDrawable::PLAIN);
            points->set_lighting(false);
            points->set_uniform_coloring(vec4(0, 1, 0, 1));
            viewer.add_drawable(points);
            const auto point_plane = viewer.capture(1);
            const auto point_uv = viewer.camera()->projected_coordinates_of(vec3(0.5f, 0, 0.2f));
            const auto clipped_uv = viewer.camera()->projected_coordinates_of(vec3(-0.5f, 0, 0.2f));
            const auto pixel = [&](const vec3 &uv) {
                return size_t(uv.y * point_plane.height / viewer.height()) * point_plane.width +
                       size_t(uv.x * point_plane.width / viewer.width());
            };
            require(point_plane.ids[pixel(clipped_uv)].object == UINT32_MAX &&
                    point_plane.ids[pixel(point_uv)].object == 1,
                    "plain point capture ignored the settings clipping plane");
            auto lines = std::make_shared<LinesDrawable>("clipped line");
            lines->update_vertex_buffer({vec3(-0.8f, 0.6f, 0.3f), vec3(0.8f, 0.6f, 0.3f)});
            lines->set_line_width(9);
            lines->set_lighting(false);
            lines->set_uniform_coloring(vec4(0, 1, 0, 1));
            viewer.add_drawable(lines);
            const auto line_plane = viewer.capture(1);
            const auto line_kept_uv = viewer.camera()->projected_coordinates_of(vec3(0.5f, 0.6f, 0.3f));
            const auto line_clipped_uv = viewer.camera()->projected_coordinates_of(vec3(-0.5f, 0.6f, 0.3f));
            const auto line_pixel = [&](const vec3 &uv) {
                return size_t(uv.y * line_plane.height / viewer.height()) * line_plane.width +
                       size_t(uv.x * line_plane.width / viewer.width());
            };
            const auto kept_line = line_pixel(line_kept_uv), clipped_line = line_pixel(line_clipped_uv);
            require(!std::equal(line_plane.rgba.begin() + 4 * kept_line,
                                line_plane.rgba.begin() + 4 * kept_line + 4,
                                point_plane.rgba.begin() + 4 * kept_line) &&
                    std::equal(line_plane.rgba.begin() + 4 * clipped_line,
                               line_plane.rgba.begin() + 4 * clipped_line + 4,
                               point_plane.rgba.begin() + 4 * clipped_line),
                    "plain line capture ignored the settings clipping plane");
            const auto hit = viewer.pick(3 * viewer.width() / 4, viewer.height() / 2);
            const auto miss = viewer.pick(viewer.width() / 4, viewer.height() / 2);
            require(hit.model.get() == mesh.get() && miss.model == nullptr,
                    "point picking disagrees with the settings clipping planes");
            require(viewer.snapshot(output_path(output_dir, "render-settings-clipping.png"), 1, 1, 1),
                    "clipped snapshot failed");
            const auto clipped_image = read_snapshot(output_path(output_dir, "render-settings-clipping.png"),
                                                     plain.width, plain.height);
            require(clipped_image != plain.rgba, "settings clipping did not change the rendered image");
        }
        std::cout << "PASS: render settings\n";
        return EXIT_SUCCESS;
    } catch (const std::exception &e) {
        std::cerr << "FAIL: render settings: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}