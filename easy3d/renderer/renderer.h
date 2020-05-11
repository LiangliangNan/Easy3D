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
#include <vector>

#include <easy3d/core/types.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    class Model;
    class Graph;
    class PointCloud;
    class SurfaceMesh;
    class PointsDrawable;
    class LinesDrawable;
    class TrianglesDrawable;

    /**
     * Model is the base class of renderable 3D models, e.g., SurfaceMesh, PointCloud, Graph.
     * A Model can have multiple drawables, e.g., faces, vertices, edges.
     */
    class Renderer
    {
    public:
        /// create == true: create default drawables. See create_default_drawables(...).
        Renderer(Model* model, bool create = true);
        virtual ~Renderer();

        Model* model() { return model_; }

        //-------------------- rendering functionalities  -----------------------

        bool is_visible() const { return visible_; }
        void set_visible(bool b) { visible_ = b; }

        bool is_selected() const { return selected_; }
        void set_selected(bool b) { selected_ = b; }

        /**
         * @brief Sets the model modified after processing (e.g., remeshing, denoising). This ensure the OpenGL buffers
         *        are up-to-date before rendering.
         * @details All drawables associated with this model will be updated. This is equivalent to call the
         *          update_buffers() function for all the drawables of this model. To achieve better performance (for
         *          huge models), it is wiser to update only the affected drawables and buffers. For example, the change
         *          in texture coordinates of a surface mesh doesn't change the rendering of its "edges" and "vertices".
         *          It is not necessary to update "edges" and "vertices". Besides, the vertex buffer and the index
         *          buffer of the "faces" are also not affected and can avoid update. So, you only need to update the
         *          texcoord buffer of the "faces".
         */
        void update();

        //-------------------- drawable management  -----------------------

        // Gets the drawable named 'name'.
        // Returns a nullptr if the drawable does not exist.
        PointsDrawable* get_points_drawable(const std::string& name) const;
        LinesDrawable*  get_lines_drawable(const std::string& name) const;
        TrianglesDrawable*  get_triangles_drawable(const std::string& name) const;

        /**
         * Adds a drawable to this model.
         * @param name The name of the drawable to assign.
         * @return The created drawable. If a drawable with 'name' already exists, the creation will be ignored and the
         *         existing drawable is returned.
         */
        PointsDrawable* add_points_drawable(const std::string& name);
        LinesDrawable*  add_lines_drawable(const std::string& name);
        TrianglesDrawable*  add_triangles_drawable(const std::string& name);

        // Returns all available drawables.
        const std::vector<PointsDrawable*>&  points_drawables() const { return points_drawables_; }
        const std::vector<LinesDrawable*>&   lines_drawables() const { return lines_drawables_; }
        const std::vector<TrianglesDrawable*>& triangles_drawables() const { return triangles_drawables_; }

    public:
        /**
         * @brief Create default drawables for rendering.
         * @details This method creates defaults drawables for rendering a model. The supported
         *          default drawables are
         *              - for point clouds: "vertices".
         *              - for surface meshes: "faces", "vertices", "edges", "borders".
         *              - for graphs: "vertices", "edges".
         *          These drawables are usually sufficient for basic rendering of the model. In case
         *          the default drawables don't meet the particular visualization purpose, you can
         *          override this function or set 'create_default_drawables' to false and create the
         *          drawables by calling Model::add_[type]_drawable().
         */
        static void create_default_drawables(Model *model);

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
        static void set_default_rendering_state(PointCloud *model, PointsDrawable *drawable);

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
        static void set_default_rendering_state(SurfaceMesh *model, TrianglesDrawable *drawable);

        /**
         * @brief Generate a color property for visualizing segmentation. Each segment will be given a random color.
         * @param model  The model.
         * @param segments  The <int> type vertex property storing the segmentation.
         * @param color_name The name of the color property to be created.
         */
        static void color_from_segmentation(PointCloud* model, PointCloud::VertexProperty<int> segments, const std::string& color_name = "v:segments");


    protected:
        Model*  model_;

        bool	visible_;
        bool    selected_;

        std::vector<PointsDrawable*>    points_drawables_;
        std::vector<LinesDrawable*>     lines_drawables_;
        std::vector<TrianglesDrawable*> triangles_drawables_;
    };
}

#endif  // EASY3D_RENDERER_RENDERER_H
