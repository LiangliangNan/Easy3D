/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "viewer.h"
#include <easy3d/core/model.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/primitives.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/renderer.h>


using namespace easy3d;


CompositeView::CompositeView(const std::string& title)
    : Viewer(title)
    , grid_size_(30)
{
    grid_ = new LinesDrawable;
    update_grid();
}


void CompositeView::draw() const {
    // remember the viewer port and scissor status, later we will have to restore them
    int viewport[4], scissor[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetIntegerv(GL_SCISSOR_BOX, scissor);
    const int w = viewport[2];
    const int h = viewport[3];

    // ------------------------------------------------------------

    // Upper left view (wireframe)
    glViewport(0, h / 2, w / 2, h / 2);
    glScissor(0, h / 2, w / 2, h / 2);
    auto edges = current_model()->renderer()->get_lines_drawable("edges");
    if (!edges)
        edges = current_model()->renderer()->add_lines_drawable("edges");
    edges->draw(camera(), false);
    draw_grid();

    // ------------------------------------------------------------

    // Lower left view (wireframe + vertices)
    glViewport(0, 0, w / 2, h / 2);
    glScissor(0, 0, w / 2, h / 2);
    auto vertices = current_model()->renderer()->get_points_drawable("vertices");
    if (!vertices)
        vertices = current_model()->renderer()->add_points_drawable("vertices");
    vertices->set_point_size(15.0f);
    vertices->set_impostor_type(PointsDrawable::SPHERE);
    vertices->draw(camera(), false);
    edges->draw(camera(), false);
    draw_grid();

    // ------------------------------------------------------------

    // Lower right view (faces)
    glViewport(w / 2, 0, w / 2, h / 2);
    glScissor(w / 2, 0, w / 2, h / 2);
    auto faces = current_model()->renderer()->get_triangles_drawable("faces");
    if (!faces)
        faces = current_model()->renderer()->add_triangles_drawable("faces");
    faces->draw(camera(), false);
    draw_grid();

    // ------------------------------------------------------------

    // Upper right view (wireframe + faces)
    glViewport(w / 2, h / 2, w / 2, h / 2);
    glScissor(w / 2, h / 2, w / 2, h / 2);
    faces->draw(camera(), false);
    edges->draw(camera(), false);
    draw_grid();

    // ------------------------------------------------------------

    // restore scissor and viewport states.
    glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}


void CompositeView::draw_grid() const {
    ShaderProgram* program = ShaderManager::get_program("lines/lines_plain_color");
    if (!program) {
        std::vector<ShaderProgram::Attribute> attributes;
        attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
        program = ShaderManager::create_program_from_files("lines/lines_plain_color", attributes);
    }
    if (!program)
        return;

    int w = width() * 0.5f * dpi_scaling();
    int h = height() * 0.5f * dpi_scaling();
    const mat4& proj = transform::ortho(0.0f, w, h, 0.0f, 0.0f, -1.0f);

    // To make the grid appear behind other objects: reserve a tiny bit of the
    // back depth range. NOTE: do remember to restore it later.
    glDepthRangef(0.99f, 1.000f);

    program->bind();
    // the translation moves the grid to the view center
    program->set_uniform("MVP", proj * mat4::translation(w * 0.5f, h * 0.5f, 0.0f));
    program->set_uniform("per_vertex_color", false);
    program->set_uniform("default_color", vec4(0.0f, 0.0f, 1.0f, 1.0f));
    grid_->gl_draw(false);
    program->release();

    glDepthRangef(0.0f, 1.0f);
}


void CompositeView::post_resize(int w, int h) {
    update_grid();
}


void CompositeView::cleanup() {
    delete grid_;
    Viewer::cleanup();
}


void CompositeView::update_grid() {
    int x_steps = width() * 0.5f / grid_size_;
    int y_steps = height() * 0.5f / grid_size_;
    std::vector<vec3> points;
    opengl::prepare_grid(x_steps, y_steps, points, 0.99f, grid_size_ * dpi_scaling());
    grid_->update_vertex_buffer(points);
}
