/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: a lightweight, easy-to-use, and efficient C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include <easy3d/viewer/tessellator_gen.h>

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

#include <iostream>
#include <map>

namespace easy3d {


    namespace details {

        // VertexManager manages the actual vertices to make sure that the points are unique.
        class VertexManager
        {
        public:
            VertexManager() {}
            ~VertexManager() {}

            const std::vector<const double*>& vertices() const { return vertices_; }

            void clear() {
                vertices_.clear();
                names_to_index_.clear();
            }

        #if 1 // duplicated vertices will be removed
            std::size_t get_vertex_id(const double *vert)
            {
                char keystr[200];
                snprintf(keystr, 200, "%12.12e,%12.12e,%12.12e", vert[0], vert[1], vert[2]);
                std::string key(keystr);
                std::map<std::string, std::size_t>::const_iterator pos = names_to_index_.find(key);
                if (pos != names_to_index_.end())
                    return pos->second;
                else {
                    std::size_t index = vertices_.size();
                    vertices_.push_back(vert);
                    names_to_index_[key] = index;
                    return index;
                }
            }

        private:
            std::vector<const double*>			vertices_;
            std::map<std::string, std::size_t>	names_to_index_;

        #else  // duplicated vertices will NOT be removed

            std::size_t get_vertex_id(const double *vert)
            {
                std::map<const double*, std::size_t>::const_iterator pos = names_to_index_.find(vert);
                if (pos != names_to_index_.end())
                    return pos->second;
                else {
                    std::size_t index = vertices_.size();
                    vertices_.push_back(vert);
                    names_to_index_[vert] = index;
                    return index;
                }
            }

        private:
            std::vector<const double*>				vertices_;
            std::map<const double*, std::size_t>	names_to_index_;

        #endif
        };
    }


