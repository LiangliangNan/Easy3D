#ifndef DIALOG_SURFACE_SAMPLING_H
#define DIALOG_SURFACE_SAMPLING_H

#include <QDialog>

namespace Ui {
class DialogSurfaceSampling;
}

class DialogSurfaceSampling : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSurfaceSampling(QWidget *parent = nullptr);
    ~DialogSurfaceSampling();

    int pointNumber() const;

private:
    Ui::DialogSurfaceSampling *ui;
};

#endif // DIALOG_SURFACE_SAMPLING_H
