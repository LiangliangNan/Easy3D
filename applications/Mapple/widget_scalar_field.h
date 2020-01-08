#ifndef WIDGET_SCALAR_FIELD_H
#define WIDGET_SCALAR_FIELD_H

#include <QWidget>


namespace Ui {
    class WidgetScalarField;
}

namespace easy3d {
    class SurfaceMesh;
    class PointCloud;
}

class PaintCanvas;

class WidgetScalarField : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetScalarField(QWidget *parent);
    ~WidgetScalarField();

    // update the panel to be consistent with the drawable's rendering parameters
    void updatePanel();
    //make sure the appropriate rendering data are uploaded to GPU
    void ensureBuffers();

private slots:
    void setScalarField(const QString&);
    void setScalarColormapStyle(const QString&);
    void setScalarEdgeStyle(const QString&);

private:
    easy3d::SurfaceMesh* mesh();
    easy3d::PointCloud*  cloud();

private:
    Ui::WidgetScalarField* ui;

    PaintCanvas* viewer_;
};

#endif // WIDGET_SCALAR_FIELD_H
