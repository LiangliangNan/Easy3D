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

#include "window.h"

#include <string>
#include <iostream>

#include <QFileDialog>
#include <QDropEvent>
#include <QMimeData>
#include <QSettings>
#include <QMessageBox>
#include <QColorDialog>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/poly_mesh_io.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/fileio/point_cloud_io_ptx.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/progress.h>
#include <easy3d/algo/surface_mesh_components.h>
#include <easy3d/algo/surface_mesh_topology.h>

#include "viewer.h"

#include <ui_window.h>


namespace easy3d {

    Window::Window(QWidget *parent)
            : QMainWindow(parent), ui(new Ui::Window) {
        ui->setupUi(this);

        viewer_ = new Viewer(this);
        connect(viewer_, SIGNAL(currentModelChanged()), this, SLOT(onCurrentModelChanged()));
        setCentralWidget(viewer_);

        createActions();

        setWindowIcon(QIcon(QString::fromStdString(":/resources/icons/ViewerQt.png")));
        setFocusPolicy(Qt::StrongFocus);
        setContextMenuPolicy(Qt::CustomContextMenu);
        setAcceptDrops(true);

        setBaseSize(1280, 960);
//    setWindowState(Qt::WindowMaximized);

        readSettings();
        updateWindowTitle();
    }


    void Window::dragEnterEvent(QDragEnterEvent *e) {
        if (e->mimeData()->hasUrls())
            e->acceptProposedAction();
    }


