#include "dialogs/dialog_properties.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/graph.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/string.h>

#include "paint_canvas.h"
#include "main_window.h"

#include <algorithm>
#include <QDockWidget>

#include "ui_dialog_properties.h"


static std::vector<std::string> key_words = {
        "v:point", "v:connectivity", "v:deleted", "v:normal", "f:normal",
        "f:connectivity", "f:deleted", "f:triangle_range",
        "e:deleted", "h:connectivity"
};


using namespace easy3d;

DialogProperties::DialogProperties(MainWindow *window, QDockWidget *dockWidgetCommand)
        : Dialog(window, dockWidgetCommand), ui(new Ui::DialogProperties) {
    ui->setupUi(this);

    ui->comboBoxCommand->addItem("Remove");
    ui->comboBoxCommand->addItem("Rename");
    ui->comboBoxCommand->addItem("Convert Data Type");
    ui->comboBoxCommand->setCurrentIndex(0);

    ui->comboBoxSourceType->setEditable(false);

    ui->comboBoxTargetType->addItem("float");
    ui->comboBoxTargetType->addItem("double");
    ui->comboBoxTargetType->addItem("int");
    ui->comboBoxTargetType->addItem("unsigned int");
    ui->comboBoxTargetType->addItem("std::size_t");
    ui->comboBoxTargetType->addItem("char");
    ui->comboBoxTargetType->addItem("unsigned char");
    ui->comboBoxTargetType->addItem("vec2");
    ui->comboBoxTargetType->addItem("vec3");

    QMargins margins = ui->lineEditNewPropertyName->textMargins();
    margins.setLeft(7);
    ui->lineEditNewPropertyName->setTextMargins(margins);

    connect(ui->comboBoxCommand, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(commandChanged(const QString &)));
    connect(ui->comboBoxModels, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(modelChanged(const QString &)));
    connect(ui->comboBoxPropertyLocation, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(locationChanged(const QString &)));
    connect(ui->comboBoxPropertyName, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(propertyChanged(const QString &)));
    connect(ui->updateButton, SIGNAL(clicked()), this, SLOT(updateProperties()));
    connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(applyCommand()));

    commandChanged(ui->comboBoxCommand->currentText());
}


DialogProperties::~DialogProperties() {

}


