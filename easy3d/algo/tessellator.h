/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
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
 ********************************************************************/

#ifndef EASY3D_ALGO_TESSELLATOR_H
#define EASY3D_ALGO_TESSELLATOR_H

#include <vector>
#include <easy3d/core/types.h>

namespace easy3d {

    /**
     * \brief Tessellator subdivides concave planar polygons, polygons with holes, or polygons with intersecting edges
     * into triangles or simple contours. \par
     * \class Tessellator easy3d/algo/tessellator.h
     * This implementation is also able to keep track of the unique vertices and respective indices, which allows to
     * take advantage of the element buffer for efficient rendering (i.e., avoid sending vertices with the same
     * geometry to the GPU).
     * \par
     * Typical applications:
     *   - Tessellate concave polygons, polygons with holes, or polygons with intersecting edges;
     *   - Generate buffer data for rendering;
     *   - Triangulate non-triangle surfaces;
     *   - Stitch patches of a triangle meshes;
     *   - CSG operations
     *  @see
     *   - csg::tessellate(std::vector<Polygon2> &polygons, Tessellator::WindingRule rule);
     *   - csg::union_of(std::vector<Polygon2> &polygons);
     *   - csg::intersection_of(const Polygon2& polygon_a, const Polygon2& polygon_b, std::vector<Polygon2> &result);
     *   - csg::difference_of(const Polygon2& polygon_a, const Polygon2& polygon_b, std::vector<Polygon2> &result).
     */

    class Tessellator {
    public:
        /** A vertex carries both xyz coordinates and its attributes (e.g., color, texcoord). */
        struct Vertex : std::vector<double> {

            /**
             * \brief initialize with xyz coordinates and an optional index.
             * @param idx The index of this vertex. Providing a non-negative index allows to map a resulting vertex to 
             *            the original vertex. Any new vertex generated in the tessellation will have a negative index -1.
             */
            explicit Vertex(const vec3 &xyz, int idx = 0) : index(idx) { append(xyz); }

            /**
             * \brief initialize from a C-style array.
             * @param idx The index of this vertex. Providing a non-negative index allows to map a resulting vertex to 
             *            the original vertex. Any new vertex generated in the tessellation will have a negative index -1.
             * @attention The first 3 components must be the xyz coordinates.
             */
            template<typename FT>
            Vertex(const FT *data, std::size_t size, int idx = 0) : std::vector<double>(data, data + size), index(idx) {}

            /**
             * \brief initialize with a known size but memory is allocated without data initialization.
             * @param idx The index of this vertex. Providing a non-negative index allows to map a resulting vertex to 
             *            the original vertex. Any new vertex generated in the tessellation will have a negative index -1.
             */
            explicit Vertex(std::size_t size = 0, int idx = 0) : std::vector<double>(size), index(idx) {}

            /**
             * \brief copy constructor.
             * @param idx The index of this vertex. Providing a non-negative index allows to map a resulting vertex to 
             *            the original vertex. Any new vertex generated in the tessellation will have a negative index -1.
             */
            Vertex(const Vertex &v, int idx = 0) : std::vector<double>(v.begin(), v.end()), index(idx) {}

            /**
             * \brief append a property (e.g., color, texture coordinates) to this vertex.
             * @tparam Vec The vector type of the vertex property, e.g., vec2, vec3.
             * @param v The value of the vertex property.
             * @note The order you retrieve the properties must be the same as they were appended.
             */
            template<typename Vec>
            inline void append(const Vec &v) {
                for (int i = 0; i < v.size(); ++i)
                    push_back(v[i]);
            }

            // this can be used to carry and map to the original vertex index
            int index;
        };

    public:
        Tessellator();

        ~Tessellator();

        /// \brief The winding rule (default rule is ODD, modify if needed)
        enum WindingRule {
            WINDING_ODD = 100130,
            WINDING_NONZERO = 100131,
            WINDING_POSITIVE = 100132,
            WINDING_NEGATIVE = 100133,
            WINDING_ABS_GEQ_TWO = 100134
        };

