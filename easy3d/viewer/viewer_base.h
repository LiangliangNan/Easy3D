// SPDX-License-Identifier: GPL-3.0-or-later
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
 ******************************************************************* *
 * \see ViewerInput, ViewerCallbacks, Viewer, MultiViewer, OffScreen
 */
#ifndef EASY3D_VIEWER_VIEWER_BASE_H
#define EASY3D_VIEWER_VIEWER_BASE_H
#include <easy3d/viewer/viewer_input.h>
#include <easy3d/viewer/viewer_callbacks.h>
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
    /**
 * \brief Common API for interactive and offscreen viewing hosts.
 *
 * \details ViewerBase defines the operations shared by Viewer, MultiViewer, and
 * OffScreen. Construct one of those concrete classes directly; the selected
 * graphics backend is fixed when Easy3D is built. Each host manages its own
 * scene. Create, use, and destroy hosts on the UI/main thread.
 *
 * \section viewer_coordinates Coordinate systems
 * Window dimensions and input positions use logical window coordinates: x
 * increases rightward, y increases downward, and the origin is at the top-left.
 * These coordinates are DPI-independent and are the units for width(),
 * height(), mouse events, world_to_window(), and window_to_world().
 *
 * Framebuffer dimensions are physical pixels and are intended for GPU work,
 * render targets, and pixel readback. On a high-DPI window they are related to
 * logical dimensions by the DPI scale, subject to integer rounding. OffScreen
 * uses a fixed-size pixel canvas, so its DPI scale is 1. The *_pixel conversions
 * (world_to_pixel(), pixel_to_world()) use these framebuffer pixels.
 *
 * world_to_window() and window_to_world() use continuous logical coordinates;
 * their depth component is normalized to [0, 1], with 0 at the near plane and
 * 1 at the far plane. point_under_pixel() instead accepts integer logical
 * coordinates and queries the visible scene at the corresponding framebuffer
 * pixel. It excludes HUD and overlay content.
 *
 * \par Frame lifecycle
 * draw_frame() renders at most one frame; it does not itself run an interactive
 * event loop. run() and run_frames() provide the concrete host's loop. The
 * unified ViewerCallbacks are distinct from Viewer subclasses' legacy virtual
 * event hooks.
 */
    class ViewerBase : public ViewerInput {
    public:
        /// Destroys the host and releases its backend resources.
        virtual ~ViewerBase();

        /**
     * \brief Dispatch pending window-system events for all Easy3D viewers.
     * \details This is a static operation and does not render a frame. Normal
     * run() and run_frames() loops poll events themselves.
     */
        static void poll_events();

        /**
     * \brief Replace the set of unified event and frame callbacks.
     * \param callbacks New callbacks; empty members disable the corresponding hook.
     * \details Call on the UI thread. It is safe to replace callbacks from a
     * callback. A callback already in progress remains alive until it returns,
     * and state held by a callable is retained while that callable is installed.
     * Input callbacks returning true consume the event before legacy viewer
     * handlers and built-in controls are invoked.
     */
        void set_callbacks(ViewerCallbacks callbacks);

        /// Returns the number of graphics validation errors observed by this host.
        /// \return The backend's validation-error count.
        virtual unsigned validation_errors() const = 0;

        /**
     * \brief Run the interactive viewer until it is closed.
     * \param see_all If true, fit the scene before entering the loop.
     * \return A status code; Viewer returns EXIT_SUCCESS on success and
     *         EXIT_FAILURE when a standard exception is reported.
     */
        virtual int run(bool see_all = true) = 0;

        /**
     * \brief Run the viewer loop for a bounded number of rendered frames.
     * \param max_frames Maximum number of successfully rendered frames; zero
     *        means continue until the viewer is closed.
     * \return A backend-defined status code; see the concrete implementation.
     * \details A frame that cannot be rendered, for example while minimized,
     * does not count toward a positive frame limit. Unlike run(), this method
     * is intended for bounded processing and does not promise to show the
     * window or fit the scene first.
     */
        virtual int run_frames(unsigned max_frames = 0) = 0;

        /**
     * \brief Render one frame if the host is ready to render.
     * \return True if a frame was rendered and presented; false if no frame
     *         could be rendered, for example because the host is closed or
     *         minimized.
     * \details This does not poll events, show the host, or refit the camera.
     * Concrete implementations may invoke animation and frame callbacks. A
     * rendering or callback failure may be reported by the concrete backend.
     */
        virtual bool draw_frame() = 0;

        /// Request termination of the concrete host's run loop.
        virtual void exit() = 0;

        /// Request that the window close; this does not synchronously destroy it.
        virtual void request_close() = 0;

        /// Returns whether the concrete host has received a close request.
        /// \return True when close has been requested.
        virtual bool should_close() const = 0;

        /// Show the host window when the concrete backend has a window.
        virtual void show() = 0;

        /// Hide the host window when the concrete backend has a window.
        virtual void hide() = 0;

        /**
     * \brief Set the host's logical window or canvas size.
     * \param width Requested width in logical units.
     * \param height Requested height in logical units.
     */
        virtual void resize(int width, int height) = 0;

        /**
     * \brief Set the host window's screen position.
     * \param x Horizontal screen coordinate of the window's upper-left corner.
     * \param y Vertical screen coordinate of the window's upper-left corner.
     * \details Hosts without a visible window may not support positioning.
     */
        virtual void set_position(int x, int y) = 0;

        /// Set the host window title; hosts without a window may not display it.
        /// \param title New title text.
        virtual void set_title(const std::string &title) = 0;

        /// Returns the current host window title.
        /// \return The title string owned by the host.
        virtual const std::string &title() const = 0;

        /**
     * \brief Query the logical window or canvas size.
     * \param width Receives the width in logical units.
     * \param height Receives the height in logical units.
     */
        virtual void viewer_size(int &width, int &height) const = 0;

        /// Returns the logical width; equivalent to the width output of viewer_size().
        /// \return Width in logical units.
        int width() const {
            int w, h;
            viewer_size(w, h);
            return w;
        }

        /// Returns the logical height; equivalent to the height output of viewer_size().
        /// \return Height in logical units.
        int height() const {
            int w, h;
            viewer_size(w, h);
            return h;
        }

        /**
     * \brief Query the physical framebuffer size in pixels.
     * \param width Receives the framebuffer width.
     * \param height Receives the framebuffer height.
     */
        virtual void framebuffer_size(int &width, int &height) const = 0;

        /// Returns the framebuffer width in physical pixels.
        /// \return Width in pixels.
        int framebuffer_width() const {
            int w, h;
            framebuffer_size(w, h);
            return w;
        }

        /// Returns the framebuffer height in physical pixels.
        /// \return Height in pixels.
        int framebuffer_height() const {
            int w, h;
            framebuffer_size(w, h);
            return h;
        }

        /**
     * \brief Return the ratio between framebuffer pixels and logical units.
     * \return The framebuffer-to-window scale.
     * \details Window systems may round framebuffer dimensions to integer
     * pixels, so multiplying a logical dimension by this value need not exactly
     * reproduce the framebuffer dimension.
     */
        virtual float dpi_scaling() const = 0;

        /// Returns the effective window multisample count; 1 means multisampling is disabled.
        /// \return Effective sample count.
        unsigned samples() const {
            return window_samples();
        }

        /**
     * \brief Return the backend's native window handle, if it has one.
     * \return A borrowed GLFW window handle, or nullptr for a host without a
     *         native window/context.
     * \details The host retains ownership. Do not destroy the returned handle.
     */
        virtual GLFWwindow *window() const = 0; // borrowed; may have no OpenGL context

        /// Request a repaint; this does not guarantee that a frame is drawn immediately.
        virtual void update() const = 0;

        /// Returns the mutable camera owned by this host.
        /// \return Pointer to the host-owned camera.
        virtual Camera *camera() = 0;

        /// Returns the camera owned by this host for read-only access.
        /// \return Pointer to the host-owned camera.
        virtual const Camera *camera() const = 0;

        /// Set the scene background color.
        /// \param color New background color.
        virtual void set_background_color(const vec4 &color) = 0;

        /// Returns the scene background color.
        /// \return Reference to the host's background color.
        virtual const vec4 &background_color() const = 0;

        /**
     * \brief Load a model from a file and add it to the host.
     * \param file_name Model file to load.
     * \param create_drawables Whether to create the model's standard
     *        rendering drawables.
     * \param fit_scene If true, fit the camera to the scene after addition; false
     *        keeps the current view.
     * \return The added model, or nullptr if loading or addition fails. The returned
     *         pointer is a non-owning observer; do not delete it.
     * \details On success, the host retains ownership of the model (held internally by
     *      a shared_ptr). If a model with the same file name is already present, that
     *      model is returned unchanged.
     */
        virtual Model *add_model(const std::string &file_name, bool create_drawables = true,
                                 bool fit_scene = true) = 0;

        /**
     * \brief Add a shared model to the host.
     * \param model Shared ownership handle for the model.
     * \param create_drawables Whether to create standard drawables.
     * \param fit_scene If true, fit the camera to the scene after addition; false
     *        keeps the current view.
     * \return The added model, or nullptr if the model is null or empty, or if
     *         addition fails. The returned pointer is a non-owning observer; do not
     *         delete it.
     * \details Ownership is shared: the host stores a copy of the shared_ptr, so the
     *      model stays alive while either the caller or the host still references it.
     *      Do not delete it manually. Prefer this form when your code keeps using the
     *      model after adding it.
     *
     *      The standard viewer implementation installs a *new* Renderer on the model,
     *      replacing any renderer and drawables it already had (add custom drawables
     *      afterwards), and returns the existing entry unchanged if the same model is
     *      added again.
     * \sa add_model(Model*, bool, bool) for the legacy ownership-transferring raw-pointer form.
     */
        virtual Model *add_model(std::shared_ptr<Model> model, bool create_drawables = true,
                                 bool fit_scene = true) = 0;

        /**
     * \brief Add a model supplied as a raw pointer, transferring ownership to the host
     *        (legacy).
     * \param model Pointer to a heap-allocated model; ownership is transferred to the host.
     * \param create_drawables Whether to create standard drawables.
     * \param fit_scene If true, fit the camera to the scene after addition; false
     *        keeps the current view.
     * \return The added model, or nullptr if the model is null or empty, or if
     *         addition fails. The returned pointer is a non-owning observer; do not
     *         delete it.
     * \details Legacy convenience equivalent to
     *      add_model(std::shared_ptr<Model>(model), create_drawables, fit_scene).
     *      The host takes sole ownership and deletes the model later. After the call the
     *      caller must not delete the model or create another shared_ptr from it, or it
     *      would be freed twice. Ownership, renderer replacement, camera fitting and
     *      de-duplication behave exactly as in the shared_ptr overload. Prefer the
     *      shared_ptr overload for new code.
     * \sa add_model(std::shared_ptr<Model>, bool, bool) for the shared-ownership form.
     */
        virtual Model *add_model(Model *model, bool create_drawables = true, bool fit_scene = true) = 0;

        /**
     * \brief Remove a model and its associated drawables.
     * \param model Model to remove.
     * \return True if the model was managed by this host and was removed.
     * \details Removal releases the host's ownership; other shared owners may
     * keep the model alive.
     */
        virtual bool delete_model(Model *model) = 0;

        /// Returns the models managed by the host; the returned vector is host-owned.
        /// \return Reference to the host-owned model collection.
        virtual const std::vector<std::shared_ptr<Model>> &models() const = 0;

        /// Returns the active model used by the viewer's model-oriented commands, or nullptr.
        /// \return Active model, or nullptr when none is active.
        virtual Model *current_model() const = 0;

        /**
     * \brief Add a shared drawable to the host.
     * \param drawable Drawable to add.
     * \return The added drawable, or nullptr if the pointer is null. The returned
     *         pointer is a non-owning observer; do not delete it.
     * \details Ownership is shared: the host stores a copy of the shared_ptr and the
     *      drawable stays alive while either side still references it. Do not delete
     *      it manually. If the same drawable is already managed by the host, the
     *      existing entry is returned unchanged.
     * \sa add_drawable(Drawable*) for the legacy ownership-transferring raw-pointer form.
     */
        virtual Drawable *add_drawable(std::shared_ptr<Drawable> drawable) = 0;

        /**
     * \brief Add a drawable supplied as a raw pointer, transferring ownership to the host
     *        (legacy).
     * \param drawable Pointer to the drawable; ownership is transferred to the host.
     * \return The added drawable, or nullptr if the pointer is null. The returned
     *         pointer is a non-owning observer; do not delete it.
     * \details Legacy convenience equivalent to
     *      add_drawable(std::shared_ptr<Drawable>(drawable)). The host takes sole
     *      ownership. After the call the caller must not delete the drawable or create
     *      another shared_ptr from it. Ownership and de-duplication behave exactly as
     *      in add_drawable(std::shared_ptr<Drawable>). Prefer the shared_ptr overload
     *      for new code.
     * \sa add_drawable(std::shared_ptr<Drawable>) for the shared-ownership form.
     */
        virtual Drawable *add_drawable(Drawable *drawable) = 0;

        /**
     * \brief Remove a drawable managed directly by the host.
     * \param drawable Drawable to remove.
     * \return True if the drawable was removed.
     */
        virtual bool delete_drawable(Drawable *drawable) = 0;

        /// Returns the standalone drawables managed by the host.
        /// \return Reference to the host-owned drawable collection.
        virtual const std::vector<std::shared_ptr<Drawable>> &drawables() const = 0;

        /// Remove all models and standalone drawables from the host's scene.
        virtual void clear_scene() = 0;

        /**
     * \brief Adjust the camera to frame a model or the complete scene.
     * \param model Model to frame; nullptr means frame the host's scene.
     */
        virtual void fit_screen(const Model *model = nullptr) = 0;

        /**
     * \brief Convert a world-space point to logical window coordinates.
     * \param world Point in world coordinates.
     * \return Continuous logical window position (top-left origin) and
     *         normalized depth in [0, 1].
     * \throws std::invalid_argument for non-finite input, invalid window size,
     *         or invalid camera matrices.
     * \details The result is not clipped to the view frustum; finite points
     * outside the visible region can produce coordinates outside the window.
     */
        virtual vec3 world_to_window(const vec3 &world) const;

        /**
     * \brief Convert logical window coordinates to world coordinates.
     * \param window Continuous logical x/y coordinates (top-left origin) and
     *        normalized depth in [0, 1].
     * \return The corresponding world-space point.
     * \throws std::invalid_argument for non-finite input, invalid window size,
     *         or a singular/invalid camera matrix.
     */
        virtual vec3 window_to_world(const vec3 &window) const;

        /**
     * \brief Convert a world-space point to framebuffer pixel coordinates.
     * \param world Point in world coordinates.
     * \return Continuous framebuffer x/y (top-left origin, physical pixels) and
     *         normalized depth in [0, 1].
     * \details Equivalent to world_to_window() scaled by the framebuffer-to-window
     * ratio. Floor the x/y components to obtain a pixel index.
     */
        virtual vec3 world_to_pixel(const vec3 &world) const;

        /**
     * \brief Convert a framebuffer pixel and normalized depth to world space.
     * \param x Framebuffer x coordinate, with the origin at top-left.
     * \param y Framebuffer y coordinate, with the origin at top-left.
     * \param depth Normalized depth in [0, 1].
     * \return The world-space position at the center of the given pixel.
     * \throws std::invalid_argument for an out-of-range pixel, invalid depth, or a
     *         singular/invalid camera matrix.
     */
        virtual vec3 pixel_to_world(int x, int y, float depth) const;

        /**
     * \brief Query the visible scene point beneath a logical window pixel.
     * \param x Integer logical x coordinate from the top-left.
     * \param y Integer logical y coordinate from the top-left.
     * \param found Set to true when a visible scene surface was hit.
     * \return The world-space position at the center of the mapped framebuffer
     *         pixel, or vec3(0) if no scene surface was found.
     * \details This is a fresh synchronous query; it is not affected by window
     * multisampling. HUD, overlays, clipping-plane guides, and callback drawing
     * are excluded. Coordinates outside the logical window set found to false.
     */
        virtual vec3 point_under_pixel(int x, int y, bool &found) const;

        /// Open a file-selection dialog and load a model.
        /// \return True on success, false on cancellation or failure.
        virtual bool open() = 0;

        /// Save the current model through a file-selection dialog.
        /// \return True on success, false on cancellation or failure.
        virtual bool save() const = 0;

        /// Save the viewer image through a file-selection dialog.
        /// \return True on success, false on cancellation or failure.
        virtual bool snapshot() const = 0;

        /**
     * \brief Render the scene to an image file.
     * \param file_name Destination image path.
     * \param scaling Output-size multiplier relative to the logical canvas size.
     * \param samples Requested antialiasing sample count.
     * \param background Background mode: 0 uses the host background, 1 uses
     *        white, and 2 uses transparency.
     * \param expand If true, expand the frustum as needed to preserve the
     *        original view while matching the output aspect ratio.
     * \return True on success, false if the image could not be written.
     * \details The concrete implementation may reject unsupported parameters.
     * File snapshots do not change the live camera, visibility, or animation
     * state.
     */
        virtual bool snapshot(const std::string &file_name,
                              float scaling = 1.0f,
                              int samples = 4,
                              int background = 0,
                              bool expand = true) const = 0;

        /**
     * \brief Set the viewer's usage text and on-screen hint.
     * \param manual Manual text; concrete Viewer treats "default" specially to
     *        display its default shortcut list.
     * \param hint Short hint shown by concrete Viewer when supported.
     */
        virtual void set_usage(const std::string &manual, const std::string &hint) = 0;

        /// Callback invoked for each rendered frame when animation is enabled.
        using AnimationFunction = std::function<bool(ViewerBase *)>;

        /// Install or clear the animation callback.
        /// \param callback Callback to install, or an empty function to clear it.
        virtual void set_animation_callback(const AnimationFunction &callback) = 0;

        /// Enable or disable animation updates.
        virtual void set_animation(bool) = 0;

        /// Returns true when animation is enabled and an animation callback is installed.
        /// \return True if animation is currently enabled and has a callback.
        virtual bool is_animating() const = 0;

        /// Function type used by bind(); return true when the operation succeeds.
        using Function = std::function<bool(ViewerBase *, Model *)>;

        /**
     * \brief Bind a function to a key and optional modifier.
     * \param func Function invoked when the shortcut is activated.
     * \param model Model passed to the function; may be nullptr.
     * \param key Key identifier from ViewerInput.
     * \param modifier Modifier identifier from ViewerInput.
     * \details A concrete viewer may replace an existing binding for the same
     * key and modifier.
     */
        void bind(const Function &func, Model *model, Key key, Modifier modifier = MODIF_NONE) {
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
        bool dispatch_drop(const std::vector<std::string> &);
        bool dispatch_focus(bool);
        virtual void bind_command(const Function &, Model *, Key, Modifier) = 0;
        virtual unsigned window_samples() const = 0;
        virtual bool read_scene_depth(int x, int y, const Camera &snapshot, float &depth) const = 0;
        bool window_pixel(int x, int y, int &px, int &py) const;

    private:
        std::shared_ptr<ViewerCallbacks> callbacks_;
        int callback_button_ = -1;
        bool callback_capture_ = false;
        int resize_width_ = 0, resize_height_ = 0, resize_fb_width_ = 0, resize_fb_height_ = 0;
    };
} // namespace easy3d
#endif
