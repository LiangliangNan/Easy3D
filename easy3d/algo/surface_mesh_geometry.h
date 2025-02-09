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
#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    namespace geom {

        /**
         * \brief Compute the area of a triangle face in a mesh.
         * \param mesh The surface mesh containing the face.
         * \param f The face for which to compute the area.
         * \return The area of the triangle face.
         */
        float triangle_area(const SurfaceMesh *mesh, SurfaceMesh::Face f);

        /**
         * \brief Compute the surface area of the mesh.
         * \pre The input mesh needs to be a pure triangle mesh.
         * \param mesh The surface mesh for which to compute the surface area.
         * \return The surface area of the mesh.
         */
        float surface_area(const SurfaceMesh *mesh);

        /**
         * \brief Compute the volume of a mesh.
         * \details See "Zhang and Chen. Efficient feature extraction for 2D/3D objects in mesh representation. 2021".
         * \pre The input mesh needs to be a pure triangle mesh and must be closed.
         * \param mesh The surface mesh for which to compute the volume.
         * \return The volume of the mesh.
         */
        float volume(const SurfaceMesh *mesh);

        /**
         * \brief Compute the barycenter/centroid of a face.
         * \param mesh The surface mesh containing the face.
         * \param f The face for which to compute the centroid.
         * \return The centroid of the face.
         */
        vec3 centroid(const SurfaceMesh *mesh, SurfaceMesh::Face f);

        /**
         * \brief Compute the barycenter/centroid of the mesh.
         * \details Computed as the area-weighted mean of vertices. Assumes triangular faces.
         * \param mesh The surface mesh for which to compute the centroid.
         * \return The centroid of the mesh.
         */
        vec3 centroid(const SurfaceMesh *mesh);

        /**
         * \brief Compute the dual of a mesh.
         * \warning Changes the mesh in place. All properties are cleared.
         * \param mesh The surface mesh for which to compute the dual.
         */
        void dual(SurfaceMesh *mesh);

        /**
         * \brief Compute the cotangent weight for an edge.
         * \param mesh The surface mesh containing the edge.
         * \param e The edge for which to compute the cotangent weight.
         * \return The cotangent weight of the edge.
         */
        double cotan_weight(const SurfaceMesh *mesh, SurfaceMesh::Edge e);

        /**
         * \brief Compute the (mixed) Voronoi area of a vertex.
         * \param mesh The surface mesh containing the vertex.
         * \param v The vertex for which to compute the Voronoi area.
         * \return The Voronoi area of the vertex.
         */
        double voronoi_area(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

        /**
         * \brief Compute the barycentric Voronoi area of a vertex.
         * \param mesh The surface mesh containing the vertex.
         * \param v The vertex for which to compute the barycentric Voronoi area.
         * \return The barycentric Voronoi area of the vertex.
         */
        double voronoi_area_barycentric(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

        /**
         * \brief Compute the Laplace vector for a vertex.
         * \details Normalized by the Voronoi area.
         * \param mesh The surface mesh containing the vertex.
         * \param v The vertex for which to compute the Laplace vector.
         * \return The Laplace vector of the vertex.
         */
        vec3 laplace(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

        /**
         * \brief Compute the sum of angles around a vertex.
         * \details Used for Gaussian curvature.
         * \param mesh The surface mesh containing the vertex.
         * \param v The vertex for which to compute the angle sum.
         * \return The sum of angles around the vertex.
         */
        double angle_sum(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

        /**
         * \brief Discrete curvature information for a vertex.
         * \struct VertexCurvature easy3d/algo/surface_mesh_geometry.h
         * \details Used for vertex_curvature().
         */
        struct VertexCurvature {
            VertexCurvature() : mean(0.0), gauss(0.0), max(0.0), min(0.0) {}

            float mean;  //!< Mean curvature.
            float gauss; //!< Gaussian curvature.
            float max;   //!< Maximum curvature.
            float min;   //!< Minimum curvature.
        };

        /**
         * \brief Compute min, max, mean, and Gaussian curvature for a vertex.
         * \attention This will not give reliable values for boundary vertices.
         * \param mesh The surface mesh containing the vertex.
         * \param v The vertex for which to compute the curvature.
         * \return The curvature information of the vertex.
         */
        VertexCurvature vertex_curvature(const SurfaceMesh *mesh, SurfaceMesh::Vertex v);

    }

} // namespace easy3d


#endif  // EASY3D_ALGO_SURFACE_MESH_GEOMETRY_H