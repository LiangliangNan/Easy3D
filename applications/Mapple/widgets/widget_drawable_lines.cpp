#include "widget_drawable_lines.h"

#include <QColorDialog>

#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/model.h>
#include <easy3d/viewer/texture_manager.h>
#include <easy3d/viewer/renderer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/file_system.h>
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
    if (!model || !model->is_visible() || model->lines_drawables().empty()) {
        setEnabled(false);
        return;
    }

    setEnabled(true);

    disconnectAll();

    auto &state = states_[drawable()];

    ui->comboBoxDrawables->clear();
    const auto &drawables = model->lines_drawables();
    for (auto d : drawables)
        ui->comboBoxDrawables->addItem(QString::fromStdString(d->name()));
    ui->comboBoxDrawables->setCurrentText(QString::fromStdString(drawable()->name()));

    // visible
    ui->checkBoxVisible->setChecked(drawable()->is_visible());

    // thickness
    ui->doubleSpinBoxLineWidth->setValue(drawable()->line_width());

    // imposter
    switch (drawable()->impostor_type()) {
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
        const std::vector<std::string> &schemes = colorSchemes(viewer_->currentModel());
        for (const auto &scheme : schemes)
            ui->comboBoxColorScheme->addItem(QString::fromStdString(scheme));

        ui->comboBoxColorScheme->setCurrentText(QString::fromStdString(state.coloring));

        // default color
        vec3 c = drawable()->default_color();
        QPixmap pixmap(ui->toolButtonDefaultColor->size());
        pixmap.fill(
                QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonDefaultColor->setIcon(QIcon(pixmap));

//        // texture
//        auto tex = drawable()->texture();
//        if (tex) {
//            const std::string &tex_name = file_system::simple_name(tex->file_name());
//            ui->lineEditTextureFile->setText(QString::fromStdString(tex_name));
//        }
//        else
//            ui->lineEditTextureFile->setText("");
//
//        ui->spinBoxTextureRepeat->setValue(drawable()->texture_repeat());
//        ui->spinBoxTextureFractionalRepeat->setValue(drawable()->texture_fractional_repeat());
    }

    {   // highlight
        bool highlight = drawable()->highlight();
        ui->checkBoxHighlight->setChecked(highlight);

        const auto &range = drawable()->highlight_range();
        ui->spinBoxHighlightMin->setValue(range.first);
        ui->spinBoxHighlightMax->setValue(range.second);
    }

    {   // scalar field
        ui->comboBoxScalarFieldStyle->setCurrentIndex(state.scalar_style);
        ui->checkBoxScalarFieldClamp->setChecked(state.clamp_value);
        ui->doubleSpinBoxScalarFieldClampLower->setValue(state.clamp_value_lower);
        ui->doubleSpinBoxScalarFieldClampUpper->setValue(state.clamp_value_upper);
    }

    {   // vector field
        ui->comboBoxVectorField->clear();
        const std::vector<std::string> &fields = vectorFields(viewer_->currentModel());
        for (auto name : fields)
            ui->comboBoxVectorField->addItem(QString::fromStdString(name));

        ui->comboBoxVectorField->setCurrentText(QString::fromStdString(state.vector_field));
        ui->doubleSpinBoxVectorFieldScale->setValue(state.vector_field_scale);
    }

    disableUnavailableOptions();

    connectAll();

    state.initialized = true;
}


// update the OpenGL buffers
void WidgetLinesDrawable::updateRendering() {
    if (!drawable())
        return;
    setColorScheme(ui->comboBoxColorScheme->currentText());
}


LinesDrawable *WidgetLinesDrawable::drawable() {
    auto model = viewer_->currentModel();
    auto pos = active_drawable_.find(model);
    if (pos != active_drawable_.end())
        return model->lines_drawable(pos->second);
    else {
        const auto &drawables = model->lines_drawables();
        if (drawables.empty())
            return nullptr;
        else {
            active_drawable_[model] = drawables[0]->name();
            return drawables[0];
        }
    }
}


