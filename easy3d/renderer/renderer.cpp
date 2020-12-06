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

#include <easy3d/renderer/renderer.h>
#include <easy3d/core/random.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/setting.h>


namespace easy3d {

    Renderer::Renderer(Model* model, bool create)
            : visible_(true)
            , selected_(false)
    {
        model_ = model;
        if (model_) {
            model_->set_renderer(this);

            if (create)
                create_default_drawables(model_);
        }
    }


    Renderer::~Renderer() {
        for (auto d : points_drawables_)	delete d;
        for (auto d : lines_drawables_)	    delete d;
        for (auto d : triangles_drawables_)	delete d;
    }

    
    void Renderer::create_default_drawables(Model *model) {
        if (dynamic_cast<PointCloud *>(model)) {
            PointCloud *cloud = dynamic_cast<PointCloud *>(model);
            auto vertices = cloud->renderer()->add_points_drawable("vertices");
            vertices->set_point_size(setting::point_cloud_point_size);
            vertices->set_visible(true);
            set_default_rendering_state(cloud, vertices);
        } else if (dynamic_cast<SurfaceMesh *>(model)) {
            SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(model);

            // faces
            auto faces = mesh->renderer()->add_triangles_drawable("faces");
            faces->set_visible(true);
            set_default_rendering_state(mesh, faces);

            // edges
            auto edges = mesh->renderer()->add_lines_drawable("edges");
            edges->set_uniform_coloring(setting::surface_mesh_edges_color);
            edges->set_line_width(setting::surface_mesh_edges_line_width);
            edges->set_visible(setting::surface_mesh_show_edges);

            // vertices
            auto vertices = mesh->renderer()->add_points_drawable("vertices");
            vertices->set_uniform_coloring(setting::surface_mesh_vertices_color);
            vertices->set_impostor_type(PointsDrawable::SPHERE);
            vertices->set_point_size(setting::surface_mesh_vertices_point_size);
            vertices->set_visible(setting::surface_mesh_show_vertices);

            // borders
            auto borders = mesh->renderer()->add_lines_drawable("borders");
            borders->set_uniform_coloring(setting::surface_mesh_borders_color);
            borders->set_impostor_type(LinesDrawable::CYLINDER);
            borders->set_line_width(setting::surface_mesh_borders_line_width);
            borders->set_visible(setting::surface_mesh_show_borders);
        } else if (dynamic_cast<Graph *>(model)) {
            Graph *graph = dynamic_cast<Graph *>(model);
            // create points drawable for the edges
            auto vertices = graph->renderer()->add_points_drawable("vertices");
            vertices->set_uniform_coloring(setting::graph_vertices_color);
            vertices->set_point_size(setting::graph_vertices_point_size);
            vertices->set_impostor_type(PointsDrawable::SPHERE);
            vertices->set_visible(true);

            // create liens drawable for the edges
            auto edges = graph->renderer()->add_lines_drawable("edges");
            edges->set_uniform_coloring(setting::graph_edges_color);
            edges->set_line_width(setting::graph_edges_line_width);
            edges->set_impostor_type(LinesDrawable::CYLINDER);
            edges->set_visible(true);
        }
    }


    void Renderer::set_default_rendering_state(PointCloud *model, PointsDrawable *drawable) {
        assert(model);
        assert(drawable);

        auto colors = model->get_vertex_property<vec3>("v:color");
        if (colors) {
            drawable->set_property_coloring(State::VERTEX, "v:color");
            return;
        }

        auto texcoord = model->get_vertex_property<vec2>("v:texcoord");
        if (texcoord) {
            drawable->set_texture_coloring(State::VERTEX, "v:texcoord");
            return;
        }

        auto primitive_index = model->get_vertex_property<int>("v:primitive_index");
        if (primitive_index) { // model has segmentation information
            drawable->set_scalar_coloring(State::VERTEX, "v:primitive_index");
            return;
        }

        drawable->set_uniform_coloring(setting::point_cloud_points_color);
    }


    // -----------------------------------------------------------------------------------------------------------

