#ifndef WIDGET_GLOBAL_SETTING_H
#define WIDGET_GLOBAL_SETTING_H

#include <QWidget>


namespace Ui {
class WidgetGlobalSetting;
}

class PaintCanvas;

class WidgetGlobalSetting : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetGlobalSetting(QWidget *parent);
    ~WidgetGlobalSetting();

private slots:
    void setEnableClippingPlane(bool);
    void setClippingPlaneVisible(bool);
    void setClippingPlaneColor();
    void setEnableCrossSection(bool);
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
    Ui::WidgetGlobalSetting* ui;

    PaintCanvas* viewer_;
};


#endif // WIDGET_GLOBAL_SETTING_H
