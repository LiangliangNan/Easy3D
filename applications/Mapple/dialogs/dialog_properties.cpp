#include "dialogs/dialog_properties.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/graph.h>
#include <easy3d/util/file_system.h>

#include "paint_canvas.h"
#include "main_window.h"

#include <algorithm>
#include <QDockWidget>

#include "ui_dialog_properties.h"


static std::vector<std::string> key_words = {
        "v:point", "v:connectivity", "v:deleted", "v:lock",
        "f:connectivity", "f:deleted", "f:triangle_range",
        "e:deleted", "h:connectivity", "f:connectivity"
};


using namespace easy3d;

DialogProperties::DialogProperties(MainWindow *window, QDockWidget* dockWidgetCommand)
        : Dialog(window, dockWidgetCommand), ui(new Ui::DialogProperties) {
    ui->setupUi(this);

    ui->comboBoxCommands->addItem("Remove");
    ui->comboBoxCommands->addItem("Rename");
    ui->comboBoxCommands->addItem("Convert Data Type");
    ui->comboBoxCommands->addItem("Split (Vector -> Scalars)");
    ui->comboBoxCommands->addItem("Merge (Scalars -> Vector)");
    ui->comboBoxCommands->setCurrentIndex(0);

    QMargins margins = ui->lineEditNewPropertyName->textMargins();
    margins.setLeft(7);
    ui->lineEditNewPropertyName->setTextMargins(margins);

    connect(ui->comboBoxCommands, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(commandChanged(const QString &)));
    connect(ui->comboBoxModels, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(modelChanged(const QString &)));
    connect(ui->comboBoxPropertyLocations, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(locationChanged(const QString &)));
    connect(ui->updateButton, SIGNAL(clicked()), this, SLOT(updateProperties()));
    connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(applyCommand()));

    commandChanged(ui->comboBoxCommands->currentText());
}


DialogProperties::~DialogProperties() {

}


void DialogProperties::commandChanged(const QString &) {
    const QString& command = ui->comboBoxCommands->currentText();
    if (command == "Remove") {
        ui->labelPropertyName_1->setText("Property");
        ui->labelPropertyName_2->setVisible(false);
        ui->labelPropertyName_3->setVisible(false);
        ui->comboBoxPropertyNames_2->setVisible(false);
        ui->comboBoxPropertyNames_3->setVisible(false);
        ui->labelNewPropertyName->setText("New name");
        ui->labelNewPropertyName->setVisible(false);
        ui->lineEditNewPropertyName->setVisible(false);
        ui->labelDataType->setVisible(false);
        ui->comboBoxSourceType->setVisible(false);
        ui->labelPropertyTo->setVisible(false);
        ui->comboBoxTargetType->setVisible(false);
    }
    else if (command == "Rename") {
        ui->labelPropertyName_1->setText("Property");
        ui->labelPropertyName_2->setVisible(false);
        ui->labelPropertyName_3->setVisible(false);
        ui->comboBoxPropertyNames_2->setVisible(false);
        ui->comboBoxPropertyNames_3->setVisible(false);
        ui->labelNewPropertyName->setText("New name");
        ui->labelNewPropertyName->setVisible(true);
        ui->lineEditNewPropertyName->setVisible(true);
        ui->labelDataType->setVisible(false);
        ui->comboBoxSourceType->setVisible(false);
        ui->labelPropertyTo->setVisible(false);
        ui->comboBoxTargetType->setVisible(false);
    }
    else if (command == "Convert Data Type") {
        ui->labelPropertyName_1->setText("Property");
        ui->labelPropertyName_2->setVisible(false);
        ui->labelPropertyName_3->setVisible(false);
        ui->comboBoxPropertyNames_2->setVisible(false);
        ui->comboBoxPropertyNames_3->setVisible(false);
        ui->labelNewPropertyName->setText("New name");
        ui->labelNewPropertyName->setVisible(false);
        ui->lineEditNewPropertyName->setVisible(false);
        ui->labelDataType->setVisible(true);
        ui->comboBoxSourceType->setVisible(true);
        ui->labelPropertyTo->setVisible(true);
        ui->comboBoxTargetType->setVisible(true);
    }
    else if (command == "Split (Vector -> Scalars)") {
        ui->labelPropertyName_1->setText("Property");
        ui->labelPropertyName_2->setVisible(false);
        ui->labelPropertyName_3->setVisible(false);
        ui->comboBoxPropertyNames_2->setVisible(false);
        ui->comboBoxPropertyNames_3->setVisible(false);
        ui->labelNewPropertyName->setText("New property");
        ui->labelNewPropertyName->setVisible(true);
        ui->lineEditNewPropertyName->setVisible(true);
        ui->labelDataType->setVisible(false);
        ui->comboBoxSourceType->setVisible(false);
        ui->labelPropertyTo->setVisible(false);
        ui->comboBoxTargetType->setVisible(false);
    }
    else if (command == "Merge (Scalars -> Vector)") {
        ui->labelPropertyName_1->setText("Property 1");
        ui->labelPropertyName_2->setVisible(true);
        ui->labelPropertyName_3->setVisible(true);
        ui->comboBoxPropertyNames_2->setVisible(true);
        ui->comboBoxPropertyNames_3->setVisible(true);
        ui->labelNewPropertyName->setText("New property");
        ui->labelNewPropertyName->setVisible(true);
        ui->lineEditNewPropertyName->setVisible(true);
        ui->labelDataType->setVisible(false);
        ui->comboBoxSourceType->setVisible(false);
        ui->labelPropertyTo->setVisible(false);
        ui->comboBoxTargetType->setVisible(false);
    }

    bestSize();
    updateProperties();
}


