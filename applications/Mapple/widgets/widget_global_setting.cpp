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
#include <easy3d/renderer/soft_shadow.h>
#include <easy3d/renderer/ambient_occlusion.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/util/setting.h>

#include "main_window.h"
#include "paint_canvas.h"
#include "widgets/widget_drawable_triangles.h"

#include <ui_widget_global_setting.h>


using namespace easy3d;

WidgetGlobalSetting::WidgetGlobalSetting(QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::WidgetGlobalSetting)
    , widgetTrianglesDrawable_(nullptr)
{
    ui_->setupUi(this);
    viewer_ = dynamic_cast<MainWindow*>(parent)->viewer();

    // SSAO
    ui_->comboBoxSSAOAlgorithm->addItem("None");
    ui_->comboBoxSSAOAlgorithm->addItem("Classic");
    ui_->comboBoxSSAOAlgorithm->addItem("HBO");
    ui_->comboBoxSSAOAlgorithm->addItem("HBO+");
    ui_->comboBoxSSAOAlgorithm->setCurrentIndex(0);
    ui_->horizontalSliderSSAORadius->setValue(setting::effect_ssao_radius * 100);
    ui_->horizontalSliderSSAOIntensity->setValue(setting::effect_ssao_intensity * 100);
    ui_->horizontalSliderSSAOBias->setValue(setting::effect_ssao_bias * 100);
    ui_->horizontalSliderSSAOSharpness->setValue(setting::effect_ssao_sharpness * 100);

    // EDL
    ui_->checkBoxEyeDomeLighting->setChecked(setting::effect_edl_enabled);
    ui_->checkBoxTransparency->setChecked(setting::effect_transparency_enabled);
    ui_->checkBoxShadow->setChecked(setting::effect_shadow_enabled);

    // shadow
    ui_->comboBoxShadowSmoothPattern->addItem("Poisson  25 x 25 ");
    ui_->comboBoxShadowSmoothPattern->addItem("Poisson  32 x 64 ");
    ui_->comboBoxShadowSmoothPattern->addItem("Poisson  64 x 128");
    ui_->comboBoxShadowSmoothPattern->addItem("Poisson 100 x 100");
    ui_->comboBoxShadowSmoothPattern->addItem("Regular  49 x 225");
    ui_->comboBoxShadowSmoothPattern->setCurrentIndex(2);
    connect(ui_->comboBoxShadowSmoothPattern, SIGNAL(currentIndexChanged(int)), this, SLOT(setShadowSmoothPattern(int)));

    ui_->horizontalSliderShadowLightDistance->setValue(setting::effect_shadow_light_distance);
    ui_->horizontalSliderShadowSoftness->setValue(setting::effect_shadow_softness * 100);
    ui_->horizontalSliderShadowDarkness->setValue(setting::effect_shadow_darkness * 100);

    //----------------------------------------------

    QPalette::ColorRole role = parent->backgroundRole();
    const QColor& bc = parent->palette().color(role);
    ui_->checkerSphere->setBackgroundColor(bc);
    connect(ui_->checkerSphere, SIGNAL(lightPositionChanged()), viewer_, SLOT(update()));

    connect(ui_->checkBoxClippingPlaneEnable, SIGNAL(toggled(bool)), this, SLOT(setEnableClippingPlane(bool)));
    connect(ui_->checkBoxClippingPlaneVisible, SIGNAL(toggled(bool)), this, SLOT(setClippingPlaneVisible(bool)));
    connect(ui_->toolButtonRecenterClippingPlane, SIGNAL(clicked()), this, SLOT(recenterClippingPlane()));
    connect(ui_->toolButtonClippingPlaneColor, SIGNAL(clicked()), this, SLOT(setClippingPlaneColor()));
    connect(ui_->checkBoxCrossSectionEnable, SIGNAL(toggled(bool)), this, SLOT(setEnableCrossSection(bool)));
    connect(ui_->doubleSpinBoxCrossSectionThickness, SIGNAL(valueChanged(double)), this, SLOT(setCrossSectionThickness(double)));

    // visible
    ui_->checkBoxClippingPlaneVisible->setChecked(true);
    // default color
    const auto& c = ClippingPlane::instance()->color();
    QPixmap pixmap(ui_->toolButtonClippingPlaneColor->size());
    pixmap.fill(
            QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
    ui_->toolButtonClippingPlaneColor->setIcon(QIcon(pixmap));

    connect(ui_->comboBoxSSAOAlgorithm, SIGNAL(currentIndexChanged(int)), this, SLOT(setSSAOAlgorithm(int)));
    connect(ui_->horizontalSliderSSAORadius, SIGNAL(valueChanged(int)), this, SLOT(setSSAORadius(int)));
    connect(ui_->horizontalSliderSSAOIntensity, SIGNAL(valueChanged(int)), this, SLOT(setSSAOIntensity(int)));
    connect(ui_->horizontalSliderSSAOBias, SIGNAL(valueChanged(int)), this, SLOT(setSSAOBias(int)));
    connect(ui_->horizontalSliderSSAOSharpness, SIGNAL(valueChanged(int)), this, SLOT(setSSAOSharpness(int)));

    connect(ui_->checkBoxEyeDomeLighting, SIGNAL(toggled(bool)), this, SLOT(setEyeDomeLighting(bool)));
    connect(ui_->checkBoxTransparency, SIGNAL(toggled(bool)), this, SLOT(setTransparency(bool)));
    connect(ui_->checkBoxShadow, SIGNAL(toggled(bool)), this, SLOT(setShadow(bool)));

    connect(ui_->horizontalSliderShadowLightDistance, SIGNAL(valueChanged(int)), this, SLOT(setLightDistance(int)));
    connect(ui_->horizontalSliderShadowSoftness, SIGNAL(valueChanged(int)), this, SLOT(setShadowSoftness(int)));
    connect(ui_->horizontalSliderShadowDarkness, SIGNAL(valueChanged(int)), this, SLOT(setShadowDarkness(int)));
}


WidgetGlobalSetting::~WidgetGlobalSetting()
{
    delete ui_;
}


void WidgetGlobalSetting::setEnableClippingPlane(bool b) {
    ClippingPlane::instance()->set_enabled(b);
    // connect the manipulator's signal to the viewer's update function to automatically update rendering.
    ClippingPlane::instance()->manipulator()->frame()->modified.connect(viewer_,
                                                                        static_cast<void (PaintCanvas::*)(void)>(&PaintCanvas::update));
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
    ClippingPlane::instance()->set_visible(b);
    viewer_->update();
    disableUnavailableOptions();
}


void WidgetGlobalSetting::recenterClippingPlane() {
    Box3 box;
    for (auto m : viewer_->models()) {
        if (m->renderer()->is_visible())
            box.grow(m->bounding_box(true));
    }

    if (box.is_valid()) {
        ClippingPlane::instance()->fit_scene(box.center(), box.radius());
        viewer_->camera()->setSceneBoundingBox(box.min_point(), box.max_point());
        viewer_->update();
    }
}


void WidgetGlobalSetting::setClippingPlaneColor() {
    const auto &c = ClippingPlane::instance()->color();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
    const QColor &color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec4 new_color(color.redF(), color.greenF(), color.blueF(), c.a);
        ClippingPlane::instance()->set_color(new_color);
        viewer_->update();

        QPixmap pixmap(ui_->toolButtonClippingPlaneColor->size());
        pixmap.fill(color);
        ui_->toolButtonClippingPlaneColor->setIcon(QIcon(pixmap));
    }
}


