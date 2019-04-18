/********************************************************************************
** Form generated from reading UI file 'VRenderInterface.ui'
**
** Created by: Qt User Interface Compiler version 5.12.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VRENDERINTERFACE_H
#define UI_VRENDERINTERFACE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_VRenderInterface
{
public:
    QVBoxLayout *vboxLayout;
    QCheckBox *includeHidden;
    QCheckBox *cullBackFaces;
    QCheckBox *blackAndWhite;
    QCheckBox *colorBackground;
    QCheckBox *tightenBBox;
    QHBoxLayout *hboxLayout;
    QLabel *sortLabel;
    QComboBox *sortMethod;
    QSpacerItem *spacerItem;
    QHBoxLayout *hboxLayout1;
    QPushButton *SaveButton;
    QPushButton *CancelButton;

    void setupUi(QDialog *VRenderInterface)
    {
        if (VRenderInterface->objectName().isEmpty())
            VRenderInterface->setObjectName(QString::fromUtf8("VRenderInterface"));
        VRenderInterface->resize(309, 211);
        vboxLayout = new QVBoxLayout(VRenderInterface);
        vboxLayout->setSpacing(6);
        vboxLayout->setContentsMargins(11, 11, 11, 11);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        vboxLayout->setContentsMargins(5, 5, 5, 5);
        includeHidden = new QCheckBox(VRenderInterface);
        includeHidden->setObjectName(QString::fromUtf8("includeHidden"));

        vboxLayout->addWidget(includeHidden);

        cullBackFaces = new QCheckBox(VRenderInterface);
        cullBackFaces->setObjectName(QString::fromUtf8("cullBackFaces"));

        vboxLayout->addWidget(cullBackFaces);

        blackAndWhite = new QCheckBox(VRenderInterface);
        blackAndWhite->setObjectName(QString::fromUtf8("blackAndWhite"));

        vboxLayout->addWidget(blackAndWhite);

        colorBackground = new QCheckBox(VRenderInterface);
        colorBackground->setObjectName(QString::fromUtf8("colorBackground"));

        vboxLayout->addWidget(colorBackground);

        tightenBBox = new QCheckBox(VRenderInterface);
        tightenBBox->setObjectName(QString::fromUtf8("tightenBBox"));

        vboxLayout->addWidget(tightenBBox);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        hboxLayout->setContentsMargins(11, 11, 11, 11);
        sortLabel = new QLabel(VRenderInterface);
        sortLabel->setObjectName(QString::fromUtf8("sortLabel"));

        hboxLayout->addWidget(sortLabel);

        sortMethod = new QComboBox(VRenderInterface);
        sortMethod->addItem(QString());
        sortMethod->addItem(QString());
        sortMethod->addItem(QString());
        sortMethod->addItem(QString());
        sortMethod->setObjectName(QString::fromUtf8("sortMethod"));

        hboxLayout->addWidget(sortMethod);


        vboxLayout->addLayout(hboxLayout);

        spacerItem = new QSpacerItem(31, 41, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout->addItem(spacerItem);

        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setSpacing(6);
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        SaveButton = new QPushButton(VRenderInterface);
        SaveButton->setObjectName(QString::fromUtf8("SaveButton"));

        hboxLayout1->addWidget(SaveButton);

        CancelButton = new QPushButton(VRenderInterface);
        CancelButton->setObjectName(QString::fromUtf8("CancelButton"));

        hboxLayout1->addWidget(CancelButton);


        vboxLayout->addLayout(hboxLayout1);

        QWidget::setTabOrder(SaveButton, CancelButton);
        QWidget::setTabOrder(CancelButton, includeHidden);
        QWidget::setTabOrder(includeHidden, cullBackFaces);
        QWidget::setTabOrder(cullBackFaces, blackAndWhite);
        QWidget::setTabOrder(blackAndWhite, colorBackground);
        QWidget::setTabOrder(colorBackground, tightenBBox);
        QWidget::setTabOrder(tightenBBox, sortMethod);

        retranslateUi(VRenderInterface);
        QObject::connect(SaveButton, SIGNAL(released()), VRenderInterface, SLOT(accept()));
        QObject::connect(CancelButton, SIGNAL(released()), VRenderInterface, SLOT(reject()));

        sortMethod->setCurrentIndex(3);


        QMetaObject::connectSlotsByName(VRenderInterface);
    } // setupUi

    void retranslateUi(QDialog *VRenderInterface)
    {
        VRenderInterface->setWindowTitle(QApplication::translate("VRenderInterface", "Vectorial rendering options", nullptr));
#ifndef QT_NO_TOOLTIP
        includeHidden->setToolTip(QApplication::translate("VRenderInterface", "Hidden polygons are also included in the output (usually twice bigger)", nullptr));
#endif // QT_NO_TOOLTIP
        includeHidden->setText(QApplication::translate("VRenderInterface", "Include hidden parts", nullptr));
#ifndef QT_NO_TOOLTIP
        cullBackFaces->setToolTip(QApplication::translate("VRenderInterface", "Back faces (non clockwise point ordering) are removed from the output", nullptr));
#endif // QT_NO_TOOLTIP
        cullBackFaces->setText(QApplication::translate("VRenderInterface", "Cull back faces", nullptr));
#ifndef QT_NO_TOOLTIP
        blackAndWhite->setToolTip(QApplication::translate("VRenderInterface", "Black and white rendering", nullptr));
#endif // QT_NO_TOOLTIP
        blackAndWhite->setText(QApplication::translate("VRenderInterface", "Black and white", nullptr));
#ifndef QT_NO_TOOLTIP
        colorBackground->setToolTip(QApplication::translate("VRenderInterface", "Use current background color instead of white", nullptr));
#endif // QT_NO_TOOLTIP
        colorBackground->setText(QApplication::translate("VRenderInterface", "Color background", nullptr));
#ifndef QT_NO_TOOLTIP
        tightenBBox->setToolTip(QApplication::translate("VRenderInterface", "Fit output bounding box to current display", nullptr));
#endif // QT_NO_TOOLTIP
        tightenBBox->setText(QApplication::translate("VRenderInterface", "Tighten bounding box", nullptr));
#ifndef QT_NO_TOOLTIP
        sortLabel->setToolTip(QApplication::translate("VRenderInterface", "Polygon depth sorting method", nullptr));
#endif // QT_NO_TOOLTIP
        sortLabel->setText(QApplication::translate("VRenderInterface", "Sort method:", nullptr));
        sortMethod->setItemText(0, QApplication::translate("VRenderInterface", "No sorting", nullptr));
        sortMethod->setItemText(1, QApplication::translate("VRenderInterface", "BSP", nullptr));
        sortMethod->setItemText(2, QApplication::translate("VRenderInterface", "Topological", nullptr));
        sortMethod->setItemText(3, QApplication::translate("VRenderInterface", "Advanced topological", nullptr));

#ifndef QT_NO_TOOLTIP
        sortMethod->setToolTip(QApplication::translate("VRenderInterface", "Polygon depth sorting method", nullptr));
#endif // QT_NO_TOOLTIP
        SaveButton->setText(QApplication::translate("VRenderInterface", "Save", nullptr));
        CancelButton->setText(QApplication::translate("VRenderInterface", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class VRenderInterface: public Ui_VRenderInterface {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VRENDERINTERFACE_H
