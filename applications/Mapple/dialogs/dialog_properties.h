#ifndef DIALOG_PROPERTIES_H
#define DIALOG_PROPERTIES_H

#include "dialog.h"

namespace Ui {
    class DialogProperties;
}

namespace easy3d {
    class Model;
}

class DialogProperties : public Dialog {
Q_OBJECT

public:
    DialogProperties(MainWindow *window, QDockWidget* dockWidgetCommand);
    ~DialogProperties();

private Q_SLOTS:

    void updateProperties();
    void applyCommand();

    void commandChanged(const QString &);
    void modelChanged(const QString &);
    void locationChanged(const QString &);

private:
    easy3d::Model *getModel();

    void removeProperty();
    void renameProperty();
    void convertPropertyDataTye();
    void mergeProperties();   // x, y, z -> vec3
    void splitProperty();     // vec3 -> x, y, z

private:
    Ui::DialogProperties *ui;
};

#endif
