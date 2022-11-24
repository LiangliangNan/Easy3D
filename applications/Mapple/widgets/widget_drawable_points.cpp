/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/


#include "widget_drawable_points.h"

#include <QColorDialog>

#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/buffer.h>
#include <easy3d/util/logging.h>

#include "paint_canvas.h"
#include "main_window.h"
#include "widget_model_list.h"

#include "ui_widget_drawable_points.h"


using namespace easy3d;


WidgetPointsDrawable::WidgetPointsDrawable(QWidget *parent)
        : WidgetDrawable(parent), ui_(new Ui::WidgetPointsDrawable) 
{
    ui_->setupUi(this);

    if (colormaps_.empty())
        ui_->comboBoxScalarFieldStyle->addItem("not available");
    else {
        for (const auto &colormap : colormaps_)
            ui_->comboBoxScalarFieldStyle->addItem(QIcon(QString::fromStdString(colormap.file)),
                                                  QString::fromStdString("  " + colormap.name));
    }
}


void WidgetPointsDrawable::connectAll() {
    // which drawable
    connect(ui_->comboBoxDrawables, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setActiveDrawable(const QString &)));

    // visible
    connect(ui_->checkBoxVisible, SIGNAL(toggled(bool)), this, SLOT(setDrawableVisible(bool)));

    // thickness
    connect(ui_->doubleSpinBoxPointSize, SIGNAL(valueChanged(double)), this, SLOT(setPointSize(double)));

    // imposter
    connect(ui_->comboBoxImposterStyle, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setImposterStyle(const QString &)));

    // lighting
    connect(ui_->comboBoxLightingOptions, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setLighting(const QString &)));

    // color scheme
    connect(ui_->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setColorScheme(const QString &)));

    // default color
    connect(ui_->toolButtonDefaultColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

    // back color
    connect(ui_->checkBoxBackColor, SIGNAL(toggled(bool)), this, SLOT(setDistinctBackColor(bool)));
    connect(ui_->toolButtonBackColor, SIGNAL(clicked()), this, SLOT(setBackColor()));

    // highlight
    connect(ui_->checkBoxHighlight, SIGNAL(toggled(bool)), this, SLOT(setHighlight(bool)));
    connect(ui_->spinBoxHighlightMin, SIGNAL(valueChanged(int)), this, SLOT(setHighlightMin(int)));
    connect(ui_->spinBoxHighlightMax, SIGNAL(valueChanged(int)), this, SLOT(setHighlightMax(int)));

    // scalar field
    connect(ui_->comboBoxScalarFieldStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(setScalarFieldStyle(int)));
    connect(ui_->checkBoxScalarFieldDiscrete, SIGNAL(toggled(bool)), this, SLOT(setScalarFieldDiscreteColors(bool)));
    connect(ui_->spinBoxScalarFieldNumStrips, SIGNAL(valueChanged(int)), this, SLOT(setScalarFieldNumOfStripes(int)));
    connect(ui_->checkBoxScalarFieldClamp, SIGNAL(toggled(bool)), this, SLOT(setScalarFieldClamp(bool)));
    connect(ui_->doubleSpinBoxScalarFieldClampLower, SIGNAL(valueChanged(double)), this, SLOT(setScalarFieldClampLower(double)));
    connect(ui_->doubleSpinBoxScalarFieldClampUpper, SIGNAL(valueChanged(double)), this, SLOT(setScalarFieldClampUpper(double)));

    // vector field
    connect(ui_->comboBoxVectorField, SIGNAL(currentIndexChanged(const QString&)), this,
            SLOT(setVectorField(const QString&)));
    connect(ui_->doubleSpinBoxVectorFieldScale, SIGNAL(valueChanged(double)), this, SLOT(setVectorFieldScale(double)));
}


