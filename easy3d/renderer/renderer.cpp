/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
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
 ********************************************************************/

#include <easy3d/renderer/renderer.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/setting.h>


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
            auto cloud = dynamic_cast<PointCloud *>(model);
            auto vertices = cloud->renderer()->add_points_drawable("vertices");
			vertices->set_visible(setting::point_cloud_vertices_visible);
			vertices->set_color(setting::point_cloud_vertices_color);
            vertices->set_impostor_type(setting::point_cloud_vertices_impostors ? PointsDrawable::SPHERE : PointsDrawable::PLAIN);
            vertices->set_point_size(setting::point_cloud_vertices_size);
            set_default_rendering_state(cloud, vertices);
        } else if (dynamic_cast<SurfaceMesh *>(model)) {
            auto mesh = dynamic_cast<SurfaceMesh *>(model);

            // faces
            auto faces = mesh->renderer()->add_triangles_drawable("faces");
            faces->set_smooth_shading(setting::surface_mesh_faces_phong_shading);
            faces->set_visible(setting::surface_mesh_faces_visible);
            faces->set_color(setting::surface_mesh_faces_color);
            faces->set_opacity(setting::surface_mesh_faces_opacity);
            set_default_rendering_state(mesh, faces);

			// vertices
			auto vertices = mesh->renderer()->add_points_drawable("vertices");
			vertices->set_visible(setting::surface_mesh_vertices_visible);
			vertices->set_uniform_coloring(setting::surface_mesh_vertices_color);
			vertices->set_impostor_type(setting::surface_mesh_vertices_imposters ? PointsDrawable::SPHERE : PointsDrawable::PLAIN);
			vertices->set_point_size(setting::surface_mesh_vertices_size);

            // edges
            auto edges = mesh->renderer()->add_lines_drawable("edges");
            edges->set_visible(setting::surface_mesh_edges_visible);
            edges->set_uniform_coloring(setting::surface_mesh_edges_color);
            edges->set_impostor_type(setting::surface_mesh_edges_imposters ? LinesDrawable::CYLINDER : LinesDrawable::PLAIN);
            edges->set_line_width(setting::surface_mesh_edges_size);

            // borders
            auto borders = mesh->renderer()->add_lines_drawable("borders");
            borders->set_visible(setting::surface_mesh_borders_visible);
            borders->set_uniform_coloring(setting::surface_mesh_borders_color);
            borders->set_impostor_type(setting::surface_mesh_borders_imposters ? LinesDrawable::CYLINDER : LinesDrawable::PLAIN);
            borders->set_line_width(setting::surface_mesh_borders_size);

            auto locks_prop = mesh->get_vertex_property<bool>("v:locked");
            if (locks_prop) {
                auto locks = mesh->renderer()->add_points_drawable("locks");
                locks->set_uniform_coloring(vec4(1, 1, 0, 1.0f));
                locks->set_impostor_type(PointsDrawable::SPHERE);
                locks->set_point_size(setting::surface_mesh_vertices_size + 5);
            }
        } else if (dynamic_cast<Graph *>(model)) {
            auto graph = dynamic_cast<Graph *>(model);
            // create points drawable for the edges
            auto vertices = graph->renderer()->add_points_drawable("vertices");
            vertices->set_visible(setting::graph_vertices_visible);
            vertices->set_uniform_coloring(setting::graph_vertices_color);
            vertices->set_impostor_type(setting::graph_vertices_imposters ? PointsDrawable::SPHERE : PointsDrawable::PLAIN);
			vertices->set_point_size(setting::graph_vertices_size);

            // create lines drawable for the edges
            auto edges = graph->renderer()->add_lines_drawable("edges");
            edges->set_visible(setting::graph_edges_visible);
            edges->set_uniform_coloring(setting::graph_edges_color);
            edges->set_impostor_type(setting::graph_edges_imposters ? LinesDrawable::CYLINDER : LinesDrawable::PLAIN);
			edges->set_line_width(setting::graph_edges_size);
		} else if (dynamic_cast<PolyMesh *>(model)) {
            auto mesh = dynamic_cast<PolyMesh *>(model);

            // we have two faces drawables for polyhedral meshes
            // border faces
            auto border_faces = mesh->renderer()->add_triangles_drawable("faces:border");
			border_faces->set_visible(setting::poly_mesh_faces_visible);
            border_faces->set_uniform_coloring(setting::poly_mesh_faces_color);
            border_faces->set_distinct_back_color(true);
            border_faces->set_lighting_two_sides(true);

            // interior faces
            auto interior_faces = mesh->renderer()->add_triangles_drawable("faces:interior");
			interior_faces->set_visible(setting::poly_mesh_faces_visible);
            interior_faces->set_uniform_coloring(setting::triangles_drawable_backside_color);
            interior_faces->set_distinct_back_color(true);
            interior_faces->set_lighting_two_sides(true);

			// vertices
			auto vertices = mesh->renderer()->add_points_drawable("vertices");
			vertices->set_visible(setting::poly_mesh_vertices_visible);
			vertices->set_uniform_coloring(setting::poly_mesh_vertices_color);
			vertices->set_impostor_type(setting::poly_mesh_vertices_imposters ? PointsDrawable::SPHERE : PointsDrawable::PLAIN);
			vertices->set_point_size(setting::poly_mesh_vertices_size);

            // edges
            auto edges = mesh->renderer()->add_lines_drawable("edges");
			edges->set_visible(setting::poly_mesh_edges_visible);
            edges->set_uniform_coloring(setting::poly_mesh_edges_color);
            edges->set_impostor_type(setting::poly_mesh_edges_imposters ? LinesDrawable::CYLINDER : LinesDrawable::PLAIN);
			edges->set_line_width(setting::poly_mesh_edges_size);
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

        // segmentation
        auto primitive_index = model->get_vertex_property<int>("v:primitive_index");
        if (primitive_index) { // model has segmentation information
            drawable->set_scalar_coloring(State::VERTEX, "v:primitive_index");
            return;
        }

        // other unknown scalar fields
        const auto properties = model->vertex_properties();
        for (const auto& name : properties) {
            if (model->get_vertex_property<int>(name) || model->get_vertex_property<unsigned int>(name) ||
                model->get_vertex_property<float>(name)) {
                drawable->set_scalar_coloring(State::VERTEX, name);
                return;
            }
        }

        drawable->set_uniform_coloring(setting::point_cloud_vertices_color);
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

        // segmentation
        auto segmentation = model->get_face_property<int>("f:chart");
        if (segmentation) {
            drawable->set_scalar_coloring(State::FACE, "f:chart");
            return;
        }

        // other unknown scalar fields on faces
        const auto face_properties = model->face_properties();
        for (const auto& name : face_properties) {
            if (model->get_face_property<int>(name) || model->get_face_property<unsigned int>(name) ||
                model->get_face_property<float>(name)) {
                drawable->set_scalar_coloring(State::FACE, name);
                return;
            }
        }

        // other unknown scalar fields on vertices
        const auto vertex_properties = model->vertex_properties();
        for (const auto& name : vertex_properties) {
            if (model->get_vertex_property<int>(name) || model->get_vertex_property<unsigned int>(name) ||
                model->get_vertex_property<float>(name)) {
                drawable->set_scalar_coloring(State::VERTEX, name);
                return;
            }
        }

        drawable->set_uniform_coloring(setting::surface_mesh_faces_color);
    }


    void Renderer::set_selected(bool b) {
        for (auto d : points_drawables_)
            d->set_selected(b);
        for (auto d : lines_drawables_)
            d->set_selected(b);
        for (auto d : triangles_drawables_)
            d->set_selected(b);
        selected_ = b;
    }


    void Renderer::update() {
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
        auto d = new PointsDrawable(name);
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
        auto d = new LinesDrawable(name);
        d->set_model(model_);
        lines_drawables_.push_back(d);

        // for PolyMesh, we want to completely discard vertices in the vertex buffer.
        if (dynamic_cast<PolyMesh*>(model_))
            d->set_plane_clip_discard_primitive(true);

        return d;
    }


    TrianglesDrawable* Renderer::add_triangles_drawable(const std::string& name) {
        for (auto d : triangles_drawables_) {
            if (d->name() == name) {
                LOG(ERROR) << "drawable already exists: " << name;
                return d;
            }
        }
        auto d = new TrianglesDrawable(name);
        d->set_model(model_);
        triangles_drawables_.push_back(d);

        // for PolyMesh, we want to completely discard vertices in the vertex buffer.
        if (dynamic_cast<PolyMesh*>(model_))
            d->set_plane_clip_discard_primitive(true);

        return d;
    }

}