    TessellatorGen::TessellatorGen()
        : primitive_type_(GL_TRIANGLES)
        , primitive_ware_oriention_(false)
        , vertex_manager_(new details::VertexManager)
        , num_triangles_in_polygon_(0)
        , vertex_data_size_(3)
    {
        // Create a tessellator object and set up its callbacks.
        tess_obj_ = gluNewTess();
        if (!tess_obj_) {
            std::cerr << "failed to create a tessellator object" << std::endl;
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
        gluTessCallback(tess_obj_, GLU_TESS_VERTEX_DATA, (GLvoid(*)())vertexCallback);
        gluTessCallback(tess_obj_, GLU_TESS_BEGIN_DATA, (GLvoid(*)())beginCallback);
        gluTessCallback(tess_obj_, GLU_TESS_END_DATA, (GLvoid(*)())endCallback);
        gluTessCallback(tess_obj_, GLU_TESS_COMBINE_DATA, (GLvoid(*)())combineCallback);
    #endif

        gluTessProperty(tess_obj_, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
        gluTessProperty(tess_obj_, GLU_TESS_TOLERANCE, 0.);
    }


    TessellatorGen::~TessellatorGen() {
        // Free all of the vertices that we allocated for the tessellator.
        for (std::size_t i = 0; i < vertex_allocs_.size(); ++i)
            free(vertex_allocs_[i]);
        vertex_allocs_.clear();

        delete vertex_manager_;

        gluDeleteTess(tess_obj_);
    }


    void TessellatorGen::set_winding_rule(WindingRule rule) {
        gluTessProperty(tess_obj_, GLU_TESS_WINDING_RULE, rule);
    }


    void TessellatorGen::begin_polygon(const vec3 &normal)
    {
        num_triangles_in_polygon_ = 0;
        gluTessNormal(tess_obj_, normal.x, normal.y, normal.z);
        gluTessBeginPolygon(tess_obj_, (void *)this);
    }


    void TessellatorGen::begin_contour()
    {
        gluTessBeginContour(tess_obj_);
    }


    void TessellatorGen::add_vertex_data(const float* data, unsigned int size) // to be flexible (any data can be provide)
    {
        vertex_data_size_ = size;
        double *newPt = allocate_vertex(vertex_data_size_);
        for (unsigned int i = 0; i < vertex_data_size_; ++i)
            newPt[i] = data[i];

        // gluTessVertex() takes 3 params: tess object, pointer to vertex coords,
        // and pointer to vertex data to be passed to vertex callback.
        // The second param is used only to perform tessellation, and the third
        // param is the actual vertex data to draw. It is usually same as the second
        // param, but It can be more than vertex coord, for example, color, normal
        // and UV coords which are needed for actual drawing.
        gluTessVertex(tess_obj_, newPt, newPt);
    }


    void TessellatorGen::add_vertex(const vec3& p)
    {
        vertex_data_size_ = 3;
        double *newPt = allocate_vertex(vertex_data_size_);
        for (unsigned int i = 0; i < vertex_data_size_; ++i)
            newPt[i] = p[i];

        // gluTessVertex() takes 3 params: tess object, pointer to vertex coords,
        // and pointer to vertex data to be passed to vertex callback.
        // The second param is used only to perform tessellation, and the third
        // param is the actual vertex data to draw. It is usually same as the second
        // param, but It can be more than vertex coord, for example, color, normal
        // and UV coords which are needed for actual drawing.
        gluTessVertex(tess_obj_, newPt, newPt);
    }


    void TessellatorGen::add_vertex(const vec3& p, const vec3& c)
    {
        vertex_data_size_ = 6;
        double *newPt = allocate_vertex(vertex_data_size_);
        for (unsigned int i = 0; i < 3; ++i) {
            newPt[i] = p[i];
            newPt[i + 3] = c[i];
        }

        // gluTessVertex() takes 3 params: tess object, pointer to vertex coords,
        // and pointer to vertex data to be passed to vertex callback.
        // The second param is used only to perform tessellation, and the third
        // param is the actual vertex data to draw. It is usually same as the second
        // param, but It can be more than vertex coord, for example, color, normal
        // and UV coords which are needed for actual drawing.
        gluTessVertex(tess_obj_, newPt, newPt);
    }


    void TessellatorGen::add_vertex(const vec3& p, const vec2& tc)
    {
        vertex_data_size_ = 5;
        double *newPt = allocate_vertex(vertex_data_size_);
        for (unsigned int i = 0; i < 3; ++i) {
            newPt[i] = p[i];
        }
        newPt[3] = tc[0];
        newPt[4] = tc[1];

        // gluTessVertex() takes 3 params: tess object, pointer to vertex coords,
        // and pointer to vertex data to be passed to vertex callback.
        // The second param is used only to perform tessellation, and the third
        // param is the actual vertex data to draw. It is usually same as the second
        // param, but It can be more than vertex coord, for example, color, normal
        // and UV coords which are needed for actual drawing.
        gluTessVertex(tess_obj_, newPt, newPt);
    }


    void TessellatorGen::add_vertex(const vec3& p, const vec3& c, const vec2& tc)
    {
        vertex_data_size_ = 8;
        double *newPt = allocate_vertex(vertex_data_size_);
        for (unsigned int i = 0; i < 3; ++i) {
            newPt[i] = p[i];
            newPt[i + 3] = c[i];
        }
        newPt[6] = tc[0];
        newPt[7] = tc[1];

        // gluTessVertex() takes 3 params: tess object, pointer to vertex coords,
        // and pointer to vertex data to be passed to vertex callback.
        // The second param is used only to perform tessellation, and the third
        // param is the actual vertex data to draw. It is usually same as the second
        // param, but It can be more than vertex coord, for example, color, normal
        // and UV coords which are needed for actual drawing.
        gluTessVertex(tess_obj_, newPt, newPt);
    }


    void TessellatorGen::end_contour()
    {
        gluTessEndContour(tess_obj_);
    }


    void TessellatorGen::end_polygon()
    {
        gluTessEndPolygon(tess_obj_);
    }


    std::size_t TessellatorGen::num_triangles_in_last_polygon() const
    {
        return num_triangles_in_polygon_;
    }


    const std::vector<const double*>& TessellatorGen::get_vertices() const {
        return vertex_manager_->vertices();
    }


    std::size_t TessellatorGen::num_triangles() const
    {
        return triangle_list_.size() / 3;
    }


    bool TessellatorGen::get_triangle(std::size_t t, std::size_t &a, std::size_t &b, std::size_t &c) const
    {
        bool ret = (t >= 0 && t < triangle_list_.size());
        if (ret) {
            a = triangle_list_[t * 3];
            b = triangle_list_[t * 3 + 1];
            c = triangle_list_[t * 3 + 2];
        }
        return ret;
    }


    // Allocates vertex memory and logs it for deletion later.
    double * TessellatorGen::allocate_vertex(unsigned int size)
    {
        double *arr = (double *)malloc(sizeof(double) * size);
        vertex_allocs_.push_back(arr);
        return arr;
    }


    std::size_t TessellatorGen::get_vertex_id(const double* vertex)
    {
        // Adds a vertex index using the vertex manager. This can
        // cause the vertex list to grow.
        std::size_t id = vertex_manager_->get_vertex_id(vertex);
        return id;
    }


    void TessellatorGen::add_triangle(std::size_t a, std::size_t b, std::size_t c)
    {
        triangle_list_.push_back(a);
        triangle_list_.push_back(b);
        triangle_list_.push_back(c);
        ++num_triangles_in_polygon_;
    }


    // ****************************************************************************
    // Method: TessellatorGen::beginCallback
    // Purpose: Begin callback for the tessellator.
    // Arguments:
    //   w: The type of primitives being created (TRIANGLES, TRIANGLE_STRIP, TRIANGLE_FAN)
    //   cbdata: Callback data that points to "this".
    // ****************************************************************************
    void TessellatorGen::beginCallback(GLenum w, void *cbdata)
    {
        TessellatorGen* tessellator = reinterpret_cast<TessellatorGen*>(cbdata);

        tessellator->primitive_type_ = w;
        tessellator->intermediate_vertex_ids_.clear();
    }

    // ****************************************************************************
    // Method: TessellatorGen::endCallback
    // Purpose:
    //   End callback for the tessellator. Translates intermediate vertices and
    //   drawing mode into triangle definitions.
    // Arguments:
    //   w: The type of primitives being created (TRIANGLES, TRIANGLE_STRIP, TRIANGLE_FAN)
    //   cbdata : Callback data that points to "this".
    // ****************************************************************************

    void TessellatorGen::endCallback(void *cbdata)
    {
        TessellatorGen* tessellator = reinterpret_cast<TessellatorGen*>(cbdata);

        // Use the primitive type and intermediate vertex ids to create triangles that
        // get put into the triangle list
        if (tessellator->primitive_type_ == GL_TRIANGLES)
        {
            for (std::size_t i = 0; i < tessellator->intermediate_vertex_ids_.size(); i += 3)
            {
                // Prevent degenerate triangles
                std::size_t a = tessellator->intermediate_vertex_ids_[i];
                std::size_t b = tessellator->intermediate_vertex_ids_[i + 1];
                std::size_t c = tessellator->intermediate_vertex_ids_[i + 2];
                if (a == b || b == c || a == c)
                    continue;

                tessellator->add_triangle(a, b, c);
            }
        }
        else if (tessellator->primitive_type_ == GL_TRIANGLE_STRIP)
        {
            for (std::size_t i = 2; i < tessellator->intermediate_vertex_ids_.size(); ++i)
            {
                std::size_t N = i - 2;
                std::size_t N_1 = i - 1;
                std::size_t N_2 = i;

                // Prevent degenerate triangles
                std::size_t a = tessellator->intermediate_vertex_ids_[N];
                std::size_t b = tessellator->intermediate_vertex_ids_[N_1];
                std::size_t c = tessellator->intermediate_vertex_ids_[N_2];
                if (a == b || b == c || a == c)
                    continue;

                int flag = (i - 2) % 2;
                if (tessellator->primitive_ware_oriention_ || flag == 0)
                    tessellator->add_triangle(a, b, c);
                else
                    tessellator->add_triangle(b, a, c);
            }
        }
        else if (tessellator->primitive_type_ == GL_TRIANGLE_FAN)
        {
            for (std::size_t i = 2; i < tessellator->intermediate_vertex_ids_.size(); ++i)
            {
                std::size_t N = 0;
                std::size_t N_1 = i - 1;
                std::size_t N_2 = i;

                // Prevent degenerate triangles
                std::size_t a = tessellator->intermediate_vertex_ids_[N];
                std::size_t b = tessellator->intermediate_vertex_ids_[N_1];
                std::size_t c = tessellator->intermediate_vertex_ids_[N_2];
                if (a == b || b == c || a == c)
                    continue;

                tessellator->add_triangle(a, b, c);
            }
        }
    }

    // ****************************************************************************
    // Method: TessellatorGen::vertexCallback
    // Purpose:
    //   Vertex callback for the tessellator. This method uses the unique vertex
    //   object to translate the vertex into a vertex id (storing the vertex into
    //   the vertex list, if needed) and stores that id into the intermediate vertex
    //   ids that will be used to create triangle connectivity.
    // Arguments:
    //   vertex : The triangle vertex.
    //   cbdata : Callback data that points to "this".
    // ****************************************************************************
    void TessellatorGen::vertexCallback(GLvoid *vertex, void *cbdata)
    {
        double* p = reinterpret_cast<double*>(vertex);
        TessellatorGen* tessellator = reinterpret_cast<TessellatorGen*>(cbdata);

        // Add the vertex to the list of vertices.
        tessellator->intermediate_vertex_ids_.push_back(tessellator->get_vertex_id(p));
    }

    // ****************************************************************************
    // Method: TessellatorGen::combineCallback
    // Purpose:
    //   Combine callback for the tessellator that gets called when vertices need
    //   to be combined.
    // ****************************************************************************
    void TessellatorGen::combineCallback(double coords[3],
        void *vertex_data[4],
        GLfloat weight[4], void **dataOut, void *cbdata)
    {
        TessellatorGen* tessellator = reinterpret_cast<TessellatorGen*>(cbdata);
        unsigned int size = tessellator->vertex_data_size_;
        double *vertex = tessellator->allocate_vertex(size);
        for (std::size_t i = 0; i < 3; ++i)
            vertex[i] = coords[i];

        // Blend the height data for the vertex.
        double **vd = (double **)vertex_data;
        for (std::size_t i = 3; i < size; ++i) {
            double a = (vertex_data[0] != 0) ? (weight[0] * vd[0][i]) : 0.;
            double b = (vertex_data[1] != 0) ? (weight[1] * vd[1][i]) : 0.;
            double c = (vertex_data[2] != 0) ? (weight[2] * vd[2][i]) : 0.;
            double d = (vertex_data[3] != 0) ? (weight[3] * vd[3][i]) : 0.;

            vertex[i] = a + b + c + d;
        }

        *dataOut = vertex;
    }


    void TessellatorGen::reset() {
        for (std::size_t i = 0; i < vertex_allocs_.size(); ++i)
            free(vertex_allocs_[i]);
        vertex_allocs_.clear();

        vertex_manager_->clear();
        triangle_list_.clear();

        intermediate_vertex_ids_.clear();
        num_triangles_in_polygon_ = 0;
    }

}
