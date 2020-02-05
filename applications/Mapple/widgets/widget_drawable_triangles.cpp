#include "widget_drawable_triangles.h"

#include <QColorDialog>

#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/model.h>
#include <easy3d/viewer/texture.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include "paint_canvas.h"

#include "ui_widget_drawable_triangles.h"


using namespace easy3d;

WidgetTrianglesDrawable::WidgetTrianglesDrawable(QWidget *parent)
        : WidgetDrawable(parent)
        , ui(new Ui::WidgetTrianglesDrawable)
{
    ui->setupUi(this);

    // visible
    connect(ui->checkBoxVisible, SIGNAL(toggled(bool)), this, SLOT(setDrawableVisible(bool)));

    // phong shading
    connect(ui->checkBoxPhongShading, SIGNAL(toggled(bool)), this, SLOT(setPhongShading(bool)));

    // lighting
    connect(ui->checkBoxLighting, SIGNAL(toggled(bool)), this, SLOT(setLighting(bool)));
    connect(ui->comboBoxLightingOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(setLightingTwoSides(int)));

    // color property
    connect(ui->checkBoxColorProperty, SIGNAL(toggled(bool)), this, SLOT(setUseColorProperty(bool)));

    // default color
    connect(ui->toolButtonDefaultColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

    // default color
    connect(ui->checkBoxDistinctBackColor, SIGNAL(toggled(bool)), this, SLOT(setDistinctBackColor(bool)));
    connect(ui->toolButtonBackColor, SIGNAL(clicked()), this, SLOT(setBackColor()));

    // texture
    connect(ui->checkBoxTexture, SIGNAL(toggled(bool)), this, SLOT(setUseTexture(bool)));
    connect(ui->toolButtonTextureFile, SIGNAL(clicked()), this, SLOT(setTextureFile()));
    connect(ui->spinBoxTextureRepeat, SIGNAL(valueChanged(int)), this, SLOT(setTextureRepeat(int)));
    connect(ui->spinBoxTextureFractionalRepeat, SIGNAL(valueChanged(int)), this, SLOT(setTextureFractionalRepeat(int)));


    // transparency
    connect(ui->horizontalSliderTransparency, SIGNAL(valueChanged(int)), this, SLOT(setFacesOpacity(int)));

    // transparency
    connect(ui->checkBoxHighlight, SIGNAL(toggled(bool)), this, SLOT(setHighlight(bool)));
    connect(ui->spinBoxHighlightLow, SIGNAL(valueChanged(int)), this, SLOT(setHighlightLow(int)));
    connect(ui->spinBoxHighlightHigh, SIGNAL(valueChanged(int)), this, SLOT(setHighlightHigh(int)));
}


WidgetTrianglesDrawable::~WidgetTrianglesDrawable() {
    delete ui;
}


// update the panel to be consistent with the drawable's rendering parameters
void WidgetTrianglesDrawable::updatePanel() {
    auto model = viewer_->currentModel();
    if (!model || model->triangles_drawables().empty()) {
        setEnabled(false);
        return;
    }

    setEnabled(true);

    ui->comboBoxDrawables->clear();
    const auto& drawables = model->triangles_drawables();
    for (auto d : drawables)
        ui->comboBoxDrawables->addItem(QString::fromStdString(d->name()));

    TrianglesDrawable *d = dynamic_cast<TrianglesDrawable *>(drawable());
    if (d) {
        // visible
        ui->checkBoxVisible->setChecked(d->is_visible());
        // phong shading
        ui->checkBoxPhongShading->setChecked(d->smooth_shading());
        // lighting
        ui->checkBoxLighting->setChecked(d->lighting());
        ui->comboBoxLightingOptions->setCurrentIndex(drawable()->lighting_two_sides() ? 1 : 0);

        // color property
        auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
        if (mesh) {
            ui->comboBoxColorProperty->clear();
            auto vcolors = mesh->get_vertex_property<vec3>("v:color");
            if (vcolors)
                ui->comboBoxColorProperty->addItem("v:color");
            auto fcolors = mesh->get_face_property<vec3>("f:color");
            if (fcolors)
                ui->comboBoxColorProperty->addItem("f:color");
        }
        bool use_color_property = d->per_vertex_color() && ui->comboBoxColorProperty->count() > 0;
        ui->checkBoxColorProperty->setChecked(use_color_property);

        // default color
        vec3 c = d->default_color();
        QPixmap pixmap(ui->toolButtonDefaultColor->size());
        pixmap.fill(
                QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonDefaultColor->setIcon(QIcon(pixmap));

        // back side color
        ui->checkBoxDistinctBackColor->setChecked(d->distinct_back_color());
        c = d->back_color();
        pixmap.fill(
                QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonBackColor->setIcon(QIcon(pixmap));
        bool back_color_valid = ui->checkBoxLighting->isChecked() && (ui->comboBoxLightingOptions->currentIndex() == 1);
        ui->labelBackColor->setEnabled(back_color_valid);
        ui->checkBoxDistinctBackColor->setEnabled(back_color_valid);
        ui->toolButtonBackColor->setEnabled(back_color_valid);

        // texture
        ui->checkBoxTexture->setChecked(d->use_texture());
        if (d->texture()) {
            const auto &file = d->texture()->file_name();
            ui->lineEditTextureFile->setText(QString::fromStdString(file_system::simple_name(file)));
        }
        ui->spinBoxTextureRepeat->setValue(d->texture_repeat());
        ui->spinBoxTextureFractionalRepeat->setValue(d->texture_fractional_repeat());


        if (ui->checkBoxLighting->isChecked() && ui->comboBoxLightingOptions->currentIndex() == 1) {
            const vec3 &c = d->back_color();
            pixmap.fill(
                    QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
            ui->toolButtonBackColor->setIcon(QIcon(pixmap));
            ui->labelBackColor->setEnabled(true);
            ui->toolButtonBackColor->setEnabled(true);
        } else {
            ui->labelBackColor->setEnabled(false);
            ui->toolButtonBackColor->setEnabled(false);
        }
    }

    const auto &range = d->highlight_range();
    if (range.first == -1 && range.second == -1)
        setHighlight(false);
    else {
        setHighlight(true);
        ui->spinBoxHighlightLow->setValue(range.first);
        ui->spinBoxHighlightHigh->setValue(range.second);
    }
}


Drawable* WidgetTrianglesDrawable::drawable() {
    return currentDrawable();
}


TrianglesDrawable* WidgetTrianglesDrawable::currentDrawable() {
    auto model = viewer_->currentModel();
    if (model) {
        const auto& name = ui->comboBoxDrawables->currentText().toStdString();
        return model->triangles_drawable(name);
    }

    return nullptr;
}


void WidgetTrianglesDrawable::setPhongShading(bool b) {
    auto d = currentDrawable();
    if (d) {
        d->set_smooth_shading(b);
        viewer_->update();
    }
}


void WidgetTrianglesDrawable::setUseColorProperty(bool b) {
    if (b) { // just check and warn user if color property doesn't exist
        auto model = dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
        if (model) {
            auto vcolors = model->get_vertex_property<vec3>("v:color");
            auto fcolors = model->get_face_property<vec3>("f:color");
            if (vcolors || fcolors) {
                LOG_IF(WARNING, vcolors && fcolors) << "TODO: choose a color property for rendering";
                drawable()->set_per_vertex_color(true);
                ui->comboBoxColorProperty->setEnabled(true);
                viewer_->update();
            }
            else {
                LOG(WARNING) << "no color property defined on vertices/faces";
                ui->checkBoxColorProperty->setChecked(false);
                ui->comboBoxColorProperty->setEnabled(false);
            }
        }
    }
    else {
        drawable()->set_per_vertex_color(false);
        ui->comboBoxColorProperty->setEnabled(false);
        viewer_->update();
    }
}


void WidgetTrianglesDrawable::setDefaultColor() {
    WidgetDrawable::setDefaultColor();

    const vec3 &c = drawable()->default_color();
    QColor color(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    QPixmap pixmap(ui->toolButtonDefaultColor->size());
    pixmap.fill(color);
    ui->toolButtonDefaultColor->setIcon(QIcon(pixmap));
}


void WidgetTrianglesDrawable::setHighlight(bool b) {
    ui->spinBoxHighlightLow->setEnabled(b);
    ui->spinBoxHighlightHigh->setEnabled(b);

    if (!b) {
        if (ui->spinBoxHighlightLow->value() != -1)
            ui->spinBoxHighlightLow->setValue(-1);
        if (ui->spinBoxHighlightHigh->value() != -1)
            ui->spinBoxHighlightHigh->setValue(-1);
    }
}


void WidgetTrianglesDrawable::setLighting(bool b) {
    if (drawable()) {
        ui->comboBoxLightingOptions->setCurrentIndex(drawable()->lighting_two_sides() ? 1 : 0);
        ui->comboBoxLightingOptions->setEnabled(b);
    }

    WidgetDrawable::setLighting(b);
}


void WidgetTrianglesDrawable::setLightingTwoSides(int b) {
    if (drawable()) {
        ui->labelBackColor->setEnabled(b);
        ui->checkBoxDistinctBackColor->setEnabled(b);
        ui->toolButtonBackColor->setEnabled(b);
    }

    WidgetDrawable::setLightingTwoSides(b);
}


void WidgetTrianglesDrawable::setBackColor() {
    WidgetDrawable::setBackColor();

    const vec3 &c = drawable()->back_color();
    QColor color(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    QPixmap pixmap(ui->toolButtonBackColor->size());
    pixmap.fill(color);
    ui->toolButtonBackColor->setIcon(QIcon(pixmap));
}


void WidgetTrianglesDrawable::setTransparency(int a) {
    //drawable()->set_opacity(a / 100.0f);
}


void WidgetTrianglesDrawable::setUseTexture(bool b) {
    if (!drawable())
        return;

    bool use_texture = b && (drawable()->texcoord_buffer() != 0);
    WidgetDrawable::setUseTexture(use_texture);

    LOG_IF(WARNING, b && (drawable()->texcoord_buffer() == 0)) << "texcoord buffer does not exist (request ignored)";
    ui->checkBoxTexture->setChecked(use_texture);
    ui->comboBoxTexture->setEnabled(use_texture);
    ui->labelTextureFile->setEnabled(use_texture);
    ui->labelTextureRepeat->setEnabled(use_texture);
    ui->lineEditTextureFile->setEnabled(use_texture);
    ui->toolButtonTextureFile->setEnabled(use_texture);
    ui->spinBoxTextureRepeat->setEnabled(use_texture);
    ui->spinBoxTextureFractionalRepeat->setEnabled(use_texture);
}


void WidgetTrianglesDrawable::setTextureFile() {
    WidgetDrawable::setTextureFile();

    if (drawable()) {
        auto tex = drawable()->texture();
        if (tex) {
            const auto& file = tex->file_name();
            ui->lineEditTextureFile->setText(QString::fromStdString(file_system::simple_name(file)));
        }
    }
}