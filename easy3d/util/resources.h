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

#ifndef EASY3D_UTIL_RESOURCES_H
#define EASY3D_UTIL_RESOURCES_H

#include <easy3d/core/types.h>
#include <easy3d/util/export.h>


namespace easy3d {

    /**
     * Initializes Easy3D (both resources directory and logging). It internally calls (and is thus identical to calling)
     * both \c logging::initialize(...) and \c resource::initialize(...).
     * @param use_log_file \c True to create a log file along the executable program.
     * @param resource_dir The resources directory containing color maps, shaders, textures, fonts, etc.
     *      \c Easy3D_RESOURCE_DIR (default value) is the resources directory coming with the Easy3D distribution.
     * @sa resource::initialize(), logging::initialize(),
     */
    void initialize(bool use_log_file = false, const std::string& resource_dir = Easy3D_RESOURCE_DIR);


    /// \brief Resources used in Easy3D.
    /// \namespace easy3d::resource
    namespace resource {

        /// The vertices of a bunny model.
        /// \see bunny_indices
        extern const EASY3D_UTIL_EXPORT std::vector<vec3>    bunny_vertices;
        /// The triangle faces of a bunny model. Each consecutive 3 values denote the vertex indices of a triangle.
        /// \see bunny_vertices
        extern const EASY3D_UTIL_EXPORT std::vector<unsigned int>     bunny_indices;

        /// A 1D array storing a gradually changing color table. Each consecutive 3 elements represent an RGB color.
        extern const EASY3D_UTIL_EXPORT std::vector<unsigned char> cold_warm_colors;

        /// Sets the resource directory (containing color maps, shaders, textures, fonts, etc.)
        extern void initialize(const std::string& resource_dir = Easy3D_RESOURCE_DIR);

		/// Returns the resource directory (containing color maps, shaders, textures, fonts, etc.)
        extern const std::string directory();

    } // namespace resource

} // namespace easy3d


#endif // EASY3D_UTIL_RESOURCES_H
