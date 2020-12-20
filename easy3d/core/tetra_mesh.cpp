#include <easy3d/core/tetra_mesh.h>

#include <set>
#include <map>
#include <queue>
#include <fstream>


namespace easy3d {


#if 1


    void TetraMesh::sortIndexes(PositionIndex *indexes, int numberOfIndexes, Tetra const &tetra) {

        for (int i = 0; i < numberOfIndexes; i++) {
            int minimum = i;
            for (int j = i + 1; j < numberOfIndexes; j++) {
                if (tetra.TV(indexes[j]) < tetra.TV(indexes[minimum])) minimum = j;
            }

            PositionIndex tmp = indexes[i];
            indexes[i] = indexes[minimum];
            indexes[minimum] = tmp;
        }

    }

    void TetraMesh::build_adjacency() {
        unsigned int VTstarSet = 0;
        unsigned int TTandInverseTTSet = 0;
        std::vector<std::pair<Triangle, TetraIndex> *> faces;
        faces.reserve(this->n_tetrahedra() * 4);


        Triangle *tri;
        Tetra tetra;


        for (TetraIndex index = 0; index < n_tetrahedra(); index = index + 1) {
            tetra = getTetra(index);

            for (PositionIndex vp = 0; vp <= 3; vp++) {

                Vertex &vertex = getVertex(tetra.TV(vp));

                if (vertex.VTstar() == uninitializedTetraIndex) {
                    VTstarSet++;
                    vertex.setVTstar(index);
                }

                tri = tetra.TF(vp);
                faces.push_back(new std::pair<Triangle, TetraIndex>(*(tri), index));
                delete tri;

            }
        }


        sortFaces(&faces);

        Triangle firstFace;
        Triangle secondFace;
        TetraIndex firstIndex, secondIndex;


        for (unsigned int i = 0; i < faces.size() - 1; i++) {
            firstFace = faces[i]->first;
            secondFace = faces[i + 1]->first;

            if ((firstFace) == (secondFace)) {
                firstIndex = faces[i]->second;
                secondIndex = faces[i + 1]->second;

                Tetra &firstTetra = getTetra(firstIndex);
                Tetra &secondTetra = getTetra(secondIndex);
                firstTetra.setTT(firstFace.faceIndex(), secondIndex);
                secondTetra.setTT(secondFace.faceIndex(), firstIndex);
                TTandInverseTTSet += 2;

                delete faces[i];
                delete faces[i + 1];
                i++;

            } else {
                delete faces[i];
            }
        }

        faces.clear();
    }


    void TetraMesh::sortFaces(std::vector<std::pair<Triangle, TetraIndex> *> *faces, int left, int right) {
        if (right - left > 2) {
            int center = (left + right) / 2;
            sortFaces(faces, left, center);
            sortFaces(faces, center, right);
            merge(faces, left, center, right);
        } else {
//int compare=minusTriangle(faces->at(left)->first,faces->at(right-1)->first);
            if (faces->at(left)->first > faces->at(right - 1)->first) {
                std::pair<Triangle, TetraIndex> *tmp = faces->at(right - 1);
                faces->at(right - 1) = faces->at(left);
                faces->at(left) = tmp;
            }
        }
    }


    bool TetraMesh::read(const std::string& file_name) {
        std::ifstream input(file_name.c_str());
        if (!input.is_open())
            return false;

        std::string dummy;
        int num_vertices, num_tets;
        input >> dummy >> num_vertices >> num_tets;

        vec3 p;
        for (std::size_t i = 0; i < num_vertices; ++i) {
            input >> p;
            add_vertex(Vertex(p.x, p.y, p.z));
        }

        ivec4 v;
        for (std::size_t i = 0; i < num_tets; ++i) {
            input >> v;
            add_tetra(Tetra(v[0], v[1], v[2], v[3]));
        }

        build_adjacency();

        return (num_vertices > 0 && num_tets > 0);
    }


