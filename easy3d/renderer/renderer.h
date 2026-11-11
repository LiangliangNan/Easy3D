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

#ifndef EASY3D_RENDERER_RENDERER_H
#define EASY3D_RENDERER_RENDERER_H

#include <string>
#include <vector>
#include <memory>

#include <easy3d/core/types.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>

namespace easy3d {

    class Model;
    class Graph;
    class PointsDrawable;
    class LinesDrawable;
    class TrianglesDrawable;

    /**
     * \brief Manage the drawables used to render a model.
     * \details A Renderer owns the point, line, and triangle drawables
     * associated with one Model. It can create standard drawables for supported
     * model types; custom visualizations can instead create and configure
     * drawables explicitly. Model geometry or attribute changes must be
     * reflected in drawable buffers, either through the standard update path or
     * a custom drawable update function.
     * \class Renderer easy3d/renderer/renderer.h
     * \sa Drawable, PointsDrawable, LinesDrawable, TrianglesDrawable
     *
     * The following code shows how to use renderer:
     * \code
     *      // create a renderer and attach it to the model
     *      model->set_renderer(std::make_shared<Renderer>(model.get(), create));
     * \endcode
     */
    class Renderer {
    public:
        /**
         * \brief Create a renderer for a model.
         * \param model Model whose rendering drawables are managed.
         * \param create_drawables True to create default drawables for this model.
         * \details When requested, default drawables are created for supported
         * model types: PointCloud ("vertices"); SurfaceMesh ("faces", "vertices",
         * "edges", "borders"); Graph ("vertices", "edges"); and PolyMesh
         * ("faces:border", "faces:interior"). Call update() after changing model
         * data to refresh these buffers. For custom visualization, pass false,
         * create the desired drawable, and provide an update function if its
         * buffers must be rebuilt when model data changes.
         * \sa update()
         */
        explicit Renderer(Model *model, bool create_drawables = true);

        /// Destroy the renderer and release its managed drawables.
        virtual ~Renderer();

        /**
         * \brief Return the model associated with this renderer.
         * \return The non-owning model pointer supplied to the constructor.
         */
        Model *model() { return model_; }

        //-------------------- rendering functionalities  -----------------------

        /// Return whether the model is currently visible.
        /// \return True when the model's drawables are eligible for rendering.
        bool is_visible() const { return visible_; }

        /// Set whether the model is visible.
        /// \param b True to show the model; false to hide it.
        void set_visible(bool b) { visible_ = b; }

        /// Return whether the model is selected.
        /// \return True when selected.
        bool is_selected() const { return selected_; }

        /**
         * \brief Select or deselect the model.
         * \param b True to select the model; false to deselect it.
         * \details The selected state is propagated to all of the model's
         * drawables.
         */
        void set_selected(bool b);

        /**
         * @brief Invalidates the rendering buffers of the model and thus updates the rendering (delayed in rendering).
         * @details This method triggers an update of the rendering buffers of all the drawables of the model to which
         *      this renderer is attached. The effect is equivalent to calling Drawable::update() functions for all
         *      the drawables of this model.
         * todo: for better performance, it is wise to update only the affected drawables and buffers.
         * \sa  Drawable::update()
         */
        void update();

        //-------------------- drawable management  -----------------------

        /**
         * \brief Find a point drawable by name.
         * \param name Drawable name.
         * \param warning_not_found If true, log a warning when no drawable matches.
         * \return The matching drawable, or nullptr if it does not exist.
         */
        PointsDrawable *get_points_drawable(const std::string &name, bool warning_not_found = true) const;

        /**
         * \brief Find a line drawable by name.
         * \param name Drawable name.
         * \param warning_not_found If true, log a warning when no drawable matches.
         * \return The matching drawable, or nullptr if it does not exist.
         */
        LinesDrawable *get_lines_drawable(const std::string &name, bool warning_not_found = true) const;

        /**
         * \brief Find a triangle drawable by name.
         * \param name Drawable name.
         * \param warning_not_found If true, log a warning when no drawable matches.
         * \return The matching drawable, or nullptr if it does not exist.
         */
        TrianglesDrawable *get_triangles_drawable(const std::string &name, bool warning_not_found = true) const;

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
         * \brief Return the managed point drawables.
         * \return A const reference to the renderer-owned collection.
         */
        const std::vector< std::shared_ptr<PointsDrawable> > &points_drawables() const { return points_drawables_; }

