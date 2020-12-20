#ifndef TETRA_MESH_H
#define TETRA_MESH_H

#include <easy3d/core/model.h>

#include <vector>
#include <string>


namespace easy3d {

#if 1

    typedef int VertexIndex;
    typedef int TetraIndex;

    typedef unsigned short int PositionIndex;

    static TetraIndex reservedTetraIndexes = 2;
    static TetraIndex maxTetraNumber = UINTPTR_MAX - reservedTetraIndexes;

    static TetraIndex noAdjacentTetraIndex = maxTetraNumber + 1;
    static TetraIndex uninitializedTetraIndex = maxTetraNumber + 2;



    /**
     * \brief Data structure representing a tetrahedral mesh.
     * \class TetraMesh easy3d/core/tetra_mesh.h
     */
    class TetraMesh : public Model {
    public:

        // Class storing geometrical information of vertices
        class Vertex {
        private:
            float vertexCoordinates[3]; // Coordinates
            VertexIndex index;
            TetraIndex vertexVTstar; // VT* relationship

        public:

            // Constructor
            inline Vertex() {
                vertexCoordinates[0] = 0;
                vertexCoordinates[1] = 0;
                vertexCoordinates[2] = 0;
                setVTstar(uninitializedTetraIndex);
            }

            inline Vertex(float x, float y, float z) {
                vertexCoordinates[0] = x;
                vertexCoordinates[1] = y;
                vertexCoordinates[2] = z;
                setVTstar(uninitializedTetraIndex);
            }

            // VT* setter
            inline void setVTstar(TetraIndex VTstar) {
                vertexVTstar = VTstar;
            }

            // First float getter
            inline float x() const {
                return vertexCoordinates[0];
            }

            // Second float getter
            inline float y() const {
                return vertexCoordinates[1];
            }

            // Third float getter
            inline float z() const {
                return vertexCoordinates[2];
            }

            // VT* getter
            inline TetraIndex VTstar() const {
                return vertexVTstar;
            }


            // Operators -----------
            inline bool operator==(Vertex vertex) {
                return x() == vertex.x() && y() == vertex.y() && z() == vertex.z();
            }
        };


        // Explicit representation of an edge
        class Edge {
        private:
            VertexIndex vertexes[2]; // Indices of the endpoints
        public:

            // Constructor
            inline Edge() {}

            inline Edge(VertexIndex vi1, VertexIndex vi2) {
                vertexes[0] = std::min(vi1, vi2);
                vertexes[1] = std::max(vi1, vi2);
            }

            // Smaller-index endpoint getter
            inline VertexIndex minindex() const { return vertexes[0]; }

            // Bigger-index endpoint getter
            inline VertexIndex maxindex() const { return vertexes[1]; }

            // Index of the vertex in position vPos
            inline VertexIndex EV(PositionIndex vPos) {
                return vertexes[vPos];
            }

            // Index of verex v inside the edge
            inline short vertexIndex(VertexIndex v) {
                if (vertexes[0] == v) return 0;
                if (vertexes[1] == v) return 1;
                return -1;
            }

            // Operators ----------
            inline bool operator==(Edge edge) {
                if (minindex() != edge.minindex()) return false;
                if (maxindex() != edge.maxindex()) return false;
                return true;
            }

            inline bool operator!=(Edge edge) {
                return !((*this) == (edge));
            }

            inline bool operator<(Edge edge) const {
                if (maxindex() < edge.maxindex()) return true;
                else if (maxindex() == edge.maxindex() && minindex() < edge.minindex()) return true;
                else return false;
            }

            inline bool operator>(Edge edge) const {
                if (maxindex() > edge.maxindex()) return true;
                else if (maxindex() == edge.maxindex() && minindex() > edge.minindex()) return true;
                else return false;
            }

            // Standard output operator
            inline friend std::ostream &operator<<(std::ostream &out, Edge &edge) {
                out << "Edge[(" << edge.minindex() << " " << edge.maxindex() << ")]" << std::endl;
                return out;
            }

        };


        // Explicit representation of a triangle
        class Triangle {
        private:
            VertexIndex vertexes[3]; // Vertex indices
            PositionIndex fIndex; // Position index of the face in the original tetrahedron (if appliable)

