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


#include "widget_global_setting.h"

#include <QColorDialog>

#include <easy3d/core/model.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/setting.h>
#include <easy3d/renderer/soft_shadow.h>
#include <easy3d/renderer/ambient_occlusion.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>

#include "main_window.h"
#include "paint_canvas.h"

#include <ui_widget_global_setting.h>


using namespace easy3d;

WidgetGlobalSetting::WidgetGlobalSetting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WidgetGlobalSetting)
{
    ui->setupUi(this);
    viewer_ = dynamic_cast<MainWindow*>(parent)->viewer();

    // SSAO
    ui->comboBoxSSAOAlgorithm->addItem("None");
    ui->comboBoxSSAOAlgorithm->addItem("Classic");
    ui->comboBoxSSAOAlgorithm->addItem("HBO");
    ui->comboBoxSSAOAlgorithm->addItem("HBO+");
    ui->comboBoxSSAOAlgorithm->setCurrentIndex(0);
    ui->horizontalSliderSSAORadius->setValue(setting::effect_ssao_radius * 100);
    ui->horizontalSliderSSAOIntensity->setValue(setting::effect_ssao_intensity * 100);
    ui->horizontalSliderSSAOBias->setValue(setting::effect_ssao_bias * 100);
    ui->horizontalSliderSSAOSharpness->setValue(setting::effect_ssao_sharpness * 100);

    // EDL
    ui->checkBoxEyeDomeLighting->setChecked(setting::effect_edl_enabled);
    ui->checkBoxTransparency->setChecked(setting::effect_transparency_enabled);
    ui->checkBoxShadow->setChecked(setting::effect_shadow_enabled);

    // shadow
    ui->comboBoxShadowSmoothPattern->addItem("Poisson  25 x 25 ");
    ui->comboBoxShadowSmoothPattern->addItem("Poisson  32 x 64 ");
    ui->comboBoxShadowSmoothPattern->addItem("Poisson  64 x 128");
    ui->comboBoxShadowSmoothPattern->addItem("Poisson 100 x 100");
    ui->comboBoxShadowSmoothPattern->addItem("Regular  49 x 225");
    ui->comboBoxShadowSmoothPattern->setCurrentIndex(2);
    connect(ui->comboBoxShadowSmoothPattern, SIGNAL(currentIndexChanged(int)), this, SLOT(setShadowSmoothPattern(int)));

    ui->horizontalSliderShadowLightDistance->setValue(setting::shadow_light_distance);
    ui->horizontalSliderShadowSoftness->setValue(setting::shadow_softness * 100);
    ui->horizontalSliderShadowDarkness->setValue(setting::shadow_darkness * 100);

    //----------------------------------------------

    QPalette::ColorRole role = parent->backgroundRole();
    const QColor& bc = parent->palette().color(role);
    ui->checkerSphere->setBackgroundColor(bc);
    connect(ui->checkerSphere, SIGNAL(lightPositionChanged()), viewer_, SLOT(update()));

    connect(ui->checkBoxClippingPlaneEnable, SIGNAL(toggled(bool)), this, SLOT(setEnableClippingPlane(bool)));
    connect(ui->checkBoxClippingPlaneVisible, SIGNAL(toggled(bool)), this, SLOT(setClippingPlaneVisible(bool)));
    connect(ui->toolButtonRecenterClippingPlane, SIGNAL(clicked()), this, SLOT(recenterClippingPlane()));
    connect(ui->toolButtonClippingPlaneColor, SIGNAL(clicked()), this, SLOT(setClippingPlaneColor()));
    connect(ui->checkBoxCrossSectionEnable, SIGNAL(toggled(bool)), this, SLOT(setEnableCrossSection(bool)));
    connect(ui->doubleSpinBoxCrossSectionThickness, SIGNAL(valueChanged(double)), this, SLOT(setCrossSectionThickness(double)));

    // visible
    ui->checkBoxClippingPlaneVisible->setChecked(true);
    // default color
    const vec3& c = clippingPlane()->color();
    QPixmap pixmap(ui->toolButtonClippingPlaneColor->size());
    pixmap.fill(
            QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
    ui->toolButtonClippingPlaneColor->setIcon(QIcon(pixmap));

    connect(ui->comboBoxSSAOAlgorithm, SIGNAL(currentIndexChanged(int)), this, SLOT(setSSAOAlgorithm(int)));
    connect(ui->horizontalSliderSSAORadius, SIGNAL(valueChanged(int)), this, SLOT(setSSAORadius(int)));
    connect(ui->horizontalSliderSSAOIntensity, SIGNAL(valueChanged(int)), this, SLOT(setSSAOIntensity(int)));
    connect(ui->horizontalSliderSSAOBias, SIGNAL(valueChanged(int)), this, SLOT(setSSAOBias(int)));
    connect(ui->horizontalSliderSSAOSharpness, SIGNAL(valueChanged(int)), this, SLOT(setSSAOSharpness(int)));

    connect(ui->checkBoxEyeDomeLighting, SIGNAL(toggled(bool)), this, SLOT(setEyeDomeLighting(bool)));
    connect(ui->checkBoxTransparency, SIGNAL(toggled(bool)), this, SLOT(setTransparency(bool)));
    connect(ui->checkBoxShadow, SIGNAL(toggled(bool)), this, SLOT(setShadow(bool)));

    connect(ui->horizontalSliderShadowLightDistance, SIGNAL(valueChanged(int)), this, SLOT(setLightDistance(int)));
    connect(ui->horizontalSliderShadowSoftness, SIGNAL(valueChanged(int)), this, SLOT(setShadowSoftness(int)));
    connect(ui->horizontalSliderShadowDarkness, SIGNAL(valueChanged(int)), this, SLOT(setShadowDarkness(int)));
}


WidgetGlobalSetting::~WidgetGlobalSetting()
{
    delete ui;
    if (clippingPlane())
        delete clippingPlane();
}


easy3d::ClippingPlane* WidgetGlobalSetting::clippingPlane() const {
    if (!setting::clipping_plane) {
        setting::clipping_plane = new ClippingPlane;
        // connect the manipulator's signal to the viewer's update function to automatically update rendering.
        setting::clipping_plane->manipulator()->frame()->modified.connect(viewer_,
                static_cast<void (PaintCanvas::*)(void)>(&PaintCanvas::update));
    }
    return setting::clipping_plane;
}


void WidgetGlobalSetting::setEnableClippingPlane(bool b) {
    clippingPlane()->set_enabled(b);
    if (b) {
        static bool init_size = false;
        if (!init_size) {
            recenterClippingPlane();
            init_size = true;
        }
    }
    viewer_->update();
    disableUnavailableOptions();

    if (b)
        LOG(INFO) << "clipping plane enabled";
    else
        LOG(INFO) << "clipping plane disabled";
}


void WidgetGlobalSetting::setClippingPlaneVisible(bool b) {
    clippingPlane()->set_visible(b);
    viewer_->update();
    disableUnavailableOptions();
}


void WidgetGlobalSetting::recenterClippingPlane() {
    Box3 box;
    for (auto m : viewer_->models()) {
        if (m->renderer()->is_visible())
            box.add_box(m->bounding_box(true));
    }

    if (box.is_valid()) {
        clippingPlane()->fit_scene(box.center(), box.radius());
        viewer_->camera()->setSceneBoundingBox(box.min_point(), box.max_point());
        viewer_->update();
    }
}


void WidgetGlobalSetting::setClippingPlaneColor() {
    const vec4 &c = clippingPlane()->color();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    const QColor &color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec4 new_color(color.redF(), color.greenF(), color.blueF(), c.a);
        clippingPlane()->set_color(new_color);
        viewer_->update();

        QPixmap pixmap(ui->toolButtonClippingPlaneColor->size());
        pixmap.fill(color);
        ui->toolButtonClippingPlaneColor->setIcon(QIcon(pixmap));
    }
}


