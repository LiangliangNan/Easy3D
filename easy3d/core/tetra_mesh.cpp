#include <easy3d/core/tetra_mesh.h>

#include <set>
#include <map>
#include <fstream>
#include <cstdio>


namespace easy3d {

    TetraMesh::Tet::Tet(const vec3 &v0, const vec3 &v1, const vec3 &v2, const vec3 &v3) {
        v[0] = v0;
        v[1] = v1;
        v[2] = v2;
        v[3] = v3;
    }


    TetraMesh::Tet TetraMesh::getTet(int i) const {
        const ivec4 &tet = t[i];
        return Tet(v[tet[0]],
                   v[tet[1]],
                   v[tet[2]],
                   v[tet[3]]);
    }


    void TetraMesh::getAdjacentVertices(int vertex, std::vector<int> &vertices) const {
        if (dirty || incidenceMap.size() == 0) {
           const_cast<TetraMesh*>(this)->buildIncidenceMap();
        }

        assert(incidenceMap.size() == v.size());
        assert(vertex < static_cast<int>(incidenceMap.size()));

        vertices.clear();

        const std::vector<int> &incidentTets = incidenceMap[vertex];
        for (size_t i = 0; i < incidentTets.size(); ++i) {
            for (int u = 0; u < 4; ++u) {
                // Add the current vertex if it hasn't been added already.
                int vIndex = t[incidentTets[i]][u];
                bool alreadyAdded = false;
                for (size_t j = 0; j < vertices.size(); ++j) {
                    if (vertices[j] == vIndex) {
                        alreadyAdded = true;
                        break;
                    }
                }
                if (!alreadyAdded && vIndex != vertex) {
                    vertices.push_back(vIndex);
                }
            }
        }
    }


    void TetraMesh::getIncidentTets(int vertex, std::vector<Tet> &tets) const {
        if (dirty || incidenceMap.size() == 0) {
            const_cast<TetraMesh*>(this)->buildIncidenceMap();
        }

        assert(incidenceMap.size() == v.size());
        assert(vertex < static_cast<int>(incidenceMap.size()));

        tets.clear();

        const std::vector<int> &incidentTets = incidenceMap[vertex];
        for (size_t i = 0; i < incidentTets.size(); ++i) {
            tets.push_back(getTet(incidentTets[i]));
        }
    }


    void TetraMesh::getIncidentTetIndices(int vertex, std::vector<int> &tetIndices) const {
        if (dirty || incidenceMap.size() == 0) {
            const_cast<TetraMesh*>(this)->buildIncidenceMap();
        }

        assert(incidenceMap.size() == v.size());
        assert(vertex < static_cast<int>(incidenceMap.size()));

        tetIndices = incidenceMap[vertex];
    }


    void TetraMesh::compactMesh() {
        // Go through our tetrahedra, relabeling vertices.
        std::vector<int> remap(v.size(), -1);
        int nv = 0;
        for (size_t tet = 0; tet < t.size(); ++tet) {
            for (int u = 0; u < 4; ++u) {
                int i = t[tet][u];
                if (remap[i] == -1) {
                    remap[i] = nv;
                    ++nv;
                }
                t[tet][u] = remap[i];
            }
        }

        // Now go through the vertices, remapping them using their new labels.
        // Any vertices not found in tetrahedra will not be included.
        std::vector<vec3> vnew(nv);
        for (size_t i = 0; i < v.size(); ++i) {
            if (remap[i] >= 0) vnew[remap[i]] = v[i];
        }
        v.swap(vnew);

        dirty = true;
    }