    void Window::dropEvent(QDropEvent *e) {
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


    bool Window::onOpen() {
        const QStringList &fileNames = QFileDialog::getOpenFileNames(
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

        int count = 0;
        ProgressLogger progress(fileNames.size(), false, false);
        for (const auto &name: fileNames) {
            if (progress.is_canceled()) {
                LOG(WARNING) << "opening files cancelled";
                break;
            }
            if (open(name.toStdString()))
                ++count;
            progress.next();
        }
        if (count > 0)
            viewer_->update();

        return count > 0;
    }


    bool Window::onSave() {
        const Model *model = viewer_->currentModel();
        if (!model) {
            std::cerr << "no model exists" << std::endl;
            return false;
        }

        std::string default_file_name = model->name();
        if (file_system::extension(default_file_name).empty()) // no extension?
            default_file_name += ".ply"; // default to ply

        const QString &fileName = QFileDialog::getSaveFileName(
                this,
                "Open file(s)",
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
        if (dynamic_cast<const PointCloud *>(model)) {
            const auto cloud = dynamic_cast<const PointCloud *>(model);
            saved = PointCloudIO::save(fileName.toStdString(), cloud);
        } else if (dynamic_cast<const SurfaceMesh *>(model)) {
            const auto mesh = dynamic_cast<const SurfaceMesh *>(model);
            saved = SurfaceMeshIO::save(fileName.toStdString(), mesh);
        } else if (dynamic_cast<const Graph *>(model)) {
            const auto graph = dynamic_cast<const Graph *>(model);
            saved = GraphIO::save(fileName.toStdString(), graph);
        }

        if (saved) {
            std::cout << "model successfully saved to: " << fileName.toStdString();
            setCurrentFile(fileName);
            return true;
        }

        return false;
    }


    Model *Window::open(const std::string &file_name) {
        auto models = viewer_->models();
        for (auto m: models) {
            if (m->name() == file_name) {
                LOG(WARNING) << "model already loaded: " << file_name;
                return nullptr;
            }
        }

        const std::string &ext = file_system::extension(file_name, true);
        bool is_ply_mesh = false;
        if (ext == "ply")
            is_ply_mesh = (io::PlyReader::num_instances(file_name, "face") > 0);

        Model *model = nullptr;
        if ((ext == "ply" && is_ply_mesh) || ext == "obj" || ext == "off" || ext == "stl" || ext == "sm" ||
            ext == "plg") { // mesh
            model = SurfaceMeshIO::load(file_name);
        } else if (ext == "ply" && io::PlyReader::num_instances(file_name, "edge") > 0) {
            model = GraphIO::load(file_name);
        } else if (ext == "plm" || ext == "pm" || ext == "mesh") {
            model = PolyMeshIO::load(file_name);
        } else { // point cloud
            if (ext == "ptx") {
                io::PointCloudIO_ptx serializer(file_name);
                while (auto cloud = serializer.load_next())
                    viewer_->addModel(cloud);
            } else
                model = PointCloudIO::load(file_name);
        }

        if (model) {
            model->set_name(file_name);
            viewer_->addModel(model);
            setCurrentFile(QString::fromStdString(file_name));
        }

        return model;
    }


    void Window::onCurrentModelChanged() {
        const Model *m = viewer_->currentModel();
        if (m) {
            viewer_->fitScreen(m);

            const std::string &name = m->name();
            setCurrentFile(QString::fromStdString(name));
        } else
            updateWindowTitle();
    }


    void Window::setCurrentFile(const QString &fileName) {
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


    void Window::onOpenRecentFile() {
        if (okToContinue()) {
            auto action = qobject_cast<QAction *>(sender());
            if (action) {
                const QString filename(action->data().toString());
                if (open(filename.toStdString()))
                    viewer_->update();
            }
        }
    }


    void Window::onClearRecentFiles() {
        recentFiles_.clear();
        updateRecentFileActions();
    }


    void Window::saveSnapshot() {
        const Model *model = viewer_->currentModel();

        const bool overwrite = false;
        std::string default_file_name("untitled.png");
        if (model)
            default_file_name = file_system::replace_extension(model->name(), "png");
        const QString fileName = QFileDialog::getSaveFileName(
                this,
                "Please choose a file name",
                QString::fromStdString(default_file_name),
                "Image Files (*.png *.jpg *.bmp *.ppm)\n"
                "PNG (*.png)\n"
                "JPG (*.jpg)\n"
                "Windows Bitmap (*.bmp)\n"
                "24bit RGB Bitmap (*.ppm)\n"
                "All Files (*.*)",
                nullptr,
                overwrite ? QFileDialog::DontConfirmOverwrite : QFileDialog::Option()
        );
        // Hide closed dialog
        QApplication::processEvents();

        if (fileName.isEmpty())
            return;

        viewer_->saveSnapshot(fileName);
    }


    void Window::setBackgroundColor() {
        const vec4 &c = viewer_->backGroundColor();
        QColor orig(static_cast<int>(c.r * 255), static_cast<int>(c.g * 255), static_cast<int>(c.b * 255),
                    static_cast<int>(c.a * 255));
        const QColor &color = QColorDialog::getColor(orig, this);
        if (color.isValid()) {
            const vec4 newColor(static_cast<float>(color.redF()), static_cast<float>(color.greenF()), static_cast<float>(color.blueF()), static_cast<float>(color.alphaF()));
            viewer_->setBackgroundColor(newColor);
            viewer_->update();
        }
    }


    bool Window::okToContinue() {
        if (isWindowModified()) {
            int r = QMessageBox::warning(this, tr("Viewer"),
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


    void Window::onAbout() {
        const std::string name = "<h3>" + std::string(EXAMPLE_TITLE) + "</h3>";
        QString title = QMessageBox::tr(name.c_str());

        QString text = QMessageBox::tr(
                "<p>This viewer shows how to use Qt for GUI creation and event handling</p>"
                "<p>Liangliang Nan<br>"
                "<a href=\"mailto:liangliang.nan@gmail.com\">liangliang.nan@gmail.com</a><br>"
                "<a href=\"https://3d.bk.tudelft.nl/liangliang/\">https://3d.bk.tudelft.nl/liangliang/</a></p>"
        );

        //QMessageBox::about(this, title, text);
        QMessageBox::about(this, "About Viewer", title + text);
    }


    void Window::readSettings() {
        QSettings settings("liangliang.nan@gmail.com", "Viewer");
        recentFiles_ = settings.value("recentFiles").toStringList();
        updateRecentFileActions();
        curDataDirectory_ = settings.value("currentDirectory").toString();
    }


    void Window::writeSettings() {
        QSettings settings("liangliang.nan@gmail.com", "Viewer");
        settings.setValue("recentFiles", recentFiles_);
        if (!curDataDirectory_.isEmpty() && file_system::is_directory(curDataDirectory_.toStdString()))
            settings.setValue("currentDirectory", curDataDirectory_);
    }


    void Window::updateWindowTitle() {
        Model *model = viewer_->currentModel();

        QString title = QString::fromStdString(EXAMPLE_TITLE);
#ifndef NDEBUG
        title += " (Debug Version)";
#endif // NDEBUG

        QString fileName("Untitled");
        if (model)
            fileName = QString::fromStdString(model->name());

        title = tr("%1[*] - %2").arg(strippedName(fileName), title);
        setWindowTitle(title);
    }


    void Window::closeEvent(QCloseEvent *event) {
        if (okToContinue()) {
            writeSettings();
            event->accept();
        } else {
            event->ignore();
        }
    }


    void Window::updateRecentFileActions() {
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


    QString Window::strippedName(const QString &fullFileName) {
        return QFileInfo(fullFileName).fileName();
    }


    void Window::createActions() {
        // file menu
        createActionsForFileMenu();

        // view menu
        createActionsForViewMenu();

        // topology menu
        createActionsForTopologyMenu();

        // about menu
        connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));
    }


    void Window::createActionsForFileMenu() {
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


    void Window::createActionsForViewMenu() {
        connect(ui->actionSnapshot, SIGNAL(triggered()), this, SLOT(saveSnapshot()));

        ui->menuView->addSeparator();

        connect(ui->actionSetBackgroundColor, SIGNAL(triggered()), this, SLOT(setBackgroundColor()));
    }


    void Window::createActionsForTopologyMenu() {
        connect(ui->actionTopologyStatistics, SIGNAL(triggered()), this, SLOT(reportTopologyStatistics()));
    }


    void Window::reportTopologyStatistics() {
        auto mesh = dynamic_cast<SurfaceMesh *>(viewer()->currentModel());
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
        for (auto v: mesh->vertices()) {
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

            std::cout << "\t\t" << i << ": "
                      << type
                      << ", #face = " << comp.n_faces() << ", #vertex = " << comp.n_vertices() << ", #edge = "
                      << comp.n_edges()
                      << ", #border = " << topo.number_of_borders();
            if (topo.number_of_borders() == 1)
                std::cout << ", border size = " << topo.largest_border_size();
            else if (topo.number_of_borders() > 1)
                std::cout << ", largest border size = " << topo.largest_border_size();
            std::cout << std::endl;
        }
    }

}