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


namespace easy3d {

    class PointsDrawable;
    class LinesDrawable;
    class TrianglesDrawable;

    namespace renderer {

        // PointCloud -------------------------------------------------------------------------------------------------

        /**
         * @brief Update render buffers for the default "vertices" drawable of a point cloud.
         *        The "vertices" drawable of a point cloud may have both per-vertex color and per-vertex texture
         *        coordinate, it may also have segmentation information.
         *        In case of multiple coloring possibilities, the following priority applies:
         *              1. segmentation-based colors
         *              2. per-vertex texture coordinates
         *              3: per-vertex color
         *              4: uniform color
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update_buffer(PointCloud* model, PointsDrawable* drawable);

        /**
         * @brief Update render buffers for the default "vertices" drawable of a point cloud using the vertex property
         *        "v:color".
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property "v:color"
         */
        void update_buffer(PointCloud* model, PointsDrawable* drawable, PointCloud::VertexProperty<vec3> prop);

        /**
         * @brief Update render buffers for the default "vertices" drawable of a point cloud using the scalar field
         *        defined one vertices.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property that defines the scalar field.
         */
        template <typename FT>
        void update_buffer(PointCloud* model, PointsDrawable* drawable, PointCloud::VertexProperty<FT> prop);

        /**
         * @brief Update render buffers for the default "vertices" drawable of a point cloud using the texture
         *        coordinates defined on vertices.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property (i.e., "v:texcoord") that defines the texture coordinates.
         */
        void update_buffer(PointCloud* model, PointsDrawable* drawable, PointCloud::VertexProperty<vec2> prop);

        
        // SurfaceMesh ------------------------------------------------------------------------------------------------
        
        /**
         * @brief Update render buffers for the default "vertices" drawable of a surface mesh.
         *        The "vertices" drawable of a surface mesh may have a per-vertex color property and a per-vertex
         *        texture coordinate property. In this case, the following priority applies:
         *              1. per-vertex texture coordinates
         *              2: per-vertex color
         *              3: uniform color
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update_buffer(SurfaceMesh* model, PointsDrawable* drawable);

        /**
         * @brief Update render buffers for the default "vertices" drawable of a surface mesh using the vertex property
         *        "v:color".
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property "v:color"
         */
        void update_buffer(SurfaceMesh* model, PointsDrawable* drawable, SurfaceMesh::VertexProperty<vec3> prop);

        /**
         * @brief Update render buffers for the default "vertices" drawable of a surface mesh using the scalar field
         *        defined one vertices.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property that defines the scalar field.
         */
        template <typename FT>
        void update_buffer(SurfaceMesh* model, PointsDrawable* drawable, SurfaceMesh::VertexProperty<FT> prop);

        /**
         * @brief Update render buffers for the default "vertices" drawable of a surface mesh using the texture
         *        coordinates defined on vertices.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property (i.e., "v:texcoord") that defines the texture coordinates.
         */
        void update_buffer(SurfaceMesh* model, PointsDrawable* drawable, SurfaceMesh::VertexProperty<vec2> prop);

        /**
         * @brief Update render buffers for the default "edges" drawable of a surface mesh.
         *        The "edges" drawable (i.e., wireframe) of a surface mesh may have different color properties
         *        (per-edge/vertex color) and different texture coordinates (e.g., per-edge/vertex texture coordinates).
         *        In case of multiple coloring possibilities, the following priority applies:
         *              1. per-edge texture coordinates
         *              2. per-vertex texture coordinates
         *              3: per-edge color
         *              4: per-vertex color
         *              5: uniform color
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update_buffer(SurfaceMesh* model, LinesDrawable* drawable);

        /**
         * @brief Update render buffers for the default "edges" drawable of a surface mesh using the edge property
         *        "e:color".
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The edge property "e:color" the defined a per-edge color.
         */
        void update_buffer(SurfaceMesh* model, LinesDrawable* drawable, SurfaceMesh::EdgeProperty<vec3> prop);

        /**
         * @brief Update render buffers for the default "edges" drawable of a surface mesh using the scalar field
         *        defined one edges.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The edge property that defines the scalar field.
         */
        template <typename FT>
        void update_buffer(SurfaceMesh* model, LinesDrawable* drawable, SurfaceMesh::EdgeProperty<FT> prop);

