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

#include <easy3d/fileio/surface_mesh_io.h>

#include <unordered_map>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/manifold_builder.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>


// this seems to be more robust and can handle multi-materials
#define USE_TINY_OBJ_LOADER


#ifdef USE_TINY_OBJ_LOADER

#define TINYOBJLOADER_IMPLEMENTATION
#include <3rd_party/tinyobjloader/tiny_obj_loader.h>

namespace easy3d {

    namespace io {

        bool load_obj(const std::string &file_name, SurfaceMesh *mesh) {
            if (!mesh) {
                LOG(ERROR) << "null mesh pointer";
                return false;
            }

            if (!file_system::is_file(file_name)) {
                LOG(ERROR) << "file does not exist: " << file_system::simple_name(file_name);
                return false;
            }

            tinyobj::ObjReaderConfig config;
            config.triangulate = false;
            config.vertex_color = false;
            tinyobj::ObjReader reader;
            if (!reader.ParseFromFile(file_name, config)) {
                LOG(ERROR) << "failed parsing file: " + file_name + ". " << reader.Error() << reader.Warning();
                return false;
            }

            // --------------------- collect the data ------------------------

            const std::vector<tinyobj::shape_t> &shapes = reader.GetShapes();
            if (shapes.empty()) {
                LOG(WARNING) << "file contains no shape";
                return false;
            }
            const tinyobj::attrib_t &attrib = reader.GetAttrib();
            const std::vector<tinyobj::material_t> &materials = reader.GetMaterials();

            // for each texcoord
            std::vector<vec2> texcoords;
            for (std::size_t v = 0; v < attrib.texcoords.size(); v += 2) {
                texcoords.emplace_back(vec2(attrib.texcoords.data() + v));
            }

            // ------------------------ build the mesh ------------------------

            // clear the mesh in case of existing data
            mesh->clear();

            ManifoldBuilder builder(mesh);
            builder.begin_surface();

            // add vertices
            for (std::size_t v = 0; v < attrib.vertices.size(); v += 3) {
                // Should I create vertices later, to get rid of isolated vertices?
                builder.add_vertex(vec3(attrib.vertices.data() + v));
            }

            // create texture coordinate property if texture coordinates present
            if (!texcoords.empty())
                mesh->add_halfedge_property<vec2>("h:texcoord");
            auto prop_texcoords = mesh->get_halfedge_property<vec2>("h:texcoord");

            auto find_face_halfedge = [](SurfaceMesh *mesh, SurfaceMesh::Face face,
                                         SurfaceMesh::Vertex v) -> SurfaceMesh::Halfedge {
                for (auto h : mesh->halfedges(face)) {
                    if (mesh->to_vertex(h) == v)
                        return h;
                }
                LOG_FIRST_N(ERROR, 1) << "could not find a halfedge pointing to " << v << " in face " << face
                                      << " (this is the first record)";
                return SurfaceMesh::Halfedge();
            };

            // invalid face will also be added, to ensure correct face indices
            std::vector<SurfaceMesh::Face> faces;
            // for each shape
            for (size_t i = 0; i < shapes.size(); i++) {
                std::size_t index_offset = 0;
                LOG_IF(ERROR, shapes[i].mesh.num_face_vertices.size() != shapes[i].mesh.material_ids.size());
                LOG_IF(ERROR, shapes[i].mesh.num_face_vertices.size() != shapes[i].mesh.smoothing_group_ids.size());

                // For each face
                for (std::size_t face_idx = 0; face_idx < shapes[i].mesh.num_face_vertices.size(); ++face_idx) {
                    std::size_t face_size = shapes[i].mesh.num_face_vertices[face_idx];

                    // For each vertex in the face
                    std::vector<SurfaceMesh::Vertex> vertices;
                    std::vector<int> texcoord_ids;
                    for (std::size_t v = 0; v < face_size; v++) {
                        const tinyobj::index_t &face = shapes[i].mesh.indices[index_offset + v];
                        vertices.emplace_back(face.vertex_index);
                        if (prop_texcoords)
                            texcoord_ids.emplace_back(face.texcoord_index);
                    }

                    SurfaceMesh::Face face = builder.add_face(vertices);
                    // invalid face will also be added, to ensure correct face indices
                    faces.push_back(face);
                    if (prop_texcoords && face.is_valid()) {
                        auto begin = find_face_halfedge(mesh, face, builder.face_vertices()[0]);
                        auto cur = begin;
                        int idx = 0;
                        do {
                            prop_texcoords[cur] = texcoords[texcoord_ids[idx++]];
                            cur = mesh->next_halfedge(cur);
                        } while (cur != begin);
                    }

                    //int smoothing_group_id = shapes[i].mesh.smoothing_group_ids[f];
                    index_offset += face_size;
                }
            }

            builder.end_surface();

            // now the material
            if (!materials.empty()) {
                auto face_color = mesh->add_face_property<vec3>("f:color");
                int face_idx = 0;
                for (std::size_t i = 0; i < shapes.size(); i++) {
                    for (std::size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
                        auto face = faces[face_idx];
                        if (face.is_valid()) {
                            // the material id of this face
                            const int material_id = shapes[i].mesh.material_ids[f];
                            const tinyobj::material_t &mat = materials[material_id];
                            // ambient = vec3(mat.ambient);
                            // specular = vec3(mat.specular);
                            face_color[face] = vec3(mat.diffuse);
                        }
                        ++face_idx;
                    }
                }

                for (const auto &mat : materials) {
                    LOG_IF(WARNING, !mat.ambient_texname.empty()) << "ambient texture ignored: " << mat.ambient_texname;
                    LOG_IF(WARNING, !mat.diffuse_texname.empty()) << "diffuse texture ignored: " << mat.diffuse_texname;
                    LOG_IF(WARNING, !mat.specular_texname.empty())
                                    << "specular texture ignored: " << mat.specular_texname;
                }
            }

            return mesh->n_faces() > 0;
        }
    }
}

