// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_VIEWER_BASE_H
#define EASY3D_VIEWER_VIEWER_BASE_H
#include <easy3d/viewer/viewer_input.h>
#include <easy3d/viewer/viewer_callbacks.h>
#include <easy3d/renderer/backend.h>
#include <easy3d/core/types.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>
struct GLFWwindow;
namespace easy3d {
class Camera;
class Model;
class Drawable;
struct ViewerOptions {
    Backend backend = Backend::Automatic;
    int width = 800, height = 600;
    bool visible = true, resizable = true;
    unsigned samples = 4;
    bool validation = false, synchronization_validation = false;
};
// Portable interactive contract. Viewer::create() is inherited by both existing
// hosts; old constructors/subclass drawing hooks remain backend-specific.
// Create, use and destroy on the UI/main thread, with a separate scene per host.
class ViewerBase : public ViewerInput {
public:
    virtual ~ViewerBase();
    static void poll_events();
    // Replace/clear from the UI thread, including from a callback. The active
    // callable stays alive until it returns; mutable callable state is retained.
    void set_callbacks(ViewerCallbacks callbacks);
    virtual Backend backend() const = 0;
    const std::string& fallback_reason() const { return fallback_reason_; }
    virtual unsigned validation_errors() const = 0;
    virtual int run(bool see_all = true) = 0;
    virtual int run_frames(unsigned max_frames = 0) = 0;
    virtual bool draw_frame() = 0;
    virtual void exit() = 0;
    virtual void request_close() = 0;
    virtual bool should_close() const = 0;
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual void resize(int width, int height) = 0;
    virtual void set_position(int x, int y) = 0;
    virtual void set_title(const std::string&) = 0;
    virtual const std::string& title() const = 0;
    virtual void viewer_size(int& width, int& height) const = 0;
    // Logical window units. Explicit native hosts retain their historical width/height APIs.
    int width() const { int w, h; viewer_size(w, h); return w; }
    int height() const { int w, h; viewer_size(w, h); return h; }
    virtual void framebuffer_size(int& width, int& height) const = 0;
    virtual int framebuffer_width() const = 0;
    virtual int framebuffer_height() const = 0;
    virtual float dpi_scaling() const = 0;
    unsigned samples() const { return window_samples(); } // 1 means no MSAA
    virtual GLFWwindow* window() const = 0; // borrowed; may have no OpenGL context
    virtual void update() const = 0;
    virtual Camera* camera() = 0;
    virtual const Camera* camera() const = 0;
    virtual void set_background_color(const vec4&) = 0;
    virtual const vec4& background_color() const = 0;
    virtual Model* add_model(const std::string&, bool create_default_drawables = true) = 0;
    virtual Model* add_model(std::shared_ptr<Model>, bool create_default_drawables = true) = 0;
    virtual Model* add_model(Model*, bool create_default_drawables = true) = 0;
    virtual bool delete_model(Model*) = 0;
    virtual const std::vector<std::shared_ptr<Model>>& models() const = 0;
    virtual Model* current_model() const = 0;
    virtual Drawable* add_drawable(std::shared_ptr<Drawable>) = 0;
    virtual Drawable* add_drawable(Drawable*) = 0;
    virtual bool delete_drawable(Drawable*) = 0;
    virtual const std::vector<std::shared_ptr<Drawable>>& drawables() const = 0;
    virtual void clear_scene() = 0;
    virtual void fit_screen(const Model* model = nullptr) = 0;
    // Continuous logical window coordinates, top-left origin; depth uses [0,1].
    // Conversion does not clip to the frustum; finite outside coordinates work.
    vec3 project_point(const vec3& world) const;
    vec3 unproject_point(const vec3& window) const;
    // Synchronous visible-surface query, independent of window MSAA. Integer
    // logical coordinates map to their containing framebuffer pixel; returns
    // that pixel center's world position. Background/outside => found=false.
    // Scene only: HUD, clipping-plane guides and frame callbacks are excluded.
    vec3 point_under_pixel(int x, int y, bool& found) const;
    virtual bool open() = 0;
    virtual bool save() const = 0;
    virtual bool snapshot() const = 0;
    // Live camera/visibility and animation remain unchanged by file snapshots.
    bool snapshot(const std::string& filename, float scaling = 1.0f, int samples = 4,
                  int background = 1, bool expand = true) const;
    virtual void set_usage(const std::string& manual = "default", const std::string& hint = "") = 0;
    using AnimationFunction = std::function<bool(ViewerBase*)>;
    virtual void set_animation_callback(const AnimationFunction&) = 0;
    virtual void set_animation(bool) = 0;
    virtual bool is_animating() const = 0;
    using Function = std::function<bool(ViewerBase*, Model*)>;
    void bind(const Function& func, Model* model, Key key, Modifier modifier = MODIF_NONE) {
        bind_command(func, model, key, modifier);
    }
protected:
    void notify_pre_draw();
    void notify_post_draw();
    void notify_resize();
    bool dispatch_mouse_press(int, int, int, int);
    bool dispatch_mouse_release(int, int, int, int);
    bool dispatch_mouse_drag(int, int, int, int, int, int);
    bool dispatch_mouse_move(int, int, int, int, int);
    bool dispatch_mouse_scroll(int, int, int, int);
    bool dispatch_key_press(int, int);
    bool dispatch_key_release(int, int);
    bool dispatch_character(unsigned int);
    bool dispatch_drop(const std::vector<std::string>&);
    bool dispatch_focus(bool);
    virtual void bind_command(const Function&, Model*, Key, Modifier) = 0;
    virtual unsigned window_samples() const = 0;
    virtual bool read_scene_depth(int x, int y, const Camera& snapshot, float& depth) const = 0;
    virtual bool snapshot_file(const std::string&, float, int, int, bool) const = 0;
private:
    std::string fallback_reason_;
    std::shared_ptr<ViewerCallbacks> callbacks_;
    int callback_button_ = -1;
    bool callback_capture_ = false;
    int resize_width_ = 0, resize_height_ = 0, resize_fb_width_ = 0, resize_fb_height_ = 0;
};
}
#endif
