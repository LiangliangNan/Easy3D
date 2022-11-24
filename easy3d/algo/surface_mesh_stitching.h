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

#ifndef EASY3D_ALGO_SURFACE_MESH_STITCHING_H
#define EASY3D_ALGO_SURFACE_MESH_STITCHING_H


#include <vector>
#include <easy3d/core/surface_mesh.h>

namespace easy3d {


    /**
     * \brief Stitch coincident border edges of a surface mesh.
     *
     * \class SurfaceMeshStitching easy3d/algo/surface_mesh_stitching.h
     *
     * \deprecated This class only performs stitching, without reversing the orientation of components having
     * coincident but incompatible boundary cycles. It dose the same thing as Surfacer::stitch_borders()
     * To stitch incompatible boundaries please use Surfacer::merge_reversible_connected_components().
     */

    class SurfaceMeshStitching {
    public:
        SurfaceMeshStitching(SurfaceMesh *mesh);

        virtual ~SurfaceMeshStitching();

        void apply(float dist_threshold = 1e-6);

    private:

        // given a border halfedge h (its face is nullptr), return the matched border halfedge.
        //  - if multiple edges match, return the closest one;
        //  - if could not found, return an invalid halfedge.
        SurfaceMesh::Halfedge matched_border(SurfaceMesh::Halfedge h, float squared_dist_threshold) const;

        // given a border halfedge, return all border halfedges that are within a distance threshold.
        void borders_in_range(
                SurfaceMesh::Halfedge h, float squared_dist_threshold,
                std::vector<SurfaceMesh::Halfedge> &neighbors
        ) const;

        // the coordinates of a halfedge: represented by its two end points going from the xyz-lexicographically
        // smaller endpoint toward the xyz-lexicographically larger end point.
        void assign_edge_coordinate(float* coords, SurfaceMesh::Halfedge) const;

        bool lexicographically_smaller(const vec3 &p0, const vec3 &p1) const;

        float squared_distance(SurfaceMesh::Halfedge h1, SurfaceMesh::Halfedge h2) const;

    protected:
        SurfaceMesh *mesh_;

        std::vector<SurfaceMesh::Halfedge> border_edges_;

        // the coordinates of all the border edges. Each halfedge is represented by its two end points going from the
        // xyz-lexicographically smaller endpoint toward the xyz-lexicographically larger end point.
        float **coordinates_;

        void *tree_;
        int k_for_radius_search_;
    };

} // namespace easy3d


#endif  // EASY3D_ALGO_SURFACE_MESH_STITCHING_H