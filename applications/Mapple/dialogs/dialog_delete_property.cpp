#include "dialogs/dialog_delete_property.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/graph.h>
#include <easy3d/util/file_system.h>

#include "paint_canvas.h"
#include "main_window.h"

#include <algorithm>

#include "ui_dialog_delete_property.h"


static std::vector<std::string> key_words = {
        "v:point", "v:connectivity", "v:deleted", "v:lock",
        "f:connectivity", "f:deleted", "f:triangle_range",
        "e:deleted", "h:connectivity", "f:connectivity"
};


using namespace easy3d;

DialogDeleteProperty::DialogDeleteProperty(QWidget *parent)
        : QDialog(parent), ui(new Ui::DialogDeleteProperty) {
    ui->setupUi(this);

    window_ = dynamic_cast<MainWindow *>(parent);
    viewer_ = window_->viewer();

    connect(ui->comboBoxModels, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(modelChanged(const QString &)));
    connect(ui->comboBoxPropertyLocations, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(locationChanged(const QString &)));
    connect(ui->queryButton, SIGNAL(clicked()), this, SLOT(queryAvailableProperties()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteSelectedProperty()));

    queryAvailableProperties();
}


DialogDeleteProperty::~DialogDeleteProperty() {

}


Model* DialogDeleteProperty::getModel() {
    const std::string& text = ui->comboBoxModels->currentText().toStdString();
    auto &models = viewer_->models();
    for (auto m : models) {
        const std::string &name = file_system::base_name(m->name());
        if (name == text)
            return m;
    }
    return nullptr;
}


void DialogDeleteProperty::modelChanged(const QString &text) {
    Model* model = getModel();
    if (!model)
        return;

    const QString &location_text = ui->comboBoxPropertyLocations->currentText();
    bool location_text_has_match = false;

    disconnect(ui->comboBoxPropertyLocations, SIGNAL(currentIndexChanged(const QString &)), this,
               SLOT(locationChanged(const QString &)));

    ui->comboBoxPropertyLocations->clear();
    if (dynamic_cast<SurfaceMesh *>(model)) {
        ui->comboBoxPropertyLocations->addItem("Vertex");
        ui->comboBoxPropertyLocations->addItem("Face");
        ui->comboBoxPropertyLocations->addItem("Edge");
        ui->comboBoxPropertyLocations->addItem("Halfedge");
        location_text_has_match = true;
    } else if (dynamic_cast<Graph *>(model)) {
        ui->comboBoxPropertyLocations->addItem("Vertex");
        ui->comboBoxPropertyLocations->addItem("Edge");
        if (location_text == "Vertex" || location_text == "Edge")
            location_text_has_match = true;
    } else if (dynamic_cast<PointCloud *>(model)) {
        ui->comboBoxPropertyLocations->addItem("Vertex");
        if (location_text == "Vertex")
            location_text_has_match = true;
    }

    if (location_text_has_match)
        ui->comboBoxPropertyLocations->setCurrentText(location_text);
    else
        ui->comboBoxPropertyLocations->setCurrentIndex(0);

    locationChanged(ui->comboBoxPropertyLocations->currentText());

    connect(ui->comboBoxPropertyLocations, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(locationChanged(const QString &)));
}


