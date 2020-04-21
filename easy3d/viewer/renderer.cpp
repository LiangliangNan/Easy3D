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


#include <easy3d/viewer/renderer.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/texture_manager.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/viewer/tessellator.h>
#include <easy3d/core/random.h>

#include <algorithm>


namespace easy3d {

    namespace renderer {


        namespace details {

            // clamps scalar field values by the percentages specified by dummy_lower and dummy_upper.
            // min_value and max_value return the expected value range.
            template<typename FT>
            inline void
            clamp_scalar_field(const std::vector<FT> &property, float &min_value, float &max_value, float dummy_lower,
                               float dummy_upper) {
                // sort curvature values
                std::vector<FT> values = property;;
                std::sort(values.begin(), values.end());

                std::size_t n = values.size() - 1;
                std::size_t index_lower = n * dummy_lower;
                std::size_t index_upper = n - n * dummy_upper - 1;
                min_value = values[index_lower];
                max_value = values[index_upper];

                const int lower = static_cast<int>(dummy_lower * 100);
                const int upper = static_cast<int>(dummy_upper * 100);
                if (lower > 0 || upper > 0) {
                    LOG(INFO) << "scalar field clamped by "
                              << lower << "% (lower) and "
                              << upper << "% (upper). new range: [" << min_value << ", " << max_value << "]";
                }
            }

            template <typename FT>
            inline void update_buffer(PointCloud* model, PointsDrawable* drawable, PointCloud::VertexProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                const auto& scheme = drawable->color_scheme();
                const float dummy_lower = (scheme.clamp_value ? scheme.dummy_lower : 0.0f);
                const float dummy_upper = (scheme.clamp_value ? scheme.dummy_upper : 0.0f);
                float min_value = std::numeric_limits<float>::max();
                float max_value = -std::numeric_limits<float>::max();
                details::clamp_scalar_field(prop.vector(), min_value, max_value, dummy_lower, dummy_upper);

                auto points = model->get_vertex_property<vec3>("v:point");
                std::vector<vec2> d_texcoords;  d_texcoords.reserve(model->n_vertices());
                for (auto v : model->vertices()) {
                    float coord = (prop[v] - min_value) / (max_value - min_value);
                    d_texcoords.emplace_back(vec2(coord, 0.5f));
                }
                drawable->update_vertex_buffer(points.vector());
                drawable->update_texcoord_buffer(d_texcoords);

                drawable->set_use_texture(true);
            }


            template<typename FT>
            inline void
            update_buffer(SurfaceMesh *model, PointsDrawable *drawable, SurfaceMesh::VertexProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                const auto& scheme = drawable->color_scheme();
                const float dummy_lower = (scheme.clamp_value ? scheme.dummy_lower : 0.0f);
                const float dummy_upper = (scheme.clamp_value ? scheme.dummy_upper : 0.0f);
                float min_value = std::numeric_limits<float>::max();
                float max_value = -std::numeric_limits<float>::max();
                details::clamp_scalar_field(prop.vector(), min_value, max_value, dummy_lower, dummy_upper);

                auto points = model->get_vertex_property<vec3>("v:point");

                std::vector<vec2> d_texcoords;
                d_texcoords.reserve(model->n_vertices());
                for (auto v : model->vertices()) {
                    float coord = (prop[v] - min_value) / (max_value - min_value);
                    d_texcoords.emplace_back(vec2(coord, 0.5f));
                }
                drawable->update_vertex_buffer(points.vector());
                drawable->update_texcoord_buffer(d_texcoords);

                drawable->set_use_texture(true);
            }


            template<typename FT>
            inline void update_buffer(SurfaceMesh *model, LinesDrawable *drawable, SurfaceMesh::EdgeProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                const auto& scheme = drawable->color_scheme();
                const float dummy_lower = (scheme.clamp_value ? scheme.dummy_lower : 0.0f);
                const float dummy_upper = (scheme.clamp_value ? scheme.dummy_upper : 0.0f);
                float min_value = std::numeric_limits<float>::max();
                float max_value = -std::numeric_limits<float>::max();
                details::clamp_scalar_field(prop.vector(), min_value, max_value, dummy_lower, dummy_upper);

                auto points = model->get_vertex_property<vec3>("v:point");
                std::vector<vec3> d_points;
                d_points.reserve(model->n_edges() * 2);
                std::vector<vec2> d_texcoords;
                d_texcoords.reserve(model->n_edges() * 2);
                for (auto e : model->edges()) {
                    SurfaceMesh::Vertex s = model->vertex(e, 0);
                    SurfaceMesh::Vertex t = model->vertex(e, 1);
                    d_points.push_back(points[s]);
                    d_points.push_back(points[t]);
                    float coord = (prop[e] - min_value) / (max_value - min_value);
                    d_texcoords.emplace_back(vec2(coord, 0.5f));
                    d_texcoords.emplace_back(vec2(coord, 0.5f));
                }
                drawable->update_vertex_buffer(d_points);
                drawable->update_texcoord_buffer(d_texcoords);
                drawable->release_element_buffer();

                drawable->set_use_texture(true);
            }


            template<typename FT>
            inline void update_buffer(SurfaceMesh *model, LinesDrawable *drawable, SurfaceMesh::VertexProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                const auto& scheme = drawable->color_scheme();
                const float dummy_lower = (scheme.clamp_value ? scheme.dummy_lower : 0.0f);
                const float dummy_upper = (scheme.clamp_value ? scheme.dummy_upper : 0.0f);
                float min_value = std::numeric_limits<float>::max();
                float max_value = -std::numeric_limits<float>::max();
                details::clamp_scalar_field(prop.vector(), min_value, max_value, dummy_lower, dummy_upper);

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());

                std::vector<vec2> d_texcoords;
                d_texcoords.reserve(model->n_vertices());
                for (auto v : model->vertices()) {
                    float coord = (prop[v] - min_value) / (max_value - min_value);
                    d_texcoords.emplace_back(vec2(coord, 0.5f));
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

                drawable->set_use_texture(true);
            }