    std::vector<TetraMesh::Triangle *> TetraMesh::FF(Triangle face) {
        std::vector<Triangle *> faces;
        std::vector<Triangle *> *tetra0 = this->EF(face.FE(0));
        std::vector<Triangle *> *tetra1 = this->EF(face.FE(1));
        std::vector<Triangle *> *tetra2 = this->EF(face.FE(2));
        for (unsigned int i = 0; i < tetra0->size(); i++) {
            if (face != *tetra0->at(i)) {
                faces.push_back(tetra0->at(i));
            }
        }
        for (unsigned int i = 0; i < tetra1->size(); i++) {
            if (face != *tetra1->at(i)) {
                faces.push_back(tetra1->at(i));
            }
        }
        for (unsigned int i = 0; i < tetra2->size(); i++) {
            if (face != *tetra2->at(i)) {
                faces.push_back(tetra2->at(i));
            }
        }
        return faces;
    }


    std::vector<TetraIndex> *TetraMesh::FT(Triangle *face) {
        TetraIndex t = FTstar(*face);
        std::vector<TetraIndex> *ret = new std::vector<TetraIndex>();
        ret->push_back(t);
        TetraIndex t1 = getTetra(t).TT(getTetra(t).faceIndex(face->FV(0), face->FV(1), face->FV(2)));
        if (t1 != noAdjacentTetraIndex)
            ret->push_back(t1);

        return ret;
    }


    TetraIndex TetraMesh::FTstar(Triangle face) {
        std::set<TetraIndex> tetras;
        TetraIndex initial = getVertex(face.FV(0)).VTstar();

        std::queue<TetraIndex> coda;

        tetras.insert(initial);

        PositionIndex k;
        TetraIndex attuale;

        if (getTetra(initial).contains(face.FV(0)) && getTetra(initial).contains(face.FV(1)) &&
            getTetra(initial).contains(face.FV(2))) {
            return initial;
        } else {
            coda.push(initial);
        }

        while (!coda.empty()) {
            attuale = coda.front();

            k = getTetra(attuale).vertexIndex(face.FV(0));

            if (getTetra(attuale).contains(face.FV(0)) && getTetra(attuale).contains(face.FV(1)) &&
                getTetra(attuale).contains(face.FV(2))) {
                return attuale;
            }

            for (PositionIndex pos = 1; pos <= 3; pos++) {
                TetraIndex adj = this->getTetra(attuale).TT((PositionIndex) ((k + pos) % 4));
                if (adj != noAdjacentTetraIndex && tetras.find(adj) == tetras.end()) {
                    coda.push(adj);
                    tetras.insert(adj);
                }
            }
            coda.pop();
        }

        return -1;
    }


    std::vector<TetraMesh::Edge> TetraMesh::EE(Edge edge) {
        std::vector<Edge> edges;
        std::vector<Edge> *edge0 = this->VE(edge.EV(0));
        std::vector<Edge> *edge1 = this->VE(edge.EV(1));
        for (unsigned int i = 0; i < edge0->size(); i++) {
            if (edge != edge0->at(i))
                edges.push_back(edge0->at(i));
        }
        for (unsigned int i = 0; i < edge1->size(); i++) {
            if (edge != edge1->at(i))
                edges.push_back(edge1->at(i));
        }
        return edges;
    }


