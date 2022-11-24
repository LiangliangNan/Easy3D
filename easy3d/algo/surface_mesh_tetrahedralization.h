/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 ********************************************************************/

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
        ~SurfaceMeshTetrehedralization() = default;

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
         * \note A negative value indicates no such a constraint.
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
        tetgenio* to_tetgen_surface(SurfaceMesh* mesh) const;
        PolyMesh* to_easy3d_poly_mesh(tetgenio* volume) const;

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
