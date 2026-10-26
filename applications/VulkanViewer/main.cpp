// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/vulkan_viewer.h>
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
        easy3d::VulkanViewerOptions options;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--validation") { options.device.validation = true; options.device.synchronization_validation = true; }
            else if (arg == "--frames") {
                if (++i == argc) throw std::invalid_argument("--frames requires a positive integer");
                std::string text = argv[i]; size_t used = 0;
                if (text.empty() || text[0] == '-') throw std::invalid_argument("--frames requires a positive integer");
                const auto value = std::stoul(text, &used);
                if (used != text.size() || !value || value > std::numeric_limits<unsigned>::max())
                    throw std::invalid_argument("--frames requires a positive integer");
                frames = unsigned(value);
            } else if (arg == "--help") {
                std::cout << "VulkanViewer [model] [--validation] [--frames N]\n"
                          << "Left drag: rotate; right/middle drag: pan; wheel: zoom; F: fit; P: projection; Escape: close.\n";
                return 0;
            } else model = arg;
        }
        easy3d::VulkanViewer viewer("Easy3D Vulkan Viewer", options);
        if (model.empty()) {
            // The installed demo works without any source-tree data files.
            auto bunny = std::make_shared<easy3d::SurfaceMesh>();
            std::vector<easy3d::SurfaceMesh::Vertex> vertices;
            for (const auto& p : easy3d::resource::bunny_vertices) vertices.push_back(bunny->add_vertex(p));
            const auto& indices = easy3d::resource::bunny_indices;
            for (size_t i = 0; i + 2 < indices.size(); i += 3)
                bunny->add_triangle(vertices[indices[i]], vertices[indices[i+1]], vertices[indices[i+2]]);
            if (!viewer.add_model(bunny)) return 1;
        } else if (!viewer.add_model(model)) return 1;
        std::cout << "Native Vulkan device: " << viewer.device().name() << '\n';
        return viewer.run(frames);
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
