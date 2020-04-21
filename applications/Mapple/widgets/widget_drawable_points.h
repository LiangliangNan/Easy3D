#ifndef WIDGET_DRAWABLE_POINTS_H
#define WIDGET_DRAWABLE_POINTS_H

#include <unordered_map>
#include <QWidget>

#include "widgets/widget_drawable.h"

namespace Ui {
    class WidgetPointsDrawable;
}


class WidgetPointsDrawable : public WidgetDrawable {
Q_OBJECT

public:
    explicit WidgetPointsDrawable(QWidget *parent);

    ~WidgetPointsDrawable() override;

    // update the panel to be consistent with the drawable's rendering parameters
    void updatePanel() override;

public slots:

    void setActiveDrawable(const QString &);

    void setPointSize(double);
    void setImposterStyle(const QString &);

    void setColorScheme(const QString &) override ;
    void setDefaultColor();
    void setBackColor();

    void setVectorField(const QString &);

private:
    void connectAll();
    void disconnectAll();

    easy3d::Drawable *drawable() override ;
    void disableUnavailableOptions() override ;

    void updateVectorFieldBuffer(easy3d::Model *model, const std::string &name) override ;

    // model depended stuff
    std::vector<QString> colorSchemes(const easy3d::Model *model);
    std::vector<QString> vectorFields(const easy3d::Model *model);

private:
    Ui::WidgetPointsDrawable *ui;
};

#endif // WIDGET_DRAWABLE_POINTS_H