void WidgetLinesDrawable::setActiveDrawable(const QString &text) {
    auto model = viewer_->currentModel();
    const std::string &name = text.toStdString();

    if (active_drawable_.find(model) != active_drawable_.end()) {
        if (active_drawable_[model] == name)
            return; // already active
    }

    if (model->lines_drawable(name)) {
        active_drawable_[model] = name;
    } else {
        LOG(ERROR) << "drawable '" << name << "' not defined on model: " << model->name();
        const auto &drawables = model->lines_drawables();
        if (drawables.empty())
            LOG(ERROR) << "no lines drawable defined on model: " << model->name();
        else
            active_drawable_[model] = drawables[0]->name();
    }

    updatePanel();
}


void WidgetLinesDrawable::setDrawableVisible(bool b) {
    if (drawable()->is_visible() != b) {
        drawable()->set_visible(b);
        viewer_->update();
    }
    disableUnavailableOptions();
}


void WidgetLinesDrawable::setLineWidth(double w) {
    if (drawable()->line_width() != w) {
        drawable()->set_line_width(w);
        viewer_->update();
    }
}


void WidgetLinesDrawable::setImposterStyle(const QString &style) {
    if (style == "plain") {
        if (drawable()->impostor_type() != LinesDrawable::PLAIN)
            drawable()->set_impostor_type(LinesDrawable::PLAIN);
    } else if (style == "cylinder") {
        if (drawable()->impostor_type() != LinesDrawable::CYLINDER)
            drawable()->set_impostor_type(LinesDrawable::CYLINDER);
    } else if (style == "cone") {
        if (drawable()->impostor_type() != LinesDrawable::CONE)
            drawable()->set_impostor_type(LinesDrawable::CONE);
    }

    viewer_->update();
    disableUnavailableOptions();
}


namespace details {

    template<typename MODEL>
    inline void setup_scalar_field(MODEL *model, LinesDrawable *drawable, const std::string &color_scheme, float dummy_lower, float dummy_upper) {
        for (const auto &name : model->edge_properties()) {
            if (color_scheme.find(name) != std::string::npos) {
                if (model->template get_edge_property<float>(name)) {
                    renderer::update_buffer(model, drawable, model->template get_edge_property<float>(name), dummy_lower, dummy_upper);
                    return;
                }
                if (model->template get_edge_property<double>(name)) {
                    renderer::update_buffer(model, drawable, model->template get_edge_property<double>(name), dummy_lower, dummy_upper);
                    return;
                }
                if (model->template get_edge_property<unsigned int>(name)) {
                    renderer::update_buffer(model, drawable, model->template get_edge_property<unsigned int>(name), dummy_lower, dummy_upper);
                    return;
                }
                if (model->template get_edge_property<int>(name)) {
                    renderer::update_buffer(model, drawable, model->template get_edge_property<int>(name), dummy_lower, dummy_upper);
                    return;
                }
                if (model->template get_edge_property<bool>(name)) {
                    renderer::update_buffer(model, drawable, model->template get_edge_property<bool>(name), dummy_lower, dummy_upper);
                    return;
                }
            }
        }

        for (const auto &name : model->vertex_properties()) {
            if (color_scheme.find(name) != std::string::npos) {
                if (model->template get_vertex_property<float>(name)) {
                    renderer::update_buffer(model, drawable, model->template get_vertex_property<float>(name), dummy_lower, dummy_upper);
                    return;
                }
                if (model->template get_vertex_property<double>(name)) {
                    renderer::update_buffer(model, drawable, model->template get_vertex_property<double>(name), dummy_lower, dummy_upper);
                    return;
                }
                if (model->template get_vertex_property<unsigned int>(name)) {
                    renderer::update_buffer(model, drawable, model->template get_vertex_property<unsigned int>(name), dummy_lower, dummy_upper);
                    return;
                }
                if (model->template get_vertex_property<int>(name)) {
                    renderer::update_buffer(model, drawable, model->template get_vertex_property<int>(name), dummy_lower, dummy_upper);
                    return;
                }
                if (model->template get_vertex_property<bool>(name)) {
                    renderer::update_buffer(model, drawable, model->template get_vertex_property<bool>(name), dummy_lower, dummy_upper);
                    return;
                }
            }
        }
    }
}