        /**
         * @brief Update render buffers for the default "edges" drawable of a surface mesh using the vertex property
         *        "v:color".
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property "v:color"
         */
        void update_buffer(SurfaceMesh* model, LinesDrawable* drawable, SurfaceMesh::VertexProperty<vec3> prop);

        /**
         * @brief Update render buffers for the default "edges" drawable of a surface mesh using the scalar field
         *        defined one vertices.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property that defines the scalar field.
         */
        template <typename FT>
        void update_buffer(SurfaceMesh* model, LinesDrawable* drawable, SurfaceMesh::VertexProperty<FT> prop);

        /**
         * @brief Update render buffers for the default "edges" drawable of a surface mesh using the texture coordinates
         *        defined on vertices.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property (i.e., "v:texcoord") that defines the texture coordinates.
         */
        void update_buffer(SurfaceMesh* model, LinesDrawable* drawable, SurfaceMesh::VertexProperty<vec2> prop);

        /**
         * @brief Update render buffers for the default "edges" drawable of a surface mesh using the texture coordinates
         *        defined on edges.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The edge property (i.e., "e:texcoord") that defines the texture coordinates.
         */
        void update_buffer(SurfaceMesh* model, LinesDrawable* drawable, SurfaceMesh::EdgeProperty<vec2> prop);

        /**
         * @brief Update render buffers for the default "faces" drawable of a surface mesh.
         *        A surface mesh may have different color properties (per-face/vertex color) and different texture
         *        coordinates (e.g., per-halfedge/vertex texture coordinates).  
         *        In case of multiple coloring possibilities, the following priority applies:
         *              1. per-halfedge texture coordinates
         *              2. per-vertex texture coordinates
         *              3: per-face color
         *              4: per-vertex color
         *              5: uniform color
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable);

        /**
         * @brief Update render buffers for the default "faces" drawable of a surface mesh using the face property 
         *        "f:color".
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The face property "f:color"
         */
        void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::FaceProperty<vec3> prop);

