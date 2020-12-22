/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */


#include <easy3d/algo/surface_mesh_tetrahedralization.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/util/logging.h>

#include <3rd_party/tetgen/tetgen.h>


namespace easy3d {


    SurfaceMeshTetrehedralization::SurfaceMeshTetrehedralization()
            : add_steiner_points_on_exterior_boundary_(true), add_steiner_points_on_interior_boundary_(true),
              tag_regions_(false), max_tet_shape_(2.0), max_tet_volume_(-1.0) {
    }


    SurfaceMeshTetrehedralization::~SurfaceMeshTetrehedralization() {
    }


    PolyMesh *SurfaceMeshTetrehedralization::tetrahedralize(SurfaceMesh *mesh) {
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

        tetgenbehavior *tetgen_args = new tetgenbehavior;
        // Create tetgen argument string from options.
        if (cmdline_ == "") {
            std::ostringstream s;
            // Q: quiet
            // p: input data is surfacic
            // n: output tet neighbors
            // q: desired quality
            // V: verbose
            s << "Qpnq" << max_tet_shape_;
            if (max_tet_volume_ > 0.0) {
                s << "a" << max_tet_volume_;
            }

            // AA: generate region tags for each shell.
            if (tag_regions_) {
                s << "AA";
            }

            // YY: prohibit steiner points on boundaries (first Y for exterior boundary, second Y for the other ones).
            if (add_steiner_points_on_exterior_boundary_ && !add_steiner_points_on_interior_boundary_) {
                LOG(WARNING)
                        << "invalid combination of flags (does not preserve exterior boundary and preserves interior ones)"
                        << " - preserving exterior boundary as well ...";
                add_steiner_points_on_exterior_boundary_ = false;
            }

            if (!add_steiner_points_on_exterior_boundary_) {
                s << "Y";
            }

            if (!add_steiner_points_on_interior_boundary_) {
                s << "Y";
            }

            std::string arg_str = s.str();
            LOG(INFO) << "using command line: " << arg_str;
            tetgen_args->parse_commandline(const_cast<char *>(arg_str.c_str()));
        } else {
            LOG(INFO) << "using user-specified command line: " << cmdline_;
            tetgen_args->parse_commandline(const_cast<char *>(cmdline_.c_str()));
        }

        tetgenio *tetgen_surface = to_tetgen_surface(mesh);
        tetgenio *tetgen_volume = new tetgenio;

        try {
            ::tetrahedralize(tetgen_args, tetgen_surface, tetgen_volume);
        } catch (...) {
            LOG(ERROR) << "tetgen encountered an error, relaunching in diagnose mode" << std::endl;
            tetgen_args->parse_commandline(const_cast<char *>("d"));
            ::tetrahedralize(tetgen_args, tetgen_surface, tetgen_volume);
        }

        PolyMesh *result = to_easy3d_poly_mesh(tetgen_volume);

        delete tetgen_args;
        delete tetgen_surface;
        delete tetgen_volume;

        return result;
    }


    tetgenio *SurfaceMeshTetrehedralization::to_tetgen_surface(SurfaceMesh *mesh) {
        tetgenio *tetgen_surface = new tetgenio;
        tetgen_surface->initialize();

        tetgen_surface->firstnumber = 0;
        tetgen_surface->numberofpoints = mesh->n_vertices();

        // Liangliang: tetgen uses double by default.
        tetgen_surface->pointlist = new double[tetgen_surface->numberofpoints * 3];
        int idx = 0;
        for (const auto &p : mesh->points()) {
            tetgen_surface->pointlist[idx++] = p.x;
            tetgen_surface->pointlist[idx++] = p.y;
            tetgen_surface->pointlist[idx++] = p.z;
        }

        tetgen_surface->numberoffacets = mesh->n_faces();
        tetgen_surface->facetlist = new tetgenio::facet[tetgen_surface->numberoffacets];
        for (auto face : mesh->faces()) {
            int fid = face.idx();
            tetgenio::facet &f = tetgen_surface->facetlist[fid];
            f.numberofpolygons = 1;
            f.polygonlist = new tetgenio::polygon[f.numberofpolygons];
            f.numberofholes = 0;
            f.holelist = nullptr;
            tetgenio::polygon &p = f.polygonlist[0];
            p.numberofvertices = mesh->valence(face);
            p.vertexlist = new int[p.numberofvertices];
            int vid = 0;
            for (auto vertex : mesh->vertices(face)) {
                p.vertexlist[vid++] = vertex.idx();
            }
        }

        return tetgen_surface;
    }


    PolyMesh *SurfaceMeshTetrehedralization::to_easy3d_poly_mesh(tetgenio *volume) {
        PolyMesh *mesh = new PolyMesh;

        PolyMesh::CellProperty<double> region;
        if (tag_regions_)
            region = mesh-> add_cell_property<double>("c:region");

        double *p = volume->pointlist;
        for (int i = 0; i < volume->numberofpoints; i++) {
            mesh->add_vertex(vec3(p[0], p[1], p[2]));
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