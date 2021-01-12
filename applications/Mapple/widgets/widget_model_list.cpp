#include "widget_model_list.h"
#include "main_window.h"
#include "paint_canvas.h"

#include <easy3d/core/graph.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/core/manifold_builder.h>
#include <easy3d/algo/surface_mesh_components.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/walk_through.h>
#include <easy3d/util/file_system.h>

#include <QMenu>
#include <QColorDialog>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QMouseEvent>



using namespace easy3d;


class ModelItem : public QTreeWidgetItem {
public:
    ModelItem(QTreeWidget *parent) : QTreeWidgetItem(parent) {
        for (int i = 0; i < parent->columnCount(); ++i)
            QTreeWidgetItem::setTextAlignment(i, Qt::AlignLeft);
    }

    ~ModelItem() {}

    Model *model() { return model_; }

    void setModel(Model *model) { model_ = model; }

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
        }
        else if (dynamic_cast<PolyMesh*>(model())) {
            static QIcon iconTetrahedra(QString::fromUtf8(":/resources/icons/tetrahedra.png"));
            QTreeWidgetItem::setIcon(column, iconTetrahedra);
        }
    }

    void setVisibilityIcon(int column, bool visible) {
        static QIcon iconShow(QString::fromUtf8(":/resources/icons/show.png"));
        static QIcon iconHide(QString::fromUtf8(":/resources/icons/hide.png"));

        if (visible)
            QTreeWidgetItem::setIcon(column, iconShow);
        else {
            QTreeWidgetItem::setIcon(column, iconHide);
        }
    }

    void setStatus(bool is_active) {
        for (int i = 0; i < columnCount(); ++i) {
            if (is_active)
                QTreeWidgetItem::setBackground(i, QColor(255, 177, 255));
            else
                QTreeWidgetItem::setBackground(i, QBrush());
        }
    }

private:
    Model *model_;
};


