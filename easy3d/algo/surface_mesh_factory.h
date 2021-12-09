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
        //! Generate tetrahedron.
        static SurfaceMesh tetrahedron();

        //! Generate hexahedron.
        static SurfaceMesh hexahedron();

        //! Generate octahedron.
        static SurfaceMesh octahedron();

        //! Generate dodecahedron.
        static SurfaceMesh dodecahedron();

        //! Generate icosahedron.
        static SurfaceMesh icosahedron();

        //! \brief Generate icosphere refined by \p n_subdivisions .
        //! \details Uses Loop subdivision to refine the initial icosahedron.
        //! \sa SurfaceSubdivision
        static SurfaceMesh icosphere(size_t n_subdivisions = 3);

        //! \brief Generate quad sphere refined by \p n_subdivisions .
        //! \details Uses Catmull-Clark subdivision to refine the initial hexahedron.
        //! \sa SurfaceMeshSubdivision
        static SurfaceMesh quad_sphere(size_t n_subdivisions = 3);

        //! Generate UV sphere with given \p center, \p radius, \p n_slices, and \p n_stacks.
        static SurfaceMesh uv_sphere(const vec3 &center = vec3(0, 0, 0),
                                     float radius = 1.0, size_t n_slices = 15, size_t n_stacks = 15);

        //! \brief Generate a plane mesh.
        //! \details Generates a pure quad mesh in the x-y plane with origin (0,0,0)
        //! and side length 1.
        //! \param resolution Number of faces in each direction. Needs to be >= 1. Default: 4.
        static SurfaceMesh plane(size_t resolution = 4);

        //! \brief Generate a cone mesh.
        //! \details Generates a polygonal mesh of a cone. The circular base lies in
        //! the x-y-plane and the tip points in positive z-direction.
        //! \param n_subdivisions Number of subdivisions of the base circle. Needs to be >= 3. Default: 30.
        //! \param radius Radius of the base circle. Default: 1.
        //! \param height Height of the the cone. Default: 2.5.
        static SurfaceMesh cone(size_t n_subdivisions = 30, float radius = 1.0, float height = 2.5);

        //! \brief Generate a cylinder mesh.
        //! \details Generates a polygonal mesh of a cylinder. The cylinder is
        //! oriented in z-direction.
        //! \param n_subdivisions Number of subdivisions of the cylinder. Needs to be >= 3. Default: 30.
        //! \param radius Radius of the cylinder. Default: 1.
        //! \param height Height of the cylinder. Default: 2.5.
        static SurfaceMesh cylinder(size_t n_subdivisions = 30, float radius = 1.0, float height = 2.5);

        //! \brief Generate a torus mesh.
        //! \details Generates a quad mesh of a torus with its major circle in the x-y plane.
        //! \param radial_resolution Number of subdivisions of the major circle. Needs to be >= 3. Default: 20.
        //! \param tubular_resolution Number of subdivisions of along the tube. Needs to be >= 3. Default: 40.
        //! \param radius Radius of the major circle. Default: 1.
        //! \param thickness Thickness of the tube. Default: 0.4.
        static SurfaceMesh
        torus(size_t radial_resolution = 20, size_t tubular_resolution = 40, float radius = 1.0, float thickness = 0.4);
    };

} // namespace easy3d

#endif  // EASY3D_ALGO_SURFACE_MESH_FACTORY_H
