#include "widget_drawable_points.h"

#include <QColorDialog>

#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/model.h>
#include <easy3d/viewer/texture_manager.h>
#include <easy3d/viewer/renderer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include "paint_canvas.h"
#include "main_window.h"

#include "ui_widget_drawable_points.h"


using namespace easy3d;


WidgetPointsDrawable::WidgetPointsDrawable(QWidget *parent)
        : WidgetDrawable(parent), ui(new Ui::WidgetPointsDrawable) {
    ui->setupUi(this);

    if (colormaps_.empty())
        ui->comboBoxScalarFieldStyle->addItem("not available");
    else {
        for (const auto &colormap : colormaps_)
            ui->comboBoxScalarFieldStyle->addItem(QIcon(QString::fromStdString(colormap.file)),
                                                  QString::fromStdString("  " + colormap.name));
    }
}


void WidgetPointsDrawable::connectAll() {
    // which drawable
    connect(ui->comboBoxDrawables, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setActiveDrawable(const QString &)));

    // visible
    connect(ui->checkBoxVisible, SIGNAL(toggled(bool)), this, SLOT(setDrawableVisible(bool)));

    // thickness
    connect(ui->doubleSpinBoxPointSize, SIGNAL(valueChanged(double)), this, SLOT(setPointSize(double)));

    // imposter
    connect(ui->comboBoxImposterStyle, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setImposterStyle(const QString &)));

    // lighting
    connect(ui->comboBoxLightingOptions, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setLighting(const QString &)));

    // color scheme
    connect(ui->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setColorScheme(const QString &)));

    // default color
    connect(ui->toolButtonDefaultColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

    // back color
    connect(ui->checkBoxBackColor, SIGNAL(toggled(bool)), this, SLOT(setDistinctBackColor(bool)));
    connect(ui->toolButtonBackColor, SIGNAL(clicked()), this, SLOT(setBackColor()));

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


void WidgetPointsDrawable::disconnectAll() {
    // which drawable
    disconnect(ui->comboBoxDrawables, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setActiveDrawable(const QString &)));

    // visible
    disconnect(ui->checkBoxVisible, SIGNAL(toggled(bool)), this, SLOT(setDrawableVisible(bool)));

    // thickness
    disconnect(ui->doubleSpinBoxPointSize, SIGNAL(valueChanged(double)), this, SLOT(setPointSize(double)));

    // imposter
    disconnect(ui->comboBoxImposterStyle, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setImposterStyle(const QString &)));

    // lighting
    disconnect(ui->comboBoxLightingOptions, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setLighting(const QString &)));

    // color scheme
    disconnect(ui->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setColorScheme(const QString &)));

    // default color
    disconnect(ui->toolButtonDefaultColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

    // back color
    disconnect(ui->checkBoxBackColor, SIGNAL(toggled(bool)), this, SLOT(setDistinctBackColor(bool)));
    disconnect(ui->toolButtonBackColor, SIGNAL(clicked()), this, SLOT(setBackColor()));

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


WidgetPointsDrawable::~WidgetPointsDrawable() {
    delete ui;
}


namespace details {

    template<typename MODEL>
    inline void setup_scalar_field(MODEL *model, PointsDrawable *drawable, const std::string &color_scheme, float dummy_lower, float dummy_upper) {
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
            }
        }
    }


    template<typename MODEL>
    inline std::vector<std::string> color_schemes(const MODEL *model) {
        std::vector<std::string> schemes;
        schemes.push_back("uniform color");

        if (model->template get_vertex_property<vec3>("v:color"))
            schemes.push_back("v:color");
        if (model->template get_vertex_property<vec2>("v:texcoord"))
            schemes.push_back("v:texcoord");

        // scalar fields defined on vertices
        for (const auto &name : model->vertex_properties()) {
            if (model->template get_vertex_property<float>(name))
                schemes.push_back("scalar - " + name);
            else if (model->template get_vertex_property<double>(name))
                schemes.push_back("scalar - " + name);
            else if (model->template get_vertex_property<unsigned int>(name))
                schemes.push_back("scalar - " + name);
            else if (model->template get_vertex_property<int>(name))
                schemes.push_back("scalar - " + name);
        }

        return schemes;
    }


    template<typename MODEL>
    inline std::vector<std::string> vector_fields(const MODEL *model) {
        std::vector<std::string> fields;

        // vector fields defined on vertices
        for (const auto &name : model->vertex_properties()) {
            if (model->template get_vertex_property<vec3>(name)) {
                if (name != "v:point" && name != "v:color")
                    fields.push_back(name);
            }
        }

        // if no vector fields found, add a "not available" item
        if (fields.empty())
            fields.push_back("not available");
        else   // add one allowing to disable vector fields
            fields.insert(fields.begin(), "disabled");

        return fields;
    }
}


