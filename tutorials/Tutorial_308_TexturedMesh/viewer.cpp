/********************************************************************
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
 ********************************************************************/

#include "viewer.h"
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/obj_materials.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

namespace easy3d {
#ifdef EASY3D_HAS_VULKAN
    TexturedViewer::TexturedViewer(const std::string& title,const VulkanViewerOptions& options)
        : Viewer(title,options) {
#else
    TexturedViewer::TexturedViewer(const std::string& title) : Viewer(title) {
#endif
        camera()->setUpVector(vec3(0,1,0));
    }
    Model* TexturedViewer::add_model(const std::string& file,bool create) {
        try {
            // Parse before replacing the scene, so read failures retain the current model.
            Model* model=nullptr;
            if (file_system::extension(file,true)=="obj") {
                auto asset=load_obj_materials(file);
                model=Viewer::add_model(asset.mesh,false);
                if (!model) return nullptr;
                try { asset.create_drawables(); }
                catch (...) { delete_model(model); throw; }
            } else {
                model=Viewer::add_model(file,create);
                if (!model) return nullptr;
            }
            const auto previous=models();
            for (const auto& old:previous) if (old.get()!=model) delete_model(old.get());
            fit_screen();
            return model;
        } catch (const std::exception& e) {
            LOG(ERROR) << e.what(); return nullptr;
        }
    }
}
