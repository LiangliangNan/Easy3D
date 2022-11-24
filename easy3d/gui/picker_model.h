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
#include <easy3d/renderer/state.h>
#include <easy3d/renderer/camera.h>

#include <vector>
#include <unordered_map>


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
        explicit ModelPicker(const Camera *cam);

        ~ModelPicker() override = default;

        /**
         * \brief Pick a model from a set of models given the cursor position in the screen coordinate system.
         * @param models The models
         * @param x The cursor x-coordinate, relative to the left edge of the content area.
         * @param y The cursor y-coordinate, relative to the top edge of the content area.
         * @attention The screen point is expressed in the screen coordinate system with an origin in the upper left
         *            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with
         *            a Retina display.
         * @return The picked model.
         */
        Model *pick(const std::vector<Model *> &models, int x, int y);

    private:

        // render each model of the scene with a unique color
        void draw(const std::vector<Model *> &models);
        // render the drawable with color
        void draw(Drawable *drawable, const vec4 &color);

    private:

        std::unordered_map<Drawable *, State> states_;

    };


}


#endif  // EASY3D_GUI_PICKER_MODEL_H
