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
#include <easy3d/viewer/model.h>
#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/renderer.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/fileio/point_cloud_io_ptx.h>
#include <easy3d/algo/surface_mesh_sampler.h>
#include <easy3d/algo/point_cloud_normals.h>
#include <easy3d/algo_ext/mesh_surfacer.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/stop_watch.h>

#include "paint_canvas.h"
#include "dialog_snapshot.h"
#include "dialog_poisson_reconstruction.h"
#include "dialog_surface_sampling.h"
#include "dialog_ransac_primitive_extraction.h"
#include "widget_lighting.h"
#include "widget_point_cloud_renderer.h"
#include "widget_surface_mesh_renderer.h"
#include "widget_surface_mesh_texturing.h"
#include "widget_scalar_field.h"
#include "widget_vector_field.h"

#include <ui_main_window.h>


using namespace easy3d;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , dialogSanpshot_(nullptr)
    , dialogPoissonReconstruction_(nullptr)
    , dialogRansacPrimitiveExtraction_(nullptr)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    viewer_ = new PaintCanvas(this);
    connect(viewer_, SIGNAL(currentModelChanged()), this, SLOT(onCurrentModelChanged()));
    setCentralWidget(viewer_);

#ifndef _WIN32
    const QSize& size = ui->dockWidgetRendering->sizeHint();
    int width = static_cast<int>(size.width() * 1.5f);
    ui->dockWidgetRendering->setFixedWidth(width);
#endif

    // ----- rendering panel ------

    widgetCloudRenderer_ = new WidgetPointCloudRenderer(this);
    ui->verticalLayoutRenderer->addWidget(widgetCloudRenderer_);
    widgetCloudRenderer_->hide();

    widgetMeshRenderer_ = new WidgetSurfaceMeshRenderer(this);
    ui->verticalLayoutRenderer->addWidget(widgetMeshRenderer_);

    widgetMeshTexturing_ = new WidgetSurfaceMeshTexturing(this);
    ui->verticalLayoutTexturing->addWidget(widgetMeshTexturing_);

    widgetScalarField_ = new WidgetScalarField(this);
    ui->verticalLayoutScalarField->addWidget(widgetScalarField_);

    widgetVectorField_ = new WidgetVectorField(this);
    ui->verticalLayoutVectorField->addWidget(widgetVectorField_);

    ui->verticalLayoutLighting->addWidget(new WidgetLighting(this));
//  ui->toolBox->addItem(new WidgetLighting(this), QIcon("Resources/icons/light.png"), QString("Lighting"));

    int count = ui->toolBox->count();
    for (int i=0; i<count; ++i)
        ui->toolBox->widget(i)->setEnabled(false);

    // ---------------------------

    createActions();

    setWindowIcon(QIcon("Resources/icons/Mapple.png"));
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops(true);

    setBaseSize(1280, 960);
//    setWindowState(Qt::WindowMaximized);

    readSettings();
    updateWindowTitle();
}

MainWindow::~MainWindow()
{
    if (dialogSanpshot_)                delete dialogSanpshot_;
    if (dialogPoissonReconstruction_)   delete dialogPoissonReconstruction_;
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
    if (file_system::extension(default_file_name).empty()) // no extention?
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
        std::cout << "model successfully saved to: \'" << fileName.toStdString() << "\'" << std::endl;
        setCurrentFile(fileName);
        return true;
    }

    return false;
}


Model* MainWindow::open(const std::string& file_name, bool create_default_drawables /* = true*/) {
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
                viewer_->makeCurrent();
                viewer_->addModel(cloud, create_default_drawables);
                viewer_->doneCurrent();
            }
        }
        else
            model = PointCloudIO::load(file_name);
    }

    if (model) {
        model->set_name(file_name);
        viewer_->makeCurrent();
        viewer_->addModel(model, create_default_drawables);
        viewer_->doneCurrent();
        setCurrentFile(QString::fromStdString(file_name));
    }

    return model;
}