            template<typename FT>
            inline void
            update_buffer(Graph *model, PointsDrawable *drawable, Graph::VertexProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                const auto& scheme = drawable->color_scheme();
                const float dummy_lower = (scheme.clamp_value ? scheme.dummy_lower : 0.0f);
                const float dummy_upper = (scheme.clamp_value ? scheme.dummy_upper : 0.0f);
                float min_value = std::numeric_limits<float>::max();
                float max_value = -std::numeric_limits<float>::max();
                details::clamp_scalar_field(prop.vector(), min_value, max_value, dummy_lower, dummy_upper);

                auto points = model->get_vertex_property<vec3>("v:point");
                std::vector<vec2> d_texcoords;
                d_texcoords.reserve(model->n_vertices());
                for (auto v : model->vertices()) {
                    float coord = (prop[v] - min_value) / (max_value - min_value);
                    d_texcoords.emplace_back(vec2(coord, 0.5f));
                }
                drawable->update_vertex_buffer(points.vector());
                drawable->update_texcoord_buffer(d_texcoords);

                drawable->set_use_texture(true);
            }


            template<typename FT>
            inline void
            update_buffer(SurfaceMesh *model, TrianglesDrawable *drawable, SurfaceMesh::FaceProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

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

                const auto& scheme = drawable->color_scheme();
                const float dummy_lower = (scheme.clamp_value ? scheme.dummy_lower : 0.0f);
                const float dummy_upper = (scheme.clamp_value ? scheme.dummy_upper : 0.0f);
                float min_value = std::numeric_limits<float>::max();
                float max_value = -std::numeric_limits<float>::max();
                details::clamp_scalar_field(prop.vector(), min_value, max_value, dummy_lower, dummy_upper);

                for (auto face : model->faces()) {
                    tessellator.begin_polygon(model->compute_face_normal(face));
                    tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                    tessellator.begin_contour();
                    float coord = (prop[face] - min_value) / (max_value - min_value);

                    for (auto h : model->halfedges(face)) {
                        auto v = model->to_vertex(h);
                        Tessellator::Vertex vertex(points[v]);
                        vertex.append(normals[v]);
                        vertex.append(vec2(coord, 0.5f));
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

                drawable->set_use_texture(true);

                DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->n_vertices() << "/" << d_points.size();
            }


            template<typename FT>
            inline void
            update_buffer(SurfaceMesh *model, TrianglesDrawable *drawable, SurfaceMesh::VertexProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

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

                const auto& scheme = drawable->color_scheme();
                const float dummy_lower = (scheme.clamp_value ? scheme.dummy_lower : 0.0f);
                const float dummy_upper = (scheme.clamp_value ? scheme.dummy_upper : 0.0f);
                float min_value = std::numeric_limits<float>::max();
                float max_value = -std::numeric_limits<float>::max();
                details::clamp_scalar_field(prop.vector(), min_value, max_value, dummy_lower, dummy_upper);

                for (auto face : model->faces()) {
                    tessellator.begin_polygon(model->compute_face_normal(face));
                    tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                    tessellator.begin_contour();
                    for (auto h : model->halfedges(face)) {
                        auto v = model->to_vertex(h);
                        Tessellator::Vertex vertex(points[v]);
                        vertex.append(normals[v]);

                        float coord = (prop[v] - min_value) / (max_value - min_value);
                        vertex.append(vec2(coord, 0.5f));
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

                drawable->set_use_texture(true);

                DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->n_vertices() << "/" << d_points.size();
            }


            template<typename FT>
            void update_buffer(Graph *model, LinesDrawable *drawable, Graph::EdgeProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                const auto& scheme = drawable->color_scheme();
                const float dummy_lower = (scheme.clamp_value ? scheme.dummy_lower : 0.0f);
                const float dummy_upper = (scheme.clamp_value ? scheme.dummy_upper : 0.0f);
                float min_value = std::numeric_limits<float>::max();
                float max_value = -std::numeric_limits<float>::max();
                details::clamp_scalar_field(prop.vector(), min_value, max_value, dummy_lower, dummy_upper);

                auto points = model->get_vertex_property<vec3>("v:point");
                std::vector<vec3> d_points;
                d_points.reserve(model->n_edges() * 2);
                std::vector<vec2> d_texcoords;
                d_texcoords.reserve(model->n_edges() * 2);
                for (auto e : model->edges()) {
                    Graph::Vertex s = model->vertex(e, 0);
                    Graph::Vertex t = model->vertex(e, 1);
                    d_points.push_back(points[s]);
                    d_points.push_back(points[t]);
                    float coord_s = (prop[e] - min_value) / (max_value - min_value);
                    d_texcoords.emplace_back(vec2(coord_s, 0.5f));
                    float coord_t = (prop[e] - min_value) / (max_value - min_value);
                    d_texcoords.emplace_back(vec2(coord_t, 0.5f));
                }
                drawable->update_vertex_buffer(d_points);
                drawable->update_texcoord_buffer(d_texcoords);
                drawable->release_element_buffer();
                drawable->set_impostor_type(LinesDrawable::CYLINDER);

                drawable->set_use_texture(true);
            }


            template<typename FT>
            void update_buffer(Graph *model, LinesDrawable *drawable, Graph::VertexProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                const auto& scheme = drawable->color_scheme();
                const float dummy_lower = (scheme.clamp_value ? scheme.dummy_lower : 0.0f);
                const float dummy_upper = (scheme.clamp_value ? scheme.dummy_upper : 0.0f);
                float min_value = std::numeric_limits<float>::max();
                float max_value = -std::numeric_limits<float>::max();
                details::clamp_scalar_field(prop.vector(), min_value, max_value, dummy_lower, dummy_upper);

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());

                std::vector<vec2> d_texcoords;
                d_texcoords.reserve(model->n_vertices());
                for (auto v : model->vertices()) {
                    float coord = (prop[v] - min_value) / (max_value - min_value);
                    d_texcoords.emplace_back(vec2(coord, 0.5f));
                }
                drawable->update_texcoord_buffer(d_texcoords);

                std::vector<unsigned int> indices;
                indices.reserve(model->n_edges() * 2);
                for (auto e : model->edges()) {
                    Graph::Vertex s = model->vertex(e, 0);
                    Graph::Vertex t = model->vertex(e, 1);
                    indices.push_back(s.idx());
                    indices.push_back(t.idx());
                }
                drawable->update_index_buffer(indices);
                drawable->set_impostor_type(LinesDrawable::CYLINDER);

                drawable->set_use_texture(true);
            }


            void update_buffer(PointCloud* model, PointsDrawable* drawable, PointCloud::VertexProperty<vec2> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());
                auto normals = model->get_vertex_property<vec3>("v:normal");
                if (normals)
                    drawable->update_normal_buffer(normals.vector());

                drawable->update_texcoord_buffer(prop.vector());
            }


            void update_buffer(PointCloud* model, PointsDrawable* drawable, PointCloud::VertexProperty<vec3> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());
                auto normals = model->get_vertex_property<vec3>("v:normal");
                if (normals)
                    drawable->update_normal_buffer(normals.vector());

                drawable->update_color_buffer(prop.vector());
            }


            void update_buffer(SurfaceMesh* model, PointsDrawable* drawable, SurfaceMesh::VertexProperty<vec3> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());
                drawable->update_color_buffer(prop.vector());
            }


