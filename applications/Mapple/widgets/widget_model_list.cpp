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


#include "widget_model_list.h"
#include "main_window.h"
#include "paint_canvas.h"
#include "walk_through.h"

#include <easy3d/core/graph.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/algo/surface_mesh_components.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/file_system.h>

#include <QMenu>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QMouseEvent>
#include <QMessageBox>
#include <QStyledItemDelegate>


using namespace easy3d;

class ModelItem : public QTreeWidgetItem {
public:
    ModelItem(QTreeWidget *parent, Model *m) : QTreeWidgetItem(parent), model_(m) {
        for (int i = 0; i < parent->columnCount(); ++i)
            QTreeWidgetItem::setTextAlignment(i, Qt::AlignLeft);

        // allow to edit model names
        setFlags(flags() | Qt::ItemIsEditable);
    }

    ~ModelItem() override = default;

    Model *model() { return model_; }

    void setIcon(int column) {
        if (dynamic_cast<SurfaceMesh *>(model())) {
            static QIcon iconMesh(QString::fromUtf8(":/resources/icons/mesh.png"));
            QTreeWidgetItem::setIcon(column, iconMesh);
        } else if (dynamic_cast<PointCloud *>(model())) {
            static QIcon iconPointCloud(QString::fromUtf8(":/resources/icons/point_cloud.png"));
            QTreeWidgetItem::setIcon(column, iconPointCloud);
        } else if (dynamic_cast<Graph *>(model())) {
            static QIcon iconGraph(QString::fromUtf8(":/resources/icons/graph.png"));
            QTreeWidgetItem::setIcon(column, iconGraph);
        } else if (dynamic_cast<PolyMesh *>(model())) {
            static QIcon iconTetrahedra(QString::fromUtf8(":/resources/icons/tetrahedra.png"));
            QTreeWidgetItem::setIcon(column, iconTetrahedra);
        }
    }

    void setVisible(int column, bool visible) {
        static QIcon iconShow(QString::fromUtf8(":/resources/icons/show.png"));
        static QIcon iconHide(QString::fromUtf8(":/resources/icons/hide.png"));
        if (visible)
            QTreeWidgetItem::setIcon(column, iconShow);
        else
            QTreeWidgetItem::setIcon(column, iconHide);

        int num = childCount();
        for (int i = 0; i < num; ++i)
            this->child(i)->setDisabled(!visible);
    }

    void highlight(bool b) {
        for (int i = 0; i < columnCount(); ++i)
            QTreeWidgetItem::setBackground(i, b ? QColor(80, 208, 255) : QBrush());
    }

private:
    Model *model_;
};


class DrawableItem : public QTreeWidgetItem {
public:
    DrawableItem(ModelItem *parent, Drawable *d) : QTreeWidgetItem(parent), drawable_(d) {
        for (int i = 0; i < parent->columnCount(); ++i)
            QTreeWidgetItem::setTextAlignment(i, Qt::AlignLeft);
    }

    ~DrawableItem() override = default;

    Drawable *drawable() { return drawable_; }

    void setVisible(int column, bool visible) {
        static QIcon iconShow(QString::fromUtf8(":/resources/icons/show.png"));
        static QIcon iconHide(QString::fromUtf8(":/resources/icons/hide.png"));
        if (visible)
            QTreeWidgetItem::setIcon(column, iconShow);
        else
            QTreeWidgetItem::setIcon(column, iconHide);
    }

private:
    Drawable *drawable_;
};


class NoEditDelegate : public QStyledItemDelegate {
public:
    explicit NoEditDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        return nullptr;
    }
};


WidgetModelList::WidgetModelList(QWidget *parent)
        : QTreeWidget(parent), mainWindow_(nullptr), popupMenu_(nullptr), auto_focus_(false), selected_only_(false) {
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(modelItemSelectionChanged()));
    connect(this, SIGNAL(itemPressed(QTreeWidgetItem *, int)), this, SLOT(modelItemPressed(QTreeWidgetItem *, int)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    // model name has been changed
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(modelItemChanged(QTreeWidgetItem *, int)));

    if (selected_only_)
        setSelectionMode(QAbstractItemView::SingleSelection);
    else
        setSelectionMode(QAbstractItemView::ExtendedSelection);
}


