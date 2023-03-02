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


#include "main_window.h"

#include <string>
#include <iostream>

#include <QMutex>
#include <QFileDialog>
#include <QDropEvent>
#include <QMimeData>
#include <QSettings>
#include <QMessageBox>
#include <QColorDialog>
#include <QCoreApplication>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/core/random.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/poly_mesh_io.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/fileio/point_cloud_io_ptx.h>
#include <easy3d/fileio/translator.h>
#include <easy3d/algo/point_cloud_normals.h>
#include <easy3d/algo/surface_mesh_components.h>
#include <easy3d/algo/surface_mesh_topology.h>
#include <easy3d/algo/surface_mesh_triangulation.h>
#include <easy3d/algo/surface_mesh_tetrahedralization.h>
#include <easy3d/algo/surface_mesh_subdivision.h>
#include <easy3d/algo/surface_mesh_geodesic.h>
#include <easy3d/algo/surface_mesh_stitching.h>
#include <easy3d/algo/surface_mesh_enumerator.h>
#include <easy3d/algo/surface_mesh_polygonization.h>
#include <easy3d/algo/surface_mesh_geometry.h>
#include <easy3d/algo_ext/surfacer.h>
#include <easy3d/algo/delaunay_2d.h>
#include <easy3d/algo/delaunay_3d.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/stop_watch.h>
#include <easy3d/util/line_stream.h>
#include <easy3d/util/version.h>

#include "paint_canvas.h"
#include "walk_through.h"

#include "dialogs/dialog_snapshot.h"
#include "dialogs/dialog_properties.h"
#include "dialogs/dialog_poisson_reconstruction.h"
#include "dialogs/dialog_surface_mesh_curvature.h"
#include "dialogs/dialog_surface_mesh_sampling.h"
#include "dialogs/dialog_point_cloud_normal_estimation.h"
#include "dialogs/dialog_point_cloud_ransac_primitive_extraction.h"
#include "dialogs/dialog_point_cloud_simplification.h"
#include "dialogs/dialog_gaussian_noise.h"
#include "dialogs/dialog_surface_mesh_fairing.h"
#include "dialogs/dialog_surface_mesh_from_text.h"
#include "dialogs/dialog_surface_mesh_hole_filling.h"
#include "dialogs/dialog_surface_mesh_parameterization.h"
#include "dialogs/dialog_surface_mesh_remeshing.h"
#include "dialogs/dialog_surface_mesh_smoothing.h"
#include "dialogs/dialog_surface_mesh_simplification.h"
#include "dialogs/dialog_walk_through.h"

#include "widgets/widget_global_setting.h"
#include "widgets/widget_drawable_points.h"
#include "widgets/widget_drawable_lines.h"
#include "widgets/widget_drawable_triangles.h"

#include <ui_main_window.h>


using namespace easy3d;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->treeWidgetModels->init(this);

    viewer_ = new PaintCanvas(this);
    setCentralWidget(viewer_);

    // ----- the width of the rendering panel ------
    // sizeHint() doesn't suggest a good value
    // const QSize& size = ui->dockWidgetRendering->sizeHint();
    const int width = 270; //static_cast<int>(size.width() * 1.5f);
    ui->dockWidgetRendering->setFixedWidth(width);

    // ------ rendering panel ------

    widgetTrianglesDrawable_ = new WidgetTrianglesDrawable(this);
    ui->verticalLayoutTrianglesDrawable->addWidget(widgetTrianglesDrawable_);
    widgetTrianglesDrawable_->setEnabled(false);

    widgetLinesDrawable_ = new WidgetLinesDrawable(this);
    ui->verticalLayoutLinesDrawable->addWidget(widgetLinesDrawable_);
    widgetLinesDrawable_->setEnabled(false);

    widgetPointsDrawable_ = new WidgetPointsDrawable(this);
    ui->verticalLayoutPointsDrawable->addWidget(widgetPointsDrawable_);
    widgetPointsDrawable_->setEnabled(false);

    auto widgetGlobalSetting = new WidgetGlobalSetting(this);
    ui->verticalLayoutGlobalSetting->addWidget(widgetGlobalSetting);

    // communication between widgets
    widgetGlobalSetting->widgetTrianglesDrawable_ = widgetTrianglesDrawable_;

    // ---------------------------

    // file menu
    createActionsForFileMenu();

    // view menu
    createActionsForViewMenu();

    // view menu
    createActionsForCameraMenu();

    // edit menu
    createActionsForEditMenu();

    // property menu
    createActionsForPropertyMenu();

    // select menu
    createActionsForSelectMenu();

    // point cloud menu
    createActionsForPointCloudMenu();

    // surface mesh menu
    createActionsForSurfaceMeshMenu();

    // polyhedral mesh menu
    createActionsForPolyMeshMenu();

    // status bar
    createStatusBar();

    // about menu
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));
    connect(ui->actionManual, SIGNAL(triggered()), this, SLOT(showManual()));
//    QAction *actionAboutQt = ui->menuHelp->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
//    actionAboutQt->setStatusTip(tr("Show the Qt library's About box"));

    // options for the model panel
    connect(ui->checkBoxAutoFocus, SIGNAL(toggled(bool)), ui->treeWidgetModels, SLOT(setAutoFocus(bool)));
    connect(ui->checkBoxSelectedOnly, SIGNAL(toggled(bool)), ui->treeWidgetModels, SLOT(setSelectedOnly(bool)));
    connect(ui->pushButtonApplyToAllModels, SIGNAL(clicked()), ui->treeWidgetModels, SLOT(applyRenderingToAllModels()));

#if 0
    auto actionTest = ui->mainToolBar->addAction("Test");
    connect(actionTest, &QAction::triggered, [this]() -> void {
        Model* m = viewer_->currentModel();
        if (!m)
            return;

        // process the model

        viewer_->makeCurrent();
        // update the opengl buffers if needed.
        viewer_->doneCurrent();
        viewer_->update();
        updateUi();
    });
#endif

    setWindowIcon(QIcon(QString::fromStdString(":/resources/icons/Mapple.png")));
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops(true);

#ifdef NDEBUG
    setWindowState(Qt::WindowMaximized);
#else
    setBaseSize(1024, 800);
#endif

    readSettings();
    updateWindowTitle();
}


MainWindow::~MainWindow() {
    LOG(INFO) << "Mapple terminated. Bye!";
}


void MainWindow::notify(std::size_t percent, bool update_viewer) {
    progress_bar_->setValue(int(percent));
    cancelTaskButton_->setVisible(percent > 0 && percent < 100);
    progress_bar_->setVisible(percent > 0 && percent < 100);

    if (update_viewer)
        viewer_->update();

    // Force updating UI.
    // This approach has significant drawbacks. For example, imagine you wanted to perform two such loops
    // in parallel-calling one of them would effectively halt the other until the first one is finished
    // (so you can't distribute computing power among different tasks). It also makes the application react
    // with delays to events. Furthermore, the code is difficult to read and analyze, therefore this solution
    // is only suited for short and simple problems that are to be processed in a single thread, such as
    // splash screens and the monitoring of short operations.
    QApplication::processEvents();
}


void MainWindow::send(el::Level level, const std::string &msg) {
    static QMutex mutex;
    mutex.lock();
	switch (level) {
        case el::Level::Info:
            ui->listWidgetLog->addItem(QString::fromStdString("[INFO] " + msg));
            // set to black will not work if it is dark mode
//            ui->listWidgetLog->item(ui->listWidgetLog->count() - 1)->setForeground(Qt::black);
            break;
        case el::Level::Warning:
            ui->listWidgetLog->addItem(QString::fromStdString("[WARNING] " + msg));
            ui->listWidgetLog->item(ui->listWidgetLog->count() - 1)->setForeground(Qt::darkRed);
            break;
        case el::Level::Error:
            ui->listWidgetLog->addItem(QString::fromStdString("[ERROR] " + msg));
            ui->listWidgetLog->item(ui->listWidgetLog->count() - 1)->setForeground(Qt::red);
            break;
        case el::Level::Fatal:  // no need to handle (app will crash)
        default: break;
    }

    ui->listWidgetLog->scrollToBottom();
	mutex.unlock();
}


void MainWindow::createStatusBar()
{
    labelStatusInfo_ = new QLabel("Ready", this);
    labelStatusInfo_->setFixedWidth(ui->dockWidgetRendering->width() * 2);
    labelStatusInfo_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusBar()->addWidget(labelStatusInfo_);

    //////////////////////////////////////////////////////////////////////////

    const int length = 100;
    labelNumFaces_ = new QLabel(this);
    labelNumFaces_->setMinimumWidth(length);
    labelNumFaces_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusBar()->addWidget(labelNumFaces_);

    labelNumVertices_ = new QLabel(this);
    labelNumVertices_->setMinimumWidth(length);
    labelNumVertices_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusBar()->addWidget(labelNumVertices_);

    labelNumEdges_ = new QLabel(this);
    labelNumEdges_->setMinimumWidth(length);
    labelNumEdges_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusBar()->addWidget(labelNumEdges_);

    labelNumCells_ = new QLabel(this);
    labelNumCells_->setMinimumWidth(length);
    labelNumCells_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusBar()->addWidget(labelNumCells_);

    //////////////////////////////////////////////////////////////////////////

    cancelTaskButton_ = new QPushButton(this);
    cancelTaskButton_->setVisible(false);
    cancelTaskButton_->setFlat(true);
    cancelTaskButton_->setIcon(QIcon(":/resources/icons/cancel.png"));
    cancelTaskButton_->setFixedSize(30, 30);
    statusBar()->addPermanentWidget(cancelTaskButton_, 1);
    connect(cancelTaskButton_, SIGNAL(pressed()), this,  SLOT(cancelTask()));

    progress_bar_ = new QProgressBar(this);
    progress_bar_->setVisible(false);
    progress_bar_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    progress_bar_->setFixedWidth(ui->dockWidgetModels->width());
    statusBar()->addPermanentWidget(progress_bar_);

    //////////////////////////////////////////////////////////////////////////

    updateStatusBar();
}


