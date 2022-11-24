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


#include "widget_drawable_lines.h"

#include <QColorDialog>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/buffer.h>
#include <easy3d/util/logging.h>

#include "paint_canvas.h"
#include "main_window.h"
#include "widget_model_list.h"

#include "ui_widget_drawable_lines.h"


using namespace easy3d;


WidgetLinesDrawable::WidgetLinesDrawable(QWidget *parent)
        : WidgetDrawable(parent), ui_(new Ui::WidgetLinesDrawable)
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


void WidgetLinesDrawable::connectAll() {
    // which drawable
    connect(ui_->comboBoxDrawables, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setActiveDrawable(const QString &)));

    // visible
    connect(ui_->checkBoxVisible, SIGNAL(toggled(bool)), this, SLOT(setDrawableVisible(bool)));

    // thickness
    connect(ui_->doubleSpinBoxLineWidth, SIGNAL(valueChanged(double)), this, SLOT(setLineWidth(double)));

    // imposter
    connect(ui_->comboBoxImposterStyle, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setImposterStyle(const QString &)));

    // color scheme
    connect(ui_->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setColorScheme(const QString &)));

    // default color
    connect(ui_->toolButtonDefaultColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

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


void WidgetLinesDrawable::disconnectAll() {
    // which drawable
    disconnect(ui_->comboBoxDrawables, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setActiveDrawable(const QString &)));

    // visible
    disconnect(ui_->checkBoxVisible, SIGNAL(toggled(bool)), this, SLOT(setDrawableVisible(bool)));

    // thickness
    disconnect(ui_->doubleSpinBoxLineWidth, SIGNAL(valueChanged(double)), this, SLOT(setLineWidth(double)));

    // imposter
    disconnect(ui_->comboBoxImposterStyle, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setImposterStyle(const QString &)));

    // color scheme
    disconnect(ui_->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setColorScheme(const QString &)));

    // default color
    disconnect(ui_->toolButtonDefaultColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

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


WidgetLinesDrawable::~WidgetLinesDrawable() {
    delete ui_;
}


// update the panel to be consistent with the drawable's rendering parameters
void WidgetLinesDrawable::updatePanel() {
    auto model = viewer_->currentModel();
    if (!model)
        active_drawable_.clear();

    auto d = dynamic_cast<LinesDrawable*>(drawable());
    if (!model || !model->renderer()->is_visible() || !d) {
        setEnabled(false);
        return;
    }

    setEnabled(true);

    disconnectAll();

    auto &state = states_[d];
    auto &scheme = d->state();

    ui_->comboBoxDrawables->clear();
    const auto &drawables = model->renderer()->lines_drawables();
    for (auto dd : drawables)
        ui_->comboBoxDrawables->addItem(QString::fromStdString(dd->name()));
    ui_->comboBoxDrawables->setCurrentText(QString::fromStdString(d->name()));

    // visible
    ui_->checkBoxVisible->setChecked(d->is_visible());
    window_->widgetModelList()->updateDrawableVisibilities();

    // thickness
    ui_->doubleSpinBoxLineWidth->setValue(d->line_width());

    // imposter
    switch (d->impostor_type()) {
        case LinesDrawable::PLAIN:
            ui_->comboBoxImposterStyle->setCurrentText("plain");
            break;
        case LinesDrawable::CYLINDER:
            ui_->comboBoxImposterStyle->setCurrentText("cylinder");
            break;
        case LinesDrawable::CONE:
            ui_->comboBoxImposterStyle->setCurrentText("cone");
            break;
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
        const auto& c = d->color();
        QPixmap pixmap(ui_->toolButtonDefaultColor->size());
        pixmap.fill(
                QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui_->toolButtonDefaultColor->setIcon(QIcon(pixmap));
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


Drawable *WidgetLinesDrawable::drawable() {
    auto model = viewer_->currentModel();
    if (!model) {
        active_drawable_.clear();
        return nullptr;
    }

    auto pos = active_drawable_.find(model);
    if (pos != active_drawable_.end())
        return model->renderer()->get_lines_drawable(pos->second);
    else {
        for (auto d : model->renderer()->lines_drawables() ) {
            active_drawable_[model] = d->name();
            return d;
        }
        return nullptr;
    }
}


void WidgetLinesDrawable::setActiveDrawable(const QString &text) {
    auto model = viewer_->currentModel();
    const std::string &name = text.toStdString();

    if (active_drawable_.find(model) != active_drawable_.end()) {
        if (active_drawable_[model] == name)
            return; // already active
    }

    if (model->renderer()->get_lines_drawable(name)) {
        active_drawable_[model] = name;
    } else {
        LOG(ERROR) << "drawable '" << name << "' not defined on model: " << model->name();
        const auto &drawables = model->renderer()->lines_drawables();
        if (drawables.empty())
            LOG(ERROR) << "no lines drawable defined on model: " << model->name();
        else
            active_drawable_[model] = drawables[0]->name();
    }

    updatePanel();
}


void WidgetLinesDrawable::setLineWidth(double w) {
    auto d = dynamic_cast<LinesDrawable*>(drawable());
    if (d->line_width() != w) {
        d->set_line_width(w);
        viewer_->update();
    }
}


void WidgetLinesDrawable::setImposterStyle(const QString &style) {
    auto d = dynamic_cast<LinesDrawable*>(drawable());
    if (style == "plain") {
        if (d->impostor_type() != LinesDrawable::PLAIN)
            d->set_impostor_type(LinesDrawable::PLAIN);
    } else if (style == "cylinder") {
        if (d->impostor_type() != LinesDrawable::CYLINDER)
            d->set_impostor_type(LinesDrawable::CYLINDER);
    } else if (style == "cone") {
        if (d->impostor_type() != LinesDrawable::CONE)
            d->set_impostor_type(LinesDrawable::CONE);
    }

    viewer_->update();
    disableUnavailableOptions();
}


void WidgetLinesDrawable::setColorScheme(const QString &text) {
    auto d = drawable();
    auto& scheme = d->state();
    scheme.set_clamp_range(ui_->checkBoxScalarFieldClamp->isChecked());
    scheme.set_clamp_lower(ui_->doubleSpinBoxScalarFieldClampLower->value() / 100.0);
    scheme.set_clamp_upper(ui_->doubleSpinBoxScalarFieldClampUpper->value() / 100.0);
    states_[d].scalar_style = ui_->comboBoxScalarFieldStyle->currentIndex();

    WidgetDrawable::setColorScheme(text);
}


void WidgetLinesDrawable::setDefaultColor() {
    const auto &c = drawable()->color();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    const QColor &color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec4 new_color(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        drawable()->set_uniform_coloring(new_color);
        viewer_->update();

        QPixmap pixmap(ui_->toolButtonDefaultColor->size());
        pixmap.fill(color);
        ui_->toolButtonDefaultColor->setIcon(QIcon(pixmap));
    }
}


namespace internal {

    // color schemes from scalar fields defined on edges
    template <typename MODEL>
    void color_schemes_for_scalar_fields(MODEL* model, const QString& scalar_prefix, std::vector<QString>& schemes) {
        // color schemes from color properties and texture
        for (const auto &name : model->vertex_properties()) {
            if (name.find("v:color") != std::string::npos || name.find("v:texcoord") != std::string::npos)
                schemes.push_back(QString::fromStdString(name));
        }
        for (const auto &name : model->edge_properties()) {
            if (name.find("e:color") != std::string::npos || name.find("e:texcoord") != std::string::npos)
                schemes.push_back(QString::fromStdString(name));
        }

        // color schemes from scalar fields defined on edges
        for (const auto &name : model->edge_properties()) {
            if (model->template get_edge_property<float>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_edge_property<double>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_edge_property<unsigned int>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_edge_property<int>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_edge_property<unsigned char>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_edge_property<char>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_edge_property<bool>(name) && name == "e:select")
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
        }
        // scalar fields defined on vertices
        for (const auto &name : model->vertex_properties()) {
            if (model->template get_vertex_property<float>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_vertex_property<double>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_vertex_property<unsigned int>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_vertex_property<int>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_vertex_property<unsigned char>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
            else if (model->template get_vertex_property<char>(name))
                schemes.push_back(scalar_prefix + QString::fromStdString(name));
        }
    }

    // vector fields defined on edges
    template <typename MODEL>
    void vector_fields_on_edges(MODEL* model, std::vector<QString>& fields) {
        for (const auto &name : model->edge_properties()) {
            if (model->template get_edge_property<vec3>(name)) {
                if (name != "e:color")
                    fields.push_back(QString::fromStdString(name));
            }
        }
    }

}


std::vector<QString> WidgetLinesDrawable::colorSchemes(const easy3d::Model *model) {
    std::vector<QString> schemes;
    schemes.emplace_back("uniform color");

    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (mesh)
        internal::color_schemes_for_scalar_fields(mesh, scalar_prefix_, schemes);

    auto graph = dynamic_cast<Graph *>(viewer_->currentModel());
    if (graph)
        internal::color_schemes_for_scalar_fields(graph, scalar_prefix_, schemes);

    auto poly = dynamic_cast<PolyMesh *>(viewer_->currentModel());
    if (poly)
        internal::color_schemes_for_scalar_fields(poly, scalar_prefix_, schemes);

    return schemes;
}


std::vector<QString> WidgetLinesDrawable::vectorFields(const easy3d::Model *model) {
    std::vector<QString> fields;

    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (mesh)
        internal::vector_fields_on_edges(mesh, fields);

    else {
        auto graph = dynamic_cast<Graph *>(viewer_->currentModel());
        if (graph)
            internal::vector_fields_on_edges(graph, fields);

        else {
            auto poly = dynamic_cast<PolyMesh *>(viewer_->currentModel());
            if (poly)
                internal::vector_fields_on_edges(poly, fields);
        }
    }

    // if no vector fields found, add a "not available" item
    if (fields.empty())
        fields.emplace_back("not available");
    else   // add one allowing to disable vector fields
        fields.insert(fields.begin(), "disabled");

    return fields;
}


void WidgetLinesDrawable::setVectorField(const QString &text) {
    auto model = viewer_->currentModel();
    auto mesh = dynamic_cast<SurfaceMesh *>(model);
    if (!mesh)
        return;

    if (text == "disabled") {
        const auto &drawables = mesh->renderer()->lines_drawables();
        for (auto d : drawables) {
            if (d->name().find("vector - ") != std::string::npos)
                d->set_visible(false);
        }
        states_[drawable()].vector_field = "disabled";
    } else {
        const std::string &name = text.toStdString();
        updateVectorFieldBuffer(mesh, name);

        auto d = mesh->renderer()->get_lines_drawable("vector - " + name);
        if (d) {
            d->set_visible(true);
            states_[drawable()].vector_field = text;
        }
    }

    window_->updateUi();
    viewer_->update();
}


void WidgetLinesDrawable::updateVectorFieldBuffer(Model *model, const std::string &name) {
    // a vector field is visualized as a LinesDrawable whose name is the same as the vector field
    auto drawable = model->renderer()->get_lines_drawable("vector - " + name);
    if (!drawable) {
        drawable = model->renderer()->add_lines_drawable("vector - " + name);
        drawable->set_update_func([&, name](Model *m, Drawable *d) -> void {
            const float scale = ui_->doubleSpinBoxVectorFieldScale->value();
            if (dynamic_cast<SurfaceMesh *>(m))
                buffer::update(dynamic_cast<SurfaceMesh*>(m), dynamic_cast<LinesDrawable*>(d), name, State::EDGE, scale);
        });
    }
}


void WidgetLinesDrawable::disableUnavailableOptions() {
    auto d = drawable();

    bool visible = ui_->checkBoxVisible->isChecked();
    ui_->labelLineWidth->setEnabled(visible);
    ui_->doubleSpinBoxLineWidth->setEnabled(visible);
    ui_->labelImposterStyle->setEnabled(visible);
    ui_->comboBoxImposterStyle->setEnabled(visible);
    ui_->labelColorScheme->setEnabled(visible);
    ui_->comboBoxColorScheme->setEnabled(visible);

    bool can_modify_default_color = visible && (ui_->comboBoxColorScheme->currentText() == "uniform color");
    ui_->labelDefaultColor->setEnabled(can_modify_default_color);
    ui_->toolButtonDefaultColor->setEnabled(can_modify_default_color);

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
