#ifndef WIDGET_DRAWABLE_LINES_H
#define WIDGET_DRAWABLE_LINES_H

#include <unordered_map>
#include <QWidget>

#include "widgets/widget_drawable.h"

namespace Ui {
    class WidgetLinesDrawable;
}

namespace easy3d {
    class LinesDrawable;
}


class WidgetLinesDrawable : public WidgetDrawable
{
    Q_OBJECT

public:
    explicit WidgetLinesDrawable(QWidget *parent);
    ~WidgetLinesDrawable() override;

    // update the panel to be consistent with the drawable's rendering parameters
    void updatePanel() override;

    easy3d::LinesDrawable* drawable();

public slots:
    void setDrawableVisible(bool);
    void setActiveDrawable(const QString &);
    void setLineWidth(double);
    void setImposterStyle(const QString &);
    void setColorScheme(const QString &);
    void setDefaultColor();
    void setHighlight(bool);
    void setHighlightMin(int);
    void setHighlightMax(int);

    void setScalarFieldStyle(const QString &);

private:
    void connectAll();
    void disconnectAll();
    void disableUnavailableOptions();

    // model depended stuff
    std::vector<std::string> colorSchemes(const easy3d::Model* model);
    std::vector<std::string> vectorFields(const easy3d::Model* model);

private:
    Ui::WidgetLinesDrawable*  ui;
};

#endif // WIDGET_DRAWABLE_LINES_H
