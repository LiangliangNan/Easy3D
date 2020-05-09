#include "dialog_surface_mesh_from_text.h"

#include <easy3d/algo/text_mesher.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/util/file_system.h>

#include <QFileDialog>

#include "main_window.h"
#include "paint_canvas.h"
#include "ui_dialog_surface_mesh_from_text.h"


using namespace easy3d;

DialogSurfaceMeshFromText::DialogSurfaceMeshFromText(MainWindow *window, QDockWidget *dockWidgetCommand) :
        Dialog(window, dockWidgetCommand),
        ui(new Ui::DialogSurfaceMeshFromText) {
    ui->setupUi(this);
    font_path_ = resource::directory() + "/fonts/";

    connect(ui->toolButtonTFontFile, SIGNAL(clicked()), this, SLOT(setFontFile()));
    connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(apply()));

    bestSize();
}


DialogSurfaceMeshFromText::~DialogSurfaceMeshFromText() {
    delete ui;
}


void DialogSurfaceMeshFromText::setFontFile() {
    const std::string dir = resource::directory() + "/fonts";
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose a font file"), QString::fromStdString(dir),
                                                    tr("True Type Font (*.ttf)")
    );

    if (fileName.isEmpty())
        return;

    const std::string &file_name = fileName.toStdString();
    font_path_ = file_system::parent_directory(file_name);
    const std::string &font_file = file_system::simple_name(file_name);
    ui->lineEditFontFile->setText(QString::fromStdString(font_file));
}


void DialogSurfaceMeshFromText::apply() {
#if HAS_TEXT_MESHER
    const std::string &text = ui->lineEditText->text().toStdString();
    if (text.empty()) {
        LOG(WARNING) << "text is empty";
        return;
    }

    const std::string &font_file = font_path_ + "/" + ui->lineEditFontFile->text().toStdString();
    if (!file_system::is_file(font_file)) {
        LOG(WARNING) << "font file does not exit: " << font_file;
        return;
    }

    int font_size = ui->spinBoxFontSize->value();
    if (font_size < 5) {
        LOG(WARNING) << "font size must be >= 5";
        return;
    }

    int extrusion = ui->spinBoxExtrusion->value();
    if (extrusion < 1) {
        LOG(WARNING) << "extrusion value must be positive";
        return;
    }

    bool collision_free = ui->checkBoxCollisionFree->isChecked();

    TextMesher mesher(font_file, font_size);

    auto mesh = dynamic_cast<SurfaceMesh *>(viewer_->currentModel());
    if (mesh && mesh->name() == text) {
        mesh->clear();
        mesher.generate(mesh, text, 0, 0, extrusion, collision_free);
        mesh->update();
    } else {
        mesh = mesher.generate(text, 0, 0, extrusion, collision_free);
        if (mesh) {
            mesh->set_name(text);
            viewer_->addModel(mesh);
            viewer_->fitScreen(mesh);
        }
    }

    window_->updateUi();
#else
    LOG(WARNING) << "TextMesher requires FreeType but FreeType was not found.";
#endif
}