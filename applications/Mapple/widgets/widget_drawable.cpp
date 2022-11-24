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


#include "widget_drawable.h"

#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/core/model.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/resource.h>

#include "main_window.h"
#include "paint_canvas.h"
#include "widget_model_list.h"


using namespace easy3d;


std::vector<WidgetDrawable::ColorMap> WidgetDrawable::colormaps_;

WidgetDrawable::WidgetDrawable(QWidget *parent)
        : QWidget(parent), scalar_prefix_("scalar - ") {
    window_ = dynamic_cast<MainWindow *>(parent);
    viewer_ = window_->viewer();

    if (colormaps_.empty()) {
        const std::string dir = resource::directory() + "/colormaps/";
        if (file_system::is_file(dir + "default.png"))
            colormaps_.emplace_back(ColorMap(dir + "default.png", "default"));
        if (file_system::is_file(dir + "french.png"))
            colormaps_.emplace_back(ColorMap(dir + "french.png", "french"));
        if (file_system::is_file(dir + "rainbow.png"))
            colormaps_.emplace_back(ColorMap(dir + "rainbow.png", "rainbow"));
        if (file_system::is_file(dir + "blue_red.png"))
            colormaps_.emplace_back(ColorMap(dir + "blue_red.png", "blue_red"));
        if (file_system::is_file(dir + "blue_white.png"))
            colormaps_.emplace_back(ColorMap(dir + "blue_white.png", "blue_white"));
        if (file_system::is_file(dir + "blue_yellow.png"))
            colormaps_.emplace_back(ColorMap(dir + "blue_yellow.png", "blue_yellow"));
        if (file_system::is_file(dir + "black_white.png"))
            colormaps_.emplace_back(ColorMap(dir + "black_white.png", "black_white"));
        if (file_system::is_file(dir + "random.png"))
            colormaps_.emplace_back(ColorMap(dir + "random.png", "random"));
    }
}


WidgetDrawable::~WidgetDrawable() {
    if (!colormaps_.empty()) {
        for (const auto &colormap : colormaps_)
            delete colormap.texture;
        colormaps_.clear();
    }
}


void WidgetDrawable::setActiveDrawable(easy3d::Drawable* d) {
    if (!d || !d->model()) {
        LOG(ERROR) << "drawable (or model) is null";
        return;
    }

    active_drawable_[d->model()] = d->name();
}


Texture *WidgetDrawable::colormapTexture(int idx, bool discrete, int num_stripes) const {
    if (idx >= colormaps_.size())
        return nullptr;

    viewer_->makeCurrent();
    if (colormaps_[idx].name.find("random") != std::string::npos)
        return TextureManager::request(num_stripes, 256/num_stripes, Texture::CLAMP_TO_EDGE, Texture::LINEAR);
    else {
        if (discrete)
            return TextureManager::request(colormaps_[idx].file, num_stripes,Texture::CLAMP_TO_EDGE, Texture::LINEAR);
        else
            return TextureManager::request(colormaps_[idx].file, Texture::CLAMP_TO_EDGE, Texture::LINEAR);
    }
    viewer_->doneCurrent();
}


void WidgetDrawable::setDrawableVisible(bool b) {
    auto d = drawable();
    d->set_visible(b);
    window_->widgetModelList()->updateDrawableVisibility(d);
    viewer_->update();
    disableUnavailableOptions();
}


void WidgetDrawable::setLighting(const QString &text) {
    auto d = drawable();
    if (text == "front and back") {
        d->set_lighting(true);
        d->set_lighting_two_sides(true);
    } else if (text == "front only") {
        d->set_lighting(true);
        d->set_lighting_two_sides(false);
    } else if (text == "disabled") {
        d->set_lighting(false);
    }

    viewer_->update();
    disableUnavailableOptions();
}


void WidgetDrawable::setScalarFieldStyle(int idx) {
    auto d = drawable();
    states_[d].scalar_style = idx;
    auto tex = colormapTexture(states_[d].scalar_style, states_[d].discrete_color, states_[d].num_stripes);
    d->set_texture(tex);
    viewer_->update();
    disableUnavailableOptions();
}


void WidgetDrawable::setScalarFieldDiscreteColors(bool b) {
    auto d = drawable();
    states_[d].discrete_color = b;
    auto tex = colormapTexture(states_[d].scalar_style, states_[d].discrete_color, states_[d].num_stripes);
    d->set_texture(tex);
    viewer_->update();
    disableUnavailableOptions();
}


void WidgetDrawable::setScalarFieldNumOfStripes(int num) {
    auto d = drawable();
    states_[d].num_stripes = num;
    auto tex = colormapTexture(states_[d].scalar_style, states_[d].discrete_color, states_[d].num_stripes);
    d->set_texture(tex);
    viewer_->update();
}


