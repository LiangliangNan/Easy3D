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

#include "viewer.h"
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/obj_materials.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

using namespace easy3d;

// \cond
TexturedViewer::TexturedViewer(const std::string &title) : Viewer(title) {
    // Most OBJ assets (including the demo models) are authored Y-up; make the camera
    // agree so the scene does not appear lying on its side.
    camera()->set_up_vector(vec3(0, 1, 0));
}

// The texture-aware loading pipeline. The contract of add_model() is: on success the new
// model becomes the current one; on failure nothing is lost and nullptr is returned.
Model *TexturedViewer::add_model(const std::string &file, bool create, bool fit_scene) {
    try {
        // Parse before replacing the scene, so read failures retain the current model.
        Model *model = nullptr;
        if (file_system::extension(file, true) == "obj") {
            // Parse OBJ + MTL in one pass: the asset holds the SurfaceMesh plus one Part
            // per material (diffuse color, texture file, seam-split vertex attributes).
            auto asset = load_obj_materials(file);
            // Attach the mesh WITHOUT default drawables: create_drawables() below will
            // add one textured drawable per material part instead of a single plain one.
            // Skip the fit here: it is done below, once the previous models are gone.
            model = Viewer::add_model(asset.mesh, false, false);
            if (!model)
                return nullptr;
            try {
                asset.create_drawables();
            } catch (...) {
                // Drawable creation failed: remove the half-set-up model so the viewer
                // does not keep an empty scene, then report the error to the caller.
                delete_model(model);
                throw;
            }
        } else {
            // Not an OBJ: keep the standard loading behavior for all other formats.
            model = Viewer::add_model(file, create, false);
            if (!model)
                return nullptr;
        }
        // This tutorial viewer shows one model at a time: drop any previously loaded ones.
        const auto previous = models();
        for (const auto &old : previous)
            if (old.get() != model)
                delete_model(old.get());
        // Center and scale the camera so the whole model is visible.
        if (fit_scene)
            fit_screen();
        return model;
    } catch (const std::exception &e) {
        // Parsing errors (bad file, invalid indices, ...) are reported, not propagated,
        // so a failed open leaves the viewer usable.
        LOG(ERROR) << e.what();
        return nullptr;
    }
}
// \endcond