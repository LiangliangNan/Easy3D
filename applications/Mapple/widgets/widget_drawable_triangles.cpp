#include "widget_drawable_triangles.h"

#include <QColorDialog>

#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/model.h>
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

    // default color
    connect(ui->toolButtonDefaultColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

    // color property
    connect(ui->checkBoxColorProperty, SIGNAL(toggled(bool)), this, SLOT(setUseColorProperty(bool)));

    // texture
    connect(ui->checkBoxTexture, SIGNAL(toggled(bool)), this, SLOT(setUseTexture(bool)));
    connect(ui->toolButtonTextureFile, SIGNAL(clicked()), this, SLOT(setTextureFile()));
    connect(ui->spinBoxTextureRepeat, SIGNAL(valueChanged(int)), this, SLOT(setTextureRepeat(int)));
    connect(ui->spinBoxTextureFractionalRepeat, SIGNAL(valueChanged(int)), this, SLOT(setTextureFractionalRepeat(int)));

    // lighting
    connect(ui->checkBoxLighting, SIGNAL(toggled(bool)), this, SLOT(setLighting(bool)));
    connect(ui->comboBoxLightingOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(setLightingTwoSides(int)));

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


// update the panel to be consistent with the drawable's rendering parameters
void WidgetTrianglesDrawable::updatePanel() {
    auto model = viewer_->currentModel();
    if (model) {
        const auto& drawables = model->triangles_drawables();
        if (!drawables.empty()) {
            setEnabled(true);

            ui->comboBoxDrawables->clear();
            for (auto d : drawables)
                ui->comboBoxDrawables->addItem(QString::fromStdString(d->name()));

            TrianglesDrawable* d = dynamic_cast<TrianglesDrawable*>(drawable());
            if (d) {
                ui->checkBoxVisible->setChecked(d->is_visible());
                ui->checkBoxPhongShading->setChecked(d->smooth_shading());

                const vec3& c = d->default_color();
                QPixmap pixmap(ui->toolButtonDefaultColor->size());
                pixmap.fill(QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
                ui->toolButtonDefaultColor->setIcon(QIcon(pixmap));

                ui->checkBoxColorProperty->setChecked(d->per_vertex_color());

                ui->checkBoxTexture->setChecked(d->texture());

                ui->checkBoxLighting->setChecked(d->lighting());
                ui->comboBoxLightingOptions->setCurrentIndex(drawable()->lighting_two_sides() ? 1 : 0);
            }


            const auto& range = d->highlight_range();
            if (range.first == -1 && range.second == -1)
                setHighlight(false);
            else {
                setHighlight(true);
                ui->spinBoxHighlightLow->setValue(range.first);
                ui->spinBoxHighlightHigh->setValue(range.second);
            }

            return;
        }
    }

    setEnabled(false);
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
            ui->comboBoxColorProperty->clear();

            auto vcolors = model->get_vertex_property<vec3>("v:color");
            if (vcolors)
                ui->comboBoxColorProperty->addItem("v:color");

            auto fcolors = model->get_face_property<vec3>("f:color");
            if (fcolors)
                ui->comboBoxColorProperty->addItem("f:color");

            std::cout << "if multiple color properties exist, choose one and update the color buffer" << std::endl;
            drawable()->set_per_vertex_color(false);

            if (!vcolors && !fcolors) {
                LOG(WARNING) << "no color property defined on vertices/faces";
                ui->checkBoxColorProperty->setChecked(false);
            }
        }
    }
    else {
        drawable()->set_per_vertex_color(false);
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


void WidgetTrianglesDrawable::setTransparency(int a) {
    //drawable()->set_opacity(a / 100.0f);
}


void WidgetTrianglesDrawable::setUseTexture(bool b) {
    if (!drawable())
        return;

    if (b && drawable()->texcoord_buffer() == 0) {
        LOG(WARNING) << "texcoord buffer does not exist (request ignored)";
        ui->checkBoxTexture->setChecked(false);
        return;
    }

    WidgetDrawable::setUseTexture(b);

    ui->labelTextureFile->setEnabled(b);
    ui->labelTextureRepeat->setEnabled(b);
    ui->lineEditTextureFile->setEnabled(b);
    ui->toolButtonTextureFile->setEnabled(b);
    ui->spinBoxTextureRepeat->setEnabled(b);
    ui->spinBoxTextureFractionalRepeat->setEnabled(b);
}