WidgetModelList::WidgetModelList(QWidget *parent)
        : QTreeWidget(parent), mainWindow_(nullptr), popupMenu_(nullptr), auto_focus_(false), selected_only_(false)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(modelItemSelectionChanged()));
    connect(this, SIGNAL(itemPressed(QTreeWidgetItem * , int)), this, SLOT(modelItemPressed(QTreeWidgetItem * , int)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    if (selected_only_)
        setSelectionMode(QAbstractItemView::SingleSelection);
    else
        setSelectionMode(QAbstractItemView::ExtendedSelection);
}


WidgetModelList::~WidgetModelList() {
}


void WidgetModelList::init(MainWindow *w) {
    mainWindow_ = w;

    QStringList headerLabels;
    headerLabels << "Index" << "Type" << "Show" << "Name";
    setHeaderLabels(headerLabels);
    for (int i = 0; i < columnCount(); ++i)
        headerItem()->setTextAlignment(i, Qt::AlignLeft);

    header()->setDefaultSectionSize(60);

//    setColumnWidth(0, 50);
    setIndentation(10);

    setSelectionMode(QAbstractItemView::ExtendedSelection);
}


PaintCanvas *WidgetModelList::viewer() {
    return mainWindow_->viewer();
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
    disconnect(this, SIGNAL(itemPressed(QTreeWidgetItem * , int)), this, SLOT(modelItemPressed(QTreeWidgetItem * , int)));
    disconnect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    Model *active_model = viewer()->currentModel();

    const std::vector<Model *> &models = viewer()->models();
    for (unsigned int i = 0; i < models.size(); ++i) {
        Model *model = models[i];
        const std::string &name = file_system::base_name(model->name());
        ModelItem *item = dynamic_cast<ModelItem *>(this->topLevelItem(i));
        if (!item) {
            item = new ModelItem(this);
            addTopLevelItem(item);
        }
        item->setModel(model);

        item->setData(0, Qt::DisplayRole, i + 1);
        item->setIcon(1);
        item->setVisibilityIcon(2, model->renderer()->is_visible());

        item->setData(3, Qt::DisplayRole, QString::fromStdString(name));
		item->setStatus(model == active_model);

        if (model == active_model)
            setCurrentItem(item);
    }

    // remove redundant items
    if (topLevelItemCount() > models.size()) {
        int delta = int(topLevelItemCount() - models.size());
        for (int i = 0; i < delta; ++i) {
            int idx = topLevelItemCount() - 1;
            if (idx >= 0) {
                QTreeWidgetItem *item = takeTopLevelItem(idx);
                delete item;
            } else
                LOG(FATAL) << "should not have reached here";
        }
    }

    assert(topLevelItemCount() == models.size());

    if (viewer()->currentModel()) {
        const std::string &name = viewer()->currentModel()->name();
        if (!name.empty()) {
            mainWindow_->setCurrentFile(QString::fromStdString(name));
        }
    } else
        mainWindow_->setCurrentFile(QString());

    connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(modelItemSelectionChanged()));
    connect(this, SIGNAL(itemPressed(QTreeWidgetItem * , int)), this, SLOT(modelItemPressed(QTreeWidgetItem * , int)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
}


void WidgetModelList::duplicateCurrent() {
    if (viewer()->currentModel()) {
        Model* model = viewer()->currentModel();

        Model* copy = nullptr;
        if (dynamic_cast<SurfaceMesh*>(model)) {
            copy = new SurfaceMesh(*dynamic_cast<SurfaceMesh*>(model));
        }
        else if (dynamic_cast<PointCloud*>(model)) {
            copy = new PointCloud(*dynamic_cast<PointCloud*>(model));
        }
        else if (dynamic_cast<Graph*>(model)) {
            copy = new Graph(*dynamic_cast<Graph*>(model));
        }
        else if (dynamic_cast<PolyMesh*>(model)) {
            copy = new PolyMesh(*dynamic_cast<PolyMesh*>(model));
        }

        if (copy) {
            const std::string &name = file_system::name_less_extension(model->name()) + "_copy";
            copy->set_name(name);
            viewer()->addModel(copy);
            addModel(copy, true);
        }
    }
}


void WidgetModelList::hideOtherModels(Model *cur) {
    const std::vector<Model *> &models = viewer()->models();
    for (int i = 0; i < models.size(); ++i) {
        Model *model = models[i];
        if (selected_only_ && model != cur)
            model->renderer()->set_visible(false);
        else
            model->renderer()->set_visible(true);
    }
}


void WidgetModelList::showSelected() {
    if (selectedItems().empty())
        return;

	const QList<QTreeWidgetItem*>& items = selectedItems();
	for (int i = 0; i < items.size(); ++i) {
		ModelItem* item = dynamic_cast<ModelItem*>(items[i]);
		Model* model = item->model();
		model->renderer()->set_visible(true);
	}

	updateModelList();
    viewer()->update();
}


void WidgetModelList::hideSelected() {
    if (selectedItems().empty())
        return;

	const QList<QTreeWidgetItem*>& items = selectedItems();
	for (int i = 0; i < items.size(); ++i) {
		ModelItem* item = dynamic_cast<ModelItem*>(items[i]);
		Model* model = item->model();
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
			QTreeWidgetItem* item = topLevelItem(i);
			ModelItem* modelItem = dynamic_cast<ModelItem*>(item);
			visible[i] = (!modelItem->model()->renderer()->is_visible());
		}
		mainWindow_->setShowSelectedOnly(false);
		for (int i = 0; i < num; ++i) {
			QTreeWidgetItem* item = topLevelItem(i);
			ModelItem* modelItem = dynamic_cast<ModelItem*>(item);
			modelItem->model()->renderer()->set_visible(visible[i]);
		}
	}
	else {
		for (int i = 0; i < num; ++i) {
			QTreeWidgetItem* item = topLevelItem(i);
			ModelItem* modelItem = dynamic_cast<ModelItem*>(item);
			Model* model = modelItem->model();
			model->renderer()->set_visible(!model->renderer()->is_visible());;
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
        for (int i = 0; i < models.size(); ++i) {
            models[i]->renderer()->set_visible(true);
        }
        updateModelList();
        viewer()->update();
    }
}


void WidgetModelList::decomposeSelected() {
    const QList<QTreeWidgetItem *> &items = selectedItems();
    for (int i = 0; i < items.size(); ++i) {
        ModelItem *item = dynamic_cast<ModelItem *>(items[i]);
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
	std::vector<Model*>	models;
	const QList<QTreeWidgetItem*>& items = selectedItems();
	for (int i = 0; i < items.size(); ++i) {
		ModelItem* item = dynamic_cast<ModelItem*>(items[i]);
		models.push_back(item->model());
	}

	mergeModels(models);
}


void WidgetModelList::deleteSelected() {
	const QList<QTreeWidgetItem*>& items = selectedItems();
	for (int i = 0; i < items.size(); ++i) {
		ModelItem* item = dynamic_cast<ModelItem*>(items[i]);
		if (item)
            viewer()->deleteModel(item->model());
	}

	Model* current_model = viewer()->currentModel();
	if (selected_only_)
		hideOtherModels(current_model);

    //	viewer()->configureManipulation();

    mainWindow_->updateUi();

	if (auto_focus_)
		viewer()->fitScreen();
	else
		viewer()->update();
}


void WidgetModelList::currentModelItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    if (current == previous)
        return;

	ModelItem* current_item = dynamic_cast<ModelItem*>(current);
	ModelItem* previous_item = dynamic_cast<ModelItem*>(previous);
	if (!current_item)
		return;

    current_item->setStatus(true);
    if (previous_item)
        previous_item->setStatus(false);

	Model* model = current_item->model();
	viewer()->setCurrentModel(model);

	if (selected_only_)
		hideOtherModels(model);

    mainWindow_->updateUi();

    if (auto_focus_)
        viewer()->fitScreen(model);
    else
        viewer()->update();
}


void WidgetModelList::modelItemSelectionChanged() {
    Model *active_model = viewer()->currentModel();

    int num = topLevelItemCount();
    for (int i = 0; i < num; ++i) {
        ModelItem *item = dynamic_cast<ModelItem *>(topLevelItem(i));
        item->setStatus(item->model() == active_model);

        // don't allow changing selection for camera path creation
        if (viewer()->walkThrough()->status() == easy3d::WalkThrough::STOPPED)
            item->model()->renderer()->set_selected(item->isSelected());
    }

    viewer()->update();
    update();
}


void WidgetModelList::modelItemPressed(QTreeWidgetItem *current, int column) {
    ModelItem *current_item = dynamic_cast<ModelItem *>(current);
    if (!current_item)
        return;

    int num = topLevelItemCount();
    for (int i = 0; i < num; ++i) {
        ModelItem *item = dynamic_cast<ModelItem *>(topLevelItem(i));
        item->setStatus(item == current_item);
        item->setSelected(item->model()->renderer()->is_selected());
    }
    viewer()->setCurrentModel(current_item->model());

    if (column == 2 && !selected_only_) {
        Model *model = current_item->model();
        bool visible = !model->renderer()->is_visible();
        current_item->setVisibilityIcon(2, visible);
        model->renderer()->set_visible(visible);
    }

    if (auto_focus_)
        viewer()->fitScreen(current_item->model());

    viewer()->update();
    mainWindow_->updateUi();
}


void WidgetModelList::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        QTreeWidget::mousePressEvent(e);

        if (!selected_only_ && itemAt(e->pos()))
            itemAt(e->pos())->setSelected(true);
    }
}


