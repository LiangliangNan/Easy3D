#include "viewer.h"
#include "drawable_points.h"
#include "drawable_lines.h"
#include "drawable_triangles.h"
#include "happly.h"


// a simple PLY mesh loader
bool load_ply_data(const std::string& path, std::vector<lve::MyVertex>& vertices, std::vector<uint32_t>& indices) {
    happly::PLYData plyIn(path);
    const auto points = plyIn.getVertexPositions();
    const auto colors = plyIn.getVertexColors();
    if (points.empty() || colors.empty() || points.size() != colors.size()) {
        std::cerr << points.size() << " vertices from the PLY file" << std::endl;
        printf("loading file failed");
        return false;
    }
    for (auto i = 0; i < points.size(); ++i) {
        const auto pos = points.at(i);
        const auto col = colors.at(i);
        vertices.emplace_back(
                lve::MyVertex(glm::vec3(static_cast<float>(pos[0]), static_cast<float>(pos[1]), static_cast<float>(pos[2])),
                       glm::vec3(static_cast<float>(col[0]), static_cast<float>(col[1]), static_cast<float>(col[2])) / 255.0f)
        );
    }

    const auto faceIndices = plyIn.getFaceIndices<uint32_t>();
    for (const auto& ids : faceIndices) {
        indices.insert(indices.end(), ids.begin(), ids.end());
    }
    std::cout << "model has " << points.size() << " vertices and " << faceIndices.size() << " faces" << std::endl;
    return true;
}


int main(int argc, const char *argv[]) {
    lve::Viewer viewer("Viewer");

    const std::string file_name = std::string(ASSET_DIR) + "mesh.ply";
    std::vector<lve::MyVertex> vertices;
    std::vector<uint32_t> indices;
    load_ply_data(file_name, vertices, indices);

#if 1
    if (0) {
        auto& drawable = viewer.createPointsDrawable();
        drawable.set_name("Vertices");
        drawable.set_visible(true);
        drawable.update_buffers(vertices, indices);
    }
    if (0) {
        auto& drawable = viewer.createLinesDrawable();
        drawable.set_name("Wireframe");
        drawable.set_visible(true);
        drawable.update_buffers(vertices, indices);
    }
    if (1) {
        auto& drawable = viewer.createTrianglesDrawable();
        drawable.set_name("Surface");
        drawable.set_visible(true);
        drawable.update_buffers(vertices, indices);
    }
#endif
    return viewer.run();
}
