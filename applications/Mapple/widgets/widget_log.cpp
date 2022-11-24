/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/


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
	for (const auto& it : items)
        strings << it->text();

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