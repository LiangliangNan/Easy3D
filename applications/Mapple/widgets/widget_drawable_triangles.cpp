#include "widget_drawable_triangles.h"

#include <QColorDialog>
#include <QFileDialog>

#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/model.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/viewer/texture.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include "paint_canvas.h"

#include "ui_widget_drawable_triangles.h"


using namespace easy3d;

WidgetTrianglesDrawable::WidgetTrianglesDrawable(QWidget *parent)
        : WidgetDrawable(parent), ui(new Ui::WidgetTrianglesDrawable) {
    ui->setupUi(this);
}


void WidgetTrianglesDrawable::connectAll() {
    // which drawable
    connect(ui->comboBoxDrawables, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setActiveDrawable(const QString &)));

    // visible
    connect(ui->checkBoxVisible, SIGNAL(toggled(bool)), this, SLOT(setDrawableVisible(bool)));

    // phong shading
    connect(ui->checkBoxPhongShading, SIGNAL(toggled(bool)), this, SLOT(setPhongShading(bool)));

    // lighting
    connect(ui->comboBoxLightingOptions, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setLighting(const QString &)));

    // color scheme
    connect(ui->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setColorScheme(const QString &)));

    // default color
    connect(ui->toolButtonDefaultColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

    // default color
    connect(ui->checkBoxBackColor, SIGNAL(toggled(bool)), this, SLOT(setDistinctBackColor(bool)));
    connect(ui->toolButtonBackColor, SIGNAL(clicked()), this, SLOT(setBackColor()));

    // texture
    connect(ui->toolButtonTextureFile, SIGNAL(clicked()), this, SLOT(setTextureFile()));
    connect(ui->spinBoxTextureRepeat, SIGNAL(valueChanged(int)), this, SLOT(setTextureRepeat(int)));
    connect(ui->spinBoxTextureFractionalRepeat, SIGNAL(valueChanged(int)),
            this, SLOT(setTextureFractionalRepeat(int)));

    // highlight
    connect(ui->checkBoxHighlight, SIGNAL(toggled(bool)), this, SLOT(setHighlight(bool)));
    connect(ui->spinBoxHighlightLow, SIGNAL(valueChanged(int)), this, SLOT(setHighlightLow(int)));
    connect(ui->spinBoxHighlightHigh, SIGNAL(valueChanged(int)), this, SLOT(setHighlightHigh(int)));

    // transparency
    connect(ui->horizontalSliderOpacity, SIGNAL(valueChanged(int)), this, SLOT(setOpacity(int)));
}


void WidgetTrianglesDrawable::disconnectAll() {
    // which drawable
    disconnect(ui->comboBoxDrawables, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setActiveDrawable(const QString &)));

    // visible
    disconnect(ui->checkBoxVisible, SIGNAL(toggled(bool)), this, SLOT(setDrawableVisible(bool)));

    // phong shading
    disconnect(ui->checkBoxPhongShading, SIGNAL(toggled(bool)), this, SLOT(setPhongShading(bool)));

    // lighting
    disconnect(ui->comboBoxLightingOptions, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setLighting(const QString &)));

    // color scheme
    disconnect(ui->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setColorScheme(const QString &)));

    // default color
    disconnect(ui->toolButtonDefaultColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

    // default color
    disconnect(ui->checkBoxBackColor, SIGNAL(toggled(bool)), this, SLOT(setDistinctBackColor(bool)));
    disconnect(ui->toolButtonBackColor, SIGNAL(clicked()), this, SLOT(setBackColor()));

    // texture
    disconnect(ui->toolButtonTextureFile, SIGNAL(clicked()), this, SLOT(setTextureFile()));
    disconnect(ui->spinBoxTextureRepeat, SIGNAL(valueChanged(int)), this, SLOT(setTextureRepeat(int)));
    disconnect(ui->spinBoxTextureFractionalRepeat, SIGNAL(valueChanged(int)),
               this, SLOT(setTextureFractionalRepeat(int)));

    // highlight
    disconnect(ui->checkBoxHighlight, SIGNAL(toggled(bool)), this, SLOT(setHighlight(bool)));
    disconnect(ui->spinBoxHighlightLow, SIGNAL(valueChanged(int)), this, SLOT(setHighlightLow(int)));
    disconnect(ui->spinBoxHighlightHigh, SIGNAL(valueChanged(int)), this, SLOT(setHighlightHigh(int)));

    // transparency
    disconnect(ui->horizontalSliderOpacity, SIGNAL(valueChanged(int)), this, SLOT(setOpacity(int)));
}


WidgetTrianglesDrawable::~WidgetTrianglesDrawable() {
    delete ui;
}


