/** \cond LICENSE
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
 \endcond */

#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_107_SurfaceMesh_IO/main.cpp
 *
 * \brief Loading a SurfaceMesh from a file and saving it to another with SurfaceMeshIO.
 *
 * <b>Overview</b>
 * The mesh counterpart of Tutorial_103_PointCloud_IO. This console program loads the bundled
 * "sphere.obj", reports its vertex/edge/face counts, leaves a placeholder where you would process
 * the mesh, and writes it back out as "sphere-copy.obj". It shows the standard load -> use -> save
 * -> free lifecycle for a surface mesh.
 *
 * <b>The technique</b>
 * SurfaceMeshIO dispatches on the file extension and supports several mesh formats (ply, obj,
 * off, stl, poly), in ASCII or binary as the format allows. The Wavefront OBJ used here lists
 * vertex positions and face indices (and optionally normals/texture coordinates). On load, Easy3D
 * reconstructs the halfedge connectivity from those faces — building a consistent, orientable
 * 2-manifold — so the result is immediately ready for the adjacency queries of Tutorial_105.
 * Two ownership/return conventions matter: load() returns a heap-allocated SurfaceMesh* (nullptr
 * on failure) that the caller must delete, and save() returns a bool telling you whether the
 * write succeeded. The output extension determines the format written.
 *
 * <b>Easy3D API</b>
 * - SurfaceMeshIO::load(file) / SurfaceMeshIO::save(file, mesh): extension-driven mesh I/O.
 * - resource::directory(): finds the bundled data folder containing sphere.obj.
 * - SurfaceMesh::n_vertices()/n_edges()/n_faces(): element counts printed after loading.
 *
 * <b>Running it</b>
 * Console-only. It prints "mesh loaded." followed by the vertex, edge and face counts, then
 * "mesh saved to './sphere-copy.obj'". The copy is written to the current working directory and
 * can be re-loaded by any OBJ-capable tool.
 *
 * \see SurfaceMesh, SurfaceMeshIO, Tutorial_103_PointCloud_IO, Tutorial_111_Graph_IO, Tutorial_115_PolyMesh_IO
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    // Read a mesh specified by its file name. The ".obj" extension selects the format; load()
    // returns a heap-allocated mesh (nullptr on failure) that we own and must delete later.
    const std::string file_name = resource::directory() + "/data/sphere.obj";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file_name);
    if (!mesh) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "mesh loaded. " << std::endl;
    std::cout << "\tvertices: " << mesh->n_vertices() << std::endl;
    std::cout << "\tedges: " << mesh->n_edges() << std::endl;
    std::cout << "\tfaces: " << mesh->n_faces() << std::endl;

    // ...
    // Do fancy stuff with the mesh
    // ...

    // Write the mesh to a new file. The ".obj" extension picks the output format; save() returns
    // true on success so we can report whether the copy was actually written.
    const std::string save_file_name = "./sphere-copy.obj";
    if (SurfaceMeshIO::save(save_file_name, mesh))
        std::cout << "mesh saved to \'" << save_file_name << "\'" << std::endl;
    else
        std::cerr << "failed create the new file" << std::endl;

    // delete the mesh (i.e., release memory)
    delete mesh;

    return EXIT_SUCCESS;
}
