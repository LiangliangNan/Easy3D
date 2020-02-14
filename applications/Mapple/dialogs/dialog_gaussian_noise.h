#ifndef DIALOG_GAUSSIAN_NOISE_H
#define DIALOG_GAUSSIAN_NOISE_H

#include <QDialog>

namespace Ui {
    class DialogGaussianNoise;
}

class PaintCanvas;

class DialogGaussianNoise : public QDialog {
Q_OBJECT

public:
    DialogGaussianNoise(QWidget *parent = 0);
    ~DialogGaussianNoise();

private Q_SLOTS:
    void apply();
    void computeBBox();

private:
    Ui::DialogGaussianNoise *ui;

    PaintCanvas *viewer_;
    QString default_sigma_;
    float suggested_ratio_;
};

#endif
