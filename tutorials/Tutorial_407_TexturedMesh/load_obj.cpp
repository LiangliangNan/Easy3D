#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/file_system.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/texture.h>

#include <3rd_party/tinyobjloader/tiny_obj_loader.h>

#include <unordered_map>


using namespace easy3d;

bool load_obj(const std::string& file_name, SurfaceMesh* mesh) {
    if (!mesh) {
        std::cerr << "null mesh pointer" << std::endl;
        return false;
    }

    if (!file_system::is_file(file_name)) {
        std::cerr << "file does not exist: \'" << file_system::simple_name(file_name) << "\'" << std::endl;
        return false;
    }

    tinyobj::ObjReaderConfig config;
    config.triangulate = false;
    config.vertex_color = false;
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(file_name, config)) {
        std::cerr << "error occured loading obj file" << std::endl;
        const std::string& error = reader.Error();
        if (!error.empty())
            std::cerr << error << std::endl;
        else {
            const std::string& warning = reader.Warning();
            if (!warning.empty())
                std::cerr << warning << std::endl;
        }
        return false;
    }

    const tinyobj::attrib_t& attrib = reader.GetAttrib();
    const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
    const std::vector<tinyobj::material_t>& materials = reader.GetMaterials();

    // clear mesh
    mesh->clear();

    // for each vertex
    for (size_t v = 0; v < attrib.vertices.size() / 3; v++)
        mesh->add_vertex(vec3(attrib.vertices.data() + 3 * v));

    // for each texcoord
    std::vector<vec2> texcoords;
    for (size_t v = 0; v < attrib.texcoords.size() / 2; v++)
        texcoords.push_back(vec2(attrib.texcoords.data() + 2 * v));
    if (!texcoords.empty())
        mesh->add_halfedge_property<vec2>("h:texcoord");
    auto prop_texcoords = mesh->get_halfedge_property<vec2>("h:texcoord");

    // For each shape
    for (size_t i = 0; i < shapes.size(); i++) {
        size_t index_offset = 0;

        assert(shapes[i].mesh.num_face_vertices.size() == shapes[i].mesh.material_ids.size());
        assert(shapes[i].mesh.num_face_vertices.size() == shapes[i].mesh.smoothing_group_ids.size());

        // For each face
        for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
            size_t fnum = shapes[i].mesh.num_face_vertices[f];

            // For each vertex in the face
            std::vector<SurfaceMesh::Vertex> vertices;
            std::unordered_map<int, int> texcoord_ids;  // vertex id -> texcoord id
            for (size_t v = 0; v < fnum; v++) {
                tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
                SurfaceMesh::Vertex vtx(idx.vertex_index);
                vertices.push_back(vtx);
                if (prop_texcoords)
                    texcoord_ids[vtx.idx()] = idx.texcoord_index;
            }

            SurfaceMesh::Face face = mesh->add_face(vertices);
            if (prop_texcoords) {
                for (auto h : mesh->halfedges(face)) {
                    auto v = mesh->to_vertex(h);
                    int id = texcoord_ids[v.idx()];
                    prop_texcoords[h] = texcoords[id];
                }
            }

            // the smoothing group id
            //int smoothing_group_id = shapes[i].mesh.smoothing_group_ids[f];
            index_offset += fnum;
        }
    }

    // ------------- group the faces according to the material -------------
    // each group is a set of faces sharing the same material

    typedef std::vector<easy3d::SurfaceMesh::Face> Group;
    std::vector<Group> groups(materials.size());

    int face_idx = 0;
    for (size_t i = 0; i < shapes.size(); i++) {
        for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
            if (!materials.empty()) {
                // the material id of this face
                int material_id = shapes[i].mesh.material_ids[f];
                groups[material_id].push_back(easy3d::SurfaceMesh::Face(face_idx));
            }
            ++face_idx;
        }
    }

    auto points = mesh->get_vertex_property<vec3>("v:point");
    for (std::size_t i = 0; i<groups.size(); ++i) {
        const Group& group = groups[i];
        if (group.empty())
            continue;
        TrianglesDrawable* drawable = mesh->add_triangles_drawable(std::to_string(i));
                const tinyobj::material_t& mat = materials[i];
                vec3  ambient(mat.ambient);
                vec3  diffuse(mat.diffuse);
                vec3  specular(mat.specular);
                float shininess(mat.shininess);

        std::vector<vec3> vertices, vertex_normals;
        std::vector<vec2> vertex_texcoords;
        for (auto f : group) {
            // we assume convex polygonal faces and we render in triangles
            SurfaceMesh::Halfedge start = mesh->halfedge(f);
            SurfaceMesh::Halfedge cur = mesh->next_halfedge(mesh->next_halfedge(start));
            SurfaceMesh::Vertex va = mesh->to_vertex(start);
            const vec3& pa = points[va];
            const vec3& n = mesh->compute_face_normal(f);
            while (cur != start) {
                SurfaceMesh::Vertex vb = mesh->from_vertex(cur);
                SurfaceMesh::Vertex vc = mesh->to_vertex(cur);
                const vec3& pb = points[vb];
                const vec3& pc = points[vc];
                vertices.push_back(pa);
                vertices.push_back(pb);
                vertices.push_back(pc);
                vertex_normals.insert(vertex_normals.end(), 3, n);

                if (prop_texcoords) {
                    vertex_texcoords.push_back(prop_texcoords[start]);
                    vertex_texcoords.push_back(prop_texcoords[mesh->prev_halfedge(cur)]);
                    vertex_texcoords.push_back(prop_texcoords[cur]);
                }

                cur = mesh->next_halfedge(cur);
            }
        }

        drawable->update_vertex_buffer(vertices);
        drawable->update_normal_buffer(vertex_normals);
        drawable->set_phong_shading(false);
        if (prop_texcoords)
            drawable->update_texcoord_buffer(vertex_texcoords);
        drawable->release_index_buffer();

        std::string texname = mat.ambient_texname;
        if (texname.empty())
            texname = mat.diffuse_texname;
        if (texname.empty())
            texname = mat.specular_texname;

        if (!texname.empty()) {
            const std::string texture_file = file_system::dir_name(file_name) + "/" + texname;
            Texture* tex = Texture::create(texture_file, GL_REPEAT);
            if (tex) {
                std::cerr << "texture created from " << texname << std::endl;
                drawable->set_texture(tex);
            }
        }

    }

    return mesh->n_faces() > 0;
}

