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


namespace easy3d {

    Tessellator::Tessellator()
        : vertex_allocs_()
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
        gluTessCallback(tess_obj_, GLU_TESS_BEGIN, (GLvoid(*)(...))beginCallback);
        gluTessCallback(tess_obj_, GLU_TESS_END, (GLvoid(*)(...))endCallback);
        gluTessCallback(tess_obj_, GLU_TESS_COMBINE_DATA, (GLvoid(*)(...))combineCallback);
    #elif defined(_WIN32)
        gluTessCallback(tess_obj_, GLU_TESS_VERTEX_DATA, (VOID(CALLBACK *)())vertexCallback);
        gluTessCallback(tess_obj_, GLU_TESS_BEGIN, (VOID(CALLBACK *)())beginCallback);
        gluTessCallback(tess_obj_, GLU_TESS_END, (VOID(CALLBACK *)())endCallback);
        gluTessCallback(tess_obj_, GLU_TESS_COMBINE_DATA, (VOID(CALLBACK *)())combineCallback);
    #else
        gluTessCallback(tess_obj_, GLU_TESS_VERTEX_DATA, (GLvoid(*)())vertexCallback);
        gluTessCallback(tess_obj_, GLU_TESS_BEGIN, (GLvoid(*)())beginCallback);
        gluTessCallback(tess_obj_, GLU_TESS_END, (GLvoid(*)())endCallback);
        gluTessCallback(tess_obj_, GLU_TESS_COMBINE_DATA, (GLvoid(*)())combineCallback);
    #endif

        gluTessProperty(tess_obj_, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
        gluTessProperty(tess_obj_, GLU_TESS_TOLERANCE, 0.);
    }


    Tessellator::~Tessellator() {
        // Free all of the vertices that we allocated for the tessellator.
        for (std::size_t i = 0; i < vertex_allocs_.size(); ++i)
            free(vertex_allocs_[i]);
        vertex_allocs_.clear();

        gluDeleteTess(tess_obj_);
    }


    void Tessellator::set_winding_rule(WindingRule rule) {
        gluTessProperty(tess_obj_, GLU_TESS_WINDING_RULE, rule);
    }


    void Tessellator::set_polygon_normal(const vec3& n) {
        gluTessNormal(tess_obj_, n[0], n[1], n[2]);
        glNormal3fv(n);
    }


    void Tessellator::begin_polygon(const vec3 &normal)
    {
        gluTessNormal(tess_obj_, normal.x, normal.y, normal.z);
        gluTessBeginPolygon(tess_obj_, (void *)this);
    }


    void Tessellator::begin_contour()
    {
        gluTessBeginContour(tess_obj_);
    }


    void Tessellator::add_vertex(const vec3& p)
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


    void Tessellator::add_vertex(const vec3& p, const vec3& c)
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


    void Tessellator::add_vertex(const vec3& p, const vec2& tc)
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


    void Tessellator::end_contour()
    {
        gluTessEndContour(tess_obj_);
    }


    void Tessellator::end_polygon()
    {
        gluTessEndPolygon(tess_obj_);
    }


    // Allocates vertex memory and logs it for deletion later.
    double* Tessellator::allocate_vertex(unsigned int size)
    {
        double* arr = (double*)malloc(sizeof(double) * size);
        vertex_allocs_.push_back(arr);
        return arr;
    }


    void Tessellator::beginCallback(GLenum w)
    {
        glBegin(w);
    }


    void Tessellator::endCallback()
    {
        glEnd();
    }


    void Tessellator::vertexCallback(GLvoid *vertex, void *cbdata)
    {
        Tessellator* tessellator = reinterpret_cast<Tessellator*>(cbdata);

        double* ptr = reinterpret_cast<double*>(vertex);
        if (tessellator->vertex_data_size_ == 6)
            glColor3dv(ptr + 3);
        else if (tessellator->vertex_data_size_ == 5)
            glTexCoord2dv(ptr + 3);

        glVertex3dv(ptr);
    }

    // ****************************************************************************
    // Method: Tessellator::combineCallback
    // Purpose:
    //   Combine callback for the tessellator that gets called when vertices need
    //   to be combined.
    // ****************************************************************************
    void Tessellator::combineCallback(double coords[3],
        void *vertex_data[4],
        GLfloat weight[4], void **dataOut, void *cbdata)
    {
        Tessellator* tessellator = reinterpret_cast<Tessellator*>(cbdata);
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

}
