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
    connect(ui->comboBoxLightingOptions, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setLighting(int)));

    // color scheme
    connect(ui->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(setColorScheme(const QString &)));

    // default color
    connect(ui->toolButtonFrontColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

    // default color
    connect(ui->checkBoxDistinctBackColor, SIGNAL(toggled(bool)), this, SLOT(setDistinctBackColor(bool)));
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
    disconnect(ui->comboBoxLightingOptions, SIGNAL(currentIndexChanged(int)),
               this, SLOT(setLighting(int)));

    // color scheme
    disconnect(ui->comboBoxColorScheme, SIGNAL(currentIndexChanged(const QString &)),
               this, SLOT(setColorScheme(const QString &)));

    // default color
    disconnect(ui->toolButtonFrontColor, SIGNAL(clicked()), this, SLOT(setDefaultColor()));

    // default color
    disconnect(ui->checkBoxDistinctBackColor, SIGNAL(toggled(bool)), this, SLOT(setDistinctBackColor(bool)));
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

    disconnectAll();

    setEnabled(true);

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
            if (drawable()->lighting_two_sides()) {
                ui->comboBoxLightingOptions->setCurrentIndex(0);
                ui->labelDefaultColorBack->setEnabled(true);
                ui->checkBoxDistinctBackColor->setEnabled(true);
                ui->toolButtonBackColor->setEnabled(drawable()->distinct_back_color());
            }
            else {
                ui->comboBoxLightingOptions->setCurrentIndex(1);
                ui->labelDefaultColorBack->setEnabled(false);
                ui->checkBoxDistinctBackColor->setEnabled(false);
                ui->toolButtonBackColor->setEnabled(false);
            }
            ui->labelHighlight->setEnabled(true);
            ui->spinBoxHighlightLow->setEnabled(true);
            ui->spinBoxHighlightHigh->setEnabled(true);
        }
        else {
            ui->comboBoxLightingOptions->setCurrentIndex(2);
            ui->labelDefaultColorBack->setEnabled(false);
            ui->checkBoxDistinctBackColor->setEnabled(false);
            ui->toolButtonBackColor->setEnabled(false);
            ui->labelHighlight->setEnabled(false);
            ui->spinBoxHighlightLow->setEnabled(false);
            ui->spinBoxHighlightHigh->setEnabled(false);
        }
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
            const auto &name = drawable()->color_scheme();
            ui->comboBoxColorScheme->setCurrentText(QString::fromStdString(name));

            ui->labelDefaultColor->setEnabled(false);
            ui->labelDefaultColorFront->setEnabled(false);
            ui->toolButtonFrontColor->setEnabled(false);

            bool texturing = ui->comboBoxColorScheme->currentText().endsWith("texcoord");
            ui->labelTextureFile->setEnabled(texturing);
            ui->lineEditTextureFile->setEnabled(texturing);
            ui->toolButtonTextureFile->setEnabled(texturing);
            ui->labelTextureRepeat->setEnabled(texturing);
            ui->spinBoxTextureRepeat->setEnabled(texturing);
            ui->spinBoxTextureFractionalRepeat->setEnabled(texturing);
        } else {
            ui->labelDefaultColor->setEnabled(true);
            ui->labelDefaultColorFront->setEnabled(true);
            ui->toolButtonFrontColor->setEnabled(true);

            ui->comboBoxColorScheme->setCurrentIndex(0);
            ui->labelTextureFile->setEnabled(false);
            ui->lineEditTextureFile->setEnabled(false);
            ui->toolButtonTextureFile->setEnabled(false);
            ui->labelTextureRepeat->setEnabled(false);
            ui->spinBoxTextureRepeat->setEnabled(false);
            ui->spinBoxTextureFractionalRepeat->setEnabled(false);
        }

        // default color
        vec3 c = drawable()->default_color();
        QPixmap pixmap(ui->toolButtonFrontColor->size());
        pixmap.fill(
                QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonFrontColor->setIcon(QIcon(pixmap));

        // back side color
        ui->checkBoxDistinctBackColor->setChecked(drawable()->distinct_back_color());
        c = drawable()->back_color();
        pixmap.fill(
                QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonBackColor->setIcon(QIcon(pixmap));
        ui->toolButtonBackColor->setEnabled(drawable()->distinct_back_color());

        // texture
        if (drawable()->texture()) {
            const auto &file = drawable()->texture()->file_name();
            ui->lineEditTextureFile->setText(QString::fromStdString(file_system::simple_name(file)));
        }
        ui->spinBoxTextureRepeat->setValue(drawable()->texture_repeat());
        ui->spinBoxTextureFractionalRepeat->setValue(drawable()->texture_fractional_repeat());
    }

    {   // highlight
        const auto &range = drawable()->highlight_range();
        bool can_modify = range.first != -1 && range.second != -1;
        ui->checkBoxHighlight->setChecked(can_modify);
        ui->spinBoxHighlightLow->setValue(range.first);
        ui->spinBoxHighlightHigh->setValue(range.second);
        ui->spinBoxHighlightLow->setEnabled(can_modify);
        ui->spinBoxHighlightHigh->setEnabled(can_modify);
    }

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
}


void WidgetTrianglesDrawable::setDrawableVisible(bool b) {
    if (drawable()->is_visible() != b) {
        drawable()->set_visible(b);
        viewer_->update();
    }
}


void WidgetTrianglesDrawable::setPhongShading(bool b) {
    if (drawable()->smooth_shading() != b) {
        drawable()->set_smooth_shading(b);
        viewer_->update();
    }
}


