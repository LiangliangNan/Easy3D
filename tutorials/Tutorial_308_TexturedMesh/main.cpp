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

#include "viewer.h"
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_308_TexturedMesh/main.cpp
 *
 * \brief Visualizing textured meshes by overriding the viewer's model-loading function.
 *
 * <b>Overview</b>
 * The default Easy3D Viewer loads geometry but does not display OBJ texture maps. This
 * tutorial shows the extension point for that: subclass Viewer and override add_model() so
 * that OBJ files are parsed together with their materials (MTL) and one textured drawable is
 * created per material part. The TexturedViewer class below does all the work; main() just
 * loads a textured model and runs the viewer.
 *
 * <b>The technique</b>
 * Texture mapping "paints" an image onto a surface: each mesh vertex carries a 2D texture
 * coordinate (u, v) in [0, 1], and the rasterizer interpolates these coordinates across each
 * triangle so every fragment can look up its color in the image (the diffuse map). OBJ files
 * store the geometry plus per-vertex UVs, and a companion .mtl file assigns materials — diffuse
 * colors and texture image files — to groups of faces. Because a vertex on a material or UV
 * seam needs different attributes per adjacent face, the loader splits the mesh into parts
 * (one per material) with seam-preserving duplicated vertices, and each part becomes its own
 * drawable with its own texture. If a texture image is missing, the material's diffuse color
 * (Kd) is used as a fallback. Only diffuse maps/colors are rendered here; full MTL lighting
 * and PBR are out of scope.
 *
 * <b>Easy3D API</b>
 * - Viewer::add_model(): virtual, so a subclass can replace the loading pipeline.
 * - load_obj_materials(): parses an OBJ/MTL pair into an ObjMaterialMesh — a SurfaceMesh plus
 *   one Part per material (diffuse color, texture file, positions/normals/UVs/indices).
 * - ObjMaterialMesh::create_drawables(): creates one textured drawable per part in the mesh's
 *   Renderer, with CPU or OpenGL textures matching the active backend.
 * - SurfaceMesh, Renderer, Camera: the usual model/rendering/camera trio.
 *
 * <b>Running it</b>
 * The textured "domik" house model is loaded and displayed; navigate with the standard viewer
 * controls (left-drag rotate, right-drag pan, wheel zoom). Models without textures still load
 * through the default path, so the viewer remains a drop-in replacement.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_308_TexturedMesh/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_308_TexturedMesh/viewer.cpp
 *
 * \see Viewer, SurfaceMesh, Renderer, ObjMaterialMesh, load_obj_materials
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    // create the viewer.
    // TexturedViewer overrides add_model() to parse OBJ materials and build textured drawables.
    TexturedViewer viewer(EXAMPLE_TITLE);

    //----------------------- Load a mesh from a file ------------------------

    // domik.obj references an MTL file with diffuse texture maps, so it shows real texturing.
    const std::string &file_name = resource::directory() + "/data/domik/domik.obj";
    //const std::string& file_name = resource::directory() + "/data/cube/cube.obj";
    // This call dispatches to our overridden add_model(), not the base Viewer's loader.
    if (!viewer.add_model(file_name)) {
        LOG(ERROR) << "failed to load model. Please make sure the file exists and format is correct";
        return EXIT_FAILURE;
    }

    // run the viewer
    return viewer.run();
}