void WidgetModelList::init(MainWindow *w) {
    mainWindow_ = w;

    QStringList headerLabels;
    headerLabels << "Index" << "Type" << "Show" << "Name";
    name_column_ = 3;

    setHeaderLabels(headerLabels);
    for (int i = 0; i < columnCount(); ++i) {
        headerItem()->setTextAlignment(i, Qt::AlignLeft);

        // allow to edit model names
        if (i != name_column_)
            setItemDelegateForColumn(i, new NoEditDelegate(this));
    }

    header()->setDefaultSectionSize(60);

    setIndentation(10);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
}


PaintCanvas *WidgetModelList::viewer() {
    return mainWindow_->viewer();
}


void WidgetModelList::updateDrawableVisibility(easy3d::Drawable *d) {
    int num = topLevelItemCount();
    for (int i = 0; i < num; ++i) {
        auto item = dynamic_cast<ModelItem *>(topLevelItem(i));
        if (item->model() == d->model()) {
            int num_children = item->childCount();
            for (int j = 0; j < num_children; ++j) {
                auto d_item = dynamic_cast<DrawableItem *>(item->child(j));
                if (d_item->drawable() == d)
                    d_item->setVisible(2, d->is_visible());
            }
        }
    }
}


void WidgetModelList::updateVisibilities() {
    int num = topLevelItemCount();
    for (int i = 0; i < num; ++i) {
        auto item = dynamic_cast<ModelItem *>(topLevelItem(i));
        bool visible = item->model()->renderer()->is_visible();
        item->setVisible(2, visible);
        item->highlight(item->model() == viewer()->currentModel());
        int num_children = item->childCount();
        for (int j = 0; j < num_children; ++j) {
            auto d_item = dynamic_cast<DrawableItem *>(item->child(j));
            d_item->setVisible(2, d_item->drawable()->is_visible());
        }
    }
}


void WidgetModelList::updateDrawableVisibilities() {
    int num = topLevelItemCount();
    for (int i = 0; i < num; ++i) {
        auto item = dynamic_cast<ModelItem *>(topLevelItem(i));
        int num_children = item->childCount();
        for (int j = 0; j < num_children; ++j) {
            auto d_item = dynamic_cast<DrawableItem *>(item->child(j));
            d_item->setVisible(2, d_item->drawable()->is_visible());
        }
    }
}


void WidgetModelList::prepareContextMenu(QMenu *menu) {
    menu->clear();  // I want to customize the menu list

    int num_selected = 0;
    int num_selected_meshes = 0;
    int num_unselected = 0;
    int num_invisible = 0;
    int num_visible_in_selected = 0;
    int num_invisible_in_selected = 0;

    int total = topLevelItemCount();
    for (int i = 0; i < total; ++i) {
        QTreeWidgetItem *item = topLevelItem(i);
        Model *model = dynamic_cast<ModelItem *>(item)->model();

        if (item->isSelected()) {
            ++num_selected;
            if (dynamic_cast<SurfaceMesh *>(model))
                ++num_selected_meshes;
        } else
            ++num_unselected;

        if (!model->renderer()->is_visible()) {
            ++num_invisible;
            if (item->isSelected())
                ++num_invisible_in_selected;
        } else {
            if (item->isSelected())
                ++num_visible_in_selected;
        }
    }

    QAction *actionInvertSelection = menu->addAction("Invert Selection");
    connect(actionInvertSelection, SIGNAL(triggered()), this, SLOT(invertSelection()));
    menu->addSeparator();

    // hide / show
    if (num_invisible_in_selected > 0) {
        QAction *actionShowSelected = menu->addAction("Show");
        connect(actionShowSelected, SIGNAL(triggered()), this, SLOT(showSelected()));
    }

    if (num_visible_in_selected > 0) {
        QAction *actionHideSelected = menu->addAction("Hide");
        connect(actionHideSelected, SIGNAL(triggered()), this, SLOT(hideSelected()));
    }

    QAction *actionInvertShowHide = menu->addAction("Invert Show/Hide");
    connect(actionInvertShowHide, SIGNAL(triggered()), this, SLOT(invertShowHide()));

    if (num_invisible > 0) {
        QAction *actionShowAll = menu->addAction("Show All");
        connect(actionShowAll, SIGNAL(triggered()), this, SLOT(showAllModels()));
    }

    menu->addSeparator();

    // manipulation
    if (num_selected == 1) {
        QAction *actionDuplicateCurrent = menu->addAction(
                QIcon(QString::fromUtf8(":/resources/icons/duplicate.png")), "Duplicate");
        connect(actionDuplicateCurrent, SIGNAL(triggered()), this, SLOT(duplicateCurrent()));
    } else if (num_selected > 1) {
        QAction *actionMergeSelected = menu->addAction(
                QIcon(QString::fromUtf8(":/resources/icons/merge.png")), "Merge");
        connect(actionMergeSelected, SIGNAL(triggered()), this, SLOT(mergeSelected()));
    }

    if (num_selected_meshes > 0) {
        QAction *actionDecomposeSelected = menu->addAction(
                QIcon(QString::fromUtf8(":/resources/icons/decompose.png")), "Decompose");
        connect(actionDecomposeSelected, SIGNAL(triggered()), this, SLOT(decomposeSelected()));
    }

    if (num_selected > 0) {
        menu->addSeparator();
        QAction *actionDeleteSelected = menu->addAction(
                QIcon(QString::fromUtf8(":/resources/icons/delete.png")), "Delete");
        connect(actionDeleteSelected, SIGNAL(triggered()), this, SLOT(deleteSelected()));
    }
}


