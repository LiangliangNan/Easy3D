#include "widget_drawable_lines.h"

#include <QColorDialog>

#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/model.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include "paint_canvas.h"

#include "ui_widget_drawable_lines.h"

//#include "main_window.h"
//#include "paint_canvas.h"

//#include <easy3d/core/surface_mesh.h>
//#include <easy3d/core/types.h>
//#include <easy3d/util/logging.h>
//#include <easy3d/viewer/drawable_points.h>
//#include <easy3d/viewer/drawable_lines.h>

//#include <easy3d/viewer/setting.h>
//#include <easy3d/viewer/renderer.h>


using namespace easy3d;


WidgetLinesDrawable::WidgetLinesDrawable(QWidget *parent)
        : WidgetDrawable(parent)
        , ui(new Ui::WidgetLinesDrawable)
{
    ui->setupUi(this);


//    viewer_ = dynamic_cast<MainWindow*>(parent)->viewer();

//    // phong shading
//    connect(ui->checkBoxPhongShading, SIGNAL(toggled(bool)), this, SLOT(setPhongShading(bool)));

//    // faces
//    connect(ui->checkBoxUseColorProperty, SIGNAL(toggled(bool)), this, SLOT(setUseColorProperty(bool)));
//    connect(ui->checkBoxShowFaces, SIGNAL(toggled(bool)), this, SLOT(setShowFaces(bool)));
//    connect(ui->toolButtonFacesDefaultColor, SIGNAL(clicked()), this, SLOT(setFacesDefaultColor()));
//    connect(ui->horizontalSliderFacesOpacity, SIGNAL(valueChanged(int)), this, SLOT(setFacesOpacity(int)));

//    // vertices
//    connect(ui->checkBoxShowVertices, SIGNAL(toggled(bool)), this, SLOT(setShowVertices(bool)));
//    connect(ui->toolButtonVerticesDefaultColor, SIGNAL(clicked()), this, SLOT(setVerticesDefaultColor()));
//    connect(ui->toolButtonVerticesImpostors, SIGNAL(toggled(bool)), this, SLOT(setVerticesImpostors(bool)));
//    connect(ui->doubleSpinBoxVerticesSize, SIGNAL(valueChanged(double)), this, SLOT(setVerticesSize(double)));

//    // edges
//    connect(ui->checkBoxShowEdges, SIGNAL(toggled(bool)), this, SLOT(setShowEdges(bool)));
//    connect(ui->toolButtonEdgesDefaultColor, SIGNAL(clicked()), this, SLOT(setEdgesDefaultColor()));
//    connect(ui->toolButtonEdgesImpostors, SIGNAL(toggled(bool)), this, SLOT(setEdgesImpostors(bool)));
//    connect(ui->doubleSpinBoxEdgesThickness, SIGNAL(valueChanged(double)), this, SLOT(setEdgesThickness(double)));

//    // border
//    connect(ui->checkBoxShowBorders, SIGNAL(toggled(bool)), this, SLOT(setShowBorders(bool)));
//    connect(ui->toolButtonBordersDefaultColor, SIGNAL(clicked()), this, SLOT(setBordersDefaultColor()));
//    connect(ui->toolButtonBordersImpostors, SIGNAL(toggled(bool)), this, SLOT(setBordersImpostors(bool)));
//    connect(ui->doubleSpinBoxBordersThickness, SIGNAL(valueChanged(double)), this, SLOT(setBordersThickness(double)));


}



WidgetLinesDrawable::~WidgetLinesDrawable() {
    delete ui;
}


Drawable* WidgetLinesDrawable::drawable() {
    return currentDrawable();
}


LinesDrawable* WidgetLinesDrawable::currentDrawable() {
    auto model = viewer_->currentModel();
    if (model) {
        const auto& name = ui->comboBoxDrawables->currentText().toStdString();
        return model->lines_drawable(name);
    }

    return nullptr;
}


void WidgetLinesDrawable::setActiveDrawable(const QString &text) {
    auto model = viewer_->currentModel();
    if (!model)
        return;

    const std::string& name = text.toStdString();
    if (model->lines_drawable(name)) {
        active_lines_drawable_[model] = name;
    }
    else {
        LOG(ERROR) << "drawable '" << name << "' not defined on model: " << model->name();
        const auto& drawables = model->lines_drawables();
        if (drawables.empty())
            LOG(ERROR) << "no lines drawable defined on model: " << model->name();
        else
            active_lines_drawable_[model] = drawables[0]->name();
    }
}


// update the panel to be consistent with the drawable's rendering parameters
void WidgetLinesDrawable::updatePanel() {
    auto model = viewer_->currentModel();
    if (model) {
        const auto& drawables = model->lines_drawables();
        if (!drawables.empty()) {
            setEnabled(true);

            ui->comboBoxDrawables->clear();
            for (auto d : drawables)
                ui->comboBoxDrawables->addItem(QString::fromStdString(d->name()));

            LinesDrawable* d = dynamic_cast<LinesDrawable*>(drawable());
            if (d) {
                ui->checkBoxColorProperty->setChecked(d->per_vertex_color());
                ui->checkBoxVisible->setChecked(d->is_visible());
                const vec3& c = d->default_color();
                QPixmap pixmap(ui->toolButtonDefaultColor->size());
                pixmap.fill(QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
                ui->toolButtonDefaultColor->setIcon(QIcon(pixmap));
            }
            return;
        }
    }

    setEnabled(false);
}


void WidgetLinesDrawable::setLighting(int b) {
//    switch (b) {
//        case 0:
//            drawable()->set_lighting(true);
//            drawable()->set_lighting_two_sides(true);
//            ui->labelDefaultColor->setEnabled(true);
//            ui->labelDefaultColorFront->setEnabled(true);
//            ui->labelDefaultColorBack->setEnabled(true);
//            ui->checkBoxDistinctBackColor->setEnabled(true);
//            ui->toolButtonBackColor->setChecked(true);
//            break;
//        case 1:
//            drawable()->set_lighting(true);
//            drawable()->set_lighting_two_sides(false);
//            ui->labelDefaultColor->setEnabled(true);
//            ui->labelDefaultColorFront->setEnabled(true);
//            ui->labelDefaultColorBack->setEnabled(false);
//            ui->checkBoxDistinctBackColor->setEnabled(false);
//            ui->toolButtonBackColor->setChecked(false);
//            break;
//        case 2:
//            drawable()->set_lighting(false);
//            ui->labelDefaultColor->setEnabled(false);
//            ui->labelDefaultColorFront->setEnabled(false);
//            ui->labelDefaultColorBack->setEnabled(false);
//            ui->checkBoxDistinctBackColor->setEnabled(false);
//            ui->toolButtonBackColor->setChecked(false);
//            break;
//    }
//    viewer_->update();
}


void WidgetLinesDrawable::setDefaultColor() {
    const vec3 &c = drawable()->default_color();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    const QColor &color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec3 new_color(color.redF(), color.greenF(), color.blueF());
        drawable()->set_default_color(new_color);
        QPixmap pixmap(ui->toolButtonDefaultColor->size());
        pixmap.fill(color);
        ui->toolButtonDefaultColor->setIcon(QIcon(pixmap));
    }
}


void WidgetLinesDrawable::setHighlight(bool b) {
    ui->spinBoxHighlightLow->setEnabled(b);
    ui->spinBoxHighlightHigh->setEnabled(b);

    if (!b) {
        ui->spinBoxHighlightLow->setValue(-1);
        ui->spinBoxHighlightHigh->setValue(-1);
    }
}
