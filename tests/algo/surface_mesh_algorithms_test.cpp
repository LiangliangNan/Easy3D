// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <cmath>
#include <map>
#include <functional>
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

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/algo/surface_mesh_components.h>
#include <easy3d/algo/surface_mesh_curvature.h>
#include <easy3d/algo/surface_mesh_enumerator.h>
#include <easy3d/algo/surface_mesh_fairing.h>
#include <easy3d/algo/surface_mesh_geodesic.h>
#include <easy3d/algo/surface_mesh_hole_filling.h>
#include <easy3d/algo/surface_mesh_parameterization.h>
#include <easy3d/algo/surface_mesh_polygonization.h>
#include <easy3d/algo/surface_mesh_remeshing.h>
#include <easy3d/algo/surface_mesh_sampler.h>
#include <easy3d/algo/surface_mesh_simplification.h>
#include <easy3d/algo/surface_mesh_smoothing.h>
#include <easy3d/algo/surface_mesh_stitching.h>
#include <easy3d/algo/surface_mesh_subdivision.h>
#include <easy3d/algo/surface_mesh_tetrahedralization.h>
#include <easy3d/algo/surface_mesh_topology.h>
#include <easy3d/algo/surface_mesh_triangulation.h>
#include <easy3d/algo/surface_mesh_features.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/util/resource.h>

#ifdef HAS_CGAL
#include <easy3d/algo_ext/surfacer.h>
#endif


using namespace easy3d;

