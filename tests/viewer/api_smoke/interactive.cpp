// SPDX-License-Identifier: GPL-3.0-or-later
#include "api_smoke.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/viewer/viewer.h>

#include <memory>

void smoke_interactive_viewer() {
    easy3d::WindowOptions window;
    window.width = 320;
    window.height = 240;
    window.visible = false;
    easy3d::graphics::DeviceOptions options;
    easy3d::Viewer viewer("API smoke", window, options);
    viewer.resize(window.width, window.height);
    viewer.show();
    viewer.hide();
    viewer.update();

    auto model = std::make_shared<easy3d::SurfaceMesh>();
    model->add_vertex(easy3d::vec3(0, 0, 0));
    viewer.add_model(model, true, false);
    viewer.fit_screen(model.get());

    auto points = std::make_shared<easy3d::PointsDrawable>("points");
    points->update_vertex_buffer(std::vector<easy3d::vec3>{easy3d::vec3(0, 0, 0)});
    viewer.add_drawable(points);
    viewer.set_overlays({});
    viewer.pick(0, 0);
    viewer.capture(1);
    viewer.capture(1, easy3d::graphics::FrameReadback::color);
    viewer.snapshot("api-smoke.png");
    viewer.draw_frame();
    viewer.run_frames(1);
    viewer.exit();

    viewer.delete_drawable(points.get());
    viewer.delete_model(model.get());
}
