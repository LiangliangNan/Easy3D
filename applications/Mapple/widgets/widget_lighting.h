#ifndef WIDGET_LIGHTING_H
#define WIDGET_LIGHTING_H

#include <QWidget>


namespace Ui {
class WidgetLighting;
}

class PaintCanvas;

class WidgetLighting : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetLighting(QWidget *parent);
    ~WidgetLighting();

private slots:
    void setClippingPlane(bool);
    void setCrossSection(bool);
    void setCrossSectionThickness(double);


    void setSSAOAlgorithm(int);
    void setSSAORadius(int);
    void setSSAOIntensity(int);
    void setSSAOBias(int);
    void setSSAOSharpness(int);

    void setEyeDomeLighting(bool);
    void setTransparency(bool);
    void setShadow(bool);

    void setLightDistance(int);
    void setShadowSmoothPattern(int);
    void setShadowSoftness(int);
    void setShadowDarkness(int);
    void setImposterShadows(bool);

private:
    Ui::WidgetLighting* ui;

    PaintCanvas* viewer_;
};


#endif // WIDGET_LIGHTING_H
