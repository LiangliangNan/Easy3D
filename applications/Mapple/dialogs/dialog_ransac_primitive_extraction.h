#ifndef DIALOG_RANSAC_PRIMITIVE_EXTRACTION_H
#define DIALOG_RANSAC_PRIMITIVE_EXTRACTION_H

#include <QDialog>

namespace Ui {
class DialogRansacPrimitiveExtraction;
}

class PaintCanvas;

class DialogRansacPrimitiveExtraction : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRansacPrimitiveExtraction(QWidget *parent = nullptr);
    ~DialogRansacPrimitiveExtraction();

    void setWorkOnSelectedPoints(bool b) { selected_only_ = b; }

private Q_SLOTS:
    void extract();
    void reset();

private:
    Ui::DialogRansacPrimitiveExtraction *ui;

    PaintCanvas*	viewer_;

    bool	selected_only_;

    int		default_min_support_;
    double	default_distance_threshold_;
    double	default_bitmap_resolution_;
    double	default_normal_threshold_;
    double	default_overlook_probability_;
};

#endif // DIALOG_RANSAC_PRIMITIVE_EXTRACTION_H
