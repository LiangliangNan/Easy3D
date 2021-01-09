#ifndef WIDGET_LOG_H
#define WIDGET_LOG_H

#include <QListWidget>

class QMenu;

class WidgetLog : public QListWidget {
Q_OBJECT

public:
    WidgetLog(QWidget *parent = 0);
    ~WidgetLog();

private Q_SLOTS :
    void showContextMenu(const QPoint &p);
    void copySelected();
    void exportLog();
    void clearLog();

private:
    QMenu *popupMenu_;
};

#endif  // WIDGET_LOG_H