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

#include <easy3d/viewer/tessellator.h>

#include <vector>
#include <unordered_map>

#include <easy3d/core/hash.h>
#include <easy3d/util/logging.h>

#include <3rd_party/libtess/glutess.h>


namespace easy3d {

    namespace details {

        // VertexManager manages the actual vertices to make sure that the points are unique.
        class VertexManager {
        public:
            VertexManager() {}
            ~VertexManager() {
                clear();
            }

            const std::vector<Tessellator::Vertex *> &vertices() const {
                return unique_vertices_;
            }

            inline Tessellator::Vertex *find_or_create(const Tessellator::Vertex &v) {
                std::size_t key = hash_range(v.begin(), v.end());
                auto pos = hash_table_.find(key);
                if (pos == hash_table_.end()) {
                    auto vertex = new Tessellator::Vertex(v, v.index);
                    hash_table_[key] = unique_vertices_.size();
                    unique_vertices_.push_back(vertex);
                    return vertex;
                } else
                    return unique_vertices_[pos->second];
            }

            inline void clear() {
                for (auto vertex : unique_vertices_)
                    delete vertex;
                unique_vertices_.clear();
                hash_table_.clear();
            }

            inline std::size_t vertex_id(const Tessellator::Vertex &v) {
                return hash_table_[hash_range(v.begin(), v.end())];
            }

        private:
            std::vector<Tessellator::Vertex *> unique_vertices_;

            // key -> index
            std::unordered_map<std::size_t, std::size_t> hash_table_;
        };
    }



    // -------------------------------------------------------------------------------------------------------------



    Tessellator::Tessellator()
            : primitive_type_(TESS_TRIANGLES)
            , num_elements_in_polygon_(0)
            , vertex_data_size_(3)
    {
        tess_obj_ = NewTess();// Create a tessellator object and set up its callbacks.
        if (!tess_obj_) {
            LOG(ERROR) << "failed to create a tessellator object";
            return;
        }

        TessCallback(tess_obj_, TESS_VERTEX_DATA, (void (*)()) vertexCallback);
        TessCallback(tess_obj_, TESS_BEGIN_DATA, (void (*)()) beginCallback);
        TessCallback(tess_obj_, TESS_END_DATA, (void (*)()) endCallback);
        TessCallback(tess_obj_, TESS_COMBINE_DATA, (void (*)()) combineCallback);

        TessProperty(tess_obj_, TESS_WINDING_RULE, TESS_WINDING_ODD);
        TessProperty(tess_obj_, TESS_TOLERANCE, 0.);

        vertex_manager_ = new details::VertexManager;
    }


    Tessellator::~Tessellator() {
        delete reinterpret_cast<details::VertexManager *>(vertex_manager_);
        DeleteTess(tess_obj_);
    }


    void Tessellator::set_bounary_only(bool b) {
        TessProperty(tess_obj_, TESS_BOUNDARY_ONLY, b);
    }


    void Tessellator::set_winding_rule(WindingRule rule) {
        switch (rule) {
            case WINDING_ODD:
                TessProperty(tess_obj_, TESS_WINDING_RULE, TESS_WINDING_ODD);
                return;
            case WINDING_NONZERO:
                TessProperty(tess_obj_, TESS_WINDING_RULE, TESS_WINDING_NONZERO);
                return;
            case WINDING_POSITIVE:
                TessProperty(tess_obj_, TESS_WINDING_RULE, TESS_WINDING_POSITIVE);
                return;
            case WINDING_NEGATIVE:
                TessProperty(tess_obj_, TESS_WINDING_RULE, TESS_WINDING_NEGATIVE);
                return;
            case WINDING_ABS_GEQ_TWO:
                TessProperty(tess_obj_, TESS_WINDING_RULE, TESS_WINDING_ABS_GEQ_TWO);
                return;
        }
    }


    void Tessellator::begin_polygon(const vec3 &normal) {
        num_elements_in_polygon_ = 0;
        TessNormal(tess_obj_, normal.x, normal.y, normal.z);
        TessBeginPolygon(tess_obj_, this);
    }


    void Tessellator::begin_contour() {
        TessBeginContour(tess_obj_);
    }


