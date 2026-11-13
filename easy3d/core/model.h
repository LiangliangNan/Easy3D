/** \cond LICENSE
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
 \endcond */

#ifndef EASY3D_CORE_MODEL_H
#define EASY3D_CORE_MODEL_H


#include <string>
#include <vector>
#include <memory>

#include <easy3d/core/types.h>


namespace easy3d {

    class Renderer;
    class Manipulator;

    /**
     * \brief Base class for spatial models with optional rendering and manipulation state.
     * \class Model easy3d/core/model.h
     * \details Concrete models provide their vertex positions through points()
     * and may provide additional typed properties. The bounding box is cached
     * until invalidated; it does not include manipulator transforms. Renderer
     * and manipulator ownership is shared through std::shared_ptr.
     * \see Graph, PointCloud, PolyMesh, SurfaceMesh
     */
    class Model
    {
    public:
        /**
         * \brief Construct a model with a name.
         * \param name Optional model name, defaulting to "unknown".
         */
        explicit Model(const std::string& name = "unknown");

        /// Destroy the model.
        virtual ~Model();

        /// Set the model name.
        /// \param n New name.
        void set_name(const std::string& n) { name_ = n; }
        /// Return the model name.
        /// \return Name string.
        const std::string& name() const { return name_; }

        /**
         * \brief The bounding box of the model.
         * \param recompute If \c true, or if the bounding box is not known, it computes and returns the bounding
         *      box of the model. Otherwise, it returns the known bounding box.
         * \note Manipulation transformation is not handled.
         * \see invalidate_bounding_box().
         * \return Cached or recomputed model bounds.
         */
        const Box3& bounding_box(bool recompute = false) const;

        /**
         * \brief Invalidates the bounding box of the model. So when bounding_box() is called, the bounding box will
         * be re-computed. This function is typically called when the geometry of a model is changed.
         */
        void invalidate_bounding_box();

        /**
         * \brief Return mutable vertex positions.
         * \return Model positions; modifying them may require invalidating derived bounds.
         */
        virtual std::vector<vec3>& points() = 0;
        /// Return the model's vertex positions.
        /// \return Model positions.
        virtual const std::vector<vec3>& points() const = 0;

        /// Test whether the model contains any points.
        /// \return True when points() is empty.
        bool empty() const { return points().empty(); };

        /**
         * \brief Write property names and statistics to a stream.
         * \param output Destination stream.
         */
        virtual void property_stats(std::ostream &output) const {}

        /**
         * \brief Sets the renderer of this model.
         * \param r Shared renderer to associate with the model.
         */
        void set_renderer(std::shared_ptr<Renderer> r) { renderer_ = r; }
        /// Return the associated renderer.
        /// \return Non-owning renderer pointer, or nullptr if unset.
        Renderer* renderer();
        /// Return the associated renderer from a const model.
        /// \return Non-owning renderer pointer, or nullptr if unset.
        const Renderer* renderer() const;

        /**
         * \brief Attaches a manipulator to this model.
         * \param manip Shared manipulator to associate with the model.
         */
        void set_manipulator(std::shared_ptr<Manipulator> manip) { manipulator_ = manip; }

        /// Return the associated manipulator.
        /// \return Non-owning manipulator pointer, or nullptr if unset.
        Manipulator* manipulator() { return manipulator_.get(); }
        /// Return the associated manipulator from a const model.
        /// \return Non-owning manipulator pointer, or nullptr if unset.
        const Manipulator* manipulator() const { return manipulator_.get(); }

    protected:
        std::string	name_;

        Box3		bbox_;
        bool		bbox_known_;

        std::shared_ptr<Renderer> renderer_;         // for rendering
        std::shared_ptr<Manipulator> manipulator_;   // for manipulation
    };
}

#endif  // EASY3D_CORE_MODEL_H
