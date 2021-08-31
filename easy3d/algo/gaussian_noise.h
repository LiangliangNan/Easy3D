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

#ifndef EASY3D_ALGO_GAUSSIAN_NOISE_H
#define EASY3D_ALGO_GAUSSIAN_NOISE_H


namespace easy3d {

    class SurfaceMesh;
    class PointCloud;

    /// \brief Add Gaussian noise to 3D models.
    /// \class GaussianNoise easy3d/algo/gaussian_noise.h
    class GaussianNoise {
    public:

        /**
         * \brief Add Gaussian noise (that has a normal distribution) to the surface mesh.
         * @param mesh The surface mesh.
         * @param sigma The standard deviation of the noise distribution. So about 68 percent of the noise values are
         *              within one standard deviation of the mean (mathematically, μ ± σ, where μ is the arithmetic
         *              mean), about 95 percent are within two standard deviations (μ ± 2σ).
         */
        static void apply(SurfaceMesh *mesh, float sigma);

        /**
         * \brief Add Gaussian noise (that has a normal distribution) to a point cloud.
         * @param cloud The point cloud.
         * @param sigma The standard deviation of the noise distribution. So about 68 percent of the noise values are
         *              within one standard deviation of the mean (mathematically, μ ± σ, where μ is the arithmetic
         *              mean), about 95 percent are within two standard deviations (μ ± 2σ).
         */
        static void apply(PointCloud *cloud, float sigma);
    };

}


#endif  // EASY3D_ALGO_GAUSSIAN_NOISE_H