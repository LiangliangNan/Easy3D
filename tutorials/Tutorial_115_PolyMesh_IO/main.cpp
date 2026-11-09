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

#include <easy3d/core/poly_mesh.h>
#include <easy3d/fileio/poly_mesh_io.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_115_PolyMesh_IO/main.cpp
 *
 * \brief Loading a polyhedral mesh from a file and saving it back out.
 *
 * <b>Overview</b>
 * This console tutorial completes the PolyMesh series (112-114) by showing file input/output. It
 * loads a polyhedral sphere from disk, reports how many vertices, edges, faces, and cells it
 * contains, then writes an identical copy to a new file. No viewer is opened.
 *
 * <b>The technique</b>
 * Persisting geometry means serializing both the element coordinates and the connectivity (which
 * vertices form each face, which faces bound each cell). Easy3D provides a dedicated PolyMeshIO
 * helper for this. The file extension selects the format: ".plm" is Easy3D's built-in ASCII
 * polyhedral format (".pm" is the binary counterpart). load() returns a heap-allocated mesh, or
 * nullptr if the file is missing or malformed — so the result must always be checked. save()
 * returns true on success. Because the caller owns the loaded mesh, it must delete it when done.
 *
 * <b>Easy3D API</b>
 * - PolyMeshIO::load(file_name): reads a polyhedral mesh; returns PolyMesh* (nullptr on failure).
 * - PolyMeshIO::save(file_name, mesh): writes the mesh; returns bool.
 * - PolyMesh::n_vertices() / n_edges() / n_faces() / n_cells(): element counts for the four
 *   element types of a volumetric mesh.
 * - resource::directory(): locates Easy3D's bundled sample data (here "data/sphere.plm").
 *
 * <b>Running it</b>
 * Console-only. It prints "mesh loaded." followed by the four element counts, then
 * "mesh saved to './sphere-copy.plm'". A copy of the sphere is written to the current working
 * directory. The mesh is explicitly deleted before exit.
 *
 * \see PolyMesh, PolyMeshIO, Tutorial_112_PolyMesh, Tutorial_107_SurfaceMesh_IO
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // Read a mesh specified by its file name. The ".plm" extension selects Easy3D's ASCII
    // polyhedral format. load() returns nullptr on failure, so we must check the result.
    const std::string file_name = resource::directory() + "/data/sphere.plm";
    PolyMesh *mesh = PolyMeshIO::load(file_name);
    if (!mesh) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }
    // Report the four element counts that characterize a volumetric (polyhedral) mesh.
    std::cout << "mesh loaded. " << std::endl;
    std::cout << "\tvertices: " << mesh->n_vertices() << std::endl;
    std::cout << "\tedges: " << mesh->n_edges() << std::endl;
    std::cout << "\tfaces: " << mesh->n_faces() << std::endl;
    std::cout << "\tcells: " << mesh->n_cells() << std::endl;

    // ...
    // Do fancy stuff with the mesh
    // ...

    // Write the mesh to a new file. save() returns true on success; the ".plm" extension again
    // selects the ASCII format. The copy lands in the current working directory.
    const std::string save_file_name = "./sphere-copy.plm";
    if (PolyMeshIO::save(save_file_name, mesh))
        std::cout << "mesh saved to \'" << save_file_name << "\'" << std::endl;
    else
        std::cerr << "failed create the new file" << std::endl;

    // Delete the mesh (i.e., release memory). The caller owns what load() returned.
    delete mesh;

    return EXIT_SUCCESS;
}
