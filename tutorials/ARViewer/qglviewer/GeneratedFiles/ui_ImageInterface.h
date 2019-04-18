/********************************************************************************
** Form generated from reading UI file 'ImageInterface.ui'
**
** Created by: Qt User Interface Compiler version 5.12.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IMAGEINTERFACE_H
#define UI_IMAGEINTERFACE_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ImageInterface
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *label_2;
    QSpinBox *imgWidth;
    QLabel *label_3;
    QSpinBox *imgHeight;
    QFrame *line_4;
    QLabel *label_5;
    QSpinBox *imgScale;
    QSpacerItem *spacer;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_2;
    QLabel *label_4;
    QSpinBox *imgQuality;
    QLabel *label;
    QDoubleSpinBox *oversampling;
    QSpacerItem *spacer_2;
    QGroupBox *groupBox_3;
    QGridLayout *gridLayout_3;
    QCheckBox *whiteBackground;
    QCheckBox *expandFrustum;
    QSpacerItem *spacerItem;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer;
    QPushButton *okButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *ImageInterface)
    {
        if (ImageInterface->objectName().isEmpty())
            ImageInterface->setObjectName(QString::fromUtf8("ImageInterface"));
        ImageInterface->resize(266, 391);
        QIcon icon;
        icon.addFile(QString::fromUtf8("../../../../../../Documents and Settings/debunne/Bureau/libQGLViewer-2.2.0-1/doc/images/qglviewer.icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        ImageInterface->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(ImageInterface);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox = new QGroupBox(ImageInterface);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 0, 0, 1, 1);

        imgWidth = new QSpinBox(groupBox);
        imgWidth->setObjectName(QString::fromUtf8("imgWidth"));
        imgWidth->setMinimumSize(QSize(60, 0));
        imgWidth->setMaximumSize(QSize(200, 16777215));
        imgWidth->setAlignment(Qt::AlignCenter);
        imgWidth->setMinimum(1);
        imgWidth->setMaximum(32000);

        gridLayout->addWidget(imgWidth, 0, 1, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 1, 0, 1, 1);

        imgHeight = new QSpinBox(groupBox);
        imgHeight->setObjectName(QString::fromUtf8("imgHeight"));
        imgHeight->setMinimumSize(QSize(60, 0));
        imgHeight->setMaximumSize(QSize(200, 16777215));
        imgHeight->setAlignment(Qt::AlignCenter);
        imgHeight->setMinimum(1);
        imgHeight->setMaximum(32000);

        gridLayout->addWidget(imgHeight, 1, 1, 1, 1);

        line_4 = new QFrame(groupBox);
        line_4->setObjectName(QString::fromUtf8("line_4"));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_4, 2, 0, 1, 2);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 3, 0, 1, 1);

        imgScale = new QSpinBox(groupBox);
        imgScale->setObjectName(QString::fromUtf8("imgScale"));
        imgScale->setAlignment(Qt::AlignCenter);
        imgScale->setMinimum(1);
        imgScale->setMaximum(10);
        imgScale->setValue(1);

        gridLayout->addWidget(imgScale, 3, 1, 1, 1);


        verticalLayout->addWidget(groupBox);

        spacer = new QSpacerItem(17, 13, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(spacer);

        groupBox_2 = new QGroupBox(ImageInterface);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        gridLayout_2 = new QGridLayout(groupBox_2);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_4 = new QLabel(groupBox_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout_2->addWidget(label_4, 0, 0, 1, 1);

        imgQuality = new QSpinBox(groupBox_2);
        imgQuality->setObjectName(QString::fromUtf8("imgQuality"));
        imgQuality->setMinimumSize(QSize(60, 0));
        imgQuality->setMaximumSize(QSize(200, 16777215));
        imgQuality->setAlignment(Qt::AlignCenter);
        imgQuality->setMinimum(0);
        imgQuality->setMaximum(100);
        imgQuality->setValue(100);

        gridLayout_2->addWidget(imgQuality, 0, 1, 1, 1);

        label = new QLabel(groupBox_2);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout_2->addWidget(label, 1, 0, 1, 1);

        oversampling = new QDoubleSpinBox(groupBox_2);
        oversampling->setObjectName(QString::fromUtf8("oversampling"));
        oversampling->setMinimumSize(QSize(60, 0));
        oversampling->setMaximumSize(QSize(200, 16777215));
        oversampling->setAlignment(Qt::AlignCenter);
        oversampling->setDecimals(0);
        oversampling->setMinimum(0.000000000000000);
        oversampling->setMaximum(64.000000000000000);
        oversampling->setSingleStep(1.000000000000000);
        oversampling->setValue(1.000000000000000);

        gridLayout_2->addWidget(oversampling, 1, 1, 1, 1);


        verticalLayout->addWidget(groupBox_2);

        spacer_2 = new QSpacerItem(17, 13, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(spacer_2);

        groupBox_3 = new QGroupBox(ImageInterface);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        gridLayout_3 = new QGridLayout(groupBox_3);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        whiteBackground = new QCheckBox(groupBox_3);
        whiteBackground->setObjectName(QString::fromUtf8("whiteBackground"));
        whiteBackground->setChecked(true);

        gridLayout_3->addWidget(whiteBackground, 0, 0, 1, 1);

        expandFrustum = new QCheckBox(groupBox_3);
        expandFrustum->setObjectName(QString::fromUtf8("expandFrustum"));

        gridLayout_3->addWidget(expandFrustum, 1, 0, 1, 1);


        verticalLayout->addWidget(groupBox_3);

        spacerItem = new QSpacerItem(17, 13, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(spacerItem);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalSpacer = new QSpacerItem(80, 22, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);

        okButton = new QPushButton(ImageInterface);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        horizontalLayout_3->addWidget(okButton);

        cancelButton = new QPushButton(ImageInterface);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        horizontalLayout_3->addWidget(cancelButton);


        verticalLayout->addLayout(horizontalLayout_3);

        QWidget::setTabOrder(imgWidth, imgHeight);
        QWidget::setTabOrder(imgHeight, imgScale);
        QWidget::setTabOrder(imgScale, imgQuality);
        QWidget::setTabOrder(imgQuality, oversampling);
        QWidget::setTabOrder(oversampling, whiteBackground);
        QWidget::setTabOrder(whiteBackground, expandFrustum);
        QWidget::setTabOrder(expandFrustum, okButton);
        QWidget::setTabOrder(okButton, cancelButton);

        retranslateUi(ImageInterface);
        QObject::connect(okButton, SIGNAL(clicked()), ImageInterface, SLOT(accept()));
        QObject::connect(cancelButton, SIGNAL(clicked()), ImageInterface, SLOT(reject()));

        QMetaObject::connectSlotsByName(ImageInterface);
    } // setupUi

    void retranslateUi(QDialog *ImageInterface)
    {
        ImageInterface->setWindowTitle(QApplication::translate("ImageInterface", "Image settings", nullptr));
        groupBox->setTitle(QApplication::translate("ImageInterface", "Size", nullptr));
        label_2->setText(QApplication::translate("ImageInterface", "Width", nullptr));
#ifndef QT_NO_TOOLTIP
        imgWidth->setToolTip(QApplication::translate("ImageInterface", "Width of the image (in pixels)", nullptr));
#endif // QT_NO_TOOLTIP
        imgWidth->setSuffix(QApplication::translate("ImageInterface", " px", nullptr));
        label_3->setText(QApplication::translate("ImageInterface", "Height", nullptr));
#ifndef QT_NO_TOOLTIP
        imgHeight->setToolTip(QApplication::translate("ImageInterface", "Height of the image (in pixels)", nullptr));
#endif // QT_NO_TOOLTIP
        imgHeight->setSuffix(QApplication::translate("ImageInterface", " px", nullptr));
        label_5->setText(QApplication::translate("ImageInterface", "Scale", nullptr));
#ifndef QT_NO_TOOLTIP
        imgScale->setToolTip(QApplication::translate("ImageInterface", "Scale image", nullptr));
#endif // QT_NO_TOOLTIP
        imgScale->setPrefix(QApplication::translate("ImageInterface", "x ", nullptr));
        groupBox_2->setTitle(QApplication::translate("ImageInterface", "Quality", nullptr));
        label_4->setText(QApplication::translate("ImageInterface", "Image quality", nullptr));
#ifndef QT_NO_TOOLTIP
        imgQuality->setToolTip(QApplication::translate("ImageInterface", "Between 0 (smallest files) and 100 (highest quality)", nullptr));
#endif // QT_NO_TOOLTIP
        label->setText(QApplication::translate("ImageInterface", "Oversampling", nullptr));
#ifndef QT_NO_TOOLTIP
        oversampling->setToolTip(QApplication::translate("ImageInterface", "Antialiases image (when larger then 1.0)", nullptr));
#endif // QT_NO_TOOLTIP
        oversampling->setPrefix(QApplication::translate("ImageInterface", "x ", nullptr));
        groupBox_3->setTitle(QApplication::translate("ImageInterface", "Options", nullptr));
#ifndef QT_NO_TOOLTIP
        whiteBackground->setToolTip(QApplication::translate("ImageInterface", "Use white as background color", nullptr));
#endif // QT_NO_TOOLTIP
        whiteBackground->setText(QApplication::translate("ImageInterface", "Use white background", nullptr));
#ifndef QT_NO_TOOLTIP
        expandFrustum->setToolTip(QApplication::translate("ImageInterface", "When image aspect ratio differs from viewer's one, expand frustum as needed. Fits inside current frustum otherwise.", nullptr));
#endif // QT_NO_TOOLTIP
        expandFrustum->setText(QApplication::translate("ImageInterface", "Expand frustum if needed", nullptr));
        okButton->setText(QApplication::translate("ImageInterface", "OK", nullptr));
        cancelButton->setText(QApplication::translate("ImageInterface", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ImageInterface: public Ui_ImageInterface {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IMAGEINTERFACE_H
