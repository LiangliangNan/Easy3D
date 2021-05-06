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


#include <easy3d/algo/triangle_mesh_kdtree.h>
#include <easy3d/algo/surface_mesh_geometry.h>

#include <cfloat>


namespace easy3d {

    TriangleMeshKdTree::TriangleMeshKdTree(const SurfaceMesh *mesh, unsigned int max_faces,
                                           unsigned int max_depth) {
        // init
        root_ = new Node();
        root_->faces = new Triangles();
        SurfaceMesh::VertexProperty<vec3> points = mesh->get_vertex_property<vec3>("v:point");

        // collect triangles
        Triangle tri;
        root_->faces->reserve(mesh->n_faces());
        for (SurfaceMesh::FaceIterator fit = mesh->faces_begin();
             fit != mesh->faces_end(); ++fit) {
            SurfaceMesh::VertexAroundFaceCirculator vfit = mesh->vertices(*fit);
            tri.x[0] = points[*vfit];
            ++vfit;
            tri.x[1] = points[*vfit];
            ++vfit;
            tri.x[2] = points[*vfit];
            tri.f = *fit;
            root_->faces->push_back(tri);
        }

        // call recursive helper
        build_recurse(root_, max_faces, max_depth);
    }

    //-----------------------------------------------------------------------------

    unsigned int TriangleMeshKdTree::build_recurse(Node *node, unsigned int max_faces,
                                                   unsigned int depth) {
        // should we stop at this level ?
        if ((depth == 0) || (node->faces->size() <= max_faces))
            return depth;

        std::vector<Triangle>::const_iterator fit, fend = node->faces->end();
        unsigned int i;

        // compute bounding box
        Box3 bbox;
        for (fit = node->faces->begin(); fit != fend; ++fit) {
            for (i = 0; i < 3; ++i) {
                bbox.add_point(fit->x[i]);
            }
        }

        // split longest side of bounding box
        vec3 bb = bbox.max_point() - bbox.min_point();
        float length = bb[0];
        int axis = 0;
        if (bb[1] > length)
            length = bb[(axis = 1)];
        if (bb[2] > length)
            length = bb[(axis = 2)];

#if 1
        // split in the middle
        float split = bbox.center()[axis];
#else
        // find split position as median
        std::vector<float> v;
        v.reserve(node->faces->size() * 3);
        for (fit = node->faces->begin(); fit != fend; ++fit)
            for (i = 0; i < 3; ++i)
                v.push_back(fit->x[i][axis]]);
        std::sort(v.begin(), v.end());
        split = v[v.size() / 2];
#endif

        // create children
        auto *left = new Node();
        left->faces = new Triangles();
        left->faces->reserve(node->faces->size() / 2);
        auto *right = new Node();
        right->faces = new Triangles;
        right->faces->reserve(node->faces->size() / 2);

        // partition for left and right child
        for (fit = node->faces->begin(); fit != fend; ++fit) {
            bool l = false, r = false;

            const Triangle &t = *fit;
            if (t.x[0][axis] <= split)
                l = true;
            else
                r = true;
            if (t.x[1][axis] <= split)
                l = true;
            else
                r = true;
            if (t.x[2][axis] <= split)
                l = true;
            else
                r = true;

            if (l) {
                left->faces->push_back(t);
            }

            if (r) {
                right->faces->push_back(t);
            }
        }

        // stop here?
        if (left->faces->size() == node->faces->size() ||
            right->faces->size() == node->faces->size()) {
            // compact my memory
            node->faces->shrink_to_fit();

            // delete new nodes
            delete left;
            delete right;

            // return tree depth
            return depth;
        }

            // or recurse further?
        else {
            // free my memory
            delete node->faces;
            node->faces = nullptr;

            // store internal data
            node->axis = axis;
            node->split = split;
            node->left_child = left;
            node->right_child = right;

            // recurse to childen
            int depthLeft = build_recurse(node->left_child, max_faces, depth - 1);
            int depthRight = build_recurse(node->right_child, max_faces, depth - 1);

            return std::min(depthLeft, depthRight);
        }
    }

    //-----------------------------------------------------------------------------

    TriangleMeshKdTree::NearestNeighbor TriangleMeshKdTree::nearest(const vec3 &p) const {
        NearestNeighbor data;
        data.dist = FLT_MAX;
        data.tests = 0;
        nearest_recurse(root_, p, data);
        return data;
    }

    //-----------------------------------------------------------------------------

    void TriangleMeshKdTree::nearest_recurse(Node *node, const vec3 &point,
                                             NearestNeighbor &data) const {
        // terminal node?
        if (!node->left_child) {
            float d;
            vec3 n;

            auto fit = node->faces->begin(), fend = node->faces->end();
            for (; fit != fend; ++fit) {
                d = geom::dist_point_triangle(point, fit->x[0], fit->x[1], fit->x[2], n);
                ++data.tests;
                if (d < data.dist) {
                    data.dist = d;
                    data.face = fit->f;
                    data.nearest = n;
                }
            }
        }

        // non-terminal node
        else {
            float dist = point[node->axis] - node->split;

            if (dist <= 0.0) {
                nearest_recurse(node->left_child, point, data);
                if (fabs(dist) < data.dist)
                    nearest_recurse(node->right_child, point, data);
            } else {
                nearest_recurse(node->right_child, point, data);
                if (fabs(dist) < data.dist)
                    nearest_recurse(node->left_child, point, data);
            }
        }
    }

} // namespace easy3d