void DialogDeleteProperty::locationChanged(const QString &text) {
    Model* model = getModel();
    if (!model)
        return;

    ui->comboBoxPropertyNames->clear();

    const QString& location = ui->comboBoxPropertyLocations->currentText();
    if (dynamic_cast<PointCloud *>(model)) {
        auto cloud = dynamic_cast<PointCloud *>(model);
        if (location == "Vertex") {
            for (const auto &name : cloud->vertex_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyNames->addItem(QString::fromStdString(name));
        }
    } else if (dynamic_cast<Graph *>(model)) {
        auto graph = dynamic_cast<Graph *>(model);
        if (location == "Vertex") {
            for (const auto &name : graph->vertex_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyNames->addItem(QString::fromStdString(name));
        } else if (location == "Edge") {
            for (const auto &name : graph->edge_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyNames->addItem(QString::fromStdString(name));
        }
    } else if (dynamic_cast<SurfaceMesh *>(model)) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (location == "Vertex") {
            for (const auto &name : mesh->vertex_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyNames->addItem(QString::fromStdString(name));
        } else if (location == "Edge") {
            for (const auto &name : mesh->edge_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyNames->addItem(QString::fromStdString(name));
        } else if (location == "Face") {
            for (const auto &name : mesh->face_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyNames->addItem(QString::fromStdString(name));
        } else if (location == "Halfedge") {
            for (const auto &name : mesh->halfedge_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyNames->addItem(QString::fromStdString(name));
        }
    }
}


void DialogDeleteProperty::queryAvailableProperties() {
    const QString &model_text = ui->comboBoxModels->currentText();
    bool model_text_has_match = false;

    disconnect(ui->comboBoxModels, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(modelChanged(const QString &)));
    ui->comboBoxModels->clear();
    auto &models = viewer_->models();
    for (auto m : models) {
        const std::string &name = file_system::base_name(m->name());
        const QString str = QString::fromStdString(name);
        ui->comboBoxModels->addItem(str);
        if (str == model_text)
            model_text_has_match = true;
    }

    if (model_text_has_match) {
        ui->comboBoxModels->setCurrentText(model_text);
        modelChanged(ui->comboBoxModels->currentText());
    }
    else {
        auto model = viewer_->currentModel();
        if (model) {
            const std::string &name = file_system::base_name(model->name());
            ui->comboBoxModels->setCurrentText(QString::fromStdString(name));
            modelChanged(ui->comboBoxModels->currentText());
        }
    }

    connect(ui->comboBoxModels, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(modelChanged(const QString &)));
}


void DialogDeleteProperty::deleteSelectedProperty() {
    Model* model = getModel();
    if (!model)
        return;

    const QString& location = ui->comboBoxPropertyLocations->currentText();
    const std::string& property = ui->comboBoxPropertyNames->currentText().toStdString();
    if (property.empty())
        return;

    bool succeed = false;
    if (dynamic_cast<PointCloud *>(model)) {
        auto cloud = dynamic_cast<PointCloud *>(model);
        if (location == "Vertex") {
            if (cloud->remove_vertex_property(property)) {
                succeed = true;
                LOG(INFO) << "vertex property '" << property << "' successfully removed";
            }
            else
                LOG(WARNING) << "failed removing vertex property '" << property << "'";
        }
    }

    else if (dynamic_cast<Graph *>(model)) {
        auto graph = dynamic_cast<Graph *>(model);
        if (location == "Vertex") {
            if (graph->remove_vertex_property(property)) {
                succeed = true;
                LOG(INFO) << "vertex property '" << property << "' successfully removed";
            }
            else
                LOG(WARNING) << "failed removing vertex property '" << property << "'";
        }
        else if (location == "Edge") {
            if (graph->remove_edge_property(property)) {
                succeed = true;
                LOG(INFO) << "edge property '" << property << "' successfully removed";
            }
            else
                LOG(WARNING) << "failed removing edge property '" << property << "'";
        }
    }

    else if (dynamic_cast<SurfaceMesh *>(model)) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (location == "Vertex") {
            if (mesh->remove_vertex_property(property)) {
                succeed = true;
                LOG(INFO) << "vertex property '" << property << "' successfully removed";
            }
            else
                LOG(WARNING) << "failed removing vertex property '" << property << "'";
        }
        else if (location == "Edge") {
            if (mesh->remove_edge_property(property)) {
                succeed = true;
                LOG(INFO) << "edge property '" << property << "' successfully removed";
            }
            else
                LOG(WARNING) << "failed removing edge property '" << property << "'";
        }
        else if (location == "Face") {
            if (mesh->remove_face_property(property)) {
                succeed = true;
                LOG(INFO) << "face property '" << property << "' successfully removed";
            }
            else
                LOG(WARNING) << "failed removing face property '" << property << "'";
        }
        else if (location == "Halfedge") {
            if (mesh->remove_halfedge_property(property)) {
                succeed = true;
                LOG(INFO) << "halfedge property '" << property << "' successfully removed";
            }
            else
                LOG(WARNING) << "failed removing halfedge property '" << property << "'";
        }
    }

    queryAvailableProperties();
    if (succeed)
        window_->updateRenderingPanel();
}