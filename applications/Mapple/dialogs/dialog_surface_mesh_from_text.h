#ifndef DIALOG_SURFACE_CREATE_FROM_TEXT_H
#define DIALOG_SURFACE_CREATE_FROM_TEXT_H

#include "dialog.h"

namespace Ui {
    class DialogSurfaceMeshFromText;
}

class DialogSurfaceMeshFromText : public Dialog {
Q_OBJECT

public:
    explicit DialogSurfaceMeshFromText(MainWindow *window, QDockWidget* dockWidgetCommand);

    ~DialogSurfaceMeshFromText();

private Q_SLOTS:

    void setFontFile();
    void apply();

private:
    Ui::DialogSurfaceMeshFromText *ui;
    std::string font_path_;
};

#endif // DIALOG_SURFACE_CREATE_FROM_TEXT_H
