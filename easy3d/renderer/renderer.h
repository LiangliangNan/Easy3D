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

#ifndef EASY3D_RENDERER_RENDERER_H
#define EASY3D_RENDERER_RENDERER_H


#include <string>

namespace easy3d {

    class Model;
    class Graph;
    class PointCloud;
    class SurfaceMesh;
    class Drawable;
    class PointsDrawable;
    class LinesDrawable;
    class TrianglesDrawable;

    namespace renderer {

        // -------------------------------------------------------------------------------------------------------------

        /**
         * @brief Set the default rendering state of the "vertices" drawable of a point cloud.
         * @details The default rendering state is determined by the availability of the vertex properties.
         *          The motivation is that the most appealing rendering is demonstrated by default. The following
         *          priority applies:
         *              1. per-vertex color: in "v:color";
         *              2. per-vertex texture coordinates: in "v:texcoord";
         *              3. segmentation: in "v:primitive_index";
         *              4: uniform color.
         */
        void set_default_rendering_state(PointCloud *model, PointsDrawable *drawable);

        /**
         * @brief Set the default rendering state of the "faces" drawable of a surface mesh.
         * @details The default rendering state is determined by the availability of the vertex/face properties.
         *          The motivation is that the most appealing rendering is demonstrated by default. The following
         *          priority applies:
         *              1: per-face color: in "f:color";
         *              2: per-vertex color: in "v:color";
         *              3. per-halfedge texture coordinates: in "h:texcoord";
         *              4. per-vertex texture coordinates: in "v:texcoord";
         *              5. segmentation: in "f:chart";
         *              6 uniform color
         */
        void set_default_rendering_state(SurfaceMesh *model, TrianglesDrawable *drawable);

        // -------------------------------------------------------------------------------------------------------------

        /**
         * @brief Creates= default drawable for a model.
         * @param model The model
         */
        void create_default_drawables(Model *model);

        // -------------------------------------------------------------------------------------------------------------

        /**
         * @brief Update render buffers of a drawable. Coloring determined by the drawable's coloring scheme.
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update_buffers(Model* model, Drawable* drawable);


        // PointCloud -------------------------------------------------------------------------------------------------

        /**
         * @brief Update render buffers for the default "vertices" drawable of a point cloud.
         * Coloring determined by the drawable's coloring scheme.
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update_buffers(PointCloud* model, PointsDrawable* drawable);

        /**
         * @brief Update render buffers for a vector field defined on a point cloud.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param field The name of the vector field.
         * @param scale The length scale of the vectors w.r.t. (0.01 * radius) of the model's bounding sphere.
         */
        void update_buffers_vector_field(PointCloud *model, LinesDrawable *drawable, const std::string& field, float scale);

        /**
         * @brief Generate a color property for visualizing segmentation. Each segment will be given a random color.
         * @param model     The model.
         * @param segmentation  The <int> type vertex property storing the segmentation.
         * @param color_name The name of the color property to be created.
         */
        void colorize_segmentation(PointCloud* model, const std::string& segmentation = "v:primitive_index", const std::string& color_name = "v:color-segments");


        // SurfaceMesh ------------------------------------------------------------------------------------------------

        /**
         * @brief Update render buffers for the default "vertices" drawable of a surface mesh.
         * Coloring determined by the drawable's coloring scheme.
         * @param model     The model.
         * @param drawable  The drawable.
         */
         void update_buffers(SurfaceMesh* model, PointsDrawable* drawable);

        /**
         * @brief Update render buffers for the default "edges" drawable of a surface mesh.
         * Coloring determined by the drawable's coloring scheme.
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update_buffers(SurfaceMesh* model, LinesDrawable* drawable);

        /**
         * @brief Update render buffers for the default "faces" drawable of a surface mesh.
         * Coloring determined by the drawable's coloring scheme.
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update_buffers(SurfaceMesh* model, TrianglesDrawable* drawable);

        /**
         * @brief Update render buffers for a vector field defined on a surface mesh.
         * @param model     The model.
         * @param drawable  The drawable.
         * @param field The name of the vector field.
         * @param location The location where the vector is defined.
         *         - 0: on faces;
         *         - 1: on vertices.
         * @param scale The scale of the vector length w.r.t. the average edge length of the surface mesh.
         */
        void update_buffers_vector_field(SurfaceMesh *model, LinesDrawable *drawable, const std::string& field, int location, float scale);


        // Graph ------------------------------------------------------------------------------------------------------

        /**
         * @brief Update render buffers for the default "vertices" drawable of a graph.
         * Coloring determined by the drawable's coloring scheme.
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update_buffers(Graph* model, PointsDrawable* drawable);


        /**
         * @brief Update render buffers for the default "edges" drawable of a graph.
         * Coloring determined by the drawable's coloring scheme.
         * @param model     The model.
         * @param drawable  The drawable.
         */
        void update_buffers(Graph* model, LinesDrawable* drawable);

    }

}


#endif  // EASY3D_RENDERER_RENDERER_H