void WidgetModelList::showContextMenu(const QPoint &pos) {
    if (!popupMenu_)
        popupMenu_ = new QMenu(this);

    prepareContextMenu(popupMenu_);

    // Liangliang: the header height was not counted by default?
    //popupMenu_->popup(mapToGlobal(pos));
    QPoint p = mapToGlobal(pos);
    popupMenu_->popup(QPoint(p.x(), p.y() + header()->height()));
}


void WidgetModelList::updateModelList() {
    disconnect(this, SIGNAL(itemSelectionChanged()), this, SLOT(modelItemSelectionChanged()));
    disconnect(this, SIGNAL(itemPressed(QTreeWidgetItem * , int)), this,
               SLOT(modelItemPressed(QTreeWidgetItem * , int)));
    disconnect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    disconnect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(modelItemChanged(QTreeWidgetItem *, int)));

    // clear everything and creat the list from scratch
    clear();

    if (selected_only_) {
        for (auto m : viewer()->models())
            m->renderer()->set_visible(m == viewer()->currentModel());
    }

    const std::vector<Model *> &models = viewer()->models();
    for (unsigned int i = 0; i < models.size(); ++i) {
        Model *model = models[i];
        const std::string &name = file_system::base_name(model->name());
        auto item = dynamic_cast<ModelItem *>(this->topLevelItem(i));
        if (!item) {
            item = new ModelItem(this, model);
            addTopLevelItem(item);
        }

        item->setData(0, Qt::DisplayRole, i + 1);
        item->setIcon(1);
        item->setVisible(2, model->renderer()->is_visible());
        item->setData(3, Qt::DisplayRole, QString::fromStdString(name));
        item->highlight(model == viewer()->currentModel());

#if 1   // add the drawables as children
        for (auto d : model->renderer()->points_drawables()) {
            auto d_item = new DrawableItem(item, d);
            d_item->setVisible(2, d->is_visible());
            d_item->setData(3, Qt::DisplayRole, QString::fromStdString(d->name()));
            item->addChild(d_item);
        }

        for (auto d : model->renderer()->lines_drawables()) {
            auto d_item = new DrawableItem(item, d);
            d_item->setVisible(2, d->is_visible());
            d_item->setData(3, Qt::DisplayRole, QString::fromStdString(d->name()));
            item->addChild(d_item);
        }

        for (auto d : model->renderer()->triangles_drawables()) {
            auto d_item = new DrawableItem(item, d);
            d_item->setVisible(2, d->is_visible());
            d_item->setData(3, Qt::DisplayRole, QString::fromStdString(d->name()));
            item->addChild(d_item);
        }
#endif
    }

    connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(modelItemSelectionChanged()));
    connect(this, SIGNAL(itemPressed(QTreeWidgetItem * , int)), this, SLOT(modelItemPressed(QTreeWidgetItem * , int)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(modelItemChanged(QTreeWidgetItem *, int)));

    mainWindow_->updateRenderingPanel();
    mainWindow_->updateWindowTitle();
    mainWindow_->updateStatusBar();
}


