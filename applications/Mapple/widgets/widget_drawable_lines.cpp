#include "widget_drawable_lines.h"

#include <QColorDialog>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/buffers.h>
#include <easy3d/util/logging.h>

#include "paint_canvas.h"
#include "main_window.h"

#include "ui_widget_drawable_lines.h"


using namespace easy3d;


WidgetLinesDrawable::WidgetLinesDrawable(QWidget *parent)
        : WidgetDrawable(parent), ui(new Ui::WidgetLinesDrawable) {
    ui->setupUi(this);

    if (colormaps_.empty())
        ui->comboBoxScalarFieldStyle->addItem("not available");
    else {
        for (const auto &colormap : colormaps_)
            ui->comboBoxScalarFieldStyle->addItem(QIcon(QString::fromStdString(colormap.file)),
                                                  QString::fromStdString("  " + colormap.name));
    }
}


void WidgetLinesDrawable::connectAll() {
    // which drawable
    connect(ui->comboBoxDrawables, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setActiveDrawable(const QString &)));

    // visible
    connect(ui->checkBoxVisible, SIGNAL(toggled(bool)), this, SLOT(setDrawableVisible(bool)));

    // thickness
    connect(ui->doubleSpinBoxLineWidth, SIGNAL(valueChanged(double)), this, SLOT(setLineWidth(double)));

    // imposter
    connect(ui->comboBoxImposterStyle, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setImposterStyle(const QString &)));

    // color scheme
    connect(ui->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setColorScheme(const QString &)));

    // default color
    connect(ui->toolButtonDefaultColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

    // highlight
    connect(ui->checkBoxHighlight, SIGNAL(toggled(bool)), this, SLOT(setHighlight(bool)));
    connect(ui->spinBoxHighlightMin, SIGNAL(valueChanged(int)), this, SLOT(setHighlightMin(int)));
    connect(ui->spinBoxHighlightMax, SIGNAL(valueChanged(int)), this, SLOT(setHighlightMax(int)));

    // scalar field
    connect(ui->comboBoxScalarFieldStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(setScalarFieldStyle(int)));
    connect(ui->checkBoxScalarFieldDiscrete, SIGNAL(toggled(bool)), this, SLOT(setScalarFieldDiscreteColors(bool)));
    connect(ui->spinBoxScalarFieldNumStrips, SIGNAL(valueChanged(int)), this, SLOT(setScalarFieldNumOfStripes(int)));
    connect(ui->checkBoxScalarFieldClamp, SIGNAL(toggled(bool)), this, SLOT(setScalarFieldClamp(bool)));
    connect(ui->doubleSpinBoxScalarFieldClampLower, SIGNAL(valueChanged(double)), this, SLOT(setScalarFieldClampLower(double)));
    connect(ui->doubleSpinBoxScalarFieldClampUpper, SIGNAL(valueChanged(double)), this, SLOT(setScalarFieldClampUpper(double)));

    // vector field
    connect(ui->comboBoxVectorField, SIGNAL(currentIndexChanged(const QString&)), this,
            SLOT(setVectorField(const QString&)));
    connect(ui->doubleSpinBoxVectorFieldScale, SIGNAL(valueChanged(double)), this, SLOT(setVectorFieldScale(double)));
}


void WidgetLinesDrawable::disconnectAll() {
    // which drawable
    disconnect(ui->comboBoxDrawables, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setActiveDrawable(const QString &)));

    // visible
    disconnect(ui->checkBoxVisible, SIGNAL(toggled(bool)), this, SLOT(setDrawableVisible(bool)));

    // thickness
    disconnect(ui->doubleSpinBoxLineWidth, SIGNAL(valueChanged(double)), this, SLOT(setLineWidth(double)));

    // imposter
    disconnect(ui->comboBoxImposterStyle, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setImposterStyle(const QString &)));

    // color scheme
    disconnect(ui->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setColorScheme(const QString &)));

    // default color
    disconnect(ui->toolButtonDefaultColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

    // highlight
    disconnect(ui->checkBoxHighlight, SIGNAL(toggled(bool)), this, SLOT(setHighlight(bool)));
    disconnect(ui->spinBoxHighlightMin, SIGNAL(valueChanged(int)), this, SLOT(setHighlightMin(int)));
    disconnect(ui->spinBoxHighlightMax, SIGNAL(valueChanged(int)), this, SLOT(setHighlightMax(int)));

    // scalar field
    disconnect(ui->comboBoxScalarFieldStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(setScalarFieldStyle(int)));
    disconnect(ui->checkBoxScalarFieldDiscrete, SIGNAL(toggled(bool)), this, SLOT(setScalarFieldDiscreteColors(bool)));
    disconnect(ui->spinBoxScalarFieldNumStrips, SIGNAL(valueChanged(int)), this, SLOT(setScalarFieldNumOfStripes(int)));
    disconnect(ui->checkBoxScalarFieldClamp, SIGNAL(toggled(bool)), this, SLOT(setScalarFieldClamp(bool)));
    disconnect(ui->doubleSpinBoxScalarFieldClampLower, SIGNAL(valueChanged(double)), this, SLOT(setScalarFieldClampLower(double)));
    disconnect(ui->doubleSpinBoxScalarFieldClampUpper, SIGNAL(valueChanged(double)), this, SLOT(setScalarFieldClampUpper(double)));

    // vector field
    disconnect(ui->comboBoxVectorField, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setVectorField(const QString&)));
    disconnect(ui->doubleSpinBoxVectorFieldScale, SIGNAL(valueChanged(double)), this, SLOT(setVectorFieldScale(double)));
}


