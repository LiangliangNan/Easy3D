/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
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
 ********************************************************************/

#ifndef EASY3D_ALGO_SURFACE_MESH_HOLE_FILLING_H
#define EASY3D_ALGO_SURFACE_MESH_HOLE_FILLING_H

#include <easy3d/core/surface_mesh.h>
#include <vector>
#include <float.h>

namespace easy3d {

    /**
     * \brief This class closes simple holes in a surface mesh.
     * \class SurfaceMeshHoleFilling easy3d/algo/surface_mesh_hole_filling.h
     * \details It closes simple holes (boundary loops of manifold vertices) by first filling the hole with an
     * angle/area-minimizing triangulation, followed by isometric remeshing, and finished by curvature-minimizing
     * fairing of the filled-in patch. See the following paper for more details:
     *  - Peter Liepa. Filling holes in meshes. SGP, pages 200–205, 2003.
     */
    class SurfaceMeshHoleFilling {
    public:
        /// \brief construct with mesh
        SurfaceMeshHoleFilling(SurfaceMesh *mesh);

        /// \brief fill the hole specified by halfedge h
        bool fill_hole(SurfaceMesh::Halfedge h);

    private:
        struct Weight {
            Weight(float _angle = FLT_MAX, float _area = FLT_MAX)
                    : angle(_angle), area(_area) {
            }

            Weight operator+(const Weight &_rhs) const {
                return Weight(std::max(angle, _rhs.angle), area + _rhs.area);
            }

            bool operator<(const Weight &_rhs) const {
                return (angle < _rhs.angle ||
                        (angle == _rhs.angle && area < _rhs.area));
            }

            float angle;
            float area;
        };

    private:
        // compute optimal triangulation of hole
        bool triangulate_hole(SurfaceMesh::Halfedge h);

        // compute the weight of the triangle (i,j,k).
        Weight compute_weight(int i, int j, int k) const;

        // refine triangulation (isotropic remeshing)
        void refine();

        void split_long_edges(const float lmax);

        void collapse_short_edges(const float lmin);

        void flip_edges();

        void relaxation();

        void fairing();

    private:
        // return i'th vertex of hole
        SurfaceMesh::Vertex hole_vertex(unsigned int i) const {
            assert(i < hole_.size());
            return mesh_->target(hole_[i]);
        }

        // return vertex opposite edge (i-1,i)
        SurfaceMesh::Vertex opposite_vertex(unsigned int i) const {
            assert(i < hole_.size());
            return mesh_->target(
                    mesh_->next(mesh_->opposite(hole_[i])));
        }

        // does interior edge (_a,_b) exist already?
        bool is_interior_edge(SurfaceMesh::Vertex _a, SurfaceMesh::Vertex _b) const;

        // triangle area
        float compute_area(SurfaceMesh::Vertex _a, SurfaceMesh::Vertex _b, SurfaceMesh::Vertex _c) const;

        // triangle normal
        vec3 compute_normal(SurfaceMesh::Vertex _a, SurfaceMesh::Vertex _b, SurfaceMesh::Vertex _c) const;

        // dihedral angle
        float compute_angle(const vec3 &_n1, const vec3 &_n2) const;

    private:
        // mesh and properties
        SurfaceMesh *mesh_;
        SurfaceMesh::VertexProperty <vec3> points_;
        SurfaceMesh::VertexProperty<bool> vlocked_;
        SurfaceMesh::EdgeProperty<bool> elocked_;

        std::vector<SurfaceMesh::Halfedge> hole_;

        // data for computing optimal triangulation
        std::vector<std::vector<Weight>> weight_;
        std::vector<std::vector<int>> index_;
    };

}

#endif  // EASY3D_ALGO_SURFACE_MESH_HOLE_FILLING_H