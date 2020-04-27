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
    void propertyChanged(const QString &);

private:
    easy3d::Model *getModel();

    bool removeProperty();
    bool renameProperty();
    bool convertPropertyDataType();
    bool mergeProperties();   // x, y, z -> vec3
    bool splitProperty();     // vec3 -> x, y, z

    bool updatePropertyType(const QString& location, const QString& name);

private:
    Ui::DialogProperties *ui;
};

#endif
