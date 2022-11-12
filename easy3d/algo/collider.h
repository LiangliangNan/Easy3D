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


#ifndef EASY3D_ALGO_COLLIDER_H
#define EASY3D_ALGO_COLLIDER_H

#include <easy3d/core/surface_mesh.h>


namespace internal {
    class ColliderImpl;
}


namespace easy3d {

    class SurfaceMesh;

    /**
     * \brief Efficient collision detection.
     * \details This class takes two triangle meshes and their transformation matrices as input and outputs the
     *      intersecting face pairs. This implementation is a wrapper of Opcode. It can achieve real-time performance
     *      for large meshes.
     * \class Collider easy3d/algo/collider.h
     * \todo Extension to general polygonal meshes (by internally triangulate the input).
     */
    class Collider {
    public:
        /**
         * Constructor. It internally initializes the collider (by building the AABB trees) for subsequent
         * efficient collision detection.
         * @param mesh0 The first mesh (must be triangle mesh).
         * @param mesh1 The second mesh (must be triangle mesh).
         */
        Collider(SurfaceMesh* mesh0, SurfaceMesh* mesh1);
        ~Collider();

        /**
         * Performs collision detection.
         * @param t0 The transformation of the first mesh.
         * @param t1 The transformation of the second mesh.
         * @return The intersecting face pairs.
         */
        std::vector< std::pair<SurfaceMesh::Face, SurfaceMesh::Face> > detect(const mat4& t0, const mat4& t1) const;

    private:
        internal::ColliderImpl* collider_;
    };

}

#endif  // EASY3D_ALGO_COLLIDER_H