#include "main_window.h"

#include <string>
#include <iostream>

#include <QFileDialog>
#include <QDropEvent>
#include <QMimeData>
#include <QSettings>
#include <QMessageBox>
#include <QToolBox>
#include <QColorDialog>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/random.h>
#include <easy3d/viewer/model.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/fileio/point_cloud_io_ptx.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/algo/point_cloud_normals.h>
#include <easy3d/algo/surface_mesh_components.h>
#include <easy3d/algo/surface_mesh_topology.h>
#include <easy3d/algo/surface_mesh_triangulation.h>
#include <easy3d/algo/surface_mesh_subdivision.h>
#include <easy3d/algo/surface_mesh_features.h>
#include <easy3d/algo/surface_mesh_remeshing.h>
#include <easy3d/algo/surface_mesh_parameterization.h>
#include <easy3d/algo/surface_mesh_simplification.h>
#include <easy3d/algo/surface_mesh_fairing.h>
#include <easy3d/algo/surface_mesh_smoothing.h>
#include <easy3d/algo/surface_mesh_hole_filling.h>
#include <easy3d/algo/surface_mesh_geodesic.h>
#include <easy3d/algo_ext/mesh_surfacer.h>
#include <easy3d/algo_ext/delaunay_2d.h>
#include <easy3d/algo_ext/delaunay_3d.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/stop_watch.h>

#include "paint_canvas.h"

#include "dialogs/dialog_snapshot.h"
#include "dialogs/dialog_properties.h"
#include "dialogs/dialog_poisson_reconstruction.h"
#include "dialogs/dialog_surface_mesh_curvature.h"
#include "dialogs/dialog_surface_mesh_sampling.h"
#include "dialogs/dialog_ransac_primitive_extraction.h"
#include "dialogs/dialog_point_cloud_simplification.h"
#include "dialogs/dialog_gaussian_noise.h"

#include "widgets/widget_global_setting.h"
#include "widgets/widget_drawable_points.h"
#include "widgets/widget_drawable_lines.h"
#include "widgets/widget_drawable_triangles.h"

#include <ui_main_window.h>


using namespace easy3d;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , dockWidgetCommand_(nullptr)
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

    // ----- rendering panel ------

    widgetTrianglesDrawable_ = new WidgetTrianglesDrawable(this);
    ui->verticalLayoutTrianglesDrawable->addWidget(widgetTrianglesDrawable_);
    widgetTrianglesDrawable_->setEnabled(false);

    widgetLinesDrawable_ = new WidgetLinesDrawable(this);
    ui->verticalLayoutLinesDrawable->addWidget(widgetLinesDrawable_);
    widgetLinesDrawable_->setEnabled(false);

    widgetPointsDrawable_ = new WidgetPointsDrawable(this);
    ui->verticalLayoutPointsDrawable->addWidget(widgetPointsDrawable_);
    widgetPointsDrawable_->setEnabled(false);

    ui->verticalLayoutGlobalSetting->addWidget(new WidgetGlobalSetting(this));

    // ---------------------------

    // file menu
    createActionsForFileMenu();

    // view menu
    createActionsForViewMenu();

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

    // about menu
    connect(ui->actionAboutMapple, SIGNAL(triggered()), this, SLOT(onAboutMapple()));
    connect(ui->actionManual, SIGNAL(triggered()), this, SLOT(showManual()));

    // options for the model panel
    connect(ui->checkBoxAutoFocus, SIGNAL(toggled(bool)), ui->treeWidgetModels, SLOT(setAutoFocus(bool)));
    connect(ui->checkBoxSelectedOnly, SIGNAL(toggled(bool)), ui->treeWidgetModels, SLOT(setSelectedOnly(bool)));

    setWindowIcon(QIcon(QString::fromStdString(resource::directory() + "/icons/Mapple.png")));
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops(true);

    setBaseSize(1280, 960);

#ifdef NDEBUG
    setWindowState(Qt::WindowMaximized);
#endif

    readSettings();
    updateWindowTitle();

#ifdef NDEBUG
    QMessageBox::warning(this, "Mapple is not ready yet!",
                         "Mapple is still under development. This version is not feature complete nor is tested. Using it is at your own risk.",
                         QMessageBox::Ok);
#endif
}


