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


#ifndef WIDGET_DRAWABLE_TRIANGLES_H
#define WIDGET_DRAWABLE_TRIANGLES_H

#include <QWidget>

#include "widgets/widget_drawable.h"

namespace Ui {
    class WidgetTrianglesDrawable;
}


class WidgetTrianglesDrawable : public WidgetDrawable {
Q_OBJECT

public:
    explicit WidgetTrianglesDrawable(QWidget *parent);
    ~WidgetTrianglesDrawable() override;

public slots:
    // update the panel to be consistent with the drawable's rendering parameters
    void updatePanel() override;

    void setActiveDrawable(const QString &);
    void setPhongShading(bool);
    void setColorScheme(const QString &) override;

    void setDefaultColor();
    void setBackColor();
    void setTextureFile();
    void setOpacity(int);
    void setScalarFieldStyle(int) override;
    void setVectorField(const QString &);

    void setHighlightMin(int) override;
    void setHighlightMax(int) override;

private:
    void connectAll();
    void disconnectAll();

    easy3d::Drawable *drawable() override;

    void disableUnavailableOptions() override;
    void updateVectorFieldBuffer(easy3d::Model *model, const std::string &name) override;

    // model dependent stuff
    std::vector<QString> colorSchemes(const easy3d::Model *model);
    std::vector<QString> vectorFields(const easy3d::Model *model);

private:
    Ui::WidgetTrianglesDrawable *ui_;
};

#endif // WIDGET_DRAWABLE_TRIANGLES_H
