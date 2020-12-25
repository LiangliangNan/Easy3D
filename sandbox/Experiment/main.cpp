#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/core/graph.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/core/curve.h>
#include <easy3d/util/string.h>
#include <easy3d/util/stop_watch.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/core/surface_mesh.h>

#include <cstring>
#include <cstdio>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>

#define STB_TRUETYPE_IMPLEMENTATION
#include <3rd_party/stb/stb_truetype.h>

#include <3rd_party/meshoptimizer/src/meshoptimizer.h>

using namespace easy3d;


void simplify(const SurfaceMesh& mesh)
{
    static const size_t lod_count = 5;

    std::vector<vec3> mesh_vertices = mesh.points();
    std::vector<unsigned int> mesh_indices(mesh.n_faces() * 3);
    int idx = 0;
    for (auto f : mesh.faces()) {
        for (auto v : mesh.vertices(f)) {
            mesh_indices[idx] = v.idx();
            ++idx;
        }
    }

    StopWatch w;

    // generate 4 LOD levels (1-4), with each subsequent LOD using 70% triangles
    // note that each LOD uses the same (shared) vertex buffer
    std::vector<unsigned int> lods[lod_count];
    lods[0] = mesh_indices;

    for (size_t i = 1; i < lod_count; ++i)
    {
        std::vector<unsigned int>& lod = lods[i];

        float threshold = powf(0.7f, float(i));
        size_t target_index_count = size_t(mesh_indices.size() * threshold) / 3 * 3;
        float target_error = 1e-2f;

        // we can simplify all the way from base level or from the last result
        // simplifying from the base level sometimes produces better results, but simplifying from last level is faster
        const std::vector<unsigned int>& source = lods[i - 1];

        if (source.size() < target_index_count)
            target_index_count = source.size();

        lod.resize(source.size());
        lod.resize(meshopt_simplify(&lod[0], &source[0], source.size(), &mesh_vertices[0].x, mesh_vertices.size(), sizeof(vec3), target_index_count, target_error));
    }

    double middle = w.elapsed_seconds(4);
    w.restart();

    // optimize each individual LOD for vertex cache & overdraw
    for (size_t i = 0; i < lod_count; ++i)
    {
        std::vector<unsigned int>& lod = lods[i];

        meshopt_optimizeVertexCache(&lod[0], &lod[0], lod.size(), mesh_vertices.size());
        meshopt_optimizeOverdraw(&lod[0], &lod[0], lod.size(), &mesh_vertices[0].x, mesh_vertices.size(), sizeof(vec3), 1.0f);

        printf("\tLOD %d: %d triangles\n", int(i), int(lod.size() / 3));
    }

    // concatenate all LODs into one IB
    // note: the order of concatenation is important - since we optimize the entire IB for vertex fetch,
    // putting coarse LODs first makes sure that the vertex range referenced by them is as small as possible
    // some GPUs process the entire range referenced by the index buffer region so doing this optimizes the vertex transform
    // cost for coarse LODs
    // this order also produces much better vertex fetch cache coherency for coarse LODs (since they're essentially optimized first)
    // somewhat surprisingly, the vertex fetch cache coherency for fine LODs doesn't seem to suffer that much.
    size_t lod_index_offsets[lod_count] = {};
    size_t lod_index_counts[lod_count] = {};
    size_t total_index_count = 0;

    for (int i = lod_count - 1; i >= 0; --i)
    {
        lod_index_offsets[i] = total_index_count;
        lod_index_counts[i] = lods[i].size();

        total_index_count += lods[i].size();
    }

    std::vector<unsigned int> indices(total_index_count);

    for (size_t i = 0; i < lod_count; ++i)
        memcpy(&indices[lod_index_offsets[i]], &lods[i][0], lods[i].size() * sizeof(lods[i][0]));

    std::vector<vec3> vertices = mesh_vertices;

    // vertex fetch optimization should go last as it depends on the final index order
    // note that the order of LODs above affects vertex fetch results
    meshopt_optimizeVertexFetch(&vertices[0], &indices[0], indices.size(), &vertices[0], vertices.size(), sizeof(vec3));

    double end = w.elapsed_seconds(4);

    printf("%d triangles => %d LOD levels down to %d triangles in %.2f sec, optimized in %.2f sec\n",
           int(lod_index_counts[0]) / 3, int(lod_count), int(lod_index_counts[lod_count - 1]) / 3, middle, end);

    // for using LOD data at runtime, in addition to vertices and indices you have to save lod_index_offsets/lod_index_counts.
    // check meshoptimizer -> "simplifyComplete(const Mesh& mesh)"




    // for each LOD, we create a mesh
    for (size_t i = 1; i < lod_count; ++i) {
        SurfaceMesh m;
        for (auto p : mesh_vertices)
            m.add_vertex(p);

        const auto& indices = lods[i];
        for (size_t j = 0; j < indices.size(); j+=3) {
            m.add_triangle(
                    SurfaceMesh::Vertex(indices[j]),
                    SurfaceMesh::Vertex(indices[j + 1]),
                    SurfaceMesh::Vertex(indices[j + 2])
            );
        }
        m.write_smesh("lod" + std::to_string(i) + ".smesh");
    }
}



int main(int argc, char **argv) {
    // Initialize logging.
    logging::initialize();

    const std::string file_name = resource::directory() + "/data/mannequin2.ply";
    SurfaceMesh* mesh = SurfaceMeshIO::load(file_name);
    if (!mesh) {
        LOG(ERROR) << "Error: failed to load model. Please make sure the file exists and format is correct.";
        return EXIT_FAILURE;
    }

    simplify(*mesh);

    Viewer viewer("Optmizing rendering");
    viewer.add_model(mesh);

    // Run the viewer
    return viewer.run();
}
