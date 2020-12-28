#ifndef WIDGET_DRAWABLE_TRIANGLES_H
#define WIDGET_DRAWABLE_TRIANGLES_H

#include <QWidget>

#include "widgets/widget_drawable.h"

namespace Ui {
    class WidgetTrianglesDrawable;
}


class WidgetTrianglesDrawable : public WidgetDrawable {
Q_OBJECT

public:
    explicit WidgetTrianglesDrawable(QWidget *parent);
    ~WidgetTrianglesDrawable() override;

    // update the panel to be consistent with the drawable's rendering parameters
    void updatePanel() override;

public slots:

    void setActiveDrawable(const QString &);
    void setPhongShading(bool);
    void setColorScheme(const QString &) override;

    void setDefaultColor();
    void setBackColor();
    void setTextureFile();
    void setOpacity(int);
    void setScalarFieldStyle(int);
    void setVectorField(const QString &);

    virtual void setHighlightMin(int) override;
    virtual void setHighlightMax(int) override;

private:
    void connectAll();
    void disconnectAll();

    easy3d::Drawable *drawable() override;

    void disableUnavailableOptions() override;
    void updateVectorFieldBuffer(easy3d::Model *model, const std::string &name) override;

    // model depended stuff
    std::vector<QString> colorSchemes(const easy3d::Model *model);
    std::vector<QString> vectorFields(const easy3d::Model *model);

private:
    Ui::WidgetTrianglesDrawable *ui;
};

#endif // WIDGET_DRAWABLE_TRIANGLES_H
