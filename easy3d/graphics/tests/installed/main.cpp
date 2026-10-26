// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/offscreen.h>
#include <easy3d/viewer/vulkan_offscreen.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/file_system.h>
#include <stdexcept>
#include <iostream>
int main() {
    easy3d::initialize(false, false, false);
    if (easy3d::resource::directory() != easy3d::file_system::convert_to_native_style(Easy3D_RESOURCE_DIR))
        throw std::runtime_error("installed resources were rejected or resolved to the source tree");
#ifdef EASY3D_NO_OPENGL
    easy3d::OffScreen renderer(101, 77);
#else
    easy3d::VulkanOffScreen renderer(101, 77);
#endif
    auto mesh = std::make_shared<easy3d::SurfaceMesh>();
    auto a = mesh->add_vertex(easy3d::vec3(-1, 0, -1));
    auto b = mesh->add_vertex(easy3d::vec3(1, 0, -1));
    auto c = mesh->add_vertex(easy3d::vec3(0, 0, 1));
    mesh->add_triangle(a, b, c);
    renderer.add_model(mesh);
    mesh->renderer()->get_triangles_drawable("faces")->set_uniform_coloring(easy3d::vec4(1, 0.4f, 0, 1));
    auto frame = renderer.capture();
    unsigned hits = 0;
    for (auto id : frame.ids) hits += id.object != UINT32_MAX;
    std::cout << "Installed OffScreen rendered " << hits << " pixels on " << renderer.device().name() << '\n';
    return hits > 100 && renderer.render("installed.png") ? 0 : 1;
}
