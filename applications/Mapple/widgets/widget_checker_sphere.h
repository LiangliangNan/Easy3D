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


#ifndef WIDGET_CHECKER_SPHERE_H
#define WIDGET_CHECKER_SPHERE_H

#include <QObject>
#include <QOpenGLWidget>

#include <easy3d/core/types.h>


namespace easy3d {
    class Camera;
    class TrianglesDrawable;
}

class WidgetCheckerSphere : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit WidgetCheckerSphere(QWidget *parent = nullptr);
    ~WidgetCheckerSphere() override;

    void setBackgroundColor(const QColor& c) { backgroundColor_ = c; }
    const QColor& backgroundColor() const { return backgroundColor_; }

signals:
    void lightPositionChanged();

public Q_SLOTS:
    void resetLightPosition();

private slots:
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

private:
    void createSpheres();
    void updateLighting();
    easy3d::vec3 projectToSphere(const QPoint& p);

private:
    easy3d::Camera*         camera_;
    easy3d::TrianglesDrawable*  checkerSphere_;
    easy3d::TrianglesDrawable*  lightSphere_;

    bool	 mouseDown_;
    float    scene_radius_;
    QColor   backgroundColor_;

    easy3d::vec4	lightPos_;
    easy3d::vec4	defaultLightPos_;
  };


#endif // WIDGET_CHECKER_SPHERE_H
