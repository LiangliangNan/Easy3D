// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/gui/picker_model.h>
#include <easy3d/gui/picker_point_cloud.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/renderer.h>

using namespace easy3d;
using test::require;

int main() {
    return test::run("GUI model and point-cloud picking", [] {
        Viewer viewer("Scene picker test", 201, 151, 1, false, false);
        viewer.set_usage("");
        auto cloud = std::make_shared<PointCloud>();
        const auto dead = cloud->add_vertex(vec3(0, 0, .4f));
        const auto rear = cloud->add_vertex(vec3(0, 0, -.2f));
        const auto front = cloud->add_vertex(vec3(0, 0, .2f));
        const auto side = cloud->add_vertex(vec3(.6f, 0, .2f));
        cloud->delete_vertex(dead);
        auto mesh = std::make_shared<SurfaceMesh>();
        const auto a = mesh->add_vertex(vec3(-1, -1, -.5f));
        const auto b = mesh->add_vertex(vec3(1, -1, -.5f));
        const auto c = mesh->add_vertex(vec3(0, 1, -.5f));
        mesh->add_triangle(a, b, c);
        viewer.add_model(cloud);
        viewer.add_model(mesh);
        auto* camera = viewer.camera();
        camera->setViewDirection(vec3(0, 0, -1));
        camera->setUpVector(vec3(0, 1, 0));
        camera->setType(Camera::PERSPECTIVE);
        viewer.fit_screen();
        camera->setType(Camera::ORTHOGRAPHIC);
        auto* drawable = cloud->renderer()->get_points_drawable("vertices");
        drawable->set_impostor_type(PointsDrawable::SPHERE);
        drawable->set_point_size(25);
        require(viewer.run_frames(2) == 0, "cannot initialize scene pickers");

        ModelPicker model_picker(camera);
        PointCloudPicker point_picker(camera);
        const auto center = camera->projectedCoordinatesOf(cloud->position(front));
        const int x = int(center.x), y = int(center.y);
        require(model_picker.pick({mesh, cloud}, x, y) == cloud.get(), "model pick lost closest model");
        require(model_picker.pick({mesh}, x, y) == mesh.get(), "model pick ignored candidate set");
        const auto picked = point_picker.pick_vertex(cloud.get(), x, y);
        if (picked != front)
            throw std::runtime_error("expected closest live source slot " + std::to_string(front.idx()) +
                                     ", got " + std::to_string(picked.idx()));
        drawable->set_impostor_type(PointsDrawable::PLAIN);
        require(point_picker.pick_vertex(cloud.get(), x, y) == front, "plain point pick lost depth or source ID");
        drawable->set_impostor_type(PointsDrawable::SPHERE);
        require(!model_picker.pick({mesh, cloud}, -1, y) &&
                !point_picker.pick_vertex(cloud.get(), x, -1).is_valid(), "out-of-bounds pick hit the scene");

        cloud->renderer()->set_visible(false);
        require(model_picker.pick({cloud, mesh}, x, y) == mesh.get() &&
                !point_picker.pick_vertex(cloud.get(), x, y).is_valid(), "hidden model was pickable");
        cloud->renderer()->set_visible(true);
        drawable->set_visible(false);
        require(!point_picker.pick_vertex(cloud.get(), x, y).is_valid(), "hidden vertices were pickable");
        drawable->set_visible(true);

        const Rect rect(x - 10, x + 10, y - 10, y + 10);
        point_picker.pick_vertices(cloud.get(), rect, false);
        auto selected = cloud->get_vertex_property<bool>("v:select");
        require(selected[front] && selected[rear] && !selected[side] && !selected[dead], "rectangle did not select through the cloud");
        Polygon2 polygon;
        polygon.push_back(vec2(rect.left(), rect.top()));
        polygon.push_back(vec2(rect.right(), rect.top()));
        polygon.push_back(vec2(rect.right(), rect.bottom()));
        polygon.push_back(vec2(rect.left(), rect.bottom()));
        selected[side] = true;
        point_picker.pick_vertices(cloud.get(), polygon, true);
        require(!selected[front] && !selected[rear] && selected[side], "lasso deselection changed unrelated selection bits");

        cloud->set_manipulator(std::make_shared<Manipulator>(cloud.get()));
        const vec3 translation(-.3f, .3f, 0);
        cloud->manipulator()->frame()->translate(translation);
        const auto moved = camera->projectedCoordinatesOf(cloud->position(front) + translation);
        require(point_picker.pick_vertex(cloud.get(), int(moved.x), int(moved.y)) == front,
                "point pick ignored the model transform");
        drawable->update_element_buffer(std::vector<unsigned>{unsigned(rear.idx()), unsigned(front.idx())});
        require(point_picker.pick_vertex(cloud.get(), int(moved.x), int(moved.y)) == front,
                "indexed point pick returned the draw slot instead of the source slot");
        auto* plane = ClippingPlane::instance();
        plane->fit_scene(vec3(0), 1);
        plane->set_enabled(true);
        require(point_picker.pick_vertex(cloud.get(), int(moved.x), int(moved.y)) == rear,
                "point pick ignored the clipping plane");
        plane->set_enabled(false);
        PointCloud detached;
        const auto vertex = detached.add_vertex(vec3(0));
        point_picker.pick_vertices(&detached, rect, false);
        require(detached.get_vertex_property<bool>("v:select")[vertex], "region selection required an attached renderer");
    });
}