void MainWindow::updateStatusBar()
{
    QString faces(""), vertices(""), edges(""), cells("");

    Model* model = viewer_->currentModel();
    if (dynamic_cast<SurfaceMesh*>(model)) {
        auto mesh = dynamic_cast<SurfaceMesh*>(model);
        faces = QString("#faces: %1  ").arg(mesh->n_faces());
        vertices = QString("#vertices: %1  ").arg(mesh->n_vertices());
        edges = QString("#edges: %1  ").arg(mesh->n_edges());
        labelNumFaces_->setVisible(true);
        labelNumEdges_->setVisible(true);
        labelNumCells_->setVisible(false);
    }

    else if (dynamic_cast<PointCloud*>(model)) {
        auto cloud = dynamic_cast<PointCloud*>(model);
        vertices = QString("#vertices: %1  ").arg(cloud->n_vertices());
        labelNumFaces_->setVisible(false);
        labelNumEdges_->setVisible(false);
        labelNumCells_->setVisible(false);
    }

    else if (dynamic_cast<Graph*>(model)) {
        auto graph = dynamic_cast<Graph*>(model);
        vertices = QString("#vertices: %1  ").arg(graph->n_vertices());
        edges = QString("#edges: %1  ").arg(graph->n_edges());
        labelNumFaces_->setVisible(false);
        labelNumEdges_->setVisible(true);
        labelNumCells_->setVisible(false);
    }

    else if (dynamic_cast<PolyMesh*>(model)) {
        auto mesh = dynamic_cast<PolyMesh*>(model);
        faces = QString("#faces: %1  ").arg(mesh->n_faces());
        vertices = QString("#vertices: %1  ").arg(mesh->n_vertices());
        edges = QString("#edges: %1  ").arg(mesh->n_edges());
        cells = QString("#cells: %1  ").arg(mesh->n_cells());
        labelNumFaces_->setVisible(true);
        labelNumEdges_->setVisible(true);
        labelNumCells_->setVisible(true);
    }


    labelNumVertices_->setText( vertices );
    labelNumFaces_->setText( faces );
    labelNumCells_->setText( cells );
    labelNumEdges_->setText( edges );
}


void MainWindow::cancelTask() {
    cancel();
    cancelTaskButton_->setVisible(false);
    progress_bar_->reset();
    progress_bar_->setTextVisible(false);
    progress_bar_->setVisible(false);
    viewer_->update();
}


void MainWindow::dragEnterEvent(QDragEnterEvent *e) {
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}


void MainWindow::dropEvent(QDropEvent *e) {
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();

    int count = 0;
    foreach (const QUrl &url, e->mimeData()->urls()) {
        const QString &fileName = url.toLocalFile();
        if (open(fileName.toStdString()))
            ++count;
    }

    if (count > 0)
        viewer_->update();
}


int MainWindow::openFiles(const QStringList &fileNames) {
    int count = 0;
    ProgressLogger progress(fileNames.size(), true, false);
    for (const auto& name : fileNames) {
        if (progress.is_canceled()) {
            LOG(WARNING) << "opening files cancelled";
            break;
        }
        if (open(name.toStdString()))
            ++count;
        progress.next();
    }

    return count > 0;
}


void MainWindow::loadModelTranslateChanged(QAction* act) {
    if (act == ui->actionTranslateDisabled) {
        Translator::instance()->set_status(Translator::DISABLED);
        LOG(INFO) << "translation in file IO has been disabled";
    } else if (act == ui->actionTranslateUseFirstVertex) {
        Translator::instance()->set_status(Translator::TRANSLATE_USE_FIRST_POINT);
        LOG(INFO) << "translation with respect to first vertex in file IO";
    }
    else if (act == ui->actionTranslateUseLastKnownVertex) {
        Translator::instance()->set_status(Translator::TRANSLATE_USE_LAST_KNOWN_OFFSET);
        const dvec3 &origin = Translator::instance()->translation();
        LOG(INFO) << "translation with respect to last know vertex (" << origin << ") in file IO";
    }
}


bool MainWindow::onOpen() {
    const QStringList& fileNames = QFileDialog::getOpenFileNames(
                this,
                "Open file(s)",
                curDataDirectory_,
                "Supported formats (*.ply *.obj *.off *.stl *.sm *.geojson *.trilist *.bin *.las *.laz *.xyz *.bxyz *.vg *.bvg *.ptx *.plm *.pm *.mesh)\n"
                "Surface Mesh (*.ply *.obj *.off *.stl *.sm *.geojson *.trilist)\n"
                "Point Cloud (*.ply *.bin *.ptx *.las *.laz *.xyz *.bxyz *.vg *.bvg *.ptx)\n"
                "Polyhedral Mesh (*.plm *.pm *.mesh)\n"
                "Graph (*.ply)\n"
                "All formats (*.*)"
            );

    // Hide closed dialog
    QApplication::processEvents();

    if (fileNames.empty())
        return false;

    return openFiles(fileNames);
}


bool MainWindow::onSave() {
    const Model* model = viewer_->currentModel();
    if (!model) {
        std::cerr << "no model exists" << std::endl;
        return false;
    }

    std::string default_file_name = model->name();
    if (file_system::extension(default_file_name).empty()) // no extension?
        default_file_name += ".ply"; // default to ply

    const QString& fileName = QFileDialog::getSaveFileName(
                this,
                "Save file",
                QString::fromStdString(default_file_name),
                "Supported formats (*.ply *.obj *.off *.stl *.sm *.bin *.las *.laz *.xyz *.bxyz *.vg *.bvg *.plm *.pm *.mesh)\n"
                "Surface Mesh (*.ply *.obj *.off *.stl *.sm)\n"
                "Point Cloud (*.ply *.bin *.ptx *.las *.laz *.xyz *.bxyz *.vg *.bvg)\n"
                "Polyhedral Mesh (*.plm *.pm *.mesh)\n"
                "Graph (*.ply)\n"
                "All formats (*.*)"
    );

    if (fileName.isEmpty())
        return false;

    bool saved = false;
    if (dynamic_cast<const PointCloud*>(model)) {
        const auto cloud = dynamic_cast<const PointCloud*>(model);
        saved = PointCloudIO::save(fileName.toStdString(), cloud);
    }
    else if (dynamic_cast<const SurfaceMesh*>(model)) {
        const auto mesh = dynamic_cast<const SurfaceMesh*>(model);
        saved = SurfaceMeshIO::save(fileName.toStdString(), mesh);
    }
    else if (dynamic_cast<const Graph*>(model)) {
        const auto graph = dynamic_cast<const Graph*>(model);
        saved = GraphIO::save(fileName.toStdString(), graph);
    }
    else if (dynamic_cast<const PolyMesh*>(model)) {
        const auto mesh = dynamic_cast<const PolyMesh*>(model);
        saved = PolyMeshIO::save(fileName.toStdString(), mesh);
    }

    if (saved) {
        LOG(INFO) << "model successfully saved to: " << fileName.toStdString();
        setCurrentFile(fileName);
        return true;
    }

    return false;
}


Model* MainWindow::open(const std::string& file_name) {
    auto models = viewer_->models();
    for (auto m : models) {
        if (m->name() == file_name) {
            LOG(WARNING) << "model already loaded: " << file_name;
            return nullptr;
        }
    }

    const std::string& ext = file_system::extension(file_name, true);
    bool is_ply_mesh = false;
    if (ext == "ply")
        is_ply_mesh = (io::PlyReader::num_instances(file_name, "face") > 0);

    Model* model = nullptr;
    if ((ext == "ply" && is_ply_mesh) || ext == "obj" || ext == "off" || ext == "stl" || ext == "sm" || ext == "geojson" || ext == "trilist") { // mesh
        model = SurfaceMeshIO::load(file_name);
    }
    else if (ext == "ply" && io::PlyReader::num_instances(file_name, "edge") > 0) {
        model = GraphIO::load(file_name);
    } else if (ext == "plm" || ext == "pm" || ext == "mesh") {
        model = PolyMeshIO::load(file_name);
    }
    else { // point cloud
        if (ext == "ptx") {
            io::PointCloudIO_ptx serializer(file_name);
            PointCloud* cloud = nullptr;
            while ((cloud = serializer.load_next())) {
                viewer_->addModel(cloud);
                ui->treeWidgetModels->addModel(cloud, true);
            }
        }
        else
            model = PointCloudIO::load(file_name);
    }

    if (model) {
        model->set_name(file_name);
        viewer_->addModel(model);
        ui->treeWidgetModels->addModel(model, true);
        setCurrentFile(QString::fromStdString(file_name));

        const auto keyframe_file = file_system::replace_extension(model->name(), "kf");
        if (file_system::is_file(keyframe_file)) {
            if (viewer_->walkThrough()->interpolator()->read_keyframes(keyframe_file)) {
                LOG(INFO) << "model has an accompanying animation file \'"
                          << file_system::simple_name(keyframe_file) << "\' (loaded)";
                viewer_->walkThrough()->set_scene({model});
            }
        }
    }

    return model;
}