// update the panel to be consistent with the drawable's rendering parameters
void WidgetTrianglesDrawable::updatePanel() {
    auto model = viewer_->currentModel();
    if (!model || model->triangles_drawables().empty()) {
        setEnabled(false);
        return;
    }

    setEnabled(true);

    disconnectAll();

    ui->comboBoxDrawables->clear();
    const auto &drawables = model->triangles_drawables();
    for (auto d : drawables)
        ui->comboBoxDrawables->addItem(QString::fromStdString(d->name()));
    ui->comboBoxDrawables->setCurrentText(QString::fromStdString(drawable()->name()));

    // visible
    ui->checkBoxVisible->setChecked(drawable()->is_visible());
    // phong shading
    ui->checkBoxPhongShading->setChecked(drawable()->smooth_shading());

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
        auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
        ui->comboBoxColorScheme->addItem("uniform color");
        auto vcolors = mesh->get_vertex_property<vec3>("v:color");
        if (vcolors)
            ui->comboBoxColorScheme->addItem("v:color");
        auto fcolors = mesh->get_face_property<vec3>("f:color");
        if (fcolors)
            ui->comboBoxColorScheme->addItem("f:color");
        auto vtexcoords = mesh->get_vertex_property<vec2>("v:texcoord");
        if (vtexcoords)
            ui->comboBoxColorScheme->addItem("v:texcoord");
        auto htexcoords = mesh->get_halfedge_property<vec2>("h:texcoord");
        if (htexcoords)
            ui->comboBoxColorScheme->addItem("h:texcoord");

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

        // texture
        if (drawable()->texture()) {
            const auto &file = drawable()->texture()->file_name();
            ui->lineEditTextureFile->setText(QString::fromStdString(file_system::simple_name(file)));
        } else
            ui->lineEditTextureFile->setText("not available");

        ui->spinBoxTextureRepeat->setValue(drawable()->texture_repeat());
        ui->spinBoxTextureFractionalRepeat->setValue(drawable()->texture_fractional_repeat());
    }

    {   // highlight
        bool highlight = drawable()->highlight();
        ui->checkBoxHighlight->setChecked(highlight);

        const auto &range = drawable()->highlight_range();
        ui->spinBoxHighlightLow->setValue(range.first);
        ui->spinBoxHighlightHigh->setValue(range.second);
    }

    disableUnnecessaryWidgets();

    connectAll();
}


TrianglesDrawable *WidgetTrianglesDrawable::drawable() {
    auto model = viewer_->currentModel();
    auto pos = active_triangles_drawable_.find(model);
    if (pos != active_triangles_drawable_.end())
        return model->triangles_drawable(pos->second);
    else {
        const auto& drawables = model->triangles_drawables();
        if (drawables.empty())
            return nullptr;
        else {
            active_triangles_drawable_[model] = drawables[0]->name();
            return drawables[0];
        }
    }
}


void WidgetTrianglesDrawable::setActiveDrawable(const QString &text) {
    auto model = viewer_->currentModel();
    const std::string &name = text.toStdString();

    if (active_triangles_drawable_.find(model) != active_triangles_drawable_.end()) {
        if (active_triangles_drawable_[model] == name)
            return; // already active
    }

    if (model->triangles_drawable(name)) {
        active_triangles_drawable_[model] = name;
    } else {
        LOG(ERROR) << "drawable '" << name << "' not defined on model: " << model->name();
        const auto &drawables = model->triangles_drawables();
        if (drawables.empty())
            LOG(ERROR) << "no triangles drawable defined on model: " << model->name();
        else
            active_triangles_drawable_[model] = drawables[0]->name();
    }

    updatePanel();
}


void WidgetTrianglesDrawable::setDrawableVisible(bool b) {
    if (drawable()->is_visible() != b) {
        drawable()->set_visible(b);
        viewer_->update();
    }
    disableUnnecessaryWidgets();
}


void WidgetTrianglesDrawable::setPhongShading(bool b) {
    if (drawable()->smooth_shading() != b) {
        drawable()->set_smooth_shading(b);
        viewer_->update();
    }
}


void WidgetTrianglesDrawable::setLighting(const QString & text) {
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
    disableUnnecessaryWidgets();
}


void WidgetTrianglesDrawable::setColorScheme(const QString& text) {
    bool expected = text != "uniform color";
    if (drawable()->per_vertex_color() != expected) {
        drawable()->set_per_vertex_color(expected);
    }

    bool use_texture = text.contains(":texcoord");
    if (drawable()->use_texture() != use_texture)
        drawable()->set_use_texture(use_texture);

    viewer_->update();
    disableUnnecessaryWidgets();
}


