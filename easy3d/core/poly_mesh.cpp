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

#include <easy3d/core/poly_mesh.h>

#include <cmath>
#include <fstream>

#include <easy3d/util/logging.h>


namespace easy3d {

    namespace internal {

        template<typename T>
        inline void read(std::istream &input, std::set<T>& data) {
            unsigned int size(0);
            input.read((char*)&size, sizeof(unsigned int));
            std::vector<T> array(size);
            input.read((char*)array.data(), size * sizeof(T));
            data = std::set<T>(array.begin(), array.end());
        }

        template<typename T>
        inline void write(std::ostream &output, const std::set<T>& data) {
            unsigned int size = static_cast<unsigned int>(data.size());
            output.write((char*)&size, sizeof(unsigned int));
            std::vector<T> array(data.begin(), data.end());
            output.write((char*)array.data(), size * sizeof(T));
        }

        template<typename T>
        inline void read(std::istream &input, std::vector<T>& data) {
            unsigned int size(0);
            input.read((char*)&size, sizeof(unsigned int));
            data.resize(size);
            input.read((char*)data.data(), size * sizeof(T));
        }

        template<typename T>
        inline void write(std::ostream &output, const std::vector<T>& data) {
            unsigned int size = static_cast<unsigned int>(data.size());
            output.write((char*)&size, sizeof(unsigned int));
            output.write((char*)data.data(), size * sizeof(T));
        }
    }


    void PolyMesh::VertexConnectivity::read(std::istream &input) {
        internal::read(input, vertices_);
        internal::read(input, edges_);
        internal::read(input, halffaces_);
        internal::read(input, cells_);
    }


    void PolyMesh::VertexConnectivity::write(std::ostream &output) const {
        internal::write(output, vertices_);
        internal::write(output, edges_);
        internal::write(output, halffaces_);
        internal::write(output, cells_);
    }


    void PolyMesh::EdgeConnectivity::read(std::istream &input) {
        internal::read(input, vertices_);
        internal::read(input, halffaces_);
        internal::read(input, cells_);
    }


    void PolyMesh::EdgeConnectivity::write(std::ostream &output) const {
        internal::write(output, vertices_);
        internal::write(output, halffaces_);
        internal::write(output, cells_);
    }


    void PolyMesh::HalfFaceConnectivity::read(std::istream &input) {
        internal::read(input, vertices_);
        internal::read(input, edges_);
        input.read((char*)(&cell_), sizeof(Cell));
        input.read((char*)(&opposite_), sizeof(HalfFace));
    }


    void PolyMesh::HalfFaceConnectivity::write(std::ostream &output) const {
        internal::write(output, vertices_);
        internal::write(output, edges_);
        output.write((char*)(&cell_), sizeof(Cell));
        output.write((char*)(&opposite_), sizeof(HalfFace));
    }


    void PolyMesh::CellConnectivity::read(std::istream &input) {
        internal::read(input, vertices_);
        internal::read(input, edges_);
        internal::read(input, halffaces_);
    }


    void PolyMesh::CellConnectivity::write(std::ostream &output) const {
        internal::write(output, vertices_);
        internal::write(output, edges_);
        internal::write(output, halffaces_);
    }


    PolyMesh::PolyMesh()
    {
        // allocate standard properties
        // same list is used in operator=() and assign()
        vconn_    = add_vertex_property<VertexConnectivity>("v:connectivity");
        econn_    = add_edge_property<EdgeConnectivity>("e:connectivity");
        hconn_    = add_halfface_property<HalfFaceConnectivity>("h:connectivity");
        cconn_    = add_cell_property<CellConnectivity>("c:connectivity");

        vpoint_   = add_vertex_property<vec3>("v:point");

        mprops_.push_back();
    }


    //-----------------------------------------------------------------------------


