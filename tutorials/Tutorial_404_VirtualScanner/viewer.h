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

#ifndef EASY3D_TUTORIAL_VIRTUAL_SCANNER_H
#define EASY3D_TUTORIAL_VIRTUAL_SCANNER_H

#include <easy3d/viewer/viewer.h>

/**
 * \brief A Viewer that captures the visible surface as a point cloud, like a virtual 3D scanner.
 *
 * Only key_press_event() is overridden: 'space' renders the scene, reads back the depth
 * buffer, back-projects every valid pixel into world space, and adds the resulting point
 * cloud as a new model; 'n' toggles optional Gaussian sensor noise. All navigation events
 * are handled by the base Viewer, so the user can freely choose the scanning viewpoint.
 */
class TutorialVirtualScanner : public easy3d::Viewer {
public:
    // Installs the usage manual/hint and fixes the camera's up direction.
    explicit TutorialVirtualScanner(const std::string &title = "VirtualScanner");

protected:
    // Handles 'space' (capture a scan) and 'n' (toggle noise); forwards other keys to Viewer.
    bool key_press_event(int key, int modifiers) override;

private:
    bool add_noise_ = false; //!< When true, captured points are perturbed with Gaussian noise.
};

#endif // EASY3D_TUTORIAL_VIRTUAL_SCANNER_H
