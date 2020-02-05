#ifndef DIALOG_SNAPSHOT_H
#define DIALOG_SNAPSHOT_H

#include <QDialog>

namespace Ui {
    class DialogSnapshot;
}

class PaintCanvas;

class DialogSnapshot : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSnapshot(QWidget *parent = 0);
    ~DialogSnapshot();

    void setImageFileName(const QString& fileName);

private slots:
    void computeImageSize();
    void saveSnapshot();

private:
    Ui::DialogSnapshot *ui;

    PaintCanvas*	viewer_;
    QString         fileName_;
};

#endif // DIALOG_SNAPSHOT_H
