#include "widget_drawable_lines.h"

#include <QColorDialog>

#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/model.h>
#include <easy3d/viewer/texture.h>
#include <easy3d/viewer/renderer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include "paint_canvas.h"

#include "ui_widget_drawable_lines.h"


using namespace easy3d;


WidgetLinesDrawable::WidgetLinesDrawable(QWidget *parent)
        : WidgetDrawable(parent), ui(new Ui::WidgetLinesDrawable)
{
    ui->setupUi(this);

    if (colormap_files_.empty())
        ui->comboBoxScalarFieldStyle->addItem("not available");
    else {
        for (const auto& colormap : colormap_files_)
            ui->comboBoxScalarFieldStyle->addItem(QIcon(QString::fromStdString(colormap.file)), QString::fromStdString(colormap.name));
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
    connect(ui->comboBoxScalarFieldStyle, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setScalarFieldStyle(const QString&)));
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
    disconnect(ui->comboBoxScalarFieldStyle, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setScalarFieldStyle(const QString&)));
}


WidgetLinesDrawable::~WidgetLinesDrawable() {
    delete ui;
}


// update the panel to be consistent with the drawable's rendering parameters
void WidgetLinesDrawable::updatePanel() {
    auto model = viewer_->currentModel();
    if (!model || model->lines_drawables().empty()) {
        setEnabled(false);
        return;
    }

    setEnabled(true);

    disconnectAll();

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
        case LinesDrawable::PLAIN:      ui->comboBoxImposterStyle->setCurrentText("plain");     break;
        case LinesDrawable::CYLINDER:   ui->comboBoxImposterStyle->setCurrentText("cylinder");  break;
        case LinesDrawable::CONE:       ui->comboBoxImposterStyle->setCurrentText("cone");      break;
    }

    {   // color scheme
        ui->comboBoxColorScheme->clear();
        auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
        ui->comboBoxColorScheme->addItem("uniform color");
        auto vcolors = mesh->get_vertex_property<vec3>("v:color");
        if (vcolors)
            ui->comboBoxColorScheme->addItem("v:color");
        auto ecolors = mesh->get_edge_property<vec3>("e:color");
        if (ecolors)
            ui->comboBoxColorScheme->addItem("e:color");
        auto vtexcoords = mesh->get_vertex_property<vec2>("v:texcoord");
        if (vtexcoords)
            ui->comboBoxColorScheme->addItem("v:texcoord");
        auto htexcoords = mesh->get_halfedge_property<vec2>("h:texcoord");
        if (htexcoords)
            ui->comboBoxColorScheme->addItem("h:texcoord");

        {   // color schemes from scalar fields
            SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(model);

            // scalar fields defined on edges
            for (const auto& name : mesh->edge_properties()) {
                if (mesh->get_edge_property<float>(name))
                    ui->comboBoxColorScheme->addItem(QString::fromStdString("scalar - " + name));
                else if (mesh->get_face_property<int>(name))
                    ui->comboBoxColorScheme->addItem(QString::fromStdString("scalar - " + name));
            }
            // scalar fields defined on vertices
            for (const auto& name : mesh->vertex_properties()) {
                if (mesh->get_vertex_property<float>(name))
                    ui->comboBoxColorScheme->addItem(QString::fromStdString("scalar - " + name));
                else if (mesh->get_vertex_property<int>(name))
                    ui->comboBoxColorScheme->addItem(QString::fromStdString("scalar - " + name));
            }
        }

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
    }

    {   // highlight
        bool highlight = drawable()->highlight();
        ui->checkBoxHighlight->setChecked(highlight);

        const auto &range = drawable()->highlight_range();
        ui->spinBoxHighlightMin->setValue(range.first);
        ui->spinBoxHighlightMax->setValue(range.second);
    }

    {   // scalar field
    }

    {   // vector field
        ui->comboBoxVectorField->clear();

        SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(model);

        // vector fields defined on faces
        for (const auto& name : mesh->face_properties()) {
            if (mesh->get_face_property<vec3>(name))
                ui->comboBoxVectorField->addItem(QString::fromStdString(name));
        }

        // if no vector fields found, add a "not available" item
        if (ui->comboBoxVectorField->count() == 0)
            ui->comboBoxVectorField->addItem("not available");
        else {  // add one allowing to disable vector fields
            ui->comboBoxVectorField->insertItem(0, "disabled");
            ui->comboBoxVectorField->setCurrentIndex(0);
        }
    }

    disableUnavailableOptions();

    connectAll();
}


