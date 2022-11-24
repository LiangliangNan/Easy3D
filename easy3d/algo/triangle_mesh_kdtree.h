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
        explicit TriangleMeshKdTree(const SurfaceMesh *mesh, unsigned int max_faces = 10, unsigned int max_depth = 30);

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
            Triangle() = default;

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