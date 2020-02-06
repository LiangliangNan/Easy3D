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

    // which drawable
    connect(ui->comboBoxDrawables, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(setActiveDrawable(const QString &)));

    // visible
    connect(ui->checkBoxVisible, SIGNAL(toggled(bool)), this, SLOT(setDrawableVisible(bool)));

    // phong shading
    connect(ui->checkBoxPhongShading, SIGNAL(toggled(bool)), this, SLOT(setPhongShading(bool)));

    // lighting
    connect(ui->checkBoxLighting, SIGNAL(toggled(bool)), this, SLOT(setLighting(bool)));
    connect(ui->comboBoxLightingOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(setLightingTwoSides(int)));

    // color scheme
    connect(ui->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(setColorScheme(const QString &)));

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
    // was there a drawable being active?
    auto pos = active_triangles_drawable_.find(model);
    if (pos == active_triangles_drawable_.end()) {
        if (!drawables.empty()) {
            ui->comboBoxDrawables->setCurrentIndex(0);
            active_triangles_drawable_[model] = drawables[0]->name();
        }
    }
    else {
        const auto& name = active_triangles_drawable_[model];
        auto drawable = model->triangles_drawable(name);
        for (int i=0; i<drawables.size(); ++i) {
            if (drawable == drawables[i]) {
                ui->comboBoxDrawables->setCurrentIndex(i);
                break;
            }
        }
    }

    // visible
    ui->checkBoxVisible->setChecked(activeDrawable()->is_visible());
    // phong shading
    ui->checkBoxPhongShading->setChecked(activeDrawable()->smooth_shading());
    // lighting
    ui->checkBoxLighting->setChecked(activeDrawable()->lighting());
    ui->comboBoxLightingOptions->setCurrentIndex(drawable()->lighting_two_sides() ? 1 : 0);

    // color scheme
    ui->comboBoxColorScheme->clear();
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    ui->comboBoxColorScheme->addItem("uniform color");
    auto vcolors = mesh->get_vertex_property<vec3>("v:color");
    if (vcolors)
        ui->comboBoxColorScheme->addItem("v:color");
    auto fcolors = mesh->get_face_property<vec3>("f:color");
    if (fcolors)
        ui->comboBoxColorScheme->addItem("f:color");
    if (activeDrawable()->per_vertex_color()) {
        const auto& name = activeDrawable()->active_color_property();
        ui->comboBoxColorScheme->setCurrentText(QString::fromStdString(name));
    }
    else {
        ui->comboBoxColorScheme->setCurrentIndex(0);
    }


    // default color
    vec3 c = drawable()->default_color();
    QPixmap pixmap(ui->toolButtonDefaultColor->size());
    pixmap.fill(
            QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
    ui->toolButtonDefaultColor->setIcon(QIcon(pixmap));

    // back side color
    ui->checkBoxDistinctBackColor->setChecked(drawable()->distinct_back_color());
    c = drawable()->back_color();
    pixmap.fill(
            QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
    ui->toolButtonBackColor->setIcon(QIcon(pixmap));

    // texture
    ui->checkBoxTexture->setChecked(drawable()->use_texture());
    if (drawable()->texture()) {
        const auto &file = drawable()->texture()->file_name();
        ui->lineEditTextureFile->setText(QString::fromStdString(file_system::simple_name(file)));
    }
    ui->spinBoxTextureRepeat->setValue(drawable()->texture_repeat());
    ui->spinBoxTextureFractionalRepeat->setValue(drawable()->texture_fractional_repeat());

    const auto &range = drawable()->highlight_range();
    if (range.first == -1 && range.second == -1)
        setHighlight(false);
    else {
        setHighlight(true);
        ui->spinBoxHighlightLow->setValue(range.first);
        ui->spinBoxHighlightHigh->setValue(range.second);
    }
}


Drawable* WidgetTrianglesDrawable::drawable() {
    return activeDrawable();
}


TrianglesDrawable* WidgetTrianglesDrawable::activeDrawable() {
    auto model = viewer_->currentModel();
    if (!model)
        return nullptr;

    const auto& name = ui->comboBoxDrawables->currentText().toStdString();
    return model->triangles_drawable(name);
}


void WidgetTrianglesDrawable::setActiveDrawable(const QString & text) {
    auto model = viewer_->currentModel();
    if (!model)
        return;

    const std::string& name = text.toStdString();
    if (model->triangles_drawable(name)) {
        active_triangles_drawable_[model] = name;
    }
    else {
        LOG(ERROR) << "drawable '" << name << "' not defined on model: " << model->name();
        const auto& drawables = model->triangles_drawables();
        if (drawables.empty())
            LOG(ERROR) << "no triangles drawable defined on model: " << model->name();
        else
            active_triangles_drawable_[model] = drawables[0]->name();
    }
}


void WidgetTrianglesDrawable::setPhongShading(bool b) {
    auto d = activeDrawable();
    if (d) {
        d->set_smooth_shading(b);
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


void WidgetTrianglesDrawable::setBackColor() {
    WidgetDrawable::setBackColor();

    const vec3 &c = drawable()->back_color();
    QColor color(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    QPixmap pixmap(ui->toolButtonBackColor->size());
    pixmap.fill(color);
    ui->toolButtonBackColor->setIcon(QIcon(pixmap));
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


void WidgetTrianglesDrawable::setHighlight(bool b) {
    if (!b) {
        if (ui->spinBoxHighlightLow->value() != -1)
            ui->spinBoxHighlightLow->setValue(-1);
        if (ui->spinBoxHighlightHigh->value() != -1)
            ui->spinBoxHighlightHigh->setValue(-1);
    }
}


void WidgetTrianglesDrawable::setTransparency(int a) {
    //drawable()->set_opacity(a / 100.0f);
}