            void update_buffer(SurfaceMesh* model, PointsDrawable* drawable, SurfaceMesh::VertexProperty<vec2> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());
                drawable->update_texcoord_buffer(prop.vector());
            }


            void update_buffer(SurfaceMesh *model, TrianglesDrawable *drawable) {
                assert(model);
                assert(drawable);

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

                for (auto face : model->faces()) {
                    tessellator.begin_polygon(model->compute_face_normal(face));
                    // tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                    tessellator.begin_contour();
                    for (auto h : model->halfedges(face)) {
                        auto v = model->to_vertex(h);
                        Tessellator::Vertex vertex(points[v]);
                        vertex.append(normals[v]);
                        tessellator.add_vertex(vertex);
                    }
                    tessellator.end_contour();
                    tessellator.end_polygon();

                    std::size_t num = tessellator.num_triangles_in_last_polygon();
                    triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                    count_triangles += num;
                }

                std::vector<vec3> d_points, d_normals;
                const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
                for (auto v :vts) {
                    std::size_t offset = 0;
                    d_points.emplace_back(v->data() + offset);
                    offset += 3;
                    d_normals.emplace_back(v->data() + offset);
                }

                const std::vector<unsigned int> &indices = tessellator.indices();

                drawable->update_vertex_buffer(d_points);
                drawable->update_index_buffer(indices);
                drawable->update_normal_buffer(d_normals);

                DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->n_vertices() << "/" << d_points.size();
            }

            // with a per-face color
            void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::FaceProperty<vec3> fcolor) {
                assert(model);
                assert(drawable);
                assert(fcolor);

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

                for (auto face : model->faces()) {
                    tessellator.begin_polygon(model->compute_face_normal(face));
                    // tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                    tessellator.begin_contour();
                    const vec3& color = fcolor[face];
                    for (auto h : model->halfedges(face)) {
                        auto v = model->to_vertex(h);
                        Tessellator::Vertex vertex(points[v]);
                        vertex.append(normals[v]);
                        vertex.append(color);
                        tessellator.add_vertex(vertex);
                    }
                    tessellator.end_contour();
                    tessellator.end_polygon();

                    std::size_t num = tessellator.num_triangles_in_last_polygon();
                    triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                    count_triangles += num;
                }

                std::vector<vec3> d_points, d_normals, d_colors;
                const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
                for (auto v :vts) {
                    std::size_t offset = 0;
                    d_points.emplace_back(v->data() + offset);
                    offset += 3;
                    d_normals.emplace_back(v->data() + offset);
                    offset += 3;
                    d_colors.emplace_back(v->data() + offset);
                }

                const std::vector<unsigned int> &d_indices = tessellator.indices();

                drawable->update_vertex_buffer(d_points);
                drawable->update_index_buffer(d_indices);
                drawable->update_normal_buffer(d_normals);
                drawable->update_color_buffer(d_colors);

                DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->n_vertices() << "/" << d_points.size();
            }


