#ifndef WIDGET_DRAWABLE_H
#define WIDGET_DRAWABLE_H

#include <unordered_map>
#include <QWidget>


namespace easy3d {
    class Model;
    class Texture;
}

class PaintCanvas;
class MainWindow;

class WidgetDrawable : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetDrawable(QWidget *parent);
    ~WidgetDrawable();

    // update the panel to be consistent with the drawable's rendering parameters
    virtual void updatePanel() = 0;

    // update the OpenGL buffers
    virtual void updateRendering() = 0;

protected:
    easy3d::Texture* colormapTexture(int) const;

protected:
    MainWindow*     main_window_;
    PaintCanvas*    viewer_;

    // the rendering of only the selected drawable can be changed.
    // this variable keeps the history so the rendering panels are up to date when switching between models.
    std::unordered_map<easy3d::Model*, std::string> active_drawable_;

    struct ColorMap {
        ColorMap(const std::string& f, const std::string& n) : file(f), name(n), texture(nullptr) {}
        std::string file;
        std::string name;
        easy3d::Texture* texture;
    };
    static std::vector<ColorMap> colormaps_;
};

#endif // WIDGET_DRAWABLE_H
