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
     *      faces, edges, and halfedges.
     * \class SurfaceMeshComponent easy3d/algo/surface_mesh_components.h
     */
    class SurfaceMeshComponent {
    public:
        // convenient type definitions
        typedef SurfaceMesh::Face       Face;       //!< Face type
        typedef SurfaceMesh::Vertex     Vertex;     //!< Vertex type
        typedef SurfaceMesh::Edge       Edge;       //!< Edge type
        typedef SurfaceMesh::Halfedge   Halfedge;   //!< Halfedge type

    public:
        /**
         * \brief Constructor that initializes the component with a given mesh.
         * \param mesh The surface mesh to which this component belongs.
         */
        explicit SurfaceMeshComponent(SurfaceMesh *mesh) : mesh_(mesh) {}

        /**
         * \brief Extracts connected components from the given mesh.
         * \param mesh The surface mesh from which to extract components.
         * \param descending If true, the components are sorted in descending order by the number of faces.
         * \return A vector of extracted SurfaceMeshComponent objects.
         */
        static std::vector<SurfaceMeshComponent> extract(SurfaceMesh *mesh, bool descending = true);

        /**
         * \brief Extracts a single connected component from the given seed face.
         * \param mesh The surface mesh from which to extract the component.
         * \param seed The seed face to start the extraction.
         * \return The extracted SurfaceMeshComponent object.
         */
        static SurfaceMeshComponent extract(SurfaceMesh *mesh, SurfaceMesh::Face seed);

        /**
         * \brief Extracts a single connected component from the given seed vertex.
         * \param mesh The surface mesh from which to extract the component.
         * \param seed The seed vertex to start the extraction.
         * \return The extracted SurfaceMeshComponent object.
         */
        static SurfaceMeshComponent extract(SurfaceMesh *mesh, SurfaceMesh::Vertex seed);

        /**
         * \brief Returns the list of faces in this component.
         * \return A constant reference to the vector of faces.
         */
        const std::vector<Face> &faces() const { return faces_; }

        /**
         * \brief Returns the list of vertices in this component.
         * \return A constant reference to the vector of vertices.
         */
        const std::vector<Vertex> &vertices() const { return vertices_; }

        /**
         * \brief Returns the list of edges in this component.
         * \return A constant reference to the vector of edges.
         */
        const std::vector<Edge> &edges() const { return edges_; }

        /**
         * \brief Returns the list of halfedges in this component.
         * \return A constant reference to the vector of halfedges.
         */
        const std::vector<Halfedge> &halfedges() const { return halfedges_; }

        /**
         * \brief Returns the number of faces in this component.
         * \return The number of faces.
         */
        std::size_t n_faces() const { return faces_.size(); }

        /**
         * \brief Returns the number of vertices in this component.
         * \return The number of vertices.
         */
        std::size_t n_vertices() const { return vertices_.size(); }

        /**
         * \brief Returns the number of edges in this component.
         * \return The number of edges.
         */
        std::size_t n_edges() const { return edges_.size(); }

        /**
         * \brief Returns the number of halfedges in this component.
         * \return The number of halfedges.
         */
        std::size_t n_halfedges() const { return halfedges_.size(); }

        /**
         * \brief Returns the surface mesh to which this component belongs.
         * \return A pointer to the surface mesh.
         */
        SurfaceMesh *mesh() const { return mesh_; }

        /**
         * \brief Returns the surface area of this component.
         * \details Internally it triangulates the face using the tessellator. So this method also works for concave faces.
         * \return The surface area.
         */
        float area() const;

        /**
         * \brief Returns the total border length of this component.
         * \return The total border length.
         */
        float border_length() const;

        /**
         * \brief Returns the bounding box of this component.
         * \return The bounding box.
         */
        Box3 bbox() const;

        /**
         * \brief Translates this component by an offset vector.
         * \param offset The offset vector.
         */
        void translate(const vec3 &offset);

        /**
         * \brief Constructs a surface mesh from this component.
         * \return A pointer to the constructed surface mesh.
         */
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