    void Renderer::set_default_rendering_state(SurfaceMesh *model, TrianglesDrawable *drawable) {
        assert(model);
        assert(drawable);

        //  Priorities:
        //      1: per-face color
        //      2: per-vertex color
        //      3. per-halfedge texture coordinates
        //      4. per-vertex texture coordinates
        //      5. segmentation
        //      6 uniform color

        auto face_colors = model->get_face_property<vec3>("f:color");
        if (face_colors) {
            drawable->set_property_coloring(State::FACE, "f:color");
            return;
        }

        auto vertex_colors = model->get_vertex_property<vec3>("v:color");
        if (vertex_colors) {
            drawable->set_property_coloring(State::VERTEX, "v:color");
            return;
        }

        auto halfedge_texcoords = model->get_halfedge_property<vec2>("h:texcoord");
        if (halfedge_texcoords) {
            drawable->set_texture_coloring(State::HALFEDGE, "h:texcoord");
            return;
        }

        auto vertex_texcoords = model->get_vertex_property<vec2>("v:texcoord");
        if (vertex_texcoords) {
            drawable->set_texture_coloring(State::VERTEX, "v:texcoord");
            return;
        }

        auto segmentation = model->get_face_property<int>("f:chart");
        if (segmentation) {
            drawable->set_scalar_coloring(State::FACE, "f:chart");
            return;
        }

        drawable->set_uniform_coloring(setting::surface_mesh_faces_color);
    }


    void Renderer::color_from_segmentation(SurfaceMesh *model, const std::string &segments_name, const std::string &color_name) {
        if (model->empty()) {
            LOG(WARNING) << "model has no valid geometry";
            return;
        }

        auto segments = model->get_face_property<int>(segments_name);
        if (!segments) {
            LOG(WARNING) << "the surface mesh does not have a face property named '" << segments_name << "\'";
            return;
        }

        int max_index = 0;
        for (auto f : model->faces())
            max_index = std::max(max_index, segments[f]);

        // assign each segment a unique color
        std::vector<vec3> color_table(max_index + 1);   // index starts from 0
        for (auto &c : color_table)
            c = random_color();

        auto colors = model->face_property<vec3>(color_name, vec3(0, 0, 0));
        for (auto f : model->faces()) {
            int idx = segments[f];
            if (idx == -1)
                colors[f] = vec3(0, 0, 0);
            else
                colors[f] = color_table[idx];
        }
    }


    void Renderer::color_from_segmentation(PointCloud *model, const std::string &segments_name, const std::string &color_name) {
        if (model->empty()) {
            LOG(WARNING) << "model has no valid geometry";
            return;
        }

        auto segments = model->get_vertex_property<int>(segments_name);
        if (!segments) {
            LOG(WARNING) << "the point cloud does not have a vertex property named '" << segments_name << "\'";
            return;
        }

        int max_index = 0;
        for (auto v : model->vertices())
            max_index = std::max(max_index, segments[v]);

        // assign each segment a unique color
        std::vector<vec3> color_table(max_index + 1);   // index starts from 0
        for (auto &c : color_table)
            c = random_color();

        auto colors = model->vertex_property<vec3>(color_name, vec3(0, 0, 0));
        for (auto v : model->vertices()) {
            int idx = segments[v];
            if (idx == -1)
                colors[v] = vec3(0, 0, 0);
            else
                colors[v] = color_table[idx];
        }
    }


    void Renderer::update() {
        model_->invalidate_bounding_box();

        for (auto d : points_drawables_)
            d->update();
        for (auto d : lines_drawables_)
            d->update();
        for (auto d : triangles_drawables_)
            d->update();
    }


    PointsDrawable* Renderer::get_points_drawable(const std::string& name) const {
        for (auto d : points_drawables_) {
            if (d->name() == name)
                return d;
        }
        return nullptr;
    }


    LinesDrawable* Renderer::get_lines_drawable(const std::string& name) const {
        for (auto d : lines_drawables_) {
            if (d->name() == name)
                return d;
        }
        return nullptr;
    }


    TrianglesDrawable* Renderer::get_triangles_drawable(const std::string &name) const {
        for (auto d : triangles_drawables_) {
            if (d->name() == name)
                return d;
        }
        return nullptr;
    }


    PointsDrawable* Renderer::add_points_drawable(const std::string& name) {
        for (auto d : points_drawables_) {
            if (d->name() == name) {
                LOG(ERROR) << "drawable already exists: " << name;
                return d;
            }
        }
        PointsDrawable* d = new PointsDrawable(name);
        d->set_model(model_);
        points_drawables_.push_back(d);
        return d;
    }


    LinesDrawable* Renderer::add_lines_drawable(const std::string& name) {
        for (auto d : lines_drawables_) {
            if (d->name() == name) {
                LOG(ERROR) << "drawable already exists: " << name;
                return d;
            }
        }
        LinesDrawable* d = new LinesDrawable(name);
        d->set_model(model_);
        lines_drawables_.push_back(d);
        return d;
    }


    TrianglesDrawable* Renderer::add_triangles_drawable(const std::string& name) {
        for (auto d : triangles_drawables_) {
            if (d->name() == name) {
                LOG(ERROR) << "drawable already exists: " << name;
                return d;
            }
        }
        TrianglesDrawable* d = new TrianglesDrawable(name);
        d->set_model(model_);
        triangles_drawables_.push_back(d);
        return d;
    }

}