void WidgetTrianglesDrawable::setDefaultColor() {
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


void WidgetTrianglesDrawable::setBackColor() {
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


void WidgetTrianglesDrawable::setDistinctBackColor(bool b) {
    if (drawable()->distinct_back_color() != b) {
        drawable()->set_distinct_back_color(b);
        viewer_->update();
        disableUnnecessaryWidgets();
    }
}


void WidgetTrianglesDrawable::setTextureFile() {
    const std::string dir = setting::resource_directory() + "/textures/";
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose an image file"), QString::fromStdString(dir),
                                                    tr("Image format (*.xpm *.png *.jpg *.bmp *.ppm)")
    );

    if (fileName.isEmpty())
        return;

    const std::string &file_name = fileName.toStdString();
    Texture *tex = drawable()->texture();
    if (tex) {
        if (tex->file_name() == file_name)
            return;

        delete tex;
    }

    tex = Texture::create(file_name, GL_REPEAT);
    if (tex) {
        drawable()->set_texture(tex);
        drawable()->set_use_texture(true);
        viewer_->update();
        ui->lineEditTextureFile->setText(QString::fromStdString(file_system::simple_name(file_name)));
    } else
        LOG(WARNING) << "failed creating texture from file: " << file_name;

    disableUnnecessaryWidgets();
}


void WidgetTrianglesDrawable::setTextureRepeat(int r) {
    if (drawable()->texture_repeat() != r) {
        drawable()->set_texture_repeat(r);
        viewer_->update();
    }
}


void WidgetTrianglesDrawable::setTextureFractionalRepeat(int r) {
    if (drawable()->texture_fractional_repeat() != r) {
        drawable()->set_texture_fractional_repeat(r);
        viewer_->update();
    }
}


void WidgetTrianglesDrawable::setHighlight(bool b) {
    if (drawable()->highlight() != b) {
        drawable()->set_highlight(b);
        viewer_->update();
        disableUnnecessaryWidgets();
    }
}


void WidgetTrianglesDrawable::setHighlightLow(int v) {
    const auto &range = drawable()->highlight_range();
    if (range.first != v) {
        drawable()->set_highlight_range(std::make_pair(v, range.second));
        viewer_->update();
    }
}


void WidgetTrianglesDrawable::setHighlightHigh(int v) {
    const auto &range = drawable()->highlight_range();
    if (range.second != v) {
        drawable()->set_highlight_range(std::make_pair(range.first, v));
        viewer_->update();
    }
}


void WidgetTrianglesDrawable::setOpacity(int a) {
    drawable()->set_opacity(a / 100.0f);
    viewer_->update();
}


void WidgetTrianglesDrawable::disableUnnecessaryWidgets() {
    bool visible = ui->checkBoxVisible->isChecked();
    ui->labelPhongShading->setEnabled(visible);
    ui->checkBoxPhongShading->setEnabled(visible);
    ui->labelLighting->setEnabled(visible);
    ui->comboBoxLightingOptions->setEnabled(visible);
    ui->labelColorScheme->setEnabled(visible);
    ui->comboBoxColorScheme->setEnabled(visible);
    ui->comboBoxLightingOptions->setEnabled(visible);
    ui->comboBoxLightingOptions->setEnabled(visible);

    bool can_modify_default_color = visible && (ui->comboBoxColorScheme->currentText() == "uniform color");
    ui->labelColor->setEnabled(can_modify_default_color);
    ui->labelDefaultColor->setEnabled(can_modify_default_color);
    ui->toolButtonDefaultColor->setEnabled(can_modify_default_color);

    const auto& lighting_option = ui->comboBoxLightingOptions->currentText();
    bool can_modify_back_color = visible && lighting_option == "front and back";
    ui->labelBackColor->setEnabled(can_modify_back_color);
    ui->checkBoxBackColor->setEnabled(can_modify_back_color);
    ui->toolButtonBackColor->setEnabled(can_modify_back_color && drawable()->distinct_back_color());

    bool can_create_texture = visible && ui->comboBoxColorScheme->currentText().contains(":texcoord");
    ui->labelTexture->setEnabled(can_create_texture);
    ui->lineEditTextureFile->setEnabled(can_create_texture);
    ui->toolButtonTextureFile->setEnabled(can_create_texture);

    bool can_modify_texture = can_create_texture && drawable()->texture();
    ui->labelTextureRepeat->setEnabled(can_modify_texture);
    ui->spinBoxTextureRepeat->setEnabled(can_modify_texture);
    ui->spinBoxTextureFractionalRepeat->setEnabled(can_modify_texture);

    bool can_modify_highlight = visible && lighting_option != "disabled";
    ui->labelHighlight->setEnabled(can_modify_highlight);
    ui->checkBoxHighlight->setEnabled(can_modify_highlight);
    bool can_modify_highlight_range = can_modify_highlight && ui->checkBoxHighlight->isChecked();
    ui->spinBoxHighlightLow->setEnabled(can_modify_highlight_range);
    ui->spinBoxHighlightHigh->setEnabled(can_modify_highlight_range);

    bool can_modify_opacity = visible && false;
    ui->labelOpacity->setEnabled(can_modify_opacity);
    ui->horizontalSliderOpacity->setEnabled(can_modify_opacity);
}