        /**
         * @brief Update render buffers for the default "faces" drawable of a surface mesh using the scalar field 
         *        defined one faces.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The face property that defines the scalar field.
         */
        template <typename FT>
        void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::FaceProperty<FT> prop);

        /**
         * @brief Update render buffers for the default "faces" drawable of a surface mesh using the vertex property 
         *        "v:color".
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property "v:color"
         */
        void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::VertexProperty<vec3> prop);
        
        /**
         * @brief Update render buffers for the default "faces" drawable of a surface mesh using the scalar field 
         *        defined one vertices.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property that defines the scalar field.
         */
        template <typename FT>
        void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::VertexProperty<FT> prop);

        /**
         * @brief Update render buffers for the default "faces" drawable of a surface mesh using the texture coordinates
         *        defined on vertices.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property (i.e., "v:texcoord") that defines the texture coordinates.
         */
        void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::VertexProperty<vec2> prop);

        /**
         * @brief Update render buffers for the default "faces" drawable of a surface mesh using the texture coordinates
         *        defined on halfedges.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The halfedge property (i.e., "h:texcoord") that defines the texture coordinates.
         */
        void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::HalfedgeProperty<vec2> prop);

        // Graph ------------------------------------------------------------------------------------------------------

        /**
         * @brief Update render buffers for the default "vertices" drawable of a graph.
         *        The "vertices" drawable of a graph may have both per-vertex color and per-vertex texture coordinate.
         *        In case of multiple coloring possibilities, the following priority applies:
         *              1. per-vertex texture coordinates
         *              2: per-vertex color
         *              3: uniform color
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update_buffer(Graph* model, PointsDrawable* drawable);

        /**
         * @brief Update render buffers for the default "vertices" drawable of a graph using the vertex property
         *        "v:color".
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property "v:color"
         */
        void update_buffer(Graph* model, PointsDrawable* drawable, Graph::VertexProperty<vec3> prop);

        /**
         * @brief Update render buffers for the default "vertices" drawable of a graph using the scalar field
         *        defined one vertices.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property that defines the scalar field.
         */
        template <typename FT>
        void update_buffer(Graph* model, PointsDrawable* drawable, Graph::VertexProperty<FT> prop);

        /**
         * @brief Update render buffers for the default "vertices" drawable of a graph using the texture
         *        coordinates defined on vertices.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property (i.e., "v:texcoord") that defines the texture coordinates.
         */
        void update_buffer(Graph* model, PointsDrawable* drawable, Graph::VertexProperty<vec2> prop);

        /**
         * @brief Update render buffers for the default "edges" drawable of a graph.
         *        The "edges" drawable (i.e., wireframe) of a graph may have different color properties
         *        (per-edge/vertex color) and different texture coordinates (e.g., per-edge/vertex texture coordinates).
         *        In case of multiple coloring possibilities, the following priority applies:
         *              1. per-edge texture coordinates
         *              2. per-vertex texture coordinates
         *              3: per-edge color
         *              4: per-vertex color
         *              5: uniform color
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update_buffer(Graph* model, LinesDrawable* drawable);

        /**
         * @brief Update render buffers for the default "edges" drawable of a graph using the edge property "e:color".
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The edge property "e:color" the defined a per-edge color.
         */
        void update_buffer(Graph* model, LinesDrawable* drawable, Graph::EdgeProperty<vec3> prop);

        /**
         * @brief Update render buffers for the default "edges" drawable of a graph using the scalar field
         *        defined one edges.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The edge property that defines the scalar field.
         */
        template <typename FT>
        void update_buffer(Graph* model, LinesDrawable* drawable, Graph::EdgeProperty<FT> prop);

        /**
         * @brief Update render buffers for the default "edges" drawable of a graph using the vertex property "v:color".
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property "v:color"
         */
        void update_buffer(Graph* model, LinesDrawable* drawable, Graph::VertexProperty<vec3> prop);

        /**
         * @brief Update render buffers for the default "edges" drawable of a graph using the scalar field defined one
         *        vertices.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property that defines the scalar field.
         */
        template <typename FT>
        void update_buffer(Graph* model, LinesDrawable* drawable, Graph::VertexProperty<FT> prop);

        /**
         * @brief Update render buffers for the default "edges" drawable of a graph using the texture coordinates
         *        defined on vertices.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The vertex property (i.e., "v:texcoord") that defines the texture coordinates.
         */
        void update_buffer(Graph* model, LinesDrawable* drawable, Graph::VertexProperty<vec2> prop);

        /**
         * @brief Update render buffers for the default "edges" drawable of a graph using the texture coordinates
         *        defined on edges.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param prop      The edge property (i.e., "e:texcoord") that defines the texture coordinates.
         */
        void update_buffer(Graph* model, LinesDrawable* drawable, Graph::EdgeProperty<vec2> prop);


        // a template version
        template <typename DRAWABLE>
        inline void update_buffer(Model* model, DRAWABLE* drawable) {
            if (dynamic_cast<PointCloud*>(model))
                update_buffer(dynamic_cast<PointCloud*>(model), drawable);
            else if (dynamic_cast<Graph*>(model))
                update_buffer(dynamic_cast<Graph*>(model), drawable);
            else if (dynamic_cast<SurfaceMesh*>(model))
                update_buffer(dynamic_cast<SurfaceMesh*>(model), drawable);
        }
    }

}




#include <cassert>

#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/tessellator.h>
#include <easy3d/util/logging.h>

namespace easy3d {

    namespace renderer {

        template <typename FT>
        inline void update_buffer(SurfaceMesh* model, PointsDrawable* drawable, SurfaceMesh::VertexProperty<FT> prop) {
            float min_value = std::numeric_limits<FT>::max();
            float max_value = -std::numeric_limits<FT>::max();
            for (auto v : model->vertices()) {
                float value = prop[v];
                min_value = std::min(value, min_value);
                max_value = std::max(value, max_value);
            }

            auto points = model->get_vertex_property<vec3>("v:point");

            std::vector<vec2> d_texcoords;  d_texcoords.reserve(model->n_vertices());
            for (auto v : model->vertices()) {
                float coord = (prop[v] - min_value) / (max_value - min_value);
                d_texcoords.emplace_back(vec2(coord, 0.5));
            }
            drawable->update_vertex_buffer(points.vector());
            drawable->update_texcoord_buffer(d_texcoords);
        }


