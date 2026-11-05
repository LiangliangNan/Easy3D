
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

    class TrianglesDrawable;

    /**
     * \brief Implementation of picking elements (i.e, vertices, faces, edges) from a surface mesh.
     * \class SurfaceMeshPicker easy3d/gui/picker_surface_mesh.h
     * \details Coordinates follow Camera::projectedCoordinatesOf(). Callers convert
     *      window coordinates to the camera's screen size.
     *      Single-face picks rasterize the visible face drawable on both backends,
     *      including active clipping planes, and map the generated triangle back to
     *      its original face. A model without a rendered face drawable cannot be
     *      picked. Vertex and edge selection then snaps within the picked face using
     *      the picker resolution. Region selection remains geometric
     *      through-selection: all original face vertices must be inside the region,
     *      camera frustum and clipping planes.
     * \see Picker, ModelPicker, PointCloudPicker
     */
    class SurfaceMeshPicker : public Picker {
    public:
        /**
         * \brief Constructor.
         * \param cam The camera used for picking.
         */
        explicit SurfaceMeshPicker(const Camera *cam);
        /**
         * \brief Destructor.
         */
        ~SurfaceMeshPicker() override = default;

        //------------------ sensitivity  ---------------------

        /**
         * \brief Returns the picker resolution (in pixels).
         * \details The picker resolution indicates the sensitivity for picking a vertex or an edge.
         * \return The picker resolution in pixels.
         */
        unsigned int resolution() const { return hit_resolution_; }
        /**
         * \brief Sets the picker resolution (in pixels).
         * \details The picker resolution indicates the sensitivity for picking a vertex or an edge.
         * \param r The new picker resolution in pixels.
         */
        void set_resolution(unsigned int r) { hit_resolution_ = r; }

        //--------------- pick a single element ---------------

        /**
         * \brief Pick a face from a surface mesh given the cursor position.
         * \param model The surface mesh model.
         * \param x The cursor x-coordinate, relative to the left edge of the content area.
         * \param y The cursor y-coordinate, relative to the top edge of the content area.
         * \attention The screen point is expressed in the screen coordinate system with an origin in the upper left
         *            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with
         *            a Retina display.
         * \return The picked face.
         */
        SurfaceMesh::Face pick_face(SurfaceMesh *model, int x, int y);

        /**
         * \brief Pick a vertex from a surface mesh given the cursor position.
         * \param model The surface mesh model.
         * \param x The cursor x-coordinate, relative to the left edge of the content area.
         * \param y The cursor y-coordinate, relative to the top edge of the content area.
         * \attention The screen point is expressed in the screen coordinate system with an origin in the upper left
         *            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with
         *            a Retina display.
         * \return The picked vertex.
         */
        SurfaceMesh::Vertex pick_vertex(SurfaceMesh *model, int x, int y);

        /**
         * \brief Pick an edge from a surface mesh given the cursor position.
         * \param model The surface mesh model.
         * \param x The cursor x-coordinate, relative to the left edge of the content area.
         * \param y The cursor y-coordinate, relative to the top edge of the content area.
         * \attention The screen point is expressed in the screen coordinate system with an origin in the upper left
         *            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with
         *            a Retina display.
         * \return The picked halfedge.
         */
        SurfaceMesh::Halfedge pick_edge(SurfaceMesh *model, int x, int y);

        /**
         * \brief Pick a vertex from a surface mesh given the cursor position and a known picked face.
         * \param model The surface mesh model.
         * \param picked_face The picked face.
         * \param x The cursor x-coordinate, relative to the left edge of the content area.
         * \param y The cursor y-coordinate, relative to the top edge of the content area.
         * \attention The screen point is expressed in the screen coordinate system with an origin in the upper left
         *            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with
         *            a Retina display.
         * \return The picked vertex.
         * \attention This method must be called after calling to pick_face(). The result is valid only if the
         *            picked_face is valid.
         */
        SurfaceMesh::Vertex pick_vertex(SurfaceMesh *model, SurfaceMesh::Face picked_face, int x, int y);

        /**
         * \brief Pick an edge from a surface mesh given the cursor position and a known picked face.
         * \param model The surface mesh model.
         * \param picked_face The picked face.
         * \param x The cursor x-coordinate, relative to the left edge of the content area.
         * \param y The cursor y-coordinate, relative to the top edge of the content area.
         * \attention The screen point is expressed in the screen coordinate system with an origin in the upper left
         *            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with
         *            a Retina display.
         * \return The picked halfedge.
         * \attention This method must be called after calling to pick_face(). The result is valid only if the
         *            picked_face is valid.
         */
        SurfaceMesh::Halfedge pick_edge(SurfaceMesh *model, SurfaceMesh::Face picked_face, int x, int y);

        //-------------------- query after picking ----------------------

        /**
         * \brief Query the previously picked face.
         * \return The previously picked face.
         * \attention This method must be called after calling to one of the above pick element methods. The result is
         *            valid only if a face has been picked.
         */
        SurfaceMesh::Face picked_face() const;

        /**
         * \brief Query the visible surface position of the previously picked face.
         * \param model The surface mesh model.
         * \param picked_face The picked face.
         * \param x The cursor x-coordinate, relative to the left edge of the content area.
         * \param y The cursor y-coordinate, relative to the top edge of the content area.
         * \return The world-space surface point sampled during the last pick, or zero if invalid.
         * \attention This method must be called after calling to pick_face(). The result is valid only if the
         *            picked_face is valid. The sampled point corresponds to the pixel of that pick.
         */
        vec3 picked_point(SurfaceMesh *model, SurfaceMesh::Face picked_face, int x, int y) const;


        //------------------ multiple selection of faces ------------------

        /**
         * \brief Pick faces of a surface mesh by a rectangle.
         * \param model The surface mesh model.
         * \param rect The rectangle region.
         * \return The faces selected during this operation (regardless of their previous status).
         */
        std::vector<SurfaceMesh::Face> pick_faces(SurfaceMesh *model, const Rect &rect);

        /**
         * \brief Pick faces of a surface mesh by a polygon/lasso.
         * \param model The surface mesh model.
         * \param plg The polygon region.
         * \return The faces selected during this operation (regardless of their previous status).
         */
        std::vector<SurfaceMesh::Face> pick_faces(SurfaceMesh *model, const Polygon2 &plg);

    private:
        // Rasterized face ID/depth pass. The drawable is the visible face drawable.
        SurfaceMesh::Face pick_face_gpu(SurfaceMesh *model, const std::shared_ptr<TrianglesDrawable>& drawable,
                                        int x, int y);

    private:
        unsigned int hit_resolution_;     // in pixels
        SurfaceMesh::Face picked_face_;
        SurfaceMesh* picked_model_ = nullptr; // borrowed, same lifetime contract as method arguments
        vec3 picked_point_ = vec3(0);         // visible surface point of the last pick
        int picked_x_ = -1, picked_y_ = -1;   // screen position of the last pick
    };

}



#endif  // EASY3D_GUI_PICKER_SURFACE_MESH_H
