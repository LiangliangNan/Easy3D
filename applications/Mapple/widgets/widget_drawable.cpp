#include "widget_drawable.h"

#include <QColorDialog>
#include <QFileDialog>

#include <easy3d/viewer/drawable.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/viewer/texture.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include "main_window.h"
#include "paint_canvas.h"


using namespace easy3d;


WidgetDrawable::WidgetDrawable(QWidget *parent)
        : QWidget(parent) {
    auto window = dynamic_cast<MainWindow *>(parent);
    viewer_ = window->viewer();
}


WidgetDrawable::~WidgetDrawable() {
}


void WidgetDrawable::setDrawableVisible(bool b) {
    if (!drawable())
        return;

    if (drawable()->is_visible() != b) {
        drawable()->set_visible(b);
        viewer_->update();
    }
}


void WidgetDrawable::setHighlightLow(int v) {
    if (!drawable())
        return;

    const auto &range = drawable()->highlight_range();
    if (range.first != v) {
        drawable()->set_highlight_range(std::make_pair(v, range.second));
        viewer_->update();
    }
}


void WidgetDrawable::setHighlightHigh(int v) {
    if (!drawable())
        return;

    const auto &range = drawable()->highlight_range();
    if (range.second != v) {
        drawable()->set_highlight_range(std::make_pair(range.first, v));
        viewer_->update();
    }
}


void WidgetDrawable::setLighting(bool b) {
    if (!drawable())
        return;

    if (drawable()->lighting() != b) {
        drawable()->set_lighting(b);
        viewer_->update();
    }
}


void WidgetDrawable::setLightingTwoSides(int b) {
    if (!drawable())
        return;

    if (drawable()->lighting_two_sides() != b) {
        drawable()->set_lighting_two_sides(b);
        viewer_->update();
    }
}


void WidgetDrawable::setDefaultColor() {
    const vec3 &c = drawable()->default_color();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    const QColor &color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec3 new_color(color.redF(), color.greenF(), color.blueF());
        drawable()->set_default_color(new_color);
        viewer_->update();
    }
}


void WidgetDrawable::setUseTexture(bool b) {
    if (!drawable())
        return;

    if (drawable()->use_texture() != b) {
        drawable()->set_use_texture(b);
        viewer_->update();
    }
}


void WidgetDrawable::setTextureFile() {
    if (!drawable())
        return;

    const std::string dir = setting::resource_directory() + "/textures/";
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose an image file"), QString::fromStdString(dir),
                                                    tr("Image format (*.xpm *.png *.jpg *.bmp *.ppm)")
    );

    if (fileName.isEmpty())
        return;

    const std::string &file_name = fileName.toStdString();
    Texture *tex = drawable()->texture();
    if (tex) {
        if (tex->file_name() == file_name)
            return;

        delete tex;
    }

    tex = Texture::create(file_name, GL_REPEAT);
    if (tex) {
        drawable()->set_texture(tex);
        viewer_->update();
    } else
        LOG(WARNING) << "failed creating texture from file: " << file_name;
}


void WidgetDrawable::setTextureRepeat(int r) {
    if (!drawable())
        return;

    if (drawable()->texture_repeat() != r) {
        std::cout << "repeat: " << r << std::endl;
        drawable()->set_texture_repeat(r);
        viewer_->update();
    }
}


void WidgetDrawable::setTextureFractionalRepeat(int r) {
    if (!drawable())
        return;

    if (drawable()->texture_fractional_repeat() != r) {
        drawable()->set_texture_fractional_repeat(r);
        viewer_->update();
    }
}