Model* DialogProperties::getModel() {
    const std::string& text = ui->comboBoxModels->currentText().toStdString();
    auto &models = viewer_->models();
    for (auto m : models) {
        const std::string &name = file_system::simple_name(m->name());
        if (name == text)
            return m;
    }
    return nullptr;
}


void DialogProperties::modelChanged(const QString &text) {
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


void DialogProperties::locationChanged(const QString &text) {
    Model* model = getModel();
    if (!model)
        return;

    ui->comboBoxPropertyNames_1->clear();

    const QString& location = ui->comboBoxPropertyLocations->currentText();
    if (dynamic_cast<PointCloud *>(model)) {
        auto cloud = dynamic_cast<PointCloud *>(model);
        if (location == "Vertex") {
            ui->lineEditNewPropertyName->setText("v:");
            for (const auto &name : cloud->vertex_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyNames_1->addItem(QString::fromStdString(name));
        }
    } else if (dynamic_cast<Graph *>(model)) {
        auto graph = dynamic_cast<Graph *>(model);
        if (location == "Vertex") {
            ui->lineEditNewPropertyName->setText("v:");
            for (const auto &name : graph->vertex_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyNames_1->addItem(QString::fromStdString(name));
        } else if (location == "Edge") {
            ui->lineEditNewPropertyName->setText("e:");
            for (const auto &name : graph->edge_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyNames_1->addItem(QString::fromStdString(name));
        }
    } else if (dynamic_cast<SurfaceMesh *>(model)) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (location == "Vertex") {
            ui->lineEditNewPropertyName->setText("v:");
            for (const auto &name : mesh->vertex_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyNames_1->addItem(QString::fromStdString(name));
        } else if (location == "Edge") {
            ui->lineEditNewPropertyName->setText("e:");
            for (const auto &name : mesh->edge_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyNames_1->addItem(QString::fromStdString(name));
        } else if (location == "Face") {
            ui->lineEditNewPropertyName->setText("f:");
            for (const auto &name : mesh->face_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyNames_1->addItem(QString::fromStdString(name));
        } else if (location == "Halfedge") {
            ui->lineEditNewPropertyName->setText("h:");
            for (const auto &name : mesh->halfedge_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyNames_1->addItem(QString::fromStdString(name));
        }
    }
}


void DialogProperties::updateProperties() {
    const QString &model_text = ui->comboBoxModels->currentText();
    bool model_text_has_match = false;

    disconnect(ui->comboBoxModels, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(modelChanged(const QString &)));
    ui->comboBoxModels->clear();
    auto &models = viewer_->models();
    for (auto m : models) {
        const std::string &name = file_system::simple_name(m->name());
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
            const std::string &name = file_system::simple_name(model->name());
            ui->comboBoxModels->setCurrentText(QString::fromStdString(name));
            modelChanged(ui->comboBoxModels->currentText());
        }
    }

    connect(ui->comboBoxModels, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(modelChanged(const QString &)));
}


void DialogProperties::applyCommand() {
    const QString& command = ui->comboBoxCommands->currentText();
    if (command == "Remove")
        removeProperty();
    else if (command == "Rename")
        renameProperty();
    else if (command == "Convert Data Type")
        convertPropertyDataTye();
    else if (command == "Split (Vector -> Scalars)")
        splitProperty();
    else if (command == "Merge (Scalars -> Vector)")
        mergeProperties();
}


void DialogProperties::removeProperty() {
    Model* model = getModel();
    if (!model)
        return;

    const QString& location = ui->comboBoxPropertyLocations->currentText();
    const std::string& property = ui->comboBoxPropertyNames_1->currentText().toStdString();
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

    updateProperties();
    if (succeed)
        window_->updateRenderingPanel();
}


void DialogProperties::renameProperty() {
    Model* model = getModel();
    if (!model)
        return;

    const QString& location = ui->comboBoxPropertyLocations->currentText();
    const std::string& old_name = ui->comboBoxPropertyNames_1->currentText().toStdString();
    if (old_name.empty()) {
        return;
    }

    std::string new_name = ui->lineEditNewPropertyName->text().toStdString();
    if (new_name.size() <= 2) { // because of the prefix "v:", "f:"...
        LOG(WARNING) << "property's new name cannot be empty";
        return;
    }

    bool succeed = false;
    if (dynamic_cast<PointCloud *>(model)) {
        auto cloud = dynamic_cast<PointCloud *>(model);
        if (location == "Vertex") {
            if (new_name.substr(0, 2) != "v:")
                new_name.insert(0, "v:");
            if (cloud->rename_vertex_property(old_name, new_name)) {
                succeed = true;
                LOG(INFO) << "vertex property '" << old_name << "' successfully renamed to '" << new_name << "'";
            }
            else
                LOG(WARNING) << "failed removing vertex property '" << old_name << "'";
        }
    }

    else if (dynamic_cast<Graph *>(model)) {
        auto graph = dynamic_cast<Graph *>(model);
        if (location == "Vertex") {
            if (new_name.substr(0, 2) != "v:")
                new_name.insert(0, "v:");
            if (graph->rename_vertex_property(old_name, new_name)) {
                succeed = true;
                LOG(INFO) << "vertex property '" << old_name << "' successfully renamed to '" << new_name << "'";
            }
            else
                LOG(WARNING) << "failed removing vertex property '" << old_name << "'";
        }
        else if (location == "Edge") {
            if (new_name.substr(0, 2) != "e:")
                new_name.insert(0, "e:");
            if (graph->rename_edge_property(old_name, new_name)) {
                succeed = true;
                LOG(INFO) << "edge property '" << old_name << "' successfully renamed to '" << new_name << "'";
            }
            else
                LOG(WARNING) << "failed removing edge property '" << old_name << "'";
        }
    }

    else if (dynamic_cast<SurfaceMesh *>(model)) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (location == "Vertex") {
            if (new_name.substr(0, 2) != "v:")
                new_name.insert(0, "v:");
            if (mesh->rename_vertex_property(old_name, new_name)) {
                succeed = true;
                LOG(INFO) << "vertex property '" << old_name << "' successfully renamed to '" << new_name << "'";
            }
            else
                LOG(WARNING) << "failed removing vertex property '" << old_name << "'";
        }
        else if (location == "Edge") {
            if (new_name.substr(0, 2) != "e:")
                new_name.insert(0, "e:");
            if (mesh->rename_edge_property(old_name, new_name)) {
                succeed = true;
                LOG(INFO) << "edge property '" << old_name << "' successfully renamed to '" << new_name << "'";
            }
            else
                LOG(WARNING) << "failed removing edge property '" << old_name << "'";
        }
        else if (location == "Face") {
            if (new_name.substr(0, 2) != "f:")
                new_name.insert(0, "f:");
            if (mesh->rename_face_property(old_name, new_name)) {
                succeed = true;
                LOG(INFO) << "face property '" << old_name << "' successfully renamed to '" << new_name << "'";
            }
            else
                LOG(WARNING) << "failed removing face property '" << old_name << "'";
        }
        else if (location == "Halfedge") {
            if (new_name.substr(0, 2) != "h:")
                new_name.insert(0, "h:");
            if (mesh->rename_halfedge_property(old_name, new_name)) {
                succeed = true;
                LOG(INFO) << "halfedge property '" << old_name << "' successfully renamed to '" << new_name << "'";
            }
            else
                LOG(WARNING) << "failed removing halfedge property '" << old_name << "'";
        }
    }

    updateProperties();
    if (succeed)
        window_->updateRenderingPanel();
}


void DialogProperties::convertPropertyDataTye() {

}


// x, y, z -> vec3
void DialogProperties::mergeProperties() {

}


// vec3 -> x, y, z
void DialogProperties::splitProperty() {

}
