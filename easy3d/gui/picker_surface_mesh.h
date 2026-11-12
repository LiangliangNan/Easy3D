
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
     * \details Coordinates use the camera's logical screen extent; on HiDPI displays
     *      this differs from framebuffer pixels.
     *      A single-face pick renders the visible face drawable into a private
     *      primitive-ID/depth target, including active clipping planes, then decodes
     *      the cursor pixel and maps the triangle index back to its original face.
     *      A model without a rendered face drawable cannot be picked. Vertex and
     *      edge selection then snaps within the picked face using the picker
     *      resolution. Region selection remains geometric through-selection: all
     *      original face vertices must be inside the region, camera frustum and
     *      clipping planes. Vulkan face picking also requires CPU-side drawable
     *      buffers so the rasterized triangle can be mapped back to a source face.
     *      Like all pickers, it acts on the supplied model and never traverses a
     *      Viewer's scene; use Viewer::pick() to identify the hit drawable first
     *      (see Picker for the division of work).
     * \see Picker, ModelPicker, PointCloudPicker
     */
    class SurfaceMeshPicker : public Picker {
    public:
        /**
         * \brief Constructor.
         * \param cam Non-null camera used for picking; it is borrowed and must outlive the picker.
         */
        explicit SurfaceMeshPicker(const Camera *cam);
        /**
         * \brief Destructor.
         */
        ~SurfaceMeshPicker() override = default;

        //------------------ sensitivity  ---------------------

        /**
         * \brief Returns the vertex/edge hit tolerance in logical screen units.
         * \details The tolerance is a radius around the cursor, measured in the same logical units as pick coordinates.
         * \return The hit tolerance.
         */
        unsigned int resolution() const { return hit_resolution_; }
        /**
         * \brief Set the vertex/edge hit tolerance in logical screen units.
         * \details The tolerance is a radius around the cursor, measured in the same logical units as pick coordinates.
         * \param r New hit tolerance.
         */
        void set_resolution(unsigned int r) { hit_resolution_ = r; }

        //--------------- pick a single element ---------------

        /**
         * \brief Pick a face from a surface mesh given the cursor position.
         * \param model Surface mesh with a visible face drawable.
         * \param x The cursor x-coordinate in logical window units (top-left origin, x right).
         * \param y The cursor y-coordinate in logical window units (top-left origin, y down).
         * \return The picked face, or an invalid face if no face is under the cursor.
         * \note The coordinate is in logical window units (camera()->screen_width()/Height()), not framebuffer pixels.
         *       The method renders the visible face drawable with primitive IDs into an offscreen target, samples
         *       the pixel at (x, y), and maps the primitive ID back to the mesh face. Both OpenGL and Vulkan
         *       backends present the same coordinate contract.
         */
        SurfaceMesh::Face pick_face(SurfaceMesh *model, int x, int y);

        /**
         * \brief Pick a vertex from a surface mesh given the cursor position.
         * \param model The surface mesh model.
         * \param x The cursor x-coordinate in logical window units (top-left origin, x right).
         * \param y The cursor y-coordinate in logical window units (top-left origin, y down).
         * \return The picked vertex, or an invalid vertex if no vertex is within the hit resolution.
         * \note The method first picks the face under the cursor, then searches for the closest vertex of that
         *       face within the hit tolerance.
         */
        SurfaceMesh::Vertex pick_vertex(SurfaceMesh *model, int x, int y);

        /**
         * \brief Pick an edge from a surface mesh given the cursor position.
         * \param model The surface mesh model.
         * \param x The cursor x-coordinate in logical window units (top-left origin, x right).
         * \param y The cursor y-coordinate in logical window units (top-left origin, y down).
         * \return The picked halfedge, or an invalid halfedge if no edge is within the hit resolution.
         * \note The method first picks the face under the cursor, then searches for the closest edge of that
         *       face within the hit tolerance.
         */
        SurfaceMesh::Halfedge pick_edge(SurfaceMesh *model, int x, int y);

        /**
         * \brief Pick a vertex from a surface mesh given the cursor position and a known picked face.
         * \param model The surface mesh model.
         * \param picked_face The previously picked face (must be valid).
         * \param x The cursor x-coordinate in logical window units (top-left origin, x right).
         * \param y The cursor y-coordinate in logical window units (top-left origin, y down).
         * \return The picked vertex, or an invalid vertex if no vertex is within the hit resolution.
         * \note Skips face rasterization. \p picked_face and \p model must match the face/model from the most recent
         *       successful pick_face() call; otherwise an invalid vertex is returned.
         */
        SurfaceMesh::Vertex pick_vertex(SurfaceMesh *model, SurfaceMesh::Face picked_face, int x, int y);

        /**
         * \brief Pick an edge from a surface mesh given the cursor position and a known picked face.
         * \param model The surface mesh model.
         * \param picked_face The previously picked face (must be valid).
         * \param x The cursor x-coordinate in logical window units (top-left origin, x right).
         * \param y The cursor y-coordinate in logical window units (top-left origin, y down).
         * \return The picked halfedge, or an invalid halfedge if no edge is within the hit resolution.
         * \note Skips face rasterization. \p picked_face and \p model must match the face/model from the most recent
         *       successful pick_face() call; otherwise an invalid halfedge is returned.
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
         * \param x The cursor x-coordinate in logical window units (must match the last pick).
         * \param y The cursor y-coordinate in logical window units (must match the last pick).
         * \return The world-space surface point sampled during the last pick, or zero if invalid.
         * \note This method must be called after calling pick_face(). The result is valid only if the
         *       picked_face is valid and the coordinates match the last pick.
         */
        vec3 picked_point(SurfaceMesh *model, SurfaceMesh::Face picked_face, int x, int y) const;


        //------------------ multiple selection of faces ------------------

        /**
         * \brief Return faces of a surface mesh fully inside a rectangle.
         * \param model The surface mesh model.
         * \param rect Rectangle in logical window coordinates (top-left origin).
         * \return Faces whose vertices satisfy the region tests. This does not modify mesh selection properties.
         * \note The rectangle is evaluated with through-selection: a face is selected when all its vertices
         *       project inside the rectangle, are in front of the camera, and are not clipped. The coordinates
         *       are in logical window units (camera()->screen_width()/Height()), not framebuffer pixels. Both
         *       OpenGL and Vulkan backends present the same coordinate contract.
         */
        std::vector<SurfaceMesh::Face> pick_faces(SurfaceMesh *model, const Rect &rect);

        /**
         * \brief Return faces of a surface mesh fully inside a polygon/lasso.
         * \param model The surface mesh model.
         * \param plg Polygon region in logical window coordinates (top-left origin).
         * \return Faces whose vertices satisfy the region tests. This does not modify mesh selection properties.
         * \note The polygon is evaluated with through-selection, same as the rectangle overload. The polygon
         *       may be concave; it is given in logical window units (camera()->screen_width()/Height()), not
         *       framebuffer pixels. A polygon with fewer than three points or zero area returns no faces.
         *       This operation does not modify mesh selection properties.
         */
        std::vector<SurfaceMesh::Face> pick_faces(SurfaceMesh *model, const Polygon2 &plg);

    private:
        // Rasterized face-ID/depth pass: renders the visible face drawable with a
        // primitive ID in the color target, samples the cursor pixel, and maps the
        // primitive back to the mesh face. Backend-specific offscreen machinery is
        // in picker_surface_mesh.cpp.
        SurfaceMesh::Face pick_face_impl(SurfaceMesh *model, const std::shared_ptr<TrianglesDrawable>& drawable,
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