        /**
         * \brief Return the managed line drawables.
         * \return A const reference to the renderer-owned collection.
         */
        const std::vector< std::shared_ptr<LinesDrawable> > &lines_drawables() const { return lines_drawables_; }

        /**
         * \brief Return the managed triangle drawables.
         * \return A const reference to the renderer-owned collection.
         */
        const std::vector< std::shared_ptr<TrianglesDrawable> > &triangles_drawables() const { return triangles_drawables_; }

    public:

        /**
         * @brief Generates random colors for visualizing face-based segmentation of a SurfaceMesh.
         * @param mesh The surface mesh.
         * @param segments The face property storing the segmentation of the surface mesh.
         * @param colors The face property to store the colors. Must be allocated before hand.
         * \attention The face property \c segments must have a type of \c int, \c {unsigned int}, \c std::size_t,
         *      \c bool, or \c {unsigned char}. Besides, any negative values are treated as out of range.
         */
        template<typename FT>
        static void color_from_segmentation(
                SurfaceMesh *mesh,
                SurfaceMesh::FaceProperty<FT> segments,
                SurfaceMesh::FaceProperty<vec3> colors
        );

        /**
         * @brief Generates random colors for visualizing vertex-based segmentation of a SurfaceMesh.
         * @param mesh The surface mesh.
         * @param segments The vertex property storing the segmentation of the surface mesh.
         * @param colors The vertex property to store the colors. Must be allocated before hand.
         * \attention The face property \c segments must have a type of \c int, \c {unsigned int}, \c std::size_t,
         *      \c bool, or \c {unsigned char}. Besides, any negative values are treated as out of range.
         */
        template<typename FT>
        static void color_from_segmentation(
                SurfaceMesh *mesh,
                SurfaceMesh::VertexProperty<FT> segments,
                SurfaceMesh::VertexProperty<vec3> colors
        );

        /**
         * @brief Generates random colors for visualizing the segmentation of a PointCloud.
         * @param cloud  The point cloud.
         * @param segments  The vertex property storing the segmentation of the point cloud.
         * \attention The face property \c segments must have a type of \c int, \c {unsigned int}, \c std::size_t,
         *      \c bool, or \c {unsigned char}. Besides, any negative values are treated as out of range.
         * @param colors The vertex property to store the colors. Must be allocated before hand.
         */
        template<typename FT>
        static void color_from_segmentation(
                PointCloud *cloud,
                PointCloud::VertexProperty<FT> segments,
                PointCloud::VertexProperty<vec3> colors
        );

    private:
        /*
         * @brief Create default drawables for rendering.
         * @details This method creates defaults drawables for rendering a model. The supported default drawables are
         *              - PointCloud: "vertices".
         *              - SurfaceMesh: "faces", "vertices", "edges", "borders".
         *              - Graph: "vertices", "edges".
         *          These drawables are usually sufficient for basic rendering of the model. In case
         *          the default drawables don't meet the particular visualization purpose, you can
         *          override this function or set 'create_drawables' to false and create the
         *          drawables by calling Model::add_[type]_drawable().
         */
        void create_default_drawables();

        // -------------------------------------------------------------------------------------------------------------

        /*
         * @brief Set the default rendering state of the "vertices" drawable of a point cloud.
         * @details The default rendering state is determined by the availability of the vertex properties.
         *          The motivation is that the most appealing rendering is demonstrated by default. The following
         *          priority applies:
         *              1. per-vertex color: in "v:color";
         *              2. per-vertex texture coordinates: in "v:texcoord";
         *              3. segmentation: in "v:primitive_index";
         *              4. scalar field;
         *              5. uniform color.
         */
        void set_default_rendering_state(PointCloud *model, PointsDrawable *drawable);

        /*
         * @brief Set the default rendering state of the "faces" drawable of a surface mesh.
         * @details The default rendering state is determined by the availability of the vertex/face properties.
         *          The motivation is that the most appealing rendering is demonstrated by default. The following
         *          priority applies:
         *              1. per-face color: in "f:color";
         *              2. per-vertex color: in "v:color";
         *              3. per-halfedge texture coordinates: in "h:texcoord";
         *              4. per-vertex texture coordinates: in "v:texcoord";
         *              5. segmentation: in "f:chart";
         *              6. scalar field on faces;
         *              7. scalar field on vertices;
         *              8. uniform color
         */
        void set_default_rendering_state(SurfaceMesh *model, TrianglesDrawable *drawable);