#elif 1

namespace easy3d {

    namespace io {

        bool load_obj(const std::string& file_name, SurfaceMesh* mesh)
        {
            if (!mesh) {
                LOG(ERROR) << "null mesh pointer";
                return false;
            }

            char   s[200];
            float  x, y, z;
            std::vector<SurfaceMesh::Vertex>  vertices;
            std::vector<vec2> all_tex_coords;   //individual texture coordinates
            std::vector<int> halfedge_tex_idx; //texture coordinates sorted for halfedges
            SurfaceMesh::HalfedgeProperty <vec2> tex_coords = mesh->halfedge_property<vec2>("h:texcoord");
            bool with_tex_coord = false;

            // clear mesh
            mesh->clear();


            // open file (in ASCII mode)
            FILE* in = fopen(file_name.c_str(), "r");
            if (!in) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }


            // clear line once
            memset(&s, 0, 200);


            // parse line by line (currently only supports vertex positions & faces
            while (in && !feof(in) && fgets(s, 200, in))
            {
                // comment
                if (s[0] == '#' || isspace(s[0])) continue;

                // vertex
                else if (strncmp(s, "v ", 2) == 0)
                {
                    if (sscanf(s, "v %f %f %f", &x, &y, &z))
                    {
                        mesh->add_vertex(vec3(x, y, z));
                    }
                }
                // normal
                else if (strncmp(s, "vn ", 3) == 0)
                {
                    if (sscanf(s, "vn %f %f %f", &x, &y, &z))
                    {
                        // problematic as it can be either a vertex property when interpolated
                        // or a halfedge property for hard edges
                    }
                }

                // texture coordinate
                else if (strncmp(s, "vt ", 3) == 0)
                {
                    if (sscanf(s, "vt %f %f", &x, &y))
                    {
                        all_tex_coords.emplace_back(vec2(x, y));
                    }
                }

                // face
                else if (strncmp(s, "f ", 2) == 0)
                {
                    int component(0), nV(0);
                    bool endOfVertex(false);
                    char *p0, *p1(s + 1);

                    vertices.clear();
                    halfedge_tex_idx.clear();

                    // skip white-spaces
                    while (*p1 == ' ') ++p1;

                    while (p1)
                    {
                        p0 = p1;

                        // overwrite next separator

                        // skip '/', '\n', ' ', '\0', '\r' <-- don't forget Windows
                        while (*p1 != '/' && *p1 != '\r' && *p1 != '\n' && *p1 != ' ' && *p1 != '\0') ++p1;

                        // detect end of vertex
                        if (*p1 != '/')
                        {
                            endOfVertex = true;
                        }

                        // replace separator by '\0'
                        if (*p1 != '\0')
                        {
                            *p1 = '\0';
                            p1++; // point to next token
                        }

                        // detect end of line and break
                        if (*p1 == '\0' || *p1 == '\n')
                        {
                            p1 = 0;
                        }

                        // read next vertex component
                        if (*p0 != '\0')
                        {
                            switch (component)
                            {
                            case 0: // vertex
                            {
                                vertices.emplace_back(SurfaceMesh::Vertex(atoi(p0) - 1));
                                break;
                            }
                            case 1: // texture coord
                            {
                                int idx = atoi(p0) - 1;
                                halfedge_tex_idx.push_back(idx);
                                with_tex_coord = true;
                                break;
                            }
                            case 2: // normal
                                break;
                            }
                        }

                        ++component;

                        if (endOfVertex)
                        {
                            component = 0;
                            nV++;
                            endOfVertex = false;
                        }
                    }

                    SurfaceMesh::Face f = mesh->add_face(vertices);


                    // add texture coordinates
                    if (with_tex_coord)
                    {
                        SurfaceMesh::HalfedgeAroundFaceCirculator h_fit = mesh->halfedges(f);
                        SurfaceMesh::HalfedgeAroundFaceCirculator h_end = h_fit;
                        unsigned v_idx = 0;
                        do
                        {
                            tex_coords[*h_fit] = all_tex_coords.at(halfedge_tex_idx.at(v_idx));
                            ++v_idx;
                            ++h_fit;
                        } while (h_fit != h_end);
                    }
                }
                // clear line
                memset(&s, 0, 200);
            }

