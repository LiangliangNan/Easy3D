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

    // wireframe
    connect(ui->checkBoxShowWireframe, SIGNAL(toggled(bool)), this, SLOT(setShowWireframe(bool)));
    connect(ui->toolButtonWireframeDefaultColor, SIGNAL(clicked()), this, SLOT(setWireframeDefaultColor()));
    connect(ui->toolButtonWireframeImpostors, SIGNAL(toggled(bool)), this, SLOT(setWireframeImpostors(bool)));
    connect(ui->doubleSpinBoxWireframeThickness, SIGNAL(valueChanged(double)), this, SLOT(setWireframeThickness(double)));

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
    TrianglesDrawable* surface = mesh()->triangles_drawable("surface");
    if (surface) {
        ui->checkBoxPhongShading->setChecked(surface->phong_shading());
        ui->checkBoxUseColorProperty->setChecked(surface->per_vertex_color());
        ui->checkBoxShowFaces->setChecked(surface->is_visible());
        const vec3& c = surface->default_color();
        QPixmap pixmap(ui->toolButtonFacesDefaultColor->size());
        pixmap.fill(QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonFacesDefaultColor->setIcon(QIcon(pixmap));
        ui->horizontalSliderFacesOpacity->setValue(surface->opacity() * 100);
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
        ui->toolButtonVerticesImpostors->setChecked(vertices->impostors());
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

    // wireframe
    LinesDrawable* wireframe = mesh()->lines_drawable("wireframe");
    if (wireframe) {
        ui->checkBoxShowWireframe->setChecked(wireframe->is_visible());
        const vec3& c = wireframe->default_color();
        QPixmap pixmap(ui->toolButtonWireframeDefaultColor->size());
        pixmap.fill(QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonWireframeDefaultColor->setIcon(QIcon(pixmap));
        ui->toolButtonWireframeImpostors->setChecked(wireframe->impostor_type());
        ui->doubleSpinBoxWireframeThickness->setValue(wireframe->line_width());
    }
    else {
        ui->checkBoxShowWireframe->setChecked(setting::surface_mesh_show_wireframe);
        const vec3& c = setting::surface_mesh_wireframe_color;
        QPixmap pixmap(ui->toolButtonWireframeDefaultColor->size());
        pixmap.fill(QColor(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255)));
        ui->toolButtonWireframeDefaultColor->setIcon(QIcon(pixmap));
        ui->toolButtonWireframeImpostors->setChecked(setting::surface_mesh_wireframe_imposters);
        ui->doubleSpinBoxWireframeThickness->setValue(setting::surface_mesh_wireframe_line_width);
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

    TrianglesDrawable* surface = mesh()->triangles_drawable("surface");
    if (surface && surface->phong_shading() == b)
        return;
    else if (!surface)  // make sure the drawable exists
        surface = mesh()->add_triangles_drawable("surface");

    std::cout << "executed" << std::endl;

    viewer_->makeCurrent();
    if (b) {
        auto points = mesh()->get_vertex_property<vec3>("v:point");
        surface->update_vertex_buffer(points.vector());
        auto colors = mesh()->get_vertex_property<vec3>("v:color");
        if (colors)
            surface->update_color_buffer(colors.vector());

        auto normals = mesh()->get_vertex_property<vec3>("v:normal");
        if (normals)
             surface->update_normal_buffer(normals.vector());
        else {
            std::vector<vec3> normals;
            normals.reserve(mesh()->n_vertices());
            for (auto v : mesh()->vertices()) {
                const vec3& n = mesh()->compute_vertex_normal(v);
                normals.push_back(n);
            }
            surface->update_normal_buffer(normals);
        }

        std::vector<unsigned int> indices;
        for (auto f : mesh()->faces()) {
            // we assume convex polygonal faces and we render in triangles
            SurfaceMesh::Halfedge start = mesh()->halfedge(f);
            SurfaceMesh::Halfedge cur = mesh()->next_halfedge(mesh()->next_halfedge(start));
            SurfaceMesh::Vertex va = mesh()->to_vertex(start);
            while (cur != start) {
                SurfaceMesh::Vertex vb = mesh()->from_vertex(cur);
                SurfaceMesh::Vertex vc = mesh()->to_vertex(cur);
                indices.push_back(static_cast<unsigned int>(va.idx()));
                indices.push_back(static_cast<unsigned int>(vb.idx()));
                indices.push_back(static_cast<unsigned int>(vc.idx()));
                cur = mesh()->next_halfedge(cur);
            }
        }
        surface->update_index_buffer(indices);
    }
    else {  // flat shading
        auto points = mesh()->get_vertex_property<vec3>("v:point");
        auto colors = mesh()->get_vertex_property<vec3>("v:color");

        std::vector<vec3> vertices, vertex_normals, vertex_colors;
        for (auto f : mesh()->faces()) {
            // we assume convex polygonal faces and we render in triangles
            SurfaceMesh::Halfedge start = mesh()->halfedge(f);
            SurfaceMesh::Halfedge cur = mesh()->next_halfedge(mesh()->next_halfedge(start));
            SurfaceMesh::Vertex va = mesh()->to_vertex(start);
            const vec3& pa = points[va];
            while (cur != start) {
                SurfaceMesh::Vertex vb = mesh()->from_vertex(cur);
                SurfaceMesh::Vertex vc = mesh()->to_vertex(cur);
                const vec3& pb = points[vb];
                const vec3& pc = points[vc];
                vertices.push_back(pa);
                vertices.push_back(pb);
                vertices.push_back(pc);

                const vec3& n = geom::triangle_normal(pa, pb, pc);
                vertex_normals.insert(vertex_normals.end(), 3, n);

                if (colors) {
                    vertex_colors.push_back(colors[va]);
                    vertex_colors.push_back(colors[vb]);
                    vertex_colors.push_back(colors[vc]);
                }
                cur = mesh()->next_halfedge(cur);
            }
        }
        surface->update_vertex_buffer(vertices);
        surface->update_normal_buffer(vertex_normals);
        if (colors)
            surface->update_color_buffer(vertex_colors);
        surface->release_index_buffer();
    }

    surface->set_phong_shading(b);

    viewer_->update();
    viewer_->doneCurrent();
}


void WidgetSurfaceMeshRenderer::setUseColorProperty(bool b) {
    if (b) { // just check and warn user if color not available
        auto vcolors = mesh()->get_vertex_property<vec3>("v:color");
        auto fcolors = mesh()->get_face_property<vec3>("f:color");
        if (!vcolors && !fcolors)
            LOG(WARNING) << "no color property defined on vertices/faces";
    }

    TrianglesDrawable* surface = mesh()->triangles_drawable("surface");
    if (surface) {
        surface->set_per_vertex_color(b);
        viewer_->update();
    }
}


void WidgetSurfaceMeshRenderer::setShowFaces(bool b) {
    TrianglesDrawable* surface = mesh()->triangles_drawable("surface");
    if (surface) {
        surface->set_visible(b);
        viewer_->update();
    }
}


void WidgetSurfaceMeshRenderer::setFacesDefaultColor() {
    TrianglesDrawable* surface = mesh()->triangles_drawable("surface");
    if (surface) {
        const vec3& c = surface->default_color();
        QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
        const QColor& color = QColorDialog::getColor(orig, this);
        if (color.isValid()) {
            const vec3 new_color(color.redF(), color.greenF(), color.blueF());
            surface->set_default_color(new_color);
            viewer_->update();
            QPixmap pixmap(ui->toolButtonFacesDefaultColor->size());
            pixmap.fill(color);
            ui->toolButtonFacesDefaultColor->setIcon(QIcon(pixmap));
        }
    }
}


void WidgetSurfaceMeshRenderer::setFacesOpacity(int a) {
    TrianglesDrawable* surface = mesh()->triangles_drawable("surface");
    if (surface) {
        surface->set_opacity(a / 100.0f);
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
        vertices->set_impostors(b);
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


// wireframe
void WidgetSurfaceMeshRenderer::setShowWireframe(bool b) {
    LinesDrawable* wireframe = mesh()->lines_drawable("wireframe");
    if (b && !wireframe) {
        wireframe = mesh()->add_lines_drawable("wireframe");
        viewer_->makeCurrent();
        auto points = mesh()->get_vertex_property<vec3>("v:point");
        std::vector<vec3> vertices;
        for (auto e : mesh()->edges()) {
            SurfaceMesh::Vertex s = mesh()->vertex(e, 0);
            SurfaceMesh::Vertex t = mesh()->vertex(e, 1);
            vertices.push_back(points[s]);
            vertices.push_back(points[t]);
        }
        wireframe->update_vertex_buffer(vertices);
        viewer_->doneCurrent();
    }

    if (wireframe) {
        wireframe->set_visible(b);
        viewer_->update();
    }
}


void WidgetSurfaceMeshRenderer::setWireframeDefaultColor() {
    LinesDrawable* wireframe = mesh()->lines_drawable("wireframe");
    if (wireframe) {
        const vec3& c = wireframe->default_color();
        QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255));
        const QColor& color = QColorDialog::getColor(orig, this);
        if (color.isValid()) {
            const vec3 new_color(color.redF(), color.greenF(), color.blueF());
            wireframe->set_default_color(new_color);
            viewer_->update();
            QPixmap pixmap(ui->toolButtonWireframeDefaultColor->size());
            pixmap.fill(color);
            ui->toolButtonWireframeDefaultColor->setIcon(QIcon(pixmap));
        }
    }
}


void WidgetSurfaceMeshRenderer::setWireframeImpostors(bool b) {
    LinesDrawable* wireframe = mesh()->lines_drawable("wireframe");
    if (wireframe) {
        wireframe->set_impostor_type(IT_CYLINDERS);
        viewer_->update();
    }
}


void WidgetSurfaceMeshRenderer::setWireframeThickness(double s){
    LinesDrawable* wireframe = mesh()->lines_drawable("wireframe");
    if (wireframe) {
        wireframe->set_line_width(s);
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
        borders->set_impostor_type(IT_CYLINDERS);
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
