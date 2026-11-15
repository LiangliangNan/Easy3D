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
#include <easy3d/algo/surface_mesh_ambient_occlusion.h>
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


static void require_manifold_and_clean(SurfaceMesh *mesh) {
    std::vector<int> incident(mesh->vertices_size(), 0);
    for (auto f : mesh->faces())
        for (auto v : mesh->vertices(f))
            ++incident[v.idx()];
    for (auto v : mesh->vertices()) {
        test::require(mesh->is_manifold(v), "polygonization produced a non-manifold vertex");
        // is_manifold() only detects several boundary gaps; also require a single fan of faces.
        int circulating = 0;
        for (auto h : mesh->halfedges(v)) {
            if (mesh->face(h).is_valid())
                ++circulating;
        }
        test::require(circulating == incident[v.idx()],
                      "polygonization produced a vertex with multiple face fans");
    }
    for (auto f : mesh->faces()) {
        test::require(!mesh->is_degenerate(f), "polygonization produced a degenerate face");
        for (auto h : mesh->halfedges(f)) {
            test::require(mesh->position(mesh->source(h)) != mesh->position(mesh->target(h)),
                          "polygonization produced a zero-length edge");
        }
    }
    // Merged faces must be convex. They are only approximately planar, so each face is checked in its best-fit
    // plane (the Newell normal). Corners whose cross product is negligible against the largest one are collinear
    // up to float precision and are not treated as reflex.
    for (auto f : mesh->faces()) {
        const auto valence = mesh->valence(f);
        if (valence < 4)
            continue;
        std::vector<vec3> p;
        p.reserve(valence);
        for (auto v : mesh->vertices(f))
            p.push_back(mesh->position(v));

        vec3 normal(0.0f, 0.0f, 0.0f);
        for (std::size_t i = 0; i < p.size(); ++i)
            normal += cross(p[i] - p[0], p[(i + 1) % p.size()] - p[0]);
        if (length(normal) < 1e-12f)
            continue;  // cannot orient a self-overlapping face

        std::vector<float> corners(p.size());
        float max_cross = 0.0f;
        for (std::size_t i = 0; i < p.size(); ++i) {
            const vec3 &prev = p[(i + p.size() - 1) % p.size()];
            const vec3 &cur = p[i];
            const vec3 &next = p[(i + 1) % p.size()];
            corners[i] = dot(cross(cur - prev, next - cur), normal);
            max_cross = std::max(max_cross, std::fabs(corners[i]));
        }
        for (float c : corners)
            test::require(c >= -1e-3f * max_cross, "polygonization produced a non-convex face");
    }
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

    // A degenerate face without a non-degenerate edge neighbor keeps id -1 (documented contract); the
    // polygonizer removes it (it encloses no area) instead of indexing per-patch arrays with that id.
    auto* synthetic = new SurfaceMesh;
    auto a = synthetic->add_vertex(vec3(0,0,0));
    auto b = synthetic->add_vertex(vec3(1,0,0));
    auto c = synthetic->add_vertex(vec3(0,1,0));
    auto regular = synthetic->add_triangle(a,b,c);
    auto d = synthetic->add_vertex(vec3(10,0,0));
    auto e = synthetic->add_vertex(vec3(11,0,0));
    auto g = synthetic->add_vertex(vec3(11,0,0));
    auto degenerate = synthetic->add_triangle(d,e,g); // collapsed edge and isolated: degenerate
    auto synthetic_ids = synthetic->face_property<int>("f:planar_partition", -1);
    const int synthetic_num = SurfaceMeshEnumerator::enumerate_planar_components(synthetic, synthetic_ids, 1.0f);
    test::require(synthetic_ids[regular]>=0 && synthetic_ids[regular]<synthetic_num,
                  "a non-degenerate face must belong to a planar patch");
    test::require(synthetic_ids[degenerate]==-1,
                  "an isolated degenerate component must keep the unassigned id -1");
    SurfaceMeshPolygonization polygonizer;
    polygonizer.apply(synthetic);
    test::require(synthetic->n_faces()==1,"polygonization must keep the regular face and drop the degenerate one");
    delete synthetic;
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
    require_manifold_and_clean(mesh);
    delete mesh;

    // house.obj is a non-manifold polygon soup with degenerate faces; the result must be a clean manifold.
    SurfaceMesh *house = SurfaceMeshIO::load(resource::directory() + "/data/house/house.obj");
    test::require(house != nullptr, "failed to load house.obj");
    const unsigned int house_faces = house->n_faces();
    SurfaceMeshPolygonization house_polygonizer;
    house_polygonizer.apply(house);
    test::require(house->n_faces() > 0 && house->n_faces() < house_faces, "house.obj was not polygonized");
    require_manifold_and_clean(house);
    delete house;

    // A textured closed mesh with borders and no degeneracies.
    SurfaceMesh *domik = SurfaceMeshIO::load(resource::directory() + "/data/domik/domik.obj");
    test::require(domik != nullptr, "failed to load domik.obj");
    const unsigned int domik_faces = domik->n_faces();
    SurfaceMeshPolygonization domik_polygonizer;
    domik_polygonizer.apply(domik);
    test::require(domik->n_faces() > 0 && domik->n_faces() < domik_faces, "domik.obj was not polygonized");
    require_manifold_and_clean(domik);
    delete domik;

    // A coplanar patch with two holes must be merged without filling the holes.
    auto* ring = new SurfaceMesh;
    SurfaceMesh::Vertex grid[7][7];
    for (int y = 0; y < 7; ++y)
        for (int x = 0; x < 7; ++x)
            grid[y][x] = ring->add_vertex(vec3(static_cast<float>(x), static_cast<float>(y), 0.0f));

    auto removed = [](int x, int y) {
        return (x == 1 && y == 1) || (x == 3 && y == 3) || (x == 4 && y == 3);
    };
    for (int y = 0; y < 6; ++y) {
        for (int x = 0; x < 6; ++x) {
            if (removed(x, y))
                continue;
            ring->add_triangle(grid[y][x], grid[y][x + 1], grid[y + 1][x + 1]);
            ring->add_triangle(grid[y][x], grid[y + 1][x + 1], grid[y + 1][x]);
        }
    }
    test::require(ring->n_faces() == 66, "unexpected synthetic annulus");

    SurfaceMeshPolygonization ring_polygonizer;
    ring_polygonizer.apply(ring);
    test::require(ring->n_faces() > 0 && ring->n_faces() < 66,
                  "a planar patch with holes was not merged");

    float area = 0.0f;
    for (auto f : ring->faces()) {
        test::require(!ring->is_degenerate(f), "polygonization produced a degenerate face");
        vec3 acc(0.0f, 0.0f, 0.0f);
        auto vertices = ring->vertices(f);
        auto it = vertices.begin();
        auto prev = ring->position(*it);
        const auto first = prev;
        for (++it; it != vertices.end(); ++it) {
            const auto p = ring->position(*it);
            acc += cross(prev, p);
            prev = p;
        }
        acc += cross(prev, first);
        area += 0.5f * acc.z;
    }
    test::require(std::fabs(area - 33.0f) < 1e-3f,
                  "polygonization changed the area of a patch with holes (holes must stay open)");

    auto visited = ring->halfedge_property<bool>("h:boundary_visited", false);
    int loops = 0;
    for (auto h : ring->halfedges()) {
        if (!ring->is_border(h) || visited[h])
            continue;
        ++loops;
        auto cur = h;
        std::size_t guard = 0;
        do {
            visited[cur] = true;
            cur = ring->next(cur);
            ++guard;
        } while (cur.is_valid() && cur != h && guard <= ring->n_halfedges());
    }
    ring->remove_halfedge_property(visited);
    test::require(loops == 3, "the outer boundary and the two holes must remain");
    require_manifold_and_clean(ring);
    delete ring;

    // A self-touching boundary loop must be normalized rather than handed to the mesh builder as a polygon
    // with a repeated vertex (bunny.ply contains such a patch), otherwise the patch is silently dropped.
    SurfaceMesh *bunny = SurfaceMeshIO::load(resource::directory() + "/data/bunny.ply");
    test::require(bunny != nullptr, "failed to load bunny.ply");
    auto mesh_area = [](SurfaceMesh *m) {
        double sum = 0.0;
        for (auto f : m->faces()) {
            std::vector<vec3> p;
            for (auto v : m->vertices(f))
                p.push_back(m->position(v));
            vec3 acc(0.0f, 0.0f, 0.0f);
            for (std::size_t i = 0; i < p.size(); ++i)
                acc += cross(p[i], p[(i + 1) % p.size()]);
            sum += 0.5 * length(acc);
        }
        return sum;
    };
    const int bunny_faces = static_cast<int>(bunny->n_faces());
    const double bunny_area = mesh_area(bunny);
    SurfaceMeshPolygonization bunny_polygonizer;
    bunny_polygonizer.apply(bunny);
    test::require(bunny->n_faces() > 0 && bunny->n_faces() < bunny_faces,
                  "bunny.ply was not polygonized");
    test::require(std::fabs(mesh_area(bunny) - bunny_area) < 1e-3 * bunny_area,
                  "polygonization dropped a self-touching patch of bunny.ply");
    require_manifold_and_clean(bunny);
    delete bunny;
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
    const unsigned int num_overlapping = Surfacer::remove_overlapping_faces(mesh);
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



bool test_algo_surface_mesh_ambient_occlusion() {
    // A floor at z=0 (x in [-10, -0.05]) and a tall wall at x=0. A floor vertex
    // next to the wall sees half of its hemisphere blocked (AO ~ 0.5); a floor
    // vertex farther from the wall than the search radius sees the open sky
    // (AO = 1). The 200-wide wall keeps the side edges out of the kernel.
    // The vertices sit 0.05 from the wall, a hundredth of the search radius,
    // so the truncated kernel still covers essentially the full half-hemisphere.
    SurfaceMesh mesh;
    auto add_quad = [&mesh](const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &d) {
        const auto v0 = mesh.add_vertex(a), v1 = mesh.add_vertex(b);
        const auto v2 = mesh.add_vertex(c), v3 = mesh.add_vertex(d);
        mesh.add_triangle(v0, v1, v2);
        mesh.add_triangle(v0, v2, v3);
    };
    add_quad(vec3(-10, -10, 0), vec3(-0.05f, -10, 0), vec3(-0.05f, 10, 0), vec3(-10, 10, 0));
    add_quad(vec3(0, -100, 0), vec3(0, -100, 10), vec3(0, 100, 10), vec3(0, 100, 0));

    SurfaceMeshAmbientOcclusion::Parameters parameters;
    parameters.radius_ratio = 0.05f; // bounding-sphere radius ~100 -> radius ~5
    parameters.sample_count = 1024;
    parameters.thread_count = 1;
    auto ao = SurfaceMeshAmbientOcclusion::bake(&mesh, parameters);
    test::require(ao, "the bake did not produce a valid occlusion property");

    std::vector<std::pair<int, float> > first;
    for (auto v : mesh.vertices()) {
        const vec3 p = mesh.points()[v.idx()];
        if (p.z != 0 || p.x == 0) // keep the floor vertices only
            continue;
        if (p.x == -0.05f)
            test::require(std::abs(ao[v] - 0.5f) < 0.03f,
                          "a vertex next to a blocking wall was not occluded by about half");
        else if (p.x == -10.0f)
            test::require(ao[v] > 0.999f, "a vertex beyond the search radius was occluded");
        first.emplace_back(v.idx(), ao[v]);
    }
    test::require(!first.empty(), "the test mesh has no floor vertices");

    // The bake is deterministic and does not depend on the thread count.
    parameters.thread_count = 4;
    auto ao_parallel = SurfaceMeshAmbientOcclusion::bake(&mesh, parameters);
    for (const auto &value : first)
        test::require(ao_parallel[SurfaceMesh::Vertex(value.first)] == value.second,
                      "the bake is not deterministic");

    // colorize() writes albedo * occlusion into "v:color".
    SurfaceMeshAmbientOcclusion::colorize(&mesh, ao_parallel, vec3(0.5f, 0.25f, 1.0f));
    auto colors = mesh.get_vertex_property<vec3>("v:color");
    test::require(colors, "colorize() did not create the vertex colors");
    for (auto v : mesh.vertices()) {
        const vec3 expected = vec3(0.5f, 0.25f, 1.0f) * ao_parallel[v];
        test::require(length(colors[v] - expected) < 1e-6f, "colorize() wrote a wrong color");
    }

    // A non-triangle mesh is rejected.
    SurfaceMesh quad;
    const auto q0 = quad.add_vertex(vec3(0, 0, 0)), q1 = quad.add_vertex(vec3(1, 0, 0));
    const auto q2 = quad.add_vertex(vec3(1, 1, 0)), q3 = quad.add_vertex(vec3(0, 1, 0));
    quad.add_face({q0, q1, q2, q3});
    bool rejected = false;
    try { SurfaceMeshAmbientOcclusion::bake(&quad); }
    catch (const std::invalid_argument &) { rejected = true; }
    test::require(rejected, "a non-triangle mesh was accepted");
    return true;
}


int main(int argc,char** argv) { return test::run("surface_mesh algorithms",[&] {
    test::require(argc==2,"expected a registered algorithm case");
    const std::map<std::string,std::function<bool()>> cases{
        {"test_algo_surface_mesh_components",test_algo_surface_mesh_components},
        {"test_algo_surface_mesh_ambient_occlusion",test_algo_surface_mesh_ambient_occlusion},
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
