/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 ********************************************************************/

#include <easy3d/algo/surface_mesh_tetrahedralization.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/stop_watch.h>

#include <3rd_party/tetgen/tetgen.h>


namespace easy3d {


    SurfaceMeshTetrehedralization::SurfaceMeshTetrehedralization()
            : allow_steiner_points_on_boundary_(true), tag_regions_(false), max_tet_shape_(2.0),
              min_dihedral_angle_(0.0), max_tet_volume_(-1.0), command_line_("") {
    }


    PolyMesh *SurfaceMeshTetrehedralization::apply(SurfaceMesh *mesh) {
        if (!mesh) {
            LOG(WARNING) << "input mesh is NULL";
            return nullptr;
        }

        for (auto e : mesh->edges()) {
            if (mesh->is_border(e)) {
                LOG(WARNING) << "input mesh is not closed, cannot perform tetrahedralization";
                return nullptr;
            }
        }

        StopWatch w;
        LOG(INFO) << "tetrahedralizing...";

        auto *tetgen_args = new tetgenbehavior;
        // Create tetgen argument string from options.
        if (command_line_.empty()) {
            std::ostringstream s;
            // Q: quiet
            // p: input data is surface
            // n: output tet neighbors
            // q: desired quality
            // V: verbose
            s << "Qpnq" << max_tet_shape_;
            LOG(INFO) << "maximum allowed radius-edge ratio: " << max_tet_shape_;

            if (min_dihedral_angle_ > 0) {
                s << "/" << min_dihedral_angle_;
                LOG(INFO) << "minimum allowed dihedral angle: " << min_dihedral_angle_;
            }

            if (max_tet_volume_ > 0.0) {
                s << "a" << max_tet_volume_;
                LOG(INFO) << "max allowed tetrahedron volume: " << max_tet_volume_;
            }

            // A: generate region tags for each shell.
            if (tag_regions_) {
                s << "A";
                LOG(INFO) << "adding a cell property \"c:region\" to indicate different bounded regions";
            }

            // Y: prohibit steiner points on boundaries
            if (allow_steiner_points_on_boundary_)
                LOG(INFO) << "allowing Steiner points on the boundary edges and faces";
            if (!allow_steiner_points_on_boundary_) {
                s << "Y";
                LOG(INFO) << "preserving boundary edges and faces";
            }

            std::string arg_str = s.str();
            LOG(INFO) << "using command line: " << arg_str;
            tetgen_args->parse_commandline(const_cast<char *>(arg_str.c_str()));
        } else {
            LOG(INFO) << "using user-specified command line: " << command_line_;
            tetgen_args->parse_commandline(const_cast<char *>(command_line_.c_str()));
        }

        tetgenio *tetgen_surface = to_tetgen_surface(mesh);
        auto *tetgen_volume = new tetgenio;

        try {
            ::tetrahedralize(tetgen_args, tetgen_surface, tetgen_volume);
        } catch (...) {
            LOG(ERROR) << "tetgen encountered an error, relaunching in diagnose mode";
            tetgen_args->parse_commandline(const_cast<char *>("pd"));
            try {
                ::tetrahedralize(tetgen_args, tetgen_surface, tetgen_volume);
            } catch (const std::exception& e) {
                LOG(ERROR) << e.what() << " Result may not be valid.";
            }
        }

        PolyMesh *result = to_easy3d_poly_mesh(tetgen_volume);

        if (result) {
            LOG(INFO) << "done. "
                      << "#vertex: " << result->n_vertices() << ", "
                      << "#edge: " << result->n_edges() << ", "
                      << "#face: " << result->n_faces() << ", "
                      << "#cell: " << result->n_cells() << "). "
                      << w.time_string();
        } else
            LOG(WARNING) << "tetrehedralization failed. " << w.time_string();

        delete tetgen_args;
        delete tetgen_surface;
        delete tetgen_volume;

        return result;
    }


    tetgenio *SurfaceMeshTetrehedralization::to_tetgen_surface(SurfaceMesh *mesh) const{
        auto *tetgen_surface = new tetgenio;
        tetgen_surface->initialize();

        tetgen_surface->firstnumber = 0;
        tetgen_surface->numberofpoints = static_cast<int>(mesh->n_vertices());

        // Liangliang: tetgen uses double by default.
        tetgen_surface->pointlist = new double[tetgen_surface->numberofpoints * 3];
        int idx = 0;
        for (const auto &p : mesh->points()) {
            tetgen_surface->pointlist[idx++] = p.x;
            tetgen_surface->pointlist[idx++] = p.y;
            tetgen_surface->pointlist[idx++] = p.z;
        }

        tetgen_surface->numberoffacets = static_cast<int>(mesh->n_faces());
        tetgen_surface->facetlist = new tetgenio::facet[tetgen_surface->numberoffacets];
        for (auto face : mesh->faces()) {
            int fid = face.idx();
            tetgenio::facet &f = tetgen_surface->facetlist[fid];
            f.numberofpolygons = 1;
            f.polygonlist = new tetgenio::polygon[f.numberofpolygons];
            f.numberofholes = 0;
            f.holelist = nullptr;
            tetgenio::polygon &p = f.polygonlist[0];
            p.numberofvertices = static_cast<int>(mesh->valence(face));
            p.vertexlist = new int[p.numberofvertices];
            int vid = 0;
            for (auto vertex : mesh->vertices(face)) {
                p.vertexlist[vid++] = vertex.idx();
            }
        }

        return tetgen_surface;
    }


    PolyMesh *SurfaceMeshTetrehedralization::to_easy3d_poly_mesh(tetgenio *volume) const {
        if (volume->numberofpoints <= 0 || volume->numberoftetrahedra <= 0)
            return nullptr;

        auto mesh = new PolyMesh;

        PolyMesh::CellProperty<double> region;
        if (tag_regions_)
            region = mesh-> add_cell_property<double>("c:region");

        double *p = volume->pointlist;
        for (int i = 0; i < volume->numberofpoints; i++) {
            mesh->add_vertex(vec3(static_cast<float>(p[0]), static_cast<float>(p[1]), static_cast<float>(p[2])));
            p += 3;
        }

        int *t = volume->tetrahedronlist;
        for (int i = 0; i < volume->numberoftetrahedra; i++) {
            PolyMesh::Cell c = mesh->add_tetra(
                    PolyMesh::Vertex(t[0] - volume->firstnumber),
                    PolyMesh::Vertex(t[1] - volume->firstnumber),
                    PolyMesh::Vertex(t[2] - volume->firstnumber),
                    PolyMesh::Vertex(t[3] - volume->firstnumber)
                    );

            if (tag_regions_)
                region[c] = volume->tetrahedronattributelist[i];

            t += 4;
        }

        return mesh;
    }

}