MainWindow::~MainWindow() {
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


bool MainWindow::onOpen() {
    const QStringList& fileNames = QFileDialog::getOpenFileNames(
                this,
                "Open file(s)",
                curDataDirectory_,
                "Supported formats (*.ply *.obj *.off *.stl *.poly *.bin *.las *.laz *.xyz *.bxyz *.vg *.bvg *.ptx)\n"
                "Mesh formats (*.ply *.obj *.off *.stl *.poly)\n"
                "Point set formats (*.ply *.bin *.ptx *.las *.laz *.xyz *.bxyz *.vg *.bvg *.ptx)\n"
                "All formats (*.*)"
            );

    // Hide closed dialog
    QApplication::processEvents();

    if (fileNames.empty())
        return false;

    int count = 0;
//	ProgressLogger progress(fileNames.size());
    for (const auto& name : fileNames) {
//        if (progress.is_canceled())
//            break;
        if (open(name.toStdString()))
            ++count;
//        progress.next();
    }
    if (count > 0)
        viewer_->update();

    return count > 0;
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
                "Open file(s)",
                QString::fromStdString(default_file_name),
                "Supported formats (*.ply *.obj *.off *.stl *.poly *.bin *.las *.laz *.xyz *.bxyz *.vg *.bvg)\n"
                "Mesh formats (*.ply *.obj *.off *.stl *.poly)\n"
                "Point set formats (*.ply *.bin *.ptx *.las *.laz *.xyz *.bxyz *.vg *.bvg)\n"
                "All formats (*.*)"
    );

    if (fileName.isEmpty())
        return false;

    bool saved = false;
    if (dynamic_cast<const PointCloud*>(model)) {
        const PointCloud* cloud = dynamic_cast<const PointCloud*>(model);
        saved = PointCloudIO::save(fileName.toStdString(), cloud);
    }
    else if (dynamic_cast<const SurfaceMesh*>(model)) {
        const SurfaceMesh* mesh = dynamic_cast<const SurfaceMesh*>(model);
        saved = SurfaceMeshIO::save(fileName.toStdString(), mesh);
    }
    else if (dynamic_cast<const Graph*>(model)) {
        const Graph* graph = dynamic_cast<const Graph*>(model);
        saved = GraphIO::save(fileName.toStdString(), graph);
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
            LOG(WARNING) << "model alreaded loaded: " << file_name;
            return nullptr;
        }
    }

    const std::string& ext = file_system::extension(file_name, true);
    bool is_ply_mesh = false;
    if (ext == "ply")
        is_ply_mesh = (io::PlyReader::num_instances(file_name, "face") > 0);

    Model* model = nullptr;
    if ((ext == "ply" && is_ply_mesh) || ext == "obj" || ext == "off" || ext == "stl" || ext == "poly" || ext == "plg") { // mesh
        model = SurfaceMeshIO::load(file_name);
    }
    else if (ext == "ply" && io::PlyReader::num_instances(file_name, "edge") > 0) {
        model = GraphIO::load(file_name);
    }
    else { // point cloud
        if (ext == "ptx") {
            io::PointCloudIO_ptx serializer(file_name);
            PointCloud* cloud = nullptr;
            while ((cloud = serializer.load_next())) {
                viewer_->addModel(cloud);
                ui->treeWidgetModels->addModel(cloud, true);
            }
            viewer_->fitScreen();
        }
        else
            model = PointCloudIO::load(file_name);
    }

    if (model) {
        model->set_name(file_name);
        viewer_->addModel(model);
        viewer_->fitScreen(model);
        ui->treeWidgetModels->addModel(model, true);
    }

    return model;
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
}


