#include "widget_drawable.h"

#include <easy3d/viewer/texture.h>
#include <easy3d/util/file_system.h>
#include <easy3d/fileio/resources.h>

#include "main_window.h"
#include "paint_canvas.h"


using namespace easy3d;


std::vector<WidgetDrawable::ColorMap> WidgetDrawable::colormap_files_;


WidgetDrawable::WidgetDrawable(QWidget *parent)
        : QWidget(parent) {
    auto window = dynamic_cast<MainWindow *>(parent);
    viewer_ = window->viewer();

    if (colormap_files_.empty()) {
        const std::string dir = resource::directory() + "/colormaps/";
        if (file_system::is_file(dir + "default.png"))  colormap_files_.emplace_back(ColorMap(dir + "default.png", "  default"));
        if (file_system::is_file(dir + "rainbow.png"))  colormap_files_.emplace_back(ColorMap(dir + "rainbow.png", "  rainbow"));
        if (file_system::is_file(dir + "blue_red.png"))  colormap_files_.emplace_back(ColorMap(dir + "blue_red.png", "  blue_red"));
        if (file_system::is_file(dir + "blue_white.png"))  colormap_files_.emplace_back(ColorMap(dir + "blue_white.png", "  blue_white"));
        if (file_system::is_file(dir + "blue_yellow.png"))  colormap_files_.emplace_back(ColorMap(dir + "blue_yellow.png", "  blue_yellow"));
        if (file_system::is_file(dir + "black_white.png"))  colormap_files_.emplace_back(ColorMap(dir + "black_white.png", "  black_white"));
        if (file_system::is_file(dir + "ceil.png"))  colormap_files_.emplace_back(ColorMap(dir + "ceil.png", "  ceil"));
        if (file_system::is_file(dir + "random.png"))  colormap_files_.emplace_back(ColorMap(dir + "random.png", "  random"));
    }
}


WidgetDrawable::~WidgetDrawable() {
}


Texture* WidgetDrawable::createColormapTexture(const QString& name) const {
    const std::string colormap = name.toStdString();
    for (const auto& info : colormap_files_) {
        if (info.name == colormap) {
            return Texture::create(info.file, GL_CLAMP_TO_EDGE, GL_LINEAR);
        }
    }
    return nullptr;
}
