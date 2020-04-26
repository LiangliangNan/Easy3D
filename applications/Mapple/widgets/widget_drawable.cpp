#include "widget_drawable.h"

#include <easy3d/viewer/texture_manager.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/model.h>
#include <easy3d/util/file_system.h>
#include <easy3d/fileio/resources.h>

#include "main_window.h"
#include "paint_canvas.h"


using namespace easy3d;


std::vector<WidgetDrawable::ColorMap> WidgetDrawable::colormaps_;

WidgetDrawable::WidgetDrawable(QWidget *parent)
        : QWidget(parent), scalar_prefix_("scalar - ") {
    main_window_ = dynamic_cast<MainWindow *>(parent);
    viewer_ = main_window_->viewer();

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
        if (file_system::is_file(dir + "random.png")) {
            colormaps_.emplace_back(ColorMap(dir + "random.png", "random-32"));
            colormaps_.emplace_back(ColorMap(dir + "random.png", "random-64"));
        }
    }
}


WidgetDrawable::~WidgetDrawable() {
    if (!colormaps_.empty()) {
        for (const auto &colormap : colormaps_) {
            if (colormap.texture)
                delete colormap.texture;
        }
        colormaps_.clear();
    }
}


Texture *WidgetDrawable::colormapTexture(int idx) const {
    if (idx < colormaps_.size()) {
        if (colormaps_[idx].name.find("random") == std::string::npos)
            return TextureManager::request(colormaps_[idx].file, Texture::CLAMP_TO_EDGE, Texture::LINEAR);
        else {
            int num_colors = 16;
            if (colormaps_[idx].name.find("32") != std::string::npos)
                num_colors = 32;
            else if (colormaps_[idx].name.find("64") != std::string::npos)
                num_colors = 64;
            else if (colormaps_[idx].name.find("128") != std::string::npos)
                num_colors = 128;
            return TextureManager::request(num_colors, Texture::CLAMP_TO_EDGE, Texture::LINEAR);
        }
    } else
        return nullptr;
}


void WidgetDrawable::setDrawableVisible(bool b) {
    drawable()->set_visible(b);
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
    auto tex = colormapTexture(idx);
    d->set_texture(tex);
    viewer_->update();
    states_[d].scalar_style = idx;
}


void WidgetDrawable::setScalarFieldClamp(bool b) {
    auto d = drawable();
    d->set_clamp_range(b);
    d->update_buffers();
    viewer_->update();
    disableUnavailableOptions();
}


void WidgetDrawable::setScalarFieldClampLower(double v) {
    auto d = drawable();
    d->set_clamp_lower(v / 100.0f);
    d->update_buffers();
    viewer_->update();
}


void WidgetDrawable::setScalarFieldClampUpper(double v) {
    auto d = drawable();
    d->set_clamp_upper(v / 100.0f);
    d->update_buffers();
    viewer_->update();
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

    auto drawa = viewer_->currentModel()->get_lines_drawable("vector - " + states_[d].vector_field.toStdString());
    if (drawa)
        drawa->update_buffers();
    viewer_->update();
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
    else if (name.find("v:color") != std::string::npos || name.find("f:color") != std::string::npos)
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
State::PropertyLocation WidgetDrawable::color_location(const std::string& name) const {
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
        state.set_texture(colormapTexture(states_[d].scalar_style));
        if (state.coloring_method() == State::SCALAR_FIELD) {
            state.set_texture_repeat(1.0f);
            state.set_texture_fractional_repeat(0.0f);
        }
    }

    d->update_buffers();

    viewer_->update();

    updatePanel();
}
