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


namespace Opcode {
    class Model;
}

namespace easy3d {

    class SurfaceMesh;
    class Collider {
    public:
        Collider(SurfaceMesh* mesh0, SurfaceMesh* mesh1);
        ~Collider();

        std::vector< std::pair<SurfaceMesh::Face, SurfaceMesh::Face> > detect(const mat4& t0, const mat4& t1) const;

    private:
        Opcode::Model* model0_;
        Opcode::Model* model1_;
    };

}

#endif  // EASY3D_ALGO_COLLIDER_H