void WidgetLinesDrawable::setColorScheme(const QString &text) {
    disableUnavailableOptions();

//    auto tex = drawable()->texture();
//    if (tex) {
//        const std::string &tex_name = file_system::simple_name(tex->file_name());
//        ui->lineEditTextureFile->setText(QString::fromStdString(tex_name));
//    }
//    else
//        ui->lineEditTextureFile->setText("");

    drawable()->set_per_vertex_color(text != "uniform color");

    bool is_scalar_field = text.contains("scalar - ");
    if (is_scalar_field) {
        float clamp_lower = 0.0f, clamp_upper = 0.0f;
        if (ui->checkBoxScalarFieldClamp->isChecked()) {
            clamp_lower = ui->doubleSpinBoxScalarFieldClampLower->value() / 100.0f;
            clamp_upper = ui->doubleSpinBoxScalarFieldClampUpper->value() / 100.0f;
        }

        viewer_->makeCurrent();
        if (dynamic_cast<SurfaceMesh *>(viewer_->currentModel())) {
            SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
            details::setup_scalar_field(mesh, drawable(), text.toStdString(), clamp_lower, clamp_upper);
            drawable()->set_texture(colormapTexture(ui->comboBoxScalarFieldStyle->currentIndex()));
        } else if (dynamic_cast<Graph *>(viewer_->currentModel())) {
            Graph *graph = dynamic_cast<Graph *>(viewer_->currentModel());
            details::setup_scalar_field(graph, drawable(), text.toStdString(), clamp_lower, clamp_upper);
            drawable()->set_texture(colormapTexture(ui->comboBoxScalarFieldStyle->currentIndex()));
        }
//        else if (dynamic_cast<PointCloud*>(viewer_->currentModel())) {
//            PointCloud* cloud = dynamic_cast<PointCloud*>(viewer_->currentModel());
//            details::setup_scalar_field(cloud, drawable(), text.toStdString());
//            drawable()->set_texture(colormapTexture(ui->comboBoxScalarFieldStyle->currentIndex()));
//        }
        viewer_->doneCurrent();
    }

    bool use_texture = (text.contains(":texcoord") || is_scalar_field);
    drawable()->set_use_texture(use_texture);

    viewer_->update();
    states_[drawable()].coloring = text.toStdString();
}


void WidgetLinesDrawable::setDefaultColor() {
    const vec3 &c = drawable()->default_color();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    const QColor &color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec3 new_color(color.redF(), color.greenF(), color.blueF());
        drawable()->set_default_color(new_color);
        viewer_->update();

        QPixmap pixmap(ui->toolButtonDefaultColor->size());
        pixmap.fill(color);
        ui->toolButtonDefaultColor->setIcon(QIcon(pixmap));
    }
}


void WidgetLinesDrawable::setHighlight(bool b) {
    if (drawable()->highlight() != b) {
        drawable()->set_highlight(b);
        viewer_->update();
        disableUnavailableOptions();
    }
}


void WidgetLinesDrawable::setHighlightMin(int v) {
    const auto &range = drawable()->highlight_range();
    if (range.first != v) {
        drawable()->set_highlight_range(std::make_pair(v, range.second));
        viewer_->update();
    }
}


void WidgetLinesDrawable::setHighlightMax(int v) {
    const auto &range = drawable()->highlight_range();
    if (range.second != v) {
        drawable()->set_highlight_range(std::make_pair(range.first, v));
        viewer_->update();
    }
}


void WidgetLinesDrawable::setScalarFieldStyle(int idx) {
    auto tex = colormapTexture(idx);
    drawable()->set_texture(tex);
    drawable()->set_use_texture(true);
    viewer_->update();
    states_[drawable()].scalar_style = idx;
}


void WidgetLinesDrawable::setScalarFieldClamp(bool b) {
    auto& state = states_[drawable()];
    state.clamp_value = b;

    setColorScheme(ui->comboBoxColorScheme->currentText());
}


void WidgetLinesDrawable::setScalarFieldClampLower(double v) {
    auto& state = states_[drawable()];
    state.clamp_value_lower = v;

    setColorScheme(ui->comboBoxColorScheme->currentText());
}