    void TetraMesh::getBoundary(std::vector<int> &boundary_verts,
                              std::vector<ivec3> &boundary_tris) const {
        boundary_verts.clear();
        boundary_tris.clear();

        // Go through every triangle of every tetrahedron.
        // If the same triangle has already been found
        // it is not a boundary face.
        std::set<ivec3> boundary_set;
        ivec3 tet_tris[4];
        ivec3 permuted_tris[3];

        for (size_t tet = 0; tet < t.size(); ++tet) {
            // Consider each triangle
            tet_tris[0] = ivec3(t[tet][0], t[tet][1], t[tet][2]);
            tet_tris[1] = ivec3(t[tet][0], t[tet][2], t[tet][3]);
            tet_tris[2] = ivec3(t[tet][0], t[tet][3], t[tet][1]);
            tet_tris[3] = ivec3(t[tet][1], t[tet][3], t[tet][2]);

            // If the winding is wrong on the boundary, try this instead
            //tet_tris[0] = ivec3 t1(t[tet][0], t[tet][2], t[tet][1]);
            //tet_tris[1] = ivec3 t2(t[tet][0], t[tet][3], t[tet][2]);
            //tet_tris[2] = ivec3 t3(t[tet][0], t[tet][1], t[tet][3]);
            //tet_tris[3] = ivec3 t4(t[tet][1], t[tet][2], t[tet][3]);

            for (int tri = 0; tri < 4; ++tri) {
                // If the current triangle already exists in our boundary_set,
                // it will have opposite winding and an arbitrary first vertex.
                // Check all possible valid permutations.
                permuted_tris[0] = ivec3(tet_tris[tri][0],
                                         tet_tris[tri][2],
                                         tet_tris[tri][1]);
                permuted_tris[1] = ivec3(tet_tris[tri][1],
                                         tet_tris[tri][0],
                                         tet_tris[tri][2]);
                permuted_tris[2] = ivec3(tet_tris[tri][2],
                                         tet_tris[tri][1],
                                         tet_tris[tri][0]);
                // Attempt to erase each permutation from the set.
                // If erasure was not successful, it means the triangle isn't
                // in the set yet, so add it.
                if (!boundary_set.erase(permuted_tris[0]) &&
                    !boundary_set.erase(permuted_tris[1]) &&
                    !boundary_set.erase(permuted_tris[2])) {
                    // Add the triangle to the set.
                    boundary_set.insert(tet_tris[tri]);
                }
            }
        }

        // Once we're done traversing the mesh, we should have all boundary
        // triangles in boundary_set.
        // Build the set of boundary vertices, and store the boundary triangles
        // and boundary vertices in boundary_verts and boundary_tris.
        std::set<int> vertex_set;
        for (std::set<ivec3>::iterator itr = boundary_set.begin();
             itr != boundary_set.end(); ++itr) {
            boundary_tris.push_back(*itr);

            vertex_set.insert((*itr)[0]);
            vertex_set.insert((*itr)[1]);
            vertex_set.insert((*itr)[2]);
        }
        for (std::set<int>::iterator itr = vertex_set.begin();
             itr != vertex_set.end(); ++itr) {
            boundary_verts.push_back(*itr);
        }
    }


    void TetraMesh::getBoundary(std::vector<vec3> &boundary_verts,
                              std::vector<ivec3> &boundary_tris) const {
        boundary_verts.clear();
        boundary_tris.clear();

        std::vector<int> vertIndices;
        std::vector<ivec3> triIndices;
        getBoundary(vertIndices, triIndices);

        boundary_verts.reserve(vertIndices.size());
        boundary_tris.reserve(triIndices.size());

        std::map<int, int> meshToBoundaryMap;

        for (size_t vIndex = 0; vIndex < vertIndices.size(); ++vIndex) {
            boundary_verts.push_back(v[vertIndices[vIndex]]);
            meshToBoundaryMap[vertIndices[vIndex]] = vIndex;
        }
        for (size_t tIndex = 0; tIndex < triIndices.size(); ++tIndex) {
            ivec3 tri;
            for (int i = 0; i < 3; ++i) {
                assert(meshToBoundaryMap.find(triIndices[tIndex][i]) !=
                       meshToBoundaryMap.end());
                tri[i] = meshToBoundaryMap[triIndices[tIndex][i]];
            }
            boundary_tris.push_back(tri);
        }
    }


    void TetraMesh::buildIncidenceMap() {
        incidenceMap.clear();
        incidenceMap.resize(v.size());

        // For each vertex, find and store all the tetrahedra that contain it.
        for (size_t vIndex = 0; vIndex < v.size(); ++vIndex) {
            for (size_t tIndex = 0; tIndex < t.size(); ++tIndex) {
                for (int i = 0; i < 4; ++i) {
                    if (t[tIndex][i] == static_cast<int>(vIndex)) {
                        incidenceMap[vIndex].push_back(tIndex);
                    }
                }
            }
        }

        dirty = false;
    }


    bool TetraMesh::read(const std::string& file_name) {
        std::ifstream input(file_name.c_str());
        if (!input.is_open())
            return false;

        std::string dummy;
        int num_vertices, num_tets;
        input >> dummy >> num_vertices >> num_tets;
        v.resize(num_vertices);
        t.resize(num_tets);

        for (std::size_t i = 0; i < num_vertices; ++i)
            input >> v[i];

        for (std::size_t i = 0; i < num_tets; ++i)
            input >> t[i];

        buildIncidenceMap();

        return num_tets > 0;
    }


    bool TetraMesh::save(const std::string& file_name) const {
        std::ofstream output(file_name.c_str());
        if (!output.is_open())
            return false;

        output << "tet " << v.size() << " " << t.size() << std::endl;

        for (std::size_t i = 0; i < v.size(); ++i)
            output << v[i] << std::endl;

        for (std::size_t i = 0; i < t.size(); ++i)
            output << t[i] << std::endl;

        return true;
    }

}
