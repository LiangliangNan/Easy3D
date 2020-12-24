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

        /**
         * Sets if Steiner points are allowed on the boundary edges and faces of the input surface. Default is true.
         * \details Disable this will preserve the input boundary edges and faces. In this case, Steiner points (if
         *      there exists any) will appear only in the interior space of the input surface.
         */
        void set_allow_steiner_points_on_boundary(bool x) {
            allow_steiner_points_on_boundary_ = x;
        }

        /**
         * Sets the maximum allowable radius-edge ratio. Default value is 2.0.
         * The value controls how new points can be added to improve the mesh quality.
         * More complicated constraints can be set by using set_command_line(). See the "-q" switch in tetgen manual.
         * http://wias-berlin.de/software/tetgen/1.5/doc/manual/manual005.html#cmd-q
         * \sa set_max_dihedral_angle()
         */
        void set_max_tet_shape(double x) { max_tet_shape_ = x; }

        /**
         * Sets the minimum allowable dihedral angle. Default value is 0.
         * The value controls how new points can be added to improve the mesh quality.
         * More complicated constraints can be set by using set_command_line(). See the "-q" switch in tetgen manual.
         * http://wias-berlin.de/software/tetgen/1.5/doc/manual/manual005.html#cmd-q
         * \sa set_max_tet_shape()
         */
        void set_min_dihedral_angle(double x) { min_dihedral_angle_ = x; }

        /**
         * Sets the maximum volume constraint on all tetrahedra. Default value is -1 (no max volume constraint).
         * \details A positive value indicates that no tetrahedra is generated whose volume is larger than this value.
         * \note A negagive value indicates no such a constraint.
         * More complicated constraints can be set by using set_command_line(). See the "-a" switch in tetgen manual.
         * http://wias-berlin.de/software/tetgen/1.5/doc/manual/manual005.html#cmd-a
         */
        void set_max_tet_volume(double x) { max_tet_volume_ = x; }

        /**
         * If enabled, assigns an additional attribute (an integer number) to each tetrahedron that identifies to what
         * facet-bounded region it belongs. In the output mesh, all tetrahedra in the same region will get a
         * corresponding non-zero attribute.
         */
        void set_tag_regions(bool x) { tag_regions_ = x; }

        /** If specified, overrides all other options. */
        void set_command_line(const std::string &x) { command_line_ = x; }

        /** Performs tetrahedralization on the input mesh. */
        PolyMesh* apply(SurfaceMesh *mesh);

    protected:
        tetgenio* to_tetgen_surface(SurfaceMesh* mesh);
        PolyMesh* to_easy3d_poly_mesh(tetgenio* volume);

    private:
        bool allow_steiner_points_on_boundary_;
        bool tag_regions_;
        double max_tet_shape_;
        double min_dihedral_angle_;
        double max_tet_volume_;
        std::string command_line_;
    };

}

#endif  // EASY3D_ALGO_SURFACE_MESH_TETRAHEDRALIZATION_H
