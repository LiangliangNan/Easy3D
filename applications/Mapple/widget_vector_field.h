#ifndef WIDGET_VECTOR_FIELD_H
#define WIDGET_VECTOR_FIELD_H

#include <QWidget>


namespace Ui {
    class WidgetVectorField;
}

namespace easy3d {
    class SurfaceMesh;
    class PointCloud;
}

class PaintCanvas;

class WidgetVectorField : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetVectorField(QWidget *parent);
    ~WidgetVectorField();

    // update the panel to be consistent with the drawable's rendering parameters
    void updatePanel();
    //make sure the appropriate rendering data are uploaded to GPU
    void ensureBuffers();

private slots:
    void setVectorField(const QString& field);

private:
    easy3d::SurfaceMesh* mesh();
    easy3d::PointCloud*  cloud();

private:
    Ui::WidgetVectorField* ui;

    PaintCanvas* viewer_;
};

#endif // WIDGET_VECTOR_FIELD_H
