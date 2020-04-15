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

    virtual void updatePanel() = 0;

protected:
    easy3d::Texture* createColormapTexture(int) const;

protected:
    MainWindow*     main_window_;
    PaintCanvas*    viewer_;

    // the rendering of only the selected drawable can be changed.
    // this variable keeps the history so the rendering panels are up to date when switching between models.
    std::unordered_map<easy3d::Model*, std::string> active_drawable_;

    struct ColorMap {
        ColorMap(const std::string& f, const std::string& n) : file(f), name(n) {}
        std::string file;
        std::string name;
    };
    static std::vector<ColorMap> colormap_files_;
};

#endif // WIDGET_DRAWABLE_H
