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

#ifndef EASY3D_TESSELLATOR_H
#define EASY3D_TESSELLATOR_H

#include <easy3d/core/types.h>
#include <easy3d/viewer/opengl.h>

#include <vector>


class GLUtesselator;

namespace easy3d {

    /**
     * @brief Tessellator subdivides concave planar polygons, polygons with holes, or polygons with intersecting edges
     *        into triangles. This implementation also keeps track of the unique vertices and respective indices, so as
     *        to allow client code to take advantage of the index buffer for efficient rendering (to avoid sending
     *        duplicated vertex to GPU). Typical applications:
     *          - Tessellate concave polygons, polygons with holes, or polygons with intersecting edges;
     *          - Generate buffer data for rendering;
     *          - Triangulate non-triangle surfaces;
     *          - Stitch patches of a triangle meshes.
     */

    class Tessellator {
    public:
        struct Vertex : std::vector<double> {
            Vertex(std::size_t size = 0) : std::vector<double>(size) {}
            explicit Vertex(const Vertex *v) : std::vector<double>(v->data(), v->data() + v->size()) {}
            template<typename Vec>
            inline void append(const Vec &v) {
                for (int i = 0; i < v.size(); ++i)
                    push_back(v[i]);
            }
        };

    public:
        Tessellator();
        ~Tessellator();

        // Set the winding rule (default rule is ODD, modify if needed)
        enum WindingRule {
            ODD = 100130,   // These values are the same as in GLU.
            NONZERO = 100131,
            POSITIVE = 100132,
            NEGATIVE = 100133,
            ABS_GEQ_TWO = 100134
        };

        void set_winding_rule(WindingRule rule);

        /**
         * \brief Let the user supply the polygon normal, if known (to improve robustness). All input data is projected
         *        into a plane perpendicular to the normal before tesselation.  All output triangles are oriented CCW
         *        with respect to the normal.
         *        If the supplied normal is (0,0,0) (the default value), the normal is determined as follows. The
         *        direction of the normal, up to its sign, is found by fitting a plane to the vertices, without regard
         *        to how the vertices are connected.  It is expected that the input data lies approximately in plane;
         *        otherwise projection perpendicular to the computed normal may substantially change the geometry. The
         *        sign of the normal is chosen so that the sum of the signed areas of all input contours is non-negative
         *        (where a CCW contour has positive area).
         * \attention The supplied normal persists until it is changed by another call to this function.
         */
        void begin_polygon(const vec3 &normal = vec3(0, 0, 0));

        void begin_contour();    // a polygon can have multiple contours

        // general data
        void add_vertex(const Vertex &data);
        void add_vertex(const float *data, unsigned int size);

        // commonly used data
        void add_vertex(const vec3 &v0);
        void add_vertex(const vec3 &v0, const vec2 &t);
        void add_vertex(const vec3 &v0, const vec3 &v1);
        void add_vertex(const vec3 &v0, const vec3 &v1, const vec2 &t);
        void add_vertex(const vec3 &v0, const vec3 &v1, const vec3 &v2);
        void add_vertex(const vec3 &v0, const vec3 &v1, const vec3 &v2, const vec2 &t);

        void end_contour();

        void end_polygon();

        // the vertices of the resulted triangle set.
        const std::vector<Vertex *> &vertices() const;

        // list of triangle indices created over many calls (every consecutive 3 entries form a triangle)
        // NOTE: the indices are w.r.t. the vertex list returned by 'vertices()'
        const std::vector<unsigned int> &indices() const { return triangle_list_; }

        // get the vertex indices of the i'th triangle in the triangle list.
        // NOTE: the indices are w.r.t. the vertex list returned by 'vertices()'
        bool get_triangle(unsigned int i, unsigned int &a, unsigned int &b, unsigned int &c) const;

        // number of generated triangles
        unsigned int num_triangles() const;

        // number of triangles generated for the last polygon.
        // NOTE: must be used after calling to end_polygon();
        unsigned int num_triangles_in_last_polygon() const;

        /**
         * Clear all recorded data (triangle list and vertices) and restart index counter.
         * This function is useful if you don't want to stitch faces/components. In this case, call reset() before you
         * process each mesh faces/components. Then for each faces/components, you collect the vertices and vertex
         * indices of the triangles.
         */
        void reset();

    private:
        void add_triangle(unsigned int a, unsigned int b, unsigned int c);

        // GLU tessellator callbacks
        static void beginCallback(GLenum w, void *cbdata);
        static void endCallback(void *cbdata);
        static void vertexCallback(GLvoid *vertex, void *cbdata);
        static void
        combineCallback(GLdouble coords[3], void *vertex_data[4], GLdouble weight[4], void **dataOut, void *cbdata);

    private:
        GLUtesselator *tess_obj_;

        // The tessellator decides the most efficient primitive type while performing tessellation,
        // e.g., GL_TRIANGLES, GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP.
        GLenum primitive_type_;

        // If true, the orientations of the resulted triangles will comply with the primitive_type_
        // (decided by the tessellator), which is useful for rendering as GL_TRIANGLE_STRIP.
        // However, when tringulating a mesh, the output triangles must have the orientation as
        // the input polygons. In this case, you should set primitive_aware_orientation_ to false.
        bool primitive_aware_orientation_;

        // vertex ids in the last polygon
        std::vector<unsigned int> vertex_ids_in_polygon_;
        // the number of triangles in the last polygon
        unsigned int num_triangles_in_polygon_;

        // List of triangles created over many calls (every consecutive 3 entries form a triangle)
        std::vector<unsigned int> triangle_list_;

        void *vertex_manager_;

        unsigned int vertex_data_size_;
    };

}

#endif  // EASY3D_TESSELLATOR_H
