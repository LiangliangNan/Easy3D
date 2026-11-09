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

#ifndef EASY3D_TUTORIAL_MODEL_PICKER_VIEWER_H
#define EASY3D_TUTORIAL_MODEL_PICKER_VIEWER_H

#include <easy3d/viewer/viewer.h>

// This class demonstrates how to pick a model using the mouse.

// Forward declaration only: the header just needs the Model name for the mark() signature,
// so we avoid pulling the full model header into every includer.
namespace easy3d {
    class Model;
}

/**
 * \brief A Viewer that selects/deselects a model when the user clicks on it.
 *
 * The subclass overrides mouse_press_event() to run a GPU pick (Viewer::pick()) at the
 * cursor position and toggle the selection state of the hit model. All other events are
 * forwarded to the base Viewer, so camera navigation is unaffected.
 */
class TutorialModelPicker : public easy3d::Viewer {
public:
    // Sets up a front view of the logo and installs the on-screen usage hint.
    explicit TutorialModelPicker(const std::string &title);

protected:
    // Intercepts plain left clicks to pick a model; forwards everything else to Viewer.
    bool mouse_press_event(int x, int y, int button, int modifiers) override;

private:
    // Toggles the selection of the picked model and refreshes the colors of all models.
    void mark(easy3d::Model *model);
};

#endif // EASY3D_TUTORIAL_MODEL_PICKER_VIEWER_H
