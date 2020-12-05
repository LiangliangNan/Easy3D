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

#ifndef EASY3D_ALGO_SURFACE_MESH_GEOMETRY_H
#define EASY3D_ALGO_SURFACE_MESH_GEOMETRY_H


#include <easy3d/core/types.h>
#include <easy3d/core//surface_mesh.h>


namespace easy3d {

    namespace geom {

        /** \brief compute area of triangle f    */
        float triangle_area(const SurfaceMesh *mesh, SurfaceMesh::Face f);

        /** \brief surface area of the mesh (assumes triangular faces)    */
        float surface_area(const SurfaceMesh *mesh);

        /** \brief barycenter/centroid of a face    */
        vec3 centroid(const SurfaceMesh *mesh, SurfaceMesh::Face f);

        /** \brief barycenter/centroid of mesh, computed as area-weighted mean of vertices.    */
        /** \note assumes triangular faces.    */
        vec3 centroid(const SurfaceMesh *mesh);

        /** \brief compute the cotangent weight for edge e    */
        double cotan_weight(const SurfaceMesh *mesh, SurfaceMesh::Edge e);

        /** \brief compute (mixed) Voronoi area of vertex v    */
        double voronoi_area(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

        /** \brief compute barycentric Voronoi area of vertex v    */
        double voronoi_area_barycentric(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

        /** \brief compute Laplace vector for vertex v (normalized by Voronoi area)    */
        vec3 laplace(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

        /** \brief compute the sum of angles around vertex v (used for Gaussian curvature)    */
        float angle_sum(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

        /** \brief discrete curvature information for a vertex. used for vertex_curvature()    */
        struct VertexCurvature {
            VertexCurvature() : mean(0.0), gauss(0.0), max(0.0), min(0.0) {}

            float mean;
            float gauss;
            float max;
            float min;
        };

        /** \brief compute min, max, mean, and Gaussian curvature for vertex v. */
        /** \attention This will not give reliable values for boundary vertices.    */
        VertexCurvature vertex_curvature(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

    }

} // namespace easy3d


#endif  // EASY3D_ALGO_SURFACE_MESH_GEOMETRY_H