void WidgetLinesDrawable::setScalarFieldClampUpper(double v) {
    auto& state = states_[drawable()];
    state.clamp_value_upper = v;

    setColorScheme(ui->comboBoxColorScheme->currentText());
}


std::vector<std::string> WidgetLinesDrawable::colorSchemes(const easy3d::Model *model) {
    std::vector<std::string> schemes;
    schemes.push_back("uniform color");

    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (mesh) {
        if (mesh->get_vertex_property<vec3>("v:color"))
            schemes.push_back("v:color");
        if (mesh->get_edge_property<vec3>("e:color"))
            schemes.push_back("e:color");
        if (mesh->get_vertex_property<vec2>("v:texcoord"))
            schemes.push_back("v:texcoord");
        if (mesh->get_halfedge_property<vec2>("h:texcoord"))
            schemes.push_back("h:texcoord");

        // color schemes from scalar fields
        // scalar fields defined on edges
        for (const auto &name : mesh->edge_properties()) {
            if (mesh->get_edge_property<float>(name))
                schemes.push_back("scalar - " + name);
            else if (mesh->get_edge_property<double>(name))
                schemes.push_back("scalar - " + name);
            else if (mesh->get_edge_property<unsigned int>(name))
                schemes.push_back("scalar - " + name);
            else if (mesh->get_edge_property<int>(name))
                schemes.push_back("scalar - " + name);
        }
        // scalar fields defined on vertices
        for (const auto &name : mesh->vertex_properties()) {
            if (mesh->get_vertex_property<float>(name))
                schemes.push_back("scalar - " + name);
            else if (mesh->get_vertex_property<double>(name))
                schemes.push_back("scalar - " + name);
            else if (mesh->get_vertex_property<unsigned int>(name))
                schemes.push_back("scalar - " + name);
            else if (mesh->get_vertex_property<int>(name))
                schemes.push_back("scalar - " + name);
        }
    }

    auto graph = dynamic_cast<Graph *>(viewer_->currentModel());
    if (graph) {
        if (graph->get_vertex_property<vec3>("v:color"))
            schemes.push_back("v:color");
        if (graph->get_edge_property<vec3>("e:color"))
            schemes.push_back("e:color");
        if (graph->get_vertex_property<vec2>("v:texcoord"))
            schemes.push_back("v:texcoord");
        if (graph->get_edge_property<vec2>("e:texcoord"))
            schemes.push_back("e:texcoord");

        // color schemes from scalar fields
        // scalar fields defined on edges
        for (const auto &name : graph->edge_properties()) {
            if (graph->get_edge_property<float>(name))
                schemes.push_back("scalar - " + name);
            else if (graph->get_edge_property<double>(name))
                schemes.push_back("scalar - " + name);
            else if (graph->get_edge_property<unsigned int>(name))
                schemes.push_back("scalar - " + name);
            else if (graph->get_edge_property<int>(name))
                schemes.push_back("scalar - " + name);
            else if (graph->get_edge_property<bool>(name))
                schemes.push_back("scalar - " + name);
        }
        // scalar fields defined on vertices
        for (const auto &name : graph->vertex_properties()) {
            if (graph->get_vertex_property<float>(name))
                schemes.push_back("scalar - " + name);
            else if (graph->get_vertex_property<double>(name))
                schemes.push_back("scalar - " + name);
            else if (graph->get_vertex_property<unsigned int>(name))
                schemes.push_back("scalar - " + name);
            else if (graph->get_vertex_property<int>(name))
                schemes.push_back("scalar - " + name);
            else if (graph->get_vertex_property<bool>(name))
                schemes.push_back("scalar - " + name);
        }
    }

    return schemes;
}


