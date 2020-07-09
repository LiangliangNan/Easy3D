#include "widget_drawable_triangles.h"

#include <QColorDialog>
#include <QFileDialog>

#include <easy3d/core/model.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/setting.h>
#include <easy3d/renderer/buffers.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#include <easy3d/fileio/resources.h>

#include "paint_canvas.h"
#include "main_window.h"

#include "ui_widget_drawable_triangles.h"


using namespace easy3d;


WidgetTrianglesDrawable::WidgetTrianglesDrawable(QWidget *parent)
        : WidgetDrawable(parent), ui(new Ui::WidgetTrianglesDrawable) {
    ui->setupUi(this);

    if (colormaps_.empty())
        ui->comboBoxScalarFieldStyle->addItem("not available");
    else {
        for (const auto &colormap : colormaps_)
            ui->comboBoxScalarFieldStyle->addItem(QIcon(QString::fromStdString(colormap.file)),
                                                  QString::fromStdString("  " + colormap.name));
    }
}


void WidgetTrianglesDrawable::connectAll() {
    // which drawable
    connect(ui->comboBoxDrawables, SIGNAL(currentIndexChanged(const QString &)),this, SLOT(setActiveDrawable(const QString &)));

    // visible
    connect(ui->checkBoxVisible, SIGNAL(toggled(bool)), this, SLOT(setDrawableVisible(bool)));

    // phong shading
    connect(ui->checkBoxPhongShading, SIGNAL(toggled(bool)), this, SLOT(setPhongShading(bool)));

    // lighting
    connect(ui->comboBoxLightingOptions, SIGNAL(currentIndexChanged(const QString &)),this, SLOT(setLighting(const QString &)));

    // color scheme
    connect(ui->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)),this, SLOT(setColorScheme(const QString &)));

    // default color
    connect(ui->toolButtonDefaultColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

    // back color
    connect(ui->checkBoxBackColor, SIGNAL(toggled(bool)), this, SLOT(setDistinctBackColor(bool)));
    connect(ui->toolButtonBackColor, SIGNAL(clicked()), this, SLOT(setBackColor()));

    // texture
    connect(ui->toolButtonTextureFile, SIGNAL(clicked()), this, SLOT(setTextureFile()));
    connect(ui->spinBoxTextureRepeat, SIGNAL(valueChanged(int)), this, SLOT(setTextureRepeat(int)));
    connect(ui->spinBoxTextureFractionalRepeat, SIGNAL(valueChanged(int)),this, SLOT(setTextureFractionalRepeat(int)));

    // highlight
    connect(ui->checkBoxHighlight, SIGNAL(toggled(bool)), this, SLOT(setHighlight(bool)));
    connect(ui->spinBoxHighlightMin, SIGNAL(valueChanged(int)), this, SLOT(setHighlightMin(int)));
    connect(ui->spinBoxHighlightMax, SIGNAL(valueChanged(int)), this, SLOT(setHighlightMax(int)));

    // transparency
    connect(ui->horizontalSliderOpacity, SIGNAL(valueChanged(int)), this, SLOT(setOpacity(int)));

    // scalar field
    connect(ui->comboBoxScalarFieldStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(setScalarFieldStyle(int)));
    connect(ui->checkBoxScalarFieldDiscrete, SIGNAL(toggled(bool)), this, SLOT(setScalarFieldDiscreteColors(bool)));
    connect(ui->spinBoxScalarFieldNumStrips, SIGNAL(valueChanged(int)), this, SLOT(setScalarFieldNumOfStripes(int)));
    connect(ui->checkBoxScalarFieldClamp, SIGNAL(toggled(bool)), this, SLOT(setScalarFieldClamp(bool)));
    connect(ui->doubleSpinBoxScalarFieldClampLower, SIGNAL(valueChanged(double)), this, SLOT(setScalarFieldClampLower(double)));
    connect(ui->doubleSpinBoxScalarFieldClampUpper, SIGNAL(valueChanged(double)), this, SLOT(setScalarFieldClampUpper(double)));

    // vector field
    connect(ui->comboBoxVectorField, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setVectorField(const QString&)));
    connect(ui->doubleSpinBoxVectorFieldScale, SIGNAL(valueChanged(double)), this, SLOT(setVectorFieldScale(double)));
}


