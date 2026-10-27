// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_OBJ_MATERIALS_H
#define EASY3D_RENDERER_OBJ_MATERIALS_H
#include <easy3d/core/surface_mesh.h>
#include <memory>
#include <string>
#include <vector>

namespace easy3d {
// CPU import snapshot for OBJ/MTL diffuse materials. No context is required.
// Parts contain tessellated, seam-preserving attributes; they describe the mesh
// at load time. Reload/rebuild after editing the mesh's topology or attributes.
struct ObjMaterialMesh {
    struct Part {
        std::string name;
        vec3 diffuse = vec3(1);
        std::string texture_file; // empty for missing maps or incomplete face UVs
        std::vector<vec3> positions, normals;
        std::vector<vec2> texcoords;
        std::vector<unsigned int> indices;
        std::vector<int> source_faces;
    };
    std::shared_ptr<SurfaceMesh> mesh;
    std::vector<Part> parts;
    // Attach mesh to a host with create_default_drawables=false first. Creates
    // one drawable per part in its renderer, with CPU or OpenGL textures matching
    // that renderer. Does not replace an existing drawable of the same name.
    void create_drawables() const;
};
// Throws on unreadable files, invalid indices or faces rejected by the mesh
// builder. Texture lookup failures use Kd. Only diffuse maps/colors are rendered;
// ambient/specular maps serve as color-map fallbacks. Independent material
// lighting, MTL opacity and PBR are not implemented.
ObjMaterialMesh load_obj_materials(const std::string& filename);
}
#endif
