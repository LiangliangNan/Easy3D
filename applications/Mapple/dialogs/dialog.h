#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

class MainWindow;
class PaintCanvas;
class QDockWidget;


class Dialog : public QDialog {
Q_OBJECT

public:
    Dialog(MainWindow *window, QDockWidget* dockWidgetCommand);
    ~Dialog();

    void bestSize();

protected:

    PaintCanvas *viewer_;
    MainWindow *window_;
    QDockWidget* dockWidgetCommand_;
};

#endif