void WidgetTrianglesDrawable::disconnectAll() {
    // which drawable
    disconnect(ui->comboBoxDrawables, SIGNAL(currentIndexChanged(const QString &)),this, SLOT(setActiveDrawable(const QString &)));

    // visible
    disconnect(ui->checkBoxVisible, SIGNAL(toggled(bool)), this, SLOT(setDrawableVisible(bool)));

    // phong shading
    disconnect(ui->checkBoxPhongShading, SIGNAL(toggled(bool)), this, SLOT(setPhongShading(bool)));

    // lighting
    disconnect(ui->comboBoxLightingOptions, SIGNAL(currentIndexChanged(const QString &)),this, SLOT(setLighting(const QString &)));

    // color scheme
    disconnect(ui->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)),this, SLOT(setColorScheme(const QString &)));

    // default color
    disconnect(ui->toolButtonDefaultColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

    // back color
    disconnect(ui->checkBoxBackColor, SIGNAL(toggled(bool)), this, SLOT(setDistinctBackColor(bool)));
    disconnect(ui->toolButtonBackColor, SIGNAL(clicked()), this, SLOT(setBackColor()));

    // texture
    disconnect(ui->toolButtonTextureFile, SIGNAL(clicked()), this, SLOT(setTextureFile()));
    disconnect(ui->spinBoxTextureRepeat, SIGNAL(valueChanged(int)), this, SLOT(setTextureRepeat(int)));
    disconnect(ui->spinBoxTextureFractionalRepeat, SIGNAL(valueChanged(int)),this, SLOT(setTextureFractionalRepeat(int)));

    // highlight
    disconnect(ui->checkBoxHighlight, SIGNAL(toggled(bool)), this, SLOT(setHighlight(bool)));
    disconnect(ui->spinBoxHighlightMin, SIGNAL(valueChanged(int)), this, SLOT(setHighlightMin(int)));
    disconnect(ui->spinBoxHighlightMax, SIGNAL(valueChanged(int)), this, SLOT(setHighlightMax(int)));

    // transparency
    disconnect(ui->horizontalSliderOpacity, SIGNAL(valueChanged(int)), this, SLOT(setOpacity(int)));

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


WidgetTrianglesDrawable::~WidgetTrianglesDrawable() {
    delete ui;
}


// update the panel to be consistent with the drawable's rendering parameters
void WidgetTrianglesDrawable::updatePanel() {
    auto model = viewer_->currentModel();
    if (!model)
        active_drawable_.clear();

    auto d = dynamic_cast<TrianglesDrawable*>(drawable());
    if (!model || !model->renderer()->is_visible() || !d) {
        setEnabled(false);
        return;
    }

    setEnabled(true);

    disconnectAll();

    auto& state = states_[d];
    auto& scheme = d->state();

    ui->comboBoxDrawables->clear();
    const auto &drawables = model->renderer()->triangles_drawables();
    for (auto d : drawables)
        ui->comboBoxDrawables->addItem(QString::fromStdString(d->name()));
    ui->comboBoxDrawables->setCurrentText(QString::fromStdString(d->name()));

    // visible
    ui->checkBoxVisible->setChecked(d->is_visible());

    // phong shading
    ui->checkBoxPhongShading->setChecked(d->smooth_shading());

    {   // lighting
        if (d->lighting()) {
            if (d->lighting_two_sides())
                ui->comboBoxLightingOptions->setCurrentText("front and back");
            else
                ui->comboBoxLightingOptions->setCurrentText("front only");
        } else
            ui->comboBoxLightingOptions->setCurrentText("disabled");
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

        // back side color
        ui->checkBoxBackColor->setChecked(d->distinct_back_color());
        c = d->back_color();
        pixmap.fill(
                QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonBackColor->setIcon(QIcon(pixmap));

        // texture
        auto tex = d->texture();
        if (tex) {
            const std::string &tex_name = file_system::simple_name(tex->name());
            ui->lineEditTextureFile->setText(QString::fromStdString(tex_name));
        }
        else
            ui->lineEditTextureFile->setText("not available");

        ui->spinBoxTextureRepeat->setValue(d->texture_repeat());
        ui->spinBoxTextureFractionalRepeat->setValue(d->texture_fractional_repeat());
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



std::vector<QString> WidgetTrianglesDrawable::colorSchemes(const easy3d::Model *model) {
    std::vector<QString> schemes;
    schemes.push_back("uniform color");

    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (mesh) {
        // color schemes from color properties and texture
        for (const auto &name : mesh->face_properties()) {
            if (name.find("f:color") != std::string::npos)
                schemes.push_back(QString::fromStdString(name));
        }
        for (const auto &name : mesh->vertex_properties()) {
            if (name.find("v:color") != std::string::npos || name.find("v:texcoord") != std::string::npos)
                schemes.push_back(QString::fromStdString(name));
        }
        for (const auto &name : mesh->halfedge_properties()) {
            if (name.find("h:texcoord") != std::string::npos)
                schemes.push_back(QString::fromStdString(name));
        }

        // color schemes from scalar fields

        // scalar fields defined on faces
        for (const auto &name : mesh->face_properties()) {
            if (mesh->get_face_property<float>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (mesh->get_face_property<double>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (mesh->get_face_property<unsigned int>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (mesh->get_face_property<int>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (mesh->get_vertex_property<char>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (mesh->get_vertex_property<unsigned char>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
        }

        // scalar fields defined on vertices
        for (const auto &name : mesh->vertex_properties()) {
            if (mesh->get_vertex_property<float>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (mesh->get_vertex_property<double>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (mesh->get_vertex_property<unsigned int>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (mesh->get_vertex_property<int>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (mesh->get_vertex_property<char>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
            else if (mesh->get_vertex_property<unsigned char>(name))
                schemes.push_back(scalar_prefix_ + QString::fromStdString(name));
        }
    }

    return schemes;
}


std::vector<QString> WidgetTrianglesDrawable::vectorFields(const easy3d::Model *model) {
    std::vector<QString> fields;

    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (mesh) {
        // vector fields defined on faces
        fields.push_back("f:normal");

        for (const auto &name : mesh->face_properties()) {
            if (mesh->get_face_property<vec3>(name)) {
                if (name != "f:normal" && name != "f:color")
                    fields.push_back(QString::fromStdString(name));
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


Drawable *WidgetTrianglesDrawable::drawable() {
    auto model = viewer_->currentModel();
    if (!model) {
        active_drawable_.clear();
        return nullptr;
    }

    auto pos = active_drawable_.find(model);
    if (pos != active_drawable_.end())
        return model->renderer()->get_triangles_drawable(pos->second);
    else {
        for (auto d : model->renderer()->triangles_drawables() ) {
            active_drawable_[model] = d->name();
            return d;
        }
        return nullptr;
    }
}


void WidgetTrianglesDrawable::setActiveDrawable(const QString &text) {
    auto model = viewer_->currentModel();
    const std::string &name = text.toStdString();

    if (active_drawable_.find(model) != active_drawable_.end()) {
        if (active_drawable_[model] == name)
            return; // already active
    }

    if (model->renderer()->get_triangles_drawable(name)) {
        active_drawable_[model] = name;
    } else {
        LOG(ERROR) << "drawable '" << name << "' not defined on model: " << model->name();
        const auto &drawables = model->renderer()->triangles_drawables();
        if (drawables.empty())
            LOG(ERROR) << "no triangles drawable defined on model: " << model->name();
        else
            active_drawable_[model] = drawables[0]->name();
    }

    updatePanel();
}


void WidgetTrianglesDrawable::setPhongShading(bool b) {
    auto d = dynamic_cast<TrianglesDrawable*>(drawable());
    if (d->smooth_shading() != b) {
        d->set_smooth_shading(b);
        viewer_->update();
    }
}


void WidgetTrianglesDrawable::setColorScheme(const QString &text) {
    auto d = drawable();

    auto tex = d->texture();
    if (tex) {
        const std::string &tex_name = file_system::simple_name(tex->name());
        ui->lineEditTextureFile->setText(QString::fromStdString(tex_name));
    }

    auto& scheme = d->state();
    scheme.set_clamp_range(ui->checkBoxScalarFieldClamp->isChecked());
    scheme.set_clamp_lower(ui->doubleSpinBoxScalarFieldClampLower->value() / 100.0);
    scheme.set_clamp_upper(ui->doubleSpinBoxScalarFieldClampUpper->value() / 100.0);
    states_[d].scalar_style = ui->comboBoxScalarFieldStyle->currentIndex();

    WidgetDrawable::setColorScheme(text);
}


void WidgetTrianglesDrawable::setTextureFile() {
    const std::string dir = resource::directory() + "/textures/";
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose an image file"), QString::fromStdString(dir),
                                                    tr("Image format (*.png *.jpg *.bmp *.tga)")
    );

    if (fileName.isEmpty())
        return;

    const std::string &file_name = fileName.toStdString();
    viewer_->makeCurrent();
    Texture* tex = TextureManager::request(file_name, Texture::REPEAT);
    viewer_->doneCurrent();

    if (tex) {
        auto d = drawable();
        d->set_texture(tex);
        viewer_->update();
        const std::string& simple_name = file_system::simple_name(file_name);
        ui->lineEditTextureFile->setText(QString::fromStdString(simple_name));
    } else
        LOG(WARNING) << "failed creating texture from file: " << file_name;

    disableUnavailableOptions();
}


void WidgetTrianglesDrawable::setOpacity(int a) {
    auto d = dynamic_cast<TrianglesDrawable*>(drawable());
    d->set_opacity(a / 100.0f);
    viewer_->update();
}


void WidgetTrianglesDrawable::setDefaultColor() {
    auto d = drawable();
    const vec3 &c = d->color();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    const QColor &color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec4 new_color(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        d->set_uniform_coloring(new_color);
        viewer_->update();

        QPixmap pixmap(ui->toolButtonDefaultColor->size());
        pixmap.fill(color);
        ui->toolButtonDefaultColor->setIcon(QIcon(pixmap));
    }
}


void WidgetTrianglesDrawable::setBackColor() {
    auto d = drawable();
    const vec3 &c = d->back_color();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    const QColor &color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec4 new_color(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        d->set_back_color(new_color);
        viewer_->update();

        QPixmap pixmap(ui->toolButtonBackColor->size());
        pixmap.fill(color);
        ui->toolButtonBackColor->setIcon(QIcon(pixmap));
    }
}


void WidgetTrianglesDrawable::setVectorField(const QString &text) {
    SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (!mesh)
        return;

    if (text == "disabled") {
        const auto &drawables = mesh->renderer()->lines_drawables();
        for (auto d : drawables) {
            if (d->name().find("vector - f") != std::string::npos)
                d->set_visible(false);
        }
        states_[drawable()].vector_field = "disabled";
    } else {
        const std::string &name = text.toStdString();
        updateVectorFieldBuffer(mesh, name);

        auto d = mesh->renderer()->get_lines_drawable("vector - " + name);
        if (d) { // just in case the vector field has been removed
            d->set_visible(true);
            states_[drawable()].vector_field = text;
        }
    }

    viewer_->update();
    window_->updateRenderingPanel();
}


void WidgetTrianglesDrawable::setScalarFieldStyle(int idx) {
    WidgetDrawable::setScalarFieldStyle(idx);
    ui->lineEditTextureFile->setText(QString::fromStdString(colormaps_[idx].name));
}


void WidgetTrianglesDrawable::updateVectorFieldBuffer(Model *model, const std::string &name) {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(model);
    if (mesh) {
        if (name == "f:normal") {
            auto normals = mesh->get_face_property<vec3>(name);
            if (!normals)
                mesh->update_face_normals();
        }

        auto prop = mesh->get_face_property<vec3>(name);
        if (!prop && name != "disabled") {
            LOG(ERROR) << "vector field '" << name << "' doesn't exist";
            return;
        }

        // a vector field is visualized as a LinesDrawable whose name is the same as the vector field
        auto drawable = mesh->renderer()->get_lines_drawable("vector - " + name);
        if (!drawable) {
            drawable = mesh->renderer()->add_lines_drawable("vector - " + name);
            drawable->set_update_func([&, name](Model *m, Drawable *d) -> void {
                const float scale = ui->doubleSpinBoxVectorFieldScale->value();
                buffers::update(dynamic_cast<SurfaceMesh*>(m), dynamic_cast<LinesDrawable*>(d), name, 0, scale);
            });
        }
    }
}


void WidgetTrianglesDrawable::disableUnavailableOptions() {
    auto d = drawable();

    bool visible = ui->checkBoxVisible->isChecked();
    ui->labelPhongShading->setEnabled(visible);
    ui->checkBoxPhongShading->setEnabled(visible);
    ui->labelLighting->setEnabled(visible);
    ui->comboBoxLightingOptions->setEnabled(visible);
    ui->labelColorScheme->setEnabled(visible);
    ui->comboBoxColorScheme->setEnabled(visible);

    bool can_modify_default_color = visible && (ui->comboBoxColorScheme->currentText() == "uniform color");
    ui->labelDefaultColor->setEnabled(can_modify_default_color);
    ui->toolButtonDefaultColor->setEnabled(can_modify_default_color);

    const auto &lighting_option = ui->comboBoxLightingOptions->currentText();
    bool can_modify_back_color = visible && lighting_option == "front and back";
    ui->labelBackColor->setEnabled(can_modify_back_color);
    ui->checkBoxBackColor->setEnabled(can_modify_back_color);
    ui->toolButtonBackColor->setEnabled(can_modify_back_color && d->distinct_back_color());

    bool can_create_texture = visible && ui->comboBoxColorScheme->currentText().contains(":texcoord");
    ui->labelTexture->setEnabled(can_create_texture);
    ui->lineEditTextureFile->setEnabled(can_create_texture);
    ui->toolButtonTextureFile->setEnabled(can_create_texture);

    bool can_modify_texture = can_create_texture && d->texture();
    ui->labelTextureRepeat->setEnabled(can_modify_texture);
    ui->spinBoxTextureRepeat->setEnabled(can_modify_texture);
    ui->spinBoxTextureFractionalRepeat->setEnabled(can_modify_texture);

    bool can_modify_highlight = visible && lighting_option != "disabled";
    ui->labelHighlight->setEnabled(can_modify_highlight);
    ui->checkBoxHighlight->setEnabled(can_modify_highlight);
    bool can_modify_highlight_range = can_modify_highlight && ui->checkBoxHighlight->isChecked();
    ui->spinBoxHighlightMin->setEnabled(can_modify_highlight_range);
    ui->spinBoxHighlightMax->setEnabled(can_modify_highlight_range);

    bool can_modify_opacity = visible && false;
    ui->labelOpacity->setEnabled(can_modify_opacity);
    ui->horizontalSliderOpacity->setEnabled(can_modify_opacity);

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