void WidgetGlobalSetting::setEnableCrossSection(bool b) {
    clippingPlane()->set_cross_section(b);
    viewer_->update();
    disableUnavailableOptions();

    if (b)
        LOG(INFO) << "cross-section view enabled";
    else
        LOG(INFO) << "cross-section view disabled";
}


void WidgetGlobalSetting::setCrossSectionThickness(double w) {
    clippingPlane()->set_cross_section_width(w);
    viewer_->update();
    LOG(INFO) << "cross-section thickness: " << w;
}


void WidgetGlobalSetting::setSSAOAlgorithm(int algo) {
    viewer_->enableSsao(algo != 0);
    if (algo != 0 && ui->checkBoxTransparency->isChecked()) // ssao and transparency cannot co-exist
        ui->checkBoxTransparency->setChecked(false);

    if (algo != 0 && ui->checkBoxShadow->isChecked())
        ui->checkBoxShadow->setChecked(false);

    viewer_->update();
}


void WidgetGlobalSetting::setSSAORadius(int v) {
    if (viewer_->ssao()) {
        viewer_->ssao()->set_radius(v / 100.0f);
        viewer_->update();
    }
}


void WidgetGlobalSetting::setSSAOIntensity(int v) {
    if (viewer_->ssao()) {
//    viewer_->ssao()->set_intensity(v / 100.0f);
//    viewer_->update();
    }
}


