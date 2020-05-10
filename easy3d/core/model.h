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

#ifndef EASY3D_CORE_MODEL_H
#define EASY3D_CORE_MODEL_H


#include <string>
#include <vector>

#include <easy3d/core/types.h>


namespace easy3d {

    class Drawable;
    class PointsDrawable;
    class LinesDrawable;
    class TrianglesDrawable;

    /**
     * Model is the base class of renderable 3D models, e.g., SurfaceMesh, PointCloud, Graph.
     * A Model can have multiple drawables, e.g., faces, vertices, edges.
     */
    class Model
    {
    public:
        Model(const std::string& name = "unknown");
        virtual ~Model();

        const std::string& name() const { return name_; }
        void set_name(const std::string& n) { name_ = n; }

        /** The bounding box of the model. */
        virtual const Box3& bounding_box() const;

        /** The vertices of the model. */
        virtual const std::vector<vec3>& points() const = 0;

        /** Test if the model is empty. */
        bool empty() const { return points().empty(); };

        /// prints the names of all properties to an output stream (e.g., std::cout).
        virtual void property_stats(std::ostream &output) const = 0;

        //-------------------- drawable management  -----------------------

        /**
         * Add a drawable to this model. The model will take the ownership of the drawable (e.g., memory management).
         * @param drawable The drawable to be added to this model.
         * @return True if the drawable has been successfully added to this model. False if a drawable with the same
         *         name already exists and could not been added.
         */
        bool add_drawable(Drawable* drawable);

        /**
         * Get the drawable of a given name.
         * Return a nullptr if the drawable does not exist.
         */
        Drawable* drawable(const std::string& name) const;

        // Return all available drawables.
        const std::vector<Drawable*>&  drawables() const { return drawables_; }

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

    protected:
        std::string	name_;

        Box3		bbox_;
        bool		bbox_known_;

        bool		visible_;
        bool        selected_;

        std::vector<Drawable*>  drawables_;
    };
}

#endif  // EASY3D_CORE_MODEL_H