void MainWindow::generateColorPropertyFromIndexedColors() {
    const QString& fileName = QFileDialog::getOpenFileName(
            this,
            "Open file",
            curDataDirectory_,
            "Indexed colors (*.ic)\n"
            "All formats (*.*)"
    );
    if (fileName.isEmpty())
        return;
    // Hide closed dialog
    QApplication::processEvents();

    std::ifstream input(fileName.toStdString().c_str());
    if (input.fail())
        return;

    // I want to handle skip lines and comments starting with '#'
    auto get_line = [](io::LineInputStream &in) -> void {
        in.get_line();
        const char *p = in.current_line().c_str();
        while (!in.eof() && (
                (strlen(p) == 0 || !isprint(*p)) || // empty line
                (strlen(p) > 0 && p[0] == '#')
        )) {
            in.get_line();
            p = in.current_line().c_str();
        }
    };

    std::unordered_map<int, vec3> indexed_colors;
    io::LineInputStream line_input(input);
    int location = -1; // 0: vertex, 1: face
    std::string attribute_name;
    while (!line_input.eof()) {
        get_line(line_input);
        const std::string current_line = line_input.current_line();
        if (current_line.empty())
            continue;
        if (current_line[0] == 'v' || current_line[1] == ':') {
            location = 0;
            line_input >> attribute_name;
        }
        else if (current_line[0] == 'f' || current_line[1] == ':') {
            location = 1;
            line_input >> attribute_name;
        }
        else {
            int index;
            vec3 color;
            line_input >> index >> color;
            if (!line_input.fail())
                indexed_colors[index] = color / 255.0f;
        }
    }

    if (location == 0) {
        if (dynamic_cast<PointCloud*>(viewer_->currentModel())) {
            auto cloud = dynamic_cast<PointCloud*>(viewer_->currentModel());
            auto indices = cloud->get_vertex_property<int>(attribute_name);
            if (indices) {
                const std::string color_name = "v:color_indexed";
                auto colors = cloud->vertex_property<vec3>(color_name);
                for (auto v : cloud->vertices()) {
                    colors[v] = indexed_colors[indices[v]];
                }
                updateUi();
                LOG(INFO) << "color property '" << color_name << "' has been generated";
            }
        }
        else if (dynamic_cast<SurfaceMesh*>(viewer_->currentModel())) {
            LOG(INFO) << "applying indexed colors on vertices";
            LOG(WARNING) << "not implemented yet... Please remind Liangliang to implement it";
        }
    }
    else if (location == 1) {
        LOG(INFO) << "applying indexed colors on faces";
        LOG(WARNING) << "not implemented yet... Please remind Liangliang to implement it";
    }
    else
        LOG(WARNING) << "unknown location of the indexed colors";
}


void MainWindow::updateUi() {
    const Model* model = viewer_->currentModel();
    if (model) {
        const std::string& name = model->name();
        setCurrentFile(QString::fromStdString(name));
    } else
        updateWindowTitle();

    ui->treeWidgetModels->updateModelList();
    updateRenderingPanel();
    updateStatusBar();
}


void MainWindow::updateRenderingPanel() {
    widgetTrianglesDrawable_->updatePanel();
    widgetLinesDrawable_->updatePanel();
    widgetPointsDrawable_->updatePanel();
}


void MainWindow::activeDrawableChanged(Drawable* d) {
    if (!d)
        return;

    switch (d->type()) {
        case Drawable::DT_POINTS:
            ui->toolBox->setCurrentWidget(ui->toolBoxPointsDrawable);
            dynamic_cast<WidgetDrawable*>(widgetPointsDrawable_)->setActiveDrawable(d);
            break;
        case Drawable::DT_LINES:
            ui->toolBox->setCurrentWidget(ui->toolBoxLinesDrawable);
            dynamic_cast<WidgetDrawable*>(widgetLinesDrawable_)->setActiveDrawable(d);
            break;
        case Drawable::DT_TRIANGLES:
            ui->toolBox->setCurrentWidget(ui->toolBoxTrianglesDrawable);
            dynamic_cast<WidgetDrawable*>(widgetTrianglesDrawable_)->setActiveDrawable(d);
            break;
    }
}


void MainWindow::setCurrentFile(const QString &fileName)
{
    QString dir = fileName.left(fileName.lastIndexOf("/"));
    if (!dir.isEmpty() && file_system::is_directory(dir.toStdString()))
        curDataDirectory_ = dir;

    setWindowModified(false);

    if (!fileName.isEmpty()) {
        recentFiles_.removeAll(fileName);
        recentFiles_.prepend(fileName);
        updateRecentFileActions();
    }

    updateWindowTitle();
}


void MainWindow::enableCameraManipulation() {
    ui->actionCameraManipulation->trigger();
}


void MainWindow::setShowSelectedOnly(bool b) {
    ui->checkBoxSelectedOnly->setChecked(b);
}


WidgetModelList* MainWindow::widgetModelList() const {
    return ui->treeWidgetModels;
}


void MainWindow::onOpenRecentFile() {
    if (okToContinue()) {
        auto action = qobject_cast<QAction *>(sender());
        if (action) {
            const QString filename(action->data().toString());
            if (open(filename.toStdString()))
                viewer_->update();
        }
    }
}


void MainWindow::onClearRecentFiles() {
    recentFiles_.clear();
    updateRecentFileActions();
}


void MainWindow::saveSnapshot() {
    DialogSnapshot dialog(this);
    if (dialog.exec() == QDialog::Accepted)
        dialog.saveSnapshot();
}


void MainWindow::setBackgroundColor() {
    const vec4& c = viewer_->backGroundColor();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255), static_cast<int>(c.a * 255));
    const QColor& color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec4 newColor(
                static_cast<float>(color.redF()),
                static_cast<float>(color.greenF()),
                static_cast<float>(color.blueF()),
                static_cast<float>(color.alphaF())
                );
        viewer_->setBackgroundColor(newColor);
        viewer_->update();
    }
}


void MainWindow::saveCameraStateToFile() {
    QString suggested_name = curDataDirectory_;
    if (viewer()->currentModel()) {
        const std::string name = file_system::replace_extension(viewer()->currentModel()->name(), "view");
        suggested_name = QString::fromStdString(name);
    }
    const QString fileName = QFileDialog::getSaveFileName(
            this,
            "Save viewer state to file",
            suggested_name,
            "Viewer state (*.view)\n"
            "All formats (*.*)"
    );

    if (fileName.isEmpty())
        return;

    std::ofstream output(fileName.toStdString().c_str());
    if (output.fail()) {
        QMessageBox::warning(window(), tr("Save state to file error"), tr("Unable to create file %1").arg(fileName));
        return;
    }

    viewer_->saveState(output);
    // assume the user will soon restore the state from this file.
    curDataDirectory_ = fileName.left(fileName.lastIndexOf("/"));
}


void MainWindow::restoreCameraStateFromFile() {
    const QString fileName = QFileDialog::getOpenFileName(
            this,
            "Restore viewer state from file",
            curDataDirectory_,
            "Viewer state (*.view)\n"
            "Viewer state - old deprecated version (*.state)\n"
            "All formats (*.*)"
    );

    if (fileName.isEmpty())
        return;

    // read the state from file
    std::ifstream input(fileName.toStdString().c_str());
    if (input.fail()) {
        QMessageBox::warning(this, tr("Read state file error"), tr("Unable to read file %1").arg(fileName));
        return;
    }

    viewer_->restoreState(input);
}


bool MainWindow::okToContinue()
{
    if (isWindowModified()) {
        int r = QMessageBox::warning(this, tr("Mapple"),
            tr("The model has been modified.\n"
            "Do you want to save your changes?"),
            QMessageBox::Yes | QMessageBox::Default,
            QMessageBox::No,
            QMessageBox::Cancel | QMessageBox::Escape);
        if (r == QMessageBox::Yes)
            return onSave();
        else if (r == QMessageBox::Cancel)
            return false;
    }
    return true;
}


void MainWindow::onAbout()
{
    QString title = QMessageBox::tr(
        R"(<p align="center"><span style="font-style:italic;">I'm good software, though I have defects.</span></p>)"
        );

    int bits = 64;
#if defined (ENV_32_BIT)
    bits = 32;
#endif

#ifndef NDEBUG
    title += QMessageBox::tr("<h3>Mapple (%1 bit) - Debug Version</h3>").arg(bits);
#else
    title += QMessageBox::tr("<h3>Mapple (%1 bit)</h3>").arg(bits);
#endif

    title += QMessageBox::tr("<h4>Version %1 (based on Qt v%2)</h4>").arg(version().c_str()).arg(QT_VERSION_STR);

    QString text = QMessageBox::tr(
            "<p>Mapple is software for processing and rendering 3D data (e.g., point clouds, graphs, surface meshes, "
            "and polyhedral meshes), and more.</p>"
            "<p>Liangliang Nan<br>"
            "<a href=\"mailto:liangliang.nan@gmail.com\">liangliang.nan@gmail.com</a><br>"
            "<a href=\"https://3d.bk.tudelft.nl/liangliang/\">https://3d.bk.tudelft.nl/liangliang/</a></p>"
    );

    //QMessageBox::about(this, title, text);
    QMessageBox::about(this, "About Mapple", title + text);
}


void MainWindow::showManual() {
    LOG(INFO) << viewer()->usage();
}


void MainWindow::readSettings()
{
    QSettings settings("liangliang.nan@gmail.com", "Mapple");
    recentFiles_ = settings.value("recentFiles").toStringList();
    updateRecentFileActions();
    curDataDirectory_ = settings.value("currentDirectory").toString();
}


void MainWindow::writeSettings()
{
    QSettings settings("liangliang.nan@gmail.com", "Mapple");
    settings.setValue("recentFiles", recentFiles_);
    if (!curDataDirectory_.isEmpty() && file_system::is_directory(curDataDirectory_.toStdString()))
        settings.setValue("currentDirectory", curDataDirectory_);
}


void MainWindow::updateWindowTitle() {
    Model* model = viewer_->currentModel();

#ifndef NDEBUG
    QString title = "Mapple (Debug Version)";
#else
    QString title = "Mapple";
#endif // NDEBUG

    QString fileName("Untitled");
    if (model)
        fileName = QString::fromStdString(model->name());

    title = tr("%1[*] - %2").arg(strippedName(fileName), title);
    setWindowTitle(title);
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (okToContinue()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}


void MainWindow::updateRecentFileActions()
{
    QMutableStringListIterator i(recentFiles_);
    while (i.hasNext()) {
        if (!QFile::exists(i.next()))
            i.remove();
    }

    for (int j = 0; j < MaxRecentFiles; ++j) {
        if (j < recentFiles_.count()) {
            QString text = tr("&%1 %2").arg(j + 1).arg(strippedName(recentFiles_[j]));
            actionsRecentFile[j]->setText(text);
            actionsRecentFile[j]->setData(recentFiles_[j]);
            actionsRecentFile[j]->setVisible(true);
        } else {
            actionsRecentFile[j]->setVisible(false);
        }
    }

    actionSeparator->setVisible(!recentFiles_.isEmpty());
}


QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}


