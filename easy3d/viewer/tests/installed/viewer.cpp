// SPDX-License-Identifier: GPL-3.0-or-later
// Public interactive API only: no Vulkan SDK, GLFW or OpenGL headers/calls.
#include <easy3d/viewer/viewer_base.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
int main() {
    using namespace easy3d;
    initialize(false, false, false);
    ViewerOptions options; options.width = 181; options.height = 151; options.visible = false; options.samples = 1;
    const auto require = [](bool ok, const char* message) { if (!ok) throw std::runtime_error(message); };
    try {
        auto viewer = ViewerBase::create("Installed runtime viewer", options);
        auto mesh = std::make_shared<SurfaceMesh>();
        const auto a = mesh->add_vertex(vec3(0, -1, -1)), b = mesh->add_vertex(vec3(0, 1, -1));
        const auto c = mesh->add_vertex(vec3(0, 0, 1)); mesh->add_triangle(a, b, c); viewer->add_model(mesh);
        auto faces = mesh->renderer()->get_triangles_drawable("faces");
        faces->set_uniform_coloring(vec4(.8f, .2f, .1f, 1)); faces->set_lighting(false);
        viewer->set_usage(""); viewer->fit_screen();
        unsigned ticks = 0, before = 0, after = 0;
        ViewerCallbacks callbacks;
        callbacks.pre_draw = [&](ViewerBase* host) { require(host == viewer.get(), "callback received another viewer"); ++before; };
        callbacks.post_draw = [&](ViewerBase*) { ++after; };
        viewer->set_callbacks(std::move(callbacks));
        viewer->set_animation_callback([&](ViewerBase* host) { ++ticks; host->set_animation(false); return true; });
        viewer->set_animation(true);
        require(viewer->run_frames(2) == 0 && ticks == 1 && viewer->width() == 181 && viewer->height() == 151 && viewer->samples() == 1 && before >= 2 && after == 2,
                "installed viewer loop/dimensions/animation/callbacks failed");
        const auto frame_callbacks = before + after;
        require(viewer->snapshot("installed-viewer.png", 1, 4, 1), "installed viewer snapshot failed");
        require(before + after == frame_callbacks, "installed viewer snapshot invoked frame callbacks");
        int w, h, channels; std::vector<unsigned char> rgba;
        require(ImageIO::load("installed-viewer.png", rgba, w, h, channels, 4, false) &&
                w == viewer->framebuffer_width() && h == viewer->framebuffer_height(), "installed viewer output dimensions differ");
        const auto center = (size_t(h / 2) * w + w / 2) * 4;
        require(std::abs(int(rgba[center]) - 204) <= 2 && std::abs(int(rgba[center+1]) - 51) <= 2,
                "installed viewer material/default camera did not render the model");
        std::cout << "Runtime Viewer rendered using " << (viewer->backend() == Backend::Vulkan ? "Vulkan" : "OpenGL") << '\n';
        return viewer->validation_errors() ? 1 : 0;
    } catch (const BackendUnavailable& e) {
#if !defined(EASY3D_HAS_VULKAN_VIEWER) && defined(EASY3D_NO_OPENGL)
        const std::string reason = e.what();
        if (reason.find("Vulkan:") != std::string::npos && reason.find("OpenGL:") != std::string::npos) {
            std::cout << "Headless package reports both unavailable interactive backends\n"; return 0;
        }
#endif
        std::cerr << e.what() << '\n'; return 1;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