void MainWindow::onCurrentModelChanged() {
    const Model* m = viewer_->currentModel();
    if (m) {



        if (true) {
            viewer_->fitScreen(m);
        }






        const std::string& name = m->name();
        setCurrentFile(QString::fromStdString(name));

        if (dynamic_cast<const PointCloud*>(m)) {
            if (widgetMeshRenderer_)
                widgetMeshRenderer_->hide();
            widgetCloudRenderer_->show();
            widgetCloudRenderer_->ensureBuffers();
            widgetCloudRenderer_->updatePanel();
         }
        else if (dynamic_cast<const SurfaceMesh*>(m)) {
            if (widgetCloudRenderer_)
                widgetCloudRenderer_->hide();
            widgetMeshRenderer_->show();
            widgetMeshRenderer_->ensureBuffers();
            widgetMeshRenderer_->updatePanel();
        }

        widgetScalarField_->updatePanel();
        widgetVectorField_->updatePanel();

        int count = ui->toolBox->count();
        for (int i=0; i<count; ++i)
             ui->toolBox->widget(i)->setEnabled(true);
    }
    else {
        setCurrentFile(QString(""));

        widgetCloudRenderer_->hide();
        widgetMeshRenderer_->show();
        int count = ui->toolBox->count();
        for (int i=0; i<count; ++i)
            ui->toolBox->widget(i)->setEnabled(false);
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

    if (!dialogSanpshot_) {
        dialogSanpshot_ = new DialogSnapshot(this);
        connect(viewer_, SIGNAL(resized()), dialogSanpshot_, SLOT(computeImageSize()));
    }

    dialogSanpshot_->setImageFileName(fileName);
    dialogSanpshot_->show();
    dialogSanpshot_->raise();
    dialogSanpshot_->activateWindow();
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


void MainWindow::createActions() {
    // file menu
    createActionsForFileMenu();

    // view menu
    createActionsForViewMenu();

    // topology menu
    createActionsForTopologyMenu();

    // orientation menu
    createActionsForOrientationMenu();

    // conversion menu
    createActionsForConversionMenu();

    // about menu
    connect(ui->actionAboutMapple, SIGNAL(triggered()), this, SLOT(onAboutMapple()));
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


void MainWindow::createActionsForTopologyMenu() {
    connect(ui->actionTopologyStatistics, SIGNAL(triggered()), this, SLOT(reportTopologyStatistics()));

    connect(ui->actionDetectDuplicatedFaces, SIGNAL(triggered()), this, SLOT(detectDuplicatedFaces()));
    connect(ui->actionRemoveDuplicatedFaces, SIGNAL(triggered()), this, SLOT(removeDuplicatedFaces()));

    connect(ui->actionDetectSelfIntersections, SIGNAL(triggered()), this, SLOT(detectSelfIntersections()));
    connect(ui->actionRemeshSelfIntersections, SIGNAL(triggered()), this, SLOT(remeshSelfIntersections()));
}


void MainWindow::createActionsForOrientationMenu() {
    connect(ui->actionEstimatePointCloudNormals, SIGNAL(triggered()), this, SLOT(estimatePointCloudNormals()));
    connect(ui->actionReorientPointCloudNormals, SIGNAL(triggered()), this, SLOT(reorientPointCloudNormals()));
}


void MainWindow::createActionsForConversionMenu() {
    connect(ui->actionPoissonSurfaceReconstruction, SIGNAL(triggered()), this, SLOT(poissonSurfaceReconstruction()));
    connect(ui->actionRansacPrimitiveExtraction, SIGNAL(triggered()), this, SLOT(ransacPrimitiveExtraction()));
    connect(ui->actionSamplingSurfaceMesh, SIGNAL(triggered()), this, SLOT(samplingSurfaceMesh()));
}


void MainWindow::reportTopologyStatistics() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    std::cout << "#face:   " << mesh->faces_size() << std::endl;
    std::cout << "#vertex: " << mesh->vertices_size() << std::endl;
    std::cout << "#edge:   " << mesh->edges_size() << std::endl;
    std::cout << "#connected component:   " << "not implemented yet" << std::endl;
    std::cout << "..." << std::endl;
}


void MainWindow::detectDuplicatedFaces() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    StopWatch w;
    w.start();
    std::cerr << "detecting duplicated faces..." << std::endl;

    MeshSurfacer ms;
    const auto& faces = ms.detect_duplicated_faces(mesh, true);
    if (!faces.empty())
        std::cerr << "done. " << faces.size() << " faces duplicating others. Time: " << w.time_string() << std::endl;
    else
        std::cerr << "done. No duplicated faces detected" << w.time_string() << std::endl;
}


void MainWindow::removeDuplicatedFaces() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    StopWatch w;
    w.start();
    std::cerr << "removing duplicated faces..." << std::endl;

    MeshSurfacer ms;
    unsigned int num = ms.remove_duplicated_faces(mesh, true);
    if (num > 0) {
        viewer()->makeCurrent();
        renderer::update_data(mesh, mesh->triangles_drawable("faces"));
        viewer()->doneCurrent();
        update();
        std::cerr << "done. " << num << " faces deleted. Time: " << w.time_string() << std::endl;
    }
    else
        std::cerr << "done. No duplicated faces detected" << w.time_string() << std::endl;
}