        template <typename FT>
        inline void update_buffer(SurfaceMesh* model, LinesDrawable* drawable, SurfaceMesh::EdgeProperty<FT> prop) {
            float min_value = std::numeric_limits<FT>::max();
            float max_value = -std::numeric_limits<FT>::max();
            for (auto e : model->edges()) {
                float value = prop[e];
                min_value = std::min(value, min_value);
                max_value = std::max(value, max_value);
            }

            auto points = model->get_vertex_property<vec3>("v:point");
            std::vector<vec3> d_points;     d_points.reserve(model->n_edges() * 2);
            std::vector<vec2> d_texcoords;  d_texcoords.reserve(model->n_edges() * 2);
            for (auto e : model->edges()) {
                SurfaceMesh::Vertex s = model->vertex(e, 0);
                SurfaceMesh::Vertex t = model->vertex(e, 1);
                d_points.push_back(points[s]);
                d_points.push_back(points[t]);
                float coord = (prop[e] - min_value) / (max_value - min_value);
                d_texcoords.emplace_back(vec2(coord, 0.5));
                d_texcoords.emplace_back(vec2(coord, 0.5));
            }
            drawable->update_vertex_buffer(d_points);
            drawable->update_texcoord_buffer(d_texcoords);
            drawable->release_element_buffer();
        }


        template <typename FT>
        inline void update_buffer(SurfaceMesh* model, LinesDrawable* drawable, SurfaceMesh::VertexProperty<FT> prop) {
            float min_value = std::numeric_limits<FT>::max();
            float max_value = -std::numeric_limits<FT>::max();
            for (auto v : model->vertices()) {
                float value = prop[v];
                min_value = std::min(value, min_value);
                max_value = std::max(value, max_value);
            }

            auto points = model->get_vertex_property<vec3>("v:point");
            drawable->update_vertex_buffer(points.vector());

            std::vector<vec2> d_texcoords;
            d_texcoords.reserve(model->n_vertices());
            for (auto v : model->vertices()) {
                float coord = (prop[v] - min_value) / (max_value - min_value);
                d_texcoords.emplace_back(vec2(coord, 0.5));
            }
            drawable->update_texcoord_buffer(d_texcoords);

            std::vector<unsigned int> indices;
            indices.reserve(model->n_edges() * 2);
            for (auto e : model->edges()) {
                auto s = model->vertex(e, 0);
                auto t = model->vertex(e, 1);
                indices.push_back(s.idx());
                indices.push_back(t.idx());
            }
            drawable->update_index_buffer(indices);
        }


        template <typename FT>
        inline void update_buffer(PointCloud* model, PointsDrawable* drawable, PointCloud::VertexProperty<FT> prop) {
            float min_value = std::numeric_limits<FT>::max();
            float max_value = -std::numeric_limits<FT>::max();
            for (auto v : model->vertices()) {
                float value = prop[v];
                min_value = std::min(value, min_value);
                max_value = std::max(value, max_value);
            }

            auto points = model->get_vertex_property<vec3>("v:point");
            std::vector<vec2> d_texcoords;  d_texcoords.reserve(model->n_vertices());
            for (auto v : model->vertices()) {
                float coord = (prop[v] - min_value) / (max_value - min_value);
                d_texcoords.emplace_back(vec2(coord, 0.5));
            }
            drawable->update_vertex_buffer(points.vector());
            drawable->update_texcoord_buffer(d_texcoords);
        }


        template <typename FT>
        inline void update_buffer(Graph* model, PointsDrawable* drawable, Graph::VertexProperty<FT> prop) {
            float min_value = std::numeric_limits<FT>::max();
            float max_value = -std::numeric_limits<FT>::max();
            for (auto v : model->vertices()) {
                float value = prop[v];
                min_value = std::min(value, min_value);
                max_value = std::max(value, max_value);
            }

            auto points = model->get_vertex_property<vec3>("v:point");
            std::vector<vec2> d_texcoords;  d_texcoords.reserve(model->n_vertices());
            for (auto v : model->vertices()) {
                float coord = (prop[v] - min_value) / (max_value - min_value);
                d_texcoords.emplace_back(vec2(coord, 0.5));
            }
            drawable->update_vertex_buffer(points.vector());
            drawable->update_texcoord_buffer(d_texcoords);
        }


