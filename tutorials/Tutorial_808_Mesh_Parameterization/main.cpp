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

#include <easy3d/viewer/viewer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/algo/surface_mesh_parameterization.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_808_Mesh_Parameterization/main.cpp
 *
 * \brief Computing a 2D texture parameterization of an open surface and visualizing it with a checkerboard.
 *
 * <b>Overview</b>
 * This tutorial demonstrates surface parameterization with SurfaceMeshParameterization. It computes a
 * (u, v) coordinate for every vertex of the girl_face mesh — an open surface patch (a scanned face) with
 * a boundary — and visualizes the result by mapping a checkerboard texture onto the surface. Two methods
 * are available: discrete harmonic parameterization and least squares conformal mapping (LSCM).
 * Parameterization is the foundation of texture mapping: it flattens a 3D surface into the 2D domain
 * where images live, so a texture can be painted or stored without seams or excessive distortion.
 *
 * <b>The technique</b>
 * A parameterization is a map from the surface to the plane; the goal is to minimize distortion (of
 * angles and/or areas) while keeping the map bijective. Both methods here require a mesh with a boundary
 * (disk topology) and store the resulting 2D coordinates in the "v:texcoord" vertex property. Discrete
 * harmonic parameterization (Desbrun et al., "Intrinsic parameterizations of surface meshes", CGF 2002)
 * fixes the boundary to a convex polygon (the unit circle) and solves a Laplace equation for the interior
 * vertices, minimizing the Dirichlet (stretching) energy; it is simple and guaranteed flip-free for a
 * convex boundary, but can distort areas. LSCM (Lévy et al., "Least squares conformal maps for automatic
 * texture atlas generation", SIGGRAPH 2002) instead pins only two boundary vertices and solves a least-
 * squares system enforcing the Cauchy-Riemann equations, producing a conformal (angle-preserving) map
 * with free boundaries; it preserves local shapes well but may stretch areas. The checkerboard makes the
 * distortion visible: square, evenly sized cells indicate a good parameterization, while sheared or
 * stretched cells reveal angle/area distortion.
 *
 * <b>Easy3D API</b>
 * - SurfaceMeshParameterization::harmonic(): discrete harmonic map (boundary mapped to the unit circle).
 * - SurfaceMeshParameterization::lscm(): least squares conformal map; requires a triangle mesh, so the
 *   mesh is triangulated first. Both fill the "v:texcoord" vertex property.
 * - DrawableTriangles::set_texture_coloring(): samples a texture using the per-vertex "v:texcoord"
 *   coordinates to paint the checkerboard onto the surface.
 * - Renderer::update(): re-extracts the rendering buffers after the texture coordinates change.
 * - Viewer::bind() / set_usage(): register the Ctrl shortcuts and the help text.
 *
 * <b>Running it</b>
 * The harmonic parameterization is shown first. Press Ctrl+a for harmonic parameterization and Ctrl+l for
 * LSCM; compare how the checkerboard cells deform under each method.
 *
 * \see SurfaceMeshParameterization, SurfaceMesh, DrawableTriangles, Tutorial_801_Mesh_Curvature
 */

using namespace easy3d;

namespace {

    bool parameterize(int method, Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;

        SurfaceMeshParameterization parameterization(mesh);
        if (method == 0)
            parameterization.harmonic();
        else {
            mesh->triangulate(); // LSCM requires a triangle mesh
            parameterization.lscm();
        }

        auto drawable = mesh->renderer()->get_triangles_drawable("faces");
        drawable->set_texture_coloring(State::VERTEX, "v:texcoord");
        mesh->renderer()->update();
        viewer->update();
        return true;
    }

} // namespace

int main(int argc, char **argv) {
    initialize();

    const std::string file = resource::directory() + "/data/girl_face.ply";

    Viewer viewer(EXAMPLE_TITLE);
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer.add_model(file, true));
    if (!mesh) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    viewer.camera()->setUpVector(vec3(0, 1, 0));
    viewer.camera()->setViewDirection(vec3(0, 0, 1));

    // Show the harmonic parameterization initially.
    parameterize(0, &viewer, mesh);

    viewer.bind([](Viewer *v, Model *m) { return parameterize(0, v, m); }, mesh, Viewer::KEY_A, Viewer::MODIF_CTRL);
    viewer.bind([](Viewer *v, Model *m) { return parameterize(1, v, m); }, mesh, Viewer::KEY_L, Viewer::MODIF_CTRL);
    viewer.set_usage("default",
                     "Ctrl + a: harmonic parameterization\n" "Ctrl + l: least squares conformal mapping (LSCM)");

    return viewer.run();
}