void MainWindow::createActionsForFileMenu() {
    auto actionGroup = new QActionGroup(this);
    actionGroup->addAction(ui->actionTranslateDisabled);
    actionGroup->addAction(ui->actionTranslateUseFirstVertex);
    actionGroup->addAction(ui->actionTranslateUseLastKnownVertex);
    connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(loadModelTranslateChanged(QAction*)));

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(onOpen()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(onSave()));
    actionSeparator = ui->menuFile->addSeparator();

    QList<QAction*> actions;
    for (auto& action : actionsRecentFile) {
        action = new QAction(this);
        action->setVisible(false);
        connect(action, SIGNAL(triggered()), this, SLOT(onOpenRecentFile()));
        actions.push_back(action);
    }
    ui->menuRecentFiles->insertActions(ui->actionClearRecentFiles, actions);
    ui->menuRecentFiles->insertSeparator(ui->actionClearRecentFiles);
    connect(ui->actionClearRecentFiles, SIGNAL(triggered()), this, SLOT(onClearRecentFiles()));

    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    ui->actionExit->setShortcut(QString("Ctrl+Q"));
}


void MainWindow::createActionsForViewMenu() {
    connect(ui->actionShowPrimitiveIDUnderMouse, SIGNAL(toggled(bool)), viewer_, SLOT(showPrimitiveIDUnderMouse(bool)));
    connect(ui->actionShowPrimitivePropertyUnderMouse, SIGNAL(toggled(bool)), this, SLOT(showPrimitivePropertyUnderMouse(bool)));
    connect(ui->actionShowCoordinatesUnderMouse, SIGNAL(toggled(bool)), this, SLOT(showCoordinatesUnderMouse(bool)));

    connect(ui->actionShowEasy3DLogo, SIGNAL(toggled(bool)), viewer_, SLOT(showEasy3DLogo(bool)));
    connect(ui->actionShowFrameRate, SIGNAL(toggled(bool)), viewer_, SLOT(showFrameRate(bool)));
    connect(ui->actionShowAxes, SIGNAL(toggled(bool)), viewer_, SLOT(showAxes(bool)));

    connect(ui->actionShowCameraPath, SIGNAL(toggled(bool)), this, SLOT(setShowCameraPath(bool)));
    connect(ui->actionShowKeyframeCameras, SIGNAL(toggled(bool)), this, SLOT(setShowKeyframeCameras(bool)));

    QAction* actionToggleDockWidgetRendering = ui->dockWidgetRendering->toggleViewAction();
    actionToggleDockWidgetRendering->setText("Rendering Panel");
    ui->menuView->addAction(actionToggleDockWidgetRendering);

    QAction* actionToggleDockWidgetModels = ui->dockWidgetModels->toggleViewAction();
    actionToggleDockWidgetModels->setText("Model Panel");
    ui->menuView->addAction(actionToggleDockWidgetModels);

    QAction* actionToggleDockWidgetLogger = ui->dockWidgetLog->toggleViewAction();
    actionToggleDockWidgetLogger->setText("Log Panel");
    ui->menuView->addAction(actionToggleDockWidgetLogger);

    connect(ui->actionSetBackgroundColor, SIGNAL(triggered()), this, SLOT(setBackgroundColor()));
}


void MainWindow::createActionsForCameraMenu() {
    connect(ui->actionPerspectiveOrthographic, SIGNAL(toggled(bool)), viewer_, SLOT(setPerspective(bool)));
    connect(ui->actionFitScreen, SIGNAL(triggered()), viewer_, SLOT(fitScreen()));
    connect(ui->actionSnapshot, SIGNAL(triggered()), this, SLOT(saveSnapshot()));

    connect(ui->actionCopyCamera, SIGNAL(triggered()), viewer_, SLOT(copyCamera()));
    connect(ui->actionPasteCamera, SIGNAL(triggered()), viewer_, SLOT(pasteCamera()));

    connect(ui->actionSaveCameraStateToFile, SIGNAL(triggered()), this, SLOT(saveCameraStateToFile()));
    connect(ui->actionRestoreCameraStateFromFile, SIGNAL(triggered()), this, SLOT(restoreCameraStateFromFile()));

    connect(ui->actionImportCameraPath, SIGNAL(triggered()), this, SLOT(importCameraPath()));
    connect(ui->actionExportCameraPath, SIGNAL(triggered()), this, SLOT(exportCameraPath()));
    connect(ui->actionAnimation, SIGNAL(triggered()), this, SLOT(animation()));
}


void MainWindow::createActionsForPropertyMenu() {
    connect(ui->actionManipulateProperties, SIGNAL(triggered()), this, SLOT(manipulateProperties()));
    connect(ui->actionComputeHeightField, SIGNAL(triggered()), this, SLOT(computeHeightField()));
    connect(ui->actionComputeSurfaceMeshCurvatures, SIGNAL(triggered()), this, SLOT(computeSurfaceMeshCurvatures()));
    connect(ui->actionTopologyStatistics, SIGNAL(triggered()), this, SLOT(reportTopologyStatistics()));
}


void MainWindow::createActionsForEditMenu() {
    connect(ui->actionTranslationalRecenter, SIGNAL(triggered()), this, SLOT(translationalRecenter()));
    connect(ui->actionAddGaussianNoise, SIGNAL(triggered()), this, SLOT(addGaussianNoise()));
    connect(ui->actionApplyManipulatedTransformation, SIGNAL(triggered()), this, SLOT(applyManipulatedTransformation()));
    connect(ui->actionGiveUpManipulatedTransformation, SIGNAL(triggered()), this, SLOT(giveUpManipulatedTransformation()));

    connect(ui->actionGenerateColorPropertyFromIndexedColors, SIGNAL(triggered()), this, SLOT(generateColorPropertyFromIndexedColors()));
}


void MainWindow::createActionsForSelectMenu() {
    connect(ui->actionSelectModel, SIGNAL(toggled(bool)), viewer_, SLOT(enableSelectModel(bool)));

    connect(ui->actionInvertSelection, SIGNAL(triggered()), viewer_, SLOT(invertSelection()));
    connect(ui->actionDeleteSelectedPrimitives, SIGNAL(triggered()), viewer_, SLOT(deleteSelectedPrimitives()));

    //////////////////////////////////////////////////////////////////////////

    auto actionGroup = new QActionGroup(this);
    actionGroup->addAction(ui->actionCameraManipulation);
    actionGroup->addAction(ui->actionSelectClick);
    actionGroup->addAction(ui->actionSelectRect);
    actionGroup->addAction(ui->actionSelectLasso);

    connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(operationModeChanged(QAction*)));
}


void MainWindow::createActionsForPointCloudMenu() {
    connect(ui->actionDownSampling, SIGNAL(triggered()), this, SLOT(pointCloudDownsampling()));

    connect(ui->actionEstimatePointCloudNormals, SIGNAL(triggered()), this, SLOT(pointCloudEstimateNormals()));
    connect(ui->actionReorientPointCloudNormals, SIGNAL(triggered()), this, SLOT(pointCloudReorientNormals()));
    connect(ui->actionNormalizePointCloudNormals, SIGNAL(triggered()), this, SLOT(pointCloudNormalizeNormals()));

    connect(ui->actionRansacPrimitiveExtraction, SIGNAL(triggered()), this, SLOT(pointCloudRansacPrimitiveExtraction()));
    connect(ui->actionPoissonSurfaceReconstruction, SIGNAL(triggered()), this, SLOT(pointCloudPoissonSurfaceReconstruction()));

    connect(ui->actionDelaunayTriangulation2D, SIGNAL(triggered()), this, SLOT(pointCloudDelaunayTriangulation2D()));
    connect(ui->actionDelaunayTriangulation3D, SIGNAL(triggered()), this, SLOT(pointCloudDelaunayTriangulation3D()));
}


void MainWindow::createActionsForSurfaceMeshMenu() {
    connect(ui->actionExtractConnectedComponents, SIGNAL(triggered()), this, SLOT(surfaceMeshExtractConnectedComponents()));
    connect(ui->actionDualMesh, SIGNAL(triggered()), this, SLOT(surfaceMeshDual()));
    connect(ui->actionPlanarPartition, SIGNAL(triggered()), this, SLOT(surfaceMeshPlanarPartition()));
    connect(ui->actionPolygonization, SIGNAL(triggered()), this, SLOT(surfaceMeshPolygonization()));
    connect(ui->actionSurfaceMeshTriangulation, SIGNAL(triggered()), this, SLOT(surfaceMeshTriangulation()));
    connect(ui->actionSurfaceMeshTetrahedralization, SIGNAL(triggered()), this, SLOT(surfaceMeshTetrahedralization()));

    connect(ui->actionSurfaceMeshRepairPolygonSoup, SIGNAL(triggered()), this, SLOT(surfaceMeshRepairPolygonSoup()));
    connect(ui->actionSurfaceMeshOrientAndStitchPolygonSoup, SIGNAL(triggered()), this, SLOT(surfaceMeshOrientAndStitchPolygonSoup()));

    connect(ui->actionSurfaceMeshClip, SIGNAL(triggered()), this, SLOT(surfaceMeshClip()));
    connect(ui->actionSurfaceMeshSplit, SIGNAL(triggered()), this, SLOT(surfaceMeshSplit()));
    connect(ui->actionSurfaceMeshSlice, SIGNAL(triggered()), this, SLOT(surfaceMeshSlice()));

    connect(ui->actionStitchWithReorientation, SIGNAL(triggered()), this, SLOT(surfaceMeshStitchWithReorientation()));
    connect(ui->actionStitchWithoutReorientation, SIGNAL(triggered()), this, SLOT(surfaceMeshStitchWithoutReorientation()));

    connect(ui->actionOrientClosedTriangleMesh, SIGNAL(triggered()), this, SLOT(surfaceMeshOrientClosedTriangleMesh()));
    connect(ui->actionReverseOrientation, SIGNAL(triggered()), this, SLOT(surfaceMeshReverseOrientation()));

    connect(ui->actionSurfaceMeshRemoveIsolatedVertices, SIGNAL(triggered()), this, SLOT(surfaceMeshRemoveIsolatedVertices()));
    connect(ui->actionRemoveDuplicateAndFoldingFaces, SIGNAL(triggered()), this, SLOT(surfaceMeshRemoveDuplicateAndFoldingFaces()));
    connect(ui->actionDetectSelfIntersections, SIGNAL(triggered()), this, SLOT(surfaceMeshDetectSelfIntersections()));
    connect(ui->actionRemeshSelfIntersections, SIGNAL(triggered()), this, SLOT(surfaceMeshRemeshSelfIntersections()));

    connect(ui->actionSurfaceMeshSubdivisionCatmullClark, SIGNAL(triggered()), this, SLOT(surfaceMeshSubdivisionCatmullClark()));
    connect(ui->actionSurfaceMeshSubdivisionLoop, SIGNAL(triggered()), this, SLOT(surfaceMeshSubdivisionLoop()));
    connect(ui->actionSurfaceMeshSubdivisionSqrt3, SIGNAL(triggered()), this, SLOT(surfaceMeshSubdivisionSqrt3()));

    connect(ui->actionSurfaceMeshFairing, SIGNAL(triggered()), this, SLOT(surfaceMeshFairing()));
    connect(ui->actionSurfaceMeshSmoothing, SIGNAL(triggered()), this, SLOT(surfaceMeshSmoothing()));
    connect(ui->actionSurfaceMeshHoleFilling, SIGNAL(triggered()), this, SLOT(surfaceMeshHoleFilling()));
    connect(ui->actionSurfaceMeshSimplification, SIGNAL(triggered()), this, SLOT(surfaceMeshSimplification()));
    connect(ui->actionSurfaceMeshParameterization, SIGNAL(triggered()), this, SLOT(surfaceMeshParameterization()));
    connect(ui->actionSurfaceMeshRemeshing, SIGNAL(triggered()), this, SLOT(surfaceMeshRemeshing()));
    connect(ui->actionSurfaceMeshGeodesic, SIGNAL(triggered()), this, SLOT(surfaceMeshGeodesic()));
    connect(ui->actionSamplingSurfaceMesh, SIGNAL(triggered()), this, SLOT(surfaceMeshSampling()));
    connect(ui->actionCreateSurfaceMeshFromText, SIGNAL(triggered()), this, SLOT(surfaceMeshCreateMeshFromText()));
}


