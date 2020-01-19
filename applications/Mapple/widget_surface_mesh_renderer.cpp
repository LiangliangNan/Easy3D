#include "widget_surface_mesh_renderer.h"

#include <QColorDialog>

#include "ui_widget_surface_mesh_renderer.h"

#include "main_window.h"
#include "paint_canvas.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/types.h>
#include <easy3d/util/logging.h>
#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/viewer/renderer.h>


using namespace easy3d;


WidgetSurfaceMeshRenderer::WidgetSurfaceMeshRenderer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WidgetSurfaceMeshRenderer)
{
    ui->setupUi(this);

    viewer_ = dynamic_cast<MainWindow*>(parent)->viewer();

    // phong shading
    connect(ui->checkBoxPhongShading, SIGNAL(toggled(bool)), this, SLOT(setPhongShading(bool)));

    // faces
    connect(ui->checkBoxUseColorProperty, SIGNAL(toggled(bool)), this, SLOT(setUseColorProperty(bool)));
    connect(ui->checkBoxShowFaces, SIGNAL(toggled(bool)), this, SLOT(setShowFaces(bool)));
    connect(ui->toolButtonFacesDefaultColor, SIGNAL(clicked()), this, SLOT(setFacesDefaultColor()));
    connect(ui->horizontalSliderFacesOpacity, SIGNAL(valueChanged(int)), this, SLOT(setFacesOpacity(int)));

    // vertices
    connect(ui->checkBoxShowVertices, SIGNAL(toggled(bool)), this, SLOT(setShowVertices(bool)));
    connect(ui->toolButtonVerticesDefaultColor, SIGNAL(clicked()), this, SLOT(setVerticesDefaultColor()));
    connect(ui->toolButtonVerticesImpostors, SIGNAL(toggled(bool)), this, SLOT(setVerticesImpostors(bool)));
    connect(ui->doubleSpinBoxVerticesSize, SIGNAL(valueChanged(double)), this, SLOT(setVerticesSize(double)));

    // edges
    connect(ui->checkBoxShowEdges, SIGNAL(toggled(bool)), this, SLOT(setShowEdges(bool)));
    connect(ui->toolButtonEdgesDefaultColor, SIGNAL(clicked()), this, SLOT(setEdgesDefaultColor()));
    connect(ui->toolButtonEdgesImpostors, SIGNAL(toggled(bool)), this, SLOT(setEdgesImpostors(bool)));
    connect(ui->doubleSpinBoxEdgesThickness, SIGNAL(valueChanged(double)), this, SLOT(setEdgesThickness(double)));

    // border
    connect(ui->checkBoxShowBorders, SIGNAL(toggled(bool)), this, SLOT(setShowBorders(bool)));
    connect(ui->toolButtonBordersDefaultColor, SIGNAL(clicked()), this, SLOT(setBordersDefaultColor()));
    connect(ui->toolButtonBordersImpostors, SIGNAL(toggled(bool)), this, SLOT(setBordersImpostors(bool)));
    connect(ui->doubleSpinBoxBordersThickness, SIGNAL(valueChanged(double)), this, SLOT(setBordersThickness(double)));


}


WidgetSurfaceMeshRenderer::~WidgetSurfaceMeshRenderer()
{
    delete ui;
}


SurfaceMesh* WidgetSurfaceMeshRenderer::mesh() {
    return dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
}


