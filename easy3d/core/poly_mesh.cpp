/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
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
 */


#include <easy3d/core/poly_mesh.h>
#include <easy3d/util/logging.h>

#include <cmath>
#include <fstream>

namespace easy3d {

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


    PolyMesh::~PolyMesh()
    {
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

            // normals might be there, therefore use get_property
            fnormal_  = get_halfface_property<vec3>("f:normal");
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
            
            // normals might be there, therefore use get_property
            fnormal_  = get_halfface_property<vec3>("f:normal");

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

        std::ifstream input(file_name.c_str());
        if (input.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return false;
        }

        std::string dummy;
        int num_vertices, num_cells;
        input >> dummy >> num_vertices >> dummy >> num_cells;

        vec3 p;
        for (std::size_t v = 0; v < num_vertices; ++v) {
            input >> p;
            add_vertex(p);
        }

        int num_halffaces, num_valence, idx;
        for (std::size_t c = 0; c < num_cells; ++c) {
            input >> num_halffaces;
            std::vector<PolyMesh::HalfFace> halffaces(num_halffaces);
            for (std::size_t hf = 0; hf < num_halffaces; ++hf) {
                input >> num_valence;
                std::vector<PolyMesh::Vertex> vts(num_valence);
                for (std::size_t v = 0; v < num_valence; ++v) {
                    input >> idx;
                    vts[v] = PolyMesh::Vertex(idx);
                }
                halffaces[hf] = add_face(vts);
            }
            add_cell(halffaces);
        }

        return num_vertices > 0 && num_cells > 0;
    }


    //-----------------------------------------------------------------------------


    bool PolyMesh::write(const std::string &file_name) const {
        if (n_vertices() == 0 || n_faces() == 0 || n_cells() == 0) {
            LOG(ERROR) << "empty polyhedral mesh";
            return false;
        }

        std::ofstream output(file_name.c_str());
        if (output.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return false;
        }

        output << "#vertices " << n_vertices() << std::endl
               << "#cells    " << n_cells() << std::endl;

        for (auto v : vertices())
            output << position(v) << std::endl;

        for (auto c : cells()) {
            int num_halffaces = halffaces(c).size();
            output << num_halffaces << std::endl;
            for (auto h : halffaces(c)) {
                output << vertices(h).size() << " ";
                for (auto v : vertices(h))
                    output << v.idx() << " ";
                output << std::endl;
            }
        }

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

        // update/invalidate the normal properties
        fnormal_  = HalfFaceProperty<vec3>();
    }


    //-----------------------------------------------------------------------------


    void PolyMesh::property_stats(std::ostream& output) const
    {
        std::vector<std::string> props;

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
        auto is_same_set = [](const std::vector<Vertex>& set1, const std::vector<Vertex>& set2) -> bool { // allows permutation
            if (set1.size() != set2.size())
                return false;
            for (std::size_t start=0; start<set1.size(); ++start) {
                bool found = true;
                for (std::size_t id = 0; id < set1.size(); ++id) {
                    if (set1[(id + start) % set1.size()] != set2[id]) {
                        found = false;
                        break;
                    }
                }
                if (found)
                    return true;
            }
            return false;
        };

        assert(vts.size() >= 3);
        for (auto h : halffaces(vts[0])) {
            if (is_same_set(vertices(h), vts))
                return h;
        }
        return HalfFace();
    }


    //-----------------------------------------------------------------------------


    PolyMesh::Vertex PolyMesh::add_vertex(const vec3& p)
    {
        Vertex v = new_vertex();
        vpoint_[v] = p;
        return v;
    }


    PolyMesh::HalfFace PolyMesh::add_face(const std::vector<Vertex>& vertices) {
        auto f = find_half_face(vertices);
        if (!f.is_valid()) {
            f = new_face();
            hconn_[f].vertices_ = vertices;
            hconn_[opposite(f)].vertices_ = std::vector<Vertex>(vertices.rbegin(), vertices.rend());
        }

        for (auto v : vertices) {
            vconn_[v].halffaces_.insert(f);
            vconn_[v].halffaces_.insert(opposite(f));
        }

        for (int i=0; i<vertices.size(); ++i) {
            auto s = vertices[i];
            auto t = vertices[(i+1)%3];
            auto e = find_edge(s, t);
            if (!e.is_valid())
                e = new_edge(s, t);
            econn_[e].halffaces_.insert(f);
            hconn_[f].edges_.insert(e);
            hconn_[opposite(f)].edges_.insert(e);
        }

        return f;
    }


    PolyMesh::HalfFace PolyMesh::add_triangle(Vertex v1, Vertex v2, Vertex v3) {
        std::vector<Vertex> vertices = { v1, v2, v3 };
        return add_face(vertices);
    }


    PolyMesh::HalfFace PolyMesh::add_quad(Vertex v1, Vertex v2, Vertex v3, Vertex v4) {
        std::vector<Vertex> vertices = { v1, v2, v3, v4 };
        return add_face(vertices);
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
        std::vector<HalfFace> faces = {
                add_triangle(v0, v1, v2),
                add_triangle(v1, v3, v2),
                add_triangle(v0, v2, v3),
                add_triangle(v1, v0, v3)
        };

        return add_cell(faces);
    }


    //-----------------------------------------------------------------------------


    void PolyMesh::update_face_normals()
    {
        if (!fnormal_)
            fnormal_ = halfface_property<vec3>("f:normal");

        HalfFaceIterator fit, fend=halffaces_end();

        int num_degenerate = 0;
        for (fit=halffaces_begin(); fit!=fend; ++fit) {
            if (is_degenerate(*fit)) {
                ++num_degenerate;
                fnormal_[*fit] = vec3(0, 0, 1);
            } else
                fnormal_[*fit] = compute_face_normal(*fit);
        }

        if (num_degenerate > 0)
            LOG(WARNING) << "model has " << num_degenerate << " degenerate faces" << std::endl;
    }


    //-----------------------------------------------------------------------------


    vec3 PolyMesh::compute_face_normal(HalfFace h) const
    {
        const auto& vts = vertices(h);

        const vec3& p0 = vpoint_[vts[0]];
        const vec3& p1 = vpoint_[vts[1]];
        const vec3& p2 = vpoint_[vts[2]];

        return cross(p2-p1, p0-p1).normalize();
    }


    //-----------------------------------------------------------------------------


    float PolyMesh::edge_length(Edge e) const
    {
        return norm(vpoint_[vertex(e,0)] - vpoint_[vertex(e,1)]);
    }


    //-----------------------------------------------------------------------------


    bool PolyMesh::is_degenerate(HalfFace h) const {
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
        angle = rad2deg(std::abs(angle));
        if (std::abs(angle) < std::numeric_limits<float>::min())
            return true;

        return false;
    }


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


} // namespace easy3d
