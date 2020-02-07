#ifndef WIDGET_DRAWABLE_TRIANGLES_H
#define WIDGET_DRAWABLE_TRIANGLES_H

#include <unordered_map>
#include <QWidget>

#include "widgets/widget_drawable.h"

namespace Ui {
    class WidgetTrianglesDrawable;
}

namespace easy3d {
    class Model;
    class TrianglesDrawable;
}


class WidgetTrianglesDrawable : public WidgetDrawable
{
    Q_OBJECT

public:
    explicit WidgetTrianglesDrawable(QWidget *parent);
    ~WidgetTrianglesDrawable() override;

    // update the panel to be consistent with the drawable's rendering parameters
    void updatePanel() override;

    easy3d::TrianglesDrawable* drawable();

public slots:
    void setDrawableVisible(bool);
    void setActiveDrawable(const QString &);
    void setPhongShading(bool);
    void setLighting(const QString &);
    void setDistinctBackColor(bool);
    void setColorScheme(const QString &);
    void setDefaultColor();
    void setBackColor();
    void setTextureFile();
    void setTextureRepeat(int);
    void setTextureFractionalRepeat(int);
    void setHighlight(bool);
    void setHighlightLow(int);
    void setHighlightHigh(int);
    void setOpacity(int);

private:
    void connectAll();
    void disconnectAll();
    void disableUnnecessaryWidgets();

private:
    Ui::WidgetTrianglesDrawable*  ui;

    // on the rendering panel, the rendering of only the selected drawable can be changed.
    // this is used to keep the history so the user can switch between different models.
    std::unordered_map<easy3d::Model*, std::string> active_triangles_drawable_;
};

#endif // WIDGET_DRAWABLE_TRIANGLES_H
