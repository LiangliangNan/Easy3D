#ifndef WIDGET_DRAWABLE_H
#define WIDGET_DRAWABLE_H

#include <QWidget>

class PaintCanvas;

class WidgetDrawable : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetDrawable(QWidget *parent);
    ~WidgetDrawable();

    virtual void updatePanel() = 0;

protected:
    PaintCanvas* viewer_;
};

#endif // WIDGET_DRAWABLE_H