            fclose(in);
            return mesh->n_faces() > 0;
        }
    }
}

#else // use the tokenizer: supports triangle mesh ONLY!!!

#include <easy3d/util/tokenizer.h>

namespace easy3d {

    namespace io {

        bool load_obj(const std::string& file_name, SurfaceMesh* mesh)
        {
            if (!mesh) {
                LOG(ERROR) << "null mesh pointer";
                return false;
            }

            // open file (in binary mode)
            FILE* fp = fopen(file_name.c_str(), "rb");
            if (fp == nullptr) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

            fseek(fp, 0, SEEK_END);
            std::size_t length = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            char *data = new char[length + 1];
            fread(data, 1, length, fp);
            fclose(fp);
            data[length] = '\0';

#ifndef NDEBUG
            fprintf(stderr, "Read file '%s', %zd bytes\n", file_name.c_str(), length);
#endif

            float val[4];
            int32_t idx[3][3];
            int32_t match;
            char format = 0;
            bool vtx4Comp = false;
            bool tex3Comp = false;
            std::vector<SurfaceMesh::Vertex>  vertices;
            std::vector<vec2> all_tex_coords;   //individual texture coordinates
            std::vector<int> halfedge_tex_idx; //texture coordinates sorted for halfedges
            SurfaceMesh::HalfedgeProperty <vec2> tex_coords = mesh->halfedge_property<vec2>("h:texcoord");
            bool with_tex_coord = false;

            // clear mesh
            mesh->clear();


            Tokenizer tok(data, "/");
#ifndef NDEBUG
            tok.setVerbose();
#endif

            // parse line by line (currently only supports vertex positions & faces
            while (!tok.atEOF())
            {
                if (!tok.readToken()) {
                    tok.consumeToEOL();
                    continue; // likely EOL we didn't explicitly handle?
                }


                const char* tmp = tok.getLastTokenPtr();
                switch (tmp[0]) {
                case '#':
                    //comment line, eat the remainder
                    tok.consumeToEOL();
                    break;
                case 'v':
                    switch (tmp[1]) {
                    case '\0': //vertex, 3 or 4 components
                        val[3] = 1.0f;  //default w coordinate
                        match = tok.getTokenFloatArray(val, 4);
                        mesh->add_vertex(vec3(val));
                        vtx4Comp |= (match == 4);
                        assert(match > 2 && match < 5);
                        break;
                    case 'n': //normal, 3 components
                        match = tok.getTokenFloatArray(val, 3);
                        // problematic as it can be either a vertex property when interpolated
                        // or a halfedge property for hard edges
                        assert(match == 3);
                        break;
                    case 't': //texcoord, 2 or 3 components
                        val[2] = 0.0f;  //default r coordinate
                        match = tok.getTokenFloatArray(val, 3);
                        all_tex_coords.emplace_back(vec2(val[0], val[1]));
                        tex3Comp |= (match == 3);
                        assert(match > 1 && match < 4);
                        break;
                    case 'p': // Parameter space vertices not supported...
                        tok.consumeToEOL();
                        break;
                    }
                    break;
                case 'f':
                {
                    //face
                    // determine the type, and read the initial vertex, all entries in a face must have the same format
                    // formats are:
                    // 1  #
                    // 2  #/#
                    // 3  #/#/#
                    // 4  #//#
                    // we need to 'hand read' the first run, to decode the formatting.
                    format = 0;
                    if (!tok.getTokenInt(idx[0][0])) {
                        assert(0);
                        return false;
                    }
                    // on our way.
                    format = 1;
                    if (tok.consumeOneDelim()) {
                        if (tok.consumeOneDelim()) {
                            // automatically format 4.
                            format = 4;
                        }
                        if (!tok.getTokenInt(idx[0][1])) {
                            assert(0);
                            return false;
                        }
                        if (format != 4)
                        {
                            format = 2; // at least format 2.
                            tok.setConsumeWS(false);
                            if (tok.consumeOneDelim()) {
                                if (tok.getTokenInt(idx[0][2])) {
                                    // automatically format 3
                                    format = 3;
                                }
                                // else remain format 2, in case of "#/#/" wacky format.
                            }
                            tok.setConsumeWS(true);
                        }
                    }
                    switch (format) {
                    case 1: // #
                    { //This face has only vertex indices
                        //grab the second vertex to prime
                        tok.getTokenInt(idx[1][0]);
                        while (tok.getTokenInt(idx[2][0])) {
                            //add the indices
                            vertices.clear();
                            for (unsigned char ii = 0; ii < 3; ii++) {
                                vertices.emplace_back(SurfaceMesh::Vertex(idx[ii][0] - 1));
                            }
                            //prepare for the next iteration
                            idx[1][0] = idx[2][0];
                        }
                        break;
                    }

                    case 2: // #/#
                    { //This face has vertex and texture coordinate indices

                        //grab the second vertex to prime
                        tok.getTokenIntArray(idx[1], 2);

                        while (tok.getTokenIntArray(idx[2], 2) == 2) {

                            //add the indices
                            vertices.clear();
                            halfedge_tex_idx.clear();
                            for (unsigned char ii = 0; ii < 3; ii++) {
                                vertices.emplace_back(SurfaceMesh::Vertex(idx[ii][0] - 1));
                                halfedge_tex_idx.push_back(idx[ii][1] - 1);
                            }
                            //prepare for the next iteration
                            idx[1][0] = idx[2][0];
                            idx[1][1] = idx[2][1];
                        }
                        with_tex_coord = true;
                        break;
                    }


                    case 3: // #/#/#
                    { //This face has vertex, texture coordinate, and normal indices

                        //grab the second vertex to prime
                        tok.getTokenIntArray(idx[1], 3);

                        //create the fan
                        while (tok.getTokenIntArray(idx[2], 3) == 3) {

                            //add the indices
                            vertices.clear();
                            halfedge_tex_idx.clear();
                            for (unsigned char ii = 0; ii < 3; ii++) {
                                vertices.emplace_back(SurfaceMesh::Vertex(idx[ii][0] - 1));
                                halfedge_tex_idx.push_back(idx[ii][1] - 1);
                                //_nIndex.push_back( idx[ii][2]);
                            }

                            //prepare for the next iteration
                            idx[1][0] = idx[2][0];
                            idx[1][1] = idx[2][1];
                            idx[1][2] = idx[2][2];
                        }

                        with_tex_coord = true;
                        break;
                    }

                    case 4: // #//#
                    { //This face has vertex and normal indices

                        //grab the second vertex to prime
                        tok.getTokenIntArray(idx[1], 2);

                        //create the fan
                        while (tok.getTokenIntArray(idx[2], 2) == 2) {

                            //add the indices
                            vertices.clear();
                            for (unsigned char ii = 0; ii < 3; ii++) {
                                vertices.emplace_back(SurfaceMesh::Vertex(idx[ii][0] - 1));
                            }
                            //prepare for the next iteration
                            idx[1][0] = idx[2][0];
                            idx[1][1] = idx[2][1];
                        }

                        //hasNormals = true;
                        break;
                    }
                    }

                    SurfaceMesh::Face f = mesh->add_face(vertices);
                    // add texture coordinates
                    if (with_tex_coord)
                    {
                        SurfaceMesh::HalfedgeAroundFaceCirculator h_fit = mesh->halfedges(f);
                        SurfaceMesh::HalfedgeAroundFaceCirculator h_end = h_fit;
                        unsigned v_idx = 0;
                        do
                        {
                            tex_coords[*h_fit] = all_tex_coords.at(halfedge_tex_idx.at(v_idx));
                            ++v_idx;
                            ++h_fit;
                        } while (h_fit != h_end);
                    }

                }
                break;

                case 's':
                case 'g':
                case 'u':
                    //all presently ignored
                default:
                    tok.consumeToEOL();
                }
            }
            LOG(INFO) << "read file done";
            return mesh->n_faces() > 0;
        }
    }
}