void MainWindow::updateRenderingPanel() {
    widgetTrianglesDrawable_->updatePanel();
    widgetLinesDrawable_->updatePanel();
    widgetPointsDrawable_->updatePanel();
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


void MainWindow::onOpenRecentFile()
{
    if (okToContinue()) {
        QAction *action = qobject_cast<QAction *>(sender());
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


void MainWindow::showDialog(QDialog* dialog) {
    if (!dockWidgetCommand_) {
        dockWidgetCommand_ = new QDockWidget(this);
        dockWidgetCommand_->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
        dockWidgetCommand_->setAllowedAreas(Qt::RightDockWidgetArea);
        dockWidgetCommand_->setFloating(true);
    }
    delete dockWidgetCommand_->widget();
    dockWidgetCommand_->setWidget(dialog);
    dockWidgetCommand_->show();
}


void MainWindow::saveSnapshot() {
    const Model* model = viewer_->currentModel();

    const bool overwrite = false;
    std::string default_file_name("untitled.png");
    if (model)
        default_file_name = file_system::replace_extension(model->name(), "png");

    QString proposedFormat = "PNG (*.png)";
    const QString fileName = QFileDialog::getSaveFileName(
        this,
        "Choose an image file name",
        QString::fromStdString(default_file_name),
        "Image formats (*.png *.jpg *.bmp *.ppm)\n"
        "PNG (*.png)\n"
        "JPG (*.jpg)\n"
        "Windows Bitmap (*.bmp)\n"
        "24bit RGB Bitmap (*.ppm)",
        &proposedFormat,
        overwrite ? QFileDialog::DontConfirmOverwrite : QFlags<QFileDialog::Option>(nullptr)
    );

    // Hide closed dialog
    QApplication::processEvents();

    if (fileName.isEmpty())
        return;

    auto dialog = new DialogSnapshot(this, dockWidgetCommand());
    dialog->setImageFileName(fileName);
    connect(viewer_, SIGNAL(resized()), dialog, SLOT(computeImageSize()));
    showDialog(dialog);
}


void MainWindow::setBackgroundColor() {
    const vec4& c = viewer_->backGroundColor();
    QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255), static_cast<int>(c.a * 255));
    const QColor& color = QColorDialog::getColor(orig, this);
    if (color.isValid()) {
        const vec4 newColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        viewer_->setBackgroundColor(newColor);
        viewer_->update();
    }
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


void MainWindow::onAboutMapple()
{
    QString title = QMessageBox::tr(
        "<p align=\"center\"><span style=\"font-style:italic;\">I'm good software, though I have defects.</span></p>"
        );

#if defined (ENV_32_BIT)
    title += QMessageBox::tr("<h3>Mapple (32-bit)</h3>");
#elif defined (ENV_64_BIT)
    title += QMessageBox::tr("<h3>Mapple (64-bit)</h3>");
#else
    title += QMessageBox::tr("<h3>Mapple</h3>");
#endif

#ifndef NDEBUG
    title += QMessageBox::tr(" (Debug Version)");
#endif

    QString text = QMessageBox::tr(
        "<p><h4> Build %1</h4></p>"
        "<p>Mapple is software for processing and rendering meshes and point clouds.</p>"
        "<p>Liangliang Nan<br>"
        "<a href=\"mailto:liangliang.nan@gmail.com\">liangliang.nan@gmail.com</a><br>"
        "<a href=\"https://3d.bk.tudelft.nl/liangliang/\">https://3d.bk.tudelft.nl/liangliang/</a></p>"
        ).arg("20181206");

    //QMessageBox::about(this, title, text);
    QMessageBox::about(this, "About Mapple", title + text);
}


void MainWindow::showManual() {
    std::cout << viewer()->usage() << std::endl;
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
#endif // _DEBUG

    QString fileName("Untitled");
    if (model)
        fileName = QString::fromStdString(model->name());

    title = tr("%1[*] - %2").arg(strippedName(fileName)).arg(title);
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
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(onOpen()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(onSave()));

    actionSeparator = ui->menuFile->addSeparator();

    QList<QAction*> actions;
    for (int i = 0; i < MaxRecentFiles; ++i) {
        actionsRecentFile[i] = new QAction(this);
        actionsRecentFile[i]->setVisible(false);
        connect(actionsRecentFile[i], SIGNAL(triggered()), this, SLOT(onOpenRecentFile()));

        actions.push_back(actionsRecentFile[i]);
    }
    ui->menuRecentFiles->insertActions(ui->actionClearRecentFiles, actions);
    ui->menuRecentFiles->insertSeparator(ui->actionClearRecentFiles);
    connect(ui->actionClearRecentFiles, SIGNAL(triggered()), this, SLOT(onClearRecentFiles()));

    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    ui->actionExit->setShortcut(QString("Ctrl+Q"));
}


void MainWindow::createActionsForViewMenu() {
    connect(ui->actionSnapshot, SIGNAL(triggered()), this, SLOT(saveSnapshot()));

    ui->menuView->addSeparator();

    connect(ui->actionCopyCamera, SIGNAL(triggered()), viewer_, SLOT(copyCamera()));
    connect(ui->actionPasteCamera, SIGNAL(triggered()), viewer_, SLOT(pasteCamera()));

    QAction* actionToggleDockWidgetRendering = ui->dockWidgetRendering->toggleViewAction();
    actionToggleDockWidgetRendering->setText("Rendering Panel");
    ui->menuView->addAction(actionToggleDockWidgetRendering);

    QAction* actionToggleDockWidgetModels = ui->dockWidgetModels->toggleViewAction();
    actionToggleDockWidgetModels->setText("Model Panel");
    ui->menuView->addAction(actionToggleDockWidgetModels);

    QAction* actionToggleDockWidgetLogger = ui->dockWidgetLogger->toggleViewAction();
    actionToggleDockWidgetLogger->setText("Logger Panel");
    ui->menuView->addAction(actionToggleDockWidgetLogger);

    connect(ui->actionBackgroundColor, SIGNAL(triggered()), this, SLOT(setBackgroundColor()));
}


void MainWindow::createActionsForSelectMenu() {
    connect(ui->actionInvertSelection, SIGNAL(triggered()), viewer_, SLOT(invertSelection()));
    connect(ui->actionDeleteSelectedPrimitives, SIGNAL(triggered()), viewer_, SLOT(deleteSelectedPrimitives()));

    //////////////////////////////////////////////////////////////////////////

    QActionGroup* actionGroup = new QActionGroup(this);
    actionGroup->addAction(ui->actionCameraManipulation);
    actionGroup->addAction(ui->actionSelectClick);
    actionGroup->addAction(ui->actionSelectRect);
    actionGroup->addAction(ui->actionSelectLasso);

    connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(operationModeChanged(QAction*)));
}


void MainWindow::createActionsForEditMenu() {
    connect(ui->actionAddGaussianNoise, SIGNAL(triggered()), this, SLOT(addGaussianNoise()));
}


void MainWindow::createActionsForPropertyMenu() {
    connect(ui->actionManipulateProperties, SIGNAL(triggered()), this, SLOT(manipulateProperties()));
    connect(ui->actionComputeHeightField, SIGNAL(triggered()), this, SLOT(computeHeightField()));
    connect(ui->actionComputeSurfaceMeshCurvatures, SIGNAL(triggered()), this, SLOT(computeSurfaceMeshCurvatures()));
}


void MainWindow::createActionsForPointCloudMenu() {
    connect(ui->actionDownSampling, SIGNAL(triggered()), this, SLOT(pointCloudDownsampling()));

    connect(ui->actionEstimatePointCloudNormals, SIGNAL(triggered()), this, SLOT(pointCloudEstimateNormals()));
    connect(ui->actionReorientPointCloudNormals, SIGNAL(triggered()), this, SLOT(pointCloudReorientNormals()));
    connect(ui->actionNormalizePointCloudNormals, SIGNAL(triggered()), this, SLOT(pointCloudNormalizeNormals()));

    connect(ui->actionRansacPrimitiveExtraction, SIGNAL(triggered()), this, SLOT(pointCloudRansacPrimitiveExtraction()));
    connect(ui->actionPoissonSurfaceReconstruction, SIGNAL(triggered()), this, SLOT(pointCloudPoissonSurfaceReconstruction()));

    connect(ui->actionDelaunayTriangulation2D, SIGNAL(triggered()), this, SLOT(pointCloudDelaunayTriangulation2D()));
    connect(ui->actionDelaynayTriangulation3D, SIGNAL(triggered()), this, SLOT(pointCloudDelaunayTriangulation3D()));
}


void MainWindow::createActionsForSurfaceMeshMenu() {
    connect(ui->actionTopologyStatistics, SIGNAL(triggered()), this, SLOT(surfaceMeshReportTopologyStatistics()));
    connect(ui->actionExtractConnectedComponents, SIGNAL(triggered()), this, SLOT(surfaceMeshExtractConnectedComponents()));
    connect(ui->actionSurfaceMeshStitchCoincidentEdges, SIGNAL(triggered()), this, SLOT(surfaceMeshStitchCoincidentEdges()));
    connect(ui->actionSurfaceMeshRemoveIsolatedVertices, SIGNAL(triggered()), this, SLOT(surfaceMeshRemoveIsolatedVertices()));

    connect(ui->actionSurfaceMeshTriangulation, SIGNAL(triggered()), this, SLOT(surfaceMeshTriangulation()));

    connect(ui->actionDetectDuplicatedFaces, SIGNAL(triggered()), this, SLOT(surfaceMeshDetectDuplicatedFaces()));
    connect(ui->actionRemoveDuplicatedFaces, SIGNAL(triggered()), this, SLOT(surfaceMeshRemoveDuplicatedFaces()));

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
}


void MainWindow::operationModeChanged(QAction* act) {
    if (act == ui->actionCameraManipulation) {
        viewer()->tool_manager()->set_tool(tools::ToolManager::EMPTY_TOOL);
    }
    else if (act == ui->actionSelectClick) {
        if (dynamic_cast<SurfaceMesh*>(viewer()->currentModel()))
            viewer()->tool_manager()->set_tool(tools::ToolManager::SELECT_SURFACE_MESH_FACE_CLICK_TOOL);
    }
    else if (act == ui->actionSelectRect) {
        if (dynamic_cast<SurfaceMesh*>(viewer()->currentModel()))
            viewer()->tool_manager()->set_tool(tools::ToolManager::SELECT_SURFACE_MESH_FACE_RECT_TOOL);
        else if (dynamic_cast<PointCloud*>(viewer()->currentModel()))
            viewer()->tool_manager()->set_tool(tools::ToolManager::SELECT_POINT_CLOUD_RECT_TOOL);
    }
    else if (act == ui->actionSelectLasso) {
        if (dynamic_cast<SurfaceMesh*>(viewer()->currentModel()))
            viewer()->tool_manager()->set_tool(tools::ToolManager::SELECT_SURFACE_MESH_FACE_LASSO_TOOL);
        else if (dynamic_cast<PointCloud*>(viewer()->currentModel()))
            viewer()->tool_manager()->set_tool(tools::ToolManager::SELECT_POINT_CLOUD_LASSO_TOOL);
    }
    viewer()->update();
}


void MainWindow::surfaceMeshReportTopologyStatistics() {
    SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(viewer()->currentModel());
    if (!mesh)
        return;

    const std::string simple_name = file_system::simple_name(mesh->name());
    if (simple_name.empty())
        std::cout << "#elements in model (with unknown name): ";
    else
        std::cout << "#elements in model '" << file_system::simple_name(mesh->name()) << "': ";

    std::cout << "#face = " << mesh->n_faces() << ", #vertex = " << mesh->n_vertices() << ", #edge = "
              << mesh->n_edges() << std::endl;

    // count isolated vertices
    std::size_t count = 0;
    for (auto v : mesh->vertices()) {
        if (mesh->is_isolated(v))
            ++count;
    }
    if (count > 0)
        std::cout << "#isolated vertices: " << count << std::endl;

    const auto &components = SurfaceMeshComponent::extract(mesh);
    std::cout << "#connected component: " << components.size() << std::endl;

    const std::size_t num = 10;
    if (components.size() > num)
        std::cout << "\ttopology of the first " << num << " components:" << std::endl;

    for (std::size_t i = 0; i < std::min(components.size(), num); ++i) {
        const SurfaceMeshComponent& comp = components[i];
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

        std::cout << "\t\t" << i << ": "
                  << type
                  << ", #face = " << comp.n_faces() << ", #vertex = " << comp.n_vertices() << ", #edge = " << comp.n_edges()
                  << ", #border = " << topo.number_of_borders();
        if (topo.number_of_borders() == 1)
            std::cout << ", border size = " << topo.largest_border_size();
        else if (topo.number_of_borders() > 1)
            std::cout << ", largest border size = " << topo.largest_border_size();
        std::cout << std::endl;
    }
}


void MainWindow::surfaceMeshTriangulation() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
    if (!mesh)
        return;

    SurfaceMeshTriangulation triangulator(mesh);
    triangulator.triangulate(SurfaceMeshTriangulation::MAX_ANGLE);

    mesh->update();
    viewer_->update();
}


