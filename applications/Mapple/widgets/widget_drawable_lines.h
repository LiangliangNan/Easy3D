#ifndef WIDGET_DRAWABLE_LINES_H
#define WIDGET_DRAWABLE_LINES_H

#include <unordered_map>
#include <QWidget>

#include "widgets/widget_drawable.h"

namespace Ui {
    class WidgetLinesDrawable;
}


class WidgetLinesDrawable : public WidgetDrawable {
Q_OBJECT

public:
    explicit WidgetLinesDrawable(QWidget *parent);

    ~WidgetLinesDrawable() override;

    // update the panel to be consistent with the drawable's rendering parameters
    void updatePanel() override;

public slots:

    void setActiveDrawable(const QString &);

    void setLineWidth(double);
    void setImposterStyle(const QString &);

    void setColorScheme(const QString &) override ;
    void setDefaultColor();

    void setVectorField(const QString &);

private:
    void connectAll();
    void disconnectAll();

    easy3d::Drawable *drawable() override ;
    void disableUnavailableOptions() override ;

    // model depended stuff
    std::vector<QString> colorSchemes(const easy3d::Model *model);
    std::vector<QString> vectorFields(const easy3d::Model *model);

    void updateVectorFieldBuffer(easy3d::Model *model, const std::string &name) override ;

private:
    Ui::WidgetLinesDrawable *ui;
};

#endif // WIDGET_DRAWABLE_LINES_H
