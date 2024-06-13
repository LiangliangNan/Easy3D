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

#include <easy3d/viewer/offscreen.h>


namespace easy3d {

    OffScreen::OffScreen(int width, int height)
            : Viewer("Easy3D OffScreen Renderer", 4, 3, 2, false, true, 24, 8, width, height)
    {
        usage_string_ = "";
        init();
    }


    Camera *OffScreen::camera() {
        return Viewer::camera();
    }


    const Camera *OffScreen::camera() const {
        return Viewer::camera();
    }


    bool OffScreen::render(const std::string &file_name, float scaling, int samples, int back_ground, bool expand) const {
        return snapshot(file_name, scaling, samples, back_ground, expand);
    }


    void OffScreen::resize(int w, int h) {
        Viewer::resize(w, h);
    }


    int OffScreen::width() const {
        return Viewer::width();
    }


    int OffScreen::height() const {
        return Viewer::height();
    }


    void OffScreen::set_background_color(const easy3d::vec4 &c) {
        Viewer::set_background_color(c);
    }


    const easy3d::vec4& OffScreen::background_color() const {
        return Viewer::background_color();
    }


    Model* OffScreen::add_model(const std::string &file_name, bool create_default_drawables) {
        Model* model = Viewer::add_model(file_name, create_default_drawables);
        fit_screen();
        return model;
    }


    Model* OffScreen::add_model(Model *model, bool create_default_drawables) {
        return Viewer::add_model(model, create_default_drawables);
    }


    bool OffScreen::delete_model(Model *model) {
        return Viewer::delete_model(model);
    }


    const std::vector<Model *>& OffScreen::models() const {
        return Viewer::models();
    }


    Model * OffScreen::current_model() const {
        return Viewer::current_model();
    }


    bool OffScreen::add_drawable(Drawable *drawable) {
        return Viewer::add_drawable(drawable);
    }


    bool OffScreen::delete_drawable(Drawable *drawable) {
        return Viewer::delete_drawable(drawable);
    }


    const std::vector<Drawable *>& OffScreen::drawables() const {
        return drawables_;
    }


    void OffScreen::clear_scene() {
        return Viewer::clear_scene();
    }

}