void WidgetGlobalSetting::setSSAOBias(int v) {
    if (viewer_->ssao()) {
        viewer_->ssao()->set_bias(v / 100.0f);
        viewer_->update();
    }
}


void WidgetGlobalSetting::setSSAOSharpness(int v) {
    if (viewer_->ssao()) {
//    viewer_->ssao()->set_sharpness(v / 100.0f);
//    viewer_->update();
    }
}


void WidgetGlobalSetting::setEyeDomeLighting(bool b) {
    if (b && ui->checkBoxShadow->isChecked())
        ui->checkBoxShadow->setChecked(false);  // shadow and EDL cannot co-exist

    if (b && ui->checkBoxTransparency->isChecked())
        ui->checkBoxTransparency->setChecked(false);  // transparency and EDL cannot co-exist
        
    viewer_->enableEyeDomeLighting(b);
    viewer_->update();
}


void WidgetGlobalSetting::setTransparency(bool b) {
    if (b && ui->checkBoxShadow->isChecked())
        ui->checkBoxShadow->setChecked(false);  // shadow and transparency cannot co-exist

    if (b && ui->checkBoxEyeDomeLighting->isChecked())
        ui->checkBoxEyeDomeLighting->setChecked(false);  // transparency and EDL cannot co-exist

    if (b && ui->comboBoxSSAOAlgorithm->currentIndex() != 0)
        ui->comboBoxSSAOAlgorithm->setCurrentIndex(0);

    viewer_->enableTransparency(b);
    viewer_->update();

}


void WidgetGlobalSetting::setShadow(bool b) {
    if (b && ui->checkBoxTransparency->isChecked())
        ui->checkBoxTransparency->setChecked(false);   // shadow and transparency cannot co-exist
    if (b && ui->checkBoxEyeDomeLighting->isChecked())
        ui->checkBoxEyeDomeLighting->setChecked(false);  // shadow and EDL cannot co-exist

    if (b && ui->comboBoxSSAOAlgorithm->currentIndex() != 0)
        ui->comboBoxSSAOAlgorithm->setCurrentIndex(0);

    viewer_->enableShadow(b);
    viewer_->update();
}


void WidgetGlobalSetting::setLightDistance(int d) {
    viewer_->shadow()->set_light_distance(d);
    viewer_->update();
}


void WidgetGlobalSetting::setShadowSmoothPattern(int v) {
    SoftShadow* shadow = dynamic_cast<SoftShadow*>(viewer_->shadow());
    if (shadow) {
        shadow->set_sample_pattern(SoftShadow::SamplePattern(v));
        viewer_->update();
    }
}


void WidgetGlobalSetting::setShadowSoftness(int v) {
    SoftShadow* shadow = dynamic_cast<SoftShadow*>(viewer_->shadow());
    if (shadow) {
        shadow->set_softness(static_cast<float>(v) / 100.0f);
        viewer_->update();
    }
}


void WidgetGlobalSetting::setShadowDarkness(int v) {
    viewer_->shadow()->set_darkness(static_cast<float>(v) / 100.0f);
    viewer_->update();
}


void WidgetGlobalSetting::setImposterShadows(bool) {

}


void WidgetGlobalSetting::disableUnavailableOptions() {
    // ground plane
    bool visible = ui->checkBoxGroundPlane->isChecked();
    ui->spinBoxGroundPlaneSize->setEnabled(visible);
    ui->labelGroundPlaneTexture->setEnabled(visible);
    ui->checkBoxGroundPlaneTexture->setEnabled(visible);
    bool can_change_ground_plane_texture = visible && (ui->checkBoxGroundPlaneTexture->isChecked());
    ui->toolButtonGroundPlaneTexture->setEnabled(can_change_ground_plane_texture);

    // clipping plane
    visible = ui->checkBoxClippingPlaneEnable->isChecked();
    ui->toolButtonRecenterClippingPlane->setEnabled(visible);
    ui->labelClippingPlaneVisible->setEnabled(visible);
    ui->checkBoxClippingPlaneVisible->setEnabled(visible);
    bool can_change_clipping_plane_color = visible && (ui->checkBoxClippingPlaneVisible->isChecked());
    ui->toolButtonClippingPlaneColor->setEnabled(can_change_clipping_plane_color);
    ui->labelCrossSectionEnable->setEnabled(visible);
    ui->checkBoxCrossSectionEnable->setEnabled(visible);
    bool can_change_crosssection_thickness = visible && (ui->checkBoxCrossSectionEnable->isChecked());
    ui->doubleSpinBoxCrossSectionThickness->setEnabled(can_change_crosssection_thickness);

    update();
    qApp->processEvents();
}