void WidgetDrawable::setScalarFieldClamp(bool b) {
    auto d = drawable();
    d->set_clamp_range(b);
    d->update();
    viewer_->update();
    disableUnavailableOptions();
}


void WidgetDrawable::setScalarFieldClampLower(double v) {
    auto d = drawable();
    if (d->clamp_upper() * 100 + v < 100) {
        d->set_clamp_lower(v / 100.0f);
        d->update();
        viewer_->update();
    }
    else
        LOG(WARNING) << "invalid clamp range (the sum of lower and upper must be smaller than 100)";
}


void WidgetDrawable::setScalarFieldClampUpper(double v) {
    auto d = drawable();
    if (d->clamp_lower() * 100 + v < 100) {
        d->set_clamp_upper(v / 100.0f);
        d->update();
        viewer_->update();
    }
    else
        LOG(WARNING) << "invalid clamp range (the sum of lower and upper must be smaller than 100)";
}


void WidgetDrawable::setHighlight(bool b) {
    drawable()->set_highlight(b);
    viewer_->update();
    disableUnavailableOptions();
}


void WidgetDrawable::setHighlightMin(int v) {
    auto d = drawable();
    const auto &range = d->highlight_range();
    d->set_highlight_range(std::make_pair(v, range.second));
    viewer_->update();
}


void WidgetDrawable::setHighlightMax(int v) {
    auto d = drawable();
    const auto &range = d->highlight_range();
    d->set_highlight_range(std::make_pair(range.first, v));
    viewer_->update();
}


void WidgetDrawable::setDistinctBackColor(bool b) {
    drawable()->set_distinct_back_color(b);
    viewer_->update();
    disableUnavailableOptions();
}


void WidgetDrawable::setTextureRepeat(int r) {
    auto d = drawable();
    d->set_texture_repeat(r);
    viewer_->update();
}


void WidgetDrawable::setTextureFractionalRepeat(int r) {
    drawable()->set_texture_fractional_repeat(r);
    viewer_->update();
}


void WidgetDrawable::setVectorFieldScale(double s) {
    auto d = drawable();
    states_[d].vector_field_scale = s;

    auto drawa = viewer_->currentModel()->renderer()->get_lines_drawable("vector - " + states_[d].vector_field.toStdString());
    if (drawa) {
        drawa->update();
        viewer_->update();
    }
}


// get the property name from the color scheme name, i.e., remove scalar_prefix_ substring
std::string WidgetDrawable::color_property_name(const std::string& name, const std::string& scalar_prefix) const {
    std::string property = name;
    if (name.find(scalar_prefix) != std::string::npos) {
        property.erase(0, scalar_prefix.size());
    }
    return property;
}


// get the color source from the color scheme name
State::Method WidgetDrawable::color_method(const std::string& name, const std::string& scalar_prefix) const {
    if (name == "uniform color")
        return State::UNIFORM_COLOR;
    else if (name.find("v:color") != std::string::npos || name.find("e:color") != std::string::npos || name.find("f:color") != std::string::npos)
        return State::COLOR_PROPERTY;
    else if (name.find("texcoord") != std::string::npos)
        return State::TEXTURED;
    else if (name.find(scalar_prefix) != std::string::npos)
        return State::SCALAR_FIELD;
    else {
        LOG(ERROR) << "unknown color source (scheme = " << name << ")";
        return State::UNIFORM_COLOR;
    }
}


// get the color location from the color scheme name
State::Location WidgetDrawable::color_location(const std::string& name) const {
    if (name.find("e:") != std::string::npos)
        return State::EDGE;
    else if (name.find("h:") != std::string::npos)
        return State::HALFEDGE;
    else if (name.find("f:") != std::string::npos)
        return State::FACE;
    else
        return State::VERTEX;
}


void WidgetDrawable::setColorScheme(const QString &text) {
    auto d = drawable();
    auto& state = d->state();

    state.set_coloring(
            color_method(text.toStdString(), scalar_prefix_.toStdString()),
            color_location(text.toStdString()),
            color_property_name(text.toStdString(), scalar_prefix_.toStdString())
    );
    
    if (state.coloring_method() == State::TEXTURED || state.coloring_method() == State::SCALAR_FIELD) {
        state.set_texture(colormapTexture(states_[d].scalar_style, states_[d].discrete_color, states_[d].num_stripes));
        if (state.coloring_method() == State::SCALAR_FIELD) {
            state.set_texture_repeat(1.0f);
            state.set_texture_fractional_repeat(0.0f);
        }
    }

    d->update();

    viewer_->update();
    window_->enableCameraManipulation();

    updatePanel();
}
