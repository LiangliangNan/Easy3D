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

#ifndef EASY3D_GUI_PICKER_POINT_CLOUD_H
#define EASY3D_GUI_PICKER_POINT_CLOUD_H


#include <easy3d/gui/picker.h>


namespace easy3d {

    class PointCloud;

    class ShaderProgram;

    class PointCloudPicker : public Picker {
    public:
        PointCloudPicker(Camera *cam);

        /**
         * @brief Pick vertices of a point cloud by a rectangle. The selected vertices will be marked in vertex property
         * "v:select".
         * @param min_x The min_x component of the rectangle.
         * @param max_x The max_x component of the rectangle.
         * @param min_y The min_y component of the rectangle.
         * @param max_y The max_y component of the rectangle.
         * @param deselect True to perform an inverse operation.
         * @return The number of vertices selected during this operation (despite their previous status).
         */
        int pick_vertices(PointCloud *model, int min_x, int max_x, int min_y, int max_y, bool deselect);

        /**
         * @brief Pick vertices of a point cloud by a polygon/lasso. The selected vertices will be marked in vertex
         * property "v:select".
         * @param plg The polygon stored as a 1D array.
         * @param deselect True to perform an inverse operation.
         * @return The number of vertices selected during this operation (despite their previous status).
         */
        int pick_vertices(PointCloud *model, const std::vector<vec2> &plg, bool deselect);

    private:
        int pick_vertices_cpu(PointCloud *model, int min_x, int max_x, int min_y, int max_y, bool deselect);

        int pick_vertices_cpu(PointCloud *model, const std::vector<vec2> &plg, bool deselect);

        int pick_vertices_gpu(PointCloud *model, int min_x, int max_x, int min_y, int max_y, bool deselect,
                              ShaderProgram *program);

        int pick_vertices_gpu(PointCloud *model, const std::vector<vec2> &plg, bool deselect, ShaderProgram *program);
    };

}

#endif // EASY3D_GUI_PICKER_POINT_CLOUD_H
