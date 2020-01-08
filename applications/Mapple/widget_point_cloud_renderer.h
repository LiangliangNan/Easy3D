#ifndef WIDGET_POINT_CLOUD_RENDERER_H
#define WIDGET_POINT_CLOUD_RENDERER_H

#include <QWidget>


namespace Ui {
    class WidgetPointCloudRenderer;
}

namespace easy3d {
    class PointCloud;
}

class PaintCanvas;

class WidgetPointCloudRenderer : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetPointCloudRenderer(QWidget *parent);
    ~WidgetPointCloudRenderer();

    // update the panel to be consistent with the drawable's rendering parameters
    void updatePanel();
    //make sure the appropriate rendering data are uploaded to GPU
    void ensureBuffers();

public slots:
    // vertices
    void setUseColorProperty(bool);
    void setShowVertices(bool);
    void setVerticesDefaultColor();
    void setVerticesImpostors(bool);
    void setVerticesSize(double);

private:
    easy3d::PointCloud* cloud();

private:
    Ui::WidgetPointCloudRenderer* ui;

    PaintCanvas* viewer_;
};

#endif // WIDGET_POINT_CLOUD_RENDERER_H