void WidgetPointsDrawable::disconnectAll() {
    // which drawable
    disconnect(ui_->comboBoxDrawables, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setActiveDrawable(const QString &)));

    // visible
    disconnect(ui_->checkBoxVisible, SIGNAL(toggled(bool)), this, SLOT(setDrawableVisible(bool)));

    // thickness
    disconnect(ui_->doubleSpinBoxPointSize, SIGNAL(valueChanged(double)), this, SLOT(setPointSize(double)));

    // imposter
    disconnect(ui_->comboBoxImposterStyle, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setImposterStyle(const QString &)));

    // lighting
    disconnect(ui_->comboBoxLightingOptions, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setLighting(const QString &)));

    // color scheme
    disconnect(ui_->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setColorScheme(const QString &)));

    // default color
    disconnect(ui_->toolButtonDefaultColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

    // back color
    disconnect(ui_->checkBoxBackColor, SIGNAL(toggled(bool)), this, SLOT(setDistinctBackColor(bool)));
    disconnect(ui_->toolButtonBackColor, SIGNAL(clicked()), this, SLOT(setBackColor()));

    // highlight
    disconnect(ui_->checkBoxHighlight, SIGNAL(toggled(bool)), this, SLOT(setHighlight(bool)));
    disconnect(ui_->spinBoxHighlightMin, SIGNAL(valueChanged(int)), this, SLOT(setHighlightMin(int)));
    disconnect(ui_->spinBoxHighlightMax, SIGNAL(valueChanged(int)), this, SLOT(setHighlightMax(int)));

    // scalar field
    disconnect(ui_->comboBoxScalarFieldStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(setScalarFieldStyle(int)));
    disconnect(ui_->checkBoxScalarFieldDiscrete, SIGNAL(toggled(bool)), this, SLOT(setScalarFieldDiscreteColors(bool)));
    disconnect(ui_->spinBoxScalarFieldNumStrips, SIGNAL(valueChanged(int)), this, SLOT(setScalarFieldNumOfStripes(int)));
    disconnect(ui_->checkBoxScalarFieldClamp, SIGNAL(toggled(bool)), this, SLOT(setScalarFieldClamp(bool)));
    disconnect(ui_->doubleSpinBoxScalarFieldClampLower, SIGNAL(valueChanged(double)), this, SLOT(setScalarFieldClampLower(double)));
    disconnect(ui_->doubleSpinBoxScalarFieldClampUpper, SIGNAL(valueChanged(double)), this, SLOT(setScalarFieldClampUpper(double)));

    // vector field
    disconnect(ui_->comboBoxVectorField, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setVectorField(const QString&)));
    disconnect(ui_->doubleSpinBoxVectorFieldScale, SIGNAL(valueChanged(double)), this, SLOT(setVectorFieldScale(double)));
}


WidgetPointsDrawable::~WidgetPointsDrawable() {
    delete ui_;
}


namespace internal {

    // color schemes from scalar fields defined on vertices
    template <typename MODEL>
    void color_schemes_for_scalar_fields(MODEL* model, const QString& scalar_prefix, std::vector<QString>& schemes) {
        // color schemes from color properties and texture
        for (const auto &name : model->vertex_properties()) {
            if (name.find("v:color") != std::string::npos || name.find("v:texcoord") != std::string::npos)
                schemes.push_back(QString::fromStdString(name));
        }
        for (const auto &name : model->vertex_properties()) {
            if (model->template get_vertex_property<float>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_vertex_property<double>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_vertex_property<unsigned int>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_vertex_property<int>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_vertex_property<char>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_vertex_property<unsigned char>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_vertex_property<bool>(name) && name == "v:select")
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
        }
    }

    // vector fields defined on vertices
    template <typename MODEL>
    void vector_fields_on_vertices(MODEL* model, std::vector<QString>& fields) {
        for (const auto &name : model->vertex_properties()) {
            if (model->template get_vertex_property<vec3>(name)) {
                if (name != "v:color" && name != "v:point")
                    fields.push_back(QString::fromStdString(name));
            }
        }
    }

}


std::vector<QString> WidgetPointsDrawable::colorSchemes(const easy3d::Model *model) {
    std::vector<QString> schemes;
    schemes.emplace_back("uniform color");

    auto cloud = dynamic_cast<PointCloud *>(viewer_->currentModel());
    if (cloud)
        internal::color_schemes_for_scalar_fields(cloud, scalar_prefix_, schemes);

    else {
        auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
        if (mesh)
            internal::color_schemes_for_scalar_fields(mesh, scalar_prefix_, schemes);

        else {
            auto graph = dynamic_cast<Graph *>(viewer_->currentModel());
            if (graph)
                internal::color_schemes_for_scalar_fields(graph, scalar_prefix_, schemes);

            else {
                auto poly = dynamic_cast<PolyMesh *>(viewer_->currentModel());
                if (poly)
                    internal::color_schemes_for_scalar_fields(poly, scalar_prefix_, schemes);
            }
        }
    }

    return schemes;
}