void MainWindow::createActionsForPolyMeshMenu() {
    connect(ui->actionPolyMeshExtractBoundary, SIGNAL(triggered()), this, SLOT(polymeshExtractBoundary()));
}


void MainWindow::operationModeChanged(QAction* act) {
    if (act == ui->actionCameraManipulation) {
        viewer()->tool_manager()->set_tool(tools::ToolManager::EMPTY_TOOL);
        return;
    }

    auto tool_manager = viewer()->tool_manager();
    if (act == ui->actionSelectClick) {
        if (dynamic_cast<SurfaceMesh *>(viewer()->currentModel()))
            tool_manager->set_tool(tools::ToolManager::SELECT_SURFACE_MESH_FACE_CLICK_TOOL);
        else if (dynamic_cast<PointCloud *>(viewer()->currentModel()))
            tool_manager->set_tool(tools::ToolManager::SELECT_POINT_CLOUD_CLICK_TOOL);
    }
    else if (act == ui->actionSelectRect) {
        if (dynamic_cast<SurfaceMesh *>(viewer()->currentModel()))
            tool_manager->set_tool(tools::ToolManager::SELECT_SURFACE_MESH_FACE_RECT_TOOL);
        else if (dynamic_cast<PointCloud *>(viewer()->currentModel()))
            tool_manager->set_tool(tools::ToolManager::SELECT_POINT_CLOUD_RECT_TOOL);
    }
    else if (act == ui->actionSelectLasso) {
        if (dynamic_cast<SurfaceMesh *>(viewer()->currentModel()))
            tool_manager->set_tool(tools::ToolManager::SELECT_SURFACE_MESH_FACE_LASSO_TOOL);
        else if (dynamic_cast<PointCloud *>(viewer()->currentModel()))
            tool_manager->set_tool(tools::ToolManager::SELECT_POINT_CLOUD_LASSO_TOOL);
    }

    if (viewer()->tool_manager()->current_tool())
        statusBar()->showMessage(QString::fromStdString(tool_manager->current_tool()->instruction()), 2000);
}


void MainWindow::reportTopologyStatistics() {
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer()->currentModel());
    if (mesh) {
        std::stringstream stream;

        const auto &components = SurfaceMeshComponent::extract(mesh);
        stream << "model has " << components.size() << " connected components";

        // count isolated vertices
        std::size_t count = 0;
        for (auto v : mesh->vertices()) {
            if (mesh->is_isolated(v))
                ++count;
        }
        if (count > 0)
            stream << "and " << count << " isolated vertices";
        stream << std::endl;

        const std::size_t num = 10;
        if (components.size() > num)
            stream << "    topology of the first " << num << " components:" << std::endl;

        for (std::size_t i = 0; i < std::min(components.size(), num); ++i) {
            const SurfaceMeshComponent &comp = components[i];
            SurfaceMeshTopology topo(&comp);
            std::string type = "unknown";
            if (topo.is_sphere())
                type = "sphere";
            else if (topo.is_disc())
                type = "disc";
            else if (topo.is_cylinder())
                type = "cylinder";
            else if (topo.is_torus())
                type = "torus";
            else if (topo.is_closed())
                type = "unknown closed";

            stream << "        " << i << ": " << type
                   << ", F = " << comp.n_faces() << ", V = " << comp.n_vertices() << ", E = " << comp.n_edges()
                   << ", B = " << topo.number_of_borders();
            if (topo.number_of_borders() == 1)
                stream << ", border size = " << topo.largest_border_size();
            else if (topo.number_of_borders() > 1)
                stream << ", largest border size = " << topo.largest_border_size();
            stream << std::endl;
        }

        LOG(INFO) << stream.str();
    }

    auto graph = dynamic_cast<Graph *>(viewer()->currentModel());
    if (graph) {
        std::stringstream stream;
        stream << "graph has " << graph->n_vertices() << " vertices and " << graph->n_edges() << " edges" << std::endl;

        std::map<unsigned int, int> count; // key (i.e., first element) is the valence
        for (auto v : graph->vertices())
            ++count[graph->valence(v)];

        for (const auto& elem : count)
            stream << "    number of degree " << elem.first << " vertices: " << elem.second << std::endl;

        LOG(INFO) << stream.str();
    }
}


void MainWindow::surfaceMeshTriangulation() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
    if (!mesh)
        return;

    SurfaceMeshTriangulation triangulator(mesh);
    triangulator.triangulate(SurfaceMeshTriangulation::MIN_AREA);

    mesh->renderer()->update();
    viewer_->update();
    updateUi();
}


void MainWindow::surfaceMeshTetrahedralization() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
    if (!mesh)
        return;

    SurfaceMeshTetrehedralization tetra;
    PolyMesh* result = tetra.apply(mesh);
    if (result) {
        const std::string &name = file_system::name_less_extension(mesh->name()) + "_tetrahedralization.plm";
        result->set_name(name);

        viewer_->addModel(result);
        updateUi();
        viewer_->update();
    }
}


void MainWindow::surfaceMeshRepairPolygonSoup() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

#if HAS_CGAL
    Surfacer::repair_polygon_soup(mesh);

    mesh->renderer()->update();
    viewer_->update();
    updateUi();
#else
    LOG(WARNING) << "This function requires CGAL but CGAL was not found when Easy3D was built.";
#endif
}


void MainWindow::surfaceMeshStitchWithReorientation() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

#if HAS_CGAL
    Surfacer::stitch_borders(mesh);
    Surfacer::merge_reversible_connected_components(mesh);
#else
    SurfaceMeshStitching stitch(mesh);
    stitch.apply();
    LOG(WARNING) << "install CGAL to allow stitching connected components with incompatible boundaries";
#endif

    mesh->renderer()->update();
    viewer_->update();
    updateUi();
}


void MainWindow::surfaceMeshOrientAndStitchPolygonSoup() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

#if HAS_CGAL
    Surfacer::orient_and_stitch_polygon_soup(mesh);
#else
    SurfaceMeshStitching stitch(mesh);
    stitch.apply();
    LOG(WARNING) << "install CGAL to allow stitching connected components with incompatible boundaries";
#endif

    mesh->renderer()->update();
    viewer_->update();
    updateUi();
}


void MainWindow::surfaceMeshStitchWithoutReorientation() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

#if HAS_CGAL
    Surfacer::stitch_borders(mesh);
#else
    SurfaceMeshStitching stitch(mesh);
    stitch.apply();
    LOG(WARNING) << "install CGAL to allow stitching connected components with incompatible boundaries";
#endif

    mesh->renderer()->update();
    viewer_->update();
    updateUi();
}


void MainWindow::surfaceMeshOrientClosedTriangleMesh() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

#if HAS_CGAL
    Surfacer::orient_closed_triangle_mesh(mesh);

    mesh->renderer()->update();
    viewer_->update();
    updateUi();
#else
    LOG(WARNING) << "This function requires CGAL but CGAL was not found when Easy3D was built.";
#endif
}


void MainWindow::surfaceMeshReverseOrientation() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    mesh->reverse_orientation();

    mesh->renderer()->update();
    viewer_->update();
    updateUi();
}


void MainWindow::surfaceMeshRemoveIsolatedVertices() {
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer()->currentModel());
    if (!mesh)
        return;

    unsigned int prev_num_vertices = mesh->n_vertices();

    // clean: remove isolated vertices
    for (auto v : mesh->vertices()) {
        if (mesh->is_isolated(v))
            mesh->delete_vertex(v);
    }
    mesh->collect_garbage();

    unsigned int count = prev_num_vertices - mesh->n_vertices();
    LOG(INFO) << count << " isolated vertices deleted.";

    mesh->renderer()->update();
    viewer_->update();
    updateUi();
}


void MainWindow::surfaceMeshRemoveDuplicateAndFoldingFaces() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

