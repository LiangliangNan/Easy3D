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
    virtual ~WidgetCheckerSphere() override;

    void setBackgroundColor(const QColor& c) { backgroundColor_ = c; }
    const QColor& backgroundColor() const { return backgroundColor_; }

signals:
    void lightPositionChanged();

public Q_SLOTS:
    void resetLightPosition();

private slots:
    virtual void mousePressEvent(QMouseEvent *) override;
    virtual void mouseMoveEvent(QMouseEvent *) override;
    virtual void mouseReleaseEvent(QMouseEvent *) override;

protected:
    virtual void initializeGL() override;
    virtual void resizeGL(int width, int height) override;
    virtual void paintGL() override;

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
