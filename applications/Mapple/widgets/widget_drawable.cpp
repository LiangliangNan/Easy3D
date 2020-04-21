#include "widget_drawable.h"

#include <easy3d/viewer/texture_manager.h>
#include <easy3d/viewer/renderer.h>
#include <easy3d/viewer/drawable.h>
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
        if (file_system::is_file(dir + "ceil.png"))
            colormaps_.emplace_back(ColorMap(dir + "ceil.png", "ceil"));
        if (file_system::is_file(dir + "random.png"))
            colormaps_.emplace_back(ColorMap(dir + "random.png", "random"));
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
        else
            return TextureManager::request(12, Texture::CLAMP_TO_EDGE, Texture::LINEAR);
    } else
        return nullptr;
}


void WidgetDrawable::setDrawableVisible(bool b) {
    if (drawable()->is_visible() != b) {
        drawable()->set_visible(b);
        viewer_->update();
    }
    disableUnavailableOptions();
}


void WidgetDrawable::setLighting(const QString &text) {
    if (text == "front and back") {
        drawable()->set_lighting(true);
        drawable()->set_lighting_two_sides(true);
    } else if (text == "front only") {
        drawable()->set_lighting(true);
        drawable()->set_lighting_two_sides(false);
    } else if (text == "disabled") {
        drawable()->set_lighting(false);
    }

    viewer_->update();
    disableUnavailableOptions();
}


void WidgetDrawable::setScalarFieldStyle(int idx) {
    states_[drawable()].scalar_style = idx;
    auto tex = colormapTexture(idx);
    drawable()->set_texture(tex);
    drawable()->set_use_texture(true);
    viewer_->update();
}


void WidgetDrawable::setScalarFieldClamp(bool b) {
    auto& scheme = drawable()->color_scheme();
    scheme.clamp_value = b;
    renderer::update_buffer(viewer_->currentModel(), drawable());
    viewer_->update();
}


void WidgetDrawable::setScalarFieldClampLower(double v) {
    auto& scheme = drawable()->color_scheme();
    scheme.dummy_lower = v / 100.0f;
    renderer::update_buffer(viewer_->currentModel(), drawable());
    viewer_->update();
}


void WidgetDrawable::setScalarFieldClampUpper(double v) {
    auto& scheme = drawable()->color_scheme();
    scheme.dummy_upper = v / 100.0f;
    renderer::update_buffer(viewer_->currentModel(), drawable());
    viewer_->update();
}


void WidgetDrawable::setHighlight(bool b) {
    if (drawable()->highlight() != b) {
        drawable()->set_highlight(b);
        viewer_->update();
        disableUnavailableOptions();
    }
}


void WidgetDrawable::setHighlightMin(int v) {
    const auto &range = drawable()->highlight_range();
    if (range.first != v) {
        drawable()->set_highlight_range(std::make_pair(v, range.second));
        viewer_->update();
    }
}


void WidgetDrawable::setHighlightMax(int v) {
    const auto &range = drawable()->highlight_range();
    if (range.second != v) {
        drawable()->set_highlight_range(std::make_pair(range.first, v));
        viewer_->update();
    }
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
    auto d = drawable();
    d->set_texture_fractional_repeat(r);
    viewer_->update();
}


void WidgetDrawable::setVectorFieldScale(double s) {
    auto d = drawable();
    const QString &name = states_[d].vector_field;
    updateVectorFieldBuffer(viewer_->currentModel(), name.toStdString());

    viewer_->update();
    states_[d].vector_field_scale = s;
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
ColorScheme::Source WidgetDrawable::color_source(const std::string& name, const std::string& scalar_prefix) const {
    if (name == "uniform color")
        return ColorScheme::UNIFORM_COLOR;
    else if (name == "v:color")
        return ColorScheme::COLOR_PROPERTY;
    else if (name.find("texcoord") != std::string::npos)
        return ColorScheme::TEXTURE;
    else if (name.find(scalar_prefix) != std::string::npos)
        return ColorScheme::SCALAR_FIELD;
    else {
        LOG(ERROR) << "unknown color source (scheme = " << name << ")";
        return ColorScheme::UNIFORM_COLOR;
    }
}


// get the color location from the color scheme name
ColorScheme::Location WidgetDrawable::color_location(const std::string& name) const {
    if (name.find("e:") != std::string::npos)
        return ColorScheme::EDGE;
    else if (name.find("h:") != std::string::npos)
        return ColorScheme::HALFEDGE;
    else if (name.find("f:") != std::string::npos)
        return ColorScheme::FACE;
    else
        return ColorScheme::VERTEX;
}


void WidgetDrawable::setColorScheme(const QString &text) {
    auto d = drawable();
    auto& scheme = d->color_scheme();

    scheme.source = color_source(text.toStdString(), scalar_prefix_.toStdString());
    scheme.location = color_location(text.toStdString());
    scheme.name = color_property_name(text.toStdString(), scalar_prefix_.toStdString());

    viewer_->makeCurrent();
    renderer::update_buffer(viewer_->currentModel(), d);
    if (scheme.source == ColorScheme::TEXTURE || scheme.source == ColorScheme::SCALAR_FIELD)
        d->set_texture(colormapTexture(states_[d].scalar_style));
    else {
        d->set_texture(nullptr);
        d->set_use_texture(false);
        d->set_per_vertex_color(false);
    }
    viewer_->doneCurrent();

    viewer_->update();
    disableUnavailableOptions();
}