#include "widget_drawable_points.h"

#include <QColorDialog>

#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/model.h>
#include <easy3d/viewer/texture.h>
#include <easy3d/viewer/renderer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include "paint_canvas.h"

#include "ui_widget_drawable_points.h"


using namespace easy3d;


WidgetPointsDrawable::WidgetPointsDrawable(QWidget *parent)
        : WidgetDrawable(parent), ui(new Ui::WidgetPointsDrawable)
{
    ui->setupUi(this);

    if (colormap_files_.empty())
        ui->comboBoxScalarFieldStyle->addItem("not available");
    else {
        for (const auto& colormap : colormap_files_)
            ui->comboBoxScalarFieldStyle->addItem(QIcon(QString::fromStdString(colormap.file)), QString::fromStdString(colormap.name));
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
    connect(ui->comboBoxScalarFieldStyle, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setScalarFieldStyle(const QString&)));
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
    disconnect(ui->comboBoxScalarFieldStyle, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setScalarFieldStyle(const QString&)));
}


WidgetPointsDrawable::~WidgetPointsDrawable() {
    delete ui;
}


namespace details {

    template<typename MODEL>
    inline void setup_scalar_field(MODEL* model, PointsDrawable* drawable, const std::string& color_scheme) {
        for (const auto &name : model->vertex_properties()) {
            if (color_scheme.find(name) != std::string::npos) {
                if (model->template get_vertex_property<float>(name)) {
                    renderer::update_buffer(model, drawable, model->template get_vertex_property<float>(name));
                    return;
                }
                if (model->template get_vertex_property<double>(name)) {
                    renderer::update_buffer(model, drawable, model->template get_vertex_property<double>(name));
                    return;
                }
                if (model->template get_vertex_property<unsigned int>(name)) {
                    renderer::update_buffer(model, drawable, model->template get_vertex_property<unsigned int>(name));
                    return;
                }
                if (model->template get_vertex_property<int>(name)) {
                    renderer::update_buffer(model, drawable, model->template get_vertex_property<int>(name));
                    return;
                }
                if (model->template get_vertex_property<bool>(name)) {
                    renderer::update_buffer(model, drawable, model->template get_vertex_property<bool>(name));
                    return;
                }
            }
        }
    }


    template<typename MODEL>
    inline std::vector<std::string> color_schemes(const MODEL* model) {
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
            else if (model->template get_vertex_property<bool>(name))
                schemes.push_back("scalar - " + name);
        }

        return schemes;
    }


    template<typename MODEL>
    inline std::vector<std::string> vector_fields(const MODEL* model) {
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
    if (!model || model->points_drawables().empty()) {
        setEnabled(false);
        return;
    }

    setEnabled(true);

    disconnectAll();

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
        case PointsDrawable::PLAIN:    ui->comboBoxImposterStyle->setCurrentText("plain");   break;
        case PointsDrawable::SPHERE:   ui->comboBoxImposterStyle->setCurrentText("sphere");  break;
        case PointsDrawable::SURFEL:   ui->comboBoxImposterStyle->setCurrentText("surfel");  break;
    }

    {   // lighting
        if (drawable()->lighting()) {
            if (drawable()->lighting_two_sides())
                ui->comboBoxLightingOptions->setCurrentText("front and back");
            else
                ui->comboBoxLightingOptions->setCurrentText("front only");
        }
        else
            ui->comboBoxLightingOptions->setCurrentText("disabled");
    }

    {   // color scheme
        ui->comboBoxColorScheme->clear();
        std::vector<std::string> schemes;
        if (dynamic_cast<PointCloud*>(viewer_->currentModel()))
            schemes = details::color_schemes(dynamic_cast<PointCloud*>(viewer_->currentModel()));
        else if (dynamic_cast<SurfaceMesh*>(viewer_->currentModel()))
            schemes = details::color_schemes(dynamic_cast<SurfaceMesh*>(viewer_->currentModel()));
        else if (dynamic_cast<Graph*>(viewer_->currentModel()))
            schemes = details::color_schemes(dynamic_cast<Graph*>(viewer_->currentModel()));
        for (const auto& scheme : schemes)
            ui->comboBoxColorScheme->addItem(QString::fromStdString(scheme));

        if (drawable()->per_vertex_color()) {
            const auto &name = model->color_scheme(drawable());
            ui->comboBoxColorScheme->setCurrentText(QString::fromStdString(name));
        }

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
    }

    {   // highlight
        bool highlight = drawable()->highlight();
        ui->checkBoxHighlight->setChecked(highlight);

        const auto &range = drawable()->highlight_range();
        ui->spinBoxHighlightMin->setValue(range.first);
        ui->spinBoxHighlightMax->setValue(range.second);
    }

    {   // vector field
        ui->comboBoxVectorField->clear();
        std::vector<std::string> fields;
        if (dynamic_cast<PointCloud*>(viewer_->currentModel()))
            fields = details::vector_fields(dynamic_cast<PointCloud*>(viewer_->currentModel()));
        else if (dynamic_cast<SurfaceMesh*>(viewer_->currentModel()))
            fields = details::vector_fields(dynamic_cast<SurfaceMesh*>(viewer_->currentModel()));
        else if (dynamic_cast<Graph*>(viewer_->currentModel()))
            fields = details::vector_fields(dynamic_cast<Graph*>(viewer_->currentModel()));
        for (auto name : fields)
            ui->comboBoxVectorField->addItem(QString::fromStdString(name));
    }

    disableUnavailableOptions();

    connectAll();
}