LinesDrawable *WidgetLinesDrawable::drawable() {
    auto model = viewer_->currentModel();
    auto pos = active_drawable_.find(model);
    if (pos != active_drawable_.end())
        return model->lines_drawable(pos->second);
    else {
        const auto& drawables = model->lines_drawables();
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


void WidgetLinesDrawable::setImposterStyle(const QString & style) {
    if (style == "plain") {
        if (drawable()->impostor_type() != LinesDrawable::PLAIN)
            drawable()->set_impostor_type(LinesDrawable::PLAIN);
    }
    else if (style == "cylinder") {
        if (drawable()->impostor_type() != LinesDrawable::CYLINDER)
            drawable()->set_impostor_type(LinesDrawable::CYLINDER);
    }
    else if (style == "cone") {
        if (drawable()->impostor_type() != LinesDrawable::CONE)
            drawable()->set_impostor_type(LinesDrawable::CONE);
    }

    viewer_->update();
    disableUnavailableOptions();
}



namespace details {

    inline void setup_scalar_field(SurfaceMesh* mesh, LinesDrawable* drawable, const std::string& color_scheme) {
        for (const auto &name : mesh->edge_properties()) {
            if (color_scheme.find(name) != std::string::npos) {
                if (mesh->get_edge_property<float>(name)) {
                    renderer::update_buffer(mesh, drawable, mesh->get_edge_property<float>(name));
                    return;
                }
                if (mesh->get_edge_property<double>(name)) {
                    renderer::update_buffer(mesh, drawable, mesh->get_edge_property<double>(name));
                    return;
                }
                if (mesh->get_edge_property<unsigned int>(name)) {
                    renderer::update_buffer(mesh, drawable, mesh->get_edge_property<unsigned int>(name));
                    return;
                }
                if (mesh->get_edge_property<int>(name)) {
                    renderer::update_buffer(mesh, drawable, mesh->get_edge_property<int>(name));
                    return;
                }
                if (mesh->get_edge_property<bool>(name)) {
                    renderer::update_buffer(mesh, drawable, mesh->get_edge_property<bool>(name));
                    return;
                }
            }
        }

        for (const auto &name : mesh->vertex_properties()) {
            if (color_scheme.find(name) != std::string::npos) {
                if (mesh->get_vertex_property<float>(name)) {
                    renderer::update_buffer(mesh, drawable, mesh->get_vertex_property<float>(name));
                    return;
                }
                if (mesh->get_vertex_property<double>(name)) {
                    renderer::update_buffer(mesh, drawable, mesh->get_vertex_property<double>(name));
                    return;
                }
                if (mesh->get_vertex_property<unsigned int>(name)) {
                    renderer::update_buffer(mesh, drawable, mesh->get_vertex_property<unsigned int>(name));
                    return;
                }
                if (mesh->get_vertex_property<int>(name)) {
                    renderer::update_buffer(mesh, drawable, mesh->get_vertex_property<int>(name));
                    return;
                }
                if (mesh->get_vertex_property<bool>(name)) {
                    renderer::update_buffer(mesh, drawable, mesh->get_vertex_property<bool>(name));
                    return;
                }
            }
        }
    }
}

void WidgetLinesDrawable::setColorScheme(const QString& text) {
    disableUnavailableOptions();

    bool per_vertex_color = text != "uniform color";
    if (drawable()->per_vertex_color() != per_vertex_color) {
        drawable()->set_per_vertex_color(per_vertex_color);
    }

    bool is_scalar_field = text.contains("scalar - ");
    if (is_scalar_field) {
        SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
        if (mesh) {
            ::details::setup_scalar_field(mesh, drawable(), text.toStdString());
            drawable()->set_texture(createColormapTexture(ui->comboBoxScalarFieldStyle->currentText()));
        }
    }

    bool use_texture = text.contains(":texcoord") || is_scalar_field;
    if (drawable()->use_texture() != use_texture)
        drawable()->set_use_texture(use_texture);

    viewer_->update();
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


void WidgetLinesDrawable::setScalarFieldStyle(const QString& text) {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
    if (!mesh)
        return;

    auto tex = createColormapTexture(ui->comboBoxScalarFieldStyle->currentText());
    drawable()->set_texture(tex);
    drawable()->set_use_texture(true);
    viewer_->update();
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