void WidgetModelList::duplicateCurrent() {
    if (viewer()->currentModel()) {
        Model *model = viewer()->currentModel();

        Model *copy = nullptr;
        if (dynamic_cast<SurfaceMesh *>(model))
            copy = new SurfaceMesh(*dynamic_cast<SurfaceMesh *>(model));
        else if (dynamic_cast<PointCloud *>(model))
            copy = new PointCloud(*dynamic_cast<PointCloud *>(model));
        else if (dynamic_cast<Graph *>(model))
            copy = new Graph(*dynamic_cast<Graph *>(model));
        else if (dynamic_cast<PolyMesh *>(model))
            copy = new PolyMesh(*dynamic_cast<PolyMesh *>(model));

        if (copy) {
            const std::string &name = file_system::name_less_extension(model->name()) + "_copy";
            copy->set_name(name);
            viewer()->addModel(copy);
            addModel(copy, true);
        }
    }
}


void WidgetModelList::showSelected() {
    if (selectedItems().empty())
        return;

    const QList<QTreeWidgetItem *> &items = selectedItems();
    for (const auto& it : items) {
        auto item = dynamic_cast<ModelItem *>(it);
        Model *model = item->model();
        model->renderer()->set_visible(true);
    }

    updateModelList();
    viewer()->update();
}


void WidgetModelList::hideSelected() {
    if (selectedItems().empty())
        return;

    const QList<QTreeWidgetItem *> &items = selectedItems();
    for (const auto& it : items) {
        auto item = dynamic_cast<ModelItem *>(it);
        Model *model = item->model();
        model->renderer()->set_visible(false);
    }

    updateModelList();
    viewer()->update();
}


void WidgetModelList::invertShowHide() {
    int num = topLevelItemCount();

    if (selected_only_) {
        std::vector<unsigned char> visible(num);
        for (int i = 0; i < num; ++i) {
            QTreeWidgetItem *item = topLevelItem(i);
            auto modelItem = dynamic_cast<ModelItem *>(item);
            visible[i] = (!modelItem->model()->renderer()->is_visible());
        }
        mainWindow_->setShowSelectedOnly(false);
        for (int i = 0; i < num; ++i) {
            QTreeWidgetItem *item = topLevelItem(i);
            auto modelItem = dynamic_cast<ModelItem *>(item);
            modelItem->model()->renderer()->set_visible(visible[i]);
        }
    } else {
        for (int i = 0; i < num; ++i) {
            QTreeWidgetItem *item = topLevelItem(i);
            auto modelItem = dynamic_cast<ModelItem *>(item);
            Model *model = modelItem->model();
            model->renderer()->set_visible(!model->renderer()->is_visible());
        }
    }

    updateModelList();
    viewer()->update();
}


void WidgetModelList::showAllModels() {
    if (selected_only_)
        mainWindow_->setShowSelectedOnly(false);
    else {
        const std::vector<Model *> &models = viewer()->models();
        for (auto& m : models) {
            m->renderer()->set_visible(true);
        }
        updateModelList();
        viewer()->update();
    }
}


void WidgetModelList::decomposeSelected() {
    const QList<QTreeWidgetItem *> &items = selectedItems();
    for (const auto& it : items) {
        auto item = dynamic_cast<ModelItem *>(it);
        Model *model = item->model();
        decomposeModel(model);
    }
}


void WidgetModelList::invertSelection() {
    disconnect(this, SIGNAL(itemSelectionChanged()), this, SLOT(modelItemSelectionChanged()));
    int num = topLevelItemCount();
    for (int i = 0; i < num; ++i) {
        QTreeWidgetItem *item = topLevelItem(i);
        item->setSelected(!item->isSelected());
    }
    connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(modelItemSelectionChanged()));

    emit itemSelectionChanged();
}


void WidgetModelList::mergeSelected() {
    std::vector<Model *> models;
    const QList<QTreeWidgetItem *> &items = selectedItems();
    for (const auto& it : items) {
        auto item = dynamic_cast<ModelItem *>(it);
        models.push_back(item->model());
    }

    mergeModels(models);
}


