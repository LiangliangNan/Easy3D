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

#ifndef EASY3D_TUTORIAL_OBJECT_MANIPULATION_VIEWER_H
#define EASY3D_TUTORIAL_OBJECT_MANIPULATION_VIEWER_H

#include <easy3d/viewer/viewer.h>

// This class demonstrates how to manipulate an object in the 3D space using the mouse.

// Forward declaration only: the header just needs the Model name for its signatures, so we
// avoid pulling the full model header into every includer.
namespace easy3d {
    class Model;
}

/**
 * \brief A Viewer that lets the user pick a model and then rotate/translate it with Alt+drag.
 *
 * The mouse overrides split the interaction in two: a plain left click picks (and toggles)
 * the selected model via GPU picking, while Alt + left/right drag drives the selected
 * model's ManipulatedFrame (rotate / translate). focus_event() and post_resize() abort an
 * in-flight drag when the window state changes, and pre_draw() renders the manipulation
 * gizmo as an overlay. Models are referenced weakly so deleting a model cannot leave a
 * dangling pointer behind.
 */
class TutorialObjectManipulation : public easy3d::Viewer {
public:
    // Sets up a front view of the logo and installs the usage manual/hint.
    explicit TutorialObjectManipulation(const std::string &title);

protected:
    // Plain left click picks a model; Alt + button starts a manipulation drag.
    bool mouse_press_event(int x, int y, int button, int modifiers) override;
    // Alt + drag rotates (left) or translates (right) the model being dragged.
    bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) override;
    // Any release ends an in-flight manipulation drag.
    bool mouse_release_event(int x, int y, int button, int modifiers) override;
    // Losing focus aborts the drag (the release may arrive elsewhere).
    bool focus_event(bool focused) override;
    // A resize invalidates screen-space drag coordinates, so abort the drag.
    void post_resize(int width, int height) override;
    // Draws the selected model's manipulation gizmo (or clears the overlay).
    void pre_draw() override;

private:
    // Selects/deselects the clicked model and attaches a Manipulator when selected.
    void mark(easy3d::Model *model);

    // Returns the selected model only if it is still alive, in the scene, and manipulable.
    std::shared_ptr<easy3d::Model> selected_model() const;
    // Ends the current drag (action_end on the frame) and forgets the dragged model.
    void end_model_drag();
    // Weak (non-owning) handles: the currently selected model and the one being dragged.
    std::weak_ptr<easy3d::Model> selected_model_, drag_model_;
};

#endif // EASY3D_TUTORIAL_OBJECT_MANIPULATION_VIEWER_H