void WidgetGlobalSetting::setEnableCrossSection(bool b) {
    ClippingPlane::instance()->set_cross_section(b);
    viewer_->update();
    disableUnavailableOptions();

    if (b)
        LOG(INFO) << "cross-section view enabled";
    else
        LOG(INFO) << "cross-section view disabled";
}


void WidgetGlobalSetting::setCrossSectionThickness(double w) {
    ClippingPlane::instance()->set_cross_section_width(w);
    viewer_->update();
    LOG(INFO) << "cross-section thickness: " << w;
}


void WidgetGlobalSetting::setSSAOAlgorithm(int algo) {
    viewer_->enableSsao(algo != 0);
    if (algo != 0 && ui_->checkBoxTransparency->isChecked()) // ssao and transparency cannot co-exist
        ui_->checkBoxTransparency->setChecked(false);

    if (algo != 0 && ui_->checkBoxShadow->isChecked())
        ui_->checkBoxShadow->setChecked(false);

    viewer_->update();
    disableUnavailableOptions();
    if (algo != 0)
        LOG(INFO) << "SSAO enabled";
    else
        LOG(INFO) << "SSAO disabled";
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
    if (b && ui_->checkBoxShadow->isChecked())
        ui_->checkBoxShadow->setChecked(false);  // shadow and EDL cannot co-exist

    if (b && ui_->checkBoxTransparency->isChecked())
        ui_->checkBoxTransparency->setChecked(false);  // transparency and EDL cannot co-exist
        
    viewer_->enableEyeDomeLighting(b);
    viewer_->update();
}


