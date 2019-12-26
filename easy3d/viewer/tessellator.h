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
     * @todo Current implementation menages the memory of the new allocated vertices.
     *       Check the GLUtesselator source code if the vertex memory is maintained.
     */

    class Tessellator
    {
    public:
        enum WindingRule {
            ODD,
            NONZERO,
            POSITIVE,
            NEGATIVE,
            ABS_GEQ_TWO
        };

    public:
        Tessellator();
        ~Tessellator();

        // Set the winding rule (default rule is ODD, modify if needed)
        void set_winding_rule(WindingRule rule); //

        void set_polygon_normal(const vec3& normal);

        // polygon functions
        void begin_polygon();
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
