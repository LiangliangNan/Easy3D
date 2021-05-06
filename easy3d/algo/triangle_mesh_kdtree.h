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

#ifndef EASY3D_ALGO_TRIANGLE_MESH_KDTREE_H
#define EASY3D_ALGO_TRIANGLE_MESH_KDTREE_H


#include <easy3d/core/surface_mesh.h>
#include <vector>


namespace easy3d {

    //! \brief A k-d tree for triangular surface meshes.
    /// \class TriangleMeshKdTree easy3d/algo/triangle_mesh_kdtree.h
    class TriangleMeshKdTree {
    public:
        //! \brief construct with mesh
        TriangleMeshKdTree(const SurfaceMesh *mesh, unsigned int max_faces = 10, unsigned int max_depth = 30);

        ~TriangleMeshKdTree() { delete root_; }

        //! \brief nearest neighbor information
        struct NearestNeighbor {
            float dist;
            SurfaceMesh::Face face;
            vec3 nearest;
            int tests;
        };

        //! \brief Return handle of the nearest neighbor
        NearestNeighbor nearest(const vec3 &p) const;

    private:
        // triangle stores corners and face handle
        struct Triangle {
            Triangle() {}

            Triangle(const vec3 &x0, const vec3 &x1, const vec3 &x2, SurfaceMesh::Face ff) {
                x[0] = x0;
                x[1] = x1;
                x[2] = x2;
                f = ff;
            }

            vec3 x[3];
            SurfaceMesh::Face f;
        };

        // vector of Triangle
        typedef std::vector<Triangle> Triangles;

        // Node of the tree: contains parent, children and splitting plane
        struct Node {
            Node() : faces(nullptr), left_child(nullptr), right_child(nullptr) {};

            ~Node() {
                delete faces;
                delete left_child;
                delete right_child;
            }

            unsigned char axis;
            float split;
            Triangles *faces;
            Node *left_child;
            Node *right_child;
        };

        // Recursive part of build()
        unsigned int build_recurse(Node *node, unsigned int max_handles, unsigned int depth);

        // Recursive part of nearest()
        void nearest_recurse(Node *node, const vec3 &point, NearestNeighbor &data) const;

    private:
        Node *root_;
    };

} // namespace easy3d


#endif  // EASY3D_ALGO_TRIANGLE_MESH_KDTREE_H