    void Tessellator::add_vertex(const Vertex &v) {
        vertex_data_size_ = v.size();

        Vertex *new_v = reinterpret_cast<details::VertexManager *>(vertex_manager_)->find_or_create(v);
        // TessVertex() takes 3 params: tess object, pointer to vertex coords,
        // and pointer to vertex data to be passed to vertex callback.
        // The second param is used only to perform tessellation, and the third
        // param is the actual vertex data to draw. It is usually same as the second
        // param, but It can be more than vertex coord, for example, color, normal
        // and UV coords which are needed for actual drawing.
        TessVertex(tess_obj_, new_v->data(), new_v);
    }

    // to be flexible (any data can be provide)
    void Tessellator::add_vertex(const float *data, unsigned int size, int idx)
    {
        add_vertex(Vertex(data, size, idx));
    }


    void Tessellator::add_vertex(const vec3 &xyz, int idx) {
        add_vertex(Vertex(xyz, idx));
    }


    void Tessellator::add_vertex(const vec3 &xyz, const vec2 &t, int idx) {
        Vertex v(xyz, idx);
        v.append(t);
        add_vertex(v);
    }


    void Tessellator::add_vertex(const vec3 &xyz, const vec3 &v1, int idx) {
        Vertex v(xyz, idx);
        v.append(v1);
        add_vertex(v);
    }


    void Tessellator::add_vertex(const vec3 &xyz, const vec3 &v1, const vec2 &t, int idx) {
        Vertex v(xyz, idx);
        v.append(v1);
        v.append(t);
        add_vertex(v);
    }


    void Tessellator::add_vertex(const vec3 &xyz, const vec3 &v1, const vec3 &v2, int idx) {
        Vertex v(xyz, idx);
        v.append(v1);
        v.append(v2);
        add_vertex(v);
    }


    void Tessellator::add_vertex(const vec3 &xyz, const vec3 &v1, const vec3 &v2, const vec2 &t, int idx) {
        Vertex v(xyz, idx);
        v.append(v1);
        v.append(v2);
        v.append(t);
        add_vertex(v);
    }


    void Tessellator::end_contour() {
        TessEndContour(tess_obj_);
    }


    void Tessellator::end_polygon() {
        TessEndPolygon(tess_obj_);
    }


    const std::vector<Tessellator::Vertex *> &Tessellator::vertices() const {
        return reinterpret_cast<details::VertexManager *>(vertex_manager_)->vertices();
    }


    void Tessellator::_add_element(const std::vector<unsigned int>& element) {
        elements_.push_back(element);
        ++num_elements_in_polygon_;
    }


    // ****************************************************************************
    // Method: Tessellator::beginCallback
    // Purpose: Begin callback for the tessellator.
    // Arguments:
    //   w: The type of primitives being created (e.g., TESS_LINE_LOOP, TESS_TRIANGLES,
    //   TESS_TRIANGLE_STRIP, TESS_TRIANGLE_FAN).
    //   cbdata: Callback data that points to "this".
    // ****************************************************************************
    void Tessellator::beginCallback(unsigned int w, void *cbdata) {
        Tessellator *tessellator = reinterpret_cast<Tessellator *>(cbdata);
        tessellator->primitive_type_ = w;
        tessellator->vertex_ids_.clear();

    }


