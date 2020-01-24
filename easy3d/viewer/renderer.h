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

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/random.h>
#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/setting.h>

#include <cassert>

namespace easy3d {

    namespace renderer {

        /**
         * \brief Template functions preparing and updating *standard* drawable data to GPU.
         * \tparam MODEL    The model type - must be one of PointCloud, SurfaceMesh, or Graph.
         * \tparam DRAWABLE The drawable type - must be one of PointsDrawable, LinesDrawable,
         *                  or TrianglesDrawable.
         * \param model     The model.
         * \param drawable  The drawable.
         * \param smooth_shading Phone shading? Valid only for TrianglesDrawable.
         * \attention Actual functions are implemented in the instantiations. These functions
         *            are for *standard* drawables. You may need to implement your own function
         *            for a particular rendering purpose.
         */
        template <typename MODEL, typename DRAWABLE>
        inline void update_data(MODEL* model, DRAWABLE* drawable);

        /**
         * @brief Overload functions
         * @tparam DRAWABLE The drawable type - must be one of PointsDrawable, LinesDrawable,
         *         or TrianglesDrawable.
         * @param model The model (can be a PointCloud, Graph, or SurfaceMesh.
         * @param drawable The drawable.
         */
        template <typename DRAWABLE>
        inline void update_data(Model* model, DRAWABLE* drawable);

//        /**
//         * \brief Template functions preparing and updating *standard* drawable data to GPU.
//         *        These two are specialized for scalar and vector fields.
//         * \tparam MODEL    The model type - must be one of PointCloud, SurfaceMesh, or Graph.
//         * \tparam DRAWABLE The drawable type - must be one of PointsDrawable, LinesDrawable,
//         *                  or TrianglesDrawable.
//         * \tparam PROPERTY_TYPE The property type - must be float (scalar field) or vec3
//         *                       (vector field).
//         * \param model     The model.
//         * \param drawable  The drawable.
//         * \param prop_name The property name (for scalar or vector fields).
//         * \param smooth_shading Phone shading? Valid only for TrianglesDrawable.
//         * \attention Actual functions are implemented in the instantiations. These functions
//         *            are for *standard* drawables. You may need to implement your own function
//         *            for a particular rendering purpose.
//         */
//        template <typename MODEL, typename DRAWABLE, typename PROPERTY_TYPE>
//        inline void update_data(MODEL* model, DRAWABLE* drawable, const std::string& prop_name);

//        template <typename MODEL, typename DRAWABLE, typename PROPERTY_TYPE>
//        inline void update_data(MODEL* model, DRAWABLE* drawable, bool smooth_shading, const std::string& prop_name);

        /**
         * \brief Template specializations for point clouds
         */
        template <>
        inline void update_data<PointCloud, PointsDrawable>(PointCloud* model, PointsDrawable* drawable);

        template <>
        inline void update_data<PointCloud, LinesDrawable>(PointCloud* model, LinesDrawable* drawable) {}


        /**
         * \brief Template specializations for meshes
         */
        template <>
        inline void update_data<SurfaceMesh, PointsDrawable>(SurfaceMesh* model, PointsDrawable* drawable);

        template <>
        inline void update_data<SurfaceMesh, LinesDrawable>(SurfaceMesh* model, LinesDrawable* drawable);

        template <>
        inline void update_data<SurfaceMesh, TrianglesDrawable>(SurfaceMesh* model, TrianglesDrawable* drawable);


        /**
         * \brief Template specializations for graphes
         */
        template <>
        inline void update_data<Graph, PointsDrawable>(Graph* model, PointsDrawable* drawable);

        template <>
        inline void update_data<Graph, LinesDrawable>(Graph* model, LinesDrawable* drawable);

    }

}






/// Implementation

namespace easy3d {

    namespace renderer {

        template <typename DRAWABLE>
        inline void update_data(Model* model, DRAWABLE* drawable) {
            if (dynamic_cast<PointCloud*>(model))
                update_data<PointCloud, DRAWABLE>(dynamic_cast<PointCloud*>(model), drawable);
            else if (dynamic_cast<Graph*>(model))
                update_data<Graph, DRAWABLE>(dynamic_cast<Graph*>(model), drawable);
            else if (dynamic_cast<SurfaceMesh*>(model))
                update_data<SurfaceMesh, DRAWABLE>(dynamic_cast<SurfaceMesh*>(model), drawable);
        }

        /**
         * \brief Template specializations for point clouds
         */
        template <>
        inline void update_data<PointCloud, PointsDrawable>(PointCloud* model, PointsDrawable* drawable) {
            assert(model);
            assert(drawable);

            // segmentation information has been stored as properties:
            //      - "v:primitive_type"  (one of PLANE, SPHERE, CYLINDER, CONE, TORUS, and UNKNOWN)
            //      - "v:primitive_index" (0, 1, 2...)
            auto primitive_type = model->get_vertex_property<int>("v:primitive_type");
            auto primitive_index = model->get_vertex_property<int>("v:primitive_index");
            if (primitive_type && primitive_index) { // model has segmentation informaiton
                int num = 0;
                for (auto v : model->vertices())
                    num = std::max(num, primitive_index[v]);
                ++num;
                // assign each plane a unique color
                std::vector<vec3> color_table(num);
                for (auto& c : color_table)
                    c = random_color();

                std::vector<vec3> colors;
                for (auto v : model->vertices()) {
                     int idx = primitive_index[v];
                    if (primitive_type[v] == -1)
                        colors.push_back(vec3(0, 0, 0));
                    else
                        colors.push_back(color_table[idx]); // black for unkonwn type
                }
                drawable->update_color_buffer(colors);
                drawable->set_per_vertex_color(true);

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());
                auto normals = model->get_vertex_property<vec3>("v:normal");
                if (normals)
                    drawable->update_normal_buffer(normals.vector());
            }