std::vector<std::string> WidgetLinesDrawable::vectorFields(const easy3d::Model *model) {
    std::vector<std::string> fields;

    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (mesh) {
        // vector fields defined on edges
        for (const auto &name : mesh->edge_properties()) {
            if (mesh->get_edge_property<vec3>(name)) {
                if (name != "e:color")
                    fields.push_back(name);
            }
        }
    }

    auto graph = dynamic_cast<Graph *>(viewer_->currentModel());
    if (graph) {
        // vector fields defined on edges
        for (const auto &name : graph->edge_properties()) {
            if (graph->get_edge_property<vec3>(name)) {
                if (name != "e:color")
                    fields.push_back(name);
            }
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
        const auto &drawables = mesh->lines_drawables();
        for (auto d : drawables) {
            if (d->name().find("vector - ") != std::string::npos)
                d->set_visible(false);
        }
        states_[drawable()].vector_field = "disabled";
    } else {
        const std::string &name = text.toStdString();
        updateVectorFieldBuffer(mesh, name);

        auto d = mesh->lines_drawable("vector - f:normal");
        d->set_visible(true);

        states_[drawable()].vector_field = "f:normal";
    }

    main_window_->updateUi();
    viewer_->update();
}


void WidgetLinesDrawable::updateVectorFieldBuffer(Model *model, const std::string &name) {
    LOG(ERROR) << "not implemented yet";

//    if (name == "f:normal") {
//        auto normals = mesh->get_face_property<vec3>(name);
//        if (!normals)
//            mesh->update_face_normals();
//    }
//
//    auto prop = mesh->get_face_property<vec3>(name);
//    if (!prop && name != "disabled") {
//        LOG(ERROR) << "vector field '" << name << "' doesn't exist";
//        return;
//    }
//
//    // a vector field is visualized as a LinesDrawable whose name is the same as the vector field
//    auto drawable = mesh->lines_drawable("vector - f:normal");
//    if (!drawable)
//        drawable = mesh->add_lines_drawable("vector - f:normal");
//
//    auto points = mesh->get_vertex_property<vec3>("v:point");
//
//    // use a limited number of edge to compute the length of the vectors.
//    float avg_edge_length = 0.0f;
//    const int num = std::min(static_cast<unsigned int>(500), mesh->n_edges());
//    for (unsigned int i = 0; i < num; ++i) {
//        SurfaceMesh::Edge edge(i);
//        auto vs = mesh->vertex(edge, 0);
//        auto vt = mesh->vertex(edge, 1);
//        avg_edge_length += distance(points[vs], points[vt]);
//    }
//    avg_edge_length /= num;
//
//    std::vector<vec3> vertices(mesh->n_faces() * 2, vec3(0.0f, 0.0f, 0.0f));
//    int idx = 0;
//    float scale = ui->doubleSpinBoxVectorFieldScale->value();
//    for (
//        auto f: mesh->faces()) {
//        int size = 0;
//        for (auto v: mesh->vertices(f)) {
//            vertices[idx] += points[v];
//            ++size;
//        }
//        vertices[idx] /= size;
//        vertices[idx + 1] = vertices[idx] + prop[f] * avg_edge_length * scale;
//        idx += 2;
//    }
//
//    viewer_->makeCurrent();
//    drawable->update_vertex_buffer(vertices);
//    viewer_->doneCurrent();
}


void WidgetLinesDrawable::setVectorFieldScale(double s) {
    auto model = viewer_->currentModel();

    const std::string &name = states_[drawable()].vector_field;
    updateVectorFieldBuffer(model, name);

    viewer_->update();
    states_[drawable()].vector_field_scale = s;
}


void WidgetLinesDrawable::disableUnavailableOptions() {
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
    bool can_show_scalar = visible && ui->comboBoxColorScheme->currentText().contains("scalar - ");
    ui->labelScalarFieldStyle->setEnabled(can_show_scalar);
    ui->comboBoxScalarFieldStyle->setEnabled(can_show_scalar);
    ui->labelScalarFieldClamp->setEnabled(can_show_scalar);
    ui->checkBoxScalarFieldClamp->setEnabled(can_show_scalar);
    ui->doubleSpinBoxScalarFieldClampLower->setEnabled(can_show_scalar && ui->checkBoxScalarFieldClamp->isChecked());
    ui->doubleSpinBoxScalarFieldClampUpper->setEnabled(can_show_scalar && ui->checkBoxScalarFieldClamp->isChecked());

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
