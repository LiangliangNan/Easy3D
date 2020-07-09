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


#include <easy3d/renderer/buffers.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/algo/tessellator.h>

#include <algorithm>


namespace easy3d {

    namespace buffers {


        namespace details {

            // clamps scalar field values by the percentages specified by dummy_lower and dummy_upper.
            // min_value and max_value return the expected value range.
            template<typename FT>
            inline void
            clamp_scalar_field(const std::vector<FT> &property, float &min_value, float &max_value, float dummy_lower_percent,
                               float dummy_upper_percent) {
                if (property.empty()) {
                    LOG(WARNING) << "empty property";
                    return;
                }

                // sort curvature values
                std::vector<FT> values = property;;
                std::sort(values.begin(), values.end());

                const std::size_t n = values.size() - 1;
                const std::size_t index_lower = n * dummy_lower_percent;
                const std::size_t index_upper = n - n * dummy_upper_percent;
                min_value = values[index_lower];
                max_value = values[index_upper];

                const int lower = static_cast<int>(dummy_lower_percent * 100);
                const int upper = static_cast<int>(dummy_upper_percent * 100);
                if (lower > 0 || upper > 0)
                    LOG(INFO) << "scalar field range ["
                              << static_cast<float>(values.front()) << ", " << static_cast<float>(values.back()) << "]"
                              << " clamped (" << lower << "%, " << upper << "%) to [" << min_value << ", " << max_value
                              << "]";
            }

            template<typename FT>
            inline void
            update(PointCloud *model, PointsDrawable *drawable, PointCloud::VertexProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                const float dummy_lower = (drawable->clamp_range() ? drawable->clamp_lower() : 0.0f);
                const float dummy_upper = (drawable->clamp_range() ? drawable->clamp_upper() : 0.0f);
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
            }


            template<typename FT>
            inline void
            update(SurfaceMesh *model, PointsDrawable *drawable, SurfaceMesh::VertexProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                const float dummy_lower = (drawable->clamp_range() ? drawable->clamp_lower() : 0.0f);
                const float dummy_upper = (drawable->clamp_range() ? drawable->clamp_upper() : 0.0f);
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
            }


            template<typename FT>
            inline void
            update(SurfaceMesh *model, LinesDrawable *drawable, SurfaceMesh::EdgeProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                const float dummy_lower = (drawable->clamp_range() ? drawable->clamp_lower() : 0.0f);
                const float dummy_upper = (drawable->clamp_range() ? drawable->clamp_upper() : 0.0f);
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
            }


            template<typename FT>
            inline void
            update(SurfaceMesh *model, LinesDrawable *drawable, SurfaceMesh::VertexProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                const float dummy_lower = (drawable->clamp_range() ? drawable->clamp_lower() : 0.0f);
                const float dummy_upper = (drawable->clamp_range() ? drawable->clamp_upper() : 0.0f);
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
                drawable->update_element_buffer(indices);
            }


            template<typename FT>
            inline void
            update(Graph *model, PointsDrawable *drawable, Graph::VertexProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                const float dummy_lower = (drawable->clamp_range() ? drawable->clamp_lower() : 0.0f);
                const float dummy_upper = (drawable->clamp_range() ? drawable->clamp_upper() : 0.0f);
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
            }


