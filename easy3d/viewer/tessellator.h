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

#ifndef EASY3D_TESSELLATOR_RENDERER_H
#define EASY3D_TESSELLATOR_RENDERER_H

#include <easy3d/core/types.h>
#include <easy3d/viewer/opengl.h>

#include <vector>


class GLUtesselator;

namespace easy3d {

    /**
     * @brief Tessellator provides encapsulation and a mechanism for the resulting
     *        triangles to be rendered directly.
     * @details Tessellation is used for subdividing concave planar polygons, polygons
     *          with holes, or polygons with intersecting edges into triangles. This
     *          class is based on the GLU tessellator object that can convert polygons
     *          into triangles.
     * \attention Direct rendering is deprecated. For a better performance, you're
     *            strongly recommended to use Tessellator_gen to create triangulations
     *            before sending the data to GPU.
     * @todo Current implementation menages the memory of the new allocated vertices.
     *       Check the GLUtesselator source code if the vertex memory is maintained.
     */

    class Tessellator
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
        Tessellator();
        ~Tessellator();

        // Set the winding rule (default rule is ODD, modify if needed)
        void set_winding_rule(WindingRule rule); //

        void set_polygon_normal(const vec3& normal);

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
        void add_vertex(const vec3& point);
        void add_vertex(const vec3& point, const vec3& color);		// with color
        void add_vertex(const vec3& point, const vec2& texcoord);	// with texture coordinate
        void end_contour();
        void end_polygon();

    private:
        // Allocates vertex memory and logs it for deletion later.
        double* allocate_vertex(unsigned int size);

        // GLU tessellator callbacks
        static void beginCallback(GLenum w);
        static void endCallback();
        static void vertexCallback(GLvoid *vertex, void *cbdata);
        static void combineCallback(GLdouble coords[3],
                                    void *vertex_data[4],
                                    GLfloat weight[4], void **dataOut, void *cbdata
                                    );
    private:
        GLUtesselator*				tess_obj_;

        // vertices allocated due to tessellation (including existing ones and new ones)
        std::vector<double*>		vertex_allocs_;

        unsigned int vertex_data_size_;
    };

}

#endif