void WidgetModelList::deleteSelected() {
    const QList<QTreeWidgetItem *> &items = selectedItems();
    for (const auto& it : items) {
        auto item = dynamic_cast<ModelItem *>(it);
        if (item)
            viewer()->deleteModel(item->model());
    }

    Model *current_model = viewer()->currentModel();
    if (selected_only_) {
        for (auto m : viewer()->models())
            m->renderer()->set_visible(m == current_model);
    }

    updateModelList();

    if (auto_focus_)
        viewer()->fitScreen();

    viewer()->update();

    mainWindow_->updateRenderingPanel();
    mainWindow_->updateWindowTitle();
    mainWindow_->updateStatusBar();
}


void WidgetModelList::currentModelItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    if (current == previous)
        return;

    auto current_item = dynamic_cast<ModelItem *>(current);
    (void) previous;
    if (!current_item)
        return;

    Model *model = current_item->model();
    viewer()->setCurrentModel(model);

    if (selected_only_) {
        for (auto m : viewer()->models())
            m->renderer()->set_visible(m == model);
    }
    updateVisibilities();

    if (auto_focus_)
        viewer()->fitScreen(model);

    viewer()->update();

    mainWindow_->updateRenderingPanel();
    mainWindow_->updateWindowTitle();
    mainWindow_->updateStatusBar();
}


void WidgetModelList::modelItemSelectionChanged() {
    int num = topLevelItemCount();
    for (int i = 0; i < num; ++i) {
        auto item = dynamic_cast<ModelItem *>(topLevelItem(i));

        // don't allow changing selection for camera path creation
        if (viewer()->walkThrough()->status() == WalkThrough::STOPPED && viewer()->isSelectModelEnabled())
            item->model()->renderer()->set_selected(item->isSelected());
    }

    viewer()->update();
    update();
}


void WidgetModelList::modelItemChanged(QTreeWidgetItem *current, int column) {
    if (dynamic_cast<ModelItem *>(current) == nullptr)
        return; // this is not a model item (might be a drawable item

    const std::string text = current->text(name_column_).toStdString();
    if (text.empty()) {
        QMessageBox::warning(this, "Model name error", "Model name cannot be empty", QMessageBox::Ok,
                             QMessageBox::NoButton);
        return;
    }

    Model* model = dynamic_cast<ModelItem *>(current)->model();
    if (!model) {
        QMessageBox::warning(this, "Model error", "No model is associated with the QTreeWidgetItem", QMessageBox::Ok,
                             QMessageBox::NoButton);
        return;
    }

    if (current && column == name_column_) {
        const std::string prev_name = model->name();
        const std::string path = file_system::parent_directory(prev_name);
        const std::string ext = file_system::extension(prev_name);
        std::string new_name;
        if (path.empty() && ext.empty())
            new_name = text;
        else
            new_name = path + "/" + text + (ext.empty() ? "" : ("." + ext));

        if (new_name != prev_name) {
            model->set_name(new_name);
            LOG(INFO) << "model name '" << prev_name << "' changed to '" << model->name() << "'";
            mainWindow_->updateWindowTitle();
        }
    }
}


void WidgetModelList::modelItemPressed(QTreeWidgetItem *current, int column) {
    Model *prev_active_model = viewer()->currentModel();

    Model *active_model = nullptr;
    if (dynamic_cast<ModelItem *>(current)) {
        auto current_item = dynamic_cast<ModelItem *>(current);
        active_model = current_item->model();
        viewer()->setCurrentModel(active_model);
        if (column == 2 && !selected_only_) {
            Model *model = current_item->model();
            bool visible = !model->renderer()->is_visible();
            current_item->setVisible(2, visible);
            model->renderer()->set_visible(visible);
        }
    } else if (dynamic_cast<DrawableItem *>(current)) {
        auto drawable_item = dynamic_cast<DrawableItem *>(current);
        active_model = drawable_item->drawable()->model();
        if (column == 2) {
            Drawable *d = drawable_item->drawable();
            bool visible = !d->is_visible();
            d->set_visible(visible);
            drawable_item->setVisible(2, visible);
        }
        mainWindow_->activeDrawableChanged(drawable_item->drawable());
    }

    viewer()->setCurrentModel(active_model);
    if (selected_only_) {
        for (auto m : viewer()->models()) {
            m->renderer()->set_visible(m == active_model);
        }
    }
    updateVisibilities();

    if (auto_focus_ && active_model != prev_active_model)
        viewer()->fitScreen(active_model);

    viewer()->update();

    mainWindow_->updateRenderingPanel();
    mainWindow_->updateWindowTitle();
    mainWindow_->updateStatusBar();
}


