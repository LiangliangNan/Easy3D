#include "dialog_surface_sampling.h"
#include "ui_dialog_surface_sampling.h"


DialogSurfaceSampling::DialogSurfaceSampling(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSurfaceSampling)
{
    ui->setupUi(this);
}

DialogSurfaceSampling::~DialogSurfaceSampling()
{
    delete ui;
}

int DialogSurfaceSampling::pointNumber() const {
    return ui->spinBoxPointNumber->value();
}
