// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_RENDERER_OBJ_MATERIALS_H
#define EASY3D_RENDERER_OBJ_MATERIALS_H
#include <easy3d/core/surface_mesh.h>
#include <memory>
#include <string>
#include <vector>

namespace easy3d {
/**
 * \brief CPU-side snapshot of diffuse materials imported from an OBJ/MTL file.
 * \details Loading requires no graphics context. Parts contain tessellated,
 * seam-preserving attributes and describe the mesh at load time. Reload or
 * rebuild the snapshot after changing mesh topology or attributes.
 */
struct ObjMaterialMesh {
    /// Tessellated geometry and material properties for one OBJ material group.
    struct Part {
        /// Material/group name used when creating the drawable.
        std::string name;
        /// Diffuse color, defaulting to white.
        vec3 diffuse = vec3(1);
        /// Diffuse texture path; empty if no usable map or face UVs are available.
        std::string texture_file;
        /// Tessellated vertex positions and normals.
        std::vector<vec3> positions; ///< Tessellated vertex positions.
        std::vector<vec3> normals; ///< Tessellated vertex normals.
        /// Tessellated vertex texture coordinates.
        std::vector<vec2> texcoords;
        /// Triangle indices into the vertex arrays.
        std::vector<unsigned int> indices;
        /// Source mesh face ID for each tessellated triangle.
        std::vector<int> source_faces;
    };
    /// Mesh used to resolve material groups.
    std::shared_ptr<SurfaceMesh> mesh;
    /// Imported material-group snapshots.
    std::vector<Part> parts;
    /**
     * \brief Create one renderer drawable per imported material part.
     * \details Attach the mesh to a host with create_default_drawables=false
     * first. Texture representation follows the host renderer backend. Existing
     * drawables with matching names cause std::logic_error; they are not replaced.
     * \throws std::logic_error if the mesh has no attached renderer or a part
     *         drawable name is already in use.
     */
    void create_drawables() const;
};

/**
 * \brief Load OBJ geometry and its supported MTL diffuse material data.
 * \param filename Path to the OBJ file.
 * \return CPU-side mesh and tessellated material-part snapshots.
 * \throws std::runtime_error for unreadable files, invalid indices, or faces
 *         rejected by the mesh builder.
 * \details Texture lookup failures fall back to the material Kd color. Only
 * diffuse maps/colors are rendered; ambient/specular maps are accepted as
 * color-map fallbacks. Independent material lighting, MTL opacity, and PBR
 * are not implemented.
 */
ObjMaterialMesh load_obj_materials(const std::string& filename);
}
#endif