void MainWindow::surfaceMeshDetectDuplicatedFaces() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

#if HAS_CGAL
    StopWatch w;
    w.start();
    LOG(INFO) << "detecting duplicated faces...";

    MeshSurfacer ms;
    const auto& faces = ms.detect_duplicated_faces(mesh, true);
    LOG(INFO) << "done. " << faces.size() << " faces deleted. " << w.time_string();
#else
    LOG(WARNING) << "This function requires CGAL but CGAL was disabled or not found.";
#endif
}


void MainWindow::surfaceMeshRemoveDuplicatedFaces() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

#if HAS_CGAL
    StopWatch w;
    w.start();
	LOG(INFO) << "removing duplicated faces...";

    MeshSurfacer ms;
    unsigned int num = ms.remove_duplicated_faces(mesh, true);
    if (num > 0) {
        mesh->update();
        viewer_->update();
    }
    LOG(INFO) << "done. " << num << " faces deleted. " << w.time_string();
#else
    LOG(WARNING) << "This function requires CGAL but CGAL was disabled or not found.";
#endif
}


void MainWindow::surfaceMeshDetectSelfIntersections() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

#if HAS_CGAL
    StopWatch w;
    w.start();
	LOG(INFO) << "detecting intersecting faces...";

    MeshSurfacer ms;
    const auto& pairs = ms.detect_self_intersections(mesh);
    if (!pairs.empty())
		LOG(INFO) << "done. " << pairs.size() << " pairs of faces intersect. " << w.time_string();
    else
		LOG(INFO) << "done. No intersecting faces detected. " << w.time_string();
