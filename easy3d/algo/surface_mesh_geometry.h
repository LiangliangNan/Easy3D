/********************************************************************
 * Copyright (C) 2020-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2011-2020 the Polygon Mesh Processing Library developers.
 *
 * The code in this file is adapted from the PMP (Polygon Mesh Processing
 * Library) with modifications.
 *      https://github.com/pmp-library/pmp-library
 * The original code was distributed under a MIT-style license, see
 *      https://github.com/pmp-library/pmp-library/blob/master/LICENSE.txt
 ********************************************************************/

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

        //! \brief Compute the volume of a mesh
        //! \details See \cite zhang_2002_efficient for details.
        //! \pre Input mesh needs to be a pure triangle mesh.
        float volume(const SurfaceMesh *mesh);

        /** \brief barycenter/centroid of a face    */
        vec3 centroid(const SurfaceMesh *mesh, SurfaceMesh::Face f);

        /** \brief barycenter/centroid of mesh, computed as area-weighted mean of vertices.    */
        /** \note assumes triangular faces.    */
        vec3 centroid(const SurfaceMesh *mesh);

        //! \brief Compute dual of a mesh.
        //! \warning Changes the mesh in place. All properties are cleared.
        void dual(SurfaceMesh *mesh);

        /** \brief compute the cotangent weight for edge e    */
        double cotan_weight(const SurfaceMesh *mesh, SurfaceMesh::Edge e);

        /** \brief compute (mixed) Voronoi area of vertex v    */
        double voronoi_area(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

        /** \brief compute barycentric Voronoi area of vertex v    */
        double voronoi_area_barycentric(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

        /** \brief compute Laplace vector for vertex v (normalized by Voronoi area)    */
        vec3 laplace(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

        /** \brief compute the sum of angles around vertex v (used for Gaussian curvature)    */
        double angle_sum(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

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