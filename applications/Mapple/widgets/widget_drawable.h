#ifndef WIDGET_DRAWABLE_H
#define WIDGET_DRAWABLE_H

#include <QWidget>
#include <unordered_map>

namespace easy3d {
    class Model;
    class Drawable;
}

class PaintCanvas;

class WidgetDrawable : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetDrawable(QWidget *parent);
    ~WidgetDrawable();

    virtual void updatePanel() = 0;

    // the current drawable being manipulated
    virtual easy3d::Drawable* drawable() = 0;

public slots:
    virtual void setActiveDrawable(const QString &) = 0;

    void setDrawableVisible(bool);
    void setColorScheme(const QString &);
    void setLighting(bool);
    void setLightingTwoSides(int);

    virtual void setDefaultColor();

    void setDistinctBackColor(bool);
    virtual void setBackColor();

    void setUseTexture(bool);
    virtual void setTextureFile();
    void setTextureRepeat(int);
    void setTextureFractionalRepeat(int);

    virtual void setHighlight(bool) = 0;
    virtual void setHighlightLow(int);
    virtual void setHighlightHigh(int);

protected:
    PaintCanvas* viewer_;

    // on the rendering panel, the rendering of only the selected drawable can be changed.
    // this is used to keep the history so the user can switch between different models.
    std::unordered_map<easy3d::Model*, std::string> active_points_drawable_;
    std::unordered_map<easy3d::Model*, std::string> active_lines_drawable_;
    std::unordered_map<easy3d::Model*, std::string> active_triangles_drawable_;
};

#endif // WIDGET_DRAWABLE_H