            else {
                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());
                auto normals = model->get_vertex_property<vec3>("v:normal");
                if (normals)
                    drawable->update_normal_buffer(normals.vector());
                auto colors = model->get_vertex_property<vec3>("v:color");
                if (colors) {
                    drawable->update_color_buffer(colors.vector());
                    drawable->set_per_vertex_color(true);
                }
                else {
                    drawable->set_default_color(setting::point_cloud_points_color);
                    drawable->set_per_vertex_color(false);
                }
            }
        }



        /**
         * \brief Template specializations for meshes
         */

        template <>
        inline void update_data<SurfaceMesh, PointsDrawable>(SurfaceMesh* model, PointsDrawable* drawable) {
            auto points = model->get_vertex_property<vec3>("v:point");
            drawable->update_vertex_buffer(points.vector());
            drawable->set_default_color(setting::surface_mesh_vertices_color);
            drawable->set_per_vertex_color(false);
            drawable->set_point_size(setting::surface_mesh_vertices_point_size);
            drawable->set_impostor_type(PointsDrawable::SPHERE);
        }

        template <>
        inline void update_data<SurfaceMesh>(SurfaceMesh* model, TrianglesDrawable* drawable) {
            assert(model);
            assert(drawable);

            // [Liangliang] How I achieve efficient switch between flat and smooth shading.
            //      Always transfer vertex normals to GPU and the normals for flat shading are
            //      computed on the fly in the fragment shader:
            //              normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
            //     This way, we can use a uniform smooth_shading to switch between flat and smooth
            //     shading, avoiding transfering different data to the GPU.

            auto points = model->get_vertex_property<vec3>("v:point");
            drawable->update_vertex_buffer(points.vector());
            auto colors = model->get_vertex_property<vec3>("v:color");
            if (colors) {
                drawable->update_color_buffer(colors.vector());
                drawable->set_per_vertex_color(true);
            }
            auto texcoords = model->get_vertex_property<vec2>("v:texcoord");
            if (texcoords)
                drawable->update_texcoord_buffer(texcoords.vector());

            auto normals = model->get_vertex_property<vec3>("v:normal");
            if (!normals) {
                model->update_vertex_normals();
                normals = model->get_vertex_property<vec3>("v:normal");
            }

            drawable->update_normal_buffer(normals.vector());

            std::vector<unsigned int> indices;
            for (auto f : model->faces()) {
                // we assume convex polygonal faces and we render in triangles
                SurfaceMesh::Halfedge start = model->halfedge(f);
                SurfaceMesh::Halfedge cur = model->next_halfedge(model->next_halfedge(start));
                SurfaceMesh::Vertex va = model->to_vertex(start);
                while (cur != start) {
                    SurfaceMesh::Vertex vb = model->from_vertex(cur);
                    SurfaceMesh::Vertex vc = model->to_vertex(cur);
                    indices.push_back(static_cast<unsigned int>(va.idx()));
                    indices.push_back(static_cast<unsigned int>(vb.idx()));
                    indices.push_back(static_cast<unsigned int>(vc.idx()));
                    cur = model->next_halfedge(cur);
                }
            }
            drawable->update_index_buffer(indices);
        }

        template <>
        inline void update_data<SurfaceMesh, LinesDrawable>(SurfaceMesh* model, LinesDrawable* drawable) {
            std::vector<unsigned int> indices;
            for (auto e : model->edges()) {
                SurfaceMesh::Vertex s = model->vertex(e, 0);
                SurfaceMesh::Vertex t = model->vertex(e, 1);
                indices.push_back(s.idx());
                indices.push_back(t.idx());
            }
            auto points = model->get_vertex_property<vec3>("v:point");
            drawable->update_vertex_buffer(points.vector());
            drawable->update_index_buffer(indices);
            drawable->set_default_color(setting::surface_mesh_edges_color);
            drawable->set_per_vertex_color(false);
            drawable->set_line_width(setting::surface_mesh_edges_line_width);
        }



        /**
         * \brief Template specializations for graphs
         */
        template <>
        inline void update_data<Graph, PointsDrawable>(Graph* model, PointsDrawable* drawable) {
            auto points = model->get_vertex_property<vec3>("v:point");
            drawable->update_vertex_buffer(points.vector());
            drawable->set_per_vertex_color(false);
            drawable->set_default_color(vec3(1.0f, 0.0f, 0.0f));
            drawable->set_point_size(15.0f);
            drawable->set_impostor_type(PointsDrawable::SPHERE);
        }

        template <>
        inline void update_data<Graph, LinesDrawable>(Graph* model, LinesDrawable* drawable) {
            auto points = model->get_vertex_property<vec3>("v:point");
            drawable->update_vertex_buffer(points.vector());

            std::vector<unsigned int> indices;
            for (auto e : model->edges()) {
                unsigned int s = model->from_vertex(e).idx();    indices.push_back(s);
                unsigned int t = model->to_vertex(e).idx();      indices.push_back(t);
            }
            drawable->update_index_buffer(indices);

            drawable->set_per_vertex_color(false);
            drawable->set_default_color(vec3(1.0f, 0.67f, 0.5f));
            drawable->set_line_width(3.0f);
            drawable->set_impostor_type(LinesDrawable::CYLINDER);
        }


    }

}