void WidgetModelList::mousePressEvent(QMouseEvent *e) {
    QTreeWidget::mousePressEvent(e);
}


void WidgetModelList::mouseReleaseEvent(QMouseEvent *e) {
    QTreeWidget::mouseReleaseEvent(e);
}


void WidgetModelList::setAutoFocus(bool b) {
    auto_focus_ = b;
    if (auto_focus_)
        viewer()->fitScreen(viewer()->currentModel());

    viewer()->update();
}


void WidgetModelList::setSelectedOnly(bool b) {
    selected_only_ = b;

    if (selected_only_) {
        setSelectionMode(QAbstractItemView::SingleSelection);
        connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem * , QTreeWidgetItem * )), this,
                SLOT(currentModelItemChanged(QTreeWidgetItem * , QTreeWidgetItem * )));
    } else {
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        disconnect(this, SIGNAL(currentItemChanged(QTreeWidgetItem * , QTreeWidgetItem * )), this,
                   SLOT(currentModelItemChanged(QTreeWidgetItem * , QTreeWidgetItem * )));
    }

    Model *active_model = viewer()->currentModel();
    if (selected_only_) {
        for (auto m : viewer()->models()) {
            m->renderer()->set_visible(m == active_model);
        }
    } else {
        const std::vector<Model *> &models = viewer()->models();
        for (auto& m : models) {
            m->renderer()->set_visible(true);
        }
    }
    updateVisibilities();
    viewer()->adjustSceneRadius();
    viewer()->update();
}


void WidgetModelList::addModel(Model *model, bool make_current) {
    if (!model)
        return;

    if (make_current) {
        mainWindow_->enableCameraManipulation();
        viewer()->setCurrentModel(model);
    }

    updateModelList();
    mainWindow_->updateRenderingPanel();

    if (selected_only_) {
        for (auto m : viewer()->models()) {
            m->renderer()->set_visible(m == model);
        }
    }

    if (auto_focus_)
        viewer()->fitScreen(model);
    else
        viewer()->fitScreen();

    viewer()->update();
}


void WidgetModelList::mergeModels(const std::vector<Model *> &models) {
    std::vector<SurfaceMesh *> meshes;
    std::vector<PointCloud *> clouds;

    std::vector<Model *> selectedModels;
    for (auto m : models) {
        if (dynamic_cast<SurfaceMesh *>(m))
            meshes.push_back(dynamic_cast<SurfaceMesh *>(m));
        else if (dynamic_cast<PointCloud *>(m))
            clouds.push_back(dynamic_cast<PointCloud *>(m));
    }

    std::vector<Model *> to_delete;

    if (meshes.size() > 1) {
        SurfaceMesh *to = meshes[0];
		auto index = to->add_face_property<int>("f:index", 0);
        ProgressLogger logger(meshes.size() - 1, false, false);
        for (std::size_t i = 1; i < meshes.size(); ++i) {
            logger.notify(i);
            if (logger.is_canceled())
                break;

			unsigned int id_start = to->n_faces();
            SurfaceMesh *from = meshes[i];
            to->join(*from);

			unsigned int count = from->n_faces();
			for (unsigned int id = id_start; id < id_start + count; ++id)
				index[SurfaceMesh::Face(static_cast<int>(id))] = static_cast<int>(i);

            to_delete.push_back(from);
        }

        to->set_name("merged_mesh");
        to->renderer()->update();
        to->manipulator()->reset();
    }

    if (clouds.size() > 1) {
        PointCloud *to = clouds[0];
		auto index = to->add_vertex_property<int>("v:index", 0);
        ProgressLogger logger(clouds.size() - 1, false, false);
        for (int i = 1; i < clouds.size(); ++i) {
            logger.notify(i);
            if (logger.is_canceled())
                break;

			unsigned int id_start = to->n_vertices();
            PointCloud *from = clouds[i];
			to->join(*from);

			unsigned int count = from->n_vertices();
			for (unsigned int id = id_start; id < id_start + count; ++id)
				index[PointCloud::Vertex(static_cast<int>(id))] = static_cast<int>(i);

			to_delete.push_back(from);
        }

        to->set_name("merged_point_set");
        to->renderer()->update();
    }

    for (auto model : to_delete)
        viewer()->deleteModel(model);

    // update display and ui
    if (meshes.size() > 1 || clouds.size() > 1) {
        updateModelList();
        mainWindow_->updateRenderingPanel();
        viewer()->update();
    }
}


