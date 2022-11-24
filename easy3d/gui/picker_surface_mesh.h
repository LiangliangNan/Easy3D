
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

#ifndef EASY3D_GUI_PICKER_SURFACE_MESH_H
#define EASY3D_GUI_PICKER_SURFACE_MESH_H

#include <easy3d/gui/picker.h>
#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    class ShaderProgram;

    /**
     * \brief Implementation of picking elements (i.e, vertices, faces, edges) from a surface mesh.
     * \class SurfaceMeshPicker easy3d/gui/picker_surface_mesh.h
     * \see Picker, ModelPicker, PointCloudPicker
     */
    class SurfaceMeshPicker : public Picker {
    public:
        explicit SurfaceMeshPicker(const Camera *cam);

        ~SurfaceMeshPicker() override = default;

        //------------------ sensitivity  ---------------------

        /// \brief Returns the picker resolution (in pixels).
        /// \details The picker resolution indicates the sensitivity for picking a vertex or an edge.
        unsigned int resolution() const { return hit_resolution_; }
        /// \brief Sets the picker resolution (in pixels).
        /// \details The picker resolution indicates the sensitivity for picking a vertex or an edge.
        void set_resolution(unsigned int r) { hit_resolution_ = r; }

        //--------------- pick a single element ---------------

        /**
         * Pick a face from a surface mesh given the cursor position.
         * @param x The cursor x-coordinate, relative to the left edge of the content area.
         * @param y The cursor y-coordinate, relative to the top edge of the content area.
         * @attention The screen point is expressed in the screen coordinate system with an origin in the upper left
         *            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with
         *            a Retina display.
         * @return The picked face.
         */
        SurfaceMesh::Face pick_face(SurfaceMesh *model, int x, int y);

        /**
         * \brief Pick a vertex from a surface mesh given the cursor position.
         * @param x The cursor x-coordinate, relative to the left edge of the content area.
         * @param y The cursor y-coordinate, relative to the top edge of the content area.
         * @attention The screen point is expressed in the screen coordinate system with an origin in the upper left
         *            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with
         *            a Retina display.
         * @return The picked vertex.
         */
        SurfaceMesh::Vertex pick_vertex(SurfaceMesh *model, int x, int y);

        /**
         * \brief Pick an edge from a surface mesh given the cursor position.
         * @param x The cursor x-coordinate, relative to the left edge of the content area.
         * @param y The cursor y-coordinate, relative to the top edge of the content area.
         * @attention The screen point is expressed in the screen coordinate system with an origin in the upper left
         *            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with
         *            a Retina display.
         * @return The picked halfedge.
         */
        SurfaceMesh::Halfedge pick_edge(SurfaceMesh *model, int x, int y);

        /**
         * \brief Pick a vertex from a surface mesh given the cursor position and a known picked face.
         * @param x The cursor x-coordinate, relative to the left edge of the content area.
         * @param y The cursor y-coordinate, relative to the top edge of the content area.
         * @param picked_face The picked face.
         * @attention The screen point is expressed in the screen coordinate system with an origin in the upper left
         *            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with
         *            a Retina display.
         * @return The picked vertex.
         * @attention This method must be called after calling to pick_face(). The result is valid only if the
         *            picked_face is valid.
         */
        SurfaceMesh::Vertex pick_vertex(SurfaceMesh *model, SurfaceMesh::Face picked_face, int x, int y);

        /**
         * \brief Pick an edge from a surface mesh given the cursor position and a known picked face.
         * @param x The cursor x-coordinate, relative to the left edge of the content area.
         * @param y The cursor y-coordinate, relative to the top edge of the content area.
         * @param picked_face The picked face.
         * @attention The screen point is expressed in the screen coordinate system with an origin in the upper left
         *            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with
         *            a Retina display.
         * @return The picked halfedge.
         * @attention This method must be called after calling to pick_face(). The result is valid only if the
         *            picked_face is valid.
         */
        SurfaceMesh::Halfedge pick_edge(SurfaceMesh *model, SurfaceMesh::Face picked_face, int x, int y);

        //-------------------- query after picking ----------------------

        /**
         * \brief Query the previously picked face.
         * @return The previously picked face.
         * @attention This method must be called after calling to one of the above pick element methods. The results is
         *            valid only if a face has been picked.
         */
        SurfaceMesh::Face picked_face() const;

        /**
         * \brief Query the coordinate of the previously picked position, which is the intersection between the picking
         * line and the picked face.
         * @param x The cursor x-coordinate, relative to the left edge of the content area.
         * @param y The cursor y-coordinate, relative to the top edge of the content area.
         * @param picked_face The picked face.
         * @return The xyz coordinate of the picked position.
         * @attention This method must be called after calling to pick_face(). The result is valid only if the
         *            picked_face is valid.
         */
        vec3 picked_point(SurfaceMesh *model, SurfaceMesh::Face picked_face, int x, int y) const;


        //------------------ multiple selection of faces ------------------

        /**
         * @brief Pick faces of a surface mesh by a rectangle.
         * @param rect The rectangle region.
         * @return The faces selected during this operation (regardless of their previous status).
         */
        std::vector<SurfaceMesh::Face> pick_faces(SurfaceMesh *model, const Rect &rect);

        /**
         * @brief Pick faces of a surface mesh by a polygon/lasso.
         * @param plg The polygon region.
         * @return The faces selected during this operation (regardless of their previous status).
         */
        std::vector<SurfaceMesh::Face> pick_faces(SurfaceMesh *model, const Polygon2 &plg);

    private:
        // selection implemented in GPU (using shader program)
        SurfaceMesh::Face pick_face_gpu(SurfaceMesh *model, int x, int y, ShaderProgram* program);

        // selection implemented in CPU (with OpenMP if supported)
        SurfaceMesh::Face pick_face_cpu(SurfaceMesh *model, int x, int y);

        Plane3 face_plane(SurfaceMesh *model, SurfaceMesh::Face face) const;

        /**
         * TODO: check if this works also for non-convex faces.
         */
        static inline bool do_intersect(SurfaceMesh *model, SurfaceMesh::Face picked_face, const OrientedLine3 &line) {
            // Uses Plucker coordinates (see OrientedLine)
            Sign face_sign = ZERO;
            for (auto h : model->halfedges(picked_face)) {
                auto s = model->source(h);
                auto t = model->target(h);
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
        SurfaceMesh::Face picked_face_;
    };

}


#endif  // EASY3D_GUI_PICKER_SURFACE_MESH_H