        public:

            // Constructor
            inline Triangle(VertexIndex vi1, VertexIndex vi2, VertexIndex vi3, PositionIndex index = 4) {
                vertexes[0] = std::min(std::min(vi1, vi2), vi3);
                vertexes[2] = std::max(std::max(vi1, vi2), vi3);
                if (vi1 > minindex() && vi1 < maxindex()) vertexes[1] = vi1;
                else if (vi2 > minindex() && vi2 < maxindex()) vertexes[1] = vi2;
                else vertexes[1] = vi3;
                fIndex = index;
            }

            // Null constructor
            inline Triangle() {
                vertexes[0] = vertexes[1] = vertexes[2] = 0;
                fIndex = 4;
            }

            // Swallow copy
            inline Triangle *clone() {
                return new Triangle(minindex(), middleindex(), maxindex(), faceIndex());
            }

            // Smaller-index vertex getter
            inline VertexIndex minindex() const { return vertexes[0]; }

            // Middle-index vertex getter
            inline VertexIndex middleindex() const { return vertexes[1]; }

            // Bigger-index vertex getter
            inline VertexIndex maxindex() const { return vertexes[2]; }

            // PositionIndex getter
            inline PositionIndex faceIndex() { return fIndex; }

            // Vertices getter
            inline VertexIndex FV(PositionIndex index) { return vertexes[index % 3]; }

            // Bounding edges getter
            inline Edge *FE(PositionIndex index) { return new Edge(vertexes[(index + 1) % 3], vertexes[(index + 2) % 3]); }

            // true if the triangle contains the given vertex
            inline bool contains(VertexIndex vertex) {
                for (PositionIndex pindex = 0; pindex < 3; pindex++) {
                    if (FV(pindex) == vertex) return true;
                }
                return false;
            }

            // Operators-----------
            inline bool operator==(Triangle triangle) {
                if (minindex() != triangle.minindex()) return false;
                if (middleindex() != triangle.middleindex()) return false;
                if (maxindex() != triangle.maxindex()) return false;
                return true;
            }

            inline bool operator!=(Triangle triangle) {
                return !((*this) == triangle);
            }

            inline bool operator<(Triangle tri) const {
                if (maxindex() < tri.maxindex()) return true;
                else if (maxindex() == tri.maxindex() && middleindex() < tri.middleindex()) return true;
                else if (maxindex() == tri.maxindex() && middleindex() == tri.middleindex() &&
                         minindex() < tri.minindex())
                    return true;
                else return false;
            }

            inline bool operator>(Triangle tri) const {
                if (maxindex() > tri.maxindex()) return true;
                else if (maxindex() == tri.maxindex() && middleindex() > tri.middleindex()) return true;
                else if (maxindex() == tri.maxindex() && middleindex() == tri.middleindex() &&
                         minindex() > tri.minindex())
                    return true;
                else return false;
            }

            // Standard output operator
            inline friend std::ostream &operator<<(std::ostream &out, Triangle &triangle) {
                out << "Triangle[(" << triangle.minindex() << " " << triangle.middleindex() << " " << triangle.maxindex()
                    << ")]" << std::endl;
                return out;
            }

        };


// Class storing complete topological information for tetrahedra (top simplices) with reference to geometrical information of their vertices
        class Tetra {
        private:
            VertexIndex tetraTV[4]; // Indices of geometrical information of the vertices
            TetraIndex tetraTT[4]; // Adjacency relations

        public:

            // Constructor
            inline Tetra() {
                tetraTV[0] = -1;
                tetraTV[1] = -1;
                tetraTV[2] = -1;
                tetraTV[3] = -1;
                setTT(noAdjacentTetraIndex, noAdjacentTetraIndex, noAdjacentTetraIndex, noAdjacentTetraIndex);
            }

            inline Tetra(VertexIndex a, VertexIndex b, VertexIndex c, VertexIndex d) {
                tetraTV[0] = a;
                tetraTV[1] = b;
                tetraTV[2] = c;
                tetraTV[3] = d;
                setTT(noAdjacentTetraIndex, noAdjacentTetraIndex, noAdjacentTetraIndex, noAdjacentTetraIndex);
            }