#else
    LOG(WARNING) << "This function requires CGAL but CGAL was disabled or not found.";
#endif
 }


void MainWindow::surfaceMeshRemeshSelfIntersections() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

#if HAS_CGAL
    StopWatch w;
    w.start();
	LOG(INFO) << "remeshing intersecting faces...";

    auto size = mesh->n_faces();
    MeshSurfacer ms;
    SurfaceMesh* result = ms.remesh_self_intersections(mesh, true);
    if (result) {
        const std::string& name = file_system::name_less_extension(mesh->name()) + "_remeshed." + file_system::extension(mesh->name());
        result->set_name(name);
        viewer()->addModel(result);
		LOG(INFO) << "done. #faces " << size << " -> " << result->n_faces() << ". " << w.time_string();
        mesh->update();
        viewer_->update();
        updateUi();
    }
    else
		LOG(INFO) << "done. No intersecting faces detected. " << w.time_string();
#else
    LOG(WARNING) << "This function requires CGAL but CGAL was disabled or not found.";
#endif
}


void MainWindow::pointCloudEstimateNormals() {
    PointCloud* cloud = dynamic_cast<PointCloud*>(viewer()->currentModel());
    if (!cloud)
        return;

    PointCloudNormals pcn;
    std::cout << "show the parameter dialog" << std::endl;
    pcn.estimate(cloud);

    cloud->update();
    viewer()->update();
}


