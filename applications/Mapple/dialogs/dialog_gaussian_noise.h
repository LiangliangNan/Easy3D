#ifndef DIALOG_GAUSSIAN_NOISE_H
#define DIALOG_GAUSSIAN_NOISE_H

#include "dialog.h"

namespace Ui {
    class DialogGaussianNoise;
}

class DialogGaussianNoise : public Dialog {
Q_OBJECT

public:
    DialogGaussianNoise(MainWindow *window, QDockWidget* dockWidgetCommand);
    ~DialogGaussianNoise();

private Q_SLOTS:
    void apply();
    void computeBBox();

private:
    Ui::DialogGaussianNoise *ui;

    QString default_sigma_;
    double suggested_ratio_;
};

#endif
