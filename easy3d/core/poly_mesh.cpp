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
        fconn_    = add_face_property<FaceConnectivity>("f:connectivity");
        tconn_    = add_tetra_property<TetraConnectivity>("t:connectivity");

        vpoint_   = add_vertex_property<vec3>("v:point");
        tindices_ = add_tetra_property<ivec4>("t:indices");

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
            fprops_ = rhs.fprops_;
            tprops_ = rhs.tprops_;
            mprops_ = rhs.mprops_;

            // property handles contain pointers, have to be reassigned
            vconn_    = add_vertex_property<VertexConnectivity>("v:connectivity");
            econn_    = add_edge_property<EdgeConnectivity>("e:connectivity");
            fconn_    = add_face_property<FaceConnectivity>("f:connectivity");
            tconn_    = add_tetra_property<TetraConnectivity>("t:connectivity");

            vpoint_   = vertex_property<vec3>("v:point");
            tindices_ = tetra_property<ivec4>("t:indices");

            // normals might be there, therefore use get_property
            fnormal_  = get_face_property<vec3>("f:normal");
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
            fprops_.clear();
            mprops_.clear();
            tprops_.clear();

            // allocate standard properties
            vconn_    = add_vertex_property<VertexConnectivity>("v:connectivity");
            econn_    = add_edge_property<EdgeConnectivity>("e:connectivity");
            fconn_    = add_face_property<FaceConnectivity>("f:connectivity");
            tconn_    = add_tetra_property<TetraConnectivity>("t:connectivity");
            
            vpoint_   = add_vertex_property<vec3>("v:point");
            tindices_ = add_tetra_property<ivec4>("t:indices");
            
            // normals might be there, therefore use get_property
            fnormal_  = get_face_property<vec3>("f:normal");

            // copy properties from other mesh
            vconn_.array()     = rhs.vconn_.array();
            tconn_.array()     = rhs.tconn_.array();
            fconn_.array()     = rhs.fconn_.array();
            vpoint_.array()    = rhs.vpoint_.array();

            // resize (needed by property containers)
            vprops_.resize(rhs.n_vertices());
            tprops_.resize(rhs.n_cells());
            eprops_.resize(rhs.n_edges());
            fprops_.resize(rhs.n_faces());
            mprops_.resize(1);
        }

        return *this;
    }


    //-----------------------------------------------------------------------------


    bool PolyMesh::read_tet(const std::string &file_name)
    {
        clear();

        std::ifstream input(file_name.c_str());
        if (input.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return false;
        }

        std::string dummy;
        int num_vertices, num_tets;
        input >> dummy >> num_vertices >> num_tets;
        resize(num_vertices, num_tets);

        vec3 p;
        for (std::size_t i = 0; i < num_vertices; ++i) {
            input >> p;
            add_vertex(p);
        }

        ivec4 ids;
        for (std::size_t i = 0; i < num_tets; ++i) {
            input >> ids;
            add_tetra(Vertex(ids[0]), Vertex(ids[1]), Vertex(ids[2]), Vertex(ids[3]));
        }

        return num_tets > 0;
    }


    //-----------------------------------------------------------------------------


    bool PolyMesh::write_tet(const std::string &file_name) const {
        if (n_faces() == 0) {
            LOG(ERROR) << "empty mesh";
            return false;
        }

        std::ofstream output(file_name.c_str());
        if (output.fail()) {
            LOG(ERROR) << "could not open file: " << file_name;
            return false;
        }

        output << "tet " << n_vertices() << " " << n_cells() << std::endl;
        for (auto v : vertices())
            output << vpoint_[v] << std::endl;
        for (auto t : tetrahedra())
            output << tindices_[t] << std::endl;

        return true;
    }


    //-----------------------------------------------------------------------------


    void PolyMesh::clear()
    {
        //---- clear without removing properties

        vprops_.resize(0);
        eprops_.resize(0);
        fprops_.resize(0);
        tprops_.resize(0);
        mprops_.resize(0);

        vprops_.shrink_to_fit(); // remove unused memory from vectors
        eprops_.shrink_to_fit();
        fprops_.shrink_to_fit();
        tprops_.shrink_to_fit();
        mprops_.shrink_to_fit();

        //---- keep the standard properties and remove all the other properties

        vprops_.resize_property_array(2);   // "v:connectivity", "v:point"
        eprops_.resize_property_array(1);   // "e:connectivity"
        fprops_.resize_property_array(1);   // "f:connectivity"
        tprops_.resize_property_array(2);   // "t:connectivity", "t:indices"
        mprops_.clear();

        // update/invalidate the normal properties
        fnormal_  = FaceProperty<vec3>();
    }


    //-----------------------------------------------------------------------------


    void PolyMesh::reserve(unsigned int nvertices, unsigned int ncells )
    {
        vprops_.reserve(nvertices);
        tprops_.reserve(ncells);
        mprops_.reserve(1);
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


    PolyMesh::Vertex PolyMesh::add_vertex(const vec3& p)
    {
        Vertex v = new_vertex();
        vpoint_[v] = p;
        return v;
    }


    //-----------------------------------------------------------------------------

    PolyMesh::Tetra PolyMesh::add_tetra(Vertex v0, Vertex v1, Vertex v2, Vertex v3) {
        std::vector<Vertex> vertices(4);
        vertices[0] = v0;
        vertices[1] = v1;
        vertices[2] = v2;
        vertices[3] = v3;

        Tetra t = new_cell();
        tindices_[t] = ivec4(v0.idx(), v1.idx(), v2.idx(), v3.idx());

        LOG_FIRST_N(WARNING, 1) << "TODO: build adjacency" << std::endl;

        return t;
    }


    //-----------------------------------------------------------------------------


    void PolyMesh::update_face_normals()
    {
        if (!fnormal_)
            fnormal_ = face_property<vec3>("f:normal");

        FaceIterator fit, fend=faces_end();

        int num_degenerate = 0;
        for (fit=faces_begin(); fit!=fend; ++fit) {
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


    vec3 PolyMesh::compute_face_normal(Face f) const
    {
        const auto& vts = vertices(f);

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


    bool PolyMesh::is_degenerate(Face f) const {
        const auto& vts = vertices(f);

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

} // namespace easy3d