void MainWindow::pointCloudReorientNormals() {
    PointCloud* cloud = dynamic_cast<PointCloud*>(viewer()->currentModel());
    if (!cloud)
        return;

    PointCloudNormals pcn;
    std::cout << "show the parameter dialog" << std::endl;
    pcn.reorient(cloud);

    cloud->update();
    viewer()->update();
}


void MainWindow::pointCloudNormalizeNormals() {
    PointCloud* cloud = dynamic_cast<PointCloud*>(viewer()->currentModel());
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

    cloud->update();
    viewer()->update();
}


void MainWindow::computeHeightField() {
    auto model = viewer_->currentModel();

    // add 3 scalar fields defined on vertices, edges, and faces respectively.
    if (dynamic_cast<SurfaceMesh*>(model)) {
        SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(model);

        auto v_height_x = mesh->vertex_property<float>("v:height_x");
        auto v_height_y = mesh->vertex_property<float>("v:height_y");
        auto v_height_z = mesh->vertex_property<float>("v:height_z");
        for (auto v : mesh->vertices()) {
            const auto& p = mesh->position(v);
            v_height_x[v] = p.x;
            v_height_y[v] = p.y;
            v_height_z[v] = p.z;
        }

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
            enormals[e].normalize();
        }
    }

    else if (dynamic_cast<PointCloud*>(model)) {
        PointCloud* cloud = dynamic_cast<PointCloud*>(model);

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
        Graph* graph = dynamic_cast<Graph*>(model);

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

    model->update();
    viewer()->update();
    updateRenderingPanel();
}


void MainWindow::surfaceMeshExtractConnectedComponents() {
    auto mesh = dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
    if (!mesh)
        return;

    const auto& components = SurfaceMeshComponent::extract(mesh);
    std::cout << "model has " << components.size() << " connected components" << std::endl;

    const std::string color_name = "f:color_connected_components";
    auto face_color = mesh->face_property<vec3>(color_name, vec3(0.5f, 0.5f, 0.5f));
    for (auto& comp : components) {
        const vec3& color = random_color(false);
        for (auto f : comp.faces())
            face_color[f] = color;
    }

    mesh->get_triangles_drawable("faces")->set_property_coloring(State::FACE, color_name);

    mesh->update();
    viewer()->update();
    updateRenderingPanel();
}


void MainWindow::surfaceMeshSubdivisionCatmullClark() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    if (SurfaceMeshSubdivision::catmull_clark(mesh)) {
        mesh->update_vertex_normals();
        mesh->update();
        viewer()->update();
    }
}


void MainWindow::surfaceMeshSubdivisionLoop() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    if (SurfaceMeshSubdivision::loop(mesh)) {
        mesh->update_vertex_normals();
        mesh->update();
        viewer()->update();
    }
}


void MainWindow::surfaceMeshSubdivisionSqrt3() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    if (SurfaceMeshSubdivision::sqrt3(mesh)) {
        mesh->update_vertex_normals();
        mesh->update();
        viewer()->update();
    }
}


QDockWidget* MainWindow::dockWidgetCommand() {
    if (!dockWidgetCommand_) {
        dockWidgetCommand_ = new QDockWidget(this);
        dockWidgetCommand_->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
        dockWidgetCommand_->setAllowedAreas(Qt::RightDockWidgetArea);
        dockWidgetCommand_->setFloating(true);
    }
    return dockWidgetCommand_;
}


void MainWindow::manipulateProperties() {
    auto dialog = new DialogProperties(this, dockWidgetCommand());
    showDialog(dialog);
}


void MainWindow::pointCloudPoissonSurfaceReconstruction() {
    auto dialog = new DialogPoissonReconstruction(this, dockWidgetCommand());
    showDialog(dialog);
}


void MainWindow::pointCloudRansacPrimitiveExtraction() {
    auto dialog = new DialogRansacPrimitiveExtraction(this, dockWidgetCommand());
    showDialog(dialog);
}


void MainWindow::surfaceMeshSampling() {
    auto dialog = new DialogSurfaceMeshSampling(this, dockWidgetCommand());
    showDialog(dialog);
}


void MainWindow::pointCloudDownsampling() {
    auto dialog = new DialogPointCloudSimplification(this, dockWidgetCommand());
    showDialog(dialog);
}


void MainWindow::addGaussianNoise() {
    auto dialog = new DialogGaussianNoise(this, dockWidgetCommand());
    showDialog(dialog);
}


void MainWindow::computeSurfaceMeshCurvatures() {
    auto dialog = new DialogSurfaceMeshCurvature(this, dockWidgetCommand());
    showDialog(dialog);
}