void WidgetModelList::decomposeModel(Model *model) {
    auto mesh = dynamic_cast<SurfaceMesh *>(model);
    if (!mesh)
        return;

    const auto &components = SurfaceMeshComponent::extract(mesh);
    if (components.size() == 1) {
        LOG(WARNING) << "model has only one component";
        return;
    }

    const std::string base_name =
            file_system::parent_directory(mesh->name()) + "/" + file_system::base_name(mesh->name()) + "_part_";

    ProgressLogger progress(components.size(), false, false);
    for (unsigned int i = 0; i < components.size(); i++) {
        if (progress.is_canceled()) {
            LOG(WARNING) << "decomposing model cancelled";
            return;
        }

        SurfaceMesh *new_mesh = components[i].to_mesh();
        new_mesh->set_name(base_name + std::to_string(i + 1));
        viewer()->addModel(new_mesh);
        progress.next();
    }

    // delete the original model
    viewer()->deleteModel(mesh);
    updateModelList();
    mainWindow_->updateRenderingPanel();
    viewer()->update();

    LOG(INFO) << "model decomposed into " << components.size() << " parts";
}


namespace internal {

    template <typename DRAWABLE> 
    void propagate(const std::vector<DRAWABLE*>& source_drawables, const std::vector<DRAWABLE*>& target_drawables) {
        for (auto source_drawable : source_drawables) {
            for (auto target_drawable : target_drawables) {
                if (target_drawable->name() == source_drawable->name()) { // propagate only when their names are the same
                    target_drawable->state() = source_drawable->state();
                    switch (source_drawable->type()) {
                    case Drawable::DT_POINTS:
                        dynamic_cast<PointsDrawable*>(target_drawable)->set_point_size(dynamic_cast<PointsDrawable*>(source_drawable)->point_size());
                        dynamic_cast<PointsDrawable*>(target_drawable)->set_impostor_type(dynamic_cast<PointsDrawable*>(source_drawable)->impostor_type());
                        break;
                    case Drawable::DT_LINES:
                        dynamic_cast<LinesDrawable*>(target_drawable)->set_line_width(dynamic_cast<LinesDrawable*>(source_drawable)->line_width());
                        dynamic_cast<LinesDrawable*>(target_drawable)->set_impostor_type(dynamic_cast<LinesDrawable*>(source_drawable)->impostor_type());
                        break;
                    case Drawable::DT_TRIANGLES:
                        dynamic_cast<TrianglesDrawable*>(target_drawable)->set_smooth_shading(dynamic_cast<TrianglesDrawable*>(source_drawable)->smooth_shading());
                        dynamic_cast<TrianglesDrawable*>(target_drawable)->set_opacity(dynamic_cast<TrianglesDrawable*>(source_drawable)->opacity());
                    default:
                        break;
                    }
                }
            }
        }
    }
}


void WidgetModelList::applyRenderingToAllModels() {
    const auto& models = viewer()->models();
    if (models.size() < 2)
        return;

    Model* source = viewer()->currentModel();
    for (auto& target : models) {
        if (target == source)
            continue;

        // points drawables
        internal::propagate(source->renderer()->points_drawables(), target->renderer()->points_drawables());
        // lines drawables
        internal::propagate(source->renderer()->lines_drawables(), target->renderer()->lines_drawables());
        // triangles drawables
        internal::propagate(source->renderer()->triangles_drawables(), target->renderer()->triangles_drawables());
    }

    updateModelList();
    mainWindow_->updateRenderingPanel();
    viewer()->update();
}