void WidgetTrianglesDrawable::setLighting(int b) {
    switch (b) {
        case 0: {
            drawable()->set_lighting(true);
            drawable()->set_lighting_two_sides(true);

            bool coloring = (ui->comboBoxColorScheme->currentIndex() == 0);
            ui->labelDefaultColor->setEnabled(coloring);
            ui->labelDefaultColorFront->setEnabled(coloring);
            ui->toolButtonFrontColor->setEnabled(coloring);
            ui->labelDefaultColorBack->setEnabled(coloring);
            ui->checkBoxDistinctBackColor->setEnabled(coloring);
            ui->toolButtonBackColor->setEnabled(coloring && drawable()->distinct_back_color());

            ui->labelHighlight->setEnabled(true);
            ui->spinBoxHighlightLow->setEnabled(true);
            ui->spinBoxHighlightHigh->setEnabled(true);
            break;
        }
        case 1: {
            drawable()->set_lighting(true);
            drawable()->set_lighting_two_sides(false);
            ui->labelDefaultColorBack->setEnabled(false);
            ui->checkBoxDistinctBackColor->setEnabled(false);
            ui->toolButtonBackColor->setEnabled(false);
            ui->labelHighlight->setEnabled(true);
            ui->spinBoxHighlightLow->setEnabled(true);
            ui->spinBoxHighlightHigh->setEnabled(true);
            break;
        }
        case 2: {
            drawable()->set_lighting(false);
            ui->labelDefaultColorBack->setEnabled(false);
            ui->checkBoxDistinctBackColor->setEnabled(false);
            ui->toolButtonBackColor->setEnabled(false);
            ui->labelHighlight->setEnabled(false);
            ui->spinBoxHighlightLow->setEnabled(false);
            ui->spinBoxHighlightHigh->setEnabled(false);
            break;
        }
    }
    viewer_->update();
}


void WidgetTrianglesDrawable::setColorScheme(const QString& text) {
    if (text == "uniform color") {
        drawable()->set_per_vertex_color(false);
        ui->labelDefaultColor->setEnabled(true);
        ui->labelDefaultColorFront->setEnabled(true);
        ui->toolButtonFrontColor->setEnabled(true);

        bool can_change_back = (ui->comboBoxLightingOptions->currentIndex() == 0);
        ui->labelDefaultColorBack->setEnabled(can_change_back);
        ui->checkBoxDistinctBackColor->setEnabled(can_change_back);
        ui->toolButtonBackColor->setEnabled(can_change_back && drawable()->distinct_back_color());

        bool texturing = false;
        drawable()->set_use_texture(texturing);
        ui->labelTextureFile->setEnabled(texturing);
        ui->lineEditTextureFile->setEnabled(texturing);
        ui->toolButtonTextureFile->setEnabled(texturing);
        ui->labelTextureRepeat->setEnabled(texturing);
        ui->spinBoxTextureRepeat->setEnabled(texturing);
        ui->spinBoxTextureFractionalRepeat->setEnabled(texturing);

    } else {
        drawable()->set_per_vertex_color(true);
        drawable()->set_per_vertex_color(true);

        bool coloring = false;
        ui->labelDefaultColor->setEnabled(coloring);
        ui->labelDefaultColorFront->setEnabled(coloring);
        ui->toolButtonFrontColor->setEnabled(coloring);

        bool can_change_back = coloring && (ui->comboBoxLightingOptions->currentIndex() == 0);
        ui->labelDefaultColorBack->setEnabled(can_change_back);
        ui->checkBoxDistinctBackColor->setEnabled(can_change_back);
        ui->toolButtonBackColor->setEnabled(can_change_back && drawable()->distinct_back_color());

        bool texturing = ui->comboBoxColorScheme->currentText().contains(":texcoord");
        drawable()->set_use_texture(texturing);
        ui->labelTextureFile->setEnabled(texturing);
        ui->lineEditTextureFile->setEnabled(texturing);
        ui->toolButtonTextureFile->setEnabled(texturing);
        ui->labelTextureRepeat->setEnabled(texturing);
        ui->spinBoxTextureRepeat->setEnabled(texturing);
        ui->spinBoxTextureFractionalRepeat->setEnabled(texturing);
    }
    viewer_->update();
}


void WidgetTrianglesDrawable::setDefaultColor() {
    const vec3 &c = drawable()->default_color();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    const QColor &color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec3 new_color(color.redF(), color.greenF(), color.blueF());
        drawable()->set_default_color(new_color);
        viewer_->update();

        QPixmap pixmap(ui->toolButtonFrontColor->size());
        pixmap.fill(color);
        ui->toolButtonFrontColor->setIcon(QIcon(pixmap));
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
        ui->toolButtonBackColor->setEnabled(b);
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
        viewer_->update();
        ui->lineEditTextureFile->setText(QString::fromStdString(file_system::simple_name(file_name)));
    } else
        LOG(WARNING) << "failed creating texture from file: " << file_name;
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
    if (!b) {
        if (ui->spinBoxHighlightLow->value() != -1)
            ui->spinBoxHighlightLow->setValue(-1);
        if (ui->spinBoxHighlightHigh->value() != -1)
            ui->spinBoxHighlightHigh->setValue(-1);
    }
    ui->spinBoxHighlightLow->setEnabled(b);
    ui->spinBoxHighlightHigh->setEnabled(b);
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