// update the panel to be consistent with the drawable's rendering parameters
void WidgetPointsDrawable::updatePanel() {
    auto model = viewer_->currentModel();
    if (!model || !model->is_visible() || model->points_drawables().empty()) {
        setEnabled(false);
        return;
    }

    setEnabled(true);

    disconnectAll();

    auto &state = states_[drawable()];

    ui->comboBoxDrawables->clear();
    const auto &drawables = model->points_drawables();
    for (auto d : drawables)
        ui->comboBoxDrawables->addItem(QString::fromStdString(d->name()));
    ui->comboBoxDrawables->setCurrentText(QString::fromStdString(drawable()->name()));

    // visible
    ui->checkBoxVisible->setChecked(drawable()->is_visible());

    // thickness
    ui->doubleSpinBoxPointSize->setValue(drawable()->point_size());

    // imposter
    switch (drawable()->impostor_type()) {
        case PointsDrawable::PLAIN:
            ui->comboBoxImposterStyle->setCurrentText("plain");
            break;
        case PointsDrawable::SPHERE:
            ui->comboBoxImposterStyle->setCurrentText("sphere");
            break;
        case PointsDrawable::SURFEL:
            ui->comboBoxImposterStyle->setCurrentText("surfel");
            break;
    }

    {   // lighting
        if (drawable()->lighting()) {
            if (drawable()->lighting_two_sides())
                ui->comboBoxLightingOptions->setCurrentText("front and back");
            else
                ui->comboBoxLightingOptions->setCurrentText("front only");
        } else
            ui->comboBoxLightingOptions->setCurrentText("disabled");
    }

    {   // color scheme
        ui->comboBoxColorScheme->clear();
        std::vector<std::string> schemes;
        if (dynamic_cast<PointCloud *>(viewer_->currentModel()))
            schemes = details::color_schemes(dynamic_cast<PointCloud *>(viewer_->currentModel()));
        else if (dynamic_cast<SurfaceMesh *>(viewer_->currentModel()))
            schemes = details::color_schemes(dynamic_cast<SurfaceMesh *>(viewer_->currentModel()));
        else if (dynamic_cast<Graph *>(viewer_->currentModel()))
            schemes = details::color_schemes(dynamic_cast<Graph *>(viewer_->currentModel()));
        for (const auto &scheme : schemes)
            ui->comboBoxColorScheme->addItem(QString::fromStdString(scheme));

        ui->comboBoxColorScheme->setCurrentText(QString::fromStdString(state.coloring));

        // default color
        vec3 c = drawable()->default_color();
        QPixmap pixmap(ui->toolButtonDefaultColor->size());
        pixmap.fill(
                QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonDefaultColor->setIcon(QIcon(pixmap));

        // back side color
        ui->checkBoxBackColor->setChecked(drawable()->distinct_back_color());
        c = drawable()->back_color();
        pixmap.fill(
                QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonBackColor->setIcon(QIcon(pixmap));

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
        std::vector<std::string> fields;
        if (dynamic_cast<PointCloud *>(viewer_->currentModel()))
            fields = details::vector_fields(dynamic_cast<PointCloud *>(viewer_->currentModel()));
        else if (dynamic_cast<SurfaceMesh *>(viewer_->currentModel()))
            fields = details::vector_fields(dynamic_cast<SurfaceMesh *>(viewer_->currentModel()));
        else if (dynamic_cast<Graph *>(viewer_->currentModel()))
            fields = details::vector_fields(dynamic_cast<Graph *>(viewer_->currentModel()));
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
void WidgetPointsDrawable::updateRendering() {
    if (!drawable())
        return;
    setColorScheme(ui->comboBoxColorScheme->currentText());
}


PointsDrawable *WidgetPointsDrawable::drawable() {
    auto model = viewer_->currentModel();
    auto pos = active_drawable_.find(model);
    if (pos != active_drawable_.end())
        return model->points_drawable(pos->second);
    else {
        const auto &drawables = model->points_drawables();
        if (drawables.empty())
            return nullptr;
        else {
            active_drawable_[model] = drawables[0]->name();
            return drawables[0];
        }
    }
}


void WidgetPointsDrawable::setActiveDrawable(const QString &text) {
    auto model = viewer_->currentModel();
    const std::string &name = text.toStdString();

    if (active_drawable_.find(model) != active_drawable_.end()) {
        if (active_drawable_[model] == name)
            return; // already active
    }

    if (model->points_drawable(name)) {
        active_drawable_[model] = name;
    } else {
        LOG(ERROR) << "drawable '" << name << "' not defined on model: " << model->name();
        const auto &drawables = model->points_drawables();
        if (drawables.empty())
            LOG(ERROR) << "no points drawable defined on model: " << model->name();
        else
            active_drawable_[model] = drawables[0]->name();
    }

    updatePanel();
}


void WidgetPointsDrawable::setDrawableVisible(bool b) {
    if (drawable()->is_visible() != b) {
        drawable()->set_visible(b);
        viewer_->update();
    }
    disableUnavailableOptions();
}


void WidgetPointsDrawable::setPointSize(double s) {
    if (drawable()->point_size() != s) {
        drawable()->set_point_size(s);
        viewer_->update();
    }
}


void WidgetPointsDrawable::setLighting(const QString &text) {
    if (text == "front and back") {
        if (!drawable()->lighting())
            drawable()->set_lighting(true);
        if (!drawable()->lighting_two_sides())
            drawable()->set_lighting_two_sides(true);
    } else if (text == "front only") {
        if (!drawable()->lighting())
            drawable()->set_lighting(true);
        if (drawable()->lighting_two_sides())
            drawable()->set_lighting_two_sides(false);
    } else if (text == "disabled") {
        if (drawable()->lighting())
            drawable()->set_lighting(false);
    }

    viewer_->update();
    disableUnavailableOptions();
}


void WidgetPointsDrawable::setImposterStyle(const QString &style) {
    if (style == "plain") {
        if (drawable()->impostor_type() != PointsDrawable::PLAIN)
            drawable()->set_impostor_type(PointsDrawable::PLAIN);
    } else if (style == "sphere") {
        if (drawable()->impostor_type() != PointsDrawable::SPHERE)
            drawable()->set_impostor_type(PointsDrawable::SPHERE);
    } else if (style == "surfel") {
        if (drawable()->impostor_type() != PointsDrawable::SURFEL) {
            if (drawable()->normal_buffer() == 0) { // surfel requires point normals
                SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
                if (mesh) {
                    auto normals = mesh->get_vertex_property<vec3>("v:normal");
                    if (!normals) {
                        mesh->update_vertex_normals();
                        normals = mesh->get_vertex_property<vec3>("v:normal");
                    }
                    viewer_->makeCurrent();
                    drawable()->update_normal_buffer(normals.vector());
                    viewer_->doneCurrent();
                } else if (dynamic_cast<PointCloud *>(viewer_->currentModel())) {
                    if (drawable()->normal_buffer() == 0) { // surfel requires point normals
                        PointCloud *cloud = dynamic_cast<PointCloud *>(viewer_->currentModel());
                        auto normals = cloud->get_vertex_property<vec3>("v:normal");
                        if (normals) {
                            viewer_->makeCurrent();
                            drawable()->update_normal_buffer(normals.vector());
                            viewer_->doneCurrent();
                        }
                    }
                }
            }

            if (drawable()->normal_buffer())
                drawable()->set_impostor_type(PointsDrawable::SURFEL);
            else
                LOG(WARNING) << "point imposter SURFEL requires normal information";
        }
    }

    viewer_->update();
    disableUnavailableOptions();
}


void WidgetPointsDrawable::setColorScheme(const QString &text) {
    disableUnavailableOptions();

    states_[drawable()].coloring = text.toStdString();
    states_[drawable()].scalar_style = ui->comboBoxScalarFieldStyle->currentIndex();
    states_[drawable()].clamp_value = ui->checkBoxScalarFieldClamp->isChecked();
    states_[drawable()].clamp_value_lower = ui->doubleSpinBoxScalarFieldClampLower->value();
    states_[drawable()].clamp_value_upper = ui->doubleSpinBoxScalarFieldClampUpper->value();

    updateRendering(drawable());

    viewer_->update();
}


void WidgetPointsDrawable::updateRendering(PointsDrawable* drawable) {
    if (!drawable)
        return;
    Model *model = drawable->model();
    if (!model)
        return;
    if (states_.find(drawable) == states_.end())
        return;

    const std::string& color_scheme = states_[drawable].coloring;
    drawable->set_per_vertex_color(color_scheme != "uniform color");

    bool is_scalar_field = (color_scheme.find("scalar - ") != std::string::npos);
    if (is_scalar_field) {
        float clamp_lower = 0.0f, clamp_upper = 0.0f;
        if (states_[drawable].clamp_value) {
            clamp_lower = states_[drawable].clamp_value_lower / 100.0f;
            clamp_upper = states_[drawable].clamp_value_upper / 100.0f;
        }

        viewer_->makeCurrent();
        if (dynamic_cast<SurfaceMesh *>(model)) {
            SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(model);
            details::setup_scalar_field(mesh, drawable, color_scheme, clamp_lower, clamp_upper);
        } else if (dynamic_cast<Graph *>(viewer_->currentModel())) {
            Graph *graph = dynamic_cast<Graph *>(model);
            details::setup_scalar_field(graph, drawable, color_scheme, clamp_lower, clamp_upper);
        } else if (dynamic_cast<PointCloud *>(model)) {
            PointCloud *cloud = dynamic_cast<PointCloud *>(model);
            details::setup_scalar_field(cloud, drawable, color_scheme, clamp_lower, clamp_upper);
        }
        drawable->set_texture(colormapTexture(states_[drawable].scalar_style));
        viewer_->doneCurrent();
    }
    else if (color_scheme.find(":texcoord") != std::string::npos) {
        viewer_->makeCurrent();
        if (dynamic_cast<SurfaceMesh *>(model)) {
            SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(model);
            renderer::update_buffer(mesh, drawable, mesh->get_vertex_property<vec2>(color_scheme));
        }
        viewer_->doneCurrent();
    }
    else {
        viewer_->makeCurrent();
        if (dynamic_cast<SurfaceMesh *>(model)) {
            SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(model);
            renderer::update_buffer(mesh, drawable);
        }
        viewer_->doneCurrent();
    }

    bool use_texture = (color_scheme.find(":texcoord") != std::string::npos || is_scalar_field);
    drawable->set_use_texture(use_texture);
}


void WidgetPointsDrawable::setDefaultColor() {
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


void WidgetPointsDrawable::setBackColor() {
    const vec3 &c = drawable()->back_color();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    const QColor &color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec3 new_color(color.redF(), color.greenF(), color.blueF());
        drawable()->set_back_color(new_color);
        viewer_->update();

        QPixmap pixmap(ui->toolButtonBackColor->size());
        pixmap.fill(color);
        ui->toolButtonBackColor->setIcon(QIcon(pixmap));
    }
}


void WidgetPointsDrawable::setDistinctBackColor(bool b) {
    if (drawable()->distinct_back_color() != b) {
        drawable()->set_distinct_back_color(b);
        viewer_->update();
        disableUnavailableOptions();
    }
}


void WidgetPointsDrawable::setHighlight(bool b) {
    if (drawable()->highlight() != b) {
        drawable()->set_highlight(b);
        viewer_->update();
        disableUnavailableOptions();
    }
}


void WidgetPointsDrawable::setHighlightMin(int v) {
    const auto &range = drawable()->highlight_range();
    if (range.first != v) {
        drawable()->set_highlight_range(std::make_pair(v, range.second));
        viewer_->update();
    }
}


void WidgetPointsDrawable::setHighlightMax(int v) {
    const auto &range = drawable()->highlight_range();
    if (range.second != v) {
        drawable()->set_highlight_range(std::make_pair(range.first, v));
        viewer_->update();
    }
}


void WidgetPointsDrawable::setScalarFieldStyle(int idx) {
    auto tex = colormapTexture(idx);
    drawable()->set_texture(tex);
    drawable()->set_use_texture(true);
    viewer_->update();
    states_[drawable()].scalar_style = idx;
}


void WidgetPointsDrawable::setScalarFieldClamp(bool b) {
    auto& state = states_[drawable()];
    state.clamp_value = b;

    setColorScheme(ui->comboBoxColorScheme->currentText());
}


void WidgetPointsDrawable::setScalarFieldClampLower(double v) {
    auto& state = states_[drawable()];
    state.clamp_value_lower = v;

    setColorScheme(ui->comboBoxColorScheme->currentText());
}


void WidgetPointsDrawable::setScalarFieldClampUpper(double v) {
    auto& state = states_[drawable()];
    state.clamp_value_upper = v;

    setColorScheme(ui->comboBoxColorScheme->currentText());
}


void WidgetPointsDrawable::setVectorField(const QString &text) {
    auto model = viewer_->currentModel();

    if (text == "disabled") {
        const auto &drawables = model->lines_drawables();
        for (auto d : drawables) {
            if (d->name().find("vector - v") != std::string::npos)
                d->set_visible(false);
        }
        states_[drawable()].vector_field = "disabled";
    } else {
        const std::string &name = text.toStdString();
        updateVectorFieldBuffer(model, name);

        auto d = model->lines_drawable("vector - v:normal");
        d->set_visible(true);

        states_[drawable()].vector_field = "v:normal";
    }

    main_window_->updateUi();
    viewer_->update();
}


void WidgetPointsDrawable::updateVectorFieldBuffer(Model *model, const std::string &name) {
    if (name == "v:normal") {
        if (dynamic_cast<SurfaceMesh *>(model)) {
            auto mesh = dynamic_cast<SurfaceMesh *>(model);
            auto normals = mesh->get_vertex_property<vec3>(name);
            if (!normals)
                mesh->update_vertex_normals();
        }
    }

    // a vector field is visualized as a LinesDrawable whose name is the same as the vector field
    auto drawable = model->lines_drawable("vector - v:normal");
    if (!drawable)
        drawable = model->add_lines_drawable("vector - v:normal");

    std::vector<vec3> vertices;
    if (dynamic_cast<SurfaceMesh *>(model)) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);

        auto prop = mesh->get_vertex_property<vec3>(name);
        if (!prop && name != "disabled") {
            LOG(ERROR) << "vector field '" << name << "' doesn't exist";
            return;
        }

        auto points = mesh->get_vertex_property<vec3>("v:point");

        // use a limited number of edge to compute the length of the vectors.
        float avg_edge_length = 0.0f;
        const int num = std::min(static_cast<unsigned int>(500), mesh->n_edges());
        for (unsigned int i = 0; i < num; ++i) {
            SurfaceMesh::Edge edge(i);
            auto vs = mesh->vertex(edge, 0);
            auto vt = mesh->vertex(edge, 1);
            avg_edge_length += distance(points[vs], points[vt]);
        }
        avg_edge_length /= num;

        vertices.resize(mesh->n_vertices() * 2, vec3(0.0f, 0.0f, 0.0f));
        int idx = 0;
        float scale = ui->doubleSpinBoxVectorFieldScale->value();
        for (auto v: mesh->vertices()) {
            vertices[idx] = points[v];
            vertices[idx + 1] = vertices[idx] + prop[v] * avg_edge_length * scale;
            idx += 2;
        }
    } else if (dynamic_cast<PointCloud *>(model)) {
        auto cloud = dynamic_cast<PointCloud *>(model);

        auto prop = cloud->get_vertex_property<vec3>(name);
        if (!prop && name != "disabled") {
            LOG(ERROR) << "vector field '" << name << "' doesn't exist";
            return;
        }

        auto points = cloud->get_vertex_property<vec3>("v:point");

        // use the 1% of the bbox diagonal as the vector length.
        float avg_edge_length = cloud->bounding_box().diagonal() * 0.01f;

        vertices.resize(cloud->n_vertices() * 2, vec3(0.0f, 0.0f, 0.0f));
        int idx = 0;
        float scale = ui->doubleSpinBoxVectorFieldScale->value();
        for (auto v: cloud->vertices()) {
            vertices[idx] = points[v];
            vertices[idx + 1] = vertices[idx] + prop[v] * avg_edge_length * scale;
            idx += 2;
        }
    }

    viewer_->makeCurrent();
    drawable->update_vertex_buffer(vertices);
    viewer_->doneCurrent();
}


void WidgetPointsDrawable::setVectorFieldScale(double s) {
    auto model = viewer_->currentModel();

    const std::string &name = states_[drawable()].vector_field;
    updateVectorFieldBuffer(model, name);

    viewer_->update();
    states_[drawable()].vector_field_scale = s;
}


void WidgetPointsDrawable::disableUnavailableOptions() {
    bool visible = ui->checkBoxVisible->isChecked();
    ui->labelPointSize->setEnabled(visible);
    ui->doubleSpinBoxPointSize->setEnabled(visible);
    ui->labelImposterStyle->setEnabled(visible);
    ui->comboBoxImposterStyle->setEnabled(visible);
    ui->labelLighting->setEnabled(visible);
    ui->comboBoxLightingOptions->setEnabled(visible);
    ui->labelColorScheme->setEnabled(visible);
    ui->comboBoxColorScheme->setEnabled(visible);

    bool can_modify_default_color = visible && (ui->comboBoxColorScheme->currentText() == "uniform color");
    ui->labelDefaultColor->setEnabled(can_modify_default_color);
    ui->toolButtonDefaultColor->setEnabled(can_modify_default_color);

    const auto &lighting_option = ui->comboBoxLightingOptions->currentText();
    bool can_modify_back_color = visible && lighting_option == "front and back" && ui->comboBoxImposterStyle->currentText() != "sphere";
    ui->labelBackColor->setEnabled(can_modify_back_color);
    ui->checkBoxBackColor->setEnabled(can_modify_back_color);
    ui->toolButtonBackColor->setEnabled(can_modify_back_color && drawable()->distinct_back_color());

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



