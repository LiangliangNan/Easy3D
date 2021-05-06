/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
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
 ********************************************************************/

#ifndef EASY3D_RENDERER_MANIPULATOR_H
#define EASY3D_RENDERER_MANIPULATOR_H

#include <easy3d/core/types.h>


namespace easy3d {

    class Camera;
    class Model;
    class ManipulatedFrame;
    class LinesDrawable;

    /**
     * \brief A manipulator is for manipulation of an object.
     * \details A manipulator maintains the manipulated frame of an object. The manipulated frame is used to
     *      manipulate this object. In this implementation, the origin of the manipulated frame is always at
     *      the center of the object.
     * \class Manipulator easy3d/renderer/manipulator.h
     */
    class Manipulator {
    public:
        /// Constructor. A manipulator can be attached to a \p model.
        Manipulator(Model *model = nullptr);

        ~Manipulator();

        /// Resets the manipulated frame, i.e., no transformation.
        void reset();

        /// Returns the manipulated frame.
        ManipulatedFrame *frame() { return frame_; }

        /// Returns the manipulated frame.
        const ManipulatedFrame *frame() const { return frame_; }

        /// Returns the transformation introduced by this manipulator.
        /// \note Rotation is performed around the 'center' of the object.
        mat4 matrix() const;

        /// Draws the manipulated frame.
        void draw_frame(Camera* cam) const;

    protected:
        Model *model_; // the model to be manipulated
        ManipulatedFrame *frame_;
        LinesDrawable* drawable_model_bbox_;
    };

}

#endif  // EASY3D_RENDERER_MANIPULATOR_H