            // Vertex replacement constructor
            inline Tetra(VertexIndex Avertex, PositionIndex vertexAFinalPosition, Tetra *oldTetra) {
                for (PositionIndex ind = 0; ind <= 3; ind++) {
                    tetraTV[ind] = oldTetra->TV(ind);
                }
                tetraTV[vertexAFinalPosition] = Avertex;
                setTT(noAdjacentTetraIndex, noAdjacentTetraIndex, noAdjacentTetraIndex, noAdjacentTetraIndex);
                setTT(vertexAFinalPosition, oldTetra->TT(vertexAFinalPosition));
            }

            inline VertexIndex& operator[](int i) {
                assert(i >= 0 && i < 4);
                return tetraTV[i];
            }

            inline const VertexIndex& operator[](int i) const {
                assert(i >= 0 && i < 4);
                return tetraTV[i];
            }

            // Single vertex index setter
            inline void setTV(PositionIndex vertexPosition, VertexIndex vertex) {
                tetraTV[vertexPosition] = vertex;
            }

            // Complete adjacency relations setter
            inline void setTT(TetraIndex a, TetraIndex b, TetraIndex c, TetraIndex d) {
                tetraTT[0] = a;
                tetraTT[1] = b;
                tetraTT[2] = c;
                tetraTT[3] = d;
            }

            // Single adjacency relation setter
            inline void setTT(PositionIndex oppositeVertexPosition, TetraIndex tetra) {
                tetraTT[oppositeVertexPosition] = tetra;
            }

            // Vertices getter
            inline VertexIndex TV(PositionIndex vertexPosition) const {
                return tetraTV[vertexPosition];
            }

            // Adjacent tetrahedra getter
            inline TetraIndex TT(PositionIndex TT) const {
                return tetraTT[TT];
            }

            // Inverse adjacency relations getter
            inline PositionIndex inverseTT(Tetra *t1) const {
                for (PositionIndex i = 0; i <= 3; i++)
                    if (!this->contains(t1->TV(i)))
                        return i;

                return 4;
            }

            // true if the given vertex is on the boundary of the tetrahedron
            inline bool contains(VertexIndex vertex) const {
                for (PositionIndex pindex = 0; pindex <= 3; pindex++) {
                    if (TV(pindex) == vertex) return true;
                }
                return false;
            }

            // Internal Index of the given vertex inside the tetrahedron
            inline short int vertexIndex(VertexIndex vertex) const {
                for (PositionIndex pindex = 0; pindex <= 3; pindex++) {
                    if (TV(pindex) == vertex) return pindex;
                }
                return -1;
            }

            // Internal Index of the given face (triangle) inside the tetrahedron
            inline short int faceIndex(VertexIndex v1, VertexIndex v2, VertexIndex v3) const {
                for (PositionIndex pindex = 0; pindex <= 3; pindex++) {
                    if (tetraTV[pindex] != v1 && tetraTV[pindex] != v2 && tetraTV[pindex] != v3) return pindex;
                }
                return -1;
            }

            // true if the given triangle is on the boundary of the tetrahedron
            inline bool contains(Triangle face) {
                for (PositionIndex pindex = 0; pindex <= 3; pindex++) {
                    if (face.contains(TV(pindex))) return true;
                }
                return false;
            }

            // Faces extraction
            inline Triangle *TF(PositionIndex faceIndex) const {
                PositionIndex indexes[3];
                int filled = 0;
                for (PositionIndex index = 0; index <= 3; index++) {
                    if (index != faceIndex) indexes[filled++] = index;
                }
                return new Triangle(TV(indexes[0]), TV(indexes[1]), TV(indexes[2]), faceIndex);
            }

            // Standard output operator
            inline friend std::ostream &operator<<(std::ostream &out, Tetra const &tetra) {
                out << "Tetra[(" << tetra.TV(0) << " " << tetra.TV(1) << " " << tetra.TV(2) << " " << tetra.TV(3) << ")"
                    << std::endl;
                out << "adj-(" << tetra.TT(0) << " " << tetra.TT(1) << " " << tetra.TT(2) << " " << tetra.TT(3) << ")"
                    << std::endl;
                out << "]";
                return out;
            }

        };


