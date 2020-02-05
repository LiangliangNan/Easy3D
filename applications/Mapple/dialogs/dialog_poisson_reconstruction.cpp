#include "dialog_poisson_reconstruction.h"
#include "main_window.h"
#include "paint_canvas.h"
#include "ui_dialog_poisson_reconstruction.h"


#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/algo/point_cloud_poisson_reconstruction.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>


using namespace easy3d;

DialogPoissonReconstruction::DialogPoissonReconstruction(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPoissonReconstruction)
{
    ui->setupUi(this);
    viewer_ = dynamic_cast<MainWindow*>(parent)->viewer();

    // default value
    default_octree_depth_ = 8;
    default_samples_per_node_ = 1;
    default_trim_value_ = 6.0;
    default_area_ratio_ = 0.001;

    density_attr_name_ = "v:density";

    resetParameters();
    showHint(false);

    connect(ui->pushButtonHelp, SIGNAL(toggled(bool)), this, SLOT(showHint(bool)));
    connect(ui->pushButtonDefault, SIGNAL(clicked()), this, SLOT(resetParameters()));
    connect(ui->pushButtonReconstruct, SIGNAL(clicked()), this, SLOT(reconstruct()));
    connect(ui->pushButtonTrim, SIGNAL(clicked()), this, SLOT(trim()));
}

DialogPoissonReconstruction::~DialogPoissonReconstruction()
{
    delete ui;
}


void DialogPoissonReconstruction::resetParameters() {
    ui->spinBoxOctreeDepth->setValue(default_octree_depth_);
    ui->spinBoxSamplesPerNode->setValue(default_samples_per_node_);

    ui->doubleSpinBoxTrimValue->setValue(default_trim_value_);
    ui->doubleSpinBoxIslandAreaRatio->setValue(default_area_ratio_);
}

void DialogPoissonReconstruction::showHint(bool b) {
    if (b)
        ui->widgetHint->show();
    else
        ui->widgetHint->hide();
}

void DialogPoissonReconstruction::reconstruct() {
    PointCloud* cloud = dynamic_cast<PointCloud*>(viewer_->currentModel());
    if (cloud) {
        int		octree_depth = ui->spinBoxOctreeDepth->value();
        float	sampers_per_node = ui->spinBoxSamplesPerNode->value();

        PoissonReconstruction recon;
        recon.set_depth(octree_depth);
        recon.set_sampers_per_node(sampers_per_node);

        SurfaceMesh* mesh = recon.apply(cloud, density_attr_name_);
        if (mesh) {
            const std::string& name = file_system::name_less_extension(cloud->name()) + "_poisson_reconstruction.ply";
            mesh->set_name(name);
            viewer_->makeCurrent();
            viewer_->addModel(mesh, true);
            viewer_->doneCurrent();
        }
    }
}


void DialogPoissonReconstruction::trim() {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer_->currentModel());
    if (mesh) {
        double	trim_value = ui->doubleSpinBoxTrimValue->value();
        double  area_ratio = ui->doubleSpinBoxIslandAreaRatio->value();
        bool    triangulate = false; // I can do it using my algorithm :-)

        auto density = mesh->vertex_property<float>(density_attr_name_);
        if (!density) {
            LOG(WARNING) << "no property \'density\' for trimming";
            return;
        }

        float min_density = FLT_MAX;
        float max_density = -FLT_MAX;
        for (auto v : mesh->vertices()) {
            float value = density[v];
            min_density = std::min(min_density, value);
            max_density = std::max(max_density, value);
        }

        if (trim_value <= min_density || trim_value >= max_density) {
            LOG(WARNING) << "trim value (" << trim_value
                << ") out of density range [" << min_density << ", " << max_density << "]";
            return;
        }

        SurfaceMesh* trimmed_mesh = PoissonReconstruction::trim(mesh, density_attr_name_, trim_value, area_ratio, triangulate);
        if (trimmed_mesh) {
            const std::string& name = file_system::name_less_extension(mesh->name()) + "_trimmed.ply";
            trimmed_mesh->set_name(name);
            viewer_->addModel(trimmed_mesh);
        }
    }
}

