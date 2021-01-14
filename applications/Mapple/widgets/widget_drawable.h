#ifndef WIDGET_DRAWABLE_H
#define WIDGET_DRAWABLE_H

#include <unordered_map>
#include <QWidget>

#include <easy3d/renderer/drawable.h>

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

public slots:

    void setDrawableVisible(bool b);
    void setLighting(const QString &text);

    virtual void setColorScheme(const QString &text);

    void setDistinctBackColor(bool);

    void setScalarFieldStyle(int);
    void setScalarFieldDiscreteColors(bool);
    void setScalarFieldNumOfStripes(int);
    void setScalarFieldClamp(bool);
    void setScalarFieldClampLower(double);
    void setScalarFieldClampUpper(double);

    void setTextureRepeat(int);
    void setTextureFractionalRepeat(int);

    void setVectorFieldScale(double);

    void setHighlight(bool);
    virtual void setHighlightMin(int);
    virtual void setHighlightMax(int);

protected:
    easy3d::Texture* colormapTexture(int, bool, int) const;

    // the current drawable who rendering can be manipulated
    virtual easy3d::Drawable* drawable() = 0;

    virtual void disableUnavailableOptions() = 0;

    virtual void updateVectorFieldBuffer(easy3d::Model *model, const std::string &name) = 0;

    struct ColorMap {
        ColorMap(const std::string& f, const std::string& n) : file(f), name(n), texture(nullptr) {}
        std::string file;
        std::string name;
        easy3d::Texture* texture;
    };

    // the extended rendering state
    struct StateExt {
        StateExt() : scalar_style(0), discrete_color(false), num_stripes(16), vector_field("disabled"),
                     vector_field_scale(1.0), highlight_range(-1, -1) {}
        int scalar_style;
        bool discrete_color;
        int num_stripes;
        QString vector_field;
        double vector_field_scale;
        // the face range for highlighting polygon faces (not the triangle range)
        std::pair<int, int> highlight_range;
    };

protected:

    // get the property name from the color scheme name, i.e., remove scalar_prefix_ substring
    std::string color_property_name(const std::string& name, const std::string& scalar_prefix) const;

    // get the color method from the color method string
    easy3d::State::Method color_method(const std::string& name, const std::string& scalar_prefix) const ;

    // get the color location from the color scheme name
    easy3d::State::Location color_location(const std::string& name) const;

protected:
    MainWindow*     window_;
    PaintCanvas*    viewer_;
    QString         scalar_prefix_;

    // the rendering of only the selected drawable can be changed.
    // this variable keeps the history so the rendering panels are up to date when switching between models.
    std::unordered_map<easy3d::Model*, std::string> active_drawable_;

    static std::vector<ColorMap> colormaps_;

    // the state of the rendering panel
    std::unordered_map<easy3d::Drawable *, StateExt> states_;
};

#endif // WIDGET_DRAWABLE_H
