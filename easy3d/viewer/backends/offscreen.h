// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_BACKENDS_OFFSCREEN_H
#define EASY3D_VIEWER_BACKENDS_OFFSCREEN_H
#include <easy3d/viewer/offscreen.h>
namespace easy3d { namespace detail {
class OffScreenBackend {
public:
    virtual ~OffScreenBackend() = default;
    virtual Camera* camera() = 0;
    virtual unsigned validation_errors() const = 0;
    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual void resize(int,int) = 0;
    virtual void set_background_color(const vec4&) = 0;
    virtual const vec4& background_color() const = 0;
    virtual bool render(const std::string&,float,int,int,bool) const = 0;
    virtual Model* add_model(const std::string&,bool) = 0;
    virtual Model* add_model(std::shared_ptr<Model>,bool) = 0;
    virtual bool delete_model(Model*) = 0;
    virtual const std::vector<std::shared_ptr<Model>>& models() const = 0;
    virtual Model* current_model() const = 0;
    virtual Drawable* add_drawable(std::shared_ptr<Drawable>) = 0;
    virtual bool delete_drawable(Drawable*) = 0;
    virtual const std::vector<std::shared_ptr<Drawable>>& drawables() const = 0;
    virtual void clear_scene() = 0;
    virtual void fit_screen(const Model*) = 0;
};
// Host-specific work stays in the implementation; this forwards the shared API.
template<class Host> class OffScreenAdapter final : public OffScreenBackend {
    mutable Host host_;
public:
    OffScreenAdapter(int w,int h,const OffScreenOptions& options) : host_(w,h,options) {}
    Camera* camera() override {return host_.camera();}
    unsigned validation_errors() const override {return host_.validation_errors();}
    int width() const override {return host_.width();}
    int height() const override {return host_.height();}
    void resize(int w,int h) override {host_.activate();host_.resize(w,h);}
    void set_background_color(const vec4& c) override {host_.set_background_color(c);}
    const vec4& background_color() const override {return host_.background_color();}
    bool render(const std::string& f,float scale,int samples,int bg,bool expand) const override {
        host_.activate();return host_.render(f,scale,samples,bg,expand);
    }
    Model* add_model(const std::string& f,bool create) override {host_.activate();return host_.add_model(f,create);}
    Model* add_model(std::shared_ptr<Model> m,bool create) override {host_.activate();return host_.add_model(std::move(m),create);}
    bool delete_model(Model* m) override {host_.activate();return host_.delete_model(m);}
    const std::vector<std::shared_ptr<Model>>& models() const override {return host_.models();}
    Model* current_model() const override {return host_.current_model();}
    Drawable* add_drawable(std::shared_ptr<Drawable> d) override {host_.activate();return host_.add_drawable(std::move(d));}
    bool delete_drawable(Drawable* d) override {host_.activate();return host_.delete_drawable(d);}
    const std::vector<std::shared_ptr<Drawable>>& drawables() const override {return host_.drawables();}
    void clear_scene() override {host_.activate();host_.clear_scene();}
    void fit_screen(const Model* m) override {host_.activate();host_.fit_screen(m);}
};
std::unique_ptr<OffScreenBackend> create_vulkan_offscreen(int,int,const OffScreenOptions&);
std::unique_ptr<OffScreenBackend> create_opengl_offscreen(int,int,const OffScreenOptions&);
} }
#endif
