// SPDX-License-Identifier: GPL-3.0-or-later
#include "api_smoke.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/viewer/offscreen.h>

#include <memory>

void smoke_offscreen_viewer() {
    easy3d::OffScreen canvas(101, 77);
    auto model = std::make_shared<easy3d::SurfaceMesh>();
    const auto a = model->add_vertex(easy3d::vec3(0, -1, -1));
    const auto b = model->add_vertex(easy3d::vec3(0, 1, -1));
    const auto c = model->add_vertex(easy3d::vec3(0, 0, 1));
    model->add_triangle(a, b, c);
    canvas.add_model(model, true, true);

    auto* triangles = model->renderer()->get_triangles_drawable("faces");
    triangles->set_uniform_coloring(easy3d::vec4(1, 0.4f, 0, 1));
    triangles->set_opacity(0.75f);
    triangles->set_smooth_shading(true);
    canvas.set_overlays({});
    canvas.pick(10, 10);
    canvas.capture();
    canvas.capture(1);
    canvas.capture(1, easy3d::graphics::FrameReadback::color_depth_ids);
    canvas.render("api-smoke.png");
    canvas.resize(128, 96);
    canvas.clear_scene();
}
