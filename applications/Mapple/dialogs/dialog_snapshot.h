#ifndef DIALOG_SNAPSHOT_H
#define DIALOG_SNAPSHOT_H

#include "dialog.h"

namespace Ui {
    class DialogSnapshot;
}

class DialogSnapshot : public Dialog
{
    Q_OBJECT

public:
    explicit DialogSnapshot(MainWindow *window, QDockWidget* dockWidgetCommand);
    ~DialogSnapshot();

    void setImageFileName(const QString& fileName);

private slots:
    void computeImageSize();
    void saveSnapshot();
    void closeDialog();

private:
    Ui::DialogSnapshot *ui;
    QString fileName_;
};

#endif // DIALOG_SNAPSHOT_H
