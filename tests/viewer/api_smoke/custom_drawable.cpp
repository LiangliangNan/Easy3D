// SPDX-License-Identifier: GPL-3.0-or-later
#include "api_smoke.h"

#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_renderer.h>
#include <easy3d/renderer/drawable_triangles.h>

#include <memory>

void smoke_custom_drawable() {
    auto points = std::make_shared<easy3d::PointsDrawable>("points");
    points->update_vertex_buffer(std::vector<easy3d::vec3>{
        easy3d::vec3(0, 0, 0), easy3d::vec3(1, 0, 0), easy3d::vec3(0, 1, 0)});
    points->update_color_buffer(std::vector<easy3d::vec3>(3, easy3d::vec3(1, 0, 0)));
    points->update_normal_buffer(std::vector<easy3d::vec3>(3, easy3d::vec3(0, 0, 1)));
    points->update_texcoord_buffer(std::vector<easy3d::vec2>(3, easy3d::vec2(0, 0)));
    points->update_element_buffer(std::vector<unsigned int>{0, 1, 2});
    points->set_impostor_type(easy3d::PointsDrawable::SPHERE);
    points->set_point_size(3.0f);
    points->set_uniform_coloring(easy3d::vec4(1, 1, 1, 1));
    points->update_highlight_mask(std::vector<float>{1.0f, 0.0f, 0.0f});
    points->highlight_mask();
    points->buffer_data();
    points->num_vertices();
    points->num_indices();
    points->update();

    auto lines = std::make_shared<easy3d::LinesDrawable>("lines");
    lines->set_impostor_type(easy3d::LinesDrawable::CYLINDER);
    lines->set_line_width(2.0f);

    auto triangles = std::make_shared<easy3d::TrianglesDrawable>("triangles");
    triangles->set_smooth_shading(true);
    triangles->set_opacity(0.5f);

    easy3d::Camera camera;
    easy3d::graphics::DrawableRenderer renderer;
    renderer.command(points, 1, camera, {});
    renderer.command(lines, 2, camera, {});
    renderer.command(triangles, 3, camera, {});
    renderer.prune();
    renderer.clear();
}