    std::vector<TetraMesh::Triangle *> *TetraMesh::EF(Edge *edge) {

        std::vector<Triangle *> *faces = new std::vector<Triangle *>();
        TetraIndex first = getVertex(edge->EV(0)).VTstar();
        TetraIndex adj;
        if (!getTetra(first).contains(edge->EV(1))) {

            std::set<TetraIndex> *tetras = new std::set<TetraIndex>();
            std::queue<TetraIndex> *coda = new std::queue<TetraIndex>();
            coda->push(first);
            tetras->insert(first);

            PositionIndex k;
            TetraIndex attuale;

            while (!coda->empty()) {
                attuale = coda->front();
                if (getTetra(attuale).contains(edge->EV(1))) {
                    first = attuale;
                    break;
                }
                k = getTetra(attuale).vertexIndex(edge->EV(0));
                for (PositionIndex pos = 1; pos <= 3; pos++) {
                    adj = this->getTetra(attuale).TT((PositionIndex) ((k + pos) % 4));
                    if (adj != noAdjacentTetraIndex && tetras->find(adj) == tetras->end()) {
                        coda->push(adj);
                        tetras->insert(adj);
                    }
                }
                coda->pop();
            }

            delete tetras;
            delete coda;
        }


        std::vector<short int> directions;
        for (int i = 0; i < 4; i++) {
            if (getTetra(first).TV(i) != (edge->EV(0)) &&
                getTetra(first).TV(i) != (edge->EV(1))) {

                directions.push_back(i);
            }
        }

        short int tmp_direction;
        TetraIndex next = first;
        short int next_direction = directions[0];
        while (next != noAdjacentTetraIndex) {

            faces->push_back(getTetra(next).TF(next_direction));

            if (getTetra(next).TT(next_direction) != noAdjacentTetraIndex) {
                Tetra t = getTetra(getTetra(next).TT(next_direction));
                tmp_direction = getTetra(next).inverseTT(&t);
            }

            next = getTetra(next).TT(next_direction);
            if (next == noAdjacentTetraIndex) continue;
            next_direction =
                    6 - getTetra(next).vertexIndex(edge->EV(0)) - getTetra(next).vertexIndex(edge->EV(1)) -
                    tmp_direction;

            if (next == first) {
                break;
            }
        }

        if (next == noAdjacentTetraIndex) {
            next = first;
            next_direction = directions[1];

            while (next != noAdjacentTetraIndex) {
                faces->push_back(getTetra(next).TF(next_direction));

                if (getTetra(next).TT(next_direction) != noAdjacentTetraIndex) {
                    Tetra t = getTetra(getTetra(next).TT(next_direction));
                    tmp_direction = getTetra(next).inverseTT(&t);
                }

                next = getTetra(next).TT(next_direction);
                if (next == noAdjacentTetraIndex) continue;
                next_direction = 6 - getTetra(next).vertexIndex(edge->EV(0)) - getTetra(next).vertexIndex(edge->EV(1)) -
                                 tmp_direction;

            }
        }

        return faces;
    }


    std::vector<TetraIndex> *TetraMesh::ET(Edge *edge) {
        std::vector<TetraIndex> *tetra = new std::vector<TetraIndex>();
        VertexIndex v0 = edge->EV(0);
        std::vector<TetraIndex> *vt = this->VT(v0);
        for (unsigned int w = 0; w < vt->size(); w++) {
            bool isFound = false;
            for (PositionIndex i = 0; i <= 3; i++) {
                for (PositionIndex j = i + 1; j <= 3; j++) {
                    Edge *e = new Edge(getTetra((*vt)[w]).TV(i), getTetra((*vt)[w]).TV(j));
                    if (*edge == *e) {
                        tetra->push_back((*vt)[w]);
                        isFound = true;
                        delete e;
                        break;
                    }
                    delete e;
                }
                if (isFound)
                    break;
            }
        }

        delete vt;

        return tetra;
    }


    std::vector<VertexIndex> *TetraMesh::VV(VertexIndex center) {

        std::set<int> *vert = new std::set<int>();
        std::set<int> *tetras = new std::set<int>();
        TetraIndex initial = getVertex(center).VTstar();

        std::queue<int> coda;
        coda.push(initial);
        tetras->insert(initial);

        PositionIndex k;
        TetraIndex attuale;

        while (!coda.empty()) {
            attuale = coda.front();
            coda.pop();

            k = getTetra(attuale).vertexIndex(center);
            for (PositionIndex pos = 1; pos <= 3; pos++) {
                if (vert->find(this->getTetra(attuale).TV((PositionIndex) ((k + pos) % 4))) == vert->end())
                    vert->insert(this->getTetra(attuale).TV((PositionIndex) ((k + pos) % 4)));
                TetraIndex adj = this->getTetra(attuale).TT((PositionIndex) ((k + pos) % 4));
                if (adj != noAdjacentTetraIndex && tetras->find(adj) == tetras->end()) {
                    coda.push(adj);
                    tetras->insert(adj);
                }
            }
        }

        std::vector<VertexIndex> *ret = new std::vector<VertexIndex>(vert->begin(), vert->end());

        delete vert;
        delete tetras;

        return ret;
    }