void WidgetModelList::mouseReleaseEvent(QMouseEvent* e) {
    //if (e->button() == Qt::LeftButton)
    QTreeWidget::mouseReleaseEvent(e);
}


void WidgetModelList::setAutoFocus(bool b) {
    auto_focus_ = b;
    if (auto_focus_)
        viewer()->fitScreen(viewer()->currentModel());
    else
        viewer()->update();
}


void WidgetModelList::setSelectedOnly(bool b) {
	selected_only_ = b;
	if (selected_only_) {
        setSelectionMode(QAbstractItemView::SingleSelection);
        connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem * , QTreeWidgetItem * )), this,
                SLOT(currentModelItemChanged(QTreeWidgetItem * , QTreeWidgetItem * )));
    }
	else {
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        disconnect(this, SIGNAL(currentItemChanged(QTreeWidgetItem * , QTreeWidgetItem * )), this,
                SLOT(currentModelItemChanged(QTreeWidgetItem * , QTreeWidgetItem * )));
    }

	ModelItem* item = dynamic_cast<ModelItem*>(currentItem());
	if (!item)
		return;

	Model* active_model = item->model();
	if (selected_only_)
		hideOtherModels(active_model);
	else {
		const std::vector<Model*>& models = viewer()->models();
		for (int i = 0; i < models.size(); ++i) {
			models[i]->renderer()->set_visible(true);
		}
	}

    updateModelList();
    viewer()->update();
}


