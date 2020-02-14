#ifndef DIALOG_SAMPLING_RESOLUTION_H
#define DIALOG_SAMPLING_RESOLUTION_H

#include <easy3d/core/point_cloud.h>

#include <QDialog>


namespace Ui {
    class DialogPointCloudSimplification;
}

namespace easy3d {
    class KdTreeSearch;
}

class PaintCanvas;

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