    std::vector<TetraMesh::Edge> *TetraMesh::VE(VertexIndex center) {
        std::vector<Edge> *edges = new std::vector<Edge>();
        std::vector<TetraIndex> tetra;
        std::vector<bool> visitedTetra(n_tetrahedra(), false);
        TetraIndex tet = this->getVertex(center).VTstar();
        std::queue<TetraIndex> queue;
        queue.push(tet);
        tetra.push_back(tet);
        visitedTetra[tet] = true;

        PositionIndex k = (PositionIndex) -1;

        while (!queue.empty()) {
            k = (PositionIndex) -1;
            TetraIndex current = queue.front();

            for (PositionIndex pos = 0; pos <= 3; pos++) {
                if (this->getTetra(current).TV(pos) == center) {
                    k = pos;
                    break;
                }
            }

            for (PositionIndex pos = 1; pos <= 3; pos++) {
                Edge e = Edge(this->getTetra(current).TV(k),
                              this->getTetra(current).TV(((PositionIndex) (k + pos) % 4)));
                if (!isIntoVector(e, *edges))
                    edges->push_back(e);

                TetraIndex adj = this->getTetra(current).TT((PositionIndex) ((k + pos) % 4));
                if (adj != noAdjacentTetraIndex && !visitedTetra[adj]) {
                    visitedTetra[adj] = true;
                    queue.push(adj);
                    tetra.push_back(adj);
                }
            }
            queue.pop();
        }


        return edges;
    }


    std::vector<TetraMesh::Triangle> TetraMesh::VF(VertexIndex center) {
        std::vector<Triangle> faces;
        std::vector<TetraIndex> tetra;
        std::vector<bool> visitedTetra(n_tetrahedra(), false);
        TetraIndex tet = this->getVertex(center).VTstar();
        std::queue<TetraIndex> queue;
        queue.push(tet);
        tetra.push_back(tet);
        visitedTetra[tet] = true;

        PositionIndex k = (PositionIndex) -1;

        while (!queue.empty()) {
            k = (PositionIndex) -1;

            TetraIndex current = queue.front();

            for (PositionIndex pos = 0; pos <= 3; pos++) {
                if (this->getTetra(current).TV(pos) == center) {
                    k = pos;
                    break;
                }
            }

            for (PositionIndex pos = 0; pos <= 3; pos++) {
                Triangle faceIn = *(this->getTetra(current).TF((PositionIndex) ((k + pos) % 4)));
                if (!this->isIntoVector(faceIn, faces))
                    faces.push_back(faceIn);

                TetraIndex adj = this->getTetra(current).TT((PositionIndex) ((k + pos) % 4));
                if (adj != noAdjacentTetraIndex && !visitedTetra[adj]) {
                    visitedTetra[adj] = true;
                    queue.push(adj);
                    tetra.push_back(adj);
                }
            }
            queue.pop();
        }

        return faces;
    }


    std::vector<TetraIndex> *TetraMesh::VT(VertexIndex center) {
        std::set<TetraIndex> tetras;
        TetraIndex initial = getVertex(center).VTstar();

        std::queue<TetraIndex> coda;
        coda.push(initial);
        tetras.insert(initial);

        PositionIndex k;
        TetraIndex attuale;

        while (!coda.empty()) {
            attuale = coda.front();

            k = getTetra(attuale).vertexIndex(center);

            for (PositionIndex pos = 1; pos <= 3; pos++) {
                TetraIndex adj = this->getTetra(attuale).TT((PositionIndex) ((k + pos) % 4));
                if (adj != noAdjacentTetraIndex && tetras.find(adj) == tetras.end()) {
                    coda.push(adj);
                    tetras.insert(adj);
                }
            }
            coda.pop();
        }

        return new std::vector<TetraIndex>(tetras.begin(), tetras.end());
    }


