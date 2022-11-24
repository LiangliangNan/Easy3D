/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 ********************************************************************/

#ifndef EASY3D_ALGO_SURFACE_MESH_COMPONENTS_H
#define EASY3D_ALGO_SURFACE_MESH_COMPONENTS_H

#include <vector>

#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    /**
     * \brief A connected component of a SurfaceMesh.
     * \details Internally, A connected component of a SurfaceMesh stores four lists of SurfaceMesh elements, i.e., vertices,
     * faces, edges, and halfedges.
     * \class SurfaceMeshComponent easy3d/algo/surface_mesh_components.h
     */
    class SurfaceMeshComponent {
    public:
        typedef SurfaceMesh::Face Face;
        typedef SurfaceMesh::Vertex Vertex;
        typedef SurfaceMesh::Edge Edge;
        typedef SurfaceMesh::Halfedge Halfedge;

    public:
        explicit SurfaceMeshComponent(SurfaceMesh *mesh) : mesh_(mesh) {}

        /** Extracts connected components */
        static std::vector<SurfaceMeshComponent> extract(SurfaceMesh *mesh);

        /** Extracts a single connected component from the seed face */
        static SurfaceMeshComponent extract(SurfaceMesh *mesh, SurfaceMesh::Face seed);

        /** Extracts a single connected component from the seed vertex */
        static SurfaceMeshComponent extract(SurfaceMesh *mesh, SurfaceMesh::Vertex seed);

        const std::vector<Face> &faces() const { return faces_; }

        const std::vector<Vertex> &vertices() const { return vertices_; }

        const std::vector<Edge> &edges() const { return edges_; }

        const std::vector<Halfedge> &halfedges() const { return halfedges_; }

        std::size_t n_faces() const { return faces_.size(); }

        std::size_t n_vertices() const { return vertices_.size(); }

        std::size_t n_edges() const { return edges_.size(); }

        std::size_t n_halfedges() const { return halfedges_.size(); }

        /** Returns the surface mesh to which this component belongs. */
        SurfaceMesh *mesh() const { return mesh_; }

        /**
         * Returns the surface area of this component.
         * Internally it triangulates the face using the tessellator. So this method also works for concave faces.
         */
        float area() const;

        /** Returns the total border length of this component. */
        float border_length() const;

        /** Returns the bounding box of this component. */
        Box3 bbox() const;

        /** Translates this component by an offset vector. */
        void translate(const vec3 &offset);

        /** Constructs a surface mesh from this component. */
        SurfaceMesh *to_mesh() const;

    private:
        SurfaceMesh *mesh_;
        std::vector<Face> faces_;
        std::vector<Vertex> vertices_;
        std::vector<Halfedge> halfedges_;
        std::vector<Edge> edges_;
    };

}

#endif  // EASY3D_ALGO_SURFACE_MESH_COMPONENTS_H

