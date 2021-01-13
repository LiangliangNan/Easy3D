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
    class PointsDrawable;
    class LinesDrawable;
    class TrianglesDrawable;

    /**
     * \brief A Renderer manages the drawables (and thus the rendering) of a model.
     * \details Renderer also allow to create default drawables for most rendering purposes.
     * \class Renderer easy3d/renderer/renderer.h
     * \sa Drawable, PointsDrawable, LinesDrawable, TrianglesDrawable
     */
    class Renderer {
    public:
        /**
         * @brief Constructor.
         * @param model The model to which this renderer is attached.
         * @param create_drawables \c true to create default drawables for this model. Skip the creation if \c false.
         *        The supported default drawables are
         *              - PointCloud: "vertices".
         *              - SurfaceMesh: "faces", "vertices", "edges", and "borders".
         *              - Graph: "vertices", and "edges".
         *              - PolyMesh: "faces:border" and "faces:interior".
         * After the \c model has been changed or modified (in geometry, texture, color, etc.), client code
         * can call update() for the rendering buffers of its default drawable to be automatically updated
         * during the rendering stage.
         * In case the default drawables don't meet a particular visualization purpose, client code should skip
         * the creation of the default drawables (by passing \c fase to \p create_drawables). Instead, create a
         * customized drawable and update the buffers accordingly. If the drawable changes or will be modified
         * frequently and its visualization is expected to be automatically updated after each change, provide
         * an update function.
         * \sa update(),
         */
        Renderer(Model *model, bool create_drawables = true);

        virtual ~Renderer();

        /**
         * @brief The model to which this renderer is attached.
         */
        Model *model() { return model_; }

        //-------------------- rendering functionalities  -----------------------

        /// Returns whether the model is currently visible.
        bool is_visible() const { return visible_; }
        /// Shows/Hides the model.
        void set_visible(bool b) { visible_ = b; }

        /// Returns whether the model has been selected.
        bool is_selected() const { return selected_; }
        /// Select/Deselect the model. The state of all its drawables will change accordingly.
        void set_selected(bool b);

        /**
         * @brief Invalidates the rendering buffers of the model and thus updates the rendering (delayed in rendering).
         * @details This method triggers an update of the rendering buffers of all the drawables of the model to which
         *      this renderer is attached. The effect is equivalent to calling Drawable::update() functions for all
         *      the drawables of this model.
         * @param update_bbox \c true to recompute the bounding box of the model.
         * @Note: To achieve better performance, it is wiser to update only the affected drawables and buffers.
         * \sa  Drawable::update()
         */
        void update(bool update_bbox = true);

        //-------------------- drawable management  -----------------------

        /**
         * Get the points drawable with a given name.
         * Return nullptr if the drawable does not exist.
         */
        PointsDrawable *get_points_drawable(const std::string &name) const;

        /**
         * Get the lines drawable with a given name.
         * Return nullptr if the drawable does not exist.
         */
        LinesDrawable *get_lines_drawable(const std::string &name) const;

        /**
         * Get the triangles drawable with a given name.
         * Return nullptr if the drawable does not exist.
         */
        TrianglesDrawable *get_triangles_drawable(const std::string &name) const;

        /**
         * Create a new points drawable and add it to this renderer.
         * @param name The name of the points drawable to be created.
         * @return The created points drawable. If a points drawable with 'name' already exists, the creation will be
         *         ignored and the existing drawable is returned.
         */
        PointsDrawable *add_points_drawable(const std::string &name);

        /**
         * Create a new lines drawable and add it to this renderer.
         * @param name The name of the lines drawable to be created.
         * @return The created lines drawable. If a lines drawable with 'name' already exists, the creation will be
         *         ignored and the existing drawable is returned.
         */
        LinesDrawable *add_lines_drawable(const std::string &name);

        /**
         * Create a new triangles drawable and add it to this renderer.
         * @param name The name of the triangles drawable to be created.
         * @return The created triangles drawable. If a triangles drawable with 'name' already exists, the creation will
         *         be ignored and the existing drawable is returned.
         */
        TrianglesDrawable *add_triangles_drawable(const std::string &name);

        /**
         * All available points drawables managed by this renderer.
         */
        const std::vector<PointsDrawable *> &points_drawables() const { return points_drawables_; }

        /**
         * All available lines drawables managed by this renderer.
         */
        const std::vector<LinesDrawable *> &lines_drawables() const { return lines_drawables_; }

        /**
         * All available triangles drawables managed by this renderer.
         */
        const std::vector<TrianglesDrawable *> &triangles_drawables() const { return triangles_drawables_; }

    public:
        /**
         * @brief Create default drawables for rendering.
         * @details This method creates defaults drawables for rendering a model. The supported default drawables are
         *              - PointCloud: "vertices".
         *              - SurfaceMesh: "faces", "vertices", "edges", "borders".
         *              - Graph: "vertices", "edges".
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
         *              4. scalar field;
         *              5: uniform color.
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
         *              6. scalar field;
         *              7 uniform color
         */
        static void set_default_rendering_state(SurfaceMesh *model, TrianglesDrawable *drawable);

        // -------------------------------------------------------------------------------------------------------------

        /**
         * @brief Generates random colors for visualizing face-based segmentation of a SurfaceMesh.
         * @param mesh  The surface mesh.
         * @param segments The face property storing the segmentation of the surface mesh.
         * @param colors The face property to store the colors. Must be allocated before hand.
         */
        static void color_from_segmentation(
                SurfaceMesh *mesh,
                const SurfaceMesh::FaceProperty<int> segments,
                SurfaceMesh::FaceProperty<vec3> colors
        );

        /**
         * @brief Generates random colors for visualizing the segmentation of a PointCloud.
         * @param cloud  The point cloud.
         * @param segments  The vertex property storing the segmentation of the point cloud.
         * @param colors The vertex property to store the colors. Must be allocated before hand.
         */
        static void color_from_segmentation(
                PointCloud *cloud,
                const PointCloud::VertexProperty<int> segments,
                PointCloud::VertexProperty<vec3> colors
        );

    protected:
        Model *model_;

        bool visible_;
        bool selected_;

        std::vector<PointsDrawable *> points_drawables_;
        std::vector<LinesDrawable *> lines_drawables_;
        std::vector<TrianglesDrawable *> triangles_drawables_;
    };

}

#endif  // EASY3D_RENDERER_RENDERER_H
