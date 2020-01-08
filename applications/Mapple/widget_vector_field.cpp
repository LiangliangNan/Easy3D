#include "widget_vector_field.h"

#include <QColorDialog>

#include "ui_widget_vector_field.h"

#include "main_window.h"
#include "paint_canvas.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/types.h>
#include <easy3d/util/logger.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/setting.h>


using namespace easy3d;


WidgetVectorField::WidgetVectorField(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WidgetVectorField)
{
    ui->setupUi(this);

    viewer_ = dynamic_cast<MainWindow*>(parent)->viewer();
}


WidgetVectorField::~WidgetVectorField()
{
    delete ui;
}


SurfaceMesh* WidgetVectorField::mesh() {
    return dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
}


PointCloud* WidgetVectorField::cloud() {
    return dynamic_cast<PointCloud*>(viewer_->currentModel());
}


template <typename MODEL>
void addVertexFields(QComboBox* combo, MODEL* m) {
    auto properties = m->vertex_properties();
    for (auto name : properties) {
        auto property = m->template get_vertex_property<vec3>(name);
        if (property) {
            const QString str = QString::fromStdString(name);
            if (combo->findText(str) == -1)
                combo->addItem(str);
        }
    }
}


template <typename MODEL>
void addFaceFields(QComboBox* combo, MODEL* m) {
    auto properties = m->face_properties();
    for (auto name : properties) {
        auto property = m->template get_face_property<vec3>(name);
        if (property) {
            const QString str = QString::fromStdString(name);
            if (combo->findText(str) == -1)
                combo->addItem(str);
        }
    }
}


template <typename MODEL>
void addEdgeFields(QComboBox* combo, MODEL* m) {
    auto properties = m->edge_properties();
    for (auto name : properties) {
        auto property = m->template get_edge_property<vec3>(name);
        if (property) {
            const QString str = QString::fromStdString(name);
            if (combo->findText(str) == -1)
                combo->addItem(str);
        }
    }
}


// update the panel to be consistent with the drawable's rendering parameters
void WidgetVectorField::updatePanel() {
    disconnect(ui->comboBoxVectorFieldName, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setVectorField(const QString&)));
    ui->comboBoxVectorFieldName->clear();
    ui->comboBoxVectorFieldName->addItem("Select ...");

    if (mesh()) {
        addFaceFields<SurfaceMesh>(ui->comboBoxVectorFieldName, mesh());
        addVertexFields<SurfaceMesh>(ui->comboBoxVectorFieldName, mesh());
        addEdgeFields<SurfaceMesh>(ui->comboBoxVectorFieldName, mesh());

        // surface
        TrianglesDrawable* surface = mesh()->triangles_drawable("surface");
        if (surface) {
        }
        else {
        }
    }
    else if (cloud()) {
        addVertexFields<PointCloud>(ui->comboBoxVectorFieldName, cloud());
    }

    connect(ui->comboBoxVectorFieldName, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setVectorField(const QString&)));
}


void WidgetVectorField::setVectorField(const QString& field) {
    Logger::info("vector filed: %s", field.toStdString().c_str());
//    LOG(INFO) << "vector filed: " << field.toStdString();
}


//make sure the appropriate rendering data are uploaded to GPU
void WidgetVectorField::ensureBuffers() {
}
