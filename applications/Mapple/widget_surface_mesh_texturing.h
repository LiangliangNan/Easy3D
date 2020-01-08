#ifndef WIDGET_SURFACE_MESH_TEXTURING_H
#define WIDGET_SURFACE_MESH_TEXTURING_H

#include <QWidget>


namespace Ui {
    class WidgetSurfaceMeshTexturing;
}

namespace easy3d {
    class SurfaceMesh;
}

class PaintCanvas;

class WidgetSurfaceMeshTexturing : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetSurfaceMeshTexturing(QWidget *parent);
    ~WidgetSurfaceMeshTexturing();

    // update the panel to be consistent with the drawable's rendering parameters
    void updatePanel();
    //make sure the appropriate rendering data are uploaded to GPU
    void ensureBuffers();

public slots:

private:
    easy3d::SurfaceMesh* mesh();

private:
    Ui::WidgetSurfaceMeshTexturing* ui;

    PaintCanvas* viewer_;
};

#endif // WIDGET_SURFACE_MESH_TEXTURING_H