        /*
         * @brief Set the default rendering state of the "vertices" drawable of a graph.
         * @details The default rendering state is determined by the availability of the vertex properties.
         *          The motivation is that the most appealing rendering is demonstrated by default. The following
         *          priority applies:
         *              1. per-vertex color: in "v:color";
         *              2. per-vertex texture coordinates: in "v:texcoord";
         *              3. scalar field;
         *              4. uniform color.
         */
        void set_default_rendering_state(Graph *model, PointsDrawable *drawable);

    protected:
        Model* model_;

        bool visible_;
        bool selected_;

        std::vector< std::shared_ptr<PointsDrawable> >      points_drawables_;
        std::vector< std::shared_ptr<LinesDrawable> >       lines_drawables_;
        std::vector< std::shared_ptr<TrianglesDrawable> >   triangles_drawables_;
    };

}



// ------------------------------------------- Implementation -------------------------------------------


#include <easy3d/core/random.h>


namespace easy3d {

    template<typename FT> inline
    void Renderer::color_from_segmentation(SurfaceMesh *mesh, SurfaceMesh::FaceProperty<FT> segments,
                                           SurfaceMesh::FaceProperty<vec3> colors) {
        if (mesh->empty()) {
            LOG(WARNING) << "model has no valid geometry";
            return;
        }

        if (!segments) {
            LOG(WARNING) << "the surface mesh does not have face property \'" << segments.name() << "\'";
            return;
        }
        if (!colors) {
            LOG(WARNING) << "color property not allocated";
            return;
        }

        int max_index = 0;
        for (auto f : mesh->faces())
            max_index = std::max(max_index, static_cast<int>(segments[f]));

        // assign each segment a unique color
        std::vector<vec3> color_table(max_index + 1);   // index starts from 0
        for (auto &c : color_table)
            c = random_color();

        for (auto f : mesh->faces()) {
            int idx = static_cast<int>(segments[f]);
            if (idx == -1)
                colors[f] = vec3(0, 0, 0);
            else
                colors[f] = color_table[idx];
        }
    }


    template<typename FT> inline
    void Renderer::color_from_segmentation(SurfaceMesh *mesh, SurfaceMesh::VertexProperty<FT> segments,
                                           SurfaceMesh::VertexProperty<vec3> colors) {
        if (mesh->empty()) {
            LOG(WARNING) << "model has no valid geometry";
            return;
        }

        if (!segments) {
            LOG(WARNING) << "the surface mesh does not have vertex property \'" << segments.name() << "\'";
            return;
        }
        if (!colors) {
            LOG(WARNING) << "color property not allocated";
            return;
        }

        int max_index = 0;
        for (auto v : mesh->vertices())
            max_index = std::max(max_index, static_cast<int>(segments[v]));

        // assign each segment a unique color
        std::vector<vec3> color_table(max_index + 1);   // index starts from 0
        for (auto &c : color_table)
            c = random_color();

        for (auto v : mesh->vertices()) {
            int idx = static_cast<int>(segments[v]);
            if (idx == -1)
                colors[v] = vec3(0, 0, 0);
            else
                colors[v] = color_table[idx];
        }
    }


    template<typename FT> inline
    void Renderer::color_from_segmentation(PointCloud *cloud,
                                           const PointCloud::VertexProperty<FT> segments,
                                           PointCloud::VertexProperty<vec3> colors) {
        if (cloud->empty()) {
            LOG(WARNING) << "model has no valid geometry";
            return;
        }

        if (!segments) {
            LOG(WARNING) << "the point cloud does not have vertex property \'" << segments.name() << "\'";
            return;
        }
        if (!colors) {
            LOG(WARNING) << "color property not allocated" << "\'";
            return;
        }

        int max_index = 0;
        for (auto v : cloud->vertices())
            max_index = std::max(max_index, static_cast<int>(segments[v]));

        // assign each segment a unique color
        std::vector<vec3> color_table(max_index + 1);   // index starts from 0
        for (auto &c : color_table)
            c = random_color();

        for (auto v : cloud->vertices()) {
            int idx = static_cast<int>(segments[v]);
            if (idx == -1)
                colors[v] = vec3(0, 0, 0);
            else
                colors[v] = color_table[idx];
        }
    }

}


#endif  // EASY3D_RENDERER_RENDERER_H