#endif


namespace easy3d {

    namespace io {


        bool save_obj(const std::string &file_name, const SurfaceMesh *mesh) {
            if (!mesh) {
                LOG(ERROR) << "null mesh pointer";
                return false;
            }

            FILE *out = fopen(file_name.c_str(), "w");
            if (!out) {
                LOG(ERROR) << "could not open file: " << file_name;
                return false;
            }

            // comment
            fprintf(out, "# OBJ exported from Easy3D\n");

            //vertices
            SurfaceMesh::VertexProperty<vec3> points = mesh->get_vertex_property<vec3>("v:point");
            for (SurfaceMesh::VertexIterator vit = mesh->vertices_begin(); vit != mesh->vertices_end(); ++vit) {
                const vec3 &p = points[*vit];
                fprintf(out, "v %.10f %.10f %.10f\n", p[0], p[1], p[2]);
            }

            //normals
            SurfaceMesh::VertexProperty<vec3> normals = mesh->get_vertex_property<vec3>("v:normal");
            if (normals) {
                for (SurfaceMesh::VertexIterator vit = mesh->vertices_begin(); vit != mesh->vertices_end(); ++vit) {
                    const vec3 &p = normals[*vit];
                    fprintf(out, "vn %.10f %.10f %.10f\n", p[0], p[1], p[2]);
                }
            }

            //optionally texture coordinates
            // do we have them?
            std::vector<std::string> h_props = mesh->halfedge_properties();
            bool with_tex_coord = false;
            std::vector<std::string>::iterator h_prop_end = h_props.end();
            std::vector<std::string>::iterator h_prop_start = h_props.begin();
            while (h_prop_start != h_prop_end) {
                if (0 == (*h_prop_start).compare("h:texcoord")) {
                    with_tex_coord = true;
                }
                ++h_prop_start;
            }

            //if so then add
            if (with_tex_coord) {
                SurfaceMesh::HalfedgeProperty<vec2> tex_coord = mesh->get_halfedge_property<vec2>("h:texcoord");
                for (SurfaceMesh::HalfedgeIterator hit = mesh->halfedges_begin(); hit != mesh->halfedges_end(); ++hit) {
                    const vec2 &pt = tex_coord[*hit];
                    fprintf(out, "vt %.10f %.10f\n", pt[0], pt[1]);
                }
            }

            //faces
            for (SurfaceMesh::FaceIterator fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit) {
                fprintf(out, "f");
                SurfaceMesh::VertexAroundFaceCirculator fvit = mesh->vertices(*fit), fvend = fvit;
                SurfaceMesh::HalfedgeAroundFaceCirculator fhit = mesh->halfedges(*fit);
                do {
                    if (with_tex_coord) {
                        // write vertex index, tex_coord index and normal index
                        fprintf(out, " %d/%d/%d", (*fvit).idx() + 1, (*fhit).idx() + 1, (*fvit).idx() + 1);
                        ++fhit;
                    } else {
                        // write vertex index and normal index
                        fprintf(out, " %d//%d", (*fvit).idx() + 1, (*fvit).idx() + 1);
                    }
                } while (++fvit != fvend);
                fprintf(out, "\n");
            }

            fclose(out);
            return true;
        }


    } // namespace io

} // namespace easy3d

