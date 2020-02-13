#ifndef DLG_SAMPLING_RESOLUTION_H
#define DLG_SAMPLING_RESOLUTION_H

#include <easy3d/core/point_cloud.h>

#include <QDialog>
#include "ui_dialog_point_cloud_simplification.h"

namespace Ui {
    class DialogSurfaceMeshSampling;
}

class PaintCanvas;

namespace easy3d {
    class KdTreeSearch;
}


class DialogPointCloudSimplification : public QDialog
{
	Q_OBJECT

public:
	DialogPointCloudSimplification(QWidget* parent);
	~DialogPointCloudSimplification();

private Q_SLOTS:
	void strategyChanged(int id);

	void computeAvgSpacing();
	void apply();
	void query();

protected:
	virtual void closeEvent(QCloseEvent* e);
	virtual void showEvent(QShowEvent* e);

	void constructKdTree();

private:
    Ui::DialogPointCloudSimplification *ui;

	PaintCanvas*	 viewer_;
	easy3d::KdTreeSearch*    kdtree_;
	std::vector<easy3d::PointCloud::Vertex> points_to_remove_;
};

#endif // DLG_SAMPLING_RESOLUTION_H
