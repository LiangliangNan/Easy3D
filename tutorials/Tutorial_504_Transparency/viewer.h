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

#ifndef EASY3D_TUTORIAL_TRANSPARENCY_H
#define EASY3D_TUTORIAL_TRANSPARENCY_H

#include <easy3d/viewer/viewer.h>

/**
 * \brief A Viewer that renders a surface mesh with order-independent transparency.
 *
 * This class demonstrates switching between the OIT techniques of Easy3D at runtime. All
 * techniques derive from the abstract Transparency base class, so a single base-class
 * pointer can hold whichever method is active; 'space' cycles through disabled / Average
 * Color Blending / Dual Depth Peeling / Per Pixel Linked Lists, and 'up'/'down' change the
 * surface opacity. Unlike the shadow/AO effect tutorials, draw() does not use a begin()/end()
 * sandwich: the Transparency interface takes over the rendering of the surfaces itself via
 * its draw(surfaces) call.
 */

// Forward declaration only: the header does not need the full Transparency definition (a
// pointer member is enough), so we avoid pulling its header into every includer.
namespace easy3d {
    class Transparency;
}

class TutorialTransparency : public easy3d::Viewer {
public:
    // Builds the viewer and creates the initial technique (Dual Depth Peeling).
    explicit TutorialTransparency(const std::string &title = "");
    // Releases the owned technique object. The base-class destructor handles the rest.
    ~TutorialTransparency() override;

protected:
    // Handles 'space' (cycle technique) and 'up'/'down' (opacity); forwards other keys to Viewer.
    bool key_press_event(int key, int modifiers) override;
    // Renders the surfaces with the active technique, or plainly when disabled (see viewer.cpp).
    void draw() const override;

private:
    easy3d::Transparency *transparency_; //!< The active technique; null when transparency is disabled.
    int method_;              //!< Active method index — 0: disabled; 1: ACB; 2: DDP; 3: per-pixel linked lists.
    std::string hint_string_; //!< Common prefix of the usage text; the method name is appended on each switch.
};

#endif // EASY3D_TUTORIAL_TRANSPARENCY_H
