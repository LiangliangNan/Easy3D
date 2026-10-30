// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#include <iostream>

int main(int argc, char** argv) {
    try {
        easy3d::initialize();
        easy3d::VulkanOffScreen renderer(800, 600);
        auto* mesh = renderer.add_model(argc > 1 ? argv[1] : easy3d::resource::directory() + "/data/bunny.ply");
        if (!mesh) return 1;
        // Same model / renderer / drawable / camera APIs used by the OpenGL viewer.
        auto* faces = mesh->renderer()->get_triangles_drawable("faces", false);
        if (faces) faces->set_uniform_coloring(easy3d::vec4(0.7f, 0.85f, 1, 1));
        std::cout << "Native Vulkan device: " << renderer.device().name() << '\n';
        return renderer.render(argc > 2 ? argv[2] : "vulkan.png") ? 0 : 1;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
