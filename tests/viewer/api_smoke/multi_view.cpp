// SPDX-License-Identifier: GPL-3.0-or-later
#include "api_smoke.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/viewer/multi_viewer.h>

#include <memory>

void smoke_multi_viewer() {
    easy3d::WindowOptions window;
    easy3d::MultiViewer viewer(2, 2, "API smoke", window);
    auto model = std::make_shared<easy3d::SurfaceMesh>();
    model->add_vertex(easy3d::vec3(0, 0, 0));
    viewer.add_model(model, true, false);
    viewer.assign(0, 0, model.get());
    viewer.assign(1, 1, model->renderer()->get_points_drawable("vertices"));
    viewer.set_layout(2, 3);
    viewer.set_division_visible(true);
    viewer.division_visible();
    viewer.rows();
    viewer.columns();
    viewer.pick(10, 10);
    viewer.capture(1);
    viewer.capture(1, easy3d::graphics::FrameReadback::color);
    viewer.snapshot("api-smoke.png");
}