void MainWindow::surfaceMeshSimplification() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    int target_percentage = 10;
    int normal_deviation = 180;
    int aspect_ratio = 10;
    SurfaceMeshSimplification ss(mesh);
    ss.initialize(aspect_ratio, 0.0, 0.0, normal_deviation, 0.0);
    ss.simplify(mesh->n_vertices() * 0.01 * target_percentage);

    mesh->update();
    viewer()->update();
}


void MainWindow::surfaceMeshSmoothing() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    // TODO: compute Mean Curvature first.

    bool uniform_laplace = false;

    SurfaceMeshSmoothing smoother(mesh);
    if (0) {   // Explicit Smoothing
        int iter = 10;
        smoother.explicit_smoothing(iter, uniform_laplace);
    }
    else {    // Implicit Smoothing
        float timestep = 0.001f;

        // does the mesh have a boundary?
        bool has_boundary = false;
        for (auto v: mesh->vertices())
            if (mesh->is_boundary(v))
                has_boundary = true;

        // only re-scale if we don't have a (fixed) boundary
        bool rescale = !has_boundary;
        float scene_radius = viewer()->camera()->sceneRadius();
        float dt = uniform_laplace ? timestep : timestep * scene_radius * scene_radius;
        smoother.implicit_smoothing(dt, uniform_laplace, rescale);
    }

    mesh->update();
    viewer()->update();
}


void MainWindow::surfaceMeshFairing() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    // TODO: compute Mean Curvature first.

    int method = 2;

    SurfaceMeshFairing fair(mesh);
    switch (method) {
        case 0: fair.minimize_area(); // Minimize Area
            break;
        case 1: fair.minimize_curvature(); // Minimize Curvature
            break;
        case 2: fair.fair(3);; // Minimize Curvature Variation
            break;
        default: std::cerr << "no such fairing method" << std::endl;
            return;
    }

    mesh->update();
    viewer()->update();
}


void MainWindow::surfaceMeshHoleFilling() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    // find the smallest hole
    SurfaceMesh::Halfedge hmin;
    unsigned int lmin(mesh->n_halfedges());
    for (auto h : mesh->halfedges()) {
        if (mesh->is_boundary(h)) {
            float l(0);
            SurfaceMesh::Halfedge hh = h;
            do {
                ++l;
                if (!mesh->is_manifold(mesh->to_vertex(hh))) {
                    l += 123456;
                    break;
                }
                hh = mesh->next_halfedge(hh);
            } while (hh != h);

            if (l < lmin) {
                lmin = l;
                hmin = h;
            }
        }
    }

    // close smallest hole
    if (hmin.is_valid()) {
        SurfaceMeshHoleFilling hf(mesh);
        hf.fill_hole(hmin);

        mesh->update();
        viewer()->update();
    } else {
        LOG(WARNING) << "could not find a hole (i.e., manifold boundary loop)";
    }
}


void MainWindow::surfaceMeshRemeshing() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    bool uss_features = true;
    if (uss_features) {
        static int feature_angle = 70;
        SurfaceMeshFeatures sf(mesh);
        sf.clear();
        sf.detect_angle(feature_angle);
        sf.detect_boundary();
    }

    if (true) { // Uniform remeshing
        float len(0.0f);
        for (auto eit : mesh->edges())
            len += distance(mesh->position(mesh->vertex(eit, 0)),
                            mesh->position(mesh->vertex(eit, 1)));
        len /= (float) mesh->n_edges();
        SurfaceMeshRemeshing(mesh).uniform_remeshing(len);
    } else { // Adaptive remeshing
        auto bb = mesh->bounding_box().diagonal();
        SurfaceMeshRemeshing(mesh).adaptive_remeshing(
                0.001 * bb,  // min length
                0.100 * bb,  // max length
                0.001 * bb); // approx. error
    }

    mesh->update();
    viewer()->update();
}


void MainWindow::surfaceMeshParameterization() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    SurfaceMeshParameterization para(mesh);

    bool LSCM = false;
    if (LSCM)  // Least Squares Conformal Map
        para.lscm();
    else        // Discrete Harmonic parameterization
        para.harmonic();

    mesh->update();
    viewer_->update();
    updateRenderingPanel();
}


void MainWindow::surfaceMeshGeodesic() {
    SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(viewer()->currentModel());
    if (!mesh)
        return;

    const auto& components = SurfaceMeshComponent::extract(mesh);
    if (components.size() > 1)
        LOG(WARNING) << "mesh has " << components.size() << " connected components. computed geodesic distance valid on component";

    // pick a few a random vertices and mark them locked
    auto lock = mesh->vertex_property<bool>("v:lock", false);
    lock.vector().assign(mesh->n_vertices(), false);

    // setup seeds
    std::vector<SurfaceMesh::Vertex> seeds;
    const int num_seeds = 2;
    for (int i=0; i<num_seeds; ++i) {
        const int idx = rand() % mesh->n_vertices();
        SurfaceMesh::Vertex v(idx);
        seeds.push_back(v);
        lock[v] = true;
    }

    // compute geodesic distance
    SurfaceMeshGeodesic geodist(mesh);
    geodist.compute(seeds);

    mesh->update();
    viewer_->update();
    updateRenderingPanel();
}