    public:
        // Constructor
        inline TetraMesh() {
        }

        const std::vector<vec3> &points() const override { return points_; };

        // Returns the list of this tet mesh's tetrahedron indices.
        const std::vector<Tetra> &tets() const { return tetras; }

        // Vertex reference getter
        inline Vertex &getVertex(VertexIndex index) { return vertices[index]; }

        // Tetrahedron reference getter
        inline Tetra &getTetra(TetraIndex index) { return tetras[index]; }

        // Number of stored vertices
        inline VertexIndex n_vertices() const { return vertices.size(); }

        // Number of stored tetrrahedra
        inline TetraIndex n_tetrahedra() const { return tetras.size(); }


        // Functions retrieving the star of a vertex (simplexes incident to the vertex)
        std::vector<TetraIndex> *VT(VertexIndex center);

        //
        std::vector<TetraIndex> VT(VertexIndex center, bool *foundBorder);

        //
        std::vector<Triangle> VF(VertexIndex center);

        //
        std::vector<Edge> *VE(VertexIndex center);


        // Functions retrieving the star of an edge (simplexes incident to an edge and bigger than it)
        std::vector<TetraIndex> *ET(Edge *edge);

        //
        std::vector<Triangle *> *EF(Edge *edge);


        // Functions retrieving the star of a face (simplexes incident to a triangle and bigger than it)
        TetraIndex FTstar(Triangle face);

        //
        std::vector<TetraIndex> *FT(Triangle *face);


        // Functions retrieving the adjacents of a simplex
        std::vector<VertexIndex> *VV(VertexIndex center);

        //
        std::vector<Edge> EE(Edge edge);

        //
        std::vector<Triangle *> FF(Triangle face);
        //
        // std::vector<Triangle*> TT is in the class Tetra


        // return true if the vertex is on the boundary of the mesh
        bool isBoundary(VertexIndex center);

        ///IO functions---------
        // Loader for .tet tetrahedral mesh files
        bool read(const std::string& file_name);

        //write a new mesh in .tet format
        bool save(const std::string& file_name);

    public:
        // Adding a vertex to the mesh
        inline void add_vertex(const Vertex& v) {
            vertices.push_back(v);
            points_.push_back(vec3(v.x(), v.y(), v.z()));
        }

        // Adding a tetrahedron the to mesh
        inline void add_tetra(const Tetra& t) {
            tetras.push_back(t);
        }

        // TetraMesh building function
        void build_adjacency();

    protected:
        //Functions

        // Index sorting function
        void sortIndexes(PositionIndex *indexes, int numberOfIndexes, Tetra const &tetra);

        // Sorts lexicographically triples of indices
        inline void sortFaces(std::vector<std::pair<Triangle, TetraIndex> *> *faces) {
            sortFaces(faces, 0, faces->size());
        }

        // Sorts lexicographically subsets of a list of triples of indices
        void sortFaces(std::vector<std::pair<Triangle, TetraIndex> *> *faces, int left, int right
        );

        // Merges sorted sublists of triples of indices
        void merge(std::vector<std::pair<Triangle, TetraIndex> *> *faces, int left, int center, int right);


        //Utility Function
        template<class C>
        bool isIntoVector(C c, std::vector<C> &c_vect);


        // TetraMesh coherence test
        void testCoherence();

    protected:
        //bool printingForTesting;
        std::vector<Vertex> vertices; // std::vector storing vertices
        std::vector<vec3> points_;
        std::vector<Tetra> tetras; // std::vector storing tetrahedra
    };


#else
    /**
     * \brief Data structure representing a tetrahedral mesh.
     * \class TetraMesh easy3d/core/tetra_mesh.h
     * \todo Build the adjacency information.
     * see https://github.com/UMDGeoVis/TetraMesh/blob/master/source/TetraMesh.h
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
        void build_adjacency();
    };
#endif
}

#endif  // TETRA_MESH_H
