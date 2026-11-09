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

#ifndef EASY3D_TUTORIAL_TEXTURED_VIEWER_H
#define EASY3D_TUTORIAL_TEXTURED_VIEWER_H

#include <easy3d/viewer/viewer.h>

namespace easy3d {

    class SurfaceMesh;

    /**
     * \brief An enhanced viewer that can handle textured meshes (now it supports the OBJ format only).
     *
     * The only extension point needed is add_model(): for OBJ files it loads the geometry
     * together with the MTL materials and creates one textured drawable per material part;
     * every other format falls through to the default Viewer loading path. Because
     * add_model() is virtual in Viewer, file-open dialogs and drag-and-drop automatically
     * use this texture-aware version too.
     */
    class TexturedViewer : public Viewer {
    public:
        // Sets up a Y-up camera, matching the usual OBJ authoring convention.
        explicit TexturedViewer(const std::string &title = "");

        // Texture-aware loading: OBJ files are parsed with load_obj_materials() and their
        // material parts become textured drawables; other formats use Viewer::add_model().
        Model *add_model(const std::string &file_name, bool create_default_drawables = true) override;
    };

} // namespace easy3d

#endif //EASY3D_TUTORIAL_TEXTURED_VIEWER_H