std::vector<QString> WidgetPointsDrawable::vectorFields(const easy3d::Model *model) {
    std::vector<QString> fields;

    auto cloud = dynamic_cast<PointCloud *>(viewer_->currentModel());
    if (cloud)
        internal::vector_fields_on_vertices(cloud, fields);

    else {
        auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
        if (mesh)
            internal::vector_fields_on_vertices(mesh, fields);

        else {
            auto graph = dynamic_cast<Graph *>(viewer_->currentModel());
            if (graph)
                internal::vector_fields_on_vertices(graph, fields);

            else {
                auto poly = dynamic_cast<PolyMesh *>(viewer_->currentModel());
                if (poly)
                    internal::vector_fields_on_vertices(poly, fields);
            }
        }
    }

    // if no vector fields found, add a "not available" item
    if (fields.empty())
        fields.emplace_back("not available");
    else   // add one allowing to disable vector fields
        fields.insert(fields.begin(), "disabled");

    return fields;
}


// update the panel to be consistent with the drawable's rendering parameters
void WidgetPointsDrawable::updatePanel() {
    auto model = viewer_->currentModel();
    if (!model)
        active_drawable_.clear();

    auto d = dynamic_cast<PointsDrawable*>(drawable());
    if (!model || !model->renderer()->is_visible() || !d) {
        setEnabled(false);
        return;
    }

    setEnabled(true);

    disconnectAll();

    auto &state = states_[d];
    auto &scheme = d->state();

    ui_->comboBoxDrawables->clear();
    const auto &drawables = model->renderer()->points_drawables();
    for (auto dd : drawables)
        ui_->comboBoxDrawables->addItem(QString::fromStdString(dd->name()));
    ui_->comboBoxDrawables->setCurrentText(QString::fromStdString(d->name()));

    // visible
    ui_->checkBoxVisible->setChecked(d->is_visible());
    window_->widgetModelList()->updateDrawableVisibilities();

    // thickness
    ui_->doubleSpinBoxPointSize->setValue(d->point_size());

    // imposter
    switch (d->impostor_type()) {
        case PointsDrawable::PLAIN:
            ui_->comboBoxImposterStyle->setCurrentText("plain");
            break;
        case PointsDrawable::SPHERE:
            ui_->comboBoxImposterStyle->setCurrentText("sphere");
            break;
        case PointsDrawable::SURFEL:
            ui_->comboBoxImposterStyle->setCurrentText("surfel");
            break;
    }

    {   // lighting
        if (d->lighting()) {
            if (d->lighting_two_sides())
                ui_->comboBoxLightingOptions->setCurrentText("front and back");
            else
                ui_->comboBoxLightingOptions->setCurrentText("front only");
        } else
            ui_->comboBoxLightingOptions->setCurrentText("disabled");
    }

    {   // color scheme
        ui_->comboBoxColorScheme->clear();
        const std::vector<QString> &schemes = colorSchemes(viewer_->currentModel());
        for (const auto &s : schemes)
            ui_->comboBoxColorScheme->addItem(s);

        for (const auto& name : schemes) {
            if (name.contains(QString::fromStdString(scheme.property_name()))) {
                ui_->comboBoxColorScheme->setCurrentText(name);
                break;
            }
        }

        // default color
        auto c = d->color();
        QPixmap pixmap(ui_->toolButtonDefaultColor->size());
        pixmap.fill(
                QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui_->toolButtonDefaultColor->setIcon(QIcon(pixmap));

        // back side color
        ui_->checkBoxBackColor->setChecked(d->distinct_back_color());
        c = d->back_color();
        pixmap.fill(
                QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui_->toolButtonBackColor->setIcon(QIcon(pixmap));
    }

    {   // highlight
        bool highlight = d->highlight();
        ui_->checkBoxHighlight->setChecked(highlight);

        const auto &range = d->highlight_range();
        ui_->spinBoxHighlightMin->setValue(range.first);
        ui_->spinBoxHighlightMax->setValue(range.second);
    }

    {   // scalar field
        ui_->comboBoxScalarFieldStyle->setCurrentIndex(state.scalar_style);
        ui_->checkBoxScalarFieldDiscrete->setChecked(states_[d].discrete_color);
        ui_->spinBoxScalarFieldNumStrips->setValue(states_[d].num_stripes);
        const auto& coloring = ui_->comboBoxColorScheme->currentText().toStdString();
        if (coloring.find("scalar - ") != std::string::npos && coloring.find(scheme.property_name()) != std::string::npos) {
            auto tex = colormapTexture(states_[d].scalar_style, states_[d].discrete_color, states_[d].num_stripes);
            if (tex)
                d->set_texture(tex);
        }
        ui_->checkBoxScalarFieldClamp->setChecked(scheme.clamp_range());
        ui_->doubleSpinBoxScalarFieldClampLower->setValue(scheme.clamp_lower() * 100);
        ui_->doubleSpinBoxScalarFieldClampUpper->setValue(scheme.clamp_upper() * 100);
    }

    {   // vector field
        ui_->comboBoxVectorField->clear();
        const std::vector<QString> &fields = vectorFields(viewer_->currentModel());
        for (const auto& name : fields)
            ui_->comboBoxVectorField->addItem(name);

        ui_->comboBoxVectorField->setCurrentText(state.vector_field);
        ui_->doubleSpinBoxVectorFieldScale->setValue(state.vector_field_scale);
    }

    disableUnavailableOptions();

    connectAll();
}


Drawable *WidgetPointsDrawable::drawable() {
    auto model = viewer_->currentModel();
    if (!model) {
        active_drawable_.clear();
        return nullptr;
    }

    auto pos = active_drawable_.find(model);
    if (pos != active_drawable_.end())
        return model->renderer()->get_points_drawable(pos->second);
    else {
        for (auto d : model->renderer()->points_drawables()) {
            active_drawable_[model] = d->name();
            return d;
        }
        return nullptr;
    }
}


void WidgetPointsDrawable::setActiveDrawable(const QString &text) {
    auto model = viewer_->currentModel();
    const std::string &name = text.toStdString();

    if (active_drawable_.find(model) != active_drawable_.end()) {
        if (active_drawable_[model] == name)
            return; // already active
    }

    if (model->renderer()->get_points_drawable(name)) {
        active_drawable_[model] = name;
    } else {
        LOG(ERROR) << "drawable '" << name << "' not defined on model: " << model->name();
        const auto &drawables = model->renderer()->points_drawables();
        if (drawables.empty())
            LOG(ERROR) << "no points drawable defined on model: " << model->name();
        else
            active_drawable_[model] = drawables[0]->name();
    }

    updatePanel();
}


void WidgetPointsDrawable::setPointSize(double s) {
    auto d = dynamic_cast<PointsDrawable*>(drawable());
    d->set_point_size(s);
    viewer_->update();
}


void WidgetPointsDrawable::setImposterStyle(const QString &style) {
    auto d = dynamic_cast<PointsDrawable*>(drawable());
    if (style == "plain")
        d->set_impostor_type(PointsDrawable::PLAIN);
    else if (style == "sphere")
        d->set_impostor_type(PointsDrawable::SPHERE);
    else if (style == "surfel") {
        if (d->normal_buffer() == 0) { // surfel requires point normals
            auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
            if (mesh) {
                auto normals = mesh->get_vertex_property<vec3>("v:normal");
                if (!normals) {
                    mesh->update_vertex_normals();
                    normals = mesh->get_vertex_property<vec3>("v:normal");
                }
                viewer_->makeCurrent();
                d->update_normal_buffer(normals.vector());
                viewer_->doneCurrent();
            } else if (dynamic_cast<PointCloud *>(viewer_->currentModel())) {
                if (d->normal_buffer() == 0) { // surfel requires point normals
                    auto cloud = dynamic_cast<PointCloud *>(viewer_->currentModel());
                    auto normals = cloud->get_vertex_property<vec3>("v:normal");
                    if (normals) {
                        viewer_->makeCurrent();
                        d->update_normal_buffer(normals.vector());
                        viewer_->doneCurrent();
                    }
                }
            }
        }

        if (d->normal_buffer())
            d->set_impostor_type(PointsDrawable::SURFEL);
        else
            LOG(WARNING) << "point imposter SURFEL requires normal information";
    }

    viewer_->update();
    disableUnavailableOptions();
}


void WidgetPointsDrawable::setColorScheme(const QString &text) {
    auto d = drawable();
    auto& scheme = d->state();
    scheme.set_clamp_range(ui_->checkBoxScalarFieldClamp->isChecked());
    scheme.set_clamp_lower(ui_->doubleSpinBoxScalarFieldClampLower->value() / 100.0f);
    scheme.set_clamp_upper(ui_->doubleSpinBoxScalarFieldClampUpper->value() / 100.0f);
    states_[d].scalar_style = ui_->comboBoxScalarFieldStyle->currentIndex();

    WidgetDrawable::setColorScheme(text);
}


void WidgetPointsDrawable::setDefaultColor() {
    auto d = drawable();
    const auto &c = d->color();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    const QColor &color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec4 new_color(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        d->set_uniform_coloring(new_color);
        viewer_->update();

        QPixmap pixmap(ui_->toolButtonDefaultColor->size());
        pixmap.fill(color);
        ui_->toolButtonDefaultColor->setIcon(QIcon(pixmap));
    }
}


void WidgetPointsDrawable::setBackColor() {
    auto d = drawable();
    const auto &c = d->back_color();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    const QColor &color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec4 new_color(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        d->set_back_color(new_color);
        viewer_->update();

        QPixmap pixmap(ui_->toolButtonBackColor->size());
        pixmap.fill(color);
        ui_->toolButtonBackColor->setIcon(QIcon(pixmap));
    }
}


void WidgetPointsDrawable::setVectorField(const QString &text) {
    auto model = viewer_->currentModel();

    if (text == "disabled") {
        const auto &drawables = model->renderer()->lines_drawables();
        for (auto d : drawables) {
            if (d->name().find("vector - v") != std::string::npos)
                d->set_visible(false);
        }
        states_[drawable()].vector_field = "disabled";
    } else {
        const std::string &name = text.toStdString();
        updateVectorFieldBuffer(model, name);

        auto d = model->renderer()->get_lines_drawable("vector - " + name);
        if (d) {
            d->set_visible(true);
            states_[drawable()].vector_field = text;
        }
    }

    viewer_->update();
    window_->updateRenderingPanel();
}


void WidgetPointsDrawable::updateVectorFieldBuffer(Model *model, const std::string &name) {
    if (name == "v:normal") {
        if (dynamic_cast<SurfaceMesh *>(model)) {
            auto mesh = dynamic_cast<SurfaceMesh *>(model);
            auto normals = mesh->get_vertex_property<vec3>(name);
            if (!normals)
                mesh->update_vertex_normals();
        }
        else if (dynamic_cast<PointCloud *>(model)) {
            auto cloud = dynamic_cast<PointCloud *>(model);
            auto normals = cloud->get_vertex_property<vec3>(name);
            if (!normals) {
                LOG(WARNING) << "normal information does not exist";
                return;
            }
        }
        else if (dynamic_cast<PolyMesh *>(model)) {
            auto poly = dynamic_cast<PolyMesh *>(model);
            auto normals = poly->get_vertex_property<vec3>(name);
            if (!normals)
                poly->update_vertex_normals();
        }
    }

    // a vector field is visualized as a LinesDrawable whose name is the same as the vector field
    auto drawable = model->renderer()->get_lines_drawable("vector - " + name);
    if (!drawable) {
        drawable = model->renderer()->add_lines_drawable("vector - " + name);
        drawable->set_update_func([&, name](Model *m, Drawable *d) -> void {
            const float scale = ui_->doubleSpinBoxVectorFieldScale->value();
            if (dynamic_cast<SurfaceMesh *>(m))
                buffer::update(dynamic_cast<SurfaceMesh*>(m), dynamic_cast<LinesDrawable*>(d), name, State::VERTEX, scale);
            else if (dynamic_cast<PointCloud *>(m))
                buffer::update(dynamic_cast<PointCloud*>(m), dynamic_cast<LinesDrawable*>(d), name, scale);
            else if (dynamic_cast<PolyMesh *>(m))
                buffer::update(dynamic_cast<PolyMesh*>(m), dynamic_cast<LinesDrawable*>(d), name, State::VERTEX, scale);
        });
    }
}


void WidgetPointsDrawable::disableUnavailableOptions() {
    auto d = drawable();

    bool visible = ui_->checkBoxVisible->isChecked();
    ui_->labelPointSize->setEnabled(visible);
    ui_->doubleSpinBoxPointSize->setEnabled(visible);
    ui_->labelImposterStyle->setEnabled(visible);
    ui_->comboBoxImposterStyle->setEnabled(visible);
    ui_->labelLighting->setEnabled(visible);
    ui_->comboBoxLightingOptions->setEnabled(visible);
    ui_->labelColorScheme->setEnabled(visible);
    ui_->comboBoxColorScheme->setEnabled(visible);

    bool can_modify_default_color = visible && (ui_->comboBoxColorScheme->currentText() == "uniform color");
    ui_->labelDefaultColor->setEnabled(can_modify_default_color);
    ui_->toolButtonDefaultColor->setEnabled(can_modify_default_color);

    const auto &lighting_option = ui_->comboBoxLightingOptions->currentText();
    bool can_modify_back_color = visible && (lighting_option != "disabled") && ui_->comboBoxImposterStyle->currentText() != "sphere";
    ui_->labelBackColor->setEnabled(can_modify_back_color);
    ui_->checkBoxBackColor->setEnabled(can_modify_back_color);
    ui_->toolButtonBackColor->setEnabled(can_modify_back_color && d->distinct_back_color());

    bool can_modify_highlight = visible;
    ui_->labelHighlight->setEnabled(can_modify_highlight);
    ui_->checkBoxHighlight->setEnabled(can_modify_highlight);
    bool can_modify_highlight_range = can_modify_highlight && ui_->checkBoxHighlight->isChecked();
    ui_->spinBoxHighlightMin->setEnabled(can_modify_highlight_range);
    ui_->spinBoxHighlightMax->setEnabled(can_modify_highlight_range);

    // scalar field
    bool can_show_scalar = visible && ui_->comboBoxColorScheme->currentText().contains(scalar_prefix_);
    ui_->labelScalarFieldStyle->setEnabled(can_show_scalar);
    ui_->comboBoxScalarFieldStyle->setEnabled(can_show_scalar);
    if (ui_->comboBoxScalarFieldStyle->currentText().contains("random")) {
        ui_->labelScalarFieldDiscrete->setEnabled(false);
        ui_->checkBoxScalarFieldDiscrete->setEnabled(false);
        ui_->labelScalarFieldStripes->setEnabled(false);
        ui_->spinBoxScalarFieldNumStrips->setEnabled(true);
    }
    else {
        ui_->labelScalarFieldDiscrete->setEnabled(can_show_scalar);
        ui_->checkBoxScalarFieldDiscrete->setEnabled(can_show_scalar);
        bool can_change_num_strips = can_show_scalar && ui_->checkBoxScalarFieldDiscrete->isChecked();
        ui_->labelScalarFieldStripes->setEnabled(can_change_num_strips);
        ui_->spinBoxScalarFieldNumStrips->setEnabled(can_change_num_strips);
    }
    ui_->labelScalarFieldClamp->setEnabled(can_show_scalar);
    ui_->checkBoxScalarFieldClamp->setEnabled(can_show_scalar);
    bool can_edit_clamp = can_show_scalar && d->clamp_range();
    ui_->doubleSpinBoxScalarFieldClampLower->setEnabled(can_edit_clamp && ui_->checkBoxScalarFieldClamp->isChecked());
    ui_->doubleSpinBoxScalarFieldClampUpper->setEnabled(can_edit_clamp && ui_->checkBoxScalarFieldClamp->isChecked());

    // vector field
    bool can_show_vector = visible && ui_->comboBoxVectorField->currentText() != "not available";
    ui_->labelVectorField->setEnabled(can_show_vector);
    ui_->comboBoxVectorField->setEnabled(can_show_vector);
    bool can_modify_vector_style = can_show_vector && ui_->comboBoxVectorField->currentText() != "disabled";
    ui_->labelVectorFieldScale->setEnabled(can_modify_vector_style);
    ui_->doubleSpinBoxVectorFieldScale->setEnabled(can_modify_vector_style);

    update();
    qApp->processEvents();
}