void WidgetModelList::addModel(Model *model, bool make_current) {
    if (!model)
        return;

    if (make_current) {
        mainWindow_->enableCameraManipulation();
        viewer()->setCurrentModel(model);
    }

    Model *current_model = viewer()->currentModel();
    if (selected_only_)
        hideOtherModels(current_model);

    mainWindow_->updateUi();

    viewer()->update();
}


void WidgetModelList::deleteModel(Model *model, bool fit) {
    viewer()->deleteModel(model);
    Model *active_model = viewer()->currentModel();
    if (!active_model)  // no model exists
        return;

    if (selected_only_)
        hideOtherModels(active_model);

    mainWindow_->updateUi();
    //viewer()->configureManipulation();

    if (fit)
        viewer()->fitScreen();
    else
        viewer()->update();
}


void WidgetModelList::mergeModels(const std::vector<Model *> &models) {
    std::vector<SurfaceMesh *> meshes;
    std::vector<PointCloud *> clouds;

    std::vector<Model *> selectedModels;
    for (std::size_t i = 0; i < models.size(); ++i) {
        Model *m = models[i];
        if (dynamic_cast<SurfaceMesh *>(m)) {
            SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(m);
            meshes.push_back(mesh);
        } else if (dynamic_cast<PointCloud *>(m)) {
            PointCloud *pset = dynamic_cast<PointCloud *>(m);
            clouds.push_back(pset);
        }
    }

    std::vector<Model *> to_delete;

	if (meshes.size() > 1) {
		SurfaceMesh* to = meshes[0];
		ProgressLogger logger(meshes.size() - 1, false, false);
		for (std::size_t i = 1; i < meshes.size(); ++i) {
		    logger.notify(i);
			if (logger.is_canceled())
				break;

            SurfaceMesh* from = meshes[i];
            to->join(*from);

			to_delete.push_back(from);
		}

		to->set_name("merged_mesh");
		to->renderer()->update();
	}

	if (clouds.size() > 1) {
		PointCloud* to = clouds[0];
		ProgressLogger logger(clouds.size() - 1, false, false);
		for (int i = 1; i < clouds.size(); ++i) {
            logger.notify(i);
            if (logger.is_canceled())
                break;

			PointCloud* from = clouds[i];
            auto points = from->get_vertex_property<vec3>("v:point");
            for (auto v : from->vertices()) {
                to->add_vertex(points[v]);
            }

			to_delete.push_back(from);
		}

		to->set_name("merged_point_set");
        to->renderer()->update();
	}

	for (unsigned int i = 0; i < to_delete.size(); ++i) {
		Model* model = to_delete[i];
		viewer()->deleteModel(model);
	}

	// update display and ui
	if (meshes.size() > 1 || clouds.size() > 1) {
//		viewer()->configureManipulation();
		updateModelList();
		mainWindow_->updateUi();
        viewer()->update();
	}
}


void WidgetModelList::decomposeModel(Model *model) {
    SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(model);
    if (!mesh)
        return;

	const auto& components = SurfaceMeshComponent::extract(mesh);
	if (components.size() == 1) {
		LOG(WARNING) << "model has only one component";
		return;
	}

	const std::string base_name = file_system::parent_directory(mesh->name()) + "/" + file_system::base_name(mesh->name()) + "_part_";

	ProgressLogger progress(components.size(), false, false);
	for (unsigned int i = 0; i < components.size(); i++) {
        if (progress.is_canceled()) {
            LOG(WARNING) << "decomposing model cancelled";
            return;
        }

		SurfaceMesh* new_mesh = components[i].to_mesh();
		new_mesh->set_name(base_name + std::to_string(i + 1));
		viewer()->addModel(new_mesh);
		progress.next();
	}

    // delete the original model
    viewer()->deleteModel(mesh);
    updateModelList();
    mainWindow_->updateUi();
    viewer()->update();

	LOG(INFO) << "model decomposed into " << components.size() << " parts" << std::endl;
}