    std::vector<TetraIndex> TetraMesh::VT(VertexIndex center, bool *foundBorder) {
        *foundBorder = false;
        std::vector<TetraIndex> tetras;
        std::vector<bool> visitedTetra(n_tetrahedra(), false);
        TetraIndex tet = this->getVertex(center).VTstar();
        std::queue<TetraIndex> coda;
        coda.push(tet);
        tetras.push_back(tet);
        visitedTetra[tet] = true;

        PositionIndex k = (PositionIndex) -1;

        while (!coda.empty()) {
            k = (PositionIndex) -1;
            TetraIndex attuale = coda.front();

            for (PositionIndex pos = 0; pos <= 3; pos++) {
                if (this->getTetra(attuale).TV(pos) == center) {
                    k = pos;
                    break;
                }
            }

            for (PositionIndex pos = 1; pos <= 3; pos++) {
                TetraIndex adj = this->getTetra(attuale).TT((PositionIndex) ((k + pos) % 4));
                if (adj != noAdjacentTetraIndex && !visitedTetra[adj]) {
                    visitedTetra[adj] = true;
                    coda.push(adj);
                    tetras.push_back(adj);
                } else if (adj == noAdjacentTetraIndex &&
                           this->getTetra(tet).TV((PositionIndex) ((k + pos) % 4)) != center)
                    *foundBorder = true;
            }
            coda.pop();
        }


        return tetras;
    }


    bool TetraMesh::isBoundary(VertexIndex center) {

        std::vector<TetraIndex> tetras;
        std::vector<bool> visitedTetra(n_tetrahedra(), false);
        TetraIndex tet = this->getVertex(center).VTstar();
        std::queue<TetraIndex> coda;
        coda.push(tet);
        tetras.push_back(tet);
        visitedTetra[tet] = true;

        PositionIndex k = (PositionIndex) -1;

        while (!coda.empty()) {
            k = (PositionIndex) -1;
            TetraIndex attuale = coda.front();

            for (PositionIndex pos = 0; pos <= 3; pos++) {
                if (this->getTetra(attuale).TV(pos) == center) {
                    k = pos;
                    break;
                }
            }

            for (PositionIndex pos = 1; pos <= 3; pos++) {
                TetraIndex adj = this->getTetra(attuale).TT((PositionIndex) ((k + pos) % 4));
                if (adj != noAdjacentTetraIndex && !visitedTetra[adj]) {
                    visitedTetra[adj] = true;
                    coda.push(adj);
                    tetras.push_back(adj);
                } else if (adj == noAdjacentTetraIndex &&
                           this->getTetra(tet).TV((PositionIndex) ((k + pos) % 4)) != center) {
                    return true;
                }
            }
            coda.pop();
        }

        return false;
    }


    void TetraMesh::merge(std::vector<std::pair<Triangle, TetraIndex> *> *faces, int left, int center, int right) {

        int size = right - left;

        std::vector<std::pair<Triangle, TetraIndex> *>
                aux(size);
        int i, j;

        for (i = left, j = center; i < center && j < right;) {
            if (faces->at(i)->first > faces->at(j)->first) {
                aux[i + j - center - left] = faces->at(j);
                j++;
            } else {
                aux[i + j - center - left] = faces->at(i);
                i++;
            }
        }

        for (; i < center; i++) {
            aux[i + j - center - left] = faces->at(i);
        }
        for (; j < right; j++) {
            aux[i + j - center - left] = faces->at(j);
        }

        for (int k = 0; k < size; k++) {
            faces->at(k + left) = aux[k];
        }
    }

    template<class C>
    bool TetraMesh::isIntoVector(C c, std::vector<C> &c_vect) {
        for (unsigned int i = 0; i < c_vect.size(); i++)
            if (c == c_vect.at(i))
                return true;
        return false;
    }


#else

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
       const_cast<TetraMesh*>(this)->build_adjacency();
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
        const_cast<TetraMesh*>(this)->build_adjacency();
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
        const_cast<TetraMesh*>(this)->build_adjacency();
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


void TetraMesh::build_adjacency() {
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

    build_adjacency();

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
#endif
}
