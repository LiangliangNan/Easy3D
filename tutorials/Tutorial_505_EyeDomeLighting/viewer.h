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

#ifndef EASY3D_TUTORIAL_EYE_DOME_LIGHTING_H
#define EASY3D_TUTORIAL_EYE_DOME_LIGHTING_H

#include <easy3d/viewer/viewer.h>

/**
 * \brief A Viewer that renders a point cloud with Eye Dome Lighting (EDL).
 *
 * This class renders a point cloud without normal information and uses the Eye Dome Lighting
 * technique to improve depth perception. It follows the standard Easy3D effect pattern:
 * derive from Viewer, own an effect object, and override draw() so the base-class scene
 * render is sandwiched between the effect's begin() and end() calls. key_press_event() is
 * overridden to toggle the effect interactively.
 */

// Forward declaration only: the header does not need the full EyeDomeLighting definition
// (a pointer member is enough), so we avoid pulling its header into every includer.
namespace easy3d {
    class EyeDomeLighting;
}

class TutorialEyeDomeLighting : public easy3d::Viewer {
public:
    // Builds the viewer and creates the EDL effect bound to its camera.
    explicit TutorialEyeDomeLighting(const std::string &title = "");
    // Releases the owned effect. The base-class destructor handles the rest of the cleanup.
    ~TutorialEyeDomeLighting() override;

protected:
    // Handles 'space' (toggle); forwards other keys to Viewer.
    bool key_press_event(int key, int modifiers) override;

    // Renders the cloud, applying EDL when enabled (see viewer.cpp).
    void draw() const override;

private:
    easy3d::EyeDomeLighting *edl_; //!< The owned EDL effect (created with new, deleted in the destructor).
    bool edl_enabled_;             //!< Whether the effect is currently applied; toggled with the space key.
};

#endif // EASY3D_TUTORIAL_EYE_DOME_LIGHTING_H
