#ifndef WIDGET_SURFACE_MESH_RENDERER_H
#define WIDGET_SURFACE_MESH_RENDERER_H

#include <QWidget>


namespace Ui {
    class WidgetSurfaceMeshRenderer;
}

namespace easy3d {
    class SurfaceMesh;
}

class PaintCanvas;

class WidgetSurfaceMeshRenderer : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetSurfaceMeshRenderer(QWidget *parent);
    ~WidgetSurfaceMeshRenderer();

    // update the panel to be consistent with the drawable's rendering parameters
    void updatePanel();
    //make sure the appropriate rendering data are uploaded to GPU
    void ensureBuffers();

public slots:
    // phong shading
    void setPhongShading(bool);

    // faces
    void setShowFaces(bool);
    void setFacesDefaultColor();
    void setUseColorProperty(bool);
    void setFacesOpacity(int);

    // vertices
    void setShowVertices(bool);
    void setVerticesDefaultColor();
    void setVerticesImpostors(bool);
    void setVerticesSize(double);

    // wireframe
    void setShowWireframe(bool);
    void setWireframeDefaultColor();
    void setWireframeImpostors(bool);
    void setWireframeThickness(double);

    // border
    void setShowBorders(bool);
    void setBordersDefaultColor();
    void setBordersImpostors(bool);
    void setBordersThickness(double);

private:
    easy3d::SurfaceMesh* mesh();

private:
    Ui::WidgetSurfaceMeshRenderer* ui;

    PaintCanvas* viewer_;
};

#endif // WIDGET_SURFACE_MESH_RENDERER_H