            template<typename FT>
            inline void
            update(SurfaceMesh *model, TrianglesDrawable *drawable, SurfaceMesh::FaceProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                if (model->is_triangle_mesh()) {
                    auto points = model->get_vertex_property<vec3>("v:point");
                    model->update_vertex_normals();
                    auto normals = model->get_vertex_property<vec3>("v:normal");

                    const float dummy_lower = (drawable->clamp_range() ? drawable->clamp_lower() : 0.0f);
                    const float dummy_upper = (drawable->clamp_range() ? drawable->clamp_upper() : 0.0f);
                    float min_value = std::numeric_limits<float>::max();
                    float max_value = -std::numeric_limits<float>::max();
                    details::clamp_scalar_field(prop.vector(), min_value, max_value, dummy_lower, dummy_upper);

                    std::vector<vec3> d_points, d_normals;
                    std::vector<vec2> d_texcoords;
                    d_points.reserve(model->n_faces() * 3);
                    d_normals.reserve(model->n_faces() * 3);
                    d_texcoords.reserve(model->n_faces() * 3);
                    for (auto face : model->faces()) {
                        const float coord = (prop[face] - min_value) / (max_value - min_value);
                        for (auto h : model->halfedges(face)) {
                            auto v = model->to_vertex(h);
                            d_points.emplace_back(points[v]);
                            d_normals.emplace_back(normals[v]);
                            d_texcoords.emplace_back(vec2(coord, 0.5f));
                        }
                    }

                    drawable->update_vertex_buffer(d_points);
                    drawable->update_normal_buffer(d_normals);
                    drawable->update_texcoord_buffer(d_texcoords);
                    drawable->release_element_buffer();
                } else {

                    /**
                     * We use the Tessellator to eliminate duplicated vertices. This allows us to take advantage of element
                     * buffer to minimize the number of vertices sent to the GPU.
                     */
                    Tessellator tessellator;

                    /**
                     * For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a unified
                     * rendering APIs. Thus for performance reasons, the selection of polygonal faces is also internally
                     * implemented by selecting triangle primitives using shaders. This allows data uploaded to the GPU
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

                    const float dummy_lower = (drawable->clamp_range() ? drawable->clamp_lower() : 0.0f);
                    const float dummy_upper = (drawable->clamp_range() ? drawable->clamp_upper() : 0.0f);
                    float min_value = std::numeric_limits<float>::max();
                    float max_value = -std::numeric_limits<float>::max();
                    details::clamp_scalar_field(prop.vector(), min_value, max_value, dummy_lower, dummy_upper);

                    for (auto face : model->faces()) {
                        tessellator.begin_polygon(model->compute_face_normal(face));
                        tessellator.set_winding_rule(Tessellator::WINDING_NONZERO);  // or POSITIVE
                        tessellator.begin_contour();
                        float coord = (prop[face] - min_value) / (max_value - min_value);

                        for (auto h : model->halfedges(face)) {
                            auto v = model->to_vertex(h);
                            Tessellator::Vertex vertex(points[v], v.idx());
                            vertex.append(normals[v]);
                            vertex.append(vec2(coord, 0.5f));
                            tessellator.add_vertex(vertex);
                        }
                        tessellator.end_contour();
                        tessellator.end_polygon();

                        std::size_t num = tessellator.num_elements_in_polygon();
                        triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                        count_triangles += num;
                    }

                    const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
                    std::vector<vec3> d_points, d_normals;
                    std::vector<vec2> d_texcoords;
                    d_points.reserve(vts.size());
                    d_normals.reserve(vts.size());
                    d_texcoords.reserve(vts.size());
                    for (auto v :vts) {
                        std::size_t offset = 0;
                        d_points.emplace_back(v->data() + offset);
                        offset += 3;
                        d_normals.emplace_back(v->data() + offset);
                        offset += 3;
                        d_texcoords.emplace_back(v->data() + offset);
                    }

                    const auto &d_indices = tessellator.elements();

                    drawable->update_vertex_buffer(d_points);
                    drawable->update_element_buffer(d_indices);
                    drawable->update_normal_buffer(d_normals);
                    drawable->update_texcoord_buffer(d_texcoords);

                    DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->n_vertices() << "/"
                               << d_points.size();
                }
            }


            template<typename FT>
            inline void
            update(SurfaceMesh *model, TrianglesDrawable *drawable, SurfaceMesh::VertexProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                if (model->is_triangle_mesh()) {
                    auto points = model->get_vertex_property<vec3>("v:point");
                    model->update_vertex_normals();
                    auto normals = model->get_vertex_property<vec3>("v:normal");

                    const float dummy_lower = (drawable->clamp_range() ? drawable->clamp_lower() : 0.0f);
                    const float dummy_upper = (drawable->clamp_range() ? drawable->clamp_upper() : 0.0f);
                    float min_value = std::numeric_limits<float>::max();
                    float max_value = -std::numeric_limits<float>::max();
                    details::clamp_scalar_field(prop.vector(), min_value, max_value, dummy_lower, dummy_upper);

                    std::vector<vec2> d_texcoords;
                    d_texcoords.reserve(model->n_vertices());
                    for (auto v : model->vertices()) {
                        const float coord = (prop[v] - min_value) / (max_value - min_value);
                        d_texcoords.emplace_back(vec2(coord, 0.5f));
                    }

                    std::vector<unsigned int> d_indices;
                    d_indices.reserve(model->n_faces() * 3);
                    for (auto f : model->faces()) {
                        for (auto h : model->halfedges(f))
                            d_indices.push_back(model->to_vertex(h).idx());
                    }

                    drawable->update_vertex_buffer(points.vector());
                    drawable->update_element_buffer(d_indices);
                    drawable->update_normal_buffer(normals.vector());
                    drawable->update_texcoord_buffer(d_texcoords);
                } else {
                    /**
                     * We use the Tessellator to eliminate duplicated vertices. This allows us to take advantage of element
                     * buffer to minimize the number of vertices sent to the GPU.
                     */
                    Tessellator tessellator;

                    /**
                     * For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a unified
                     * rendering APIs. Thus for performance reasons, the selection of polygonal faces is also internally
                     * implemented by selecting triangle primitives using shaders. This allows data uploaded to the GPU
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

                    const float dummy_lower = (drawable->clamp_range() ? drawable->clamp_lower() : 0.0f);
                    const float dummy_upper = (drawable->clamp_range() ? drawable->clamp_upper() : 0.0f);
                    float min_value = std::numeric_limits<float>::max();
                    float max_value = -std::numeric_limits<float>::max();
                    details::clamp_scalar_field(prop.vector(), min_value, max_value, dummy_lower, dummy_upper);

                    for (auto face : model->faces()) {
                        tessellator.begin_polygon(model->compute_face_normal(face));
                        tessellator.set_winding_rule(Tessellator::WINDING_NONZERO);  // or POSITIVE
                        tessellator.begin_contour();
                        for (auto h : model->halfedges(face)) {
                            auto v = model->to_vertex(h);
                            Tessellator::Vertex vertex(points[v], v.idx());
                            vertex.append(normals[v]);

                            float coord = (prop[v] - min_value) / (max_value - min_value);
                            vertex.append(vec2(coord, 0.5f));
                            tessellator.add_vertex(vertex);
                        }
                        tessellator.end_contour();
                        tessellator.end_polygon();

                        std::size_t num = tessellator.num_elements_in_polygon();
                        triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                        count_triangles += num;
                    }

                    const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
                    std::vector<vec3> d_points, d_normals;
                    std::vector<vec2> d_texcoords;
                    d_points.reserve(vts.size());
                    d_normals.reserve(vts.size());
                    d_texcoords.reserve(vts.size());
                    for (auto v :vts) {
                        std::size_t offset = 0;
                        d_points.emplace_back(v->data() + offset);
                        offset += 3;
                        d_normals.emplace_back(v->data() + offset);
                        offset += 3;
                        d_texcoords.emplace_back(v->data() + offset);
                    }

                    const auto &d_indices = tessellator.elements();

                    drawable->update_vertex_buffer(d_points);
                    drawable->update_element_buffer(d_indices);
                    drawable->update_normal_buffer(d_normals);
                    drawable->update_texcoord_buffer(d_texcoords);

                    DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->n_vertices() << "/"
                               << d_points.size();
                }
            }


            template<typename FT>
            void update(Graph *model, LinesDrawable *drawable, Graph::EdgeProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                const float dummy_lower = (drawable->clamp_range() ? drawable->clamp_lower() : 0.0f);
                const float dummy_upper = (drawable->clamp_range() ? drawable->clamp_upper() : 0.0f);
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
            }


            template<typename FT>
            void update(Graph *model, LinesDrawable *drawable, Graph::VertexProperty<FT> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                const float dummy_lower = (drawable->clamp_range() ? drawable->clamp_lower() : 0.0f);
                const float dummy_upper = (drawable->clamp_range() ? drawable->clamp_upper() : 0.0f);
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
                drawable->update_element_buffer(indices);
                drawable->set_impostor_type(LinesDrawable::CYLINDER);
            }


            void update(PointCloud *model, PointsDrawable *drawable, PointCloud::VertexProperty<vec2> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());
                auto normals = model->get_vertex_property<vec3>("v:normal");
                if (normals)
                    drawable->update_normal_buffer(normals.vector());

                drawable->update_texcoord_buffer(prop.vector());
            }


            void update(PointCloud *model, PointsDrawable *drawable, PointCloud::VertexProperty<vec3> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());
                auto normals = model->get_vertex_property<vec3>("v:normal");
                if (normals)
                    drawable->update_normal_buffer(normals.vector());

                drawable->update_color_buffer(prop.vector());
            }


            void update(SurfaceMesh *model, PointsDrawable *drawable, SurfaceMesh::VertexProperty<vec3> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());
                drawable->update_color_buffer(prop.vector());
            }


            void update(SurfaceMesh *model, PointsDrawable *drawable, SurfaceMesh::VertexProperty<vec2> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());
                drawable->update_texcoord_buffer(prop.vector());
            }


            void update(SurfaceMesh *model, TrianglesDrawable *drawable) {
                assert(model);
                assert(drawable);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                if (model->is_triangle_mesh()) {
                    model->update_vertex_normals();
                    auto normals = model->get_vertex_property<vec3>("v:normal");

                    std::vector<unsigned int> d_indices;
                    d_indices.reserve(model->n_faces() * 3);
                    for (auto face : model->faces()) {
                        for (auto h : model->halfedges(face))
                            d_indices.push_back(model->to_vertex(h).idx());
                    }
                    drawable->update_vertex_buffer(model->points());
                    drawable->update_element_buffer(d_indices);
                    drawable->update_normal_buffer(normals.vector());
                } else {
                    /**
                     * We use the Tessellator to eliminate duplicated vertices. This allows us to take advantage of element
                     * buffer to minimize the number of vertices sent to the GPU.
                     */
                    Tessellator tessellator;

                    /**
                     * For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a unified
                     * rendering APIs. Thus for performance reasons, the selection of polygonal faces is also internally
                     * implemented by selecting triangle primitives using shaders. This allows data uploaded to the GPU
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
                        // tessellator.set_winding_rule(Tessellator::WINDING_NONZERO);  // or POSITIVE
                        tessellator.begin_contour();
                        for (auto h : model->halfedges(face)) {
                            auto v = model->to_vertex(h);
                            Tessellator::Vertex vertex(points[v], v.idx());
                            vertex.append(normals[v]);
                            tessellator.add_vertex(vertex);
                        }
                        tessellator.end_contour();
                        tessellator.end_polygon();

                        std::size_t num = tessellator.num_elements_in_polygon();
                        triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                        count_triangles += num;
                    }

                    const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
                    std::vector<vec3> d_points, d_normals;
                    d_points.reserve(vts.size());
                    d_normals.reserve(vts.size());
                    for (auto v :vts) {
                        std::size_t offset = 0;
                        d_points.emplace_back(v->data() + offset);
                        offset += 3;
                        d_normals.emplace_back(v->data() + offset);
                    }

                    const auto &d_indices = tessellator.elements();

                    drawable->update_vertex_buffer(d_points);
                    drawable->update_element_buffer(d_indices);
                    drawable->update_normal_buffer(d_normals);

                    DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->n_vertices() << "/"
                               << d_points.size();
                }
            }

            // with a per-face color
            void update(SurfaceMesh *model, TrianglesDrawable *drawable,
                        SurfaceMesh::FaceProperty<vec3> fcolor) {
                assert(model);
                assert(drawable);
                assert(fcolor);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                if (model->is_triangle_mesh()) {
                    auto points = model->get_vertex_property<vec3>("v:point");
                    model->update_vertex_normals();
                    auto normals = model->get_vertex_property<vec3>("v:normal");

                    std::vector<vec3> d_points, d_normals, d_colors;
                    d_points.reserve(model->n_faces() * 3);
                    d_normals.reserve(model->n_faces() * 3);
                    d_colors.reserve(model->n_faces() * 3);

                    for (auto f : model->faces()) {
                        const vec3 &color = fcolor[f];
                        for (auto h : model->halfedges(f)) {
                            auto v = model->to_vertex(h);
                            d_points.push_back(points[v]);
                            d_normals.push_back(normals[v]);
                            d_colors.push_back(color);
                        }
                    }

                    drawable->update_vertex_buffer(d_points);
                    drawable->update_normal_buffer(d_normals);
                    drawable->update_color_buffer(d_colors);
                    drawable->release_element_buffer();
                } else {
                    /**
                     * We use the Tessellator to eliminate duplicated vertices. This allows us to take advantage of element
                     * buffer to minimize the number of vertices sent to the GPU.
                     */
                    Tessellator tessellator;

                    /**
                     * For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a unified
                     * rendering APIs. Thus for performance reasons, the selection of polygonal faces is also internally
                     * implemented by selecting triangle primitives using shaders. This allows data uploaded to the GPU
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
                        // tessellator.set_winding_rule(Tessellator::WINDING_NONZERO);  // or POSITIVE
                        tessellator.begin_contour();
                        const vec3 &color = fcolor[face];
                        for (auto h : model->halfedges(face)) {
                            auto v = model->to_vertex(h);
                            Tessellator::Vertex vertex(points[v], v.idx());
                            vertex.append(normals[v]);
                            vertex.append(color);
                            tessellator.add_vertex(vertex);
                        }
                        tessellator.end_contour();
                        tessellator.end_polygon();

                        std::size_t num = tessellator.num_elements_in_polygon();
                        triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                        count_triangles += num;
                    }

                    const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
                    std::vector<vec3> d_points, d_normals, d_colors;
                    d_points.reserve(vts.size());
                    d_normals.reserve(vts.size());
                    d_colors.reserve(vts.size());

                    for (auto v :vts) {
                        std::size_t offset = 0;
                        d_points.emplace_back(v->data() + offset);
                        offset += 3;
                        d_normals.emplace_back(v->data() + offset);
                        offset += 3;
                        d_colors.emplace_back(v->data() + offset);
                    }

                    const auto &d_indices = tessellator.elements();

                    drawable->update_vertex_buffer(d_points);
                    drawable->update_element_buffer(d_indices);
                    drawable->update_normal_buffer(d_normals);
                    drawable->update_color_buffer(d_colors);

                    DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->n_vertices() << "/"
                               << d_points.size();
                }
            }


            void update(SurfaceMesh *model, TrianglesDrawable *drawable,
                        SurfaceMesh::VertexProperty<vec3> vcolor) {
                assert(model);
                assert(drawable);
                assert(vcolor);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                if (model->is_triangle_mesh()) {
                    auto points = model->get_vertex_property<vec3>("v:point");
                    model->update_vertex_normals();
                    auto normals = model->get_vertex_property<vec3>("v:normal");

                    std::vector<unsigned int> d_indices;
                    d_indices.reserve(model->n_faces() * 3);
                    for (auto f : model->faces()) {
                        for (auto h : model->halfedges(f))
                            d_indices.push_back(model->to_vertex(h).idx());
                    }

                    drawable->update_vertex_buffer(points.vector());
                    drawable->update_element_buffer(d_indices);
                    drawable->update_normal_buffer(normals.vector());
                    drawable->update_color_buffer(vcolor.vector());
                } else {
                    /**
                     * We use the Tessellator to eliminate duplicated vertices. This allows us to take advantage of element
                     * buffer to minimize the number of vertices sent to the GPU.
                     */
                    Tessellator tessellator;

                    /**
                     * For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a unified
                     * rendering APIs. Thus for performance reasons, the selection of polygonal faces is also internally
                     * implemented by selecting triangle primitives using shaders. This allows data uploaded to the GPU
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
                        // tessellator.set_winding_rule(Tessellator::WINDING_NONZERO);  // or POSITIVE
                        tessellator.begin_contour();
                        for (auto h : model->halfedges(face)) {
                            auto v = model->to_vertex(h);
                            Tessellator::Vertex vertex(points[v], v.idx());
                            vertex.append(normals[v]);
                            vertex.append(vcolor[v]);
                            tessellator.add_vertex(vertex);
                        }
                        tessellator.end_contour();
                        tessellator.end_polygon();

                        std::size_t num = tessellator.num_elements_in_polygon();
                        triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                        count_triangles += num;
                    }

                    const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
                    std::vector<vec3> d_points, d_normals, d_colors;
                    d_points.reserve(vts.size());
                    d_normals.reserve(vts.size());
                    d_colors.reserve(vts.size());

                    for (auto v :vts) {
                        std::size_t offset = 0;
                        d_points.emplace_back(v->data() + offset);
                        offset += 3;
                        d_normals.emplace_back(v->data() + offset);
                        offset += 3;
                        d_colors.emplace_back(v->data() + offset);
                    }

                    const auto &d_indices = tessellator.elements();

                    drawable->update_vertex_buffer(d_points);
                    drawable->update_element_buffer(d_indices);
                    drawable->update_normal_buffer(d_normals);
                    drawable->update_color_buffer(d_colors);

                    DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->n_vertices() << "/"
                               << d_points.size();
                }
            }


            void update(SurfaceMesh *model, TrianglesDrawable *drawable,
                        SurfaceMesh::VertexProperty<vec2> vtexcoords) {
                assert(model);
                assert(drawable);
                assert(vtexcoords);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                if (model->is_triangle_mesh()) {
                    auto points = model->get_vertex_property<vec3>("v:point");
                    model->update_vertex_normals();
                    auto normals = model->get_vertex_property<vec3>("v:normal");

                    std::vector<unsigned int> d_indices;
                    d_indices.reserve(model->n_faces() * 3);
                    for (auto f : model->faces()) {
                        for (auto h : model->halfedges(f))
                            d_indices.push_back(model->to_vertex(h).idx());
                    }

                    drawable->update_vertex_buffer(points.vector());
                    drawable->update_element_buffer(d_indices);
                    drawable->update_normal_buffer(normals.vector());
                    drawable->update_texcoord_buffer(vtexcoords.vector());
                } else {
                    /**
                     * We use the Tessellator to eliminate duplicated vertices. This allows us to take advantage of element
                     * buffer to minimize the number of vertices sent to the GPU.
                     */
                    Tessellator tessellator;

                    /**
                     * For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a unified
                     * rendering APIs. Thus for performance reasons, the selection of polygonal faces is also internally
                     * implemented by selecting triangle primitives using shaders. This allows data uploaded to the GPU
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
                        // tessellator.set_winding_rule(Tessellator::WINDING_NONZERO);  // or POSITIVE
                        tessellator.begin_contour();
                        for (auto h : model->halfedges(face)) {
                            auto v = model->to_vertex(h);
                            Tessellator::Vertex vertex(points[v], v.idx());
                            vertex.append(normals[v]);
                            vertex.append(vtexcoords[v]);
                            tessellator.add_vertex(vertex);
                        }
                        tessellator.end_contour();
                        tessellator.end_polygon();

                        std::size_t num = tessellator.num_elements_in_polygon();
                        triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                        count_triangles += num;
                    }

                    const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
                    std::vector<vec3> d_points, d_normals;
                    std::vector<vec2> d_texcoords;
                    d_points.reserve(vts.size());
                    d_normals.reserve(vts.size());
                    d_texcoords.reserve(vts.size());

                    for (auto v :vts) {
                        std::size_t offset = 0;
                        d_points.emplace_back(v->data() + offset);
                        offset += 3;
                        d_normals.emplace_back(v->data() + offset);
                        offset += 3;
                        d_texcoords.emplace_back(v->data() + offset);
                    }

                    const auto &d_indices = tessellator.elements();

                    drawable->update_vertex_buffer(d_points);
                    drawable->update_element_buffer(d_indices);
                    drawable->update_normal_buffer(d_normals);
                    drawable->update_texcoord_buffer(d_texcoords);

                    DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->n_vertices() << "/"
                               << d_points.size();
                }
            }


            void update(SurfaceMesh *model, TrianglesDrawable *drawable,
                        SurfaceMesh::HalfedgeProperty<vec2> htexcoords) {
                assert(model);
                assert(drawable);
                assert(htexcoords);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                if (model->is_triangle_mesh()) {
                    auto points = model->get_vertex_property<vec3>("v:point");
                    model->update_vertex_normals();
                    auto normals = model->get_vertex_property<vec3>("v:normal");

                    std::vector<vec3> d_points, d_normals;
                    std::vector<vec2> d_texcoords;
                    d_points.reserve(model->n_faces() * 3),
                            d_normals.reserve(model->n_faces() * 3);
                    d_texcoords.reserve(model->n_faces() * 3);
                    for (auto face : model->faces()) {
                        for (auto h : model->halfedges(face)) {
                            auto v = model->to_vertex(h);
                            d_points.push_back(points[v]);
                            d_normals.push_back(normals[v]);
                            d_texcoords.push_back(htexcoords[h]);
                        }
                    }

                    drawable->update_vertex_buffer(d_points);
                    drawable->update_normal_buffer(d_normals);
                    drawable->update_texcoord_buffer(d_texcoords);
                    drawable->release_element_buffer();
                } else {
                    /**
                     * We use the Tessellator to eliminate duplicated vertices. This allows us to take advantage of element
                     * buffer to minimize the number of vertices sent to the GPU.
                     */
                    Tessellator tessellator;

                    /**
                     * For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a unified
                     * rendering APIs. Thus for performance reasons, the selection of polygonal faces is also internally
                     * implemented by selecting triangle primitives using shaders. This allows data uploaded to the GPU
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
                        // tessellator.set_winding_rule(Tessellator::WINDING_NONZERO);  // or POSITIVE
                        tessellator.begin_contour();
                        for (auto h : model->halfedges(face)) {
                            auto v = model->to_vertex(h);
                            Tessellator::Vertex vertex(points[v], v.idx());
                            vertex.append(normals[v]);
                            vertex.append(htexcoords[h]);
                            tessellator.add_vertex(vertex);
                        }
                        tessellator.end_contour();
                        tessellator.end_polygon();

                        std::size_t num = tessellator.num_elements_in_polygon();
                        triangle_range[face] = std::make_pair(count_triangles, count_triangles + num - 1);
                        count_triangles += num;
                    }

                    const std::vector<Tessellator::Vertex *> &vts = tessellator.vertices();
                    std::vector<vec3> d_points, d_normals;
                    std::vector<vec2> d_texcoords;
                    d_points.reserve(vts.size());
                    d_normals.reserve(vts.size());
                    d_texcoords.reserve(vts.size());

                    for (auto v :vts) {
                        std::size_t offset = 0;
                        d_points.emplace_back(v->data() + offset);
                        offset += 3;
                        d_normals.emplace_back(v->data() + offset);
                        offset += 3;
                        d_texcoords.emplace_back(v->data() + offset);
                    }

                    const auto &d_indices = tessellator.elements();

                    drawable->update_vertex_buffer(d_points);
                    drawable->update_element_buffer(d_indices);
                    drawable->update_normal_buffer(d_normals);
                    drawable->update_texcoord_buffer(d_texcoords);

                    DLOG(INFO) << "num of vertices in model/sent to GPU: " << model->n_vertices() << "/"
                               << d_points.size();
                }
            }


            void update(SurfaceMesh *model, LinesDrawable *drawable, SurfaceMesh::EdgeProperty<vec3> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

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


            void
            update(SurfaceMesh *model, LinesDrawable *drawable, SurfaceMesh::VertexProperty<vec3> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

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


            void
            update(SurfaceMesh *model, LinesDrawable *drawable, SurfaceMesh::VertexProperty<vec2> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                auto points = model->get_vertex_property<vec3>("v:point");
                std::vector<vec3> d_points;
                d_points.reserve(model->n_edges() * 2);
                std::vector<vec2> d_texcoords;
                d_points.reserve(model->n_edges() * 2);
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


            void update(SurfaceMesh *model, LinesDrawable *drawable, SurfaceMesh::EdgeProperty<vec2> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                auto points = model->get_vertex_property<vec3>("v:point");
                std::vector<vec3> d_points;
                d_points.reserve(model->n_edges() * 2);
                std::vector<vec2> d_texcoords;
                d_points.reserve(model->n_edges() * 2);
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


            void update_mesh_borders(SurfaceMesh *model, LinesDrawable *drawable) {
                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                auto prop = model->get_vertex_property<vec3>("v:point");
                std::vector<vec3> points;
                points.reserve(model->n_edges() * 2);
                for (auto e : model->edges()) {
                    if (model->is_boundary(e)) {
                        points.push_back(prop[model->vertex(e, 0)]);
                        points.push_back(prop[model->vertex(e, 1)]);
                    }
                }
                drawable->update_vertex_buffer(points);
            }


            void update_mesh_locked_vertices(SurfaceMesh *model, PointsDrawable *drawable) {
                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                auto locked = model->get_vertex_property<bool>("v:locked");
                if (locked) {
                    auto prop = model->get_vertex_property<vec3>("v:point");
                    std::vector<vec3> points;
                    for (auto v : model->vertices()) {
                        if (locked[v])
                            points.push_back(prop[v]);
                    }
                    drawable->update_vertex_buffer(points);
                }
            }


            void update(Graph *model, PointsDrawable *drawable, Graph::VertexProperty<vec3> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());
                drawable->update_color_buffer(prop.vector());
                drawable->set_impostor_type(PointsDrawable::SPHERE);
            }


            void update(Graph *model, PointsDrawable *drawable, Graph::VertexProperty<vec2> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());
                drawable->update_texcoord_buffer(prop.vector());
                drawable->set_impostor_type(PointsDrawable::SPHERE);
            }


            void update(Graph *model, LinesDrawable *drawable, Graph::EdgeProperty<vec3> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

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

            void update(Graph *model, LinesDrawable *drawable, Graph::VertexProperty<vec2> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());

                drawable->update_texcoord_buffer(prop.vector());

                std::vector<unsigned int> indices;
                indices.reserve(model->n_edges() * 2);
                for (auto e : model->edges()) {
                    unsigned int s = model->vertex(e, 0).idx();
                    unsigned int t = model->vertex(e, 1).idx();
                    indices.push_back(s);
                    indices.push_back(t);
                }
                drawable->update_element_buffer(indices);
                drawable->set_impostor_type(LinesDrawable::CYLINDER);
            }


            void update(Graph *model, LinesDrawable *drawable, Graph::EdgeProperty<vec2> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

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


            void update(Graph *model, LinesDrawable *drawable, Graph::VertexProperty<vec3> prop) {
                assert(model);
                assert(drawable);
                assert(prop);

                if (model->empty()) {
                    LOG(WARNING) << "model has no valid geometry";
                    return;
                }

                auto points = model->get_vertex_property<vec3>("v:point");
                drawable->update_vertex_buffer(points.vector());

                drawable->update_color_buffer(prop.vector());

                std::vector<unsigned int> indices;
                indices.reserve(model->n_edges() * 2);
                for (auto e : model->edges()) {
                    unsigned int s = model->vertex(e, 0).idx();
                    unsigned int t = model->vertex(e, 1).idx();
                    indices.push_back(s);
                    indices.push_back(t);
                }
                drawable->update_element_buffer(indices);
                drawable->set_impostor_type(LinesDrawable::CYLINDER);
            }

        }



        // -------------------------------------------------------------------------------------------------------------



        void update(PointCloud *model, PointsDrawable *drawable) {
            assert(model);
            assert(drawable);

            if (model->empty()) {
                LOG(WARNING) << "model has no valid geometry";
                return;
            }

            const std::string &name = drawable->property_name();
            switch (drawable->coloring_method()) {
                case State::TEXTURED: {
                    auto texcoord = model->get_vertex_property<vec2>(name);
                    if (texcoord)
                        details::update(model, drawable, texcoord);
                    else
                        LOG(WARNING) << "texcoord property not found: " << drawable->property_name();
                    break;
                }

                case State::COLOR_PROPERTY: {
                    auto colors = model->get_vertex_property<vec3>(name);
                    if (colors)
                        details::update(model, drawable, colors);
                    else
                        LOG(WARNING) << "color property not found: " << drawable->property_name();
                    break;
                }

                case State::SCALAR_FIELD: {
                    if (model->get_vertex_property<float>(name)) {
                        auto prop = model->get_vertex_property<float>(name);
                        details::update(model, drawable, prop);
                    } else if (model->get_vertex_property<double>(name)) {
                        auto prop = model->get_vertex_property<double>(name);
                        details::update(model, drawable, prop);
                    } else if (model->get_vertex_property<int>(name)) {
                        auto prop = model->get_vertex_property<int>(name);
                        details::update(model, drawable, prop);
                    } else if (model->get_vertex_property<unsigned int>(name)) {
                        auto prop = model->get_vertex_property<unsigned int>(name);
                        details::update(model, drawable, prop);
                    } else if (model->get_vertex_property<char>(name)) {
                        auto prop = model->get_vertex_property<char>(name);
                        details::update(model, drawable, prop);
                    } else if (model->get_vertex_property<unsigned char>(name)) {
                        auto prop = model->get_vertex_property<unsigned char>(name);
                        details::update(model, drawable, prop);
                    } else {
                        LOG(WARNING) << "scalar field not found: " << drawable->property_name();
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
                    break;
                }
            }
        }


        void update(PointCloud *model, LinesDrawable *drawable, const std::string &field, float scale) {
            if (model->empty()) {
                LOG(WARNING) << "model has no valid geometry";
                return;
            }

            auto prop = model->get_vertex_property<vec3>(field);
            if (!prop) {
                LOG(ERROR) << "vector filed '" << field << " ' not found on the point cloud (wrong name?)";
                return;
            }

            auto points = model->get_vertex_property<vec3>("v:point");
            float length = model->bounding_box().diagonal() * 0.5f * 0.01f * scale;

            std::vector<vec3> vertices(model->n_vertices() * 2, vec3(0.0f, 0.0f, 0.0f));
            int idx = 0;
            for (auto v: model->vertices()) {
                vertices[idx << 1] = points[v];
                vertices[(idx << 1) + 1] = points[v] + prop[v] * length;
                ++idx;
            }
            drawable->update_vertex_buffer(vertices);
        };


        // -------------------------------------------------------------------------------------------------------------


        void update(SurfaceMesh *model, PointsDrawable *drawable) {
            assert(model);
            assert(drawable);

            if (model->empty()) {
                LOG(WARNING) << "model has no valid geometry";
                return;
            }

            if (drawable->name() == "locks") {
                details::update_mesh_locked_vertices(model, drawable);
                return;
            }

            const std::string &name = drawable->property_name();
            switch (drawable->coloring_method()) {
                case State::TEXTURED: {
                    auto texcoord = model->get_vertex_property<vec2>(name);
                    if (texcoord)
                        details::update(model, drawable, texcoord);
                    else
                        LOG(WARNING) << "texcoord property not found: " << drawable->property_name();
                    break;
                }

                case State::COLOR_PROPERTY: {
                    auto colors = model->get_vertex_property<vec3>(name);
                    if (colors)
                        details::update(model, drawable, colors);
                    else
                        LOG(WARNING) << "color property not found: " << drawable->property_name();
                    break;
                }

                case State::SCALAR_FIELD: {
                    if (model->get_vertex_property<float>(name)) {
                        auto prop = model->get_vertex_property<float>(name);
                        details::update(model, drawable, prop);
                    } else if (model->get_vertex_property<double>(name)) {
                        auto prop = model->get_vertex_property<double>(name);
                        details::update(model, drawable, prop);
                    } else if (model->get_vertex_property<int>(name)) {
                        auto prop = model->get_vertex_property<int>(name);
                        details::update(model, drawable, prop);
                    } else if (model->get_vertex_property<unsigned int>(name)) {
                        auto prop = model->get_vertex_property<unsigned int>(name);
                        details::update(model, drawable, prop);
                    } else if (model->get_vertex_property<char>(name)) {
                        auto prop = model->get_vertex_property<char>(name);
                        details::update(model, drawable, prop);
                    } else if (model->get_vertex_property<unsigned char>(name)) {
                        auto prop = model->get_vertex_property<unsigned char>(name);
                        details::update(model, drawable, prop);
                    } else {
                        LOG(WARNING) << "scalar field not found: " << drawable->property_name();
                        return;
                    }
                    break;
                }

                default: { // uniform color
                    auto points = model->get_vertex_property<vec3>("v:point");
                    drawable->update_vertex_buffer(points.vector());
                    auto normals = model->get_vertex_property<vec3>("v:normal");
                    if (normals)
                        drawable->update_normal_buffer(normals.vector());
                    break;
                }
            }
        }



        // -------------------------------------------------------------------------------------------------------------



        void update(SurfaceMesh *model, LinesDrawable *drawable) {
            assert(model);
            assert(drawable);

            if (model->empty()) {
                LOG(WARNING) << "model has no valid geometry";
                return;
            }

            if (drawable->name() == "borders") {
                details::update_mesh_borders(model, drawable);
                return;
            }

            const std::string &name = drawable->property_name();
            switch (drawable->coloring_method()) {
                case State::TEXTURED: {
                    switch (drawable->property_location()) {
                        case State::EDGE: {
                            auto texcoord = model->get_edge_property<vec2>(name);
                            if (texcoord)
                                details::update(model, drawable, texcoord);
                            else
                                LOG(WARNING) << "texcoord property not found on edges: "
                                             << drawable->property_name();
                            break;
                        }
                        case State::VERTEX: {
                            auto texcoord = model->get_vertex_property<vec2>(name);
                            if (texcoord)
                                details::update(model, drawable, texcoord);
                            else
                                LOG(WARNING) << "texcoord property not found on vertices: "
                                             << drawable->property_name();
                            break;
                        }
                        case State::FACE:
                        case State::HALFEDGE:
                            LOG(WARNING) << "should not happen" << drawable->property_name();
                            break;
                    }
                    break;
                }

                case State::COLOR_PROPERTY: {
                    switch (drawable->property_location()) {
                        case State::EDGE: {
                            auto colors = model->get_edge_property<vec3>(name);
                            if (colors)
                                details::update(model, drawable, colors);
                            else
                                LOG(WARNING) << "color property not found: " << drawable->property_name();
                            break;
                        }
                        case State::VERTEX: {
                            auto colors = model->get_vertex_property<vec3>(name);
                            if (colors)
                                details::update(model, drawable, colors);
                            else
                                LOG(WARNING) << "color property not found: " << drawable->property_name();
                            break;
                        }
                        case State::FACE:
                        case State::HALFEDGE:
                            LOG(WARNING) << "should not happen" << drawable->property_name();
                            break;
                    }
                    break;
                }

                case State::SCALAR_FIELD: {
                    switch (drawable->property_location()) {
                        case State::EDGE: {
                            if (model->get_edge_property<float>(name)) {
                                auto prop = model->get_edge_property<float>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_edge_property<double>(name)) {
                                auto prop = model->get_edge_property<double>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_edge_property<int>(name)) {
                                auto prop = model->get_edge_property<int>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_edge_property<unsigned int>(name)) {
                                auto prop = model->get_edge_property<unsigned int>(name);
                                details::update(model, drawable, prop);
                            } else {
                                LOG(WARNING) << "scalar field not found on edges: " << drawable->property_name();
                                return;
                            }
                            break;
                        }
                        case State::VERTEX: {
                            if (model->get_vertex_property<float>(name)) {
                                auto prop = model->get_vertex_property<float>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_vertex_property<double>(name)) {
                                auto prop = model->get_vertex_property<double>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_vertex_property<int>(name)) {
                                auto prop = model->get_vertex_property<int>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_vertex_property<unsigned int>(name)) {
                                auto prop = model->get_vertex_property<unsigned int>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_vertex_property<char>(name)) {
                                auto prop = model->get_vertex_property<char>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_vertex_property<unsigned char>(name)) {
                                auto prop = model->get_vertex_property<unsigned char>(name);
                                details::update(model, drawable, prop);
                            } else {
                                LOG(WARNING) << "scalar field not found on vertices: " << drawable->property_name();
                                return;
                            }
                            break;
                        }
                        case State::FACE:
                        case State::HALFEDGE:
                            LOG(WARNING) << "should not happen" << drawable->property_name();
                            break;
                    }
                    break;
                }

                default: {// uniform color
                    // if reached here, we choose a uniform color.
                    std::vector<unsigned int> indices;
                    indices.reserve(model->n_edges() * 2);
                    for (auto e : model->edges()) {
                        SurfaceMesh::Vertex s = model->vertex(e, 0);
                        SurfaceMesh::Vertex t = model->vertex(e, 1);
                        indices.push_back(s.idx());
                        indices.push_back(t.idx());
                    }
                    auto points = model->get_vertex_property<vec3>("v:point");
                    drawable->update_vertex_buffer(points.vector());
                    drawable->update_element_buffer(indices);
                    break;
                }
            }
        }


        void update(SurfaceMesh *model, LinesDrawable *drawable, const std::string &field, int location, float scale) {
            if (model->empty()) {
                LOG(WARNING) << "model has no valid geometry";
                return;
            }

            switch (location) {
                case 0:
                    if (!model->get_face_property<vec3>(field)) {
                        LOG(ERROR) << "vector field '" << field << "' not found on the mesh faces (wrong name?)";
                        return;
                    }
                    break;
                case 1:
                    if (!model->get_vertex_property<vec3>(field)) {
                        LOG(ERROR) << "vector field '" << field << "' not found on the mesh vertices (wrong name?)";
                        return;
                    }
                    break;
                case 2:
                    if (!model->get_edge_property<vec3>(field)) {
                        LOG(ERROR) << "vector field '" << field << "' not found on the mesh edges (wrong name?)";
                        return;
                    }
                    break;
                default:
                    LOG(ERROR) << "vector field '" << field << "' not found (wrong name?)";
                    return;
            }

            auto points = model->get_vertex_property<vec3>("v:point");

            // use a limited number of edge to compute the length of the vectors.
            float avg_edge_length = 0.0f;
            const unsigned int num = std::min(static_cast<unsigned int>(500), model->n_edges());
            for (unsigned int i = 0; i < num; ++i) {
                SurfaceMesh::Edge edge(i);
                auto vs = model->vertex(edge, 0);
                auto vt = model->vertex(edge, 1);
                avg_edge_length += distance(points[vs], points[vt]);
            }
            avg_edge_length /= num;

            std::vector<vec3> d_points;

            switch (location) {
                case 0: {   // on faces
                    auto prop = model->get_face_property<vec3>(field);
                    d_points.resize(model->n_faces() * 2, vec3(0.0f, 0.0f, 0.0f));
                    int idx = 0;
                    for (auto f: model->faces()) {
                        int size = 0;
                        for (auto v: model->vertices(f)) {
                            d_points[idx] += points[v];
                            ++size;
                        }
                        d_points[idx] /= size;
                        d_points[idx + 1] = d_points[idx] + prop[f] * avg_edge_length * scale;
                        idx += 2;
                    }
                    break;
                }
                case 1: {   // on vertices
                    auto prop = model->get_vertex_property<vec3>(field);
                    d_points.resize(model->n_vertices() * 2, vec3(0.0f, 0.0f, 0.0f));
                    for (auto v: model->vertices()) {
                        d_points[v.idx() * 2] = points[v];
                        d_points[v.idx() * 2 + 1] = points[v] + prop[v] * avg_edge_length * scale;
                    }
                    break;
                }
                case 2: {   // on edges
                    auto prop = model->get_edge_property<vec3>(field);
                    d_points.resize(model->n_edges() * 2, vec3(0.0f, 0.0f, 0.0f));
                    for (auto e : model->edges()) {
                        auto v0 = model->vertex(e, 0);
                        auto v1 = model->vertex(e, 1);
                        const auto p = (points[v0] + points[v1]) * 0.5f;
                        d_points[e.idx() * 2] = p;
                        d_points[e.idx() * 2 + 1] = p + prop[e] * avg_edge_length * scale;
                    }
                    break;
                }
            }
            drawable->update_vertex_buffer(d_points);
        };


        // -------------------------------------------------------------------------------------------------------------


        void update(SurfaceMesh *model, TrianglesDrawable *drawable) {
            assert(model);
            assert(drawable);

            if (model->empty()) {
                LOG(WARNING) << "model has no valid geometry";
                return;
            }

            const std::string &name = drawable->property_name();
            switch (drawable->coloring_method()) {
                case State::TEXTURED: {
                    switch (drawable->property_location()) {
                        case State::VERTEX: {
                            auto texcoord = model->get_vertex_property<vec2>(name);
                            if (texcoord)
                                details::update(model, drawable, texcoord);
                            else
                                LOG(WARNING) << "texcoord property not found on vertices: "
                                             << drawable->property_name();
                            break;
                        }
                        case State::HALFEDGE: {
                            auto texcoord = model->get_halfedge_property<vec2>(name);
                            if (texcoord)
                                details::update(model, drawable, texcoord);
                            else
                                LOG(WARNING) << "texcoord property not found on halfedges: "
                                             << drawable->property_name();
                            break;
                        }
                        case State::FACE:
                        case State::EDGE:
                            LOG(WARNING) << "should not happen" << drawable->property_name();
                            break;
                    }
                    break;
                }

                case State::COLOR_PROPERTY: {
                    switch (drawable->property_location()) {
                        case State::FACE: {
                            auto colors = model->get_face_property<vec3>(name);
                            if (colors)
                                details::update(model, drawable, colors);
                            else
                                LOG(WARNING) << "color property not found: " << drawable->property_name();
                            break;
                        }
                        case State::VERTEX: {
                            auto colors = model->get_vertex_property<vec3>(name);
                            if (colors)
                                details::update(model, drawable, colors);
                            else
                                LOG(WARNING) << "color property not found: " << drawable->property_name();
                            break;
                        }
                        case State::EDGE:
                        case State::HALFEDGE:
                            LOG(WARNING) << "should not happen" << drawable->property_name();
                            break;
                    }
                    break;
                }

                case State::SCALAR_FIELD: {
                    switch (drawable->property_location()) {
                        case State::FACE: {
                            if (model->get_face_property<float>(name)) {
                                auto prop = model->get_face_property<float>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_face_property<double>(name)) {
                                auto prop = model->get_face_property<double>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_face_property<int>(name)) {
                                auto prop = model->get_face_property<int>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_face_property<unsigned int>(name)) {
                                auto prop = model->get_face_property<unsigned int>(name);
                                details::update(model, drawable, prop);
                            } else {
                                LOG(WARNING) << "scalar field not found on edges: " << drawable->property_name();
                                return;
                            }
                            break;
                        }
                        case State::VERTEX: {
                            if (model->get_vertex_property<float>(name)) {
                                auto prop = model->get_vertex_property<float>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_vertex_property<double>(name)) {
                                auto prop = model->get_vertex_property<double>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_vertex_property<int>(name)) {
                                auto prop = model->get_vertex_property<int>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_vertex_property<unsigned int>(name)) {
                                auto prop = model->get_vertex_property<unsigned int>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_vertex_property<char>(name)) {
                                auto prop = model->get_vertex_property<char>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_vertex_property<unsigned char>(name)) {
                                auto prop = model->get_vertex_property<unsigned char>(name);
                                details::update(model, drawable, prop);
                            } else {
                                LOG(WARNING) << "scalar field not found on vertices: " << drawable->property_name();
                                return;
                            }
                            break;
                        }
                        case State::EDGE:
                        case State::HALFEDGE:
                            LOG(WARNING) << "should not happen" << drawable->property_name();
                            break;
                    }
                    break;
                }

                default: {// uniform color
                    // if reached here, we choose a uniform color.
                    details::update(model, drawable);
                    break;
                }
            }
        }



        // -------------------------------------------------------------------------------------------------------------



        void update(Graph *model, PointsDrawable *drawable) {
            assert(model);
            assert(drawable);

            if (model->empty()) {
                LOG(WARNING) << "model has no valid geometry";
                return;
            }

            const std::string &name = drawable->property_name();
            switch (drawable->coloring_method()) {
                case State::TEXTURED: {
                    auto texcoord = model->get_vertex_property<vec2>(name);
                    if (texcoord)
                        details::update(model, drawable, texcoord);
                    else
                        LOG(WARNING) << "texcoord property not found: " << drawable->property_name();
                    break;
                }

                case State::COLOR_PROPERTY: {
                    auto colors = model->get_vertex_property<vec3>(name);
                    if (colors)
                        details::update(model, drawable, colors);
                    else
                        LOG(WARNING) << "color property not found: " << drawable->property_name();
                    break;
                }

                case State::SCALAR_FIELD: {
                    if (model->get_vertex_property<float>(name)) {
                        auto prop = model->get_vertex_property<float>(name);
                        details::update(model, drawable, prop);
                    } else if (model->get_vertex_property<double>(name)) {
                        auto prop = model->get_vertex_property<double>(name);
                        details::update(model, drawable, prop);
                    } else if (model->get_vertex_property<int>(name)) {
                        auto prop = model->get_vertex_property<int>(name);
                        details::update(model, drawable, prop);
                    } else if (model->get_vertex_property<unsigned int>(name)) {
                        auto prop = model->get_vertex_property<unsigned int>(name);
                        details::update(model, drawable, prop);
                    } else if (model->get_vertex_property<char>(name)) {
                        auto prop = model->get_vertex_property<char>(name);
                        details::update(model, drawable, prop);
                    } else if (model->get_vertex_property<unsigned char>(name)) {
                        auto prop = model->get_vertex_property<unsigned char>(name);
                        details::update(model, drawable, prop);
                    } else {
                        LOG(WARNING) << "scalar field not found: " << drawable->property_name();
                        return;
                    }
                    break;
                }

                default: // uniform color
                    auto points = model->get_vertex_property<vec3>("v:point");
                    drawable->update_vertex_buffer(points.vector());
                    break;
            }

        }



        // -------------------------------------------------------------------------------------------------------------



        void update(Graph *model, LinesDrawable *drawable) {
            assert(model);
            assert(drawable);

            if (model->empty()) {
                LOG(WARNING) << "model has no valid geometry";
                return;
            }

            const std::string &name = drawable->property_name();
            switch (drawable->coloring_method()) {
                case State::TEXTURED: {
                    switch (drawable->property_location()) {
                        case State::EDGE: {
                            auto texcoord = model->get_edge_property<vec2>(name);
                            if (texcoord)
                                details::update(model, drawable, texcoord);
                            else
                                LOG(WARNING) << "texcoord property not found on edges: "
                                             << drawable->property_name();
                            break;
                        }
                        case State::VERTEX: {
                            auto texcoord = model->get_vertex_property<vec2>(name);
                            if (texcoord)
                                details::update(model, drawable, texcoord);
                            else
                                LOG(WARNING) << "texcoord property not found on vertices: "
                                             << drawable->property_name();
                            break;
                        }
                        case State::FACE:
                        case State::HALFEDGE:
                            LOG(WARNING) << "should not happen" << drawable->property_name();
                            break;
                    }
                    break;
                }

                case State::COLOR_PROPERTY: {
                    switch (drawable->property_location()) {
                        case State::EDGE: {
                            auto colors = model->get_edge_property<vec3>(name);
                            if (colors)
                                details::update(model, drawable, colors);
                            else
                                LOG(WARNING) << "color property not found: " << drawable->property_name();
                            break;
                        }
                        case State::VERTEX: {
                            auto colors = model->get_vertex_property<vec3>(name);
                            if (colors)
                                details::update(model, drawable, colors);
                            else
                                LOG(WARNING) << "color property not found: " << drawable->property_name();
                            break;
                        }
                        case State::FACE:
                        case State::HALFEDGE:
                            LOG(WARNING) << "should not happen" << drawable->property_name();
                            break;
                    }
                    break;
                }

                case State::SCALAR_FIELD: {
                    switch (drawable->property_location()) {
                        case State::EDGE: {
                            if (model->get_edge_property<float>(name)) {
                                auto prop = model->get_edge_property<float>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_edge_property<double>(name)) {
                                auto prop = model->get_edge_property<double>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_edge_property<int>(name)) {
                                auto prop = model->get_edge_property<int>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_edge_property<unsigned int>(name)) {
                                auto prop = model->get_edge_property<unsigned int>(name);
                                details::update(model, drawable, prop);
                            } else {
                                LOG(WARNING) << "scalar field not found on edges: " << drawable->property_name();
                                return;
                            }
                            break;
                        }
                        case State::VERTEX: {
                            if (model->get_vertex_property<float>(name)) {
                                auto prop = model->get_vertex_property<float>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_vertex_property<double>(name)) {
                                auto prop = model->get_vertex_property<double>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_vertex_property<int>(name)) {
                                auto prop = model->get_vertex_property<int>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_vertex_property<unsigned int>(name)) {
                                auto prop = model->get_vertex_property<unsigned int>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_vertex_property<char>(name)) {
                                auto prop = model->get_vertex_property<char>(name);
                                details::update(model, drawable, prop);
                            } else if (model->get_vertex_property<unsigned char>(name)) {
                                auto prop = model->get_vertex_property<unsigned char>(name);
                                details::update(model, drawable, prop);
                            } else {
                                LOG(WARNING) << "scalar field not found on vertices: " << drawable->property_name();
                                return;
                            }
                            break;
                        }
                        case State::FACE:
                        case State::HALFEDGE:
                            LOG(WARNING) << "should not happen" << drawable->property_name();
                            break;
                    }
                    break;
                }

                default: {// uniform color
                    // if reached here, we choose a uniform color.
                    std::vector<unsigned int> indices;
                    indices.reserve(model->n_edges() * 2);
                    for (auto e : model->edges()) {
                        indices.push_back(model->vertex(e, 0).idx());
                        indices.push_back(model->vertex(e, 1).idx());
                    }
                    auto points = model->get_vertex_property<vec3>("v:point");
                    drawable->update_vertex_buffer(points.vector());
                    drawable->update_element_buffer(indices);
                    break;
                }
            }
        }


        // -------------------------------------------------------------------------------------------------------------


        void update(Model *model, Drawable *drawable) {
            if (model->empty()) {
                LOG(WARNING) << "model has no valid geometry";
                return;
            }

            if (dynamic_cast<SurfaceMesh *>(model)) {
                SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(model);
                switch (drawable->type()) {
                    case Drawable::DT_TRIANGLES:
                        update(mesh, dynamic_cast<TrianglesDrawable *>(drawable));
                        break;
                    case Drawable::DT_LINES:
                        update(mesh, dynamic_cast<LinesDrawable *>(drawable));
                        break;
                    case Drawable::DT_POINTS:
                        update(mesh, dynamic_cast<PointsDrawable *>(drawable));
                        break;
                }
            }

            if (dynamic_cast<PointCloud *>(model)) {
                PointCloud *cloud = dynamic_cast<PointCloud *>(model);
                switch (drawable->type()) {
                    case Drawable::DT_POINTS:
                        update(cloud, dynamic_cast<PointsDrawable *>(drawable));
                        break;
                    case Drawable::DT_LINES:
                        update(cloud, dynamic_cast<LinesDrawable *>(drawable));
                        break;
                    case Drawable::DT_TRIANGLES:
                        update(cloud, dynamic_cast<TrianglesDrawable *>(drawable));
                        break;
                }
            } else if (dynamic_cast<Graph *>(model)) {
                Graph *graph = dynamic_cast<Graph *>(model);
                switch (drawable->type()) {
                    case Drawable::DT_POINTS:
                        update(graph, dynamic_cast<PointsDrawable *>(drawable));
                        break;
                    case Drawable::DT_LINES:
                        update(graph, dynamic_cast<LinesDrawable *>(drawable));
                        break;
                    case Drawable::DT_TRIANGLES:
                        update(graph, dynamic_cast<TrianglesDrawable *>(drawable));
                        break;
                }
            }
        }
    }

}

