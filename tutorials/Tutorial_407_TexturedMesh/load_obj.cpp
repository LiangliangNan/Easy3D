#include <easy3d/core/surface_mesh.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <3rd_party/tinyobjloader/tiny_obj_loader.h>


using namespace easy3d;

bool load_obj(const std::string& file_name, SurfaceMesh* mesh) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file_name.c_str(), nullptr, false);
    if (!ret) {
        std::cerr << "error occured loading obj file" << std::endl;
        return false;
    }

    // clear mesh
    mesh->clear();

    // for each vertex
    for (size_t v = 0; v < attrib.vertices.size() / 3; v++)
        mesh->add_vertex(vec3(attrib.vertices.data() + 3 * v));

    // for each normal
    std::vector<vec3> normals;
    for (size_t v = 0; v < attrib.normals.size() / 3; v++)
        normals.push_back(vec3(attrib.normals.data() + 3 * v));
    if (!normals.empty())
        mesh->add_vertex_property<vec3>("v:normal");
    auto prop_normals = mesh->get_vertex_property<vec3>("v:normal");

    // for each texcoord
    std::vector<vec2> texcoords;
    for (size_t v = 0; v < attrib.texcoords.size() / 2; v++)
        texcoords.push_back(vec2(attrib.texcoords.data() + 2 * v));
    if (!texcoords.empty())
        mesh->add_vertex_property<vec2>("v:texcoord");
    auto prop_texcoords = mesh->get_vertex_property<vec2>("v:texcoord");

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
            for (size_t v = 0; v < fnum; v++) {
                tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
                SurfaceMesh::Vertex vtx = SurfaceMesh::Vertex(idx.vertex_index);
                vertices.push_back(vtx);

                if (prop_normals)
                    prop_normals[vtx] = normals[static_cast<std::size_t>(idx.normal_index)];
                if (prop_texcoords)
                    prop_texcoords[vtx] = texcoords[static_cast<std::size_t>(idx.texcoord_index)];
            }

            mesh->add_face(vertices);

            // the material id of this face
            //int material_id = shapes[i].mesh.material_ids[f];
            // the smoothing group id
            //int smoothing_group_id = shapes[i].mesh.smoothing_group_ids[f];
            index_offset += fnum;
        }
    }
    return mesh->n_faces() > 0;
}