void MainWindow::detectSelfIntersections() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    StopWatch w;
    w.start();
    std::cerr << "detecting intersecting faces..." << std::endl;

    MeshSurfacer ms;
    const auto& pairs = ms.detect_self_intersections(mesh);
    if (!pairs.empty())
        std::cerr << "done. " << pairs.size() << " pairs of faces intersect. Time: " << w.time_string() << std::endl;
    else
        std::cerr << "done. No intersecting faces detected" << w.time_string() << std::endl;
 }


void MainWindow::remeshSelfIntersections() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    StopWatch w;
    w.start();
    std::cerr << "remeshing intersecting faces..." << std::endl;

    auto size = mesh->faces_size();
    MeshSurfacer ms;
    SurfaceMesh* result = ms.remesh_self_intersections(mesh, true);
    if (result) {
        const std::string& name = file_system::name_less_extension(mesh->name()) + "_remeshed." + file_system::extension(mesh->name());
        result->set_name(name);
        viewer()->makeCurrent();
        viewer()->addModel(result);
        viewer()->doneCurrent();
        std::cerr << "done. #faces " << size << " -> " << result->faces_size() << ". Time: " << w.time_string() << std::endl;
    }
    else
        std::cerr << "done. No intersecting faces detected" << w.time_string() << std::endl;
}


void MainWindow::estimatePointCloudNormals() {
    PointCloud* cloud = dynamic_cast<PointCloud*>(viewer()->currentModel());
    if (!cloud)
        return;

    PointCloudNormals pcn;
    std::cout << "show the parameter dialog" << std::endl;
    pcn.estimate(cloud);

    auto normals = cloud->get_vertex_property<vec3>("v:normal");
    if (normals) {
        viewer_->makeCurrent();
        PointsDrawable* vertices = cloud->points_drawable("vertices");
        vertices->update_normal_buffer(normals.vector());
        viewer_->doneCurrent();
        viewer_->update();
    }
}


void MainWindow::reorientPointCloudNormals() {
    PointCloud* cloud = dynamic_cast<PointCloud*>(viewer()->currentModel());
    if (!cloud)
        return;

    viewer_->makeCurrent();
    PointCloudNormals pcn;
    std::cout << "show the parameter dialog" << std::endl;
    pcn.reorient(cloud);

    auto normals = cloud->get_vertex_property<vec3>("v:normal");
    if (normals) {
        viewer_->makeCurrent();
        PointsDrawable* vertices = cloud->points_drawable("vertices");
        vertices->update_normal_buffer(normals.vector());
        viewer_->doneCurrent();
        viewer_->update();
    }
    viewer_->doneCurrent();
}


void MainWindow::poissonSurfaceReconstruction() {
    if (!dialogPoissonReconstruction_)
        dialogPoissonReconstruction_ = new DialogPoissonReconstruction(this);

    dialogPoissonReconstruction_->show();
    dialogPoissonReconstruction_->raise();
    dialogPoissonReconstruction_->activateWindow();
}


void MainWindow::ransacPrimitiveExtraction() {
    if (!dialogRansacPrimitiveExtraction_)
        dialogRansacPrimitiveExtraction_ = new DialogRansacPrimitiveExtraction(this);

    dialogRansacPrimitiveExtraction_->setWorkOnSelectedPoints(false);
    dialogRansacPrimitiveExtraction_->show();
    dialogRansacPrimitiveExtraction_->raise();
    dialogRansacPrimitiveExtraction_->activateWindow();
}


void MainWindow::samplingSurfaceMesh() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer()->currentModel());
    if (!mesh)
        return;

    DialogSurfaceSampling dlg;
    if (dlg.exec()) {
        int num = dlg.pointNumber();
        if (static_cast<unsigned int>(num) < mesh->n_vertices()) {
            LOG(WARNING) << "point num must >= the num of vertices of the input mesh";
            return;
        }

        SurfaceMeshSampler sampler;
        PointCloud* cloud = sampler.apply(mesh, num);
        if (cloud) {
            viewer_->makeCurrent();
            viewer_->addModel(cloud);
            viewer_->doneCurrent();
        }
    }
}
