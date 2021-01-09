#include "widget_log.h"

#include <QMenu>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QMouseEvent>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QTime>


WidgetLog::WidgetLog(QWidget *parent)
        : QListWidget(parent), popupMenu_(nullptr)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QTreeWidget::ContiguousSelection);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));
}


WidgetLog::~WidgetLog() {
}


void WidgetLog::showContextMenu(const QPoint &pos) {
    if (!popupMenu_) {
        popupMenu_ = new QMenu(this);

        QAction *actionCopyLog = popupMenu_->addAction("Copy");
        connect(actionCopyLog, SIGNAL(triggered()), this, SLOT(copySelected()));
        popupMenu_->addSeparator();

        QAction *actionExportLog = popupMenu_->addAction("Export log");
        connect(actionExportLog, SIGNAL(triggered()), this, SLOT(exportLog()));

        popupMenu_->addSeparator();
        QAction *actionClearLog = popupMenu_->addAction("Clear log");
        connect(actionClearLog, SIGNAL(triggered()), this, SLOT(clearLog()));
    }

    popupMenu_->popup(mapToGlobal(pos));
}


void WidgetLog::copySelected() {
    QStringList strings;
	const QList<QListWidgetItem*>& items = selectedItems();
	for (int i = 0; i < items.size(); ++i)
        strings << items[i]->text();

    QApplication::clipboard()->setText(strings.join("\n"));
}


void WidgetLog::exportLog() {
    QString time_format = "yyyyMMddHHmmss";
    const QString& fileName = QFileDialog::getSaveFileName(
            this,
            "Export log to file", "Mapple_" + QDateTime::currentDateTime().toString(time_format) + ".log",
            "Mapple log file (*.log)\n"
            "All formats (*.*)"
    );

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        for (int i = 0; i < count(); ++i) {
            const QString line = item(i)->text() + "\n";
            file.write(line.toStdString().c_str());
        }
        file.close();
    }
}


void WidgetLog::clearLog() {
    clear();
}