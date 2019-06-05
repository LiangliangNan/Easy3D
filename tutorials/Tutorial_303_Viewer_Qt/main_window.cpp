/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: a lightweight, easy-to-use, and efficient C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

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
#include <easy3d/core/point_cloud.h>
#include <easy3d/viewer/model.h>
#include <easy3d/viewer/drawable.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/fileio/point_cloud_io_ptx.h>
#include <easy3d/util/file.h>

#include "viewer_qt.h"

#include <ui_main_window.h>


using namespace easy3d;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    viewer_ = new ViewerQt(this);
    connect(viewer_, SIGNAL(currentModelChanged()), this, SLOT(onCurrentModelChanged()));
    setCentralWidget(viewer_);

    createActions();

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
                "Supported formats (*.ply *.obj *.off *.stl *.poly *.plg *.geojson *.bin *.ptx *.las *.laz *.xyz *.bxyz *.vg *.bvg *.mesh *.meshb *.tet)\n"
                "Mesh formats (*.ply *.obj *.off *.stl *.poly *.plg)\n"
                "Point set formats (*.ply *.bin *.ptx *.las *.laz *.xyz *.bxyz *.vg *.bvg)\n"
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
    if (viewer_->models().empty())
        return false;

    if (viewer_->currentModel()) {
        const Model* m = viewer_->currentModel();
        const QString& fileName = QFileDialog::getSaveFileName(
                    this,
                    "Open file(s)",
                    QString::fromStdString(m->name()),
                    "Supported formats (*.ply *.obj *.off *.stl *.poly *.plg *.bin *.las *.laz *.xyz *.bxyz *.vg *.bvg *.mesh *.meshb *.tet)\n"
                    "Mesh formats (*.ply *.obj *.off *.stl *.poly *.plg)\n"
                    "Point set formats (*.ply *.bin *.ptx *.las *.laz *.xyz *.bxyz *.vg *.bvg)\n"
                    "All formats (*.*)"
        );

        if (fileName.isEmpty())
            return false;

        bool saved = false;
        if (dynamic_cast<const PointCloud*>(m)) {
            const PointCloud* cloud = dynamic_cast<const PointCloud*>(m);
            saved = PointCloudIO::save(fileName.toStdString(), cloud);
        }
        else if (dynamic_cast<const SurfaceMesh*>(m)) {
            const SurfaceMesh* mesh = dynamic_cast<const SurfaceMesh*>(m);
            saved = saved = SurfaceMeshIO::save(fileName.toStdString(), mesh);
        }

        if (saved) {
            std::cout << "model successfully saved to: \'" << fileName.toStdString() << "\'" << std::endl;
            setCurrentFile(fileName);
            return true;
        }
    }

    return false;
}


Model* MainWindow::open(const std::string& file_name) {
    auto models = viewer_->models();
    for (auto m : models) {
        if (m->name() == file_name) {
            std::cerr << "model alreaded loaded: \'" << file_name << "\'" << std::endl;
            return nullptr;
        }
    }

    const std::string& ext = file::extension(file_name, true);
    bool is_ply_mesh = false;
    if (ext == "ply")
        is_ply_mesh = (io::PlyReader::num_faces(file_name) > 0);

    Model* model = nullptr;
    if ((ext == "ply" && is_ply_mesh) || ext == "obj" || ext == "off" || ext == "stl" || ext == "poly" || ext == "plg") { // mesh
        SurfaceMesh* mesh = SurfaceMeshIO::load(file_name);
        if (mesh) {
            model = mesh;
            std::cout << "mesh loaded. num faces: " << mesh->n_faces() << "; "
                << "num vertices: " << mesh->n_vertices() << "; "
                << "num edges: " << mesh->n_edges() << std::endl;
        }
    }
    else { // point cloud
        if (ext == "ptx") {
            io::PointCloudIO_ptx serializer(file_name);
            PointCloud* cloud = nullptr;
            while ((cloud = serializer.load_next())) {
                viewer_->addModel(cloud);
                std::cout << "cloud loaded. num vertices: " << cloud->n_vertices() << std::endl;
            }
            setCurrentFile(QString::fromStdString(file_name));
        }
        else {
            PointCloud* cloud = PointCloudIO::load(file_name);
            if (cloud) {
                model = cloud;
                std::cout << "cloud loaded. num vertices: " << cloud->n_vertices() << std::endl;
            }
        }
    }

    if (model) {
        model->set_name(file_name);
		viewer_->makeCurrent();
        viewer_->addModel(model);
		viewer_->doneCurrent();
        setCurrentFile(QString::fromStdString(file_name));
        return model;
    }

    return nullptr;
}


void MainWindow::onCurrentModelChanged() {
    const Model* m = viewer_->currentModel();
    if (m) {
        viewer_->fitScreen(m);

        const std::string& name = m->name();
        setCurrentFile(QString::fromStdString(name));
    }
}


void MainWindow::setCurrentFile(const QString &fileName)
{
    QString dir = fileName.left(fileName.lastIndexOf("/"));
    if (!dir.isEmpty() && file::is_directory(dir.toStdString()))
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
    const bool overwrite = false;
    const QString proposedFileName = curDataDirectory_ + "/" + "snapshot.png";
    QString proposedFormat = "PNG (*.png)";
    const QString fileName = QFileDialog::getSaveFileName(
        this,
        "Choose an image file name",
        proposedFileName,
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

    viewer_->saveSnapshot(fileName);
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
        int r = QMessageBox::warning(this, tr("ViewerQt"),
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
    QString title = QMessageBox::tr("<h3>ViewerQt</h3>");

    QString text = QMessageBox::tr(
        "<p>This viewer shows how to use Qt for GUI creation and event handling</p>"
        "<p>Liangliang Nan<br>"
        "<a href=\"mailto:liangliang.nan@gmail.com\">liangliang.nan@gmail.com</a><br>"
        "<a href=\"https://3d.bk.tudelft.nl/liangliang/\">https://3d.bk.tudelft.nl/liangliang/</a></p>"
        );

    //QMessageBox::about(this, title, text);
    QMessageBox::about(this, "About ViewerQt", title + text);
}


void MainWindow::readSettings()
{
    QSettings settings("liangliang.nan@gmail.com", "ViewerQt");
    recentFiles_ = settings.value("recentFiles").toStringList();
    updateRecentFileActions();
    curDataDirectory_ = settings.value("currentDirectory").toString();
}


void MainWindow::writeSettings()
{
    QSettings settings("liangliang.nan@gmail.com", "ViewerQt");
    settings.setValue("recentFiles", recentFiles_);
    if (!curDataDirectory_.isEmpty() && file::is_directory(curDataDirectory_.toStdString()))
        settings.setValue("currentDirectory", curDataDirectory_);
}


void MainWindow::updateWindowTitle() {
    Model* model = viewer_->currentModel();

#ifndef NDEBUG
    QString title = "ViewerQt (Debug Version)";
#else
    QString title = "ViewerQt";
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

    // about menu
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));
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

    connect(ui->actionBackgroundColor, SIGNAL(triggered()), this, SLOT(setBackgroundColor()));
}


void MainWindow::createActionsForTopologyMenu() {
    connect(ui->actionTopologyStatistics, SIGNAL(triggered()), this, SLOT(reportTopologyStatistics()));
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
