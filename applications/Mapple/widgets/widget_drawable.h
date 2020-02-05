#ifndef WIDGET_DRAWABLE_H
#define WIDGET_DRAWABLE_H

#include <QWidget>


namespace easy3d {
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
    virtual void setUseColorProperty(bool) = 0;
    virtual void setHighlight(bool) = 0;

    virtual void setLighting(bool);
    virtual void setDefaultColor();

    virtual void setDrawableVisible(bool);
    virtual void setHighlightLow(int);
    virtual void setHighlightHigh(int);
    virtual void setLightingTwoSides(int);

    virtual void setUseTexture(bool);

    virtual void setTextureFile();
    virtual void setTextureRepeat(int);
    virtual void setTextureFractionalRepeat(int);

protected:
    PaintCanvas* viewer_;
};

#endif // WIDGET_DRAWABLE_H
