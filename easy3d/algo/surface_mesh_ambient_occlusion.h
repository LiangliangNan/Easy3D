/** \cond LICENSE
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
 \endcond */

#ifndef EASY3D_ALGO_SURFACE_MESH_AMBIENT_OCCLUSION_H
#define EASY3D_ALGO_SURFACE_MESH_AMBIENT_OCCLUSION_H

#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    /**
     * \brief Per-vertex ambient occlusion computed by hemisphere ray casting.
     * \class SurfaceMeshAmbientOcclusion easy3d/algo/surface_mesh_ambient_occlusion.h
     * \details Unlike the screen-space AmbientOcclusion rendering effect, this is a
     *      view-independent bake: around every vertex a set of rays is cast into the
     *      cosine-weighted hemisphere of its normal, and the fraction of rays that
     *      escape the mesh within the search radius is stored. The result depends only
     *      on the geometry (not on a camera or a viewport), is deterministic, and is
     *      accurate enough to render publication-quality figures.
     *
     *      Example usage:
     *          \code
     *              auto ao = SurfaceMeshAmbientOcclusion::bake(mesh);
     *              SurfaceMeshAmbientOcclusion::colorize(mesh, ao, vec3(0.8f));
     *          \endcode
     *
     *      The mesh must be a triangle mesh; vertex normals are updated by bake().
     */
    class SurfaceMeshAmbientOcclusion {
    public:
        /**
         * \brief Bake parameters.
         */
        struct Parameters {
            /// The search radius, as a fraction of the mesh bounding-sphere radius.
            float radius_ratio = 0.1f;
            /// The number of cosine-weighted hemisphere rays per vertex.
            int sample_count = 128;
            /// The ray origin offset along the normal, as a fraction of the search radius.
            float bias_ratio = 0.001f;
            /// The number of worker threads; 0 uses the hardware concurrency.
            unsigned int thread_count = 0;
        };

        /**
         * \brief Computes per-vertex ambient occlusion with the default parameters and
         *      stores it in the vertex property "v:ao".
         * \param mesh The surface mesh (must be a triangle mesh).
         * \return The vertex property "v:ao", with values in [0, 1] (1 = fully visible).
         */
        static SurfaceMesh::VertexProperty<float> bake(SurfaceMesh *mesh);

        /**
         * \brief Computes per-vertex ambient occlusion and stores it in the vertex
         *      property "v:ao".
         * \param mesh The surface mesh (must be a triangle mesh).
         * \param parameters The bake parameters (see Parameters).
         * \return The vertex property "v:ao", with values in [0, 1] (1 = fully visible).
         * \details The bake is deterministic: the same mesh and parameters always produce
         *      the same values, independent of the thread count.
         */
        static SurfaceMesh::VertexProperty<float> bake(SurfaceMesh *mesh, const Parameters &parameters);

        /**
         * \brief Writes the ambient occlusion term to the vertex color property "v:ao_color".
         * \param mesh The surface mesh.
         * \param ao The per-vertex occlusion term returned by bake().
         * \param color The unoccluded surface color (default: white).
         * \details The resulting color is color * ao, so it can be used directly as a
         *      material or exported as vertex colors. The scalar term in "v:ao" stays
         *      intact, so the mesh can be colorized repeatedly.
         */
        static void colorize(SurfaceMesh *mesh, const SurfaceMesh::VertexProperty<float> &ao,
                             const vec3 &color = vec3(1.0f));
    };

} // namespace easy3d


#endif // EASY3D_ALGO_SURFACE_MESH_AMBIENT_OCCLUSION_H
