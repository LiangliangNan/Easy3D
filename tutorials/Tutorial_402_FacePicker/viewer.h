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

#ifndef EASY3D_TUTORIAL_FACE_PICKER_VIEWER_H
#define EASY3D_TUTORIAL_FACE_PICKER_VIEWER_H

#include <easy3d/viewer/viewer.h>

// This class demonstrates how to pick faces in a surface mesh using the mouse.

// Forward declaration only: the header just needs the Model name for the add_model()
// signature, so we avoid pulling the full model header into every includer.
namespace easy3d {
    class Model;
}

/**
 * \brief A Viewer that highlights the surface-mesh face under a left click.
 *
 * Two overrides do the work: mouse_press_event() runs the GPU pick and converts the hit
 * triangle into the original face's highlight range, and add_model() ensures the mesh edges
 * are visible so the faces can be told apart. All other events go to the base Viewer.
 */
class TutorialFacePicker : public easy3d::Viewer {
public:
    // Sets up a front view and installs the on-screen usage hint.
    explicit TutorialFacePicker(const std::string &title);

    // Loads a model via the base class, then makes its "edges" drawable visible.
    easy3d::Model *add_model(const std::string &file_name, bool create_default_drawables = true) override;

protected:
    // Intercepts plain left clicks to pick and highlight a face; forwards the rest to Viewer.
    bool mouse_press_event(int x, int y, int button, int modifiers) override;
};

#endif // EASY3D_TUTORIAL_FACE_PICKER_VIEWER_H