bool test_algo_surface_mesh_components() {
    const std::string file = resource::directory() + "/data/house/house.obj";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    const std::vector<SurfaceMeshComponent> &components = SurfaceMeshComponent::extract(mesh);
    std::cout << "mesh has " << components.size() << " components." << std::endl;

    const std::size_t num = 5;
    if (components.size() > num)
        std::cout << "    information about the first " << num << " components:" << std::endl;

    for (std::size_t i = 0; i < std::min(std::size_t(num), components.size()); ++i) {
        const auto &comp = components[i];
        std::cout << "        " << i << ": " << comp.n_faces() << " faces, " << comp.n_vertices() << " vertices, " << comp.n_edges() << " edges, surface area " << comp.area() << ", border_length " << comp.border_length() << "\n";
    }

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


bool test_algo_surface_mesh_topology() {
    const std::string file = resource::directory() + "/data/house/house.obj";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "computing surface mesh topology..." << std::endl;

    const auto &components = SurfaceMeshComponent::extract(mesh);
    std::cout << "model has " << components.size() << " connected components" << std::endl;

    const std::size_t num = 5;
    if (components.size() > num)
        std::cout << "    topology of the first " << num << " components:" << std::endl;

    for (std::size_t i = 0; i < std::min(components.size(), num); ++i) {
        const SurfaceMeshComponent &comp = components[i];
        SurfaceMeshTopology topo(&comp);
        std::string type = "unknown";
        if (topo.is_sphere())
            type = "sphere";
        else if (topo.is_disc())
            type = "disc";
        else if (topo.is_cylinder())
            type = "cylinder";
        else if (topo.is_torus())
            type = "torus";
        else if (topo.is_closed())
            type = "unknown closed";

        std::cout << "        " << i << ": " << type
                  << ", F = " << comp.n_faces() << ", V = " << comp.n_vertices() << ", E = " << comp.n_edges()
                  << ", B = " << topo.number_of_borders();
        if (topo.number_of_borders() == 1)
            std::cout << ", border size = " << topo.largest_border_size();
        else if (topo.number_of_borders() > 1)
            std::cout << ", largest border size = " << topo.largest_border_size();
        std::cout << std::endl;
    }

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


bool test_algo_surface_mesh_curvature() {
    const std::string file = resource::directory() + "/data/mannequin.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    SurfaceMeshCurvature analyzer(mesh);

    std::cout << "computing surface mesh principle curvatures..." << std::endl;
    analyzer.analyze_tensor(2, true);

    std::cout << "computing surface mesh mean curvatures..." << std::endl;
    analyzer.compute_mean_curvature();

    std::cout << "computing surface mesh Gauss curvatures..." << std::endl;
    analyzer.compute_gauss_curvature();

    std::cout << "computing surface mesh max absolute curvatures..." << std::endl;
    analyzer.compute_max_abs_curvature();

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


bool test_algo_surface_mesh_enumerator() {
    const std::string file = resource::directory() + "/data/house/house.obj";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "enumerating connected components..." << std::endl;
    auto connected_components = mesh->face_property<int>("f:connected_component", -1);
    SurfaceMeshEnumerator::enumerate_connected_components(mesh, connected_components);

    std::cout << "enumerating planar components..." << std::endl;
    auto planar_segments = mesh->face_property<int>("f:planar_partition", -1);
    SurfaceMeshEnumerator::enumerate_planar_components(mesh, planar_segments, 1.0f);

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


bool test_algo_surface_mesh_fairing() {
    const std::string file = resource::directory() + "/data/hemisphere.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "fairing by minimizing area ..." << std::endl;
    {
        SurfaceMeshFairing fair(mesh);
        fair.minimize_area(); // Minimize Area
    }

    std::cout << "fairing by minimizing curvature ..." << std::endl;
    {
        SurfaceMeshFairing fair(mesh);
        fair.minimize_curvature();
    }

    std::cout << "fairing by minimizing curvature variation..." << std::endl;
    {
        SurfaceMeshFairing fair(mesh);
        fair.fair(3);
    }

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


bool test_algo_surface_mesh_geodesic() {
    const std::string file = resource::directory() + "/data/mannequin.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "computing geodesic distance from the first vertex..." << std::endl;
    std::vector<SurfaceMesh::Vertex> seeds(1, SurfaceMesh::Vertex(0));

    // compute geodesic distance
    SurfaceMeshGeodesic geodist(mesh);
    geodist.compute(seeds);

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


bool test_algo_surface_mesh_fill_holes() {
    const std::string file = resource::directory() + "/data/bunny.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    // first, find all the boundaries to be filled
    const int allowed_boundary_size = 500;

    std::vector<std::pair<SurfaceMesh::Halfedge, int> > holes;

    auto visited = mesh->add_halfedge_property<bool>("TestSurfaceMeshHoleFilling::h::visited", false);
    for (auto h : mesh->halfedges()) {
        if (!visited[h] && mesh->is_border(h)) {
            int size = 0;
            SurfaceMesh::Halfedge hh = h;
            do {
                visited[hh] = true;
                ++size;
                if (!mesh->is_manifold(mesh->target(hh))) {
                    size += 123456;
                    break;
                }
                hh = mesh->next(hh);
            } while (hh != h);

            if (size < allowed_boundary_size) {
                holes.push_back({h, size});
            }
        }
    }
    mesh->remove_halfedge_property(visited);

    std::cout << "filling holes... ";
    // close holes whose sizes are smaller than the min allowed boundary size
    int num_closed = 0;
    for (const auto &hole : holes) {
        SurfaceMeshHoleFilling hf(mesh);
        hf.fill_hole(hole.first);
        ++num_closed;
    }
    std::cout << num_closed << " (out of " << holes.size() << ") holes filled" << std::endl;

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


bool test_algo_surface_mesh_parameterization() {
    const std::string file = resource::directory() + "/data/mannequin.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    SurfaceMeshParameterization para(mesh);

    std::cout << "parameterization (least squares conformal map) ..." << std::endl;
    para.lscm();

    std::cout << "parameterization (discrete harmonic) ..." << std::endl;
    para.harmonic();

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


bool test_algo_surface_mesh_polygonization() {
    const std::string file = resource::directory() + "/data/fandisk.off";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "polygonization..." << std::endl;

#ifdef HAS_CGAL
    // stitch first: to encourage large polyons
    Surfacer::stitch_borders(mesh);
    Surfacer::merge_reversible_connected_components(mesh);
#endif

    // polygonization
    SurfaceMeshPolygonization polygonizer;
    polygonizer.apply(mesh);

#ifdef HAS_CGAL
    // stitch again (the "merge-edge" edge operation in polygonization may result in some borders)
    Surfacer::stitch_borders(mesh);
    Surfacer::merge_reversible_connected_components(mesh);
#endif

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


bool test_algo_surface_mesh_remeshing() {
    const std::string file = resource::directory() + "/data/bunny.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    { // detect features
        SurfaceMeshFeatures sf(mesh);
        sf.clear();
        sf.detect_angle(60.0f);
        sf.detect_boundary();
    }

    std::cout << "uniform remeshing..." << std::endl;
    {
        float len(0.0f);
        for (auto eit : mesh->edges())
            len += distance(mesh->position(mesh->vertex(eit, 0)),
                            mesh->position(mesh->vertex(eit, 1)));
        len /= static_cast<float>(mesh->n_edges());
        SurfaceMeshRemeshing(mesh).uniform_remeshing(len);
    }

    std::cout << "adaptive remeshing..." << std::endl;
    {
        auto bb = mesh->bounding_box().diagonal_length();
        SurfaceMeshRemeshing(mesh).adaptive_remeshing(
                0.001f * bb,  // min length
                0.100f * bb,  // max length
                0.001f * bb); // approx. error
    }

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


bool test_algo_surface_mesh_sampler() {
    const std::string file = resource::directory() + "/data/bunny.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "sampling surface mesh..." << std::endl;
    SurfaceMeshSampler sampler;
    PointCloud *cloud = sampler.apply(mesh, 100000);
    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;

    if (cloud) {
        delete cloud;
        return true;
    }
    return false;
}


bool test_algo_surface_mesh_simplification() {
    const std::string file = resource::directory() + "/data/bunny.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "simplification of surface mesh..." << std::endl;
    const float normal_deviation = 180.0f;
    const float aspect_ratio = 10.0f;

    const unsigned int expected_vertex_number = static_cast<unsigned int>(mesh->n_vertices() * 0.5f);
    SurfaceMeshSimplification ss(mesh);
    ss.initialize(aspect_ratio, 0.0f, 0.0f, normal_deviation, 0.0f);
    ss.simplify(expected_vertex_number);

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


bool test_algo_surface_mesh_smoothing() {
    const std::string file = resource::directory() + "/data/bunny.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "explicit smoothing..." << std::endl;
    {
        SurfaceMeshSmoothing smoother(mesh);
        smoother.explicit_smoothing(2, true);
    }

    std::cout << "implicit smoothing..." << std::endl;
    {
        const float timestep = 0.001f;

        // does the mesh have a boundary?
        bool has_boundary = false;
        for (auto v: mesh->vertices())
            if (mesh->is_border(v))
                has_boundary = true;

        // only re-scale if we don't have a (fixed) boundary
        const bool rescale = !has_boundary;

        SurfaceMeshSmoothing smoother(mesh);
        smoother.implicit_smoothing(timestep, true, rescale);
    }

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


bool test_algo_surface_mesh_stitching() {
    const std::string file = resource::directory() + "/data/house/house.obj";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "stitching surface mesh..." << std::endl;

#ifdef HAS_CGAL
    Surfacer::stitch_borders(mesh);
    Surfacer::merge_reversible_connected_components(mesh);
#else
    SurfaceMeshStitching stitch(mesh);
    stitch.apply();
#endif

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


bool test_algo_surface_mesh_subdivision() {
    const std::string file = resource::directory() + "/data/sphere.obj";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "Loop subdivision..." << std::endl;
    if (!SurfaceMeshSubdivision::loop(mesh)) {
        test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
        return false;
    }

    std::cout << "Sqrt3 subdivision..." << std::endl;
    if (!SurfaceMeshSubdivision::catmull_clark(mesh)) {
        test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
        return false;
    }

    std::cout << "CatmullClark subdivision..." << std::endl;
    if (!SurfaceMeshSubdivision::catmull_clark(mesh)) {
        test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
        return false;
    }

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


bool test_algo_surface_mesh_tetrahedralization() {
    const std::string file = resource::directory() + "/data/sphere.obj";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "tetrehedralization..." << std::endl;
    SurfaceMeshTetrehedralization tetra;
    PolyMesh* result = tetra.apply(mesh);
    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;

    if (result) {
        test::require(result->n_cells()>0,"tetrahedralization produced no cells");
        delete result;
        return true;
    }

    return false;
}


bool test_algo_surface_mesh_triangulation() {
    const std::string file = resource::directory() + "/data/girl_face.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "triangulating surface mesh..." << std::endl;

    SurfaceMeshTriangulation triangulator(mesh);
    triangulator.triangulate(SurfaceMeshTriangulation::MIN_AREA);

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


#ifdef HAS_CGAL

bool test_surface_mesh_remesh_self_intersections() {
    const std::string file = resource::directory() + "/data/repair/self_intersection/two_spheres.obj";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }
    std::cout << "remeshing self intersections..." << std::endl;

    auto size = mesh->n_faces();
    if (Surfacer::remesh_self_intersections(mesh, true)) {
        std::cout << "done. #faces " << size << " -> " << mesh->n_faces() << ". " << std::endl;
        test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
        return true;
    }

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return false;
}

// A triangle with its own three vertices, so that faces can be stacked on the same
// positions without sharing anything with the face underneath.
static void add_triangle(SurfaceMesh *mesh, const vec3 &a, const vec3 &b, const vec3 &c) {
    const SurfaceMesh::Vertex v0 = mesh->add_vertex(a);
    const SurfaceMesh::Vertex v1 = mesh->add_vertex(b);
    const SurfaceMesh::Vertex v2 = mesh->add_vertex(c);
    mesh->add_face({v0, v1, v2});
}

bool test_surface_mesh_remove_overlapping_faces() {
    // Six triangles instead of house.obj: the detection cost is dominated by the CGAL
    // box intersection and the unoptimized build does not finish with ten thousand faces
    // before the timeout. Here the duplicate pair sits at the origin, the folding pair
    // (two coplanar triangles sharing the edge from (5,0,0) to (6,0,0), apexes on the same
    // side) at x = 5, and nothing overlaps with the last two faces.
    SurfaceMesh *mesh = new SurfaceMesh();
    add_triangle(mesh, vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0));
    add_triangle(mesh, vec3(0, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0));
    add_triangle(mesh, vec3(5, 0, 0), vec3(6, 0, 0), vec3(5, 1, 0));
    add_triangle(mesh, vec3(5, 0, 0), vec3(6, 0, 0), vec3(6, 1, 0));
    add_triangle(mesh, vec3(0, 10, 0), vec3(1, 10, 0), vec3(0, 11, 0));
    add_triangle(mesh, vec3(10, 10, 0), vec3(11, 10, 0), vec3(10, 11, 0));
    std::cout << "removing overlapping faces from " << mesh->n_faces() << " triangles..." << std::endl;

    std::vector<std::pair<SurfaceMesh::Face, SurfaceMesh::Face> > duplicates, foldings;
    Surfacer::detect_overlapping_faces(mesh, duplicates, foldings);
    test::require(!duplicates.empty() && !foldings.empty(), "the test case has nothing to remove");
    test::require(duplicates.size() == 1 && foldings.size() == 1, "the overlapping faces were misclassified");

    const unsigned int num_degenerate = Surfacer::remove_degenerate_faces(mesh, 1e-5f);
    const unsigned int num_overlapping = Surfacer::remove_overlapping_faces(mesh, true);
    std::cout << "done. " << num_degenerate << " degenerate, " << num_overlapping << " overlapping faces removed"
              << std::endl;

    test::require(num_degenerate == 0, "a valid face was removed as degenerate");
    test::require(num_overlapping == duplicates.size() + foldings.size(), "one face per overlapping pair was not removed");
    test::require(mesh->n_faces() == 4, "the faces that do not overlap were not kept");

    duplicates.clear();
    foldings.clear();
    Surfacer::detect_overlapping_faces(mesh, duplicates, foldings);
    test::require(duplicates.empty() && foldings.empty(), "overlapping faces survived the removal");

    test::require(mesh->n_vertices() > 0 && mesh->n_faces() > 0, "algorithm produced an empty surface");
    for (const auto &p: mesh->points())
        test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z), "algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


bool test_surface_mesh_clip() {
    const std::string file = resource::directory() + "/data/bunny.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    Plane3 plane(mesh->bounding_box().center(), vec3(0, 0, 1));

    std::cout << "clipping surface mesh..." << std::endl;
    if (Surfacer::clip(mesh, plane, false)) {
        test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
        return true;
    }

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return false;
}


bool test_surface_mesh_split() {
    const std::string file = resource::directory() + "/data/bunny.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    Plane3 plane(mesh->bounding_box().center(), vec3(0, 0, 1));

    std::cout << "splitting surface mesh..." << std::endl;
    Surfacer::split(mesh, plane);

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return true;
}


bool test_surface_mesh_slice() {
    const std::string file = resource::directory() + "/data/bunny.ply";
    SurfaceMesh *mesh = SurfaceMeshIO::load(file);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct."
                  << std::endl;
        return false;
    }

    std::cout << "slicing surface mesh (by 10 horizontal planes)..." << std::endl;

    float minz = mesh->bounding_box().min_point().z;
    float maxz = mesh->bounding_box().max_point().z;

    unsigned int num = 10;
    float step = (maxz - minz) / num;

    std::vector<Plane3> planes(num);
    for (unsigned int i=0; i<num; ++i)
        planes[i] = Plane3(vec3(0, 0, minz + i * step), vec3(0, 0, 1));

    const std::vector< std::vector<Surfacer::Polyline> >& all_polylines = Surfacer::slice(mesh, planes);
    if (!all_polylines.empty()) {
        test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
        return true;
    }

    test::require(mesh->n_vertices()>0 && mesh->n_faces()>0,"algorithm produced an empty surface");
    for(const auto& p:mesh->points()) test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z),"algorithm produced non-finite geometry");
    delete mesh;
    return false;
}

#endif



int main(int argc,char** argv) { return test::run("surface_mesh algorithms",[&] {
    test::require(argc==2,"expected a registered algorithm case");
    const std::map<std::string,std::function<bool()>> cases{
        {"test_algo_surface_mesh_components",test_algo_surface_mesh_components},
        {"test_algo_surface_mesh_topology",test_algo_surface_mesh_topology},
        {"test_algo_surface_mesh_curvature",test_algo_surface_mesh_curvature},
        {"test_algo_surface_mesh_enumerator",test_algo_surface_mesh_enumerator},
        {"test_algo_surface_mesh_fairing",test_algo_surface_mesh_fairing},
        {"test_algo_surface_mesh_geodesic",test_algo_surface_mesh_geodesic},
        {"test_algo_surface_mesh_fill_holes",test_algo_surface_mesh_fill_holes},
        {"test_algo_surface_mesh_parameterization",test_algo_surface_mesh_parameterization},
        {"test_algo_surface_mesh_polygonization",test_algo_surface_mesh_polygonization},
        {"test_algo_surface_mesh_remeshing",test_algo_surface_mesh_remeshing},
        {"test_algo_surface_mesh_sampler",test_algo_surface_mesh_sampler},
        {"test_algo_surface_mesh_simplification",test_algo_surface_mesh_simplification},
        {"test_algo_surface_mesh_smoothing",test_algo_surface_mesh_smoothing},
        {"test_algo_surface_mesh_stitching",test_algo_surface_mesh_stitching},
        {"test_algo_surface_mesh_subdivision",test_algo_surface_mesh_subdivision},
        {"test_algo_surface_mesh_tetrahedralization",test_algo_surface_mesh_tetrahedralization},
        {"test_algo_surface_mesh_triangulation",test_algo_surface_mesh_triangulation},
#ifdef HAS_CGAL
        {"test_surface_mesh_remesh_self_intersections",test_surface_mesh_remesh_self_intersections},
#endif
#ifdef HAS_CGAL
        {"test_surface_mesh_remove_overlapping_faces",test_surface_mesh_remove_overlapping_faces},
#endif
#ifdef HAS_CGAL
        {"test_surface_mesh_clip",test_surface_mesh_clip},
#endif
#ifdef HAS_CGAL
        {"test_surface_mesh_split",test_surface_mesh_split},
#endif
#ifdef HAS_CGAL
        {"test_surface_mesh_slice",test_surface_mesh_slice},
#endif
    };
    const auto found=cases.find(argv[1]);test::require(found!=cases.end(),"unknown algorithm case");
    test::require(found->second(),argv[1]);
}); }
