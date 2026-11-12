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

#ifndef EASY3D_GUI_PICKER_MODEL_H
#define EASY3D_GUI_PICKER_MODEL_H


#include <easy3d/gui/picker.h>


#include <memory>
#include <vector>



namespace easy3d {

    class Model;
    class Drawable;

    /**
     * \brief Implementation of picking mechanism for set of models.
     * \class ModelPicker easy3d/gui/picker_model.h
     * \see Picker, PointCloudPicker, SurfaceMeshPicker
     */
    class ModelPicker : public Picker {
    public:
        /**
         * \brief Constructor.
         * \param cam Non-null camera used for picking; it is borrowed and must outlive the picker.
         */
        explicit ModelPicker(const Camera *cam);
        /**
         * \brief Destructor.
         */
        ~ModelPicker() override = default;

        /**
         * \brief Pick a model from a set of models given the cursor position in the screen coordinate system.
         * \param models The models to pick from.
         * \param x The cursor x-coordinate in logical window units (top-left origin, x right).
         * \param y The cursor y-coordinate in logical window units (top-left origin, y down).
         * \return The visible model at the cursor, or nullptr if the list is empty, the coordinate is out of bounds,
         *         or no visible drawable covers the pixel.
         * \note The coordinate is in logical window units (camera()->screen_width()/Height()), not framebuffer pixels.
         *       On HiDPI displays the logical extent is smaller than the framebuffer by the DPI scaling factor.
         *       Null models and models without visible renderers are skipped. The method renders visible drawables
         *       with their model's unique ID into an offscreen target, then samples the
         *       pixel at (x, y) to determine which model is nearest. Both OpenGL and Vulkan backends present the
         *       same coordinate contract.
         */
        Model *pick(const std::vector< std::shared_ptr<Model> >& models, int x, int y);

#ifndef EASY3D_HAS_VULKAN
    private:
        /**
         * \brief Render each model of the scene with a unique color.
         * \param models The models to render.
         */
        void draw(const std::vector< std::shared_ptr<Model> >& models);
        /**
         * \brief Render the drawable with color.
         * \param drawable The drawable to render.
         * \param color The color to use for rendering.
         */
        void draw(Drawable *drawable, const vec4 &color);

#endif
    };


}



#endif  // EASY3D_GUI_PICKER_MODEL_H
