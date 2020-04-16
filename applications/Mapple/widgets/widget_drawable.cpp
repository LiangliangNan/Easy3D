#include "widget_drawable.h"

#include <easy3d/viewer/texture_manager.h>
#include <easy3d/util/file_system.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/viewer/texture_manager.h>

#include "main_window.h"
#include "paint_canvas.h"


using namespace easy3d;


std::vector<WidgetDrawable::ColorMap> WidgetDrawable::colormaps_;


WidgetDrawable::WidgetDrawable(QWidget *parent)
        : QWidget(parent) {
    main_window_ = dynamic_cast<MainWindow *>(parent);
    viewer_ = main_window_->viewer();

    if (colormaps_.empty()) {
        const std::string dir = resource::directory() + "/colormaps/";
        if (file_system::is_file(dir + "default.png"))
            colormaps_.emplace_back(ColorMap(dir + "default.png", "default"));
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
            return TextureManager::request(24, Texture::CLAMP_TO_EDGE, Texture::LINEAR);
    }
    else
        return nullptr;
}
