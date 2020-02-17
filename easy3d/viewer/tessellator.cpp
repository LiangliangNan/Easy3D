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
#include <easy3d/util/logging.h>

#ifdef _WIN32
#include <Windows.h>
#else
#define CALLBACK
#endif

#  if defined(__APPLE__) && defined(__MACH__)
#    include <OpenGL/glu.h>
#  else
#    include <GL/glu.h>
#  endif

#include <vector>
#include <unordered_map>
#include <functional>


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

            Tessellator::Vertex *find_or_allocate_vertex(const Tessellator::Vertex& v) {
                std::size_t key = hash(v);
                auto pos = hash_table_.find(key);
                if (pos == hash_table_.end()) {
                    auto vertex = new Tessellator::Vertex(v);
                    hash_table_[key] = unique_vertices_.size();
                    unique_vertices_.push_back(vertex);
                    return vertex;
                } else
                    return unique_vertices_[pos->second];
            }

            void clear() {
                for (auto vertex : unique_vertices_)
                    delete vertex;
                unique_vertices_.clear();
                hash_table_.clear();
            }

            std::size_t vertex_id(const Tessellator::Vertex& v) {
                return hash_table_[hash(v)];
            }

        private:
            template<class T>
            inline void hash_combine(std::size_t &seed, T const &v) const {
                std::hash<T> hasher;
                seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }

            inline std::size_t hash(const Tessellator::Vertex& v) const {
                std::size_t seed = 0;
                std::hash<double> hasher;
                for (auto f : v) {
                    hash_combine(seed, hasher(f));
                }
                return seed;
            }

        private:

            std::vector<Tessellator::Vertex *> unique_vertices_;

            // key -> index
            std::unordered_map<std::size_t, std::size_t> hash_table_;
        };
    }

    Tessellator::Tessellator()
            : primitive_type_(GL_TRIANGLES), primitive_aware_orientation_(false), num_triangles_in_polygon_(0),
              vertex_manager_(new details::VertexManager), vertex_data_size_(3) {
        tess_obj_ = gluNewTess();// Create a tessellator object and set up its callbacks.
        if (!tess_obj_) {
            LOG(ERROR) << "failed to create a tessellator object";
            return;
        }

#if defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ == 2 && __GNUC_PATCHLEVEL__ == 0)
        gluTessCallback(tess_obj_, GLU_TESS_VERTEX_DATA, (GLvoid(*)(...))vertexCallback);
        gluTessCallback(tess_obj_, GLU_TESS_BEGIN_DATA, (GLvoid(*)(...))beginCallback);
        gluTessCallback(tess_obj_, GLU_TESS_END_DATA, (GLvoid(*)(...))endCallback);
        gluTessCallback(tess_obj_, GLU_TESS_COMBINE_DATA, (GLvoid(*)(...))combineCallback);
#elif defined(_WIN32)
        gluTessCallback(tess_obj_, GLU_TESS_VERTEX_DATA, (VOID(CALLBACK *)())vertexCallback);
        gluTessCallback(tess_obj_, GLU_TESS_BEGIN_DATA, (VOID(CALLBACK *)())beginCallback);
        gluTessCallback(tess_obj_, GLU_TESS_END_DATA, (VOID(CALLBACK *)())endCallback);
        gluTessCallback(tess_obj_, GLU_TESS_COMBINE_DATA, (VOID(CALLBACK *)())combineCallback);
#else
        gluTessCallback(tess_obj_, GLU_TESS_VERTEX_DATA, (GLvoid(*)()) vertexCallback);
        gluTessCallback(tess_obj_, GLU_TESS_BEGIN_DATA, (GLvoid(*)()) beginCallback);
        gluTessCallback(tess_obj_, GLU_TESS_END_DATA, (GLvoid(*)()) endCallback);
        gluTessCallback(tess_obj_, GLU_TESS_COMBINE_DATA, (GLvoid(*)()) combineCallback);
#endif

        gluTessProperty(tess_obj_, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
        gluTessProperty(tess_obj_, GLU_TESS_TOLERANCE, 0.);
    }

    Tessellator::~Tessellator() {
        delete reinterpret_cast<details::VertexManager *>(vertex_manager_);
        gluDeleteTess(tess_obj_);
    }

    void Tessellator::set_winding_rule(WindingRule rule) {
        gluTessProperty(tess_obj_, GLU_TESS_WINDING_RULE, rule);
    }

    void Tessellator::begin_polygon(const vec3 &normal) {
        num_triangles_in_polygon_ = 0;
        gluTessNormal(tess_obj_, normal.x, normal.y, normal.z);
        gluTessBeginPolygon(tess_obj_, this);
    }

    void Tessellator::begin_contour() {
        gluTessBeginContour(tess_obj_);
    }

    void Tessellator::add_vertex(const Vertex &v) {
        Vertex *new_v = reinterpret_cast<details::VertexManager *>(vertex_manager_)->find_or_allocate_vertex(v);
        // gluTessVertex() takes 3 params: tess object, pointer to vertex coords,
        // and pointer to vertex data to be passed to vertex callback.
        // The second param is used only to perform tessellation, and the third
        // param is the actual vertex data to draw. It is usually same as the second
        // param, but It can be more than vertex coord, for example, color, normal
        // and UV coords which are needed for actual drawing.
        gluTessVertex(tess_obj_, new_v->data(), new_v);
    }

    void Tessellator::add_vertex(const float *data, unsigned int size) // to be flexible (any data can be provide)
    {
        Vertex vertex(size);
        for (unsigned int i = 0; i < size; ++i)
            vertex[i] = data[i];
        add_vertex(vertex);
    }

    void Tessellator::add_vertex(const vec3 &v0) {
        Vertex v;
        v.append(v0);
        add_vertex(v);
    }

    void Tessellator::add_vertex(const vec3 &v0, const vec2 &t) {
        Vertex v;
        v.append(v0);
        v.append(t);
        add_vertex(v);
    }

    void Tessellator::add_vertex(const vec3 &v0, const vec3 &v1) {
        Vertex v;
        v.append(v0);
        v.append(v1);
        add_vertex(v);
    }

    void Tessellator::add_vertex(const vec3 &v0, const vec3 &v1, const vec2 &t) {
        Vertex v;
        v.append(v0);
        v.append(v1);
        v.append(t);
        add_vertex(v);
    }

    void Tessellator::add_vertex(const vec3 &v0, const vec3 &v1, const vec3 &v2) {
        Vertex v;
        v.append(v0);
        v.append(v1);
        v.append(v2);
        add_vertex(v);
    }

    void Tessellator::add_vertex(const vec3 &v0, const vec3 &v1, const vec3 &v2, const vec2 &t) {
        Vertex v;
        v.append(v0);
        v.append(v1);
        v.append(v2);
        v.append(t);
        add_vertex(v);
    }

    void Tessellator::end_contour() {
        gluTessEndContour(tess_obj_);
    }

    void Tessellator::end_polygon() {
        gluTessEndPolygon(tess_obj_);
    }

    unsigned int Tessellator::num_triangles_in_last_polygon() const {
        return num_triangles_in_polygon_;
    }

    const std::vector<Tessellator::Vertex *> &Tessellator::vertices() const {
        return reinterpret_cast<details::VertexManager *>(vertex_manager_)->vertices();
    }

    unsigned int Tessellator::num_triangles() const {
        return triangle_list_.size() / 3;
    }

    bool Tessellator::get_triangle(unsigned int t, unsigned int &a, unsigned int &b, unsigned int &c) const {
        bool ret = (t < triangle_list_.size() / 3);
        if (ret) {
            a = triangle_list_[t * 3];
            b = triangle_list_[t * 3 + 1];
            c = triangle_list_[t * 3 + 2];
        } else
            LOG(ERROR) << "triangle index " << t << " out of range [0, " << num_triangles() << "]";

        return ret;
    }

    void Tessellator::add_triangle(unsigned int a, unsigned int b, unsigned int c) {
        triangle_list_.push_back(a);
        triangle_list_.push_back(b);
        triangle_list_.push_back(c);
        ++num_triangles_in_polygon_;
    }

    // ****************************************************************************
    // Method: Tessellator::beginCallback
    // Purpose: Begin callback for the tessellator.
    // Arguments:
    //   w: The type of primitives being created (TRIANGLES, TRIANGLE_STRIP, TRIANGLE_FAN)
    //   cbdata: Callback data that points to "this".
    // ****************************************************************************
    void Tessellator::beginCallback(GLenum w, void *cbdata) {
        Tessellator *tessellator = reinterpret_cast<Tessellator *>(cbdata);

        tessellator->primitive_type_ = w;
        tessellator->vertex_ids_in_polygon_.clear();
    }

    // ****************************************************************************
    // Method: Tessellator::endCallback
    // Purpose:
    //   End callback for the tessellator. Translates intermediate vertices and
    //   drawing mode into triangle definitions.
    // Arguments:
    //   w: The type of primitives being created (TRIANGLES, TRIANGLE_STRIP, TRIANGLE_FAN)
    //   cbdata : Callback data that points to "this".
    // ****************************************************************************

    void Tessellator::endCallback(void *cbdata) {
        Tessellator *tessellator = reinterpret_cast<Tessellator *>(cbdata);

        // Use the primitive type and intermediate vertex ids to create triangles that
        // get put into the triangle list
        if (tessellator->primitive_type_ == GL_TRIANGLES) {
            for (std::size_t i = 0; i < tessellator->vertex_ids_in_polygon_.size(); i += 3) {
                // Prevent degenerate triangles
                unsigned int a = tessellator->vertex_ids_in_polygon_[i];
                unsigned int b = tessellator->vertex_ids_in_polygon_[i + 1];
                unsigned int c = tessellator->vertex_ids_in_polygon_[i + 2];
                if (a == b || b == c || a == c)
                    continue;

                tessellator->add_triangle(a, b, c);
            }
        } else if (tessellator->primitive_type_ == GL_TRIANGLE_STRIP) {
            for (std::size_t i = 2; i < tessellator->vertex_ids_in_polygon_.size(); ++i) {
                std::size_t N = i - 2;
                std::size_t N_1 = i - 1;
                std::size_t N_2 = i;

                // Prevent degenerate triangles
                unsigned int a = tessellator->vertex_ids_in_polygon_[N];
                unsigned int b = tessellator->vertex_ids_in_polygon_[N_1];
                unsigned int c = tessellator->vertex_ids_in_polygon_[N_2];
                if (a == b || b == c || a == c)
                    continue;

                int flag = (i - 2) % 2;
                if (tessellator->primitive_aware_orientation_ || flag == 0)
                    tessellator->add_triangle(a, b, c);
                else
                    tessellator->add_triangle(b, a, c);
            }
        } else if (tessellator->primitive_type_ == GL_TRIANGLE_FAN) {
            for (std::size_t i = 2; i < tessellator->vertex_ids_in_polygon_.size(); ++i) {
                std::size_t N = 0;
                std::size_t N_1 = i - 1;
                std::size_t N_2 = i;

                // Prevent degenerate triangles
                unsigned int a = tessellator->vertex_ids_in_polygon_[N];
                unsigned int b = tessellator->vertex_ids_in_polygon_[N_1];
                unsigned int c = tessellator->vertex_ids_in_polygon_[N_2];
                if (a == b || b == c || a == c)
                    continue;

                tessellator->add_triangle(a, b, c);
            }
        }
    }

    // ****************************************************************************
    // Method: Tessellator::vertexCallback
    // Purpose:
    //   Vertex callback for the tessellator. This method uses the unique vertex
    //   object to translate the vertex into a vertex id (storing the vertex into
    //   the vertex list, if needed) and stores that id into the intermediate vertex
    //   ids that will be used to create triangle connectivity.
    // Arguments:
    //   vertex : The triangle vertex.
    //   cbdata : Callback data that points to "this".
    // ****************************************************************************
    void Tessellator::vertexCallback(GLvoid *vertex, void *cbdata) {
        Vertex *v = reinterpret_cast<Vertex *>(vertex);
        Tessellator *tessellator = reinterpret_cast<Tessellator *>(cbdata);
        // Adds a vertex index using the vertex manager. This can cause the vertex list to grow.
        std::size_t id = reinterpret_cast<details::VertexManager *>(tessellator->vertex_manager_)->vertex_id(*v);
        tessellator->vertex_ids_in_polygon_.push_back(id);
    }

    // ****************************************************************************
    // Method: Tessellator::combineCallback
    // Purpose:
    //   Combine callback for the tessellator that gets called when vertices need
    //   to be combined.
    // ****************************************************************************
    void Tessellator::combineCallback(double coords[3],
                                      void *vertex_data[4],
                                      GLdouble weight[4], void **dataOut, void *cbdata) {
        Tessellator *tessellator = reinterpret_cast<Tessellator *>(cbdata);

        unsigned int size = tessellator->vertex_data_size_;
        Vertex v(size);
        for (std::size_t i = 0; i < 3; ++i)
            v[i] = coords[i];

        // Blend the height data for the vertex.
        double **vd = reinterpret_cast<double **>(vertex_data);
        for (std::size_t i = 3; i < size; ++i) {
            double a = (vertex_data[0]) ? (weight[0] * vd[0][i]) : 0.;
            double b = (vertex_data[1]) ? (weight[1] * vd[1][i]) : 0.;
            double c = (vertex_data[2]) ? (weight[2] * vd[2][i]) : 0.;
            double d = (vertex_data[3]) ? (weight[3] * vd[3][i]) : 0.;
            v[i] = a + b + c + d;
        }

        *dataOut = reinterpret_cast<details::VertexManager *>(tessellator->vertex_manager_)->find_or_allocate_vertex(v);
    }

    void Tessellator::reset() {
        reinterpret_cast<details::VertexManager *>(vertex_manager_)->clear();
        triangle_list_.clear();

        vertex_ids_in_polygon_.clear();
        num_triangles_in_polygon_ = 0;
    }

}
