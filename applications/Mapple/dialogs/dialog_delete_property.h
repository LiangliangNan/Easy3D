#ifndef DIALOG_DELETE_PROPERTY_H
#define DIALOG_DELETE_PROPERTY_H

#include <QDialog>

namespace Ui {
    class DialogDeleteProperty;
}

namespace easy3d {
    class Model;
}

class MainWindow;
class PaintCanvas;

class DialogDeleteProperty : public QDialog {
Q_OBJECT

public:
    DialogDeleteProperty(QWidget *parent = 0);
    ~DialogDeleteProperty();

private Q_SLOTS:

    void queryAvailableProperties();
    void deleteSelectedProperty();
    void modelChanged(const QString &);
    void locationChanged(const QString &);

private:
    easy3d::Model *getModel();

private:
    Ui::DialogDeleteProperty *ui;
    PaintCanvas *viewer_;
    MainWindow *window_;
};

#endif
