/********************************************************************
 * Copyright (C) 2020-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2011-2020 the Polygon Mesh Processing Library developers.
 *
 * The code in this file is adapted from the PMP (Polygon Mesh Processing
 * Library) with modifications.
 *      https://github.com/pmp-library/pmp-library
 * The original code was distributed under a MIT-style license, see
 *      https://github.com/pmp-library/pmp-library/blob/master/LICENSE.txt
 ********************************************************************/

#ifndef EASY3D_ALGO_SURFACE_MESH_HOLE_FILLING_H
#define EASY3D_ALGO_SURFACE_MESH_HOLE_FILLING_H

#include <vector>
#include <cfloat>

#include <easy3d/core/surface_mesh.h>


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
        explicit SurfaceMeshHoleFilling(SurfaceMesh *mesh);

        /// \brief fill the hole specified by halfedge h
        bool fill_hole(SurfaceMesh::Halfedge h);

    private:
        struct Weight {
            explicit Weight(float _angle = FLT_MAX, float _area = FLT_MAX)
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

        void split_long_edges(float lmax);

        void collapse_short_edges(float lmin);

        void flip_edges();

        void relaxation();

        void fairing();

    private:
        // return i'th vertex of hole
        SurfaceMesh::Vertex hole_vertex(unsigned int i) const {
            if (i < hole_.size())
                return mesh_->target(hole_[i]);
            else {
                LOG(ERROR) << "index of hole edge is out of range";
                return SurfaceMesh::Vertex{};
            }
        }

        // return vertex opposite edge (i-1,i)
        SurfaceMesh::Vertex opposite_vertex(unsigned int i) const {
            if (i < hole_.size())
                return mesh_->target(mesh_->next(mesh_->opposite(hole_[i])));
            else {
                LOG(ERROR) << "index of hole edge is out of range";
                return SurfaceMesh::Vertex{};
            }
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