    // ****************************************************************************
    // Method: Tessellator::endCallback
    // Purpose:
    //   End callback for the tessellator. Translates intermediate vertices and
    //   drawing mode into triangle/contour definitions.
    // Arguments:
    //   cbdata : Callback data that points to "this".
    // ****************************************************************************
    void Tessellator::endCallback(void *cbdata) {
        Tessellator *tessellator = reinterpret_cast<Tessellator *>(cbdata);

        // Use the primitive type and intermediate vertex ids to create triangles that
        // get put into the triangle list
        switch (tessellator->primitive_type_) {
            case TESS_TRIANGLES: {
                for (std::size_t i = 0; i < tessellator->vertex_ids_.size(); i += 3) {
                    unsigned int a = tessellator->vertex_ids_[i];
                    unsigned int b = tessellator->vertex_ids_[i + 1];
                    unsigned int c = tessellator->vertex_ids_[i + 2];
                    if (a == b || b == c || a == c) // prevent degenerate triangles
                        continue;

                    tessellator->_add_element({a, b, c});
                }
                break;
            }

            case TESS_TRIANGLE_STRIP: {
                for (std::size_t i = 2; i < tessellator->vertex_ids_.size(); ++i) {
                    unsigned int a = tessellator->vertex_ids_[i - 2];
                    unsigned int b = tessellator->vertex_ids_[i - 1];
                    unsigned int c = tessellator->vertex_ids_[i];
                    if (a == b || b == c || a == c) // prevent degenerate triangles
                        continue;

                    if ((i - 2) % 2 == 0)
                        tessellator->_add_element({a, b, c});
                    else
                        tessellator->_add_element({b, a, c});
                }
                break;
            }

            case TESS_TRIANGLE_FAN: {
                for (std::size_t i = 2; i < tessellator->vertex_ids_.size(); ++i) {
                    unsigned int a = tessellator->vertex_ids_[0];
                    unsigned int b = tessellator->vertex_ids_[i - 1];
                    unsigned int c = tessellator->vertex_ids_[i];
                    if (a == b || b == c || a == c) // prevent degenerate triangles
                        continue;

                    tessellator->_add_element({a, b, c});
                }
                break;
            }

            case TESS_LINE_LOOP: {
                tessellator->_add_element(tessellator->vertex_ids_);
                break;
            }

            default: {
                LOG_FIRST_N(WARNING, 1) << "unknown primitive type: " << tessellator->primitive_type_;
                break;
            }
        }
    }


    // ****************************************************************************
    // Method: Tessellator::vertexCallback
    // Purpose:
    //   Vertex callback for the tessellator. This method uses the unique vertex
    //   object to translate the vertex into a vertex id (storing the vertex into
    //   the vertex list, if needed) and stores that id into the intermediate vertex
    //   ids that will be used to create triangle/contour connectivity.
    // Arguments:
    //   vertex : The triangle vertex.
    //   cbdata : Callback data that points to "this".
    // ****************************************************************************
    void Tessellator::vertexCallback(void *vertex, void *cbdata) {
        Vertex *v = reinterpret_cast<Vertex *>(vertex);
        Tessellator *tessellator = reinterpret_cast<Tessellator *>(cbdata);
        // Adds a vertex index using the vertex manager. This can cause the vertex list to grow.
        std::size_t id = reinterpret_cast<details::VertexManager *>(tessellator->vertex_manager_)->vertex_id(*v);
        tessellator->vertex_ids_.push_back(id);
    }


    // ****************************************************************************
    // Method: Tessellator::combineCallback
    // Purpose:
    //   Combine callback for the tessellator that gets called when new vertices
    //   need to be created by combining existing method.
    // ****************************************************************************
    void Tessellator::combineCallback(double coords[3],
                                      void *vertex_data[4],
                                      double weight[4], void **dataOut, void *cbdata) {
        Tessellator *tessellator = reinterpret_cast<Tessellator *>(cbdata);

        unsigned int size = tessellator->vertex_data_size_;
        Vertex v(size, -1);
        for (std::size_t i = 0; i < 3; ++i)
            v[i] = coords[i];

        // Blend the other data fields for the vertex.
        double **vd = reinterpret_cast<double **>(vertex_data);
        for (std::size_t i = 3; i < size; ++i) {
            double a = (vertex_data[0]) ? (weight[0] * vd[0][i]) : 0.;
            double b = (vertex_data[1]) ? (weight[1] * vd[1][i]) : 0.;
            double c = (vertex_data[2]) ? (weight[2] * vd[2][i]) : 0.;
            double d = (vertex_data[3]) ? (weight[3] * vd[3][i]) : 0.;
            v[i] = a + b + c + d;
        }

        *dataOut = reinterpret_cast<details::VertexManager *>(tessellator->vertex_manager_)->find_or_create(v);
    }


    void Tessellator::reset() {
        reinterpret_cast<details::VertexManager *>(vertex_manager_)->clear();

        num_elements_in_polygon_ = 0;

        elements_.clear();
        vertex_ids_.clear();
    }


}
