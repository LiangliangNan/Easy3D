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

#ifndef EASY3D_TUTORIAL_SOFT_SHADOW_H
#define EASY3D_TUTORIAL_SOFT_SHADOW_H

#include <easy3d/viewer/viewer.h>

/**
 * \brief A Viewer that renders the scene with soft shadows (PCSS).
 *
 * This class renders a scene with soft shadow using the Percentage-Closer Soft Shadows
 * (PCSS) technique. It follows the standard Easy3D effect pattern: derive from Viewer, own
 * an effect object, and override draw() so the base-class scene render is sandwiched between
 * the effect's begin() and end() calls. Compared to Tutorial_502_HardShadow, only the effect
 * object differs: SoftShadow derives from Shadow and adds blocker search + variable-size PCF
 * filtering. key_press_event() is overridden to toggle the shadow interactively.
 */

// Forward declaration only: the header does not need the full Shadow definition (a pointer
// member is enough), so we avoid pulling its header into every includer. The pointer actually
// holds a SoftShadow instance (see viewer.cpp); the virtual destructor of Shadow makes
// deleting through the base pointer safe.
namespace easy3d {
    class Shadow;
}

class TutorialSoftShadow : public easy3d::Viewer {
public:
    // Builds the viewer and creates the soft-shadow effect bound to its camera.
    explicit TutorialSoftShadow(const std::string &title = "");
    // Releases the owned effect. The base-class destructor handles the rest of the cleanup.
    ~TutorialSoftShadow() override;

protected:
    // Handles 'space' (toggle); forwards other keys to Viewer.
    bool key_press_event(int key, int modifiers) override;
    // Renders the scene, applying PCSS soft shadows when enabled (see viewer.cpp).
    void draw() const override;

private:
    easy3d::Shadow *shadow_; //!< The owned effect; actually a SoftShadow (created with new, deleted in the destructor).
    bool shadow_enabled_;    //!< Whether shadowing is currently applied; toggled with the space key.
};

#endif // EASY3D_TUTORIAL_SOFT_SHADOW_H