WidgetLinesDrawable::~WidgetLinesDrawable() {
    delete ui;
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

    ui->comboBoxDrawables->clear();
    const auto &drawables = model->renderer()->lines_drawables();
    for (auto d : drawables)
        ui->comboBoxDrawables->addItem(QString::fromStdString(d->name()));
    ui->comboBoxDrawables->setCurrentText(QString::fromStdString(d->name()));

    // visible
    ui->checkBoxVisible->setChecked(d->is_visible());

    // thickness
    ui->doubleSpinBoxLineWidth->setValue(d->line_width());

    // imposter
    switch (d->impostor_type()) {
        case LinesDrawable::PLAIN:
            ui->comboBoxImposterStyle->setCurrentText("plain");
            break;
        case LinesDrawable::CYLINDER:
            ui->comboBoxImposterStyle->setCurrentText("cylinder");
            break;
        case LinesDrawable::CONE:
            ui->comboBoxImposterStyle->setCurrentText("cone");
            break;
    }

    {   // color scheme
        ui->comboBoxColorScheme->clear();
        const std::vector<QString> &schemes = colorSchemes(viewer_->currentModel());
        for (const auto &scheme : schemes)
            ui->comboBoxColorScheme->addItem(scheme);

        for (const auto& name : schemes) {
            if (name.contains(QString::fromStdString(scheme.property_name()))) {
                ui->comboBoxColorScheme->setCurrentText(name);
                break;
            }
        }

        // default color
        vec3 c = d->color();
        QPixmap pixmap(ui->toolButtonDefaultColor->size());
        pixmap.fill(
                QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonDefaultColor->setIcon(QIcon(pixmap));
    }

    {   // highlight
        bool highlight = d->highlight();
        ui->checkBoxHighlight->setChecked(highlight);

        const auto &range = d->highlight_range();
        ui->spinBoxHighlightMin->setValue(range.first);
        ui->spinBoxHighlightMax->setValue(range.second);
    }

    {   // scalar field
        ui->comboBoxScalarFieldStyle->setCurrentIndex(state.scalar_style);
        ui->checkBoxScalarFieldDiscrete->setChecked(states_[d].discrete_color);
        ui->spinBoxScalarFieldNumStrips->setValue(states_[d].num_stripes);
        const auto& coloring = ui->comboBoxColorScheme->currentText().toStdString();
        if (coloring.find("scalar - ") != std::string::npos && coloring.find(scheme.property_name()) != std::string::npos) {
            auto tex = colormapTexture(states_[d].scalar_style, states_[d].discrete_color, states_[d].num_stripes);
            if (tex)
                d->set_texture(tex);
        }
        ui->checkBoxScalarFieldClamp->setChecked(scheme.clamp_range());
        ui->doubleSpinBoxScalarFieldClampLower->setValue(scheme.clamp_lower() * 100);
        ui->doubleSpinBoxScalarFieldClampUpper->setValue(scheme.clamp_upper() * 100);
    }

    {   // vector field
        ui->comboBoxVectorField->clear();
        const std::vector<QString> &fields = vectorFields(viewer_->currentModel());
        for (auto name : fields)
            ui->comboBoxVectorField->addItem(name);

        ui->comboBoxVectorField->setCurrentText(state.vector_field);
        ui->doubleSpinBoxVectorFieldScale->setValue(state.vector_field_scale);
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
    scheme.set_clamp_range(ui->checkBoxScalarFieldClamp->isChecked());
    scheme.set_clamp_lower(ui->doubleSpinBoxScalarFieldClampLower->value() / 100.0);
    scheme.set_clamp_upper(ui->doubleSpinBoxScalarFieldClampUpper->value() / 100.0);
    states_[d].scalar_style = ui->comboBoxScalarFieldStyle->currentIndex();

    WidgetDrawable::setColorScheme(text);
}


void WidgetLinesDrawable::setDefaultColor() {
    const vec3 &c = drawable()->color();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    const QColor &color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec4 new_color(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        drawable()->set_uniform_coloring(new_color);
        viewer_->update();

        QPixmap pixmap(ui->toolButtonDefaultColor->size());
        pixmap.fill(color);
        ui->toolButtonDefaultColor->setIcon(QIcon(pixmap));
    }
}


namespace lines_details {

    // color schemes from scalar fields
    // scalar fields defined on edges
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

        // color schemes from scalar fields
        // scalar fields defined on edges
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
    schemes.push_back("uniform color");

    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (mesh)
        lines_details::color_schemes_for_scalar_fields(mesh, scalar_prefix_, schemes);

    auto graph = dynamic_cast<Graph *>(viewer_->currentModel());
    if (graph)
        lines_details::color_schemes_for_scalar_fields(graph, scalar_prefix_, schemes);

    auto poly = dynamic_cast<PolyMesh *>(viewer_->currentModel());
    if (poly)
        lines_details::color_schemes_for_scalar_fields(poly, scalar_prefix_, schemes);

    return schemes;
}


std::vector<QString> WidgetLinesDrawable::vectorFields(const easy3d::Model *model) {
    std::vector<QString> fields;

    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (mesh)
        lines_details::vector_fields_on_edges(mesh, fields);

    else {
        auto graph = dynamic_cast<Graph *>(viewer_->currentModel());
        if (graph)
            lines_details::vector_fields_on_edges(graph, fields);

        else {
            auto poly = dynamic_cast<PolyMesh *>(viewer_->currentModel());
            if (poly)
                lines_details::vector_fields_on_edges(poly, fields);
        }
    }

    // if no vector fields found, add a "not available" item
    if (fields.empty())
        fields.push_back("not available");
    else   // add one allowing to disable vector fields
        fields.insert(fields.begin(), "disabled");

    return fields;
}


void WidgetLinesDrawable::setVectorField(const QString &text) {
    auto model = viewer_->currentModel();
    SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(model);
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
            const float scale = ui->doubleSpinBoxVectorFieldScale->value();
            if (dynamic_cast<SurfaceMesh *>(m))
                buffers::update(dynamic_cast<SurfaceMesh*>(m), dynamic_cast<LinesDrawable*>(d), name, State::EDGE, scale);
        });
    }
}


