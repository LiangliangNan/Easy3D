#ifndef TETRA_MESH_H
#define TETRA_MESH_H

#include <easy3d/core/model.h>

#include <vector>
#include <string>


namespace easy3d {

    /**
     * \brief Data structure representing a tetrahedral mesh.
     * \class TetraMesh easy3d/core/tetra_mesh.h
     * \todo Build the adjacency information.
     * see https://github.com/UMDGeoVis/TetraMesh/blob/master/source/Mesh.h
     */
    class TetraMesh : public Model {
    public:
        /**
         * Data structure representing a single tetrahedron.
         */
        struct Tet {
            Tet() {}
            Tet(const vec3 &v0, const vec3 &v1, const vec3 &v2, const vec3 &v3);

            vec3 &operator[](int i) { return v[i]; }
            const vec3 &operator[](int i) const { return v[i]; }

            vec3 v[4]; // The four vertices of the tetrahedron.
        };

    public:
        // Default constructor
        TetraMesh() : dirty(false) {}

        // Copy constructor
        TetraMesh(const TetraMesh &mesh) : v(mesh.v), t(mesh.t), dirty(true) {}

        // Construct from vertices and tet indices
        TetraMesh(const std::vector<vec3> &verts,
                  const std::vector<ivec4> &tets) :
                v(verts), t(tets), dirty(true) {}

        const std::vector<vec3> &points() const override { return v; };

        // Returns the position of the vertex stored at index i.
        vec3 &V(int i) { return v[i]; }

        const vec3 &V(int i) const { return v[i]; }

        // Returns the indices of the tetrahedron stored at index i.
        ivec4 &T(int i) {
            dirty = true;
            return t[i];
        }

        const ivec4 &T(int i) const { return t[i]; }

        // Returns the list of this tet mesh's vertices.
        std::vector<vec3> &verts() {
            dirty = true;
            return v;
        }

        const std::vector<vec3> &verts() const { return v; }

        // Returns the list of this tet mesh's tetrahedron indices.
        std::vector<ivec4> &tets() {
            dirty = true;
            return t;
        }

        const std::vector<ivec4> &tets() const { return t; }

        // Returns the number of vertices in the tet mesh.
        size_t vSize() const { return v.size(); }

        // Returns the number of tetrahedra in the mesh.
        size_t tSize() const { return t.size(); }

        // Returns the tetrahedron stored at index t.
        Tet getTet(int t) const;

        // Fills the given list with all indices of all the vertices
        // that are adjacent to the given vertex.
        void getAdjacentVertices(int vertex, std::vector<int> &vertices) const;

        // Fills the given list with all the tets incident to the given vertex.
        void getIncidentTets(int vertex, std::vector<Tet> &tets) const;

        // Fills the given list with the indices of tets incident to vertex.
        void getIncidentTetIndices(int vertex, std::vector<int> &tetIndices) const;

        // Clean up the mesh, getting rid of unused vertices.
        // Tetrahedral indices adjusted accordingly.
        void compactMesh();

        // Extract boundary vertices and triangles from the tet mesh.
        // Vertices will be returned in boundary_verts as indices into the
        // tet mesh vertex list v.
        // Triangles are stored in boundary_tris.  These are also indices into v,
        // NOT as indices into boundary_verts!!!
        // Assumes that the tet mesh is well-formed and closed...
        void getBoundary(std::vector<int> &boundary_verts,
                         std::vector<ivec3> &boundary_tris) const;

        // Extract boundary vertices and triangles from the tet mesh.
        // Vertices will be copied into boundary_verts.
        // Triangles are stored in boundary_tris.  These are indices into
        // boundary_verts.
        // Assumes that the tet mesh is well-formed and closed...
        void getBoundary(std::vector<vec3> &boundary_verts,
                         std::vector<ivec3> &boundary_tris) const;

        // Reads a TetraMesh (.TET format) from a file with the given name.
        bool read(const std::string &file_name);

        // Write this TetraMesh to file with the given name.
        // The output file will be of the .TET format.
        bool save(const std::string &file_name) const;

        /** \brief Prints the names of all properties to an output stream (e.g., std::cout). */
        void property_stats(std::ostream &output) const override {}

    private:
        std::vector<vec3> v;
        std::vector<ivec4> t;

        // Stores the indices of the tetrahedra incident to each vertex.
        std::vector<std::vector<int> > incidenceMap;
        // Indicates whether the incidence map may need to be rebuilt.
        bool dirty;

        // Re-create the incidence map from the current v and t lists.
        // This is const because we need to be able to call it from
        // getIncidentTets(), which is const.
        // incidenceMap and dirty are mutable as a result.
        void buildIncidenceMap();
    };

}

#endif  // TETRA_MESH_H