        ///-------------------------------------------------------------------------

        /**
         * @brief Set the working mode of the tessellator.
         * @details The tessellator has two working modes and this function sets its working mode.
         * The two working modes:
         *  - Filled polygons: complex polygons are tessellated into filled polygons;
         *  - Boundary only: complex polygons are tessellated into simple line loops separating the polygon interior
         *                   and exterior
         *  @param b true for the boundary only mode and false for the filled polygons mode.
         */
        void set_boundary_only(bool b);

        /**
         * \brief Set the wining rule. The new rule will be effective until being changed by calling this function again.
         * With the winding rules, complex CSG operations can be implemented:
         *  - UNION: Draw all input contours as a single polygon. The winding number of each resulting region is the
         *           number of original polygons that cover it. The union can be extracted by using the WINDING_NONZERO
         *           or WINDING_POSITIVE winding rules. Note that with the WINDING_NONZERO winding rule, we would get
         *           the same result if all contour orientations were reversed.
         *  - INTERSECTION: This only works for two contours at a time. Draw a single polygon using two contours.
         *           Extract the result using WINDING_ABS_GEQ_TWO.
         *  - DIFFERENCE: Suppose you want to compute A diff (B union C union D). Draw a single polygon consisting of
         *           the unmodified contours from A, followed by the contours of B, C, and D, with their vertex order
         *           reversed. To extract the result, use the WINDING_POSITIVE winding rule. (If B, C, and D are the
         *           result of a BOUNDARY_ONLY operation, an alternative to reversing the vertex order is to reverse
         *           the sign of the supplied normal. See begin_polygon().
         * Explanation of the winding rule can be found here:
         * https://www.glprogramming.com/red/chapter11.html
         */
        void set_winding_rule(WindingRule rule);

        /**
         * @brief Begin the tessellation of a complex polygon.
         * @details This function lets the user supply the polygon normal if known (to improve robustness or to achieve
         *        a specific tessellation purpose like CSG). All input data is projected into a plane perpendicular to
         *        the normal before tesselation.  All output triangles are oriented CCW with respect to the normal.
         *        If the supplied normal is (0,0,0) (the default value), the normal is determined as follows. The
         *        direction of the normal, up to its sign, is found by fitting a plane to the vertices, without regard
         *        to how the vertices are connected. It is expected that the input data lies approximately in plane;
         *        otherwise projection perpendicular to the computed normal may substantially change the geometry. The
         *        sign of the normal is chosen so that the sum of the signed areas of all input contours is non-negative
         *        (where a CCW contour has positive area).
         * \attention The supplied normal persists until it is changed by another call to this function.
         */
        void begin_polygon(const vec3 &normal);
        /**
         * @brief Begin the tessellation of a complex polygon.
         * @details This function does not provide the polygon normal and let the tessellator decide.
         */
        void begin_polygon();

        /**
         * @brief Begin a contour of a complex polygon (a polygon may have multiple contours).
         */
        void begin_contour();

        /**
         * @brief Add a vertex of a contour to the tessellator.
         * @param data The vertex data.
         * @param idx The index of this vertex. Providing a non-negative index allows to map a resulting vertex to
         *            the original vertex. Any new vertex generated in the tessellation will have a negative index -1.
         */
        void add_vertex(const Vertex &data);
        /** @overload **/
        void add_vertex(const float *data, unsigned int size, int idx = 0);
        /** @overload **/
        void add_vertex(const vec3 &xyz, int idx = 0);
        /** @overload **/
        void add_vertex(const vec3 &xyz, const vec2 &t, int idx = 0);
        /** @overload **/
        void add_vertex(const vec3 &xyz, const vec3 &v1, int idx = 0);
        /** @overload **/
        void add_vertex(const vec3 &xyz, const vec3 &v1, const vec2 &t, int idx = 0);
        /** @overload **/
        void add_vertex(const vec3 &xyz, const vec3 &v1, const vec3 &v2, int idx = 0);
        /** @overload **/
        void add_vertex(const vec3 &xyz, const vec3 &v1, const vec3 &v2, const vec2 &t, int idx = 0);

