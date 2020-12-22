/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#ifndef EASY3D_ALGO_SURFACE_MESH_TETRAHEDRALIZATION_H
#define EASY3D_ALGO_SURFACE_MESH_TETRAHEDRALIZATION_H

#include <string>


class tetgenio;

namespace easy3d {

    class SurfaceMesh;
    class PolyMesh;

    /// \brief Generate quality tetrahedralization from closed shells.
    /// \class SurfaceMeshTetrehedralization easy3d/algo/surface_mesh_tetrahedralization.h
    class SurfaceMeshTetrehedralization {
    public:
        SurfaceMeshTetrehedralization();
        ~SurfaceMeshTetrehedralization();

        void set_add_steiner_points_on_exterior_boundary(bool x) {
            add_steiner_points_on_exterior_boundary_ = x;
        }

        void set_add_steiner_points_on_interior_boundary(bool x) {
            add_steiner_points_on_interior_boundary_ = x;
        }

        void set_max_tet_shape(double x) { max_tet_shape_ = x; }

        void set_max_tet_volume(double x) { max_tet_volume_ = x; }

        void set_tag_regions(bool x) { tag_regions_ = x; }

        /** If specified, overrides all other options. */
        void set_command_line(const std::string &x) { cmdline_ = x; }

        PolyMesh* tetrahedralize(SurfaceMesh *mesh);

    protected:
        tetgenio* to_tetgen_surface(SurfaceMesh* mesh);
        PolyMesh* to_easy3d_poly_mesh(tetgenio* volume);

    private:

        bool add_steiner_points_on_exterior_boundary_;
        bool add_steiner_points_on_interior_boundary_;
        bool tag_regions_;
        double max_tet_shape_;
        double max_tet_volume_;
        std::string cmdline_;
    };

}

#endif  // EASY3D_ALGO_SURFACE_MESH_TETRAHEDRALIZATION_H
