#include "widget_scalar_field.h"

#include <unordered_map>
#include <QDir>
#include <QColorDialog>

#include "ui_widget_scalar_field.h"

#include "main_window.h"
#include "paint_canvas.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/types.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>
#include <easy3d/viewer/drawable.h>
#include <easy3d/viewer/setting.h>


using namespace easy3d;

static std::unordered_map<std::string, QString> colormapStyles;

void addColormap(QComboBox* combo, const std::string& baseName) {
    const std::string dir = setting::resource_directory() + "/colormaps/";
    if (file_system::is_file(dir + baseName + ".xpm")) {
       QString fileName = QString::fromStdString(dir + baseName + ".xpm");
       colormapStyles[baseName] = fileName;
       combo->addItem(QIcon(fileName), QString::fromStdString(baseName));
    }
    else
        LOG(WARNING) << "could not find colormap \'" << baseName << "\' in \'" << dir << "\'";
}


WidgetScalarField::WidgetScalarField(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WidgetScalarField)
{
    ui->setupUi(this);
    ui->lineEditScalarFieldMinValue->setValidator(new QDoubleValidator(-100000, 100000, 5, this));
    ui->lineEditScalarFieldMaxValue->setValidator(new QDoubleValidator(-100000, 100000, 5, this));

    viewer_ = dynamic_cast<MainWindow*>(parent)->viewer();

    ui->comboBoxScalarFieldTexture->setObjectName("Colormap Style");
    ui->comboBoxScalarFieldTexture->setEditable(false);

    addColormap(ui->comboBoxScalarFieldTexture, "default");
    addColormap(ui->comboBoxScalarFieldTexture, "rainbow");
    addColormap(ui->comboBoxScalarFieldTexture, "blue_red");
    addColormap(ui->comboBoxScalarFieldTexture, "blue_white");
    addColormap(ui->comboBoxScalarFieldTexture, "blue_yellow");
    addColormap(ui->comboBoxScalarFieldTexture, "black_white");
    addColormap(ui->comboBoxScalarFieldTexture, "ceil");
    addColormap(ui->comboBoxScalarFieldTexture, "rainbow_iso");
    addColormap(ui->comboBoxScalarFieldTexture, "random");

    if (colormapStyles.empty()) {
        ui->comboBoxScalarFieldTexture->addItem("not available");
        LOG(WARNING) << "no colormaps available";
    }
    else {
        connect(ui->comboBoxScalarFieldTexture, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setScalarColormapStyle(const QString&)));
        ui->comboBoxScalarFieldTexture->setCurrentIndex(0);
    }

    ui->comboBoxScalarFieldEdgeStyle->setObjectName("Edge Style");
    ui->comboBoxScalarFieldEdgeStyle->setEditable(false);
    ui->comboBoxScalarFieldEdgeStyle->addItem("sticks");
    ui->comboBoxScalarFieldEdgeStyle->addItem("arrows");
    ui->comboBoxScalarFieldEdgeStyle->addItem("corners");
    ui->comboBoxScalarFieldEdgeStyle->addItem("dual");
    connect(ui->comboBoxScalarFieldEdgeStyle, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setScalarEdgeStyle(const QString&)));
}


WidgetScalarField::~WidgetScalarField()
{
    delete ui;
}


SurfaceMesh* WidgetScalarField::mesh() {
    return dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
}


PointCloud* WidgetScalarField::cloud() {
    return dynamic_cast<PointCloud*>(viewer_->currentModel());
}


template <typename FT, typename MODEL>
void addVertexFields(QComboBox* combo, MODEL* m) {
    auto properties = m->vertex_properties();
    for (auto name : properties) {
        auto property = m->template get_vertex_property<FT>(name);
        if (property) {
            const QString str = QString::fromStdString(name);
            if (combo->findText(str) == -1)
                combo->addItem(str);
        }
    }
}


template <typename FT, typename MODEL>
void addFaceFields(QComboBox* combo, MODEL* m) {
    auto properties = m->face_properties();
    for (auto name : properties) {
        auto property = m->template get_face_property<FT>(name);
        if (property) {
            const QString str = QString::fromStdString(name);
            if (combo->findText(str) == -1)
                combo->addItem(str);
        }
    }
}


template <typename FT, typename MODEL>
void addEdgeFields(QComboBox* combo, MODEL* m) {
    auto properties = m->edge_properties();
    for (auto name : properties) {
        auto property = m->template get_edge_property<FT>(name);
        if (property) {
            const QString str = QString::fromStdString(name);
            if (combo->findText(str) == -1)
                combo->addItem(str);
        }
    }
}


// update the panel to be consistent with the drawable's rendering parameters
void WidgetScalarField::updatePanel() {
    disconnect(ui->comboBoxScalarFieldName, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setScalarField(const QString&)));
    ui->comboBoxScalarFieldName->clear();
    ui->comboBoxScalarFieldName->addItem("Select ...");

    if (mesh()) {
        addFaceFields<unsigned int, SurfaceMesh>(ui->comboBoxScalarFieldName, mesh());
        addFaceFields<int, SurfaceMesh>(ui->comboBoxScalarFieldName, mesh());
        addFaceFields<float, SurfaceMesh>(ui->comboBoxScalarFieldName, mesh());
        addFaceFields<double, SurfaceMesh>(ui->comboBoxScalarFieldName, mesh());

        addVertexFields<unsigned int, SurfaceMesh>(ui->comboBoxScalarFieldName, mesh());
        addVertexFields<int, SurfaceMesh>(ui->comboBoxScalarFieldName, mesh());
        addVertexFields<float, SurfaceMesh>(ui->comboBoxScalarFieldName, mesh());
        addVertexFields<double, SurfaceMesh>(ui->comboBoxScalarFieldName, mesh());

        addEdgeFields<unsigned int, SurfaceMesh>(ui->comboBoxScalarFieldName, mesh());
        addEdgeFields<int, SurfaceMesh>(ui->comboBoxScalarFieldName, mesh());
        addEdgeFields<float, SurfaceMesh>(ui->comboBoxScalarFieldName, mesh());
        addEdgeFields<double, SurfaceMesh>(ui->comboBoxScalarFieldName, mesh());

        // surface
        TrianglesDrawable* drawable = mesh()->triangles_drawable("faces");
        if (drawable) {
        }
        else {
        }
    }
    else if (cloud()) {
        addVertexFields<unsigned int, PointCloud>(ui->comboBoxScalarFieldName, cloud());
        addVertexFields<int, PointCloud>(ui->comboBoxScalarFieldName, cloud());
        addVertexFields<float, PointCloud>(ui->comboBoxScalarFieldName, cloud());
        addVertexFields<double, PointCloud>(ui->comboBoxScalarFieldName, cloud());
    }

    connect(ui->comboBoxScalarFieldName, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setScalarField(const QString&)));
}


void WidgetScalarField::setScalarColormapStyle(const QString& styleName) {
    LOG(INFO) << "using colormap: " << styleName.toStdString();
    viewer_->update();
}


void WidgetScalarField::setScalarEdgeStyle(const QString& style) {
    LOG(INFO) << "edge style: " << style.toStdString();
    viewer_->update();
}


void WidgetScalarField::setScalarField(const QString& field) {
    LOG(INFO) << "scalar filed: " << field.toStdString();
}


//make sure the appropriate rendering data are uploaded to GPU
void WidgetScalarField::ensureBuffers() {
}


