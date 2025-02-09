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

#ifndef EASY3D_ALGO_SURFACE_MESH_FACTORY_H
#define EASY3D_ALGO_SURFACE_MESH_FACTORY_H

#include <easy3d/core/surface_mesh.h>

namespace easy3d {

    /**
     * \brief Factory class to generate different types of basic shapes.
     * \class SurfaceMeshFactory easy3d/algo/surface_mesh_factory.h
     */
    class SurfaceMeshFactory {
    public:
        /**
         * \brief Generate a tetrahedron mesh.
         * \return The generated tetrahedron mesh.
         */
        static SurfaceMesh tetrahedron();

        /**
         * \brief Generate a hexahedron mesh.
         * \return The generated hexahedron mesh.
         */
        static SurfaceMesh hexahedron();

        /**
         * \brief Generate an octahedron mesh.
         * \return The generated octahedron mesh.
         */
        static SurfaceMesh octahedron();

        /**
         * \brief Generate a dodecahedron mesh.
         * \return The generated dodecahedron mesh.
         */
        static SurfaceMesh dodecahedron();

        /**
         * \brief Generate an icosahedron mesh.
         * \return The generated icosahedron mesh.
         */
        static SurfaceMesh icosahedron();

        /**
         * \brief Generate an icosphere mesh refined by a given number of subdivisions.
         *      The initial icosahedron is refined using Loop subdivision.
         * \param n_subdivisions Number of subdivisions to refine the initial icosahedron.
         * \return The generated icosphere mesh.
         * \sa SurfaceSubdivision
         */
        static SurfaceMesh icosphere(size_t n_subdivisions = 3);

        /**
         * \brief Generate a quad sphere mesh refined by a given number of subdivisions.
         *    The initial hexahedron is refined using Catmull-Clark subdivision.
         * \param n_subdivisions Number of subdivisions to refine the initial hexahedron.
         * \return The generated quad sphere mesh.
         * \sa SurfaceMeshSubdivision
         */
        static SurfaceMesh quad_sphere(size_t n_subdivisions = 3);

        /**
         * \brief Generate a UV sphere mesh with given parameters.
         * \param center The center of the sphere.
         * \param radius The radius of the sphere.
         * \param n_slices Number of slices (longitudinal divisions).
         * \param n_stacks Number of stacks (latitudinal divisions).
         * \return The generated UV sphere mesh.
         */
        static SurfaceMesh uv_sphere(const vec3 &center = vec3(0, 0, 0),
                                     float radius = 1.0, size_t n_slices = 15, size_t n_stacks = 15);

        /**
         * \brief Generate a plane mesh.
         * \details Generates a pure quad mesh in the x-y plane with origin (0,0,0) and side length 1.
         * \param resolution Number of faces in each direction. Needs to be >= 1. Default: 4.
         * \return The generated plane mesh.
         */
        static SurfaceMesh plane(size_t resolution = 4);

        /**
         * \brief Generate a cone mesh.
         * \details Generates a polygonal mesh of a cone. The circular base lies in the x-y-plane and the tip points in positive z-direction.
         * \param n_subdivisions Number of subdivisions of the base circle. Needs to be >= 3. Default: 30.
         * \param radius Radius of the base circle. Default: 1.
         * \param height Height of the cone. Default: 2.5.
         * \return The generated cone mesh.
         */
        static SurfaceMesh cone(size_t n_subdivisions = 30, float radius = 1.0, float height = 2.5);

        /**
         * \brief Generate a cylinder mesh.
         * \details Generates a polygonal mesh of a cylinder. The cylinder is oriented in z-direction.
         * \param n_subdivisions Number of subdivisions of the cylinder. Needs to be >= 3. Default: 30.
         * \param radius Radius of the cylinder. Default: 1.
         * \param height Height of the cylinder. Default: 2.5.
         * \return The generated cylinder mesh.
         */
        static SurfaceMesh cylinder(size_t n_subdivisions = 30, float radius = 1.0, float height = 2.5);

        /**
         * \brief Generate a torus mesh.
         * \details Generates a quad mesh of a torus with its major circle in the x-y plane.
         * \param radial_resolution Number of subdivisions of the major circle. Needs to be >= 3. Default: 20.
         * \param tubular_resolution Number of subdivisions along the tube. Needs to be >= 3. Default: 40.
         * \param radius Radius of the major circle. Default: 1.
         * \param thickness Thickness of the tube. Default: 0.4.
         * \return The generated torus mesh.
         */
        static SurfaceMesh
        torus(size_t radial_resolution = 20, size_t tubular_resolution = 40, float radius = 1.0, float thickness = 0.4);
    };

} // namespace easy3d

#endif  // EASY3D_ALGO_SURFACE_MESH_FACTORY_H