void MainWindow::surfaceMeshStitchCoincidentEdges() {
    SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(viewer()->currentModel());
    if (!mesh)
        return;

    LOG(WARNING) << "TODO: This stitching function is based purely on vertex positions and no adjacency information is used. "
                    "A stable stitching function taking into account the topology will be implemented in the near future.";

    class CmpVec {
    public:
        CmpVec(float _eps = FLT_MIN) : eps_(_eps) {}

        bool operator()(const vec3 &v0, const vec3 &v1) const {
            if (fabs(v0[0] - v1[0]) <= eps_) {
                if (fabs(v0[1] - v1[1]) <= eps_) {
                    return (v0[2] < v1[2] - eps_);
                } else return (v0[1] < v1[1] - eps_);
            } else return (v0[0] < v1[0] - eps_);
        }

    private:
        float eps_;
    };

    auto find_unique_vertex = [](SurfaceMesh* mesh, SurfaceMesh::Vertex v, std::map<vec3, SurfaceMesh::Vertex, CmpVec>& vMap) -> SurfaceMesh::Vertex {
        const vec3& p = mesh->position(v);
        // has vector been referenced before?
        auto it = vMap.find(p);
        if (it == vMap.end()) { // No : add vertex and remember idx/vector mapping
            vMap[p] = v;
            return v;
        }
        else // Yes : get index from map
            return it->second;
    };

    auto copy(*mesh);
    mesh->clear(); // clear the original mesh and it will be refilled.

    CmpVec comp(FLT_MIN);
    std::map<vec3, SurfaceMesh::Vertex, CmpVec> vMap(comp);

    auto points = copy.vertex_property<vec3>("v:point");
    for (auto v : copy.vertices())
        mesh->add_vertex(points[v]);
    for (auto f : copy.faces()) {
        std::vector<SurfaceMesh::Vertex> vertices;
        for (auto v : copy.vertices(f))
            vertices.push_back(find_unique_vertex(&copy, v, vMap));
        mesh->add_face(vertices);
    }

    mesh->update();
    viewer_->update();
}


void MainWindow::surfaceMeshRemoveIsolatedVertices() {
    SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(viewer()->currentModel());
    if (!mesh)
        return;

    // clean: remove isolated vertices
    for (auto v : mesh->vertices()) {
        if (mesh->is_isolated(v))
            mesh->delete_vertex(v);
    }
    mesh->garbage_collection();

    mesh->update();
    viewer_->update();
}


void MainWindow::pointCloudDelaunayTriangulation2D() {
    auto cloud = dynamic_cast<PointCloud*>(viewer()->currentModel());
    if (!cloud)
        return;

    const std::vector<vec3>& pts = cloud->points();

    std::vector<vec2> points;
    for (std::size_t i = 0; i < pts.size(); ++i) {
        points.push_back(vec2(pts[i]));
    }

    Delaunay2 delaunay;
    delaunay.set_vertices(points);

    SurfaceMesh* mesh = new SurfaceMesh;
    const std::string& name = file_system::parent_directory(cloud->name()) + "/" + file_system::base_name(cloud->name()) + "_delaunay_XY";
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

    const std::vector<vec3>& pts = cloud->points();
    Delaunay3 delaunay;
    delaunay.set_vertices(pts);

    LOG(WARNING) << "triangulation done. TODO: implement a data structure to store and visualize the result";

//    CGraph* whet_grid = new CGraph;
//    CGraphBuilder builder;
//    builder.set_target(whet_grid);
//    builder.begin_volume();
//
//    builder.build_meta_tetrahedron();
//
//    for (std::size_t i = 0; i < points.size(); i++) {
//        builder.add_vertex(points[i]);
//    }
//
//    std::cerr << "Nb tets = " << delaunay.nb_tets() << std::endl;
//    for (unsigned int i = 0; i < delaunay.nb_tets(); i++) {
//        builder.begin_cell(0);
//        for (int j = 0; j < 4; j++) {
//            int v = delaunay.tet_vertex(i, j);
//            mpl_debug_assert(v >= 0);
//            mpl_debug_assert(v < points.size());
//            builder.add_vertex_to_cell(v);
//        }
//        builder.end_cell();
//    }
//    builder.end_volume();
//
//    const std::string& name = FileUtils::dir_name(pset->name()) + "/" + FileUtils::base_name(pset->name()) + "_delaunay";
//    whet_grid->set_name(name);
//    main_window_->addModel(whet_grid, true, false);
}
