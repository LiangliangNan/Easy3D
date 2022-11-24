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

    class ShaderProgram;

    /**
     * \brief Implementation of picking points from a point cloud.
     * \class PointCloudPicker easy3d/gui/picker_point_cloud.h
     * \see Picker, ModelPicker, SurfaceMeshPicker
     */
    class PointCloudPicker : public Picker {
    public:
        explicit PointCloudPicker(const Camera *cam);

        /// \brief Returns the picker resolution (in pixels).
        /// \details The picker resolution indicates the sensitivity of picking a point. It is used only for
        ///     the CPU implementation of picking a single point.
        unsigned int resolution() const { return hit_resolution_; }
        /// \brief Sets the picker resolution (in pixels).
        /// \details The picker resolution indicates the sensitivity of picking a point. It is used only for
        ///     the CPU implementation of picking a single point.
        void set_resolution(unsigned int r) { hit_resolution_ = r; }

        /**
         * @brief Pick vertex at a given screen location.
         * @param (x, y) The screen point.
         * @param deselect True to perform an inverse operation.
         * @return The picked vertex.
         */
        PointCloud::Vertex pick_vertex(PointCloud *model, int x, int y);

        /**
         * @brief Pick vertices of a point cloud by a rectangle. The selected vertices will be marked in vertex property
         * "v:select".
         * @param rect The rectangle region.
         * @param deselect True to perform an inverse operation.
         */
        void pick_vertices(PointCloud *model, const Rect& rect, bool deselect);

        /**
         * @brief Pick vertices of a point cloud by a polygon/lasso. The selected vertices will be marked in vertex
         * property "v:select".
         * @param plg The polygon region.
         * @param deselect True to perform an inverse operation.
         */
        void pick_vertices(PointCloud *model, const Polygon2 &plg, bool deselect);

    private:
        // pick implemented in CPU (with OpenMP if supported)
        PointCloud::Vertex pick_vertex_cpu(PointCloud *model, int x, int y);

        // pick plain points implemented in GPU (using shader program)
        PointCloud::Vertex pick_vertex_gpu_plain(PointCloud *model, int x, int y);

        // pick sphere points implemented in GPU (using shader program)
        PointCloud::Vertex pick_vertex_gpu_sphere(PointCloud *model, int x, int y);

    private:
        unsigned int hit_resolution_;     // in pixels
        ShaderProgram*	 program_;
    };

}

#endif // EASY3D_GUI_PICKER_POINT_CLOUD_H