            void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::VertexProperty<vec3> vcolor) {
                assert(model);
                assert(drawable);
                assert(vcolor);

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

                for (auto face : model->faces()) {
                    tessellator.begin_polygon(model->compute_face_normal(face));
                    // tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                    tessellator.begin_contour();
                    for (auto h : model->halfedges(face)) {
                        auto v = model->to_vertex(h);
                        Tessellator::Vertex vertex(points[v]);
                        vertex.append(normals[v]);
                        vertex.append(vcolor[v]);
                        tessellator.add_vertex(vertex);
                    }
                    tessellator.end_contour();
                    tessellator.end_polygon();

                    std::size_t num = tessellator.num_triangles_in_last_polygon();
                    triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                    count_triangles += num;
                }

                std::vector<vec3> d_points, d_normals, d_colors;
                const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
                for (auto v :vts) {
                    std::size_t offset = 0;
                    d_points.emplace_back(v->data() + offset);
                    offset += 3;
                    d_normals.emplace_back(v->data() + offset);
                    offset += 3;
                    d_colors.emplace_back(v->data() + offset);
                }

                const std::vector<unsigned int> &d_indices = tessellator.indices();

                drawable->update_vertex_buffer(d_points);
                drawable->update_index_buffer(d_indices);
                drawable->update_normal_buffer(d_normals);
                drawable->update_color_buffer(d_colors);

                DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->n_vertices() << "/" << d_points.size();
            }


            void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::VertexProperty<vec2> vtexcoords) {
                assert(model);
                assert(drawable);
                assert(vtexcoords);

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

                for (auto face : model->faces()) {
                    tessellator.begin_polygon(model->compute_face_normal(face));
                    // tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                    tessellator.begin_contour();
                    for (auto h : model->halfedges(face)) {
                        auto v = model->to_vertex(h);
                        Tessellator::Vertex vertex(points[v]);
                        vertex.append(normals[v]);
                        vertex.append(vtexcoords[v]);
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

#if 0 // Model has texture coordinates, should we put a default texture?
                const std::string texture_file = resource::directory() + "/textures/checkerboard_gray.png";
            auto tex = TextureManager::request(texture_file, GL_REPEAT);
            drawable->set_texture(tex);
            drawable->set_texture_repeat(10);
            drawable->set_use_texture(true);
#endif

                DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->n_vertices() << "/" << d_points.size();
            }


            void update_buffer(SurfaceMesh* model, TrianglesDrawable* drawable, SurfaceMesh::HalfedgeProperty<vec2> htexcoords) {
                assert(model);
                assert(drawable);
                assert(htexcoords);

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

                for (auto face : model->faces()) {
                    tessellator.begin_polygon(model->compute_face_normal(face));
                    // tessellator.set_winding_rule(Tessellator::NONZERO);  // or POSITIVE
                    tessellator.begin_contour();
                    for (auto h : model->halfedges(face)) {
                        auto v = model->to_vertex(h);
                        Tessellator::Vertex vertex(points[v]);
                        vertex.append(normals[v]);
                        vertex.append(htexcoords[h]);
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

#if 0 // Model has texture coordinates, should we put a default texture?
                const std::string texture_file = resource::directory() + "/textures/checkerboard_gray.png";
            auto tex = TextureManager::request(texture_file, GL_REPEAT);
            drawable->set_texture(tex);
            drawable->set_texture_repeat(10);
            drawable->set_use_texture(true);
#endif

                DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->n_vertices() << "/" << d_points.size();
            }


            void update_buffer(SurfaceMesh* model, LinesDrawable* drawable, SurfaceMesh::EdgeProperty<vec3> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                auto points = model->get_vertex_property<vec3>("v:point");
                std::vector<vec3> d_points, d_colors;
                d_points.reserve(model->n_edges() * 2);
                d_colors.reserve(model->n_edges() * 2);
                for (auto e : model->edges()) {
                    SurfaceMesh::Vertex s = model->vertex(e, 0);
                    SurfaceMesh::Vertex t = model->vertex(e, 1);
                    d_points.push_back(points[s]);
                    d_points.push_back(points[t]);
                    d_colors.push_back(prop[e]);
                    d_colors.push_back(prop[e]);
                }
                drawable->update_vertex_buffer(d_points);
                drawable->update_color_buffer(d_colors);
                drawable->release_element_buffer();
            }


            void update_buffer(SurfaceMesh* model, LinesDrawable* drawable, SurfaceMesh::VertexProperty<vec3> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                auto points = model->get_vertex_property<vec3>("v:point");
                std::vector<vec3> d_points, d_colors;
                d_points.reserve(model->n_edges() * 2);
                d_colors.reserve(model->n_edges() * 2);
                for (auto e : model->edges()) {
                    SurfaceMesh::Vertex s = model->vertex(e, 0);
                    SurfaceMesh::Vertex t = model->vertex(e, 1);
                    d_points.push_back(points[s]);
                    d_points.push_back(points[t]);
                    d_colors.push_back(prop[s]);
                    d_colors.push_back(prop[t]);
                }
                drawable->update_vertex_buffer(d_points);
                drawable->update_color_buffer(d_colors);
                drawable->release_element_buffer();
            }


            void update_buffer(SurfaceMesh* model, LinesDrawable* drawable, SurfaceMesh::VertexProperty<vec2> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                auto points = model->get_vertex_property<vec3>("v:point");
                std::vector<vec3> d_points;     d_points.reserve(model->n_edges() * 2);
                std::vector<vec2> d_texcoords;  d_points.reserve(model->n_edges() * 2);
                for (auto e : model->edges()) {
                    SurfaceMesh::Vertex s = model->vertex(e, 0);
                    SurfaceMesh::Vertex t = model->vertex(e, 1);
                    d_points.push_back(points[s]);
                    d_points.push_back(points[t]);
                    d_texcoords.push_back(prop[s]);
                    d_texcoords.push_back(prop[t]);
                }
                drawable->update_vertex_buffer(d_points);
                drawable->update_texcoord_buffer(d_texcoords);
                drawable->release_element_buffer();
            }


            void update_buffer(SurfaceMesh* model, LinesDrawable* drawable, SurfaceMesh::EdgeProperty<vec2> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                auto points = model->get_vertex_property<vec3>("v:point");
                std::vector<vec3> d_points;     d_points.reserve(model->n_edges() * 2);
                std::vector<vec2> d_texcoords;  d_points.reserve(model->n_edges() * 2);
                for (auto e : model->edges()) {
                    SurfaceMesh::Vertex s = model->vertex(e, 0);
                    SurfaceMesh::Vertex t = model->vertex(e, 1);
                    d_points.push_back(points[s]);
                    d_points.push_back(points[t]);
                    d_texcoords.push_back(prop[e]);
                    d_texcoords.push_back(prop[e]);
                }
                drawable->update_vertex_buffer(d_points);
                drawable->update_texcoord_buffer(d_texcoords);
                drawable->release_element_buffer();
            }


            void update_buffer(Graph* model, PointsDrawable* drawable, Graph::VertexProperty<vec3> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());
                drawable->update_color_buffer(prop.vector());
                drawable->set_impostor_type(PointsDrawable::SPHERE);
            }


            void update_buffer(Graph* model, PointsDrawable* drawable, Graph::VertexProperty<vec2> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());
                drawable->update_texcoord_buffer(prop.vector());
                drawable->set_impostor_type(PointsDrawable::SPHERE);
            }


            void update_buffer(Graph* model, LinesDrawable* drawable, Graph::EdgeProperty<vec3> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                auto points = model->get_vertex_property<vec3>("v:point");
                std::vector<vec3> d_points, d_colors;
                d_points.reserve(model->n_edges() * 2);
                d_colors.reserve(model->n_edges() * 2);
                for (auto e : model->edges()) {
                    Graph::Vertex s = model->vertex(e, 0);
                    Graph::Vertex t = model->vertex(e, 1);
                    d_points.push_back(points[s]);
                    d_points.push_back(points[t]);
                    d_colors.emplace_back(prop[e]);
                    d_colors.emplace_back(prop[e]);
                }
                drawable->update_vertex_buffer(d_points);
                drawable->update_color_buffer(d_colors);
                drawable->release_element_buffer();
                drawable->set_impostor_type(LinesDrawable::CYLINDER);
            }

            void update_buffer(Graph* model, LinesDrawable* drawable, Graph::VertexProperty<vec2> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());

                drawable->update_texcoord_buffer(prop.vector());

                std::vector<unsigned int> indices;
                for (auto e : model->edges()) {
                    unsigned int s = model->vertex(e, 0).idx();
                    unsigned int t = model->vertex(e, 1).idx();
                    indices.push_back(s);
                    indices.push_back(t);
                }
                drawable->update_index_buffer(indices);
                drawable->set_impostor_type(LinesDrawable::CYLINDER);
            }


            void update_buffer(Graph* model, LinesDrawable* drawable, Graph::EdgeProperty<vec2> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                auto points = model->get_vertex_property<vec3>("v:point");
                std::vector<vec3> d_points;
                d_points.reserve(model->n_edges() * 2);
                std::vector<vec2> d_texcoords;
                d_texcoords.reserve(model->n_edges() * 2);
                for (auto e : model->edges()) {
                    Graph::Vertex s = model->vertex(e, 0);
                    Graph::Vertex t = model->vertex(e, 1);
                    d_points.push_back(points[s]);
                    d_points.push_back(points[t]);
                    d_texcoords.emplace_back(prop[e]);
                    d_texcoords.emplace_back(prop[e]);
                }
                drawable->update_vertex_buffer(d_points);
                drawable->update_texcoord_buffer(d_texcoords);
                drawable->release_element_buffer();
                drawable->set_impostor_type(LinesDrawable::CYLINDER);
            }


            void update_buffer(Graph* model, LinesDrawable* drawable, Graph::VertexProperty<vec3> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());

                drawable->update_color_buffer(prop.vector());

                std::vector<unsigned int> indices;
                for (auto e : model->edges()) {
                    unsigned int s = model->vertex(e, 0).idx();
                    unsigned int t = model->vertex(e, 1).idx();
                    indices.push_back(s);
                    indices.push_back(t);
                }
                drawable->update_index_buffer(indices);
                drawable->set_impostor_type(LinesDrawable::CYLINDER);
            }

        }



        // -------------------------------------------------------------------------------------------------------------



        void update_buffer(PointCloud *model, PointsDrawable *drawable) {
            assert(model);
            assert(drawable);

            const auto &scheme = drawable->color_scheme();
            switch (scheme.source) {
                case ColorScheme::TEXTURE: {
                    auto texcoord = model->get_vertex_property<vec2>(scheme.name);
                    if (texcoord)
                        details::update_buffer(model, drawable, texcoord);
                    else
                        LOG(WARNING) << "texcoord property not found: " << scheme.name;
                    break;
                }

                case ColorScheme::COLOR_PROPERTY: {
                    auto colors = model->get_vertex_property<vec3>(scheme.name);
                    if (colors)
                        details::update_buffer(model, drawable, colors);
                    else
                        LOG(WARNING) << "color property not found: " << scheme.name;
                    break;
                }

                case ColorScheme::SCALAR_FIELD: {
                    if (model->get_vertex_property<float>(scheme.name)) {
                        auto prop = model->get_vertex_property<float>(scheme.name);
                        details::update_buffer(model, drawable, prop);
                    } else if (model->get_vertex_property<double>(scheme.name)) {
                        auto prop = model->get_vertex_property<double>(scheme.name);
                        details::update_buffer(model, drawable, prop);
                    } else if (model->get_vertex_property<int>(scheme.name)) {
                        auto prop = model->get_vertex_property<int>(scheme.name);
                        details::update_buffer(model, drawable, prop);
                    } else if (model->get_vertex_property<unsigned int>(scheme.name)) {
                        auto prop = model->get_vertex_property<unsigned int>(scheme.name);
                        details::update_buffer(model, drawable, prop);
                    } else {
                        LOG(WARNING) << "scalar field not found: " << scheme.name;
                        return;
                    }
                    break;
                }

                default: {// uniform color
                    auto points = model->get_vertex_property<vec3>("v:point");
                    drawable->update_vertex_buffer(points.vector());
                    auto normals = model->get_vertex_property<vec3>("v:normal");
                    if (normals)
                        drawable->update_normal_buffer(normals.vector());
                    drawable->set_per_vertex_color(false);
                    drawable->set_use_texture(false);
                    drawable->set_texture(nullptr);
                    break;
                }
            }
        }


        // -------------------------------------------------------------------------------------------------------------


        void colorize_segmentation(PointCloud *model, const std::string &segmentation, const std::string &color_name) {
            // segmentation information has been stored as properties:
            //      - "v:primitive_type"  (one of PLANE, SPHERE, CYLINDER, CONE, TORUS, and UNKNOWN)
            //      - "v:primitive_index" (-1, 0, 1, 2...)
            auto primitive_index = model->get_vertex_property<int>(segmentation);
            if (primitive_index) { // model has segmentation information
                int max_index = 0;
                for (auto v : model->vertices())
                    max_index = std::max(max_index, primitive_index[v]);

                // assign each segment a unique color
                std::vector<vec3> color_table(max_index + 1);   // index starts from 0
                for (auto &c : color_table)
                    c = random_color();

                auto colors = model->vertex_property<vec3>(color_name, vec3(0, 0, 0));
                for (auto v : model->vertices()) {
                    int idx = primitive_index[v];
                    if (idx == -1)
                        colors[v] = vec3(0, 0, 0);
                    else
                        colors[v] = color_table[idx];
                }
            }
            else {
                LOG(ERROR) << "segmentation '" << segmentation << "' does not exist";
            }
        }


        // -------------------------------------------------------------------------------------------------------------


        void update_buffer(SurfaceMesh *model, PointsDrawable *drawable) {
            assert(model);
            assert(drawable);

            const auto &scheme = drawable->color_scheme();
            switch (scheme.source) {
                case ColorScheme::TEXTURE: {
                    auto texcoord = model->get_vertex_property<vec2>(scheme.name);
                    if (texcoord)
                        details::update_buffer(model, drawable, texcoord);
                    else
                        LOG(WARNING) << "texcoord property not found: " << scheme.name;
                    break;
                }

                case ColorScheme::COLOR_PROPERTY: {
                    auto colors = model->get_vertex_property<vec3>(scheme.name);
                    if (colors)
                        details::update_buffer(model, drawable, colors);
                    else
                        LOG(WARNING) << "color property not found: " << scheme.name;
                    break;
                }

                case ColorScheme::SCALAR_FIELD: {
                    if (model->get_vertex_property<float>(scheme.name)) {
                        auto prop = model->get_vertex_property<float>(scheme.name);
                        details::update_buffer(model, drawable, prop);
                    } else if (model->get_vertex_property<double>(scheme.name)) {
                        auto prop = model->get_vertex_property<double>(scheme.name);
                        details::update_buffer(model, drawable, prop);
                    } else if (model->get_vertex_property<int>(scheme.name)) {
                        auto prop = model->get_vertex_property<int>(scheme.name);
                        details::update_buffer(model, drawable, prop);
                    } else if (model->get_vertex_property<unsigned int>(scheme.name)) {
                        auto prop = model->get_vertex_property<unsigned int>(scheme.name);
                        details::update_buffer(model, drawable, prop);
                    } else {
                        LOG(WARNING) << "scalar field not found: " << scheme.name;
                        return;
                    }
                    break;
                }

                default: // uniform color
                    auto points = model->get_vertex_property<vec3>("v:point");
                    drawable->update_vertex_buffer(points.vector());
                    auto normals = model->get_vertex_property<vec3>("v:normal");
                    if (normals)
                        drawable->update_normal_buffer(normals.vector());

                    drawable->set_per_vertex_color(false);
                    drawable->set_use_texture(false);
                    drawable->set_texture(nullptr);
                    break;
            }
        }



        // -------------------------------------------------------------------------------------------------------------



        void update_buffer(SurfaceMesh *model, LinesDrawable *drawable) {
            assert(model);
            assert(drawable);

            const auto &scheme = drawable->color_scheme();
            switch (scheme.source) {
                case ColorScheme::TEXTURE: {
                    switch (scheme.location) {
                        case ColorScheme::EDGE: {
                            auto texcoord = model->get_edge_property<vec2>(scheme.name);
                            if (texcoord)
                                details::update_buffer(model, drawable, texcoord);
                            else
                                LOG(WARNING) << "texcoord property not found on edges: " << scheme.name;
                            break;
                        }
                        case ColorScheme::VERTEX: {
                            auto texcoord = model->get_vertex_property<vec2>(scheme.name);
                            if (texcoord)
                                details::update_buffer(model, drawable, texcoord);
                            else
                                LOG(WARNING) << "texcoord property not found on vertices: " << scheme.name;
                            break;
                        }
                        case ColorScheme::FACE:
                        case ColorScheme::HALFEDGE:
                            LOG(WARNING) << "should not happen" << scheme.name;
                            break;
                    }
                    break;
                }

                case ColorScheme::COLOR_PROPERTY: {
                    switch (scheme.location) {
                        case ColorScheme::EDGE: {
                            auto colors = model->get_edge_property<vec3>(scheme.name);
                            if (colors)
                                details::update_buffer(model, drawable, colors);
                            else
                                LOG(WARNING) << "color property not found: " << scheme.name;
                            break;
                        }
                        case ColorScheme::VERTEX: {
                            auto colors = model->get_vertex_property<vec3>(scheme.name);
                            if (colors)
                                details::update_buffer(model, drawable, colors);
                            else
                                LOG(WARNING) << "color property not found: " << scheme.name;
                            break;
                        }
                        case ColorScheme::FACE:
                        case ColorScheme::HALFEDGE:
                            LOG(WARNING) << "should not happen" << scheme.name;
                            break;
                    }
                    break;
                }

                case ColorScheme::SCALAR_FIELD: {
                    switch (scheme.location) {
                        case ColorScheme::EDGE: {
                            if (model->get_edge_property<float>(scheme.name)) {
                                auto prop = model->get_edge_property<float>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_edge_property<double>(scheme.name)) {
                                auto prop = model->get_edge_property<double>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_edge_property<int>(scheme.name)) {
                                auto prop = model->get_edge_property<int>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_edge_property<unsigned int>(scheme.name)) {
                                auto prop = model->get_edge_property<unsigned int>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else {
                                LOG(WARNING) << "scalar field not found on edges: " << scheme.name;
                                return;
                            }
                            break;
                        }
                        case ColorScheme::VERTEX: {
                            if (model->get_vertex_property<float>(scheme.name)) {
                                auto prop = model->get_vertex_property<float>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_vertex_property<double>(scheme.name)) {
                                auto prop = model->get_vertex_property<double>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_vertex_property<int>(scheme.name)) {
                                auto prop = model->get_vertex_property<int>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_vertex_property<unsigned int>(scheme.name)) {
                                auto prop = model->get_vertex_property<unsigned int>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else {
                                LOG(WARNING) << "scalar field not found on vertices: " << scheme.name;
                                return;
                            }
                            break;
                        }
                        case ColorScheme::FACE:
                        case ColorScheme::HALFEDGE:
                            LOG(WARNING) << "should not happen" << scheme.name;
                            break;
                    }
                    break;
                }

                default: {// uniform color
                    // if reached here, we choose a uniform color.
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
                    break;
                }
            }
        }



        // -------------------------------------------------------------------------------------------------------------



        void update_buffer(SurfaceMesh *model, TrianglesDrawable *drawable) {
            assert(model);
            assert(drawable);

            const auto &scheme = drawable->color_scheme();
            switch (scheme.source) {
                case ColorScheme::TEXTURE: {
                    switch (scheme.location) {
                        case ColorScheme::VERTEX: {
                            auto texcoord = model->get_vertex_property<vec2>(scheme.name);
                            if (texcoord)
                                details::update_buffer(model, drawable, texcoord);
                            else
                                LOG(WARNING) << "texcoord property not found on vertices: " << scheme.name;
                            break;
                        }
                        case ColorScheme::HALFEDGE: {
                            auto texcoord = model->get_halfedge_property<vec2>(scheme.name);
                            if (texcoord)
                                details::update_buffer(model, drawable, texcoord);
                            else
                                LOG(WARNING) << "texcoord property not found on halfedges: " << scheme.name;
                            break;
                        }
                        case ColorScheme::FACE:
                        case ColorScheme::EDGE:
                            LOG(WARNING) << "should not happen" << scheme.name;
                            break;
                    }
                    break;
                }

                case ColorScheme::COLOR_PROPERTY: {
                    switch (scheme.location) {
                        case ColorScheme::FACE: {
                            auto colors = model->get_face_property<vec3>(scheme.name);
                            if (colors)
                                details::update_buffer(model, drawable, colors);
                            else
                                LOG(WARNING) << "color property not found: " << scheme.name;
                            break;
                        }
                        case ColorScheme::VERTEX: {
                            auto colors = model->get_vertex_property<vec3>(scheme.name);
                            if (colors)
                                details::update_buffer(model, drawable, colors);
                            else
                                LOG(WARNING) << "color property not found: " << scheme.name;
                            break;
                        }
                        case ColorScheme::EDGE:
                        case ColorScheme::HALFEDGE:
                            LOG(WARNING) << "should not happen" << scheme.name;
                            break;
                    }
                    break;
                }

                case ColorScheme::SCALAR_FIELD: {
                    switch (scheme.location) {
                        case ColorScheme::FACE: {
                            if (model->get_face_property<float>(scheme.name)) {
                                auto prop = model->get_face_property<float>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_face_property<double>(scheme.name)) {
                                auto prop = model->get_face_property<double>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_face_property<int>(scheme.name)) {
                                auto prop = model->get_face_property<int>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_face_property<unsigned int>(scheme.name)) {
                                auto prop = model->get_face_property<unsigned int>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else {
                                LOG(WARNING) << "scalar field not found on edges: " << scheme.name;
                                return;
                            }
                            break;
                        }
                        case ColorScheme::VERTEX: {
                            if (model->get_vertex_property<float>(scheme.name)) {
                                auto prop = model->get_vertex_property<float>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_vertex_property<double>(scheme.name)) {
                                auto prop = model->get_vertex_property<double>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_vertex_property<int>(scheme.name)) {
                                auto prop = model->get_vertex_property<int>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_vertex_property<unsigned int>(scheme.name)) {
                                auto prop = model->get_vertex_property<unsigned int>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else {
                                LOG(WARNING) << "scalar field not found on vertices: " << scheme.name;
                                return;
                            }
                            break;
                        }
                        case ColorScheme::EDGE:
                        case ColorScheme::HALFEDGE:
                            LOG(WARNING) << "should not happen" << scheme.name;
                            break;
                    }
                    break;
                }

                default: {// uniform color
                    // if reached here, we choose a uniform color.
                    details::update_buffer(model, drawable);
                    break;
                }
            }
        }



        // -------------------------------------------------------------------------------------------------------------



        void update_buffer(Graph *model, PointsDrawable *drawable) {
            assert(model);
            assert(drawable);

            const auto &scheme = drawable->color_scheme();
            switch (scheme.source) {
                case ColorScheme::TEXTURE: {
                    auto texcoord = model->get_vertex_property<vec2>(scheme.name);
                    if (texcoord)
                        details::update_buffer(model, drawable, texcoord);
                    else
                        LOG(WARNING) << "texcoord property not found: " << scheme.name;
                    break;
                }

                case ColorScheme::COLOR_PROPERTY: {
                    auto colors = model->get_vertex_property<vec3>(scheme.name);
                    if (colors)
                        details::update_buffer(model, drawable, colors);
                    else
                        LOG(WARNING) << "color property not found: " << scheme.name;
                    break;
                }

                case ColorScheme::SCALAR_FIELD: {
                    if (model->get_vertex_property<float>(scheme.name)) {
                        auto prop = model->get_vertex_property<float>(scheme.name);
                        details::update_buffer(model, drawable, prop);
                    } else if (model->get_vertex_property<double>(scheme.name)) {
                        auto prop = model->get_vertex_property<double>(scheme.name);
                        details::update_buffer(model, drawable, prop);
                    } else if (model->get_vertex_property<int>(scheme.name)) {
                        auto prop = model->get_vertex_property<int>(scheme.name);
                        details::update_buffer(model, drawable, prop);
                    } else if (model->get_vertex_property<unsigned int>(scheme.name)) {
                        auto prop = model->get_vertex_property<unsigned int>(scheme.name);
                        details::update_buffer(model, drawable, prop);
                    } else {
                        LOG(WARNING) << "scalar field not found: " << scheme.name;
                        return;
                    }
                    break;
                }

                default: // uniform color
                    auto points = model->get_vertex_property<vec3>("v:point");
                    drawable->update_vertex_buffer(points.vector());

                    drawable->set_per_vertex_color(false);
                    drawable->set_use_texture(false);
                    drawable->set_texture(nullptr);
                    break;
            }

        }



        // -------------------------------------------------------------------------------------------------------------



        void update_buffer(Graph *model, LinesDrawable *drawable) {
            assert(model);
            assert(drawable);

            const auto &scheme = drawable->color_scheme();
            switch (scheme.source) {
                case ColorScheme::TEXTURE: {
                    switch (scheme.location) {
                        case ColorScheme::EDGE: {
                            auto texcoord = model->get_edge_property<vec2>(scheme.name);
                            if (texcoord)
                                details::update_buffer(model, drawable, texcoord);
                            else
                                LOG(WARNING) << "texcoord property not found on edges: " << scheme.name;
                            break;
                        }
                        case ColorScheme::VERTEX: {
                            auto texcoord = model->get_vertex_property<vec2>(scheme.name);
                            if (texcoord)
                                details::update_buffer(model, drawable, texcoord);
                            else
                                LOG(WARNING) << "texcoord property not found on vertices: " << scheme.name;
                            break;
                        }
                        case ColorScheme::FACE:
                        case ColorScheme::HALFEDGE:
                            LOG(WARNING) << "should not happen" << scheme.name;
                            break;
                    }
                    break;
                }

                case ColorScheme::COLOR_PROPERTY: {
                    switch (scheme.location) {
                        case ColorScheme::EDGE: {
                            auto colors = model->get_edge_property<vec3>(scheme.name);
                            if (colors)
                                details::update_buffer(model, drawable, colors);
                            else
                                LOG(WARNING) << "color property not found: " << scheme.name;
                            break;
                        }
                        case ColorScheme::VERTEX: {
                            auto colors = model->get_vertex_property<vec3>(scheme.name);
                            if (colors)
                                details::update_buffer(model, drawable, colors);
                            else
                                LOG(WARNING) << "color property not found: " << scheme.name;
                            break;
                        }
                        case ColorScheme::FACE:
                        case ColorScheme::HALFEDGE:
                            LOG(WARNING) << "should not happen" << scheme.name;
                            break;
                    }
                    break;
                }

                case ColorScheme::SCALAR_FIELD: {
                    switch (scheme.location) {
                        case ColorScheme::EDGE: {
                            if (model->get_edge_property<float>(scheme.name)) {
                                auto prop = model->get_edge_property<float>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_edge_property<double>(scheme.name)) {
                                auto prop = model->get_edge_property<double>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_edge_property<int>(scheme.name)) {
                                auto prop = model->get_edge_property<int>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_edge_property<unsigned int>(scheme.name)) {
                                auto prop = model->get_edge_property<unsigned int>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else {
                                LOG(WARNING) << "scalar field not found on edges: " << scheme.name;
                                return;
                            }
                            break;
                        }
                        case ColorScheme::VERTEX: {
                            if (model->get_vertex_property<float>(scheme.name)) {
                                auto prop = model->get_vertex_property<float>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_vertex_property<double>(scheme.name)) {
                                auto prop = model->get_vertex_property<double>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_vertex_property<int>(scheme.name)) {
                                auto prop = model->get_vertex_property<int>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else if (model->get_vertex_property<unsigned int>(scheme.name)) {
                                auto prop = model->get_vertex_property<unsigned int>(scheme.name);
                                details::update_buffer(model, drawable, prop);
                            } else {
                                LOG(WARNING) << "scalar field not found on vertices: " << scheme.name;
                                return;
                            }
                            break;
                        }
                        case ColorScheme::FACE:
                        case ColorScheme::HALFEDGE:
                            LOG(WARNING) << "should not happen" << scheme.name;
                            break;
                    }
                    break;
                }

                default: {// uniform color
                    // if reached here, we choose a uniform color.
                    std::vector<unsigned int> indices;
                    for (auto e : model->edges()) {
                        Graph::Vertex s = model->vertex(e, 0);
                        Graph::Vertex t = model->vertex(e, 1);
                        indices.push_back(s.idx());
                        indices.push_back(t.idx());
                    }
                    auto points = model->get_vertex_property<vec3>("v:point");
                    drawable->update_vertex_buffer(points.vector());
                    drawable->update_index_buffer(indices);
                    break;
                }
            }
        }


        // -------------------------------------------------------------------------------------------------------------



        void update_buffer(Model *model, Drawable *drawable) {
            if (dynamic_cast<PointCloud *>(model)) {
                PointCloud* cloud = dynamic_cast<PointCloud *>(model);
                switch (drawable->type()) {
                    case Drawable::DT_POINTS:
                        update_buffer(cloud, dynamic_cast<PointsDrawable *>(drawable));
                        break;
                    case Drawable::DT_LINES:
                        update_buffer(cloud, dynamic_cast<LinesDrawable *>(drawable));
                        break;
                    case Drawable::DT_TRIANGLES:
                        update_buffer(cloud, dynamic_cast<TrianglesDrawable *>(drawable));
                        break;
                }
            } else if (dynamic_cast<Graph *>(model)) {
                Graph* graph = dynamic_cast<Graph *>(model);
                switch (drawable->type()) {
                    case Drawable::DT_POINTS:
                        update_buffer(graph, dynamic_cast<PointsDrawable *>(drawable));
                        break;
                    case Drawable::DT_LINES:
                        update_buffer(graph, dynamic_cast<LinesDrawable *>(drawable));
                        break;
                    case Drawable::DT_TRIANGLES:
                        update_buffer(graph, dynamic_cast<TrianglesDrawable *>(drawable));
                        break;
                }
            } else if (dynamic_cast<SurfaceMesh *>(model)) {
                SurfaceMesh* mesh = dynamic_cast<SurfaceMesh *>(model);
                switch (drawable->type()) {
                    case Drawable::DT_POINTS:
                        update_buffer(mesh, dynamic_cast<PointsDrawable *>(drawable));
                        break;
                    case Drawable::DT_LINES:
                        update_buffer(mesh, dynamic_cast<LinesDrawable *>(drawable));
                        break;
                    case Drawable::DT_TRIANGLES:
                        update_buffer(mesh, dynamic_cast<TrianglesDrawable *>(drawable));
                        break;
                }
            }
        }
    }

}

