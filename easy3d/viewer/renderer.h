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


#include <easy3d/core/point_cloud.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    class PointsDrawable;
    class LinesDrawable;
    class TrianglesDrawable;

    namespace renderer {

        /**
         * \brief Template functions preparing and updating *standard* drawable data to GPU.
         * \param model     The model.
         * \param drawable  The drawable.
         * \attention These functions are for *standard* drawables. You may need to implement your own function
         *            for a particular rendering purpose.
         */


        // for point clouds
        void update_data(PointCloud* model, PointsDrawable* drawable);

        // for meshes
        void update_data(SurfaceMesh* model, PointsDrawable* drawable);
        void update_data(SurfaceMesh* model, LinesDrawable* drawable);
        void update_data(SurfaceMesh* model, TrianglesDrawable* drawable);

        // for graphs
        void update_data(Graph* model, PointsDrawable* drawable);
        void update_data(Graph* model, LinesDrawable* drawable);


        // a template version
        template <typename DRAWABLE>
        inline void update_data(Model* model, DRAWABLE* drawable) {
            if (dynamic_cast<PointCloud*>(model))
                update_data(dynamic_cast<PointCloud*>(model), drawable);
            else if (dynamic_cast<Graph*>(model))
                update_data(dynamic_cast<Graph*>(model), drawable);
            else if (dynamic_cast<SurfaceMesh*>(model))
                update_data(dynamic_cast<SurfaceMesh*>(model), drawable);
        }
    }

}