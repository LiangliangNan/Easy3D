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


#ifndef WIDGET_MODEL_LIST_H
#define WIDGET_MODEL_LIST_H

#include <QTreeWidget>

namespace easy3d {
    class Model;
    class Drawable;
}

class MainWindow;
class PaintCanvas;

class WidgetModelList : public QTreeWidget {
Q_OBJECT

public:
    explicit WidgetModelList(QWidget *parent = nullptr);
    ~WidgetModelList() override = default;

    void init(MainWindow *w);

    // single model
    void addModel(easy3d::Model *obj, bool make_current);

    void mergeModels(const std::vector<easy3d::Model *> &objs);

    void decomposeModel(easy3d::Model *obj);

    void updateModelList();

    void prepareContextMenu(QMenu *menu);

    void updateDrawableVisibility(easy3d::Drawable* d); // the given drawable
    void updateDrawableVisibilities();  // all drawables

    void updateVisibilities();  // all models and drawables

private Q_SLOTS :

    void setAutoFocus(bool b);
    void setSelectedOnly(bool b);

    void duplicateCurrent();
    void mergeSelected();
    void deleteSelected();
    void decomposeSelected();

    void invertSelection();
    void showSelected();
    void hideSelected();
    void invertShowHide();

    void showAllModels();

    void modelItemPressed(QTreeWidgetItem *current, int column);
    void currentModelItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void modelItemSelectionChanged();
    void modelItemChanged(QTreeWidgetItem *current, int column);

    void showContextMenu(const QPoint &p);

    void applyRenderingToAllModels();

signals:
    void currentDrawableChanged(easy3d::Drawable*);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    PaintCanvas *viewer();

private:
    MainWindow *mainWindow_;

    QMenu *popupMenu_;

    bool auto_focus_;
    bool selected_only_;

    int name_column_;
};

#endif