void WidgetLinesDrawable::disableUnavailableOptions() {
    auto d = drawable();

    bool visible = ui->checkBoxVisible->isChecked();
    ui->labelLineWidth->setEnabled(visible);
    ui->doubleSpinBoxLineWidth->setEnabled(visible);
    ui->labelImposterStyle->setEnabled(visible);
    ui->comboBoxImposterStyle->setEnabled(visible);
    ui->labelColorScheme->setEnabled(visible);
    ui->comboBoxColorScheme->setEnabled(visible);

    bool can_modify_default_color = visible && (ui->comboBoxColorScheme->currentText() == "uniform color");
    ui->labelDefaultColor->setEnabled(can_modify_default_color);
    ui->toolButtonDefaultColor->setEnabled(can_modify_default_color);

    bool can_modify_highlight = visible;
    ui->labelHighlight->setEnabled(can_modify_highlight);
    ui->checkBoxHighlight->setEnabled(can_modify_highlight);
    bool can_modify_highlight_range = can_modify_highlight && ui->checkBoxHighlight->isChecked();
    ui->spinBoxHighlightMin->setEnabled(can_modify_highlight_range);
    ui->spinBoxHighlightMax->setEnabled(can_modify_highlight_range);

    // scalar field
    bool can_show_scalar = visible && ui->comboBoxColorScheme->currentText().contains(scalar_prefix_);
    ui->labelScalarFieldStyle->setEnabled(can_show_scalar);
    ui->comboBoxScalarFieldStyle->setEnabled(can_show_scalar);
    if (ui->comboBoxScalarFieldStyle->currentText().contains("random")) {
        ui->labelScalarFieldDiscrete->setEnabled(false);
        ui->checkBoxScalarFieldDiscrete->setEnabled(false);
        ui->labelScalarFieldStripes->setEnabled(false);
        ui->spinBoxScalarFieldNumStrips->setEnabled(true);
    }
    else {
        ui->labelScalarFieldDiscrete->setEnabled(can_show_scalar);
        ui->checkBoxScalarFieldDiscrete->setEnabled(can_show_scalar);
        bool can_change_num_strips = can_show_scalar && ui->checkBoxScalarFieldDiscrete->isChecked();
        ui->labelScalarFieldStripes->setEnabled(can_change_num_strips);
        ui->spinBoxScalarFieldNumStrips->setEnabled(can_change_num_strips);
    }
    ui->labelScalarFieldClamp->setEnabled(can_show_scalar);
    ui->checkBoxScalarFieldClamp->setEnabled(can_show_scalar);
    bool can_edit_clamp = can_show_scalar && d->clamp_range();
    ui->doubleSpinBoxScalarFieldClampLower->setEnabled(can_edit_clamp && ui->checkBoxScalarFieldClamp->isChecked());
    ui->doubleSpinBoxScalarFieldClampUpper->setEnabled(can_edit_clamp && ui->checkBoxScalarFieldClamp->isChecked());

    // vector field
    bool can_show_vector = visible && ui->comboBoxVectorField->currentText() != "not available";
    ui->labelVectorField->setEnabled(can_show_vector);
    ui->comboBoxVectorField->setEnabled(can_show_vector);
    bool can_modify_vector_style = can_show_vector && ui->comboBoxVectorField->currentText() != "disabled";
    ui->labelVectorFieldScale->setEnabled(can_modify_vector_style);
    ui->doubleSpinBoxVectorFieldScale->setEnabled(can_modify_vector_style);

    update();
    qApp->processEvents();
}