    PolyMesh& PolyMesh::operator=(const PolyMesh& rhs)
    {
        if (this != &rhs)
        {
            // deep copy of property containers
            vprops_ = rhs.vprops_;
            eprops_ = rhs.eprops_;
            hprops_ = rhs.hprops_;
            fprops_ = rhs.fprops_;
            cprops_ = rhs.cprops_;
            mprops_ = rhs.mprops_;

            // property handles contain pointers, have to be reassigned
            vconn_    = vertex_property<VertexConnectivity>("v:connectivity");
            econn_    = edge_property<EdgeConnectivity>("e:connectivity");
            hconn_    = halfface_property<HalfFaceConnectivity>("h:connectivity");
            cconn_    = cell_property<CellConnectivity>("c:connectivity");

            vpoint_   = vertex_property<vec3>("v:point");
        }

        return *this;
    }


    //-----------------------------------------------------------------------------


    PolyMesh& PolyMesh::assign(const PolyMesh& rhs)
    {
        if (this != &rhs)
        {
            // clear properties
            vprops_.clear();
            eprops_.clear();
            hprops_.clear();
            fprops_.clear();
            mprops_.clear();
            cprops_.clear();

            // allocate standard properties
            vconn_    = add_vertex_property<VertexConnectivity>("v:connectivity");
            econn_    = add_edge_property<EdgeConnectivity>("e:connectivity");
            hconn_    = add_halfface_property<HalfFaceConnectivity>("h:connectivity");
            cconn_    = add_cell_property<CellConnectivity>("c:connectivity");
            
            vpoint_   = add_vertex_property<vec3>("v:point");

            // copy properties from other mesh
            vconn_.array()     = rhs.vconn_.array();
            cconn_.array()     = rhs.cconn_.array();
            hconn_.array()     = rhs.hconn_.array();
            vpoint_.array()    = rhs.vpoint_.array();

            // resize (needed by property containers)
            vprops_.resize(rhs.n_vertices());
            cprops_.resize(rhs.n_cells());
            eprops_.resize(rhs.n_edges());
            fprops_.resize(rhs.n_faces());
            mprops_.resize(1);
        }

        return *this;
    }


    //-----------------------------------------------------------------------------


    bool PolyMesh::read(const std::string &file_name)
    {
        clear();

        // open file (in binary mode)
        std::ifstream input(file_name.c_str(), std::fstream::binary);
        if (input.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return false;
        }

        // how many elements?
        unsigned int nv, ne, nh, nf, nc;
        input.read((char*)&nv, sizeof(unsigned int));
        input.read((char*)&ne, sizeof(unsigned int));
        input.read((char*)&nf, sizeof(unsigned int));
        input.read((char*)&nc, sizeof(unsigned int));
        nh = nf * 2;

        // resize containers
        resize(nv, ne, nf, nc);

        // get properties
        auto& vconn = vertex_property<PolyMesh::VertexConnectivity>("v:connectivity").vector();
        auto& econn = edge_property<PolyMesh::EdgeConnectivity>("e:connectivity").vector();
        auto& hconn = halfface_property<PolyMesh::HalfFaceConnectivity>("h:connectivity").vector();
        auto& cconn = cell_property<PolyMesh::CellConnectivity>("c:connectivity").vector();
        auto point = vertex_property<vec3>("v:point");

        // read properties from file
        for (unsigned int i=0; i<nv; ++i)   vconn[i].read(input);
        for (unsigned int i=0; i<ne; ++i)   econn[i].read(input);
        for (unsigned int i=0; i<nh; ++i)   hconn[i].read(input);
        for (unsigned int i=0; i<nc; ++i)   cconn[i].read(input);
        input.read((char*)point.data(), static_cast<long>(nv * sizeof(vec3)));

        return (n_vertices() > 0 && n_faces() > 0 && n_cells() > 0);
    }


    //-----------------------------------------------------------------------------