void DialogProperties::commandChanged(const QString &) {
    const QString &command = ui->comboBoxCommand->currentText();
    if (command == "Remove") {
        ui->labelPropertyName->setText("Property");
        ui->labelNewPropertyName->setText("New name");
        ui->labelNewPropertyName->setVisible(false);
        ui->lineEditNewPropertyName->setVisible(false);
        ui->labelDataType->setVisible(false);
        ui->comboBoxSourceType->setVisible(false);
        ui->labelPropertyTo->setVisible(false);
        ui->comboBoxTargetType->setVisible(false);
    } else if (command == "Rename") {
        ui->labelPropertyName->setText("Property");
        ui->labelNewPropertyName->setText("New name");
        ui->labelNewPropertyName->setVisible(true);
        ui->lineEditNewPropertyName->setVisible(true);
        ui->labelDataType->setVisible(false);
        ui->comboBoxSourceType->setVisible(false);
        ui->labelPropertyTo->setVisible(false);
        ui->comboBoxTargetType->setVisible(false);
    } else if (command == "Convert Data Type") {
        ui->labelPropertyName->setText("Property");
        ui->labelNewPropertyName->setText("New name");
        ui->labelNewPropertyName->setVisible(false);
        ui->lineEditNewPropertyName->setVisible(false);
        ui->labelDataType->setVisible(true);
        ui->comboBoxSourceType->setVisible(true);
        ui->labelPropertyTo->setVisible(true);
        ui->comboBoxTargetType->setVisible(true);
    } else if (command == "Split (Vector -> Scalars)") {
        ui->labelPropertyName->setText("Property");
        ui->labelNewPropertyName->setText("New property");
        ui->labelNewPropertyName->setVisible(true);
        ui->lineEditNewPropertyName->setVisible(true);
        ui->labelDataType->setVisible(false);
        ui->comboBoxSourceType->setVisible(false);
        ui->labelPropertyTo->setVisible(false);
        ui->comboBoxTargetType->setVisible(false);
    } else if (command == "Merge (Scalars -> Vector)") {
        ui->labelPropertyName->setText("Property 1");
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


Model *DialogProperties::getModel() {
    const std::string &text = ui->comboBoxModels->currentText().toStdString();
    auto &models = viewer_->models();
    for (auto m : models) {
        const std::string &name = file_system::simple_name(m->name());
        if (name == text)
            return m;
    }
    return nullptr;
}


void DialogProperties::modelChanged(const QString &text) {
    const QString &location_text = ui->comboBoxPropertyLocation->currentText();
    bool location_text_has_match = false;

    ui->comboBoxPropertyLocation->clear();

    Model *model = getModel();
    if (!model)
        return;

    disconnect(ui->comboBoxPropertyLocation, SIGNAL(currentIndexChanged(const QString &)), this,
               SLOT(locationChanged(const QString &)));

    if (dynamic_cast<SurfaceMesh *>(model)) {
        ui->comboBoxPropertyLocation->addItem("Vertex");
        ui->comboBoxPropertyLocation->addItem("Face");
        ui->comboBoxPropertyLocation->addItem("Edge");
        ui->comboBoxPropertyLocation->addItem("Halfedge");
        location_text_has_match = true;
    } else if (dynamic_cast<Graph *>(model)) {
        ui->comboBoxPropertyLocation->addItem("Vertex");
        ui->comboBoxPropertyLocation->addItem("Edge");
        if (location_text == "Vertex" || location_text == "Edge")
            location_text_has_match = true;
    } else if (dynamic_cast<PointCloud *>(model)) {
        ui->comboBoxPropertyLocation->addItem("Vertex");
        if (location_text == "Vertex")
            location_text_has_match = true;
    }

    if (location_text_has_match)
        ui->comboBoxPropertyLocation->setCurrentText(location_text);
    else
        ui->comboBoxPropertyLocation->setCurrentIndex(0);

    locationChanged(ui->comboBoxPropertyLocation->currentText());

    connect(ui->comboBoxPropertyLocation, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(locationChanged(const QString &)));
}


void DialogProperties::locationChanged(const QString &text) {
    ui->comboBoxPropertyName->clear();

    Model *model = getModel();
    if (!model)
        return;

    const QString &location = ui->comboBoxPropertyLocation->currentText();
    if (dynamic_cast<PointCloud *>(model)) {
        auto cloud = dynamic_cast<PointCloud *>(model);
        if (location == "Vertex") {
            ui->lineEditNewPropertyName->setText("v:");
            for (const auto &name : cloud->vertex_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyName->addItem(QString::fromStdString(name));
        }
    } else if (dynamic_cast<Graph *>(model)) {
        auto graph = dynamic_cast<Graph *>(model);
        if (location == "Vertex") {
            ui->lineEditNewPropertyName->setText("v:");
            for (const auto &name : graph->vertex_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyName->addItem(QString::fromStdString(name));
        } else if (location == "Edge") {
            ui->lineEditNewPropertyName->setText("e:");
            for (const auto &name : graph->edge_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyName->addItem(QString::fromStdString(name));
        }
    } else if (dynamic_cast<SurfaceMesh *>(model)) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (location == "Vertex") {
            ui->lineEditNewPropertyName->setText("v:");
            for (const auto &name : mesh->vertex_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyName->addItem(QString::fromStdString(name));
        } else if (location == "Edge") {
            ui->lineEditNewPropertyName->setText("e:");
            for (const auto &name : mesh->edge_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyName->addItem(QString::fromStdString(name));
        } else if (location == "Face") {
            ui->lineEditNewPropertyName->setText("f:");
            for (const auto &name : mesh->face_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyName->addItem(QString::fromStdString(name));
        } else if (location == "Halfedge") {
            ui->lineEditNewPropertyName->setText("h:");
            for (const auto &name : mesh->halfedge_properties())
                if (std::find(key_words.begin(), key_words.end(), name) == key_words.end())
                    ui->comboBoxPropertyName->addItem(QString::fromStdString(name));
        }
    }

    propertyChanged(ui->comboBoxPropertyName->currentText());
}


namespace details {

    inline QString type_info_to_string(const std::type_info &info) {
        if (info == typeid(float)) return "float";
        else if (info == typeid(double)) return "double";
        else if (info == typeid(int)) return "int";
        else if (info == typeid(unsigned int)) return "unsigned int";
        else if (info == typeid(std::size_t)) return "std::size_t";
        else if (info == typeid(bool)) return "bool";
        else if (info == typeid(char)) return "char";
        else if (info == typeid(unsigned char)) return "unsigned char";
        else if (info == typeid(vec2)) return "vec2";
        else if (info == typeid(vec3)) return "vec3";
        else
            return "void";
    }

    template<typename MODEL, typename SourceTarget, typename TargetType>
    inline bool create_vertex_property_from_data(MODEL *model, const std::string &name) {
        auto old_prop = model->template get_vertex_property<SourceTarget>(name);
        if (old_prop) {
            const auto &data = old_prop.vector();
            auto new_prop = model->template add_vertex_property<TargetType>(
                    name + name);   // the old property has the same name, so ...
            if (new_prop) {
                new_prop.vector().assign(data.begin(), data.end());
                model->template remove_vertex_property(old_prop);
                new_prop.set_name(name);
                return true;
            }
        }
        return false;
    }


    template<typename MODEL, typename SourceTarget, typename TargetType>
    inline bool create_face_property_from_data(MODEL *model, const std::string &name) {
        auto old_prop = model->template get_face_property<SourceTarget>(name);
        if (old_prop) {
            const auto &data = old_prop.vector();
            auto new_prop = model->template add_face_property<TargetType>(
                    name + name);   // the old property has the same name, so ...
            if (new_prop) {
                new_prop.vector().assign(data.begin(), data.end());
                model->template remove_face_property(old_prop);
                new_prop.set_name(name);
                return true;
            }
        }
        return false;
    }


    template<typename MODEL, typename SourceTarget, typename TargetType>
    inline bool create_edge_property_from_data(MODEL *model, const std::string &name) {
        auto old_prop = model->template get_edge_property<SourceTarget>(name);
        if (old_prop) {
            const auto &data = old_prop.vector();
            auto new_prop = model->template add_edge_property<TargetType>(
                    name + name);   // the old property has the same name, so ...
            if (new_prop) {
                new_prop.vector().assign(data.begin(), data.end());
                model->template remove_edge_property(old_prop);
                new_prop.set_name(name);
                return true;
            }
        }
        return false;
    }


    template<typename MODEL, typename SourceTarget, typename TargetType>
    inline bool create_halfedge_property_from_data(MODEL *model, const std::string &name) {
        auto old_prop = model->template get_halfedge_property<SourceTarget>(name);
        if (old_prop) {
            const auto &data = old_prop.vector();
            auto new_prop = model->template add_halfedge_property<TargetType>(
                    name + name);   // the old property has the same name, so ...
            if (new_prop) {
                new_prop.vector().assign(data.begin(), data.end());
                model->template remove_halfedge_property(old_prop);
                new_prop.set_name(name);
                return true;
            }
        }
        return false;
    }

    template<typename MODEL>
    inline bool change_vertex_property_type(MODEL *model, const std::string &name, const std::string &source_type,
                                            const std::string &target_type) {
        if (source_type == target_type) {
            LOG(WARNING) << "source and target data types are identical (" << source_type << " == " << target_type
                         << ") and nothing to convert";
            return false;
        }

        if (source_type == "float") {
            if (target_type == "double")
                return details::create_vertex_property_from_data<MODEL, float, double>(model, name);
            else if (target_type == "int")
                return details::create_vertex_property_from_data<MODEL, float, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_vertex_property_from_data<MODEL, float, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_vertex_property_from_data<MODEL, float, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_vertex_property_from_data<MODEL, float, bool>(model, name);
            else if (target_type == "char")
                return details::create_vertex_property_from_data<MODEL, float, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_vertex_property_from_data<MODEL, float, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "double") {
            if (target_type == "float")
                return details::create_vertex_property_from_data<MODEL, double, float>(model, name);
            else if (target_type == "int")
                return details::create_vertex_property_from_data<MODEL, double, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_vertex_property_from_data<MODEL, double, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_vertex_property_from_data<MODEL, double, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_vertex_property_from_data<MODEL, double, bool>(model, name);
            else if (target_type == "char")
                return details::create_vertex_property_from_data<MODEL, double, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_vertex_property_from_data<MODEL, double, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "int") {
            if (target_type == "float")
                return details::create_vertex_property_from_data<MODEL, int, float>(model, name);
            else if (target_type == "double")
                return details::create_vertex_property_from_data<MODEL, int, double>(model, name);
            else if (target_type == "unsigned int")
                return details::create_vertex_property_from_data<MODEL, int, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_vertex_property_from_data<MODEL, int, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_vertex_property_from_data<MODEL, int, bool>(model, name);
            else if (target_type == "char")
                return details::create_vertex_property_from_data<MODEL, int, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_vertex_property_from_data<MODEL, int, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "unsigned int") {
            if (target_type == "float")
                return details::create_vertex_property_from_data<MODEL, unsigned int, float>(model, name);
            else if (target_type == "double")
                return details::create_vertex_property_from_data<MODEL, unsigned int, double>(model, name);
            else if (target_type == "int")
                return details::create_vertex_property_from_data<MODEL, unsigned int, int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_vertex_property_from_data<MODEL, unsigned int, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_vertex_property_from_data<MODEL, unsigned int, bool>(model, name);
            else if (target_type == "char")
                return details::create_vertex_property_from_data<MODEL, unsigned int, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_vertex_property_from_data<MODEL, unsigned int, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "std::size_t") {
            if (target_type == "float")
                return details::create_vertex_property_from_data<MODEL, std::size_t, float>(model, name);
            else if (target_type == "double")
                return details::create_vertex_property_from_data<MODEL, std::size_t, double>(model, name);
            else if (target_type == "int")
                return details::create_vertex_property_from_data<MODEL, std::size_t, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_vertex_property_from_data<MODEL, std::size_t, unsigned int>(model, name);
            else if (target_type == "bool")
                return details::create_vertex_property_from_data<MODEL, std::size_t, bool>(model, name);
            else if (target_type == "char")
                return details::create_vertex_property_from_data<MODEL, std::size_t, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_vertex_property_from_data<MODEL, std::size_t, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "bool") {
            if (target_type == "float")
                return details::create_vertex_property_from_data<MODEL, bool, float>(model, name);
            else if (target_type == "double")
                return details::create_vertex_property_from_data<MODEL, bool, double>(model, name);
            else if (target_type == "int")
                return details::create_vertex_property_from_data<MODEL, bool, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_vertex_property_from_data<MODEL, bool, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_vertex_property_from_data<MODEL, bool, std::size_t>(model, name);
            else if (target_type == "char")
                return details::create_vertex_property_from_data<MODEL, bool, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_vertex_property_from_data<MODEL, bool, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "char") {
            if (target_type == "float")
                return details::create_vertex_property_from_data<MODEL, char, float>(model, name);
            else if (target_type == "double")
                return details::create_vertex_property_from_data<MODEL, char, double>(model, name);
            else if (target_type == "int")
                return details::create_vertex_property_from_data<MODEL, char, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_vertex_property_from_data<MODEL, char, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_vertex_property_from_data<MODEL, char, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_vertex_property_from_data<MODEL, char, bool>(model, name);
            else if (target_type == "unsigned char")
                return details::create_vertex_property_from_data<MODEL, char, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "unsigned char") {
            if (target_type == "float")
                return details::create_vertex_property_from_data<MODEL, unsigned char, float>(model, name);
            else if (target_type == "double")
                return details::create_vertex_property_from_data<MODEL, unsigned char, double>(model, name);
            else if (target_type == "int")
                return details::create_vertex_property_from_data<MODEL, unsigned char, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_vertex_property_from_data<MODEL, unsigned char, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_vertex_property_from_data<MODEL, unsigned char, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_vertex_property_from_data<MODEL, unsigned char, bool>(model, name);
            else if (target_type == "char")
                return details::create_vertex_property_from_data<MODEL, unsigned char, char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else
            LOG(WARNING) << "property of type '" << source_type << "' cannot be converted to type '" << target_type
                         << "'";
        return false;
    }

    template<typename MODEL>
    inline bool change_face_property_type(MODEL *model, const std::string &name, const std::string &source_type,
                                          const std::string &target_type) {
        if (source_type == target_type) {
            LOG(WARNING) << "source and target data types are identical (" << source_type << " == " << target_type
                         << ") and nothing to convert";
            return false;
        }

        if (source_type == "float") {
            if (target_type == "double")
                return details::create_face_property_from_data<MODEL, float, double>(model, name);
            else if (target_type == "int")
                return details::create_face_property_from_data<MODEL, float, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_face_property_from_data<MODEL, float, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_face_property_from_data<MODEL, float, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_face_property_from_data<MODEL, float, bool>(model, name);
            else if (target_type == "char")
                return details::create_face_property_from_data<MODEL, float, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_face_property_from_data<MODEL, float, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "double") {
            if (target_type == "float")
                return details::create_face_property_from_data<MODEL, double, float>(model, name);
            else if (target_type == "int")
                return details::create_face_property_from_data<MODEL, double, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_face_property_from_data<MODEL, double, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_face_property_from_data<MODEL, double, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_face_property_from_data<MODEL, double, bool>(model, name);
            else if (target_type == "char")
                return details::create_face_property_from_data<MODEL, double, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_face_property_from_data<MODEL, double, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "int") {
            if (target_type == "float")
                return details::create_face_property_from_data<MODEL, int, float>(model, name);
            else if (target_type == "double")
                return details::create_face_property_from_data<MODEL, int, double>(model, name);
            else if (target_type == "unsigned int")
                return details::create_face_property_from_data<MODEL, int, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_face_property_from_data<MODEL, int, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_face_property_from_data<MODEL, int, bool>(model, name);
            else if (target_type == "char")
                return details::create_face_property_from_data<MODEL, int, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_face_property_from_data<MODEL, int, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "unsigned int") {
            if (target_type == "float")
                return details::create_face_property_from_data<MODEL, unsigned int, float>(model, name);
            else if (target_type == "double")
                return details::create_face_property_from_data<MODEL, unsigned int, double>(model, name);
            else if (target_type == "int")
                return details::create_face_property_from_data<MODEL, unsigned int, int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_face_property_from_data<MODEL, unsigned int, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_face_property_from_data<MODEL, unsigned int, bool>(model, name);
            else if (target_type == "char")
                return details::create_face_property_from_data<MODEL, unsigned int, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_face_property_from_data<MODEL, unsigned int, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "std::size_t") {
            if (target_type == "float")
                return details::create_face_property_from_data<MODEL, std::size_t, float>(model, name);
            else if (target_type == "double")
                return details::create_face_property_from_data<MODEL, std::size_t, double>(model, name);
            else if (target_type == "int")
                return details::create_face_property_from_data<MODEL, std::size_t, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_face_property_from_data<MODEL, std::size_t, unsigned int>(model, name);
            else if (target_type == "bool")
                return details::create_face_property_from_data<MODEL, std::size_t, bool>(model, name);
            else if (target_type == "char")
                return details::create_face_property_from_data<MODEL, std::size_t, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_face_property_from_data<MODEL, std::size_t, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "bool") {
            if (target_type == "float")
                return details::create_face_property_from_data<MODEL, bool, float>(model, name);
            else if (target_type == "double")
                return details::create_face_property_from_data<MODEL, bool, double>(model, name);
            else if (target_type == "int")
                return details::create_face_property_from_data<MODEL, bool, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_face_property_from_data<MODEL, bool, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_face_property_from_data<MODEL, bool, std::size_t>(model, name);
            else if (target_type == "char")
                return details::create_face_property_from_data<MODEL, bool, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_face_property_from_data<MODEL, bool, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "char") {
            if (target_type == "float")
                return details::create_face_property_from_data<MODEL, char, float>(model, name);
            else if (target_type == "double")
                return details::create_face_property_from_data<MODEL, char, double>(model, name);
            else if (target_type == "int")
                return details::create_face_property_from_data<MODEL, char, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_face_property_from_data<MODEL, char, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_face_property_from_data<MODEL, char, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_face_property_from_data<MODEL, char, bool>(model, name);
            else if (target_type == "unsigned char")
                return details::create_face_property_from_data<MODEL, char, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "unsigned char") {
            if (target_type == "float")
                return details::create_face_property_from_data<MODEL, unsigned char, float>(model, name);
            else if (target_type == "double")
                return details::create_face_property_from_data<MODEL, unsigned char, double>(model, name);
            else if (target_type == "int")
                return details::create_face_property_from_data<MODEL, unsigned char, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_face_property_from_data<MODEL, unsigned char, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_face_property_from_data<MODEL, unsigned char, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_face_property_from_data<MODEL, unsigned char, bool>(model, name);
            else if (target_type == "char")
                return details::create_face_property_from_data<MODEL, unsigned char, char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else
            LOG(WARNING) << "property of type '" << source_type << "' cannot be converted to type '" << target_type
                         << "'";
        return false;
    }


    template<typename MODEL>
    inline bool change_edge_property_type(MODEL *model, const std::string &name, const std::string &source_type,
                                          const std::string &target_type) {
        if (source_type == target_type) {
            LOG(WARNING) << "source and target data types are identical (" << source_type << " == " << target_type
                         << ") and nothing to convert";
            return false;
        }

        if (source_type == "float") {
            if (target_type == "double")
                return details::create_edge_property_from_data<MODEL, float, double>(model, name);
            else if (target_type == "int")
                return details::create_edge_property_from_data<MODEL, float, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_edge_property_from_data<MODEL, float, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_edge_property_from_data<MODEL, float, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_edge_property_from_data<MODEL, float, bool>(model, name);
            else if (target_type == "char")
                return details::create_edge_property_from_data<MODEL, float, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_edge_property_from_data<MODEL, float, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "double") {
            if (target_type == "float")
                return details::create_edge_property_from_data<MODEL, double, float>(model, name);
            else if (target_type == "int")
                return details::create_edge_property_from_data<MODEL, double, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_edge_property_from_data<MODEL, double, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_edge_property_from_data<MODEL, double, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_edge_property_from_data<MODEL, double, bool>(model, name);
            else if (target_type == "char")
                return details::create_edge_property_from_data<MODEL, double, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_edge_property_from_data<MODEL, double, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "int") {
            if (target_type == "float")
                return details::create_edge_property_from_data<MODEL, int, float>(model, name);
            else if (target_type == "double")
                return details::create_edge_property_from_data<MODEL, int, double>(model, name);
            else if (target_type == "unsigned int")
                return details::create_edge_property_from_data<MODEL, int, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_edge_property_from_data<MODEL, int, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_edge_property_from_data<MODEL, int, bool>(model, name);
            else if (target_type == "char")
                return details::create_edge_property_from_data<MODEL, int, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_edge_property_from_data<MODEL, int, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "unsigned int") {
            if (target_type == "float")
                return details::create_edge_property_from_data<MODEL, unsigned int, float>(model, name);
            else if (target_type == "double")
                return details::create_edge_property_from_data<MODEL, unsigned int, double>(model, name);
            else if (target_type == "int")
                return details::create_edge_property_from_data<MODEL, unsigned int, int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_edge_property_from_data<MODEL, unsigned int, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_edge_property_from_data<MODEL, unsigned int, bool>(model, name);
            else if (target_type == "char")
                return details::create_edge_property_from_data<MODEL, unsigned int, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_edge_property_from_data<MODEL, unsigned int, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "std::size_t") {
            if (target_type == "float")
                return details::create_edge_property_from_data<MODEL, std::size_t, float>(model, name);
            else if (target_type == "double")
                return details::create_edge_property_from_data<MODEL, std::size_t, double>(model, name);
            else if (target_type == "int")
                return details::create_edge_property_from_data<MODEL, std::size_t, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_edge_property_from_data<MODEL, std::size_t, unsigned int>(model, name);
            else if (target_type == "bool")
                return details::create_edge_property_from_data<MODEL, std::size_t, bool>(model, name);
            else if (target_type == "char")
                return details::create_edge_property_from_data<MODEL, std::size_t, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_edge_property_from_data<MODEL, std::size_t, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "bool") {
            if (target_type == "float")
                return details::create_edge_property_from_data<MODEL, bool, float>(model, name);
            else if (target_type == "double")
                return details::create_edge_property_from_data<MODEL, bool, double>(model, name);
            else if (target_type == "int")
                return details::create_edge_property_from_data<MODEL, bool, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_edge_property_from_data<MODEL, bool, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_edge_property_from_data<MODEL, bool, std::size_t>(model, name);
            else if (target_type == "char")
                return details::create_edge_property_from_data<MODEL, bool, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_edge_property_from_data<MODEL, bool, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "char") {
            if (target_type == "float")
                return details::create_edge_property_from_data<MODEL, char, float>(model, name);
            else if (target_type == "double")
                return details::create_edge_property_from_data<MODEL, char, double>(model, name);
            else if (target_type == "int")
                return details::create_edge_property_from_data<MODEL, char, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_edge_property_from_data<MODEL, char, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_edge_property_from_data<MODEL, char, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_edge_property_from_data<MODEL, char, bool>(model, name);
            else if (target_type == "unsigned char")
                return details::create_edge_property_from_data<MODEL, char, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "unsigned char") {
            if (target_type == "float")
                return details::create_edge_property_from_data<MODEL, unsigned char, float>(model, name);
            else if (target_type == "double")
                return details::create_edge_property_from_data<MODEL, unsigned char, double>(model, name);
            else if (target_type == "int")
                return details::create_edge_property_from_data<MODEL, unsigned char, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_edge_property_from_data<MODEL, unsigned char, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_edge_property_from_data<MODEL, unsigned char, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_edge_property_from_data<MODEL, unsigned char, bool>(model, name);
            else if (target_type == "char")
                return details::create_edge_property_from_data<MODEL, unsigned char, char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else
            LOG(WARNING) << "property of type '" << source_type << "' cannot be converted to type '" << target_type
                         << "'";
        return false;
    }


    template<typename MODEL>
    inline bool change_halfedge_property_type(MODEL *model, const std::string &name, const std::string &source_type,
                                              const std::string &target_type) {
        if (source_type == target_type) {
            LOG(WARNING) << "source and target data types are identical (" << source_type << " == " << target_type
                         << ") and nothing to convert";
            return false;
        }

        if (source_type == "float") {
            if (target_type == "double")
                return details::create_halfedge_property_from_data<MODEL, float, double>(model, name);
            else if (target_type == "int")
                return details::create_halfedge_property_from_data<MODEL, float, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_halfedge_property_from_data<MODEL, float, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_halfedge_property_from_data<MODEL, float, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_halfedge_property_from_data<MODEL, float, bool>(model, name);
            else if (target_type == "char")
                return details::create_halfedge_property_from_data<MODEL, float, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_halfedge_property_from_data<MODEL, float, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "double") {
            if (target_type == "float")
                return details::create_halfedge_property_from_data<MODEL, double, float>(model, name);
            else if (target_type == "int")
                return details::create_halfedge_property_from_data<MODEL, double, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_halfedge_property_from_data<MODEL, double, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_halfedge_property_from_data<MODEL, double, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_halfedge_property_from_data<MODEL, double, bool>(model, name);
            else if (target_type == "char")
                return details::create_halfedge_property_from_data<MODEL, double, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_halfedge_property_from_data<MODEL, double, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "int") {
            if (target_type == "float")
                return details::create_halfedge_property_from_data<MODEL, int, float>(model, name);
            else if (target_type == "double")
                return details::create_halfedge_property_from_data<MODEL, int, double>(model, name);
            else if (target_type == "unsigned int")
                return details::create_halfedge_property_from_data<MODEL, int, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_halfedge_property_from_data<MODEL, int, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_halfedge_property_from_data<MODEL, int, bool>(model, name);
            else if (target_type == "char")
                return details::create_halfedge_property_from_data<MODEL, int, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_halfedge_property_from_data<MODEL, int, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "unsigned int") {
            if (target_type == "float")
                return details::create_halfedge_property_from_data<MODEL, unsigned int, float>(model, name);
            else if (target_type == "double")
                return details::create_halfedge_property_from_data<MODEL, unsigned int, double>(model, name);
            else if (target_type == "int")
                return details::create_halfedge_property_from_data<MODEL, unsigned int, int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_halfedge_property_from_data<MODEL, unsigned int, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_halfedge_property_from_data<MODEL, unsigned int, bool>(model, name);
            else if (target_type == "char")
                return details::create_halfedge_property_from_data<MODEL, unsigned int, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_halfedge_property_from_data<MODEL, unsigned int, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "std::size_t") {
            if (target_type == "float")
                return details::create_halfedge_property_from_data<MODEL, std::size_t, float>(model, name);
            else if (target_type == "double")
                return details::create_halfedge_property_from_data<MODEL, std::size_t, double>(model, name);
            else if (target_type == "int")
                return details::create_halfedge_property_from_data<MODEL, std::size_t, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_halfedge_property_from_data<MODEL, std::size_t, unsigned int>(model, name);
            else if (target_type == "bool")
                return details::create_halfedge_property_from_data<MODEL, std::size_t, bool>(model, name);
            else if (target_type == "char")
                return details::create_halfedge_property_from_data<MODEL, std::size_t, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_halfedge_property_from_data<MODEL, std::size_t, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "bool") {
            if (target_type == "float")
                return details::create_halfedge_property_from_data<MODEL, bool, float>(model, name);
            else if (target_type == "double")
                return details::create_halfedge_property_from_data<MODEL, bool, double>(model, name);
            else if (target_type == "int")
                return details::create_halfedge_property_from_data<MODEL, bool, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_halfedge_property_from_data<MODEL, bool, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_halfedge_property_from_data<MODEL, bool, std::size_t>(model, name);
            else if (target_type == "char")
                return details::create_halfedge_property_from_data<MODEL, bool, char>(model, name);
            else if (target_type == "unsigned char")
                return details::create_halfedge_property_from_data<MODEL, bool, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "char") {
            if (target_type == "float")
                return details::create_halfedge_property_from_data<MODEL, char, float>(model, name);
            else if (target_type == "double")
                return details::create_halfedge_property_from_data<MODEL, char, double>(model, name);
            else if (target_type == "int")
                return details::create_halfedge_property_from_data<MODEL, char, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_halfedge_property_from_data<MODEL, char, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_halfedge_property_from_data<MODEL, char, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_halfedge_property_from_data<MODEL, char, bool>(model, name);
            else if (target_type == "unsigned char")
                return details::create_halfedge_property_from_data<MODEL, char, unsigned char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else if (source_type == "unsigned char") {
            if (target_type == "float")
                return details::create_halfedge_property_from_data<MODEL, unsigned char, float>(model, name);
            else if (target_type == "double")
                return details::create_halfedge_property_from_data<MODEL, unsigned char, double>(model, name);
            else if (target_type == "int")
                return details::create_halfedge_property_from_data<MODEL, unsigned char, int>(model, name);
            else if (target_type == "unsigned int")
                return details::create_halfedge_property_from_data<MODEL, unsigned char, unsigned int>(model, name);
            else if (target_type == "std::size_t")
                return details::create_halfedge_property_from_data<MODEL, unsigned char, std::size_t>(model, name);
            else if (target_type == "bool")
                return details::create_halfedge_property_from_data<MODEL, unsigned char, bool>(model, name);
            else if (target_type == "char")
                return details::create_halfedge_property_from_data<MODEL, unsigned char, char>(model, name);
            else
                LOG(WARNING) << "unaccepted target data type: " << target_type;
        } else
            LOG(WARNING) << "property of type '" << source_type << "' cannot be converted to type '" << target_type
                         << "'";
        return false;
    }
}


void DialogProperties::propertyChanged(const QString &name) {
    ui->comboBoxSourceType->clear();

    if (name.isEmpty())
        return;

    const std::string &location = ui->comboBoxPropertyLocation->currentText().toStdString();
    if (location.empty())
        return;

    const std::string &property_name = name.toStdString();

    Model *model = getModel();
    if (!model)
        return;

    QString type = "void";
    if (dynamic_cast<PointCloud *>(model)) {
        auto cloud = dynamic_cast<PointCloud *>(model);
        if (location == "Vertex") {
            const auto &info = cloud->get_vertex_property_type(name.toStdString());
            type = details::type_info_to_string(info);
        }
    } else if (dynamic_cast<Graph *>(model)) {
        auto graph = dynamic_cast<Graph *>(model);
        if (location == "Vertex") {
            const auto &info = graph->get_vertex_property_type(name.toStdString());
            type = details::type_info_to_string(info);
        } else if (location == "Edge") {
            const auto &info = graph->get_edge_property_type(name.toStdString());
            type = details::type_info_to_string(info);
        }
    } else if (dynamic_cast<SurfaceMesh *>(model)) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (location == "Vertex") {
            const auto &info = mesh->get_vertex_property_type(name.toStdString());
            type = details::type_info_to_string(info);
        } else if (location == "Edge") {
            const auto &info = mesh->get_edge_property_type(name.toStdString());
            type = details::type_info_to_string(info);
        } else if (location == "Face") {
            const auto &info = mesh->get_face_property_type(name.toStdString());
            type = details::type_info_to_string(info);
        } else if (location == "Halfedge") {
            const auto &info = mesh->get_halfedge_property_type(name.toStdString());
            type = details::type_info_to_string(info);
        }
    }

    if (type != "void")
        ui->comboBoxSourceType->addItem(type);
    else
        LOG(WARNING) << "unrecognized data type for property '" << property_name << "' defined on '" << location << "'";
}


void DialogProperties::updateProperties() {
    const std::string& model_text = ui->comboBoxModels->currentText().toStdString();
    bool model_text_has_match = false;

    disconnect(ui->comboBoxModels, SIGNAL(currentIndexChanged(const QString &)), this,
               SLOT(modelChanged(const QString &)));
    ui->comboBoxModels->clear();

    auto &models = viewer_->models();
    for (auto m : models) {
        const std::string &name = file_system::simple_name(m->name());
        const QString str = QString::fromStdString(name);
        ui->comboBoxModels->addItem(str);
        if (name == model_text)
            model_text_has_match = true;
    }

    if (model_text_has_match)
        ui->comboBoxModels->setCurrentText(QString::fromStdString(model_text));
    else {
        auto model = viewer_->currentModel();
        if (model) {
            const std::string &name = file_system::simple_name(model->name());
            ui->comboBoxModels->setCurrentText(QString::fromStdString(name));
        }
    }
    modelChanged(ui->comboBoxModels->currentText());

    connect(ui->comboBoxModels, SIGNAL(currentIndexChanged(const QString &)), this,
            SLOT(modelChanged(const QString &)));
}


void DialogProperties::applyCommand() {
    const QString &command = ui->comboBoxCommand->currentText();

    bool succeed = false;
    if (command == "Remove") {
        succeed = removeProperty();
        if (succeed) {
            Model *model = getModel();
            if (model)
                model->update();
        }
    }
    else if (command == "Rename")
        succeed = renameProperty();
    else if (command == "Convert Data Type")
        succeed = convertPropertyDataType();

    if (succeed) {
        updateProperties();
        window_->updateRenderingPanel();
    }
}


bool DialogProperties::removeProperty() {
    Model *model = getModel();
    if (!model)
        return false;

    const QString &location = ui->comboBoxPropertyLocation->currentText();
    const std::string &property = ui->comboBoxPropertyName->currentText().toStdString();
    if (property.empty())
        return false;

    if (dynamic_cast<PointCloud *>(model)) {
        auto cloud = dynamic_cast<PointCloud *>(model);
        if (location == "Vertex") {
            if (cloud->remove_vertex_property(property)) {
                LOG(INFO) << "vertex property '" << property << "' successfully removed";
                return true;
            } else
                LOG(WARNING) << "failed removing vertex property '" << property << "'";
        }
    } else if (dynamic_cast<Graph *>(model)) {
        auto graph = dynamic_cast<Graph *>(model);
        if (location == "Vertex") {
            if (graph->remove_vertex_property(property)) {
                LOG(INFO) << "vertex property '" << property << "' successfully removed";
                return true;
            } else
                LOG(WARNING) << "failed removing vertex property '" << property << "'";
        } else if (location == "Edge") {
            if (graph->remove_edge_property(property)) {
                LOG(INFO) << "edge property '" << property << "' successfully removed";
                return true;
            } else
                LOG(WARNING) << "failed removing edge property '" << property << "'";
        }
    } else if (dynamic_cast<SurfaceMesh *>(model)) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (location == "Vertex") {
            if (mesh->remove_vertex_property(property)) {
                LOG(INFO) << "vertex property '" << property << "' successfully removed";
                return true;
            } else
                LOG(WARNING) << "failed removing vertex property '" << property << "'";
        } else if (location == "Edge") {
            if (mesh->remove_edge_property(property)) {
                LOG(INFO) << "edge property '" << property << "' successfully removed";
                return true;
            } else
                LOG(WARNING) << "failed removing edge property '" << property << "'";
        } else if (location == "Face") {
            if (mesh->remove_face_property(property)) {
                LOG(INFO) << "face property '" << property << "' successfully removed";
                return true;
            } else
                LOG(WARNING) << "failed removing face property '" << property << "'";
        } else if (location == "Halfedge") {
            if (mesh->remove_halfedge_property(property)) {
                LOG(INFO) << "halfedge property '" << property << "' successfully removed";
                return true;
            } else
                LOG(WARNING) << "failed removing halfedge property '" << property << "'";
        }
    }

    return false;
}


bool DialogProperties::renameProperty() {
    Model *model = getModel();
    if (!model)
        return false;

    const QString &location = ui->comboBoxPropertyLocation->currentText();
    const std::string &old_name = ui->comboBoxPropertyName->currentText().toStdString();
    if (old_name.empty())
        return false;

    std::string new_name = ui->lineEditNewPropertyName->text().toStdString();
    if (new_name.size() <= 2) { // because of the prefix "v:", "f:"...
        LOG(WARNING) << "property's new name cannot be empty";
        return false;
    }

    if (dynamic_cast<PointCloud *>(model)) {
        auto cloud = dynamic_cast<PointCloud *>(model);
        if (location == "Vertex") {
            if (new_name.substr(0, 2) != "v:")
                new_name.insert(0, "v:");
            if (cloud->rename_vertex_property(old_name, new_name)) {
                LOG(INFO) << "vertex property '" << old_name << "' successfully renamed to '" << new_name << "'";
                return true;
            } else
                LOG(WARNING) << "failed removing vertex property '" << old_name << "'";
        }
    } else if (dynamic_cast<Graph *>(model)) {
        auto graph = dynamic_cast<Graph *>(model);
        if (location == "Vertex") {
            if (new_name.substr(0, 2) != "v:")
                new_name.insert(0, "v:");
            if (graph->rename_vertex_property(old_name, new_name)) {
                LOG(INFO) << "vertex property '" << old_name << "' successfully renamed to '" << new_name << "'";
                return true;
            } else
                LOG(WARNING) << "failed removing vertex property '" << old_name << "'";
        } else if (location == "Edge") {
            if (new_name.substr(0, 2) != "e:")
                new_name.insert(0, "e:");
            if (graph->rename_edge_property(old_name, new_name)) {
                LOG(INFO) << "edge property '" << old_name << "' successfully renamed to '" << new_name << "'";
                return true;
            } else
                LOG(WARNING) << "failed removing edge property '" << old_name << "'";
        }
    } else if (dynamic_cast<SurfaceMesh *>(model)) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (location == "Vertex") {
            if (new_name.substr(0, 2) != "v:")
                new_name.insert(0, "v:");
            if (mesh->rename_vertex_property(old_name, new_name)) {
                LOG(INFO) << "vertex property '" << old_name << "' successfully renamed to '" << new_name << "'";
                return true;
            } else
                LOG(WARNING) << "failed removing vertex property '" << old_name << "'";
        } else if (location == "Edge") {
            if (new_name.substr(0, 2) != "e:")
                new_name.insert(0, "e:");
            if (mesh->rename_edge_property(old_name, new_name)) {
                LOG(INFO) << "edge property '" << old_name << "' successfully renamed to '" << new_name << "'";
                return true;
            } else
                LOG(WARNING) << "failed removing edge property '" << old_name << "'";
        } else if (location == "Face") {
            if (new_name.substr(0, 2) != "f:")
                new_name.insert(0, "f:");
            if (mesh->rename_face_property(old_name, new_name)) {
                LOG(INFO) << "face property '" << old_name << "' successfully renamed to '" << new_name << "'";
                return true;
            } else
                LOG(WARNING) << "failed removing face property '" << old_name << "'";
        } else if (location == "Halfedge") {
            if (new_name.substr(0, 2) != "h:")
                new_name.insert(0, "h:");
            if (mesh->rename_halfedge_property(old_name, new_name)) {
                LOG(INFO) << "halfedge property '" << old_name << "' successfully renamed to '" << new_name << "'";
                return true;
            } else
                LOG(WARNING) << "failed removing halfedge property '" << old_name << "'";
        }
    }

    return false;
}


bool DialogProperties::convertPropertyDataType() {
    Model *model = getModel();
    if (!model)
        return false;

    const std::string &location = ui->comboBoxPropertyLocation->currentText().toStdString();
    const std::string &name = ui->comboBoxPropertyName->currentText().toStdString();
    const std::string &source_type = ui->comboBoxSourceType->currentText().toStdString();
    const std::string &target_type = ui->comboBoxTargetType->currentText().toStdString();
    if (location.empty() || name.empty() || source_type.empty() || target_type.empty())
        return false;

    bool succeed = false;
    if (dynamic_cast<PointCloud *>(model)) {
        auto cloud = dynamic_cast<PointCloud *>(model);
        if (location == "Vertex")
            succeed = details::change_vertex_property_type(cloud, name, source_type, target_type);
    } else if (dynamic_cast<Graph *>(model)) {
        auto graph = dynamic_cast<Graph *>(model);
        if (location == "Vertex")
            succeed = details::change_vertex_property_type(graph, name, source_type, target_type);
        else if (location == "Edge")
            succeed = details::change_edge_property_type(graph, name, source_type, target_type);
    } else if (dynamic_cast<SurfaceMesh *>(model)) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (location == "Vertex")
            succeed = details::change_vertex_property_type(mesh, name, source_type, target_type);
        else if (location == "Edge")
            succeed = details::change_edge_property_type(mesh, name, source_type, target_type);
        else if (location == "Face")
            succeed = details::change_face_property_type(mesh, name, source_type, target_type);
        else if (location == "Halfedge")
            succeed = details::change_halfedge_property_type(mesh, name, source_type, target_type);
    }

    if (succeed) {
        LOG(INFO) << "the type of " << string::to_lowercase(location) << " property '" << name << "' changed from '"
                  << source_type << "' to '" << target_type << "'";
        return true;
    }
    return false;
}