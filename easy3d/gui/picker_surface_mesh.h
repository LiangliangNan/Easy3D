
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

#ifndef EASY3D_GUI_PICKER_SURFACE_MESH_H
#define EASY3D_GUI_PICKER_SURFACE_MESH_H

#include <easy3d/gui/picker.h>
#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    class ShaderProgram;

    class SurfaceMeshPicker : public Picker {
    public:
        SurfaceMeshPicker(Camera *cam);

        ~SurfaceMeshPicker();

        //------------------ sensitivity  ---------------------

        int resolution() const { return hit_resolution_; }

        void set_resolution(int r) { hit_resolution_ = r; }

        //------------------ pick elements ---------------------

        /**
         * Pick a face from a surface mesh given the cursor position.
         * @param gl_x/gl_y The cursor position, relative to the bottom-left corner of the OpenGL viewport. Client code
         *                  should convert their viewer-dependent cursor position in the screen coordinate system to
         *                  the one following the OpenGL convention, i.e.,
         *                          int gl_x = x;
         *                          int gl_y = screen_height() - 1 - y;
         *                  Besides, client code is also responsible for handling high-DPI scaling, i.e.,
         *                          gl_x *= static_cast<int>(glx * 2);
         *                          gl_y *= static_cast<int>(gly * 2);
         * @return The picked face.
         */
        SurfaceMesh::Face pick_face(SurfaceMesh *model, int gl_x, int gl_y);

        /**
         * Pick a vertex from a surface mesh given the cursor position.
         * @param gl_x/gl_y The cursor position, relative to the bottom-left corner of the OpenGL viewport. Client code
         *                  should convert their viewer-dependent cursor position in the screen coordinate system to
         *                  the one following the OpenGL convention, i.e.,
         *                          int gl_x = x;
         *                          int gl_y = screen_height() - 1 - y;
         *                  Besides, client code is also responsible for handling high-DPI scaling, i.e.,
         *                          gl_x *= static_cast<int>(glx * 2);
         *                          gl_y *= static_cast<int>(gly * 2);
         * @return The picked vertex.
         */
        SurfaceMesh::Vertex pick_vertex(SurfaceMesh *model, int gl_x, int gl_y);

        /**
         * Pick an edge from a surface mesh given the cursor position.
         * @param gl_x/gl_y The cursor position, relative to the bottom-left corner of the OpenGL viewport. Client code
         *                  should convert their viewer-dependent cursor position in the screen coordinate system to
         *                  the one following the OpenGL convention, i.e.,
         *                          int gl_x = x;
         *                          int gl_y = screen_height() - 1 - y;
         *                  Besides, client code is also responsible for handling high-DPI scaling, i.e.,
         *                          gl_x *= static_cast<int>(glx * 2);
         *                          gl_y *= static_cast<int>(gly * 2);
         * @return The picked halfedge.
         */
        SurfaceMesh::Halfedge pick_edge(SurfaceMesh *model, int gl_x, int gl_y);

        /**
         * Pick a vertex from a surface mesh given the cursor position and a known picked face.
         * @param gl_x/gl_y The cursor position, relative to the bottom-left corner of the OpenGL viewport. Client code
         *                  should convert their viewer-dependent cursor position in the screen coordinate system to
         *                  the one following the OpenGL convention, i.e.,
         *                          int gl_x = x;
         *                          int gl_y = screen_height() - 1 - y;
         *                  Besides, client code is also responsible for handling high-DPI scaling, i.e.,
         *                          gl_x *= static_cast<int>(glx * 2);
         *                          gl_y *= static_cast<int>(gly * 2);
         * @return The picked vertex.
         * @attention This method must be called after calling to pick_face(). The result is valid only if the
         *            picked_face is valid.
         */
        // @attention call this version if you already picked the face
        SurfaceMesh::Vertex pick_vertex(SurfaceMesh *model, SurfaceMesh::Face picked_face, int gl_x, int gl_y);

        /**
         * Pick an edge from a surface mesh given the cursor position and a known picked face.
         * @param gl_x/gl_y The cursor position, relative to the bottom-left corner of the OpenGL viewport. Client code
         *                  should convert their viewer-dependent cursor position in the screen coordinate system to
         *                  the one following the OpenGL convention, i.e.,
         *                          int gl_x = x;
         *                          int gl_y = screen_height() - 1 - y;
         *                  Besides, client code is also responsible for handling high-DPI scaling, i.e.,
         *                          gl_x *= static_cast<int>(glx * 2);
         *                          gl_y *= static_cast<int>(gly * 2);
         * @return The picked halfedge.
         * @attention This method must be called after calling to pick_face(). The result is valid only if the
         *            picked_face is valid.
         */
        SurfaceMesh::Halfedge pick_edge(SurfaceMesh *model, SurfaceMesh::Face picked_face, int gl_x, int gl_y);

        //-------------------- query after picking ----------------------

        /**
         * Query the previously picked face.
         * @return The previously picked face.
         * @attention This method must be called after calling to one of the above pick element methods. The results is
         *            valid only if a face has been picked.
         */
        SurfaceMesh::Face picked_face() const;

        /**
         * Query the coordinate of the previously picked position.
         * @return The xyz coordinate of the picked position.
         * @attention This method must be called after calling to one of the above pick element methods. The results is
         *            valid only if a face has been picked.
         */
        vec3 picked_point() const;

    private:
        // selection implemented in GPU (using shader program)
        SurfaceMesh::Face pick_facet_gpu(SurfaceMesh *model, int gl_x, int gl_y);

        // selection implemented in CPU (with OpenMP if supported)
        SurfaceMesh::Face pick_facet_cpu(SurfaceMesh *model, int gl_x, int gl_y);

        Plane3 face_plane(SurfaceMesh *model, SurfaceMesh::Face face) const;

        /**
         * TODO: check if this works also for non-convex faces.
         */
        inline bool do_intersect(SurfaceMesh *model, SurfaceMesh::Face picked_face, const OrientedLine3 &line) const {
            // Uses Plucker coordinates (see OrientedLine)
            Sign face_sign = ZERO;
            for (auto h : model->halfedges(picked_face)) {
                auto s = model->from_vertex(h);
                auto t = model->to_vertex(h);
                const OrientedLine3 edge_line(model->position(t), model->position(s));
                Sign sign = OrientedLine3::side(line, edge_line);
                if (sign != ZERO) {
                    if (face_sign != ZERO && sign != face_sign)
                        return false;
                    face_sign = sign;
                }
            }
            return true;
        }

    private:
        unsigned int hit_resolution_;     // in pixels

        ShaderProgram *program_;

        vec3 picked_point_;
        SurfaceMesh::Face picked_face_;

    };

}


#endif  // EASY3D_GUI_PICKER_SURFACE_MESH_H