    bool PolyMesh::write(const std::string& file_name) const
    {
        if (n_vertices() == 0 || n_faces() == 0 || n_cells() == 0) {
            LOG(ERROR) << "empty polyhedral mesh";
            return false;
        }

        // open file (in binary mode)
        std::ofstream output(file_name.c_str(), std::fstream::binary);
        if (output.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return false;
        }

        // how many elements?
        unsigned int nv, ne, nh, nf, nc;
        nv = n_vertices();
        ne = n_edges();
        nf = n_faces();
        nh = nf * 2;
        nc = n_cells();

        output.write((char*)&nv, sizeof(unsigned int));
        output.write((char*)&ne, sizeof(unsigned int));
        output.write((char*)&nf, sizeof(unsigned int));
        output.write((char*)&nc, sizeof(unsigned int));

        // get properties
        const auto& vconn = get_vertex_property<PolyMesh::VertexConnectivity>("v:connectivity").vector();
        const auto& econn = get_edge_property<PolyMesh::EdgeConnectivity>("e:connectivity").vector();
        const auto& hconn = get_halfface_property<PolyMesh::HalfFaceConnectivity>("h:connectivity").vector();
        const auto& cconn = get_cell_property<PolyMesh::CellConnectivity>("c:connectivity").vector();
        auto point = get_vertex_property<vec3>("v:point");

        // write properties to file
        for (unsigned int i=0; i<nv; ++i)   vconn[i].write(output);
        for (unsigned int i=0; i<ne; ++i)   econn[i].write(output);
        for (unsigned int i=0; i<nh; ++i)   hconn[i].write(output);
        for (unsigned int i=0; i<nc; ++i)   cconn[i].write(output);
        output.write((char*)point.data(), static_cast<long>(nv * sizeof(vec3)));

        return true;
    }


    //-----------------------------------------------------------------------------


    void PolyMesh::clear()
    {
        //---- clear without removing properties

        vprops_.resize(0);
        eprops_.resize(0);
        hprops_.resize(0);
        fprops_.resize(0);
        cprops_.resize(0);
        mprops_.resize(0);

        vprops_.shrink_to_fit(); // remove unused memory from vectors
        eprops_.shrink_to_fit();
        hprops_.shrink_to_fit();
        fprops_.shrink_to_fit();
        cprops_.shrink_to_fit();
        mprops_.shrink_to_fit();

        //---- keep the standard properties and remove all the other properties

        vprops_.resize_property_array(2);   // "v:connectivity", "v:point"
        eprops_.resize_property_array(1);   // "e:connectivity"
        hprops_.resize_property_array(1);   // "h:connectivity"
        cprops_.resize_property_array(2);   // "c:connectivity", "t:indices"
        mprops_.clear();
        mprops_.resize(1);
    }


    //-----------------------------------------------------------------------------


    void PolyMesh::property_stats(std::ostream& output) const
    {
        std::vector<std::string> props;

        props = model_properties();
        if (!props.empty())
        {
            output << "model properties:\n";
            for (const auto& p : props)
                output << "\t" << p << std::endl;
        }

        props = vertex_properties();
        if (!props.empty())
        {
            output << "vertex properties:\n";
            for (const auto& p : props)
                output << "\t" << p << std::endl;
        }

        props = edge_properties();
        if (!props.empty())
        {
            output<< "edge properties:\n";
            for (const auto& p : props)
                output << "\t" << p << std::endl;
        }

        props = halfface_properties();
        if (!props.empty())
        {
            output << "halfface properties:\n";
            for (const auto& p : props)
                output << "\t" << p << std::endl;
        }

        props = face_properties();
        if (!props.empty())
        {
            output << "face properties:\n";
            for (const auto& p : props)
                output << "\t" << p << std::endl;
        }

        props = cell_properties();
        if (!props.empty())
        {
            output << "cell properties:\n";
            for (const auto& p : props)
                output << "\t" << p << std::endl;
        }

        props = model_properties();
        if (!props.empty())
        {
            output << "model properties:\n";
            for (const auto& p : props)
                output << "\t" << p << std::endl;
        }
    }


    //-----------------------------------------------------------------------------


    PolyMesh::Edge PolyMesh::find_edge(Vertex a, Vertex b) const {
        for (auto e : edges(a)) {
            if (vertex(e, 0) == b || vertex(e, 1) == b)
                return e;
        }
        return Edge();
    }


