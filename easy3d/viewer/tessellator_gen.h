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

#ifndef EASY3D_TESSELLATOR_GEN_H
#define EASY3D_TESSELLATOR_GEN_H

#include <easy3d/core/types.h>
#include <easy3d/viewer/opengl.h>

#include <vector>


class GLUtesselator;

namespace easy3d {

    namespace details {
        class VertexManager;
    }

    /**
     * @brief Tessellator provides encapsulation and a mechanism for the resulting
     *        triangles to be read out and used.
     * @details Tessellation is used for subdividing concave planar polygons, polygons
     *          with holes, or polygons with intersecting edges into triangles. This
     *          class is based on the GLU tessellator object that can convert polygons
     *          into triangles.
     * @todo Current implementation can get rid of duplicated vertices based on std::map,
     *       with which the performance is not optimized. Check the GLUtesselator source
     *       code if the vertex indices are maintained.
     */

    class TessellatorGen
    {
    public:
        enum WindingRule {
            ODD         = 100130,   // The values are the same as in GLU.
            NONZERO     = 100131,
            POSITIVE    = 100132,
            NEGATIVE    = 100133,
            ABS_GEQ_TWO = 100134
        };

    public:
        TessellatorGen();
        ~TessellatorGen();

        // Set the winding rule (default rule is ODD, modify if needed)
        void set_winding_rule(WindingRule rule);

        // polygon functions
        /**
         * \brief Lets the user supply the polygon normal, if known (improve robustness).
         *        All input data is projected into a plane perpendicular to the normal
         *        before tesselation.  All output triangles are oriented CCW with respect
         *        to the normal.
         *        If the supplied normal is (0,0,0) (the default value), the normal is
         *        determined as follows. The direction of the normal, up to its sign, is
         *        found by fitting a plane to the vertices, without regard to how the
         *        vertices are connected.  It is expected that the input data lies
         *        approximately in plane; otherwise projection perpendicular to the
         *        computed normal may substantially change the geometry.  The sign of the
         *        normal is chosen so that the sum of the signed areas of all input contours
         *        is non-negative (where a CCW contour has positive area).
         * \attention The supplied normal persists until it is changed by another call to
         *            this function.
         */
        void begin_polygon(const vec3& normal = vec3(0,0,0));
        void begin_contour();	// a polygon can have multiple contours

        // general cases
        void add_vertex(const float* data, unsigned int size); // to be flexible (any data can be provide)

        // specialized using common properties
        void add_vertex(const vec3& point);
        void add_vertex(const vec3& point, const vec3& color);		// with color
        void add_vertex(const vec3& point, const vec2& texcoord);	// with texture coordinate
        void add_vertex(const vec3& point, const vec3& color, const vec2& texcoord);	// with color and texture coordinate

        void end_contour();
        void end_polygon();

        // the vertices (including the new created ones) of the triangles
        const std::vector<const double*>& get_vertices() const;

        // number of generated triangles
        std::size_t  num_triangles() const;

        // get the vertex indices of the i'th triangle in the triangle list.
        // NOTE: the indices are w.r.t. the vertex list that can be obtained
        //       using get_vertices()
        bool get_triangle(std::size_t i, std::size_t &a, std::size_t &b, std::size_t &c) const;

        // List of triangles created over many calls (every consecutive 3 entries form a triangle)
        const std::vector<std::size_t>& get_triangle_list() const { return triangle_list_; }

        // number of triangles generated for the last polygon.
        // NOTE: must be used after calling to end_polygon();
        std::size_t  num_triangles_in_last_polygon() const;

        //-------------------------------------------------------------------------------------
        // Advanced usage
        //-------------------------------------------------------------------------------------
        // clear all recorded data (triangle list and vertices) and restart index counter.
        // This function is useful if you don't want to stitch faces/components. In this case,
        // call reset() before you process each mesh component or face. Then for each component
        // or face, you collect the vertices and triangle list from the tessellator.
        void reset();

    private:
        // Allocates vertex memory and logs it for deletion later.
        double* allocate_vertex(unsigned int size);

        void add_triangle(std::size_t a, std::size_t b, std::size_t c);

        // GLU tessellator callbacks
        static void beginCallback(GLenum w, void *cbdata);
        static void endCallback(void *cbdata);
        static void vertexCallback(GLvoid *vertex, void *cbdata);
        static void combineCallback(GLdouble coords[3], void *vertex_data[4], GLdouble weight[4], void **dataOut, void *cbdata);

    private:
        GLUtesselator*				tess_obj_;

        // The tessellator decides the most efficient primitive type while performing tessellation,
        // e.g., GL_TRIANGLES, GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP.
        GLenum						primitive_type_;

        // If true, the orientations of the resulted triangles will comply with the primitive_type_
        // (decided by the tessellator), which is usefull for rendering as GL_TRIANGLE_STRIP.
        // However, when tringulating a mesh, the output tringles must have the orientation as
        // the input polygons. In this case, you should set primitive_aware_oriention_ to false.
        bool						primitive_aware_oriention_;

        // vertex ids in the last polygon
        std::vector<std::size_t>	vertex_ids_in_polygon_;
        // the number of triangles in the last polygon
        std::size_t					num_triangles_in_polygon_;

        // List of triangles created over many calls (every consecutive 3 entries form a triangle)
        std::vector<std::size_t>	triangle_list_;

        // vertices allocated due to tesselation (including existing ones and new ones)
        std::vector<double*>		vertex_allocs_;

        details::VertexManager*		vertex_manager_;

        unsigned int vertex_data_size_;
    };

}

#endif
