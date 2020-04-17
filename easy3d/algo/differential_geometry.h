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

#ifndef EASY3D_ALGO_DIFFERENTIAL_GEOMETRY_H
#define EASY3D_ALGO_DIFFERENTIAL_GEOMETRY_H


#include <easy3d/core/types.h>
#include <easy3d/core//surface_mesh.h>


namespace easy3d {


    /** clamp cotangent values as if angles are in [1, 179]    */
    inline double clamp_cot(const double v) {
        const double bound = 19.1; // 3 degrees
        return (v < -bound ? -bound : (v > bound ? bound : v));
    }

    /** clamp cosine values as if angles are in [1, 179]    */
    inline double clamp_cos(const double v) {
        const double bound = 0.9986; // 3 degrees
        return (v < -bound ? -bound : (v > bound ? bound : v));
    }

    /** compute angle between two (un-normalized) vectors    */
    inline float angle(const vec3 &v0, const vec3 &v1) {
        return atan2(norm(cross(v0, v1)), dot(v0, v1));
    }

    /** compute sine of angle between two (un-normalized) vectors    */
    inline float sin(const vec3 &v0, const vec3 &v1) {
        return norm(cross(v0, v1)) / (norm(v0) * norm(v1));
    }

    /** compute cosine of angle between two (un-normalized) vectors    */
    inline float cos(const vec3 &v0, const vec3 &v1) {
        return dot(v0, v1) / (norm(v0) * norm(v1));
    }

    /** compute cotangent of angle between two (un-normalized) vectors    */
    inline float cotan(const vec3 &v0, const vec3 &v1) {
        return clamp_cot(dot(v0, v1) / norm(cross(v0, v1)));
    }

    /** compute area of a triangle given by three points    */
    float triangle_area(const vec3 &p0, const vec3 &p1, const vec3 &p2);

    /** compute area of triangle f    */
    float triangle_area(const SurfaceMesh *mesh, SurfaceMesh::Face f);

    /** surface area of the mesh (assumes triangular faces)    */
    float surface_area(const SurfaceMesh *mesh);

    /** barycenter/centroid of a face    */
    vec3 centroid(const SurfaceMesh *mesh, SurfaceMesh::Face f);

    /** barycenter/centroid of mesh, computed as area-weighted mean of vertices.    */
    /** assumes triangular faces.    */
    vec3 centroid(const SurfaceMesh *mesh);

    /** compute the cotangent weight for edge e    */
    double cotan_weight(const SurfaceMesh *mesh, SurfaceMesh::Edge e);

    /** compute (mixed) Voronoi area of vertex v    */
    double voronoi_area(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

    /** compute barycentric Voronoi area of vertex v    */
    double voronoi_area_barycentric(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

    /** compute Laplace vector for vertex v (normalized by Voronoi area)    */
    vec3 laplace(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

    /** compute the sum of angles around vertex v (used for Gaussian curvature)    */
    float angle_sum(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

    /** discrete curvature information for a vertex. used for vertex_curvature()    */
    struct VertexCurvature {
        VertexCurvature() : mean(0.0), gauss(0.0), max(0.0), min(0.0) {}

        float mean;
        float gauss;
        float max;
        float min;
    };

    /** compute min, max, mean, and Gaussian curvature for vertex v. this will not    */
    /** give reliable values for boundary vertices.    */
    VertexCurvature vertex_curvature(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

} // namespace easy3d


#endif  // EASY3D_ALGO_DIFFERENTIAL_GEOMETRY_H