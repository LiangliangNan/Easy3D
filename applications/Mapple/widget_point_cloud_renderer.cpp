#include "widget_point_cloud_renderer.h"

#include <QColorDialog>

#include "ui_widget_point_cloud_renderer.h"

#include "main_window.h"
#include "paint_canvas.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/util/logging.h>


using namespace easy3d;


WidgetPointCloudRenderer::WidgetPointCloudRenderer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WidgetPointCloudRenderer)
{
    ui->setupUi(this);
    viewer_ = dynamic_cast<MainWindow*>(parent)->viewer();

    // vertices
    connect(ui->checkBoxUseColorProperty, SIGNAL(toggled(bool)), this, SLOT(setUseColorProperty(bool)));
    connect(ui->checkBoxShowVertices, SIGNAL(toggled(bool)), this, SLOT(setShowVertices(bool)));
    connect(ui->toolButtonVerticesDefaultColor, SIGNAL(clicked()), this, SLOT(setVerticesDefaultColor()));
    connect(ui->toolButtonVerticesImpostors, SIGNAL(toggled(bool)), this, SLOT(setVerticesImpostors(bool)));
    connect(ui->doubleSpinBoxVerticesSize, SIGNAL(valueChanged(double)), this, SLOT(setVerticesSize(double)));
}


WidgetPointCloudRenderer::~WidgetPointCloudRenderer()
{
    delete ui;
}


PointCloud* WidgetPointCloudRenderer::cloud() {
    return dynamic_cast<PointCloud*>(viewer_->currentModel());
}


// update the panel to be consistent with the drawable's rendering parameters
void WidgetPointCloudRenderer::updatePanel() {
    if (!cloud())
        return;

    // vertices
    PointsDrawable* vertices = cloud()->points_drawable("vertices");
    if (vertices) {
        ui->checkBoxUseColorProperty->setChecked(vertices->per_vertex_color());
        ui->checkBoxShowVertices->setChecked(vertices->is_visible());
        const vec3& c = vertices->default_color();
        QPixmap pixmap(ui->toolButtonVerticesDefaultColor->size());
        pixmap.fill(QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonVerticesDefaultColor->setIcon(QIcon(pixmap));
        ui->toolButtonVerticesImpostors->setChecked(vertices->impostor_type() != PointsDrawable::PLAIN);
        ui->doubleSpinBoxVerticesSize->setValue(vertices->point_size());
    }
    else {
        ui->checkBoxUseColorProperty->setChecked(setting::point_cloud_use_color_property);
        ui->checkBoxShowVertices->setChecked(setting::point_cloud_show_points);
        const vec3& c = setting::point_cloud_points_color;
        QPixmap pixmap(ui->toolButtonVerticesDefaultColor->size());
        pixmap.fill(QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonVerticesDefaultColor->setIcon(QIcon(pixmap));
        ui->toolButtonVerticesImpostors->setChecked(setting::point_cloud_impostors);
        ui->doubleSpinBoxVerticesSize->setValue(setting::point_cloud_point_size);
    }

    // scalar field
    // collect all available scalar field
    //...


}


//make sure the appropriate rendering data are uploaded to GPU
void WidgetPointCloudRenderer::ensureBuffers() {
    PointsDrawable* drawable = cloud()->points_drawable("vertices");
    if (drawable) {

    }
}


void WidgetPointCloudRenderer::setUseColorProperty(bool b) {
    if (b) { // just check and warn user if color not available
        auto colors = cloud()->get_vertex_property<vec3>("v:color");
        if (!colors)
            LOG(WARNING) << "no color property defined on vertices";
    }

    PointsDrawable* vertices = cloud()->points_drawable("vertices");
    if (vertices) {
        vertices->set_per_vertex_color(b);
        vertices->set_default_color(setting::point_cloud_points_color);
        viewer_->update();
    }
}


// vertices
void WidgetPointCloudRenderer::setShowVertices(bool b) {
    PointsDrawable* vertices = cloud()->points_drawable("vertices");
    if (b && !vertices) {
        vertices = cloud()->add_points_drawable("vertices");
        viewer_->makeCurrent();
        auto points = cloud()->get_vertex_property<vec3>("v:point");
        vertices->update_vertex_buffer(points.vector());
        auto normals = cloud()->get_vertex_property<vec3>("v:normal");
        if (normals)
            vertices->update_normal_buffer(normals.vector());
        auto colors = cloud()->get_vertex_property<vec3>("v:color");
        if (colors)
            vertices->update_color_buffer(colors.vector());

        vertices->set_per_vertex_color(setting::point_cloud_use_color_property);
        vertices->set_visible(setting::point_cloud_show_points);
        vertices->set_default_color(setting::point_cloud_points_color);
//        vertices->set_impostors(setting::point_cloud_impostors);
        vertices->set_point_size(setting::point_cloud_point_size);

        viewer_->doneCurrent();
    }

    if (vertices) {
        vertices->set_visible(b);
        viewer_->update();
    }
}


void WidgetPointCloudRenderer::setVerticesDefaultColor() {
    PointsDrawable* vertices = cloud()->points_drawable("vertices");
    if (vertices) {
        const vec3& c = vertices->default_color();
        QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
        const QColor& color = QColorDialog::getColor(orig, this);
        if (color.isValid()) {
            const vec3 new_color(color.redF(), color.greenF(), color.blueF());
            vertices->set_default_color(new_color);
            viewer_->update();
            QPixmap pixmap(ui->toolButtonVerticesDefaultColor->size());
            pixmap.fill(color);
            ui->toolButtonVerticesDefaultColor->setIcon(QIcon(pixmap));
        }
    }
}


void WidgetPointCloudRenderer::setVerticesImpostors(bool b) {
    PointsDrawable* vertices = cloud()->points_drawable("vertices");
    if (vertices) {
//        vertices->set_impostors(b);
        viewer_->update();
    }
}


void WidgetPointCloudRenderer::setVerticesSize(double s){
    PointsDrawable* vertices = cloud()->points_drawable("vertices");
    if (vertices) {
        vertices->set_point_size(s);
        viewer_->update();
    }
}