        template <typename FT>
        inline void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::FaceProperty<FT> fscalar) {
            assert(model);
            assert(drawable);
            assert(fscalar);

            /**
             * We use the Tessellator to eliminate duplicated vertices. This allows us to take advantage of index buffer
             * to minimize the number of vertices sent to the GPU.
             */
            Tessellator tessellator;

            /**
             * For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a unified
             * rendering APIs. Thus for performance reasons, the selection of polygonal faces is also internally
             * implemented by selecting triangle primitives using program shaders. This allows data uploaded to the GPU
             * for the rendering purpose be shared for selection. Yeah, performance gain!
             */
            auto triangle_range = model->face_property<std::pair<int, int> >("f:triangle_range");
            int count_triangles = 0;

            /**
             * Efficiency in switching between flat and smooth shading.
             * Easy3d always transfer vertex normals to GPU and the normals for flat shading are computed on the fly in
             * the fragment shader:
             *          normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
             * Then, by adding a boolean uniform 'smooth_shading' to the fragment shader, client code can easily switch
             * between flat and smooth shading without transferring different data to the GPU.
             */

            auto points = model->get_vertex_property<vec3>("v:point");
            model->update_vertex_normals();
            auto normals = model->get_vertex_property<vec3>("v:normal");

            float min_value = std::numeric_limits<FT>::max();
            float max_value = -std::numeric_limits<FT>::max();
            for (auto f : model->faces()) {
                float value = fscalar[f];
                min_value = std::min(value, min_value);
                max_value = std::max(value, max_value);
            }

            for (auto face : model->faces()) {
                tessellator.begin_polygon(model->compute_face_normal(face));
                tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                tessellator.begin_contour();
                float coord = (fscalar[face] - min_value) / (max_value - min_value);

                for (auto h : model->halfedges(face)) {
                    Tessellator::Vertex vertex;
                    auto v = model->to_vertex(h);
                    vertex.append(points[v]);
                    vertex.append(normals[v]);
                    vertex.append(vec2(coord, 0.5));
                    tessellator.add_vertex(vertex);
                }
                tessellator.end_contour();
                tessellator.end_polygon();

                std::size_t num = tessellator.num_triangles_in_last_polygon();
                triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                count_triangles += num;
            }

            std::vector<vec3> d_points, d_normals;
            std::vector<vec2> d_texcoords;
            const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
            for (auto v :vts) {
                std::size_t offset = 0;
                d_points.emplace_back(v->data() + offset);
                offset += 3;
                d_normals.emplace_back(v->data() + offset);
                offset += 3;
                d_texcoords.emplace_back(v->data() + offset);
            }

            const std::vector<unsigned int> &d_indices = tessellator.indices();

            drawable->update_vertex_buffer(d_points);
            drawable->update_index_buffer(d_indices);
            drawable->update_normal_buffer(d_normals);
            drawable->update_texcoord_buffer(d_texcoords);
            DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->n_vertices() << "/" << d_points.size();
        }


