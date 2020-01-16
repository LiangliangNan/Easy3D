#include "widget_surface_mesh_texturing.h"

#include <QColorDialog>

#include "ui_widget_surface_mesh_texturing.h"

#include "main_window.h"
#include "paint_canvas.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/types.h>
#include <easy3d/viewer/drawable.h>
#include <easy3d/viewer/setting.h>


using namespace easy3d;


WidgetSurfaceMeshTexturing::WidgetSurfaceMeshTexturing(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WidgetSurfaceMeshTexturing)
{
    ui->setupUi(this);

    viewer_ = dynamic_cast<MainWindow*>(parent)->viewer();
}


WidgetSurfaceMeshTexturing::~WidgetSurfaceMeshTexturing()
{
    delete ui;
}


SurfaceMesh* WidgetSurfaceMeshTexturing::mesh() {
    return dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
}


// update the panel to be consistent with the drawable's rendering parameters
void WidgetSurfaceMeshTexturing::updatePanel() {
    if (!mesh())
        return;

    // faces
    TrianglesDrawable* drawable = mesh()->triangles_drawable("faces");
    if (drawable) {
    }
    else {
    }
 }


//make sure the appropriate rendering data are uploaded to GPU
void WidgetSurfaceMeshTexturing::ensureBuffers() {
 }
