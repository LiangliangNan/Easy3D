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

#ifndef EASY3D_GUI_PICKER_POINT_CLOUD_H
#define EASY3D_GUI_PICKER_POINT_CLOUD_H


#include <easy3d/gui/picker.h>
#include <easy3d/core/point_cloud.h>


namespace easy3d {

    namespace opengl { class PointSelector; }

    /**
     * \brief Implementation of picking points from a point cloud.
     * \class PointCloudPicker easy3d/gui/picker_point_cloud.h
     * \details Picking answers "which point is under the cursor?" or "which points
     *      fall inside this rectangle/lasso?" without walking the point cloud on
     *      the CPU.
     *
     *      A single pick rasterizes the visible point drawable into an offscreen
     *      buffer whose color encodes the source vertex, then reads back only the
     *      queried pixel. OpenGL draws point sprites with a dedicated selection
     *      shader and uses glReadPixels; Vulkan samples the same drawable through
     *      the backend RenderDevice.
     *
     *      A region pick tests every point in parallel on the GPU. OpenGL uses a
     *      vertex shader with transform feedback: each shader invocation owns one
     *      32-bit word of the packed selection mask, so no atomics are needed.
     *      Vulkan dispatches an equivalent compute shader. Positions are read
     *      straight from the rendering buffers in both backends; only the mask
     *      words travel back to the CPU.
     *
     *      Both paths respect the camera frustum, the model/drawable transform,
     *      clipping planes, indexed drawables, and point impostors, and they report
     *      original source vertices even when the drawable references them in a
     *      different order.
     *      Like all pickers, it acts on the supplied model and never traverses a
     *      Viewer's scene; use Viewer::pick() to identify the hit drawable first
     *      (see Picker for the division of work).
     * \note OpenGL picking requires a current OpenGL context. Single picks require
     *      a visible point drawable (the "vertices" drawable created automatically
     *      when a cloud is added to a viewer).
     * \see Picker, ModelPicker, SurfaceMeshPicker
     */
    class PointCloudPicker : public Picker {
    public:
        /**
         * \brief Constructor.
         * \param cam The camera used for picking.
         */
        explicit PointCloudPicker(const Camera *cam);
        /**
         * \brief Destructor.
         */
        ~PointCloudPicker() override;

        /**
         * \brief Pick the visible point at a screen location.
         * \param model The point cloud model.
         * \param x The x coordinate of the screen point.
         * \param y The y coordinate of the screen point.
         * \return The front-most point covering that pixel, or an invalid vertex if nothing is visible.
         * \details Hidden models/drawables, clipped points, and deleted source vertices are never returned.
         */
        PointCloud::Vertex pick_vertex(PointCloud *model, int x, int y);

        /**
         * \brief Pick vertices of a point cloud by a rectangle. The selected vertices will be marked in vertex
         *      property "v:select".
         * \param model The point cloud model.
         * \param rect The rectangle region.
         * \param deselect True to perform an inverse operation.
         * \details The region is evaluated on the GPU with through-selection: a
         *      point is selected when its projected center is inside the rectangle,
         *      in front of the camera, and not clipped. Occlusion by other scene
         *      geometry is not considered.
         */
        void pick_vertices(PointCloud *model, const Rect& rect, bool deselect);

        /**
         * \brief Pick vertices of a point cloud by a polygon/lasso. The selected vertices will be marked in
         *      vertex property "v:select".
         * \param model The point cloud model.
         * \param plg The polygon region.
         * \param deselect True to perform an inverse operation.
         * \details Same GPU through-selection as the rectangle overload. The polygon
         *      may be concave; it is given in screen coordinates with the origin at
         *      the upper-left corner.
         */
        void pick_vertices(PointCloud *model, const Polygon2 &plg, bool deselect);

    private:
#ifndef EASY3D_HAS_VULKAN
        // Region selection does not need a Viewer; the picker owns its GPU selector.
        std::unique_ptr<opengl::PointSelector> point_selector_;
#endif
    };

}


#endif // EASY3D_GUI_PICKER_POINT_CLOUD_H