        template <typename FT>
        inline void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::VertexProperty<FT> vscalar) {
            assert(model);
            assert(drawable);
            assert(vscalar);

            /**
             * We use the Tessellator to eliminate duplicated vertices. This allows us to take advantage of index buffer
             * to minimize the number of vertices sent to the GPU.
             */
            Tessellator tessellator;

            /**
             * For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a unified
             * rendering APIs. Thus for performance reasons, the selection of polygonal faces is also internally
             * implemented by selecting triangle primitives using program shaders. This allows data uploaded to the GPU
             * for the rendering purpose be shared for selection. Yeah, performance gain!
             */
            auto triangle_range = model->face_property<std::pair<int, int> >("f:triangle_range");
            int count_triangles = 0;

            /**
             * Efficiency in switching between flat and smooth shading.
             * Easy3d always transfer vertex normals to GPU and the normals for flat shading are computed on the fly in
             * the fragment shader:
             *          normal = normalize(cross(dFdx(DataIn.position), dFdy(DataIn.position)));
             * Then, by adding a boolean uniform 'smooth_shading' to the fragment shader, client code can easily switch
             * between flat and smooth shading without transferring different data to the GPU.
             */

            auto points = model->get_vertex_property<vec3>("v:point");
            model->update_vertex_normals();
            auto normals = model->get_vertex_property<vec3>("v:normal");

            float min_value = std::numeric_limits<FT>::max();
            float max_value = -std::numeric_limits<FT>::max();
            for (auto v : model->vertices()) {
                float value = vscalar[v];
                min_value = std::min(value, min_value);
                max_value = std::max(value, max_value);
            }

            for (auto face : model->faces()) {
                tessellator.begin_polygon(model->compute_face_normal(face));
                tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                tessellator.begin_contour();
                for (auto h : model->halfedges(face)) {
                    Tessellator::Vertex vertex;
                    auto v = model->to_vertex(h);
                    vertex.append(points[v]);
                    vertex.append(normals[v]);

                    float coord = (vscalar[v] - min_value) / (max_value - min_value);
                    vertex.append(vec2(coord, 0.5));
                    tessellator.add_vertex(vertex);
                }
                tessellator.end_contour();
                tessellator.end_polygon();

                std::size_t num = tessellator.num_triangles_in_last_polygon();
                triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                count_triangles += num;
            }

            std::vector<vec3> d_points, d_normals;
            std::vector<vec2> d_texcoords;
            const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
            for (auto v :vts) {
                std::size_t offset = 0;
                d_points.emplace_back(v->data() + offset);
                offset += 3;
                d_normals.emplace_back(v->data() + offset);
                offset += 3;
                d_texcoords.emplace_back(v->data() + offset);
            }

            const std::vector<unsigned int> &d_indices = tessellator.indices();

            drawable->update_vertex_buffer(d_points);
            drawable->update_index_buffer(d_indices);
            drawable->update_normal_buffer(d_normals);
            drawable->update_texcoord_buffer(d_texcoords);

            DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->n_vertices() << "/" << d_points.size();
        }


        template <typename FT>
        void update_buffer(Graph *model, LinesDrawable *drawable, Graph::EdgeProperty<FT> prop) {
            float min_value = std::numeric_limits<FT>::max();
            float max_value = -std::numeric_limits<FT>::max();
            for (auto e : model->edges()) {
                float value = prop[e];
                min_value = std::min(value, min_value);
                max_value = std::max(value, max_value);
            }

            auto points = model->get_vertex_property<vec3>("v:point");
            std::vector<vec3> d_points;
            d_points.reserve(model->n_edges() * 2);
            std::vector<vec2> d_texcoords;
            d_texcoords.reserve(model->n_edges() * 2);
            for (auto e : model->edges()) {
                Graph::Vertex s = model->from_vertex(e);
                Graph::Vertex t = model->to_vertex(e);
                d_points.push_back(points[s]);
                d_points.push_back(points[t]);
                float coord_s = (prop[e] - min_value) / (max_value - min_value);
                d_texcoords.emplace_back(vec2(coord_s, 0.5));
                float coord_t = (prop[e] - min_value) / (max_value - min_value);
                d_texcoords.emplace_back(vec2(coord_t, 0.5));
            }
            drawable->update_vertex_buffer(d_points);
            drawable->update_texcoord_buffer(d_texcoords);
            drawable->release_element_buffer();
            drawable->set_impostor_type(LinesDrawable::CYLINDER);
        }


        template <typename FT>
        void update_buffer(Graph* model, LinesDrawable* drawable, Graph::VertexProperty<FT> prop) {
            float min_value = std::numeric_limits<FT>::max();
            float max_value = -std::numeric_limits<FT>::max();
            for (auto v : model->vertices()) {
                float value = prop[v];
                min_value = std::min(value, min_value);
                max_value = std::max(value, max_value);
            }

            auto points = model->get_vertex_property<vec3>("v:point");
            drawable->update_vertex_buffer(points.vector());

            std::vector<vec2> d_texcoords;
            d_texcoords.reserve(model->n_vertices());
            for (auto v : model->vertices()) {
                float coord = (prop[v] - min_value) / (max_value - min_value);
                d_texcoords.emplace_back(vec2(coord, 0.5));
            }
            drawable->update_texcoord_buffer(d_texcoords);

            std::vector<unsigned int> indices;
            indices.reserve(model->n_edges() * 2);
            for (auto e : model->edges()) {
                auto s = model->from_vertex(e);
                auto t = model->to_vertex(e);
                indices.push_back(s.idx());
                indices.push_back(t.idx());
            }
            drawable->update_index_buffer(indices);
            drawable->set_impostor_type(LinesDrawable::CYLINDER);
        }
    }
}