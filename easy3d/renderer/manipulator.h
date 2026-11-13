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

#ifndef EASY3D_RENDERER_MANIPULATOR_H
#define EASY3D_RENDERER_MANIPULATOR_H

#include <easy3d/core/types.h>
#include <easy3d/renderer/render_device.h>


namespace easy3d {

    class Camera;
    class Model;
    class ManipulatedFrame;
#ifndef EASY3D_HAS_VULKAN
    class LinesDrawable;
#endif

    /**
     * \brief A manipulator is for manipulation of an object.
     * \details A manipulator maintains the manipulated frame of an object. The manipulated frame is used to
     *      manipulate this object. In this implementation, the origin of the manipulated frame is always at
     *      the center of the object.
     * \class Manipulator easy3d/renderer/manipulator.h
     *
     * The following code shows how to use this manipulator:
     * \code
     *      // create a manipulator and attach it to the model
     *      model->set_manipulator(new Manipulator(model));
     *      // connect the manipulator's signal to the viewer's update function to automatically update rendering.
     *      model->manipulator()->frame()->modified.connect(viewer, static_cast<void (Viewer::*)(void)>(&Viewer::update));
     *      // then in the mouse event, call the following functions:
     *      model->manipulator()->frame()->action_rotate(x, y, dx, dy, camera, axis); // axis = NONE for free rotation
     *      model->manipulator()->frame()->action_translate(x, y, dx, dy, camera, axis); // axis = NONE for free translation
     * \endcode
     */
    class Manipulator {
    public:
        /**
         * \brief Create a manipulator for an optional model.
         * \param model Model whose transform is manipulated; may be nullptr.
         */
        explicit Manipulator(Model *model = nullptr);

        /// Destroy the manipulator and its managed frame.
        ~Manipulator();

        /// Reset the manipulated frame to the identity transform.
        void reset();

        /// Return the manipulated frame.
        /// \return Non-owning pointer to the manipulated frame.
        ManipulatedFrame *frame() { return frame_; }

        /// Return the manipulated frame.
        /// \return Non-owning pointer to the manipulated frame.
        const ManipulatedFrame *frame() const { return frame_; }

        /**
         * \brief Return the transform applied to the manipulated object.
         * \details Rotation is performed around the object center. The returned
         * transform is therefore frame()->matrix() * translation(-center), not
         * frame()->matrix() alone.
         * \return Object transform.
         */
        mat4 matrix() const;

#ifndef EASY3D_HAS_VULKAN
        /// Draw the manipulated frame with OpenGL's native line rendering.
        /// \param cam Camera used for rendering.
        void draw_frame(const Camera* cam) const;
#endif
        /**
         * \brief Create a depth-tested frame annotation for Viewer::set_overlays().
         * \param cam Camera used to project the frame.
         * \param width Framebuffer width in pixels.
         * \param height Framebuffer height in pixels.
         * \param line_width Annotation line width in framebuffer pixels.
         * \return Immutable draw command for the annotation.
         */
        graphics::DrawCommand draw_frame(const Camera* cam, int width, int height, float line_width = 1) const;

    protected:
        Model *model_; // the model to be manipulated
        ManipulatedFrame *frame_;
        mutable graphics::DrawCommand frame_command_;
        mutable mat4 frame_mvp_;
        mutable int frame_width_ = 0, frame_height_ = 0;
        mutable float frame_line_width_ = 0;
#ifndef EASY3D_HAS_VULKAN
        LinesDrawable* drawable_model_bbox_;
#endif
    };

}

#endif  // EASY3D_RENDERER_MANIPULATOR_H