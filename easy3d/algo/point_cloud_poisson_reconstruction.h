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

#ifndef EASY3D_ALGO_POISSON_RECONSTRUCTION_H
#define EASY3D_ALGO_POISSON_RECONSTRUCTION_H


#include <string>


namespace easy3d {

    class PointCloud;
    class SurfaceMesh;

    /**
     * \brief Poisson surface reconstruction.
     * \class PoissonReconstruction easy3d/algo/point_cloud_poisson_reconstruction.h
     */
    class PoissonReconstruction {
    public:
        /**
         * \brief Constructor.
         */
        PoissonReconstruction();
        /**
         * \brief Destructor.
         */
        ~PoissonReconstruction();

        /**
         * \brief Set reconstruction depth.
         * \param d The maximum depth of the tree used for surface reconstruction.
         * \details This integer is the maximum depth of the tree that will be used for surface reconstruction. Running
         *      at depth d corresponds to solving on a voxel grid whose resolution is no larger than 2^d x 2^d x 2^d.
         *      Note that since the reconstructor adapts the octree to the sampling density, the specified
         *      reconstruction depth is only an upper bound. The default value for this parameter is 8.
         */
        void set_depth(int d) { depth_ = d; }

        /**
         * \brief Set the minimum number of samples.
         * \param s The minimum number of sample points that should fall within an octree node. This floating point
         *      value specifies the minimum number of sample points that should fall within an octree node as the
         *      octree construction is adapted to sampling density. For noise-free samples, small values in the range
         *      [1.0 - 5.0] can be used. For more noisy samples, larger values in the range [15.0 - 20.0] may be needed
         *      to provide a smoother, noise-reduced, reconstruction. The default value is 1.0.
         */
        void set_samples_per_node(float s) { samples_per_node_ = s; }

        /**
         * \brief Perform Poisson surface reconstruction.
         * \param cloud The input point cloud.
         * \param density_attr_name The name of the density attribute. Default is "v:density".
         * \return A pointer to the reconstructed surface mesh.
         */
        SurfaceMesh *apply(const PointCloud *cloud, const std::string &density_attr_name = "v:density") const;

        /**
        * \brief Trim the reconstructed surface model based on the density attribute.
        * \param mesh The surface mesh to be trimmed.
        * \param trim_value The density value used for trimming.
        * \param area_ratio The area ratio used for trimming.
        * \param triangulate Whether to triangulate the trimmed surface.
        * \param density_attr_name The name of the density attribute. Default is "v:density".
        * \return A pointer to the trimmed surface mesh.
        */
        static SurfaceMesh *trim(
                SurfaceMesh *mesh,
                float trim_value,
                float area_ratio,
                bool triangulate,
                const std::string &density_attr_name = "v:density"
        );

    public:
        // Other parameters for Poisson surface reconstruction algorithm. These parameters are usually not needed.
        /**
         * \brief Set the full depth of the octree.
         * \param v The full depth value.
         * \details This integer specifies the depth beyond which the octree will be adapted. At coarser depths, the octree will be complete.
         * The default value for this parameter is 5.
         */
        void set_full_depth(int v) { full_depth_ = v; }
        /**
         * \brief Set the conjugate gradient depth.
         * \param v The conjugate gradient depth value.
         */
        void set_cg_depth(int v) { cgDepth_ = v; }
        /**
         * \brief Set the scale factor.
         * \param v The scale factor value.
         */
        void set_scale(float v) { scale_ = v; }
        /**
         * \brief Set the point weight.
         * \param v The point weight value.
         * \details This floating point value specifies the importance that interpolation of the point samples is given
         *      in the formulation of the screened Poisson equation. The default value for this parameter is 4.
         */
        void set_point_weight(float v) { pointWeight_ = v; }
        /**
         * \brief Set the number of Gauss-Seidel iterations.
         * \param v The number of Gauss-Seidel iterations.
         */
        void set_gs_iter(int v) { gsIter_ = v; }
        /**
         * \brief Set the verbosity of the reconstruction process.
         * \param v True to enable verbose output, false otherwise.
         */
        void set_verbose(bool v) { verbose_ = v; }

    private:
        /*
        This integer is the maximum depth of the tree that will be used for surface
        reconstruction. Running at depth d corresponds to solving on a voxel grid
        whose resolution is no larger than 2^d x 2^d x 2^d. Note that since the
        reconstructor adapts the octree to the sampling density, the specified
        reconstruction depth is only an upper bound.
        The default value for this parameter is 8.
        */
        int depth_;    // reconstruction depth

        /*
        This integer specifies the depth beyond depth the octree will be adapted.
        At coarser depths, the octree will be complete, containing all 2^d x 2^d x 2^d nodes.
        The default value for this parameter is 5.	*/
        int full_depth_; // adaptive octree depth

        /*
        This floating point value specifies the minimum number of sample points that should
        fall within an octree node as the octree construction is adapted to sampling density.
        For noise-free samples, small values in the range [1.0 - 5.0] can be used. For more
        noisy samples, larger values in the range [15.0 - 20.0] may be needed to provide a
        smoother, noise-reduced, reconstruction. The default value is 1.0.
        */
        float samples_per_node_; // minimum number of samples

        bool triangulate_mesh_;

    private:
        // these parameters usually do not need to change
        int cgDepth_;
        float scale_;

        /*
        This floating point value specifies the importance that interpolation of the point
        samples is given in the formulation of the screened Poisson equation. The results
        of the original (unscreened) Poisson Reconstruction can be obtained by setting this
        value to 0. The default value for this parameter is 4.
        */
        float pointWeight_;    // interpolation weight

        int gsIter_;
        int threads_;
        bool verbose_;
    };

} // namespace easy3d

#endif  // EASY3D_ALGO_POISSON_RECONSTRUCTION_H
