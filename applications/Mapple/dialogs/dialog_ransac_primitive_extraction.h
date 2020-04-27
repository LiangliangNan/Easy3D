#ifndef DIALOG_RANSAC_PRIMITIVE_EXTRACTION_H
#define DIALOG_RANSAC_PRIMITIVE_EXTRACTION_H

#include "dialog.h"

namespace Ui {
    class DialogRansacPrimitiveExtraction;
}

class DialogRansacPrimitiveExtraction : public Dialog
{
    Q_OBJECT

public:
    explicit DialogRansacPrimitiveExtraction(MainWindow *window, QDockWidget* dockWidgetCommand);
    ~DialogRansacPrimitiveExtraction();

    void setWorkOnSelectedPoints(bool b) { selected_only_ = b; }

private Q_SLOTS:
    void extract();
    void reset();

private:
    Ui::DialogRansacPrimitiveExtraction *ui;

    bool	selected_only_;

    int		default_min_support_;
    double	default_distance_threshold_;
    double	default_bitmap_resolution_;
    double	default_normal_threshold_;
    double	default_overlook_probability_;
};

#endif // DIALOG_RANSAC_PRIMITIVE_EXTRACTION_H