// update the panel to be consistent with the drawable's rendering parameters
void WidgetSurfaceMeshRenderer::updatePanel() {
    if (!mesh())
        return;

    // surface
    TrianglesDrawable* drawable = mesh()->triangles_drawable("faces");
    if (drawable) {
        ui->checkBoxPhongShading->setChecked(drawable->smooth_shading());
        ui->checkBoxUseColorProperty->setChecked(drawable->per_vertex_color());
        ui->checkBoxShowFaces->setChecked(drawable->is_visible());
        const vec3& c = drawable->default_color();
        QPixmap pixmap(ui->toolButtonFacesDefaultColor->size());
        pixmap.fill(QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonFacesDefaultColor->setIcon(QIcon(pixmap));
        ui->horizontalSliderFacesOpacity->setValue(drawable->opacity() * 100);
    }
    else {
        ui->checkBoxPhongShading->setChecked(setting::surface_mesh_phong_shading);
        ui->checkBoxUseColorProperty->setChecked(setting::surface_mesh_use_color_property);
        ui->checkBoxShowFaces->setChecked(setting::surface_mesh_show_faces);
        const vec3& c = setting::surface_mesh_faces_color;
        QPixmap pixmap(ui->toolButtonFacesDefaultColor->size());
        pixmap.fill(QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonFacesDefaultColor->setIcon(QIcon(pixmap));
        ui->horizontalSliderFacesOpacity->setValue(setting::surface_mesh_opacity * 100);
    }

    // vertices
    PointsDrawable* vertices = mesh()->points_drawable("vertices");
    if (vertices) {
        ui->checkBoxShowVertices->setChecked(vertices->is_visible());
        const vec3& c = vertices->default_color();
        QPixmap pixmap(ui->toolButtonVerticesDefaultColor->size());
        pixmap.fill(QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonVerticesDefaultColor->setIcon(QIcon(pixmap));
        ui->toolButtonVerticesImpostors->setChecked(vertices->impostor_type() != PointsDrawable::PLAIN);
        ui->doubleSpinBoxVerticesSize->setValue(vertices->point_size());
    }
    else {
        ui->checkBoxShowVertices->setChecked(setting::surface_mesh_show_vertices);
        const vec3& c = setting::surface_mesh_vertices_color;
        QPixmap pixmap(ui->toolButtonVerticesDefaultColor->size());
        pixmap.fill(QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonVerticesDefaultColor->setIcon(QIcon(pixmap));
        ui->toolButtonVerticesImpostors->setChecked(setting::surface_mesh_vertices_imposters);
        ui->doubleSpinBoxVerticesSize->setValue(setting::surface_mesh_vertices_point_size);
    }

    // edges
    LinesDrawable* edges = mesh()->lines_drawable("edges");
    if (edges) {
        ui->checkBoxShowEdges->setChecked(edges->is_visible());
        const vec3& c = edges->default_color();
        QPixmap pixmap(ui->toolButtonEdgesDefaultColor->size());
        pixmap.fill(QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonEdgesDefaultColor->setIcon(QIcon(pixmap));
        ui->toolButtonEdgesImpostors->setChecked(edges->impostor_type());
        ui->doubleSpinBoxEdgesThickness->setValue(edges->line_width());
    }
    else {
        ui->checkBoxShowEdges->setChecked(setting::surface_mesh_show_edges);
        const vec3& c = setting::surface_mesh_edges_color;
        QPixmap pixmap(ui->toolButtonEdgesDefaultColor->size());
        pixmap.fill(QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonEdgesDefaultColor->setIcon(QIcon(pixmap));
        ui->toolButtonEdgesImpostors->setChecked(setting::surface_mesh_edges_imposters);
        ui->doubleSpinBoxEdgesThickness->setValue(setting::surface_mesh_edges_line_width);
    }

    // border
    LinesDrawable* borders = mesh()->lines_drawable("borders");
    if (borders) {
        ui->checkBoxShowBorders->setChecked(borders->is_visible());
        const vec3& c = borders->default_color();
        QPixmap pixmap(ui->toolButtonBordersDefaultColor->size());
        pixmap.fill(QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonBordersDefaultColor->setIcon(QIcon(pixmap));
        ui->toolButtonBordersImpostors->setChecked(borders->impostor_type());
        ui->doubleSpinBoxBordersThickness->setValue(borders->line_width());
    }
    else {
        ui->checkBoxShowBorders->setChecked(setting::surface_mesh_show_borders);
        const vec3& c = setting::surface_mesh_borders_color;
        QPixmap pixmap(ui->toolButtonBordersDefaultColor->size());
        pixmap.fill(QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonBordersDefaultColor->setIcon(QIcon(pixmap));
        ui->toolButtonBordersImpostors->setChecked(setting::surface_mesh_borders_imposters);
        ui->doubleSpinBoxBordersThickness->setValue(setting::surface_mesh_borders_line_width);
    }

    // face labels
    ui->checkBoxShowFaceLabels->setChecked(false);
    // vertex labels
    ui->checkBoxShowVertexLabels->setChecked(false);

    // scalar field
    // collect all available scalar field
    //...


 }


//make sure the appropriate rendering data are uploaded to GPU
void WidgetSurfaceMeshRenderer::ensureBuffers() {
    setPhongShading(ui->checkBoxPhongShading->isChecked());
}


void WidgetSurfaceMeshRenderer::setPhongShading(bool b) {
    if (!mesh())
        return;

    TrianglesDrawable* drawable = mesh()->triangles_drawable("faces");
    if (drawable && drawable->smooth_shading() == b)
        return;
    else if (!drawable) {  // make sure the drawable exists
        drawable = mesh()->add_triangles_drawable("faces");
        renderer::update_data(mesh(), drawable);
    }

    drawable->set_smooth_shading(b);
    viewer_->update();
}


void WidgetSurfaceMeshRenderer::setUseColorProperty(bool b) {
    if (b) { // just check and warn user if color not available
        auto vcolors = mesh()->get_vertex_property<vec3>("v:color");
        auto fcolors = mesh()->get_face_property<vec3>("f:color");
        if (!vcolors && !fcolors)
            LOG(WARNING) << "no color property defined on vertices/faces";
    }

    TrianglesDrawable* drawable = mesh()->triangles_drawable("faces");
    if (drawable) {
        drawable->set_per_vertex_color(b);
        viewer_->update();
    }
}


void WidgetSurfaceMeshRenderer::setShowFaces(bool b) {
    TrianglesDrawable* drawable = mesh()->triangles_drawable("faces");
    if (drawable) {
        drawable->set_visible(b);
        viewer_->update();
    }
}


void WidgetSurfaceMeshRenderer::setFacesDefaultColor() {
    TrianglesDrawable* drawable = mesh()->triangles_drawable("faces");
    if (drawable) {
        const vec3& c = drawable->default_color();
        QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
        const QColor& color = QColorDialog::getColor(orig, this);
        if (color.isValid()) {
            const vec3 new_color(color.redF(), color.greenF(), color.blueF());
            drawable->set_default_color(new_color);
            viewer_->update();
            QPixmap pixmap(ui->toolButtonFacesDefaultColor->size());
            pixmap.fill(color);
            ui->toolButtonFacesDefaultColor->setIcon(QIcon(pixmap));
        }
    }
}


void WidgetSurfaceMeshRenderer::setFacesOpacity(int a) {
    TrianglesDrawable* drawable = mesh()->triangles_drawable("faces");
    if (drawable) {
        drawable->set_opacity(a / 100.0f);
        viewer_->update();
    }
}


// vertices
void WidgetSurfaceMeshRenderer::setShowVertices(bool b) {
    PointsDrawable* vertices = mesh()->points_drawable("vertices");
    if (b && !vertices) {
        vertices = mesh()->add_points_drawable("vertices");
        viewer_->makeCurrent();
        auto points = mesh()->get_vertex_property<vec3>("v:point");
        vertices->update_vertex_buffer(points.vector());
        viewer_->doneCurrent();
    }

    if (vertices) {
        vertices->set_visible(b);
        viewer_->update();
    }
}


void WidgetSurfaceMeshRenderer::setVerticesDefaultColor() {
    PointsDrawable* vertices = mesh()->points_drawable("vertices");
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


void WidgetSurfaceMeshRenderer::setVerticesImpostors(bool b) {
    PointsDrawable* vertices = mesh()->points_drawable("vertices");
    if (vertices) {
//        vertices->set_impostors(b);
        viewer_->update();
    }
}


void WidgetSurfaceMeshRenderer::setVerticesSize(double s){
    PointsDrawable* vertices = mesh()->points_drawable("vertices");
    if (vertices) {
        vertices->set_point_size(s);
        viewer_->update();
    }
}


// edges
void WidgetSurfaceMeshRenderer::setShowEdges(bool b) {
    LinesDrawable* edges = mesh()->lines_drawable("edges");
    if (b && !edges) {
        edges = mesh()->add_lines_drawable("edges");
        viewer_->makeCurrent();
        auto points = mesh()->get_vertex_property<vec3>("v:point");
        std::vector<vec3> vertices;
        for (auto e : mesh()->edges()) {
            SurfaceMesh::Vertex s = mesh()->vertex(e, 0);
            SurfaceMesh::Vertex t = mesh()->vertex(e, 1);
            vertices.push_back(points[s]);
            vertices.push_back(points[t]);
        }
        edges->update_vertex_buffer(vertices);
        viewer_->doneCurrent();
    }

    if (edges) {
        edges->set_visible(b);
        viewer_->update();
    }
}


void WidgetSurfaceMeshRenderer::setEdgesDefaultColor() {
    LinesDrawable* edges = mesh()->lines_drawable("edges");
    if (edges) {
        const vec3& c = edges->default_color();
        QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
        const QColor& color = QColorDialog::getColor(orig, this);
        if (color.isValid()) {
            const vec3 new_color(color.redF(), color.greenF(), color.blueF());
            edges->set_default_color(new_color);
            viewer_->update();
            QPixmap pixmap(ui->toolButtonEdgesDefaultColor->size());
            pixmap.fill(color);
            ui->toolButtonEdgesDefaultColor->setIcon(QIcon(pixmap));
        }
    }
}


void WidgetSurfaceMeshRenderer::setEdgesImpostors(bool b) {
    LinesDrawable* edges = mesh()->lines_drawable("edges");
    if (edges) {
        edges->set_impostor_type(LinesDrawable::CYLINDER);
        viewer_->update();
    }
}


void WidgetSurfaceMeshRenderer::setEdgesThickness(double s){
    LinesDrawable* edges = mesh()->lines_drawable("edges");
    if (edges) {
        edges->set_line_width(s);
        viewer_->update();
    }
}


// borders
void WidgetSurfaceMeshRenderer::setShowBorders(bool b) {
    LinesDrawable* borders = mesh()->lines_drawable("borders");
    if (b && !borders) {
        borders = mesh()->add_lines_drawable("borders");
        viewer_->makeCurrent();
        auto points = mesh()->get_vertex_property<vec3>("v:point");
        std::vector<vec3> vertices;
        for (auto e : mesh()->edges()) {
            if (mesh()->is_boundary(e)) {
                SurfaceMesh::Vertex s = mesh()->vertex(e, 0);
                SurfaceMesh::Vertex t = mesh()->vertex(e, 1);
                vertices.push_back(points[s]);
                vertices.push_back(points[t]);
            }
        }
        borders->update_vertex_buffer(vertices);
        viewer_->doneCurrent();
    }

    if (borders) {
        borders->set_visible(b);
        viewer_->update();
    }
}


void WidgetSurfaceMeshRenderer::setBordersDefaultColor() {
    LinesDrawable* borders = mesh()->lines_drawable("borders");
    if (borders) {
        const vec3& c = borders->default_color();
        QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
        const QColor& color = QColorDialog::getColor(orig, this);
        if (color.isValid()) {
            const vec3 new_color(color.redF(), color.greenF(), color.blueF());
            borders->set_default_color(new_color);
            viewer_->update();
            QPixmap pixmap(ui->toolButtonBordersDefaultColor->size());
            pixmap.fill(color);
            ui->toolButtonBordersDefaultColor->setIcon(QIcon(pixmap));
        }
    }
}


void WidgetSurfaceMeshRenderer::setBordersImpostors(bool b) {
    LinesDrawable* borders = mesh()->lines_drawable("borders");
    if (borders) {
        borders->set_impostor_type(LinesDrawable::CYLINDER);
        viewer_->update();
    }
}


void WidgetSurfaceMeshRenderer::setBordersThickness(double s){
    LinesDrawable* borders = mesh()->lines_drawable("borders");
    if (borders) {
        borders->set_line_width(s);
        viewer_->update();
    }
}