#if HAS_CGAL
    StopWatch w;
    w.start();
	LOG(INFO) << "removing overlapping faces...";

    unsigned int num_degenerate = Surfacer::remove_degenerate_faces(mesh, 1e-5f);
    unsigned int num_overlapping = Surfacer::remove_overlapping_faces(mesh, true);
    if (num_degenerate + num_overlapping > 0) {
        mesh->renderer()->update();
        viewer_->update();
        updateUi();
    }
    LOG(INFO) << "done. " << num_degenerate + num_overlapping << " faces removed (" << num_degenerate
              << " degenerate, " << num_overlapping << " overlapping). " << w.time_string();
#else
    LOG(WARNING) << "This function requires CGAL but CGAL was not found when Easy3D was built.";
#endif
}


void MainWindow::surfaceMeshDetectSelfIntersections() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

#if HAS_CGAL
    StopWatch w;
    w.start();
	LOG(INFO) << "detecting intersecting faces...";

    const auto& pairs = Surfacer::detect_self_intersections(mesh);
    if (pairs.empty())
        LOG(INFO) << "done. No intersecting faces detected. " << w.time_string();
    else {
        auto select = mesh->get_face_property<bool>("f:select");
        if (select)
            select.vector().resize(mesh->n_faces(), false);
        else
            select = mesh->add_face_property<bool>("f:select", false);

        for (const auto& pair : pairs) {
            select[pair.first] = true;
            select[pair.second] = true;
        }

        LOG(INFO) << "done. " << pairs.size() << " pairs of faces intersect (marked in face property 'f:select'). " << w.time_string();
        updateRenderingPanel();
    }
#else
    LOG(WARNING) << "This function requires CGAL but CGAL was not found when Easy3D was built.";
#endif
 }


void MainWindow::surfaceMeshRemeshSelfIntersections() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

#if HAS_CGAL
    StopWatch w;
    w.start();
	LOG(INFO) << "remeshing self intersections...";

    auto size = mesh->n_faces();
    if (Surfacer::remesh_self_intersections(mesh, true)) {
		LOG(INFO) << "done. #faces " << size << " -> " << mesh->n_faces() << ". " << w.time_string();
        mesh->renderer()->update();
        viewer_->update();
        updateUi();
    }
    else
		LOG(INFO) << "done. No intersecting faces detected. " << w.time_string();
#else
    LOG(WARNING) << "This function requires CGAL but CGAL was not found when Easy3D was built.";
#endif
}


void MainWindow::surfaceMeshClip() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

#if HAS_CGAL
    if (!ClippingPlane::instance()->is_enabled()) {
        LOG(WARNING) << "clipping plane is not enabled";
        return;
    }

    Surfacer::clip(mesh, ClippingPlane::instance()->plane0(), false);

    mesh->renderer()->update();
    viewer_->update();
    updateUi();
#else
    LOG(WARNING) << "This function requires CGAL but CGAL was not found when Easy3D was built.";
#endif
}


void MainWindow::surfaceMeshSplit() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

#if HAS_CGAL
    if (!ClippingPlane::instance()->is_enabled()) {
        LOG(WARNING) << "clipping plane is not enabled";
        return;
    }

    Surfacer::split(mesh, ClippingPlane::instance()->plane0());

    mesh->renderer()->update();
    viewer_->update();
    updateUi();
#else
    LOG(WARNING) << "This function requires CGAL but CGAL was not found when Easy3D was built.";
#endif
}


void MainWindow::surfaceMeshSlice() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

#if HAS_CGAL

#if 0 // slice by the visual clipping plane

    auto clipping_plane = easy3d::setting::clipping_plane;
    if (!clipping_plane || !clipping_plane->is_enabled()) {
        LOG(WARNING) << "clipping plane is not defined";
        return;
    }

    const std::vector<Surfacer::Polyline>& polylines = Surfacer::slice(mesh, clipping_plane->plane0());

    auto graph = new Graph;
    for (const auto& polyline : polylines) {
        for (const auto &p : polyline)
            graph->add_vertex(p);
    }

    unsigned int idx = 0;
    for (const auto& polyline : polylines) {
        for (unsigned int i=0; i<polyline.size() - 1; ++i) {
            graph->add_edge(Graph::Vertex(idx), Graph::Vertex(idx + 1));
            ++idx;
        }
        ++idx;
    }

    graph->set_name(file_system::base_name(mesh->name()) + "-slice");
    viewer()->addModel(graph);
    ui->treeWidgetModels->addModel(graph, false);

#else   // slices using a set of horizontal planes

    float minz = mesh->bounding_box().min_point().z;
    float maxz = mesh->bounding_box().max_point().z;

    int num = 10;
    float step = (maxz - minz) / static_cast<float>(num);

    std::vector<Plane3> planes(num);
    for (int i=0; i<num; ++i)
        planes[i] = Plane3(vec3(0, 0, minz + static_cast<float>(i) * step), vec3(0, 0, 1));

    const std::vector< std::vector<Surfacer::Polyline> >& all_polylines = Surfacer::slice(mesh, planes);
    if (all_polylines.empty())
        return;

    auto graph = new Graph;
    for (const auto& polylines : all_polylines) {
        for (const auto &polyline : polylines) {
            for (const auto &p : polyline)
                graph->add_vertex(p);
        }
    }

    auto color = graph->add_edge_property<vec3>("e:color");
    int idx = 0;
    for (const auto& polylines : all_polylines) {
        for (const auto &polyline : polylines) {
            const auto& c = random_color();
            for (unsigned int i = 0; i < polyline.size() - 1; ++i) {
                auto e = graph->add_edge(Graph::Vertex(idx), Graph::Vertex(idx + 1));
                color[e] = c;
                ++idx;
            }
            ++idx;
        }
    }

    graph->set_name(file_system::base_name(mesh->name()) + "-slice");
    viewer()->addModel(graph);

    auto edges = graph->renderer()->get_lines_drawable("edges");
    edges->set_line_width(2.0f);
    edges->set_uniform_coloring(vec4(1, 0, 0, 1));
    LOG(INFO) << "color information added to visualize individual polylines of the slice";
    edges->set_coloring(easy3d::State::COLOR_PROPERTY, easy3d::State::EDGE, "e:color");

    auto vertices = graph->renderer()->get_points_drawable("vertices");
    vertices->set_uniform_coloring(vec4(0, 1, 0, 1));
    vertices->set_point_size(4.0f);
    vertices->set_visible(false);

    ui->treeWidgetModels->addModel(graph, false);
#endif

#else
    LOG(WARNING) << "This function requires CGAL but CGAL was not found when Easy3D was built.";
#endif
}


void MainWindow::surfaceMeshCreateMeshFromText() {
    static DialogSurfaceMeshFromText* dialog = nullptr;
    if (!dialog)
        dialog = new DialogSurfaceMeshFromText(this);
    dialog->show();
}


void MainWindow::pointCloudEstimateNormals() {
    auto cloud = dynamic_cast<PointCloud*>(viewer()->currentModel());
    if (!cloud)
        return;

    DialogPointCloudNormalEstimation dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        unsigned int k = dlg.lineEditNeighborSize->text().toUInt();
        PointCloudNormals::estimate(cloud, k);
        cloud->renderer()->update();
        viewer()->update();
    }
}


void MainWindow::pointCloudReorientNormals() {
    auto cloud = dynamic_cast<PointCloud *>(viewer()->currentModel());
    if (!cloud)
        return;

    DialogPointCloudNormalEstimation dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        unsigned int k = dlg.lineEditNeighborSize->text().toUInt();
        PointCloudNormals::reorient(cloud, k);
        cloud->renderer()->update();
        viewer()->update();
    }
}


void MainWindow::pointCloudNormalizeNormals() {
    auto cloud = dynamic_cast<PointCloud*>(viewer()->currentModel());
    if (!cloud)
        return;

    auto prop = cloud->get_vertex_property<vec3>("v:normal");
    if (!prop) {
        LOG(WARNING) << "point cloud does not have normal information";
        return;
    }

    auto &normals = prop.vector();
    for (auto &n : normals)
        n.normalize();

    cloud->renderer()->update();
    viewer()->update();
}


void MainWindow::polymeshExtractBoundary() {
    auto poly = dynamic_cast<PolyMesh*>(viewer()->currentModel());
    if (!poly)
        return;

    std::vector<std::vector<PolyMesh::Vertex> > faces;
    poly->extract_boundary(faces);

    std::unordered_map<PolyMesh::Vertex, SurfaceMesh::Vertex, PolyMesh::Vertex::Hash> unique_vertex;

    auto mesh = new SurfaceMesh;
    const std::string &name = file_system::name_less_extension(poly->name()) + "_boundary.ply";
    mesh->set_name(name);

    SurfaceMeshBuilder builder(mesh);
    builder.begin_surface();
    for (const auto& f : faces) {
        std::vector<SurfaceMesh::Vertex> vts;
        for (auto pv : f) {
            auto pos = unique_vertex.find(pv);
            if (pos == unique_vertex.end()) {
                auto sv = builder.add_vertex(poly->position(pv));
                unique_vertex[pv] = sv;
                vts.push_back(sv);
            } else
                vts.push_back(pos->second);
        }
        builder.add_face(vts);
    }
    builder.end_surface();

    viewer_->addModel(mesh);
    updateUi();
    viewer_->update();
}