void WidgetGlobalSetting::setTransparency(bool b) {
    if (b && ui_->checkBoxShadow->isChecked())
        ui_->checkBoxShadow->setChecked(false);  // shadow and transparency cannot co-exist

    if (b && ui_->checkBoxEyeDomeLighting->isChecked())
        ui_->checkBoxEyeDomeLighting->setChecked(false);  // transparency and EDL cannot co-exist

    if (b && ui_->comboBoxSSAOAlgorithm->currentIndex() != 0)
        ui_->comboBoxSSAOAlgorithm->setCurrentIndex(0);

    viewer_->enableTransparency(b);
    viewer_->update();

    // updates the triangles drawable panel
    widgetTrianglesDrawable_->updatePanel();
}


void WidgetGlobalSetting::setShadow(bool b) {
    if (b && ui_->checkBoxTransparency->isChecked())
        ui_->checkBoxTransparency->setChecked(false);   // shadow and transparency cannot co-exist
    if (b && ui_->checkBoxEyeDomeLighting->isChecked())
        ui_->checkBoxEyeDomeLighting->setChecked(false);  // shadow and EDL cannot co-exist

    if (b && ui_->comboBoxSSAOAlgorithm->currentIndex() != 0)
        ui_->comboBoxSSAOAlgorithm->setCurrentIndex(0);

    viewer_->enableShadow(b);
    viewer_->update();

    disableUnavailableOptions();
    if (b)
        LOG(INFO) << "shadow enabled";
    else
        LOG(INFO) << "shadow disabled";
}


void WidgetGlobalSetting::setLightDistance(int d) {
    viewer_->shadow()->set_light_distance(d);
    viewer_->update();
}


void WidgetGlobalSetting::setShadowSmoothPattern(int v) {
    auto shadow = dynamic_cast<SoftShadow*>(viewer_->shadow());
    if (shadow) {
        shadow->set_sample_pattern(SoftShadow::SamplePattern(v));
        viewer_->update();
    }
}


void WidgetGlobalSetting::setShadowSoftness(int v) {
    auto shadow = dynamic_cast<SoftShadow*>(viewer_->shadow());
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
    bool visible = ui_->checkBoxGroundPlane->isChecked();
    ui_->spinBoxGroundPlaneSize->setEnabled(visible);
    ui_->labelGroundPlaneTexture->setEnabled(visible);
    ui_->checkBoxGroundPlaneTexture->setEnabled(visible);
    bool can_change_ground_plane_texture = visible && (ui_->checkBoxGroundPlaneTexture->isChecked());
    ui_->toolButtonGroundPlaneTexture->setEnabled(can_change_ground_plane_texture);

    // clipping plane
    visible = ui_->checkBoxClippingPlaneEnable->isChecked();
    ui_->toolButtonRecenterClippingPlane->setEnabled(visible);
    ui_->labelClippingPlaneVisible->setEnabled(visible);
    ui_->checkBoxClippingPlaneVisible->setEnabled(visible);
    bool can_change_clipping_plane_color = visible && (ui_->checkBoxClippingPlaneVisible->isChecked());
    ui_->toolButtonClippingPlaneColor->setEnabled(can_change_clipping_plane_color);
    ui_->labelCrossSectionEnable->setEnabled(visible);
    ui_->checkBoxCrossSectionEnable->setEnabled(visible);
    bool can_change_crosssection_thickness = visible && (ui_->checkBoxCrossSectionEnable->isChecked());
    ui_->doubleSpinBoxCrossSectionThickness->setEnabled(can_change_crosssection_thickness);

    // SSAO
    visible = (ui_->comboBoxSSAOAlgorithm->currentIndex() != 0);
    ui_->labelSSAORadius->setEnabled(visible);
    ui_->horizontalSliderSSAORadius->setEnabled(visible);
    ui_->labelSSAOIntensity->setEnabled(visible);
    ui_->horizontalSliderSSAOIntensity->setEnabled(visible);
    ui_->labelSSAOBias->setEnabled(visible);
    ui_->horizontalSliderSSAOBias->setEnabled(visible);
    ui_->labelSSAOSharpness->setEnabled(visible);
    ui_->horizontalSliderSSAOSharpness->setEnabled(visible);

    // shadow
    visible = ui_->checkBoxShadow->isChecked();
    ui_->labelShadowSmoothPattern->setEnabled(visible);
    ui_->comboBoxShadowSmoothPattern->setEnabled(visible);
    ui_->labelShadowLightDistance->setEnabled(visible);
    ui_->horizontalSliderShadowLightDistance->setEnabled(visible);
    ui_->labelShadowSoftness->setEnabled(visible);
    ui_->horizontalSliderShadowSoftness->setEnabled(visible);
    ui_->labelShadowDarkness->setEnabled(visible);
    ui_->horizontalSliderShadowDarkness->setEnabled(visible);

    update();
    qApp->processEvents();
}
