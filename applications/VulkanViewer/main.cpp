// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#include <easy3d/core/surface_mesh.h>
#include <iostream>
#include <limits>
#include <stdexcept>
int main(int argc, char** argv) {
    try {
        easy3d::initialize();
        std::string model;
        unsigned frames = 0;
        easy3d::ViewerOptions options;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--validation") { options.validation = true; options.synchronization_validation = true; }
            else if (arg == "--backend") {
                if (++i == argc) throw std::invalid_argument("--backend requires auto, vulkan or opengl");
                const std::string backend = argv[i];
                if (backend == "auto") options.backend = easy3d::Backend::Automatic;
                else if (backend == "vulkan") options.backend = easy3d::Backend::Vulkan;
                else if (backend == "opengl") options.backend = easy3d::Backend::OpenGL;
                else throw std::invalid_argument("--backend requires auto, vulkan or opengl");
            } else if (arg == "--frames" || arg == "--samples") {
                const auto message = arg + " requires a positive integer";
                if (++i == argc) throw std::invalid_argument(message);
                std::string text = argv[i]; size_t used = 0;
                if (text.empty() || text[0] == '-') throw std::invalid_argument(message);
                const auto value = std::stoul(text, &used);
                if (used != text.size() || !value || value > std::numeric_limits<unsigned>::max())
                    throw std::invalid_argument(message);
                if (arg == "--frames") frames = unsigned(value); else options.samples = unsigned(value);
            } else if (arg == "--help") {
                std::cout << "VulkanViewer [model] [--backend auto|vulkan|opengl] [--validation] [--frames N] [--samples N]\n"
                          << "Left drag: rotate; right drag: pan; middle drag/wheel: zoom; F1: controls; Escape: close.\n";
                return 0;
            } else if (arg.compare(0, 2, "--") == 0) throw std::invalid_argument("unknown option: " + arg);
            else model = arg;
        }
        auto viewer = easy3d::Viewer::create("Easy3D Viewer", options);
        if (model.empty()) {
            // The installed demo works without any source-tree data files.
            auto bunny = std::make_shared<easy3d::SurfaceMesh>();
            std::vector<easy3d::SurfaceMesh::Vertex> vertices;
            for (const auto& p : easy3d::resource::bunny_vertices) vertices.push_back(bunny->add_vertex(p));
            const auto& indices = easy3d::resource::bunny_indices;
            for (size_t i = 0; i + 2 < indices.size(); i += 3)
                bunny->add_triangle(vertices[indices[i]], vertices[indices[i+1]], vertices[indices[i+2]]);
            if (!viewer->add_model(bunny)) return 1;
        } else if (!viewer->add_model(model)) return 1;
        viewer->fit_screen();
        std::cout << "Renderer backend: " << (viewer->backend() == easy3d::Backend::Vulkan ? "Vulkan" : "OpenGL")
                  << ", samples: " << viewer->samples() << '\n';
        if (!viewer->fallback_reason().empty()) std::cout << "Vulkan unavailable: " << viewer->fallback_reason() << '\n';
        const int result = viewer->run_frames(frames);
        return result || viewer->validation_errors() ? 1 : 0;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