void MainWindow::computeHeightField() {
    auto model = viewer_->currentModel();
    if (!model)
        return;

    // add 3 scalar fields defined on vertices, edges, and faces respectively.
    if (dynamic_cast<SurfaceMesh*>(model)) {
        auto mesh = dynamic_cast<SurfaceMesh*>(model);

        ProgressLogger progress(4, false, false);

        auto v_height_x = mesh->vertex_property<float>("v:height_x");
        auto v_height_y = mesh->vertex_property<float>("v:height_y");
        auto v_height_z = mesh->vertex_property<float>("v:height_z");
        for (auto v : mesh->vertices()) {
            const auto& p = mesh->position(v);
            v_height_x[v] = p.x;
            v_height_y[v] = p.y;
            v_height_z[v] = p.z;
        }
        progress.next();

        auto e_height_x = mesh->edge_property<float>("e:height_x");
        auto e_height_y = mesh->edge_property<float>("e:height_y");
        auto e_height_z = mesh->edge_property<float>("e:height_z");
        for (auto e : mesh->edges()) {
            const auto& s = mesh->vertex(e, 0);
            const auto& t = mesh->vertex(e, 1);
            const auto& c = 0.5 * (mesh->position(s) + mesh->position(t));
            e_height_x[e] = c.x;
            e_height_y[e] = c.y;
            e_height_z[e] = c.z;
        }
        progress.next();

        auto f_height_x = mesh->face_property<float>("f:height_x");
        auto f_height_y = mesh->face_property<float>("f:height_y");
        auto f_height_z = mesh->face_property<float>("f:height_z");
        for (auto f : mesh->faces()) {
            vec3 c(0,0,0);
            float count = 0.0f;
            for (auto v : mesh->vertices(f)) {
                c += mesh->position(v);
                ++count;
            }
            c /= count;
            f_height_x[f] = c.x;
            f_height_y[f] = c.y;
            f_height_z[f] = c.z;
        }
        progress.next();

        // add a vector field to the faces
        mesh->update_face_normals();
        auto fnormals = mesh->get_face_property<vec3>("f:normal");

        // add a vector field to the edges
        auto enormals = mesh->edge_property<vec3>("e:normal");
        for (auto e : mesh->edges()) {
            vec3 n(0,0,0);
            float count(0.0f);
            auto f = mesh->face(e, 0);
            if (f.is_valid()) {
                n += fnormals[f];
                count += 1.0f;
            }
            f = mesh->face(e, 1);
            if (f.is_valid()) {
                n += fnormals[f];
                count += 1.0f;
            }
            enormals[e] = n.normalize();
        }
        progress.next();
    }

    else if (dynamic_cast<PointCloud*>(model)) {
        auto cloud = dynamic_cast<PointCloud*>(model);

        auto v_height_x = cloud->vertex_property<float>("v:height_x");
        auto v_height_y = cloud->vertex_property<float>("v:height_y");
        auto v_height_z = cloud->vertex_property<float>("v:height_z");
        for (auto v : cloud->vertices()) {
            const auto& p = cloud->position(v);
            v_height_x[v] = p.x;
            v_height_y[v] = p.y;
            v_height_z[v] = p.z;
        }
    }

    else if (dynamic_cast<Graph*>(model)) {
        auto graph = dynamic_cast<Graph*>(model);

        auto v_height_x = graph->vertex_property<float>("v:height_x");
        auto v_height_y = graph->vertex_property<float>("v:height_y");
        auto v_height_z = graph->vertex_property<float>("v:height_z");
        for (auto v : graph->vertices()) {
            const auto& p = graph->position(v);
            v_height_x[v] = p.x;
            v_height_y[v] = p.y;
            v_height_z[v] = p.z;
        }

        auto e_height_x = graph->edge_property<float>("e:height_x");
        auto e_height_y = graph->edge_property<float>("e:height_y");
        auto e_height_z = graph->edge_property<float>("e:height_z");
        for (auto e : graph->edges()) {
            const auto& s = graph->vertex(e, 0);
            const auto& t = graph->vertex(e, 1);
            const auto& c = 0.5 * (graph->position(s) + graph->position(t));
            e_height_x[e] = c.x;
            e_height_y[e] = c.y;
            e_height_z[e] = c.z;
        }
    }
    // add 3 scalar fields defined on vertices, edges, and faces respectively.
    else if (dynamic_cast<PolyMesh*>(model)) {
        auto mesh = dynamic_cast<PolyMesh*>(model);

        ProgressLogger progress(4, false, false);

        auto v_height_x = mesh->vertex_property<float>("v:height_x");
        auto v_height_y = mesh->vertex_property<float>("v:height_y");
        auto v_height_z = mesh->vertex_property<float>("v:height_z");
        for (auto v : mesh->vertices()) {
            const auto& p = mesh->position(v);
            v_height_x[v] = p.x;
            v_height_y[v] = p.y;
            v_height_z[v] = p.z;
        }
        progress.next();

        auto e_height_x = mesh->edge_property<float>("e:height_x");
        auto e_height_y = mesh->edge_property<float>("e:height_y");
        auto e_height_z = mesh->edge_property<float>("e:height_z");
        for (auto e : mesh->edges()) {
            const auto& s = mesh->vertex(e, 0);
            const auto& t = mesh->vertex(e, 1);
            const auto& c = 0.5 * (mesh->position(s) + mesh->position(t));
            e_height_x[e] = c.x;
            e_height_y[e] = c.y;
            e_height_z[e] = c.z;
        }
        progress.next();

        auto f_height_x = mesh->face_property<float>("f:height_x");
        auto f_height_y = mesh->face_property<float>("f:height_y");
        auto f_height_z = mesh->face_property<float>("f:height_z");
        for (auto f : mesh->faces()) {
            vec3 c(0,0,0);
            float count = 0.0f;
            for (auto v : mesh->vertices(f)) {
                c += mesh->position(v);
                ++count;
            }
            c /= count;
            f_height_x[f] = c.x;
            f_height_y[f] = c.y;
            f_height_z[f] = c.z;
        }
        progress.next();

        // add a vector field to the faces
        mesh->update_face_normals();
        progress.next();
    }


    model->renderer()->update();
    viewer()->update();
    updateUi();
}


void MainWindow::surfaceMeshExtractConnectedComponents() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
    if (!mesh)
        return;

    const auto& components = SurfaceMeshComponent::extract(mesh);
    LOG(INFO) << "model has " << components.size() << " connected components";

    const std::string color_name = "f:color_components";
    auto face_color = mesh->face_property<vec3>(color_name, vec3(0.5f, 0.5f, 0.5f));
    for (auto& comp : components) {
        const vec3& color = random_color(false);
        for (auto f : comp.faces())
            face_color[f] = color;
    }

    auto faces = mesh->renderer()->get_triangles_drawable("faces");
    faces->set_property_coloring(State::FACE, color_name);

    mesh->renderer()->update();
    viewer()->update();
    updateUi();
}


void MainWindow::surfaceMeshPlanarPartition() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
    if (!mesh)
        return;

    const std::string partition_name = "f:planar_partition";
    auto planar_segments = mesh->face_property<int>(partition_name, -1);
    int num = SurfaceMeshEnumerator::enumerate_planar_components(mesh, planar_segments, 1.0f);
    LOG(INFO) << "model has " << num << " planar segments";

    const std::string color_name = "f:color_planar_partition";
    auto coloring = mesh->face_property<vec3>(color_name, vec3(0, 0, 0));
    Renderer::color_from_segmentation(mesh, planar_segments, coloring);
    auto faces = mesh->renderer()->get_triangles_drawable("faces");
    faces->set_property_coloring(State::FACE, color_name);

    mesh->renderer()->update();
    viewer()->update();
    updateUi();
}


void MainWindow::surfaceMeshDual() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
    if (!mesh)
        return;

    geom::dual(mesh);

    mesh->renderer()->update();
    viewer()->update();
    updateUi();
}


void MainWindow::surfaceMeshPolygonization() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
    if (!mesh)
        return;

#if HAS_CGAL
    // stitch first: to encourage large polygons
    Surfacer::stitch_borders(mesh);
    Surfacer::merge_reversible_connected_components(mesh);
#endif

    // polygonization
    SurfaceMeshPolygonization polygonizer;
    polygonizer.apply(mesh);

#if HAS_CGAL
    // stitch again (the "merge-edge" edge operation in polygonization may result in some borders)
    Surfacer::stitch_borders(mesh);
    Surfacer::merge_reversible_connected_components(mesh);
#endif

    mesh->renderer()->update();
    viewer()->update();
    updateUi();
}


void MainWindow::surfaceMeshSubdivisionCatmullClark() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    if (SurfaceMeshSubdivision::catmull_clark(mesh)) {
        mesh->renderer()->update();
        viewer()->update();
        updateUi();
    }
}


void MainWindow::surfaceMeshSubdivisionLoop() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    if (SurfaceMeshSubdivision::loop(mesh)) {
        mesh->renderer()->update();
        viewer()->update();
        updateUi();
    }
}


void MainWindow::surfaceMeshSubdivisionSqrt3() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    if (SurfaceMeshSubdivision::sqrt3(mesh)) {
        mesh->renderer()->update();
        viewer()->update();
        updateUi();
    }
}


void MainWindow::manipulateProperties() {
    static DialogProperties* dialog = nullptr;
    if (!dialog)
        dialog = new DialogProperties(this);
    dialog->show();
}


void MainWindow::pointCloudPoissonSurfaceReconstruction() {
    static DialogPoissonReconstruction* dialog = nullptr;
    if (!dialog)
        dialog = new DialogPoissonReconstruction(this);
    dialog->show();
}


void MainWindow::pointCloudRansacPrimitiveExtraction() {
    static DialogPointCloudRansacPrimitiveExtraction* dialog = nullptr;
    if (!dialog)
        dialog = new DialogPointCloudRansacPrimitiveExtraction(this);
    dialog->show();
}


void MainWindow::surfaceMeshSampling() {
    static DialogSurfaceMeshSampling* dialog = nullptr;
    if (!dialog)
        dialog = new DialogSurfaceMeshSampling(this);
    dialog->show();
}


void MainWindow::pointCloudDownsampling() {
    static DialogPointCloudSimplification* dialog = nullptr;
    if (!dialog)
        dialog = new DialogPointCloudSimplification(this);
    dialog->show();
}

namespace internal {
    template<typename MODEL>
    void translate(MODEL* model, const vec3& p) {
        auto points = model->template get_vertex_property<vec3>("v:point");
        for (auto v : model->vertices())
            points[v] -= p;
    }
}