    PolyMesh::HalfFace PolyMesh::find_half_face(const std::vector<Vertex> &vts) const {
        assert(vts.size() >= 3);

        // loop over all the halffaces that involve the 1st vertex
        for (auto h : halffaces(vts[0])) {
            const auto& tests = vertices(h);
            if (tests.size() != vts.size())
                continue;

            // we first find the element (from the first set) to match the 1st element in the second set
            for (std::size_t start = 0; start < tests.size(); ++start) {
                if (tests[start] == vts[0]) {
                    // test for the remaining elements
                    bool all_matched = true;
                    for (std::size_t id = 1; id < vts.size(); ++id) { // we can start from 1
                        if (tests[(id + start) % tests.size()] != vts[id]) {
                            all_matched = false;
                            break;
                        }
                    }
                    if (all_matched)
                        return h;
                }
            }
        }

        return HalfFace();
    }


    //-----------------------------------------------------------------------------


    PolyMesh::HalfFace PolyMesh::add_face(const std::vector<Vertex>& vertices) {
        auto h = find_half_face(vertices);
        if (!h.is_valid()) {
            h = new_face();
            auto oh = opposite(h);
            hconn_[h].vertices_ = vertices;
            hconn_[oh].vertices_ = std::vector<Vertex>(vertices.rbegin(), vertices.rend());

            for (std::size_t i=0; i<vertices.size(); ++i) {
                auto s = vertices[i];
                auto t = vertices[(i+1)%vertices.size()];
                auto e = find_edge(s, t);
                if (!e.is_valid())
                    e = new_edge(s, t);

                econn_[e].halffaces_.insert(h);
                econn_[e].halffaces_.insert(oh);
                hconn_[h].edges_.insert(e);
                hconn_[oh].edges_.insert(e);

                vconn_[s].halffaces_.insert(h);
                vconn_[s].halffaces_.insert(oh);
            }
        }

        return h;
    }


    PolyMesh::Cell PolyMesh::add_cell(const std::vector<HalfFace> &faces) {
        Cell c = new_cell();
        cconn_[c].halffaces_ = faces;

        for (auto f : faces) {
            hconn_[f].cell_ = c;
            for (auto v : vertices(f)) {
                vconn_[v].cells_.insert(c);
                cconn_[c].vertices_.insert(v);
            }
            for (auto e : edges(f)) {
                cconn_[c].edges_.insert(e);
                econn_[e].cells_.insert(c);
            }
        }

        return c;
    }


    PolyMesh::Cell PolyMesh::add_tetra(Vertex v0, Vertex v1, Vertex v2, Vertex v3) {
        // for each face, its normal points outside the cell.
        // the same as specified in Delaunay3::facet_vertex_
        //      unsigned int Delaunay3::facet_vertex_[4][3] = {
        //              {1, 2, 3},
        //              {0, 3, 2},
        //              {3, 0, 1},
        //              {2, 1, 0}
        //      };
        std::vector<HalfFace> faces = {
                add_triangle(v1, v2, v3),
                add_triangle(v0, v3, v2),
                add_triangle(v3, v0, v1),
                add_triangle(v2, v1, v0)
        };

        return add_cell(faces);
    }


    PolyMesh::Cell PolyMesh::add_hexa(Vertex v0, Vertex v1, Vertex v2, Vertex v3,
                                      Vertex v4, Vertex v5, Vertex v6, Vertex v7) {
        // for each face, its normal points outside the cell.
        // 3--------------------2
        // |\                   |\
        // | \                  | \
        // |  \                 |  \
        // |   7----------------+---6
        // |   |                |   |
        // 0---+----------------1   |
        //  \  |                 \  |
        //   \ |                  \ |
        //    \|                   \|
        //     4--------------------5
        std::vector<HalfFace> faces = {
                add_quad(v0, v3, v2, v1),   // back
                add_quad(v0, v4, v7, v3),   // left
                add_quad(v4, v5, v6, v7),   // front
                add_quad(v1, v2, v6, v5),   // right
                add_quad(v2, v3, v7, v6),   // top
                add_quad(v0, v1, v5, v4)    // bottom
        };

        return add_cell(faces);
    }


    //-----------------------------------------------------------------------------