PointsDrawable *WidgetPointsDrawable::drawable() {
    auto model = viewer_->currentModel();
    auto pos = active_drawable_.find(model);
    if (pos != active_drawable_.end())
        return model->points_drawable(pos->second);
    else {
        const auto& drawables = model->points_drawables();
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


void WidgetPointsDrawable::setLighting(const QString & text) {
    if (text == "front and back") {
        if (!drawable()->lighting())
            drawable()->set_lighting(true);
        if (!drawable()->lighting_two_sides())
            drawable()->set_lighting_two_sides(true);
    }
    else if (text == "front only") {
        if (!drawable()->lighting())
            drawable()->set_lighting(true);
        if (drawable()->lighting_two_sides())
            drawable()->set_lighting_two_sides(false);
    }
    else if (text == "disabled") {
        if (drawable()->lighting())
            drawable()->set_lighting(false);
    }

    viewer_->update();
    disableUnavailableOptions();
}


void WidgetPointsDrawable::setImposterStyle(const QString & style) {
    if (style == "plain") {
        if (drawable()->impostor_type() != PointsDrawable::PLAIN)
            drawable()->set_impostor_type(PointsDrawable::PLAIN);
    }
    else if (style == "sphere") {
        if (drawable()->impostor_type() != PointsDrawable::SPHERE)
            drawable()->set_impostor_type(PointsDrawable::SPHERE);
    }
    else if (style == "surfel") {
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
                }
                else if (dynamic_cast<PointCloud *>(viewer_->currentModel())) {
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


void WidgetPointsDrawable::setColorScheme(const QString& text) {
    disableUnavailableOptions();

    bool per_vertex_color = text != "uniform color";
    if (drawable()->per_vertex_color() != per_vertex_color) {
        drawable()->set_per_vertex_color(per_vertex_color);
    }

    bool is_scalar_field = text.contains("scalar - ");
    if (is_scalar_field) {
        viewer_->makeCurrent();
        if (dynamic_cast<SurfaceMesh*>(viewer_->currentModel())) {
            SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
            details::setup_scalar_field(mesh, drawable(), text.toStdString());
            drawable()->set_texture(createColormapTexture(ui->comboBoxScalarFieldStyle->currentText()));
        }
        else if (dynamic_cast<Graph*>(viewer_->currentModel())) {
            Graph* graph = dynamic_cast<Graph*>(viewer_->currentModel());
            details::setup_scalar_field(graph, drawable(), text.toStdString());
            drawable()->set_texture(createColormapTexture(ui->comboBoxScalarFieldStyle->currentText()));
        }
        else if (dynamic_cast<PointCloud*>(viewer_->currentModel())) {
            PointCloud* cloud = dynamic_cast<PointCloud*>(viewer_->currentModel());
            details::setup_scalar_field(cloud, drawable(), text.toStdString());
            drawable()->set_texture(createColormapTexture(ui->comboBoxScalarFieldStyle->currentText()));
        }
        viewer_->doneCurrent();
    }

    bool use_texture = text.contains(":texcoord") || is_scalar_field;
    if (drawable()->use_texture() != use_texture)
        drawable()->set_use_texture(use_texture);

    viewer_->update();
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


void WidgetPointsDrawable::setScalarFieldStyle(const QString& text) {
    auto tex = createColormapTexture(ui->comboBoxScalarFieldStyle->currentText());
    drawable()->set_texture(tex);
    drawable()->set_use_texture(true);
    viewer_->update();
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

    const auto& lighting_option = ui->comboBoxLightingOptions->currentText();
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
    bool can_show_scalar = visible &&
            ui->comboBoxColorScheme->currentText().contains("scalar - ");
    ui->labelScalarFieldStyle->setEnabled(can_show_scalar);
    ui->comboBoxScalarFieldStyle->setEnabled(can_show_scalar);
    ui->labelScalarFieldAutoRange->setEnabled(can_show_scalar);
    ui->checkBoxScalarFieldAutoRange->setEnabled(can_show_scalar);
    ui->doubleSpinBoxScalarFieldAutoRangeMin->setEnabled(can_show_scalar);
    ui->doubleSpinBoxScalarFieldAutoRangeMax->setEnabled(can_show_scalar);

    // vector field
    bool can_show_vector = visible && ui->comboBoxVectorField->currentText() != "not available";
    ui->labelVectorField->setEnabled(can_show_vector);
    ui->comboBoxVectorField->setEnabled(can_show_vector);
    bool can_modify_vector_style = can_show_vector &&
            ui->comboBoxVectorField->currentText() != "disabled";
    ui->labelVectorFieldColor->setEnabled(can_modify_vector_style);
    ui->toolButtonVectorFieldColor->setEnabled(can_modify_vector_style);
    ui->labelVectorFieldWidth->setEnabled(can_modify_vector_style);
    ui->doubleSpinBoxVectorFieldWidth->setEnabled(can_modify_vector_style);
    ui->labelVectorFieldScale->setEnabled(can_modify_vector_style);
    ui->doubleSpinBoxVectorFieldScale->setEnabled(can_modify_vector_style);

    update();
    qApp->processEvents();
}