void MainWindow::translationalRecenter() {
    Model* first_model = viewer_->models()[0];

    const vec3 origin = first_model->bounding_box().center();
    for (auto model : viewer_->models()) {
        if (dynamic_cast<SurfaceMesh*>(model))
            internal::translate(dynamic_cast<SurfaceMesh*>(model), origin);
        else if (dynamic_cast<PointCloud*>(model))
            internal::translate(dynamic_cast<PointCloud*>(model), origin);
        else if (dynamic_cast<Graph*>(model))
            internal::translate(dynamic_cast<Graph*>(model), origin);
        else if (dynamic_cast<PolyMesh*>(model))
            internal::translate(dynamic_cast<PolyMesh*>(model), origin);

        model->manipulator()->reset();
        model->renderer()->update();
    }
    
    // since translated, recenter to screen
    viewer_->fitScreen();
}


void MainWindow::addGaussianNoise() {
    static DialogGaussianNoise* dialog = nullptr;
    if (!dialog)
        dialog = new DialogGaussianNoise(this);
    dialog->show();
}


void MainWindow::applyManipulatedTransformation() {
    const auto& models = viewer_->models();
    if (models.empty())
        return;

    for (auto model : models) {
        mat4 manip = model->manipulator()->matrix();
        auto &points = model->points();
        for (auto &p: points)
            p = manip * p;

        if (dynamic_cast<SurfaceMesh *>(model)) {
            dynamic_cast<SurfaceMesh *>(model)->update_vertex_normals();
        } else if (dynamic_cast<PointCloud *>(model)) {
            auto cloud = dynamic_cast<PointCloud *>(model);
            auto normal = cloud->get_vertex_property<vec3>("v:normal");
            if (normal) {
                const mat3 &N = transform::normal_matrix(manip);
                for (auto v: cloud->vertices())
                    normal[v] = N * normal[v];
                // vector fields...
            }
        }
        model->manipulator()->reset();
        model->renderer()->update();
    }
    viewer_->update();
}


void MainWindow::giveUpManipulatedTransformation() {
    Model* model = viewer_->currentModel();
    if (!model)
        return;

    model->manipulator()->reset();
    model->renderer()->update();
    viewer_->update();
}


void MainWindow::computeSurfaceMeshCurvatures() {
    static DialogSurfaceMeshCurvature* dialog = nullptr;
    if (!dialog)
        dialog = new DialogSurfaceMeshCurvature(this);
    dialog->show();
}


void MainWindow::surfaceMeshSimplification() {
    static DialogSurfaceMeshSimplification* dialog = nullptr;
    if (!dialog)
        dialog = new DialogSurfaceMeshSimplification(this);
    dialog->show();
}


void MainWindow::surfaceMeshSmoothing() {
    static DialogSurfaceMeshSmoothing* dialog = nullptr;
    if (!dialog)
        dialog = new DialogSurfaceMeshSmoothing(this);
    dialog->show();
}


void MainWindow::surfaceMeshFairing() {
    static DialogSurfaceMeshFairing* dialog = nullptr;
    if (!dialog)
        dialog = new DialogSurfaceMeshFairing(this);
    dialog->show();
}


void MainWindow::surfaceMeshHoleFilling() {
    static DialogSurfaceMeshHoleFilling* dialog = nullptr;
    if (!dialog)
        dialog = new DialogSurfaceMeshHoleFilling(this);
    dialog->show();
}


void MainWindow::surfaceMeshRemeshing() {
    static DialogSurfaceMeshRemeshing* dialog = nullptr;
    if (!dialog)
        dialog = new DialogSurfaceMeshRemeshing(this);
    dialog->show();
}


void MainWindow::surfaceMeshParameterization() {
    static DialogSurfaceMeshParameterization* dialog = nullptr;
    if (!dialog)
        dialog = new DialogSurfaceMeshParameterization(this);
    dialog->show();
}


void MainWindow::animation() {
    static DialogWalkThrough* dialog = nullptr;
    if (!dialog)
        dialog = new DialogWalkThrough(this);

    viewer_->walkThrough()->set_scene(viewer_->models());

    // don't allow model picking when creating camera paths.
    ui->actionSelectModel->setChecked(false);
    for (auto m : viewer_->models())
        m->renderer()->set_selected(false);

    dialog->show();
}


void MainWindow::showPrimitivePropertyUnderMouse(bool b) {
    if (b)
        ui->actionShowCoordinatesUnderMouse->setChecked(false);
    viewer_->showPrimitivePropertyUnderMouse(b);
}


void MainWindow::showCoordinatesUnderMouse(bool b) {
    if (b)
        ui->actionShowPrimitivePropertyUnderMouse->setChecked(false);
    viewer_->showCoordinatesUnderMouse(b);
}


void MainWindow::importCameraPath() {
    std::string dir = "./";
    if (viewer_->currentModel())
        dir = file_system::parent_directory(viewer_->currentModel()->name());
    QString suggested_dir = QString::fromStdString(dir);
    const QString fileName = QFileDialog::getOpenFileName(
            this,
            "Import keyframes from file",
            suggested_dir,
            "Keyframe file (*.kf)\n"
            "All formats (*.*)"
    );

    if (fileName.isEmpty())
        return;

    if (viewer_->walkThrough()->interpolator()->read_keyframes(fileName.toStdString())) {
        LOG(INFO) << viewer_->walkThrough()->interpolator()->number_of_keyframes() << " keyframes loaded";
        viewer_->adjustSceneRadius();
    }

    viewer_->update();
}


void MainWindow::exportCameraPath() {
    if (viewer_->walkThrough()->interpolator()->number_of_keyframes() == 0) {
        LOG(INFO) << "nothing can be exported (path is empty)";
        return;
    }

    std::string name = "./keyframes.kf";
    if (viewer_->currentModel())
        name = file_system::replace_extension(viewer_->currentModel()->name(), "kf");

    QString suggested_name = QString::fromStdString(name);
    const QString fileName = QFileDialog::getSaveFileName(
            this,
            "Export keyframes to file",
            suggested_name,
            "Keyframe file (*.kf)\n"
            "All formats (*.*)"
    );

    if (fileName.isEmpty())
        return;

    if (viewer_->walkThrough()->interpolator()->save_keyframes(fileName.toStdString()))
        LOG(INFO) << "keyframes saved to file";
}


void MainWindow::setShowCameraPath(bool b) {
    viewer_->walkThrough()->set_path_visible(b);
    viewer_->adjustSceneRadius();
    viewer_->update();
}


void MainWindow::setShowKeyframeCameras(bool b) {
    viewer_->walkThrough()->set_cameras_visible(b);
    viewer_->adjustSceneRadius();
    viewer_->update();
}


void MainWindow::surfaceMeshGeodesic() {
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer()->currentModel());
    if (!mesh)
        return;

    const auto& components = SurfaceMeshComponent::extract(mesh);
    if (components.size() > 1)
        LOG(WARNING) << "mesh has " << components.size() << " connected components. Geodesic computation is valid on a single component";

    // pick a few a random vertices and mark them locked
    auto locked = mesh->vertex_property<bool>("v:locked", false);
    locked.vector().assign(mesh->n_vertices(), false);

    // setup seeds
    std::vector<SurfaceMesh::Vertex> seeds;
    const int num_seeds = 1;
    for (int i=0; i<num_seeds; ++i) {
        const int idx = rand() % static_cast<int>(mesh->n_vertices());
        SurfaceMesh::Vertex v(idx);
        seeds.push_back(v);
        locked[v] = true;
    }
    LOG(WARNING) << "this only demonstrates Geodesic w.r.t. " << num_seeds << " randomly picked vertices. TODO: ask for user input";

    // compute geodesic distance
    SurfaceMeshGeodesic geodist(mesh);
    geodist.compute(seeds);

    auto drawable = mesh->renderer()->get_triangles_drawable("faces");
    drawable->set_coloring(State::SCALAR_FIELD, State::VERTEX, "v:geodesic:distance");
    mesh->renderer()->update();
    viewer_->update();
    updateRenderingPanel();
}


void MainWindow::pointCloudDelaunayTriangulation2D() {
    auto cloud = dynamic_cast<PointCloud*>(viewer()->currentModel());
    if (!cloud)
        return;

    const std::vector<vec3>& pts = cloud->points();

    std::vector<vec2> points;
    for (const auto& p : pts)
        points.emplace_back(vec2(p));

    Delaunay2 delaunay;
    delaunay.set_vertices(points);

    auto mesh = new SurfaceMesh;
    const std::string &name = file_system::name_less_extension(mesh->name()) + "_delaunay_XY.ply";
    mesh->set_name(name);

    for (std::size_t i = 0; i < points.size(); i++) {
        mesh->add_vertex(vec3(points[i], pts[i].z));
    }

    for (unsigned int i = 0; i < delaunay.nb_triangles(); i++) {
        std::vector<SurfaceMesh::Vertex> vts(3);
        for (int j = 0; j < 3; j++) {
            const int v = delaunay.tri_vertex(i, j);
            assert(v >= 0);
            assert(v < points.size());
            vts[j] = SurfaceMesh::Vertex(v);
        }
        mesh->add_face(vts);
    }

    viewer_->addModel(mesh);
    updateUi();
    viewer_->update();
}


void MainWindow::pointCloudDelaunayTriangulation3D() {
    auto cloud = dynamic_cast<PointCloud*>(viewer()->currentModel());
    if (!cloud)
        return;

    const std::vector<vec3>& points = cloud->points();
    Delaunay3 delaunay;
    delaunay.set_vertices(points);

    auto mesh = new PolyMesh;
    const std::string &name = file_system::name_less_extension(cloud->name()) + "_delaunay.ply";
    mesh->set_name(name);

    for (const auto& p : points)
        mesh->add_vertex(p);

    LOG(INFO) << "building tetrahedral mesh with " << delaunay.nb_tets() << " tetrahedra...";
    StopWatch w;
    for (unsigned int i = 0; i < delaunay.nb_tets(); i++) {
        PolyMesh::Vertex vts[4];
        for (int j = 0; j < 4; j++) {
            int v = delaunay.tet_vertex(i, j);
            assert(v >= 0);
            assert(v < points.size());
            vts[j] = PolyMesh::Vertex(v);
        }
        mesh->add_tetra(vts[0], vts[1], vts[2], vts[3]);
    }
    LOG(INFO) << "done. " << w.time_string();

    viewer_->addModel(mesh);
    updateUi();
    viewer_->update();
}