    void PolyMesh::update_face_normals()
    {
        auto fnormal = face_property<vec3>("f:normal");

        FaceIterator fit, fend=faces_end();

        int num_degenerate = 0;
        for (fit=faces_begin(); fit!=fend; ++fit) {
            if (is_degenerate(*fit)) {
                ++num_degenerate;
                fnormal[*fit] = vec3(0, 0, 1);
            } else
                fnormal[*fit] = compute_face_normal(*fit);
        }

        if (num_degenerate > 0)
            LOG(WARNING) << "model has " << num_degenerate << " degenerate faces";
    }


    //-----------------------------------------------------------------------------


    vec3 PolyMesh::compute_face_normal(Face f) const
    {
        const auto& vts = vertices(f);

        if (vts.size() == 3) { // face is a triangle
            const vec3& p0 = vpoint_[vts[0]];
            const vec3& p1 = vpoint_[vts[1]];
            const vec3& p2 = vpoint_[vts[2]];
            return cross(p2 - p1, p0 - p1).normalize();
        }
        else {// face is a general polygon
            vec3 n(0,0,0);

            for (std::size_t id = 0; id < vts.size(); ++id) {
                int id_prev = static_cast<int>((id - 1 + vts.size()) % vts.size());
                int id_next = static_cast<int>((id + 1 + vts.size()) % vts.size());
                vec3 p0 = vpoint_[vts[id_prev]];
                vec3 p1 = vpoint_[vts[id]];
                vec3 p2 = vpoint_[vts[id_next]];
                n += cross(p2 - p1, p0 - p1).normalize();
            }
            return n.normalize();
        }
    }



    void PolyMesh::update_vertex_normals()
    {
        auto vnormal = vertex_property<vec3>("v:normal");

        // always re-compute face normals
        update_face_normals();
        auto fnormal = get_face_property<vec3>("f:normal");

        VertexIterator vit, vend=vertices_end();

        for (vit=vertices_begin(); vit!=vend; ++vit) {
            auto v = *vit;
            if (is_border(v)) { // compute the average of its incident border faces' normals
                vec3 n(0,0,0);
                for (auto f : halffaces(v)) {
                    if (is_border(f)) {
                        n += fnormal[face(f)];
                    }
                }
                vnormal[v] = normalize(n);
            }
            else { // interior vertex, normal not defined. We assign one of its incident face's normal
                if (!halffaces(v).empty()) {
                    auto hf = *halffaces(v).begin();
                    vnormal[v] = fnormal[face(hf)];
                }
            }
        }
    }


    //-----------------------------------------------------------------------------


    float PolyMesh::edge_length(Edge e) const
    {
        return norm(vpoint_[vertex(e,0)] - vpoint_[vertex(e,1)]);
    }


    //-----------------------------------------------------------------------------


    bool PolyMesh::is_degenerate(Face h) const {
        const auto& vts = vertices(h);

        const vec3& p0 = vpoint_[vts[0]];
        const vec3& p1 = vpoint_[vts[1]];
        const vec3& p2 = vpoint_[vts[2]];

        auto d1 = p2 - p1;
        auto len1 = d1.length();
        if (len1 < std::numeric_limits<float>::min())
            return true;
        auto d2 = p0 - p1;
        auto len2 = d2.length();
        if (len2 < std::numeric_limits<float>::min())
            return true;

        auto angle = geom::angle(d1, d2);
        angle = geom::to_degrees(std::abs(angle));
        if (std::abs(angle) < std::numeric_limits<float>::min())
            return true;

        return false;
    }


    //-----------------------------------------------------------------------------


    bool PolyMesh::is_tetraheral_mesh() const {
        for (auto f : faces()) {
            if (vertices(f).size() != 3)
                return false;
        }
        for (auto c : cells()) {
            if (vertices(c).size() != 4)
                return false;
        }
        return true;
    }


    //-----------------------------------------------------------------------------


    void PolyMesh::extract_boundary(std::vector<std::vector<Vertex> > &faces) const {
        faces.clear();
        for (auto h : halffaces()) {
            if (!is_border(h))
                continue;
            faces.push_back(vertices(h));
        }
    }

} // namespace easy3d
