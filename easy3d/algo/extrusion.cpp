/********************************************************************
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
 ********************************************************************/


#include <easy3d/algo/extrusion.h>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/algo/tessellator.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    bool extrude(SurfaceMesh *mesh, const std::vector<Polygon2> &contours, float height) {
        Tessellator tessellator;
        tessellator.set_winding_rule(Tessellator::WINDING_ODD);

#if 1   // generate bottom faces
        tessellator.begin_polygon(vec3(0, 0, -1));
        for (std::size_t index = 0; index < contours.size(); ++index) {
            const auto &contour = contours[index];
            tessellator.begin_contour();
            for (std::size_t j = 0; j < contour.size(); ++j) {
                const vec2 p = contour[j];
                Tessellator::Vertex vt(vec3(p, 0));
                vt.push_back(static_cast<double>(index)); // one extra bit to allow stitching within a contour.
                tessellator.add_vertex(vt);
            }
            tessellator.end_contour();
        }
        tessellator.end_polygon();
#endif

#if 1   // generate top faces
        tessellator.begin_polygon(vec3(0, 0, 1));
        for (std::size_t index = 0; index < contours.size(); ++index) {
            const auto &contour = contours[index];
            tessellator.set_winding_rule(Tessellator::WINDING_ODD);
            tessellator.begin_contour();
            for (std::size_t j = 0; j < contour.size(); ++j) {
                const vec2 p = contour[j];
                Tessellator::Vertex vt(vec3(p, height));
                vt.push_back(static_cast<double>(index)); // one extra bit to allow stitching within a contour.
                tessellator.add_vertex(vt);
            }
            tessellator.end_contour();
        }
        tessellator.end_polygon();
#endif

        //-------------------------------------------------------------------------------------
#if 1 // generate the side faces
        for (std::size_t index = 0; index < contours.size(); ++index) {
            const Polygon2 &contour = contours[index];
            for (int j = 0; j < contour.size(); ++j) {
                const vec3 a = vec3(contour[j], 0);
                const vec3 b = vec3(contour[(j + 1 + contour.size()) % contour.size()], 0);
                const vec3 c = a + vec3(0, 0, height);
                const vec3 d = b + vec3(0, 0, height);
                // Though I've already known the vertex indices for the side triangles, I still use my
                // tessellator, which allows me to stitch the triangles into a closed mesh.
                vec3 n = cross(b - a, c - a);
                n.normalize();
                tessellator.begin_polygon(n);
                tessellator.begin_contour();
                Tessellator::Vertex vta(a);
                vta.push_back(static_cast<int>(index)); // one extra bit to allow stitching within a contour.
                tessellator.add_vertex(vta);
                Tessellator::Vertex vtb(b);
                vtb.push_back(static_cast<int>(index)); // one extra bit to allow stitching within a contour.
                tessellator.add_vertex(vtb);
                Tessellator::Vertex vtd(d);
                vtd.push_back(static_cast<int>(index)); // one extra bit to allow stitching within a contour.
                tessellator.add_vertex(vtd);
                Tessellator::Vertex vtc(c);
                vtc.push_back(static_cast<int>(index)); // one extra bit to allow stitching within a contour.
                tessellator.add_vertex(vtc);
                tessellator.end_contour();
                tessellator.end_polygon();
            }
        }
#endif

        //-------------------------------------------------------------------------------------

        // now we can collect the triangle faces

        const auto &elements = tessellator.elements();
        if (elements.empty()) {
            LOG(WARNING) << "extrusion failed: no faces generated";
            return false;
        } else {
            // the vertex index starts from 0 for each character.
            const unsigned int offset = mesh->n_vertices();

            // use SurfaceMeshBuilder (ensuring the final model is manifold).
            SurfaceMeshBuilder builder(mesh);
            builder.begin_surface();

            const auto &final_vertices = tessellator.vertices();
            for (const auto v : final_vertices)
                builder.add_vertex(vec3(v->data()));

            for (const auto &e : elements) {
                builder.add_triangle(SurfaceMesh::Vertex(static_cast<int>(e[0] + offset)),
                                     SurfaceMesh::Vertex(static_cast<int>(e[1] + offset)),
                                     SurfaceMesh::Vertex(static_cast<int>(e[2] + offset)));
            }

            builder.end_surface(false);
            return true;
        }
    }


    SurfaceMesh *extrude(const std::vector<Polygon2> &simple_contours, float height) {
        if (simple_contours.empty())
            return nullptr;
        else {
            auto mesh = new SurfaceMesh;
            if (extrude(mesh, simple_contours, height))
                return mesh;
            else {
                delete mesh;
                return nullptr;
            }
        }
    }

}