        /**
         * \brief Finish the current contour of a polygon.
         */
        void end_contour();

        /**
         * \brief Finish the current polygon.
         */
        void end_polygon();

        ///-------------------------------------------------------------------------

        /**
         * \brief The list of vertices in the result.
         */
        const std::vector<Vertex *> &vertices() const;

        /**
         * \brief The list of elements (triangle or contour) created over many calls. Each element is represented by
         * its vertex indices.
         */
        const std::vector<std::vector<unsigned int> > &elements() const { return elements_; }

        /**
         * \brief The number of elements (triangle or contour) in the last polygon.
         * @note must be used after call to end_polygon() and before the next call to begin_polygon().
         */
        unsigned int num_elements_in_polygon() const { return num_elements_in_polygon_; }


        ///-------------------------------------------------------------------------

        /**
         * \brief Clear all recorded data (triangle list and vertices) and restart index counter.
         * This function is useful if you want to selectively stitch faces/components. In this case, call reset()
         * before you process each set. Then for each set, you collect the vertices and vertex indices of the triangles.
         */
        void reset();

    private:
        void _add_element(const std::vector<unsigned int> &element);

        // GLU tessellator callbacks
        static void beginCallback(unsigned int w, void *cbdata);
        static void endCallback(void *cbdata);
        static void vertexCallback(void *vertex, void *cbdata);
        static void combineCallback(const double coords[3], void *vertex_data[4], const float weight[4], void **dataOut, void *cbdata);

    private:
        void *tess_obj_;
        void *vertex_manager_;

        // The tessellator decides the most efficient primitive type while performing tessellation.
        unsigned int primitive_type_; // GL_TRIANGLES, GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP

        // The list of elements (triangle or contour) created over many calls. Each entry is the vertex indices of the
        // element.
        std::vector<std::vector<unsigned int> > elements_;

        // The growing number of elements (triangle or contour) in the current polygon.
        unsigned int num_elements_in_polygon_;

        // The vertex indices (including the original ones and the new vertices) of the current polygon.
        std::vector<unsigned int> vertex_ids_;

        // The length of the vertex data. Used to handle user provided data in the combine function.
        unsigned int vertex_data_size_;
    };


    namespace csg {
        /**
         * \brief Tessellate a set of polygons of an unknown structure into simple contours according to the winding
         * rule. Useful for complex CSG operations.
         * The resulting polygons will have the following properties:
         *  - free of intersections,
         *  - CCW contours define the outer boundary and CW contours define holes.
         * @param polygons The input polygons, and the result on return.
         * @rule The winding rule to determine the interior and exterior of the complex shape.
         */
        void tessellate(std::vector<Polygon2> &polygons, Tessellator::WindingRule rule);

        /**
         * \brief Compute the union of a set of polygons.
         * @param polygons The input polygons, and the result on return.
         */
        void union_of(std::vector<Polygon2> &polygons);

        /**
         * \brief Compute the intersection of two polygons.
         * @param polygon_a One of the two input polygons.
         * @param result The result.
         */
        void intersection_of(const Polygon2& polygon_a, const Polygon2& polygon_b, std::vector<Polygon2> &result);

        /**
         * \brief Compute the difference of two polygons (i.e., A diff B).
         * @param polygon_a The A polygon.
         * @param polygon_b The B polygon.
         * @param result The result.
         */
        void difference_of(const Polygon2& polygon_a, const Polygon2& polygon_b, std::vector<Polygon2> &result);
    }

}

#endif  // EASY3D_ALGO_TESSELLATOR_H
