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

#ifndef EASY3D_VIEWER_VIEWER_H
#define EASY3D_VIEWER_VIEWER_H

#include <easy3d/viewer/viewer_base.h>
#include <string>
#include <vector>
#include <memory>
#include <exception>
#include <map>
#include <chrono>
#include <easy3d/renderer/render_device.h>
#ifdef EASY3D_HAS_VULKAN
#include <easy3d/renderer/camera.h>
#endif
struct GLFWwindow;
namespace easy3d {
namespace detail { class GlfwRuntime; }
namespace opengl { class OverlayRenderer; class PointSelector; }
namespace graphics { class TextLayoutEngine; }
namespace vulkan { class KeyFrameInterpolatorRenderer; }
class Camera;
class Model;
class Drawable;
class LinesDrawable2D;
class TrianglesDrawable;
class TextRenderer;
class KeyFrameInterpolator;
#ifdef EASY3D_HAS_VULKAN
/**
 * \brief Options for creating a Vulkan-backed Viewer.
 * \details Vulkan is selected by the Easy3D build configuration; these
 * settings do not switch an existing viewer between graphics backends.
 */
struct VulkanOptions {
    /// Request Vulkan validation layers; construction fails if they are unavailable.
    bool validation = false;
    /// Request synchronization validation; this also enables validation layers.
    bool sync_validation = false;
    /// Optional substring used to select a matching physical-device name.
    std::string device_name;
    /// Use the native Vulkan large-points feature when available. Disable to
    /// exercise the portable expanded-point renderer or compare its performance.
    bool use_native_large_points = true;
};
#else
/**
 * \brief Requested OpenGL context settings for a Viewer.
 * \details OpenGL is selected by the Easy3D build configuration. The requested
 * context must support at least OpenGL 3.2.
 */
struct OpenGLOptions {
    /// Requested OpenGL context major version (at least 3).
    int major = 3;
    /// Requested OpenGL context minor version (at least 2).
    int minor = 2;
    /// Requested depth-buffer precision in bits.
    int depth_bits = 24;
    /// Requested stencil-buffer precision in bits.
    int stencil_bits = 8;
};
#endif
/**
 * \brief The default viewer implementation for both OpenGL and Vulkan backends.
 * \details Viewer provides a windowed interactive 3D scene, camera controls,
 * model and drawable management, event handling, picking, and image capture.
 * The graphics backend is chosen at build time. See ViewerBase for coordinate,
 * callback, ownership, and frame-lifecycle conventions. Use MultiViewer for a
 * grid of views or OffScreen for rendering without a displayed window.
 *
 * Subclasses may customize the protected drawing and event hooks. Backend
 * resources must be created and released only while the appropriate graphics
 * context/device is valid; see init() and cleanup().
 * \see ViewerBase, ViewerInput, ViewerCallbacks, MultiViewer, OffScreen
 */
class Viewer : public ViewerBase {
public:
    // ... some public functions: the unified APIs used by both backends

        /**
         * \brief Create a windowed viewer.
         * \param title The window title, which can be changed by set_title().
         * \param width Initial logical window width (positive).
         * \param height Initial logical window height (positive).
         * \param samples Requested multisample count; 0 or 1 disables MSAA.
         * \param full_screen Whether to use the primary monitor in fullscreen mode.
         * \param visible Whether the window is initially visible.
         * \param resizable Whether the user can resize the window.
         * \param options Backend-specific Vulkan device or OpenGL context options.
         * \details The final options parameter is backend-specific: Vulkan
         * builds accept VulkanOptions and OpenGL builds accept OpenGLOptions.
         */
		explicit Viewer(
			const std::string& title = "Easy3D Viewer",
			int width = 800,
			int height = 600,
			int samples = 1,
			bool full_screen = false,
			bool visible = true,
			bool resizable = true,

#ifdef EASY3D_HAS_VULKAN
            const VulkanOptions& options = {}
#else
            const OpenGLOptions& options = {}
#endif
		);

        /// Destroy the viewer and release its window and graphics resources.
		~Viewer() override;

        /**
         * \brief Run the viewer.
         * \param see_all See all contents within the screen? If true, the entire scene (i.e., all models) will be
         *      centered on the screen at a proper scale.
         * \return EXIT_SUCCESS when the loop exits normally, or EXIT_FAILURE
         *         when a standard exception is reported.
         * \details Initializes the viewer, optionally fits the complete scene,
         * and enters the event and rendering loop. Window visibility behavior
         * depends on the selected backend; use show() or hide() when an
         * explicit state is required. Use run_frames() when the current camera
         * must be preserved.
         */
        int run(bool see_all = true) override;
        /**
         * \brief Run the viewer loop for a frame budget.
         * \param max_frames Maximum number of successfully rendered frames;
         *        zero runs until the viewer is closed.
         * \return EXIT_SUCCESS on success or EXIT_FAILURE when a standard
         *         exception is reported by the viewer.
         * \details Unlike run(), this preserves the current camera and window
         * visibility. Frames skipped because the window is minimized or has a
         * zero-sized framebuffer do not count toward the budget.
         */
        int run_frames(unsigned max_frames = 0) override;

        /**
         * \brief Render and present at most one frame.
         * \return True if the frame was rendered and presented; false if the
         *         window is closed, minimized, or has no drawable framebuffer.
         * \details Does not poll events, show the window, or fit the scene.
         * Runs an enabled animation callback before pre_draw(), then invokes
         * the installed ViewerCallbacks and legacy drawing hooks in frame order.
         * Exceptions from rendering or callbacks are rethrown.
         */
        bool draw_frame() override;

        /// Poll pending window-system events without rendering a frame.
        static void poll_events();

        /// Request that the window close; destruction occurs separately.
        void request_close() override;

        /// Returns true after the window has been asked to close.
        /// \return True when a close request is pending.
        bool should_close() const override;

        /// Show the viewer window.
        void show() override;

        /// Hide the viewer window.
        void hide() override;

        /**
         * \brief Return the borrowed native GLFW window handle.
         * \return The viewer's window, or nullptr for a windowless Vulkan
         *         OffScreen host.
         * \details The viewer owns the handle; callers must not destroy it.
         * A non-null handle does not by itself guarantee that its graphics
         * context is current.
         */
        GLFWwindow* window() const override { return window_; }

        /**
         * \brief Terminate the viewer.
         */
        void exit() override;

        /// \name Viewer properties
        //@{

        /**
         * \brief Set the window title of the viewer.
         * \param title The string of the title.
         */
		void set_title(const std::string &title) override;

        /**
         * \brief Query the window title of the viewer.
         * \return The string of the window title.
         */
		const std::string& title() const override { return title_; }

        /**
         * \brief Set the viewer's logical window size.
         * \param w Requested logical width.
         * \param h Requested logical height.
         */
        void resize(int w, int h) override;

        /// Query the logical window size.
        /// \param w Receives the logical width.
        /// \param h Receives the logical height.
        void viewer_size(int& w, int& h) const override;

		/// Query the framebuffer size in physical pixels.
		/// \param w Receives the framebuffer width.
		/// \param h Receives the framebuffer height.
		void framebuffer_size(int& w, int& h) const override;

        /**
         * \brief Sets the position of the content area of the viewer.
         * \details This function sets the position, in screen coordinates, of the upper-left corner of the content
         *      area of the viewer. If the viewer is a full screen window, this function does nothing.
         *  \param xpos The x-coordinate of the upper-left corner of the content area.
         *  \param ypos The y-coordinate of the upper-left corner of the content area.
         *  \note This function must only be called from the main thread.
         */
        void set_position(int xpos, int ypos) override;

        /**
         * \brief Return the effective multisample count.
         * \details The requested count may be reduced to a supported count
         * during viewer creation. A result of 1 means multisampling is disabled.
         * \return Effective sample count.
         */
        int	 samples() const { return samples_ > 1 ? samples_ : 1; }
#ifdef EASY3D_HAS_VULKAN
        /**
         * \brief Set the Vulkan window's multisample count.
         * \param samples Supported nonzero power-of-two sample count.
         * \throws std::invalid_argument if the device does not support the count.
         */
	    void set_samples(unsigned samples);
#endif

        /**
         * \brief Query the framebuffer-to-window scale for high-DPI displays.
         * \return The high DPI scaling factor.
         */
        float dpi_scaling() const override;

        /**
         * \brief Return the number of graphics validation errors.
         * \return The backend's validation-error count; OpenGL builds currently
         *         report zero.
         */
        unsigned validation_errors() const override;

        /**
         * \brief Set the scene background color.
         * \param color The background color.
         */
        void set_background_color(const vec4& color) override { background_color_ = color; }

        /**
         * \brief Return the scene background color.
         * \return The background color.
         */
        const vec4& background_color() const override { return background_color_; }

        /// Return the mutable camera owned by the viewer.
        /// \return Pointer to the viewer-owned camera.
        Camera* camera() override { return camera_.get(); }
        /// Return the camera owned by the viewer for read-only access.
        /// \return Pointer to the viewer-owned camera.
        const Camera* camera() const override { return camera_.get(); }
        //@}

        /// \name File IO
        //@{

        /**
         * \brief Load and add a supported model file using a file-selection dialog.
         * \return True on success, false on cancellation or loading failure.
         * \details On success, the viewer retains ownership of the model and
         * creates its standard rendering drawables.
         */
        bool open() override;

        /**
         * \brief Save the active model (if exists) to a file.
         * \details This method saves the active model to a file. Internally, it will pop up a file
         *          dialog for specifying the file name.
         * \return true on success and false otherwise.
         */
        bool save() const override;
        //@}

        /// \name Model management
        //@{

        /**
         * \brief Add a model loaded from a file.
         * \param file_name Path to the model file.
         * \param create_drawables If true, create the model's standard
         *        rendering drawables. Pass false when you will add custom drawables.
         * \param fit_scene If true, fit the camera to the scene after addition; false
         *        keeps the current view.
         * \return The added model, or nullptr if loading or addition fails. The
         *        returned pointer is a non-owning observer owned by the viewer; do
         *        not delete it.
         * \details The viewer owns the loaded model (held internally by a shared_ptr).
         *        If a model with the same file name is already present, that model is
         *        returned unchanged, without re-fitting the camera, and a warning is
         *        logged.
         */
        Model* add_model(const std::string& file_name, bool create_drawables = true,
                         bool fit_scene = true) override;

        /**
         * \brief Add an existing model, sharing ownership with the caller.
         * \param model Shared pointer to the model.
         * \param create_drawables If true, create the model's standard
         *        rendering drawables. Pass false when you will add custom drawables.
         * \param fit_scene If true, fit the camera to the scene after addition; false
         *        keeps the current view (e.g. when attaching a helper/foreground model).
         * \return The added model, or nullptr if the model is null or empty, or if
         *        addition fails. The returned pointer is a non-owning observer owned
         *        by the viewer; do not delete it.
         * \details Ownership is shared: the viewer stores a copy of the shared_ptr, so
         *        the model lives until both the caller and the viewer release it. Do not
         *        delete it manually. Use this form when your code keeps using the model
         *        after adding it.
         *
         *        On success, adding installs a *new* Renderer on the model, replacing any
         *        renderer and drawables it already had. Create custom drawables after
         *        adding (or pass create_drawables = false and then add them to
         *        model->renderer()).
         *
         *        Adding the same model again returns the existing entry unchanged (no
         *        new renderer, no re-fit).
         * \sa add_model(Model*, bool, bool) for the legacy ownership-transferring raw-pointer form.
         */
        Model* add_model(std::shared_ptr<Model> model, bool create_drawables = true,
                         bool fit_scene = true) override;
		/**
		 * \brief Add a model supplied as a raw pointer, transferring ownership (legacy).
		 * \param model Pointer to a heap-allocated model; ownership is transferred to the viewer.
		 * \param create_drawables If true, create the model's standard
		 *        rendering drawables.
		 * \param fit_scene If true, fit the camera to the scene after addition; false
		 *        keeps the current view.
		 * \return The added model, or nullptr if the model is null or empty, or if
		 *        addition fails. The returned pointer is a non-owning observer owned by
		 *        the viewer; do not delete it.
		 * \details Legacy convenience equivalent to
		 *        add_model(std::shared_ptr<Model>(model), create_drawables, fit_scene).
		 *        The viewer takes sole ownership and deletes the model when it is removed or
		 *        the viewer is destroyed. After this call the caller must neither delete
		 *        the model nor wrap it in another shared_ptr (either would free it twice).
		 *        Ownership, renderer replacement, camera fitting and de-duplication behave
		 *        exactly as in the shared_ptr overload. Prefer the shared_ptr overload for
		 *        new code.
		 * \sa add_model(std::shared_ptr<Model>, bool, bool) for the shared-ownership form.
		 */
        Model* add_model(Model* model, bool create_drawables = true, bool fit_scene = true) override;

        /**
         * \brief Delete a model. The memory of the model will be released and its existing drawables
         *        also be deleted.
         * \param model The pointer to the model.
         * \return True if the model has been deleted.
         */
        bool delete_model(Model* model) override;

        /**
         * \brief Query the models managed by this viewer.
         * \return The models managed by this viewer.
         */
        const std::vector< std::shared_ptr<Model> >& models() const override { return models_; }

        /**
         * \brief Query the active model.
         * \details The viewer can manage/visualize/process multiple models. The default behavior
         *          of the Easy3D viewer is, when a command is triggerred (e.g., the Save menu was
         *          clicked), only the active model is processed. This method is used to identify
         *          the active model.
         * \return The active model.
         */
        Model* current_model() const override;
        //@}

        /// \name Drawable management
        //@{

        /**
         * \brief Add a standalone drawable, sharing ownership with the caller.
         * \param drawable Shared pointer to the drawable.
         * \return The added drawable, or nullptr if the pointer is null. The returned
         *        pointer is a non-owning observer owned by the viewer; do not delete it.
         * \details Ownership is shared: the viewer stores a copy of the shared_ptr, so
         *        the drawable stays alive until both the caller and the viewer release
         *        it. Do not delete it manually. Use standalone drawables to visualize
         *        geometry that is not owned by a Model. Unlike add_model, this does not
         *        create a renderer and does not fit the camera. If the same drawable is
         *        already managed by the viewer, it is returned unchanged (no duplicate).
         * \sa add_drawable(Drawable*) for the legacy ownership-transferring raw-pointer form.
         */
        Drawable* add_drawable(std::shared_ptr<Drawable> drawable) override;
		/**
		 * \brief Add a standalone drawable supplied as a raw pointer, transferring
		 *        ownership (legacy).
		 * \param drawable Drawable pointer; ownership is transferred to the viewer.
		 * \return The added drawable, or nullptr if the pointer is null. The returned
		 *        pointer is a non-owning observer owned by the viewer; do not delete it.
		 * \details Legacy convenience equivalent to
		 *        add_drawable(std::shared_ptr<Drawable>(drawable)). The viewer takes sole
		 *        ownership and deletes the drawable when it is removed or the viewer is
		 *        destroyed. After this call the caller must neither delete it nor wrap it
		 *        in another shared_ptr (either would free it twice). Ownership and
		 *        de-duplication behave exactly as in
		 *        add_drawable(std::shared_ptr<Drawable>). Prefer the shared_ptr overload
		 *        for new code.
		 * \sa add_drawable(std::shared_ptr<Drawable>) for the shared-ownership form.
		 */
        Drawable* add_drawable(Drawable* drawable) override;

        /**
         * \brief Delete a standalone drawable managed by the viewer.
         * \param drawable The pointer to the drawable.
         * \return True if the drawable was managed directly by the viewer and removed.
         */
	    bool delete_drawable(Drawable* drawable) override;

        /**
         * \brief Query the drawables managed by this viewer.
         * \return The drawables managed by this viewer.
         */
        const std::vector< std::shared_ptr<Drawable> >& drawables() const override { return drawables_; }

        /**
         * \brief Delete all visual contents of the viewer (all models and drawables).
         */
        void clear_scene() override;
        //@}

        /// \name UI-related functions
        //@{

        /**
         * \brief Request a repaint of the viewer.
         * \details Call after changing data or view state that should be rendered.
         */
        void update() const override;

        /**
         * \brief Moves the camera so that the entire scene or the active model is centered on the
         *        screen at a proper scale.
         * \param model The pointer to the model to be centered on the screen. If nullptr, the
         *        entire scene (i.e., all models) will be centered on the screen at a proper scale.
         */
        void fit_screen(const Model* model = nullptr) override;

        /**
         * \brief Take a snapshot of the screen and save it to a file.
         * \details This method takes a snapshot of the screen and saves the snapshot into an image file.
         *          Internally, it will pop up a file dialog for specifying the file name.
         * \return true on success and false otherwise.
         */
	    bool snapshot() const override;

        /**
         * \brief  Take a snapshot of the screen and save it to an image file. Supported image format: png, jpg, bmp, and tga.
         * \details This function renders the scene into a framebuffer and takes a snapshot of the framebuffer.
         *      It allows the snapshot image to have a dimension different from
         *      the viewer. The output size is subject to available memory and
         *      graphics-backend limits.
         * \param file_name The image file name.
         * \param scaling The scaling factor that determines the size of the image (default to 1.0, using the viewer size), i.e.,
         *      image_width = viewer_width * scaling;
         *      image_height = viewer_height * scaling;
         * \param samples The required number of samples for antialiased rendering (which can be different from
         *      that of the default framebuffer). The default value is 4.
         * \param background Determines the background color. 0: current color (default); 1: white; 2: transparent.
         * \param expand If true, expand the frustum as needed to match the
         *        output aspect ratio while retaining the visible scene.
         * \return true on success and false otherwise.
         */
		bool snapshot(const std::string& file_name, float scaling = 1.0f, int samples = 4, int background = 0, bool expand = true) const override;

        /**
         * \brief Set the viewer's usage text and on-screen hint.
         * \param manual Text printed to the console; "default" requests the
         *        built-in shortcut manual.
         * \param screen_hint Short text shown near the top-left of the window.
         * \attention The default shortcuts could be overridden by bind() function calls.
         */
        void set_usage(const std::string& manual = "default", const std::string& screen_hint = "") override;
	    //@}

        /// \name Algorithm execution
        //@{

        // Key, Modifier and Button are shared with Viewer through ViewerInput.
        /**
         * \brief Function type for a user operation bound to a keyboard shortcut.
         * \param viewer A pointer to this viewer.
         * \param model The model to be operated on.
         * \return True when the operation succeeds.
         * \par Example
         *      \code
         *          bool reconstruct(Viewer* viewer, Model* model) {
         *              auto cloud = dynamic_cast<PointCloud *>(model);
         *              auto mesh = surface_reconstruction(cloud);
         *              if (!mesh)
         *                  return false;
         *              viewer->add_model(mesh);
         *              viewer->update();
         *              return true;
         *          }
         *      \endcode
         */
        using Function = std::function<bool(Viewer* viewer, Model* model)>;

        /**
         * \brief Bind a function that will be triggered by the shortcut 'modifier + key'.
         * \details This operation will overwrite the previous function (if exist) bound to the same key-modifier shortcut.
         * \param func The function to be executed, which will be triggered by the shortcut.
         * \param model The model to be processed.
         * \param key The shortcut key.
         * \param modifier The shortcut key modifier (e.g., Ctrl, Shift, Alt). This can be MODIF_NONE.
         * \sa Function
         */
        void bind(const Function& func, Model* model, Key key, Modifier modifier = MODIF_NONE) {
            commands_[key][modifier] = std::make_pair(func, model);
        }
        //@}

	    /// \name Animation
	    //@{

	    /**
	     * \brief Install or clear the frame-based animation callback.
	     * \param callback Function invoked on the UI thread before the normal
	     *        pre-draw hook for each renderable frame. An empty function
	     *        clears the callback.
	     * \details Install a callback and call set_animation(true) to enable
	     * updates. The callback's return value is advisory; it does not stop
	     * animation automatically.
	     */
	    void set_animation_callback(const ViewerBase::AnimationFunction& callback) override {
	        if (callback) animation_func_ = [callback](Viewer* viewer) { return callback(viewer); };
	        else animation_func_ = nullptr;
	    }

        /// Enable or disable frame-based animation.
        /// \param b True to enable animation; false to pause it.
        /// \details Animation runs only when an animation callback has been installed.
        void set_animation(bool b) override;

        /// Returns true when animation is enabled and a callback is installed.
        /// \return True when the viewer will invoke its animation callback.
        bool is_animating() const override;
        //@}

        /// \name Picking
        //@{
        /**
         * \brief Outcome of a synchronous visible-surface pick.
         * \details The query uses logical window coordinates; the world point is
         *      resolved through the camera of this host. Standalone drawables have a
         *      model of null. Face indices describe the picked surface at query time and
         *      are not updated by later topology edits.
         */
        struct PickResult {
            /// Owning model for the hit; null when it came from a standalone drawable.
            std::shared_ptr<Model> model;
            /// Drawable that produced the hit; shared ownership keeps it alive.
            std::shared_ptr<Drawable> drawable;
            /// Generated primitive index, or UINT32_MAX when unavailable.
            std::uint32_t primitive = UINT32_MAX;
            /// Original SurfaceMesh face index when provenance is available; otherwise -1.
            int face_index = -1;
            /// World-space hit position at the queried pixel center.
            vec3 position = vec3(0);
            /// Normalized scene depth at the hit; 0 is near and 1 is far.
            float depth = 1;
            /// True when a drawable was hit.
            explicit operator bool() const { return bool(drawable); }
        };

        /**
         * \brief Pick the nearest visible scene primitive at a logical window pixel.
         * \param x Logical window x coordinate, with the origin at top-left.
         * \param y Logical window y coordinate, with the origin at top-left.
         * \return PickResult with the hit drawable and available provenance;
         *         an empty result when the pixel is outside the logical window or
         *         shows only the background.
         * \details This synchronous whole-scene query retains shared ownership
         * of the hit model and drawable. A returned face index describes the
         * mesh at query time and is not updated after topology edits. HUD and
         * overlay content is not pickable. Coordinates are DPI-independent and
         * identical on the OpenGL and Vulkan backends.
         */
        PickResult pick(int x, int y) const;
        //@}


        /// \name Overlays and clipping
        //@{

        /**
         * \brief Replace the viewer's custom overlay draw commands.
         * \param commands Commands to retain and draw.
         * \param foreground If true, draw this set after the built-in viewer HUD.
         * \details Geometry and images are retained by the commands; callers
         * provide the transforms. Scene-depth annotations are depth-tested and
         * blended before independent-depth 3D HUD and flat HUD commands. Their
         * input order is preserved; overlapping scene-depth annotations should
         * be supplied back-to-front. These overlays do not write scene depth or
         * object IDs.
         */
	    void set_overlays(std::vector<graphics::DrawCommand> commands, bool foreground = false);

        /**
         * \brief Return the effective world-space clipping planes.
         * \details A shared ClippingPlane takes precedence. Vulkan builds may
         * also support explicitly configured per-viewer planes; see
         * set_clipping_planes().
         * \return Reference to the effective world-space plane equations.
         */
	    const std::vector<vec4>& clipping_planes() const;

#ifdef EASY3D_HAS_VULKAN
        /**
         * \brief Set Vulkan-only world-space clipping halfspaces for this viewer.
         * \param planes Plane equations; a point is retained when
         *        dot(plane, vec4(world, 1)) is nonnegative.
         * \details Pass an empty vector to disable explicit planes. At most two
         * finite planes with nonzero normals are accepted. An enabled shared
         * ClippingPlane takes precedence. Overlays are not clipped, and
         * clear_scene() does not reset this configuration.
         * \throws std::invalid_argument for unsupported or invalid planes.
         */
	    void set_clipping_planes(const std::vector<vec4>& planes);
#endif
        //@}

        /// \name Point selection
        //@{

        /**
         * \brief Select point indices intersecting a graphics selection request.
         * \param request Selection region, point data, and selection options.
         * \return Indices into the request's point data selected by the backend.
         * \details This is GPU-backed selection. For portable requests, obtain
         * point data with point_resource(). OpenGL requests must use a native
         * point resource created by this viewer; Vulkan also accepts CPU
         * PointData.
         */
	    std::vector<std::uint32_t> select_points(const graphics::PointSelectionRequest& request) const;

        /**
         * \brief Get the immutable point positions for a point drawable.
         * \param drawable Drawable whose positions are requested.
         * \return Shared point data for the drawable.
         * \details Positions are not clipped and are shared by rendering and
         * selection. OpenGL resources are native snapshots usable only by the
         * viewer that created them. Position extraction is refreshed when
         * needed; attribute-only buffer changes retain the position data.
         * \throws std::invalid_argument if drawable is null, is not a point
         * drawable, or is not managed by this viewer.
         */
	    std::shared_ptr<const graphics::PointData> point_resource(Drawable* drawable) const;
        //@}

        /// \name Image capture
        //@{

        /**
         * \brief Capture the current viewer image and scene attachments.
         * \return Frame image containing composited RGBA color plus available
         *         scene depth and object/primitive identifiers.
         * \details Uses the viewer's effective sample count for color rendering.
         */
	    graphics::FrameImage capture() const;

        /**
         * \brief Capture the current viewer image with a requested color sample count.
         * \param samples Requested multisample count for color rendering.
         * \return Frame image containing composited RGBA color plus available
         *         scene depth and object/primitive identifiers.
         * \details Depth and object identifiers are center-sampled and exclude
         * HUD overlays. Default object IDs index models() followed by standalone
         * drawables(). OpenGL may report UINT32_MAX when a primitive identifier
         * is unavailable. Invalid or unsupported requests may throw.
         */
	    virtual graphics::FrameImage capture(unsigned samples) const;
#ifdef EASY3D_HAS_VULKAN

        /**
         * \brief Scan visible scene surfaces into one point per covered pixel.
         * \param samples Requested color sample count for the capture.
         * \return World-space points in framebuffer row order, top to bottom,
         *         with one point for each non-background pixel.
         * \details Uses pixel centers and one camera snapshot. Overlays are
         * excluded; color multisampling does not change center-sampled depth
         * semantics. The operation does not modify the scene or camera and
         * does not advance animation.
         */
	    std::vector<vec3> scan_visible_points(unsigned samples = 1) const;
#endif
        //@}

#ifdef EASY3D_HAS_VULKAN
        /// \name Vulkan render device
        //@{

        /// Return the Vulkan render device used by this viewer.
	    const graphics::RenderDevice& device() const;
        //@}
#endif

        /// \name Public data
        //@{

	    /**
	     * \brief Public animation callback invoked before pre_draw().
	     * \details Invoked on the UI thread once per renderable frame while
	     * animation is enabled. It is frame-based, not a fixed-rate clock; its
	     * return value does not automatically disable animation. Prefer
	     * set_animation_callback() to install or clear it.
	     */
	    std::function<bool(Viewer* viewer)> animation_func_;
        //@}

	protected:

        /**
         * \brief Draw the current scene.
         * \details Called between pre_draw() and post_draw(). The base
         * implementation performs the backend's normal scene rendering;
         * overrides may add custom rendering and call the base implementation
         * to retain that rendering.
         */
        virtual void draw() const;

        /**
         * \brief Initialize viewer rendering resources.
         * \details Called before the first frame's draw procedure, when the
         * backend is ready for resource creation. Overrides should call
         * Viewer::init() before creating subclass resources.
         */
        virtual void init();

        /**
         * \brief Release viewer rendering resources while the backend is valid.
         * \details Overrides should release subclass resources and call
         * Viewer::cleanup() as part of the cleanup sequence. Do not assume a
         * graphics context is current in a destructor.
         */
        void cleanup();

        /**
         * \brief Hook called before scene drawing on each rendered frame.
         * \details Runs after the installed ViewerCallbacks::pre_draw callback.
         */
        virtual void pre_draw();

        /**
         * \brief Hook called after scene drawing on each rendered frame.
         * \details The default implementation draws the viewer HUD. It runs
         * before the installed ViewerCallbacks::post_draw callback.
         */
        virtual void post_draw();

        /**
         * \brief Hook called after the logical viewer size changes.
         * \param w New logical width.
         * \param h New logical height.
         * \details These are logical window dimensions, not framebuffer pixels.
         */
        virtual void post_resize(int w, int h) { (void)w, (void)h; }

        /**
         * \brief Handle a mouse-button press.
         * \param x Logical x coordinate from the top-left.
         * \param y Logical y coordinate from the top-left.
         * \param button ViewerInput::Button identifier.
         * \param modifiers ViewerInput::Modifier bit mask.
         * \return True to consume the event before built-in controls.
         * \details All mouse hooks use DPI-independent logical window coordinates
         * on both the OpenGL and Vulkan backends, matching ViewerCallbacks and
         * world_to_window()/window_to_world().
         */
        virtual bool mouse_press_event(int x, int y, int button, int modifiers);

        /**
         * \brief Handle a mouse-button release.
         * \param x Logical x coordinate from the top-left.
         * \param y Logical y coordinate from the top-left.
         * \param button ViewerInput::Button identifier.
         * \param modifiers ViewerInput::Modifier bit mask.
         * \return True to consume the event before built-in controls.
         */
        virtual bool mouse_release_event(int x, int y, int button, int modifiers);

        /**
         * \brief Handle pointer motion while a mouse button is pressed.
         * \param x Current logical x coordinate from the top-left.
         * \param y Current logical y coordinate from the top-left.
         * \param dx Horizontal logical displacement since the previous event.
         * \param dy Vertical logical displacement since the previous event.
         * \param button ViewerInput::Button identifier.
         * \param modifiers ViewerInput::Modifier bit mask.
         * \return True to consume the event before built-in controls.
         */
        virtual bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers);

        /**
         * \brief Origin subtracted from mouse coordinates before camera manipulation.
         * \return Logical window coordinates of the interaction region's top-left corner.
         * \details Mouse state stays in whole-window coordinates; multi-view layouts
         * rebase camera frame actions onto the active cell through this hook.
         */
        virtual ivec2 interaction_origin() const { return ivec2(0); }

        /**
         * \brief Handle pointer motion when no mouse button is pressed.
         * \param x Current logical x coordinate from the top-left.
         * \param y Current logical y coordinate from the top-left.
         * \param dx Horizontal logical displacement since the previous event.
         * \param dy Vertical logical displacement since the previous event.
         * \param modifiers ViewerInput::Modifier bit mask.
         * \return True to consume the event.
         */
        virtual bool mouse_free_move_event(int x, int y, int dx, int dy, int modifiers);

        /**
         * \brief Handle a mouse-wheel event.
         * \param x Logical x coordinate from the top-left.
         * \param y Logical y coordinate from the top-left.
         * \param dx Horizontal scroll offset.
         * \param dy Vertical scroll offset.
         * \return True to consume the event before built-in controls.
         */
        virtual bool mouse_scroll_event(int x, int y, int dx, int dy);

        /**
         * \brief Handle text input as a Unicode code point.
         * \param codepoint Native-endian UTF-32 code point.
         * \return True to consume the event.
         */
        virtual bool char_input_event(unsigned int codepoint);

        /**
         * \brief Handle a physical key press.
         * \param key ViewerInput::Key identifier.
         * \param modifiers ViewerInput::Modifier bit mask.
         * \return True to consume the event before built-in shortcut handling.
         * \details This reports a key identifier, not the entered text; use
         * char_input_event() for text.
         */
        virtual bool key_press_event(int key, int modifiers);

        /**
         * \brief Handle a physical key release.
         * \param key ViewerInput::Key identifier.
         * \param modifiers ViewerInput::Modifier bit mask.
         * \return True to consume the event.
         */
        virtual bool key_release_event(int key, int modifiers);

        /**
         * \brief Handle files dropped onto the viewer.
         * \param filenames Dropped file paths.
         * \return True to consume the event.
         */
        virtual bool drop_event(const std::vector<std::string> & filenames);

        /**
         * \brief Handle a viewer focus change.
         * \param focused True if focus was gained, false if lost.
         * \return True to consume the event.
         */
        virtual bool focus_event(bool focused);

        // Publishes the RGB corner axes as a 3D HUD overlay. Vulkan builds the cylinder/
        // cone/sphere geometry once and caches it in corner_axes_geometry_, skipping the
        // overlay while show_corner_axes_ is off; OpenGL keeps the same shape in
        // drawable_axes_ and hides it by toggling that drawable's visibility instead.
        void draw_corner_axes() const;

        // Copies the current view to the system clipboard as "x y z qx qy qz qw"
        // (camera position then orientation, 9 digits of precision).
        void copy_view();

        // Restores the view from the clipboard text written by copy_view(), ignoring
        // malformed or non-finite input and renormalizing the quaternion before
        // assigning position and orientation.
        void paste_view();
    protected:
        // Adapts a backend-neutral ViewerBase::Function to the Viewer-internal bind()
        // overload, which also receives the target model.
        void bind_command(const ViewerBase::Function& func, Model* model, Key key, Modifier modifier) override {
            bind(Function([func](Viewer* viewer, Model* m) { return func(viewer, m); }), model, key, modifier);
        }

        // Reads the scene depth of one framebuffer pixel (x, y, top-left origin): renders
        // the whole scene with the snapshot camera into a framebuffer-sized, single-sample
        // target and samples its depth center. The flag reports a primitive hit on Vulkan
        // (unset object id); the OpenGL pass has no id to test and always returns true, so
        // callers additionally reject the far-plane depth of background pixels.
        bool read_scene_depth(int x, int y, const Camera& snapshot, float& depth) const override;

        // Sample count the backend window actually uses, for the base class's
        // capture/quality decisions; equals the clamped samples() value.
        unsigned window_samples() const override { return unsigned(samples()); }

	protected:
	    GLFWwindow*	window_;        // the GLFW window: created by this Viewer, or adopted from a host window
	    int         width_;         // logical window width in content pixels (GLFW size; the
	    int         height_;        // framebuffer size comes from framebuffer_width/height())
	    std::string	title_;         // window title, also mirrored to the host window on Vulkan
	    std::unique_ptr<Camera> camera_;   // the one camera manipulated by events, animations and picking
	    std::unique_ptr<KeyFrameInterpolator> kfi_; // camera-path recorder/player behind the animation keys
	    std::string manual_;        // shortcut manual printed once on the first drawn frame (empty = none)
	    std::string hint_; // shown in the top-left corner of the screen
	    int		samples_;	// the actual samples
	    vec4	background_color_;  // clear color of the color/depth pass
	    int     pressed_key_;       // key held down (-1 none): Z selects dolly/pivot, X-Y-O axis-locked moves
	    bool    show_frame_rate_;   // HUD toggle: show the frame-rate readout
	    bool	show_camera_path_;  // HUD toggle: show the camera path and its keyframe cameras
	    bool    show_easy3d_logo_;  // HUD toggle: show the Easy3D logo text
	    std::vector< std::shared_ptr<Model> > models_;    // models in insertion order (add_model/remove/close)
	    int model_idx_;             // model addressed by the model-bound commands, -1 when there is none
	    std::vector< std::shared_ptr<Drawable> > drawables_;  // standalone drawables, not owned by any model
	    bool initialized_ = false;  // latch ensuring init() runs exactly once before the first frame
	    int pressed_button_ = -1;   // mouse button currently held, -1 when none
	    int mouse_current_x_ = 0, mouse_current_y_ = 0;   // cursor position in logical window pixels
	    int mouse_pressed_x_ = 0, mouse_pressed_y_ = 0;   // cursor position where the ongoing drag started
	    bool show_pivot_point_ = false;  // HUD toggle: pivot mark, auto-hidden after a few seconds
	    typedef std::pair<Function, Model*> FunctionModel;
	    std::map<Key, std::map<Modifier, FunctionModel> >  commands_; // key+modifier -> bound action and its model
	private:
	    // Steps the camera-path interpolation one keyframe forward, rate-limited by
	    // next_camera_path_frame_ so playback follows the keyframe period.
	    void advance_camera_path();

	    // Time the camera path may advance to next.
	    std::chrono::steady_clock::time_point next_camera_path_frame_;

	protected:
	    /// \name Frame-loop and HUD helpers
	    //@{

	    // Counts rendered frames inside one-second windows and rebuilds the framerate_
	    // string that the HUD shows; does nothing while show_frame_rate_ is off.
	    void update_frame_rate();

	    // Prints the shortcut manual to the console the first time a frame is drawn
	    // (latched by usage_printed_ so it never repeats).
	    void print_usage();

	    // Rebuilds the scene bounding box from all models and standalone drawables
	    // (screen-space lines excluded) and hands it to the camera.
	    void restore_scene_bounds();
	    //@}

	    bool snapshot_hud_ = true; // Capture HUD policy; OffScreen/MultiViewer are scene-only.
	                               // File snapshots never carry the Viewer HUD, on either backend.
	    struct DrawSource { std::shared_ptr<Model> model; std::shared_ptr<Drawable> drawable; };

	    // State shared by both backends (the frame loop, events, HUD and model management
	    // are backend-independent; only rendering and window creation differ).
	    bool animation_enabled_ = false;
	    bool visible_ = true;
	    bool zoom_region_ = false;
	    std::exception_ptr callback_error_;
	    int framebuffer_width_ = 0, framebuffer_height_ = 0; // cached physical pixels
	    std::string framerate_ = "fps: -- (-- ms/frame)";
	    std::chrono::steady_clock::time_point frame_rate_start_;
	    unsigned frame_rate_frames_ = 0;
	    bool usage_printed_ = false;
#ifdef EASY3D_HAS_VULKAN
	    // Keep this part of code minimum
	private:
	    struct Window;
	    struct Impl;

	    /// \name Vulkan frame composition
	    //@{

	    // Rebuilds the camera-path overlay (path polyline plus per-keyframe frustum
	    // markers) from the interpolator and caches its draw commands.
	    void update_camera_path_overlays() const;

	    // Rebuilds the corner axes, pivot mark and hint commands, skipping the work
	    // while the window size, DPI, flags and hint text are all unchanged.
	    void update_interaction_overlays(bool force = false) const;

	    // Renders one transparency method (1 average color blending, 2 depth peeling,
	    // 3 per-pixel linked lists) over the frame's triangle drawables and
	    // composites the result over the opaque scene.
	    void draw_transparency(const std::vector<TrianglesDrawable*>& surfaces, int method, int peels);

	    // Renders one file-snapshot frame through the virtual draw() into an
	    // offscreen image of the requested size, clear color and sample count,
	    // then restores the camera. OpenGL binds a framebuffer around draw();
	    // the device has no framebuffer to bind, so the offscreen target lives
	    // in Impl and draw() submits into it while this runs. Like the OpenGL
	    // snapshot, the file frame carries no Viewer HUD.
	    bool snapshot_frame(int width, int height, const mat4& projection,
	                        const vec4& background, unsigned samples,
	                        graphics::FrameImage& image);

	    // Loads the bundled fonts into the text layout engine once; a failure is
	    // latched in fonts_tried_ so the HUD simply draws no text.
	    void ensure_text_renderer() const;

	    // Ends in-flight camera and clipping-plane manipulation and hides the
	    // transient interaction overlays (pivot mark, zoom region).
	    void cancel_interaction();

	    // Window-backed construction: adopts an already created GLFW window host and
	    // builds the render device for it. The public constructor delegates here.
	    Viewer(std::shared_ptr<Window> window, const VulkanOptions& options);
	    //@}

	    /// \name Vulkan handles and cached overlay state
	    //@{
	    // Owns the surface, device and swapchain, and queues the raw GLFW input state
	    // that the window callbacks forward to this viewer.
	    std::shared_ptr<Window> window_host_;
	    // Render device plus the cached scene/overlay command sets and pick resources;
	    // defined in viewer.cpp so this header does not pull in the device headers.
	    std::unique_ptr<Impl> impl_;
	    // Produces the camera-path polyline and per-keyframe frustum commands.
	    std::unique_ptr<vulkan::KeyFrameInterpolatorRenderer> camera_path_renderer_;
	    vec2 pivot_pixel_ = vec2(0);        // logical position of the pivot marked by the last pick
	    std::chrono::steady_clock::time_point pivot_expires_;   // pivot mark auto-hides once this instant passes
	    mutable bool pivot_hint_visible_ = false;   // pivot state of the cached overlays, lets rebuilds be skipped
	    mutable int hint_width_ = 0, hint_height_ = 0;  // framebuffer size the cached text was laid out for
	    mutable bool rendered_show_frame_rate_ = false, rendered_show_logo_ = false;  // HUD flags behind the cached text
	    mutable std::string rendered_frame_rate_; // frame-rate string the cached commands were built from
	    mutable std::vector<graphics::DrawCommand> logo_commands_;    // "Easy3D" text, rebuilt only when its inputs change
	    mutable std::vector<graphics::DrawCommand> frame_rate_commands_;  // cached frame-rate text commands
	    mutable std::vector<graphics::DrawCommand> hint_commands_;    // cached hint text commands
	    mutable std::shared_ptr<const graphics::Geometry> corner_axes_geometry_;  // corner axes, built on first use
	    mutable std::unique_ptr<graphics::TextLayoutEngine> texter_;  // lays out the HUD text as draw commands
	    mutable bool fonts_tried_ = false;    // latch: font loading attempted, so failures are not retried
	    mutable std::string rendered_hint_;   // hint text the cached commands were built from
	    mutable float hint_dpi_ = 0;          // DPI scale the cached text layout used
	    //@}

	protected:
	    /// \name Vulkan command assembly
	    //@{

	    // Scene commands for one camera and output size, recording in sources which
	    // model or standalone drawable produced each command.
	    virtual std::vector<graphics::DrawCommand> draw_commands(const Camera& camera, int width, int height,
	                                                           std::vector<DrawSource>* sources = nullptr, bool include_hidden_models = false) const;

	    // Scene commands plus the current annotations and, when include_hud is
	    // set, the Viewer HUD overlays, with overlay viewports scaled from the
	    // window to the requested output size. Live frames and captures include
	    // the HUD; file snapshots do not, matching the OpenGL snapshot.
	    std::vector<graphics::DrawCommand> frame_commands(const Camera& camera, int width, int height,
	                                                      std::vector<DrawSource>* sources = nullptr,
	                                                      bool include_hud = true) const;

	    // Device-only construction for OffScreen: creates neither a GLFW window nor a surface.
	    Viewer(int width, int height, const VulkanOptions& options);

	    // Updates the cached physical size, the logical camera extent and the
	    // swapchain render pass after a resize or when an offscreen target is created.
	    void resize_render_target(int width, int height);

	    // Replacement sets for the three overlay slots; each rewrites the commands'
	    // overlay flags so they draw after the scene: flat HUD (no depth test), 3D HUD
	    // (own depth range, no scene depth test) and scene-depth annotations.

	    // Flat HUD slot: overlay on, both depth tests off, so nothing occludes it.
	    void set_viewer_overlays(std::vector<graphics::DrawCommand> commands) const;

	    // 3D HUD slot (corner axes, pivot): overlay and independent depth on, scene depth
	    // test off, so the marks draw in their own depth range above the scene.
	    void set_viewer_3d_overlays(std::vector<graphics::DrawCommand> commands) const;

	    // Annotation slot: overlay and scene depth test on, so labels and markers are
	    // occluded by geometry in front of them.
	    void set_viewer_annotations(std::vector<graphics::DrawCommand> commands) const;

	    // Destroys the render device and its swapchain while the window still exists.
	    void release_device();
	    //@}

	    bool show_corner_axes_ = true;   // HUD toggle (key 'A'); OpenGL uses drawable_axes_ visibility instead

#else
    // Keep this part of code minimum

	protected:
        // Scene pass used by the query paths: with a source it draws only that model or
        // standalone drawable through query_draw(); otherwise every visible model
        // drawable and every standalone drawable. With guides it also draws the
        // clipping-plane guide and the screen-space lines depth-test free, on top.
        virtual void draw_scene(const Camera& snapshot, const DrawSource* source = nullptr, bool guides = false) const;
        // Selects the framebuffer region used by the primitive-ID picking pass.
        // All four arguments are in framebuffer pixels (top-left origin).
        virtual void prepare_pick_view(int x, int y, int width, int height) const;
        // Creates the GLFW window and its context: sets the window hints (MSAA samples,
        // depth/stencil bits, a forward-compatible core profile at gl_major.gl_minor,
        // full-screen or resizable), acquires the shared GLFW runtime, makes the context
        // current and stores the window pointer. Returns null when creation fails.
        GLFWwindow *create_window(const std::string &title,
                                  int samples,
                                  int gl_major,   // must >= 3
                                  int gl_minor,   // must >= 2
                                  bool full_screen,
                                  bool resizable,
                                  int depth_bits,
                                  int stencil_bits,
                                  int width,
                                  int height);

        // Installs the GLFW callback trampolines on the window; each one resolves the
        // user pointer back to this Viewer through dispatch_callback() and forwards to
        // the matching callback_event_* below.
        void setup_callbacks(GLFWwindow*);

		/* Event handlers. Client code should not touch these */
        // Each handler first offers the event to the commands bound through bind(), and
        // only then to the corresponding virtual event hook; returning true consumes the
        // event. An escaping exception is recorded in callback_error_ instead of
        // propagating through GLFW.
        // Cursor movement in logical window coordinates: caches the position, derives
        // the delta and dispatches a drag or free-move event depending on drag_active_.
        virtual bool callback_event_cursor_pos(double x, double y);
        // Mouse press/release: latches the drag state and press position, then
        // dispatches the press/release hooks; release also ends the active camera and
        // clipping-plane manipulation frames.
        virtual bool callback_event_mouse_button(int button, int action, int modifiers);
        // Key press/repeat/release: dispatches the bound shortcut first, then the
        // key_press_event()/key_release_event() hooks, and clears pressed_key_ on release.
        virtual bool callback_event_keyboard(int key, int action, int mods);
        // A text character was typed: bound handlers first, then char_input_event().
        virtual bool callback_event_character(unsigned int codepoint);
        // Files dropped onto the window: bound handlers first, then drop_event(), which
        // loads them as models by default.
        virtual bool callback_event_drop(int count, const char **filenames);
        // Mouse-wheel scroll: accumulates fractional deltas in scroll_remainder_x_/y_ and
        // forwards whole-notch amounts to the scroll handlers, so slow scrolling still
        // steps exactly once per notch.
        virtual bool callback_event_scroll(double dx, double dy);
        // Window resize in logical units: refreshes the cached logical and framebuffer
        // sizes, updates the camera extent and the GL viewport, then calls post_resize()
        // and notify_resize() when the size actually changed.
        virtual void callback_event_resize(int w, int h);

        // Draws one text label per mesh face at its projected pixel position, reading
        // back the stored depth first so only faces visible at that pixel are labelled.
        void draw_face_labels(Model* model, TextRenderer* texter, int font_id, const vec3& color) const;
        // Draws one text label per mesh vertex, using the same depth-tested placement as
        // draw_face_labels().
        void draw_vertex_labels(Model* model, TextRenderer* texter, int font_id, const vec3& color) const;

    private:

        // GLFW callback trampoline: resolves the window's user pointer back to this
        // Viewer, invokes the handler and swallows any escaping exception, since
        // GLFW must never see a C++ exception cross its callback boundary.
        template<class Callback> static void dispatch_callback(GLFWwindow*, const Callback&) noexcept;

        // Keeps GLFW initialized while any viewer window lives, and supplies the
        // hidden window used as the share context for new GL windows.
        std::shared_ptr<detail::GlfwRuntime> runtime_;
        int callback_mouse_x_ = 0, callback_mouse_y_ = 0;   // cursor as last seen by a GLFW callback, so
                                                            // absolute positions can be turned into deltas
        double scroll_remainder_x_ = 0, scroll_remainder_y_ = 0; // fractional wheel ticks carried over

    protected:

        // Draws the retained custom overlay commands (foreground == true selects the
        // set drawn after the built-in HUD, false the scene-depth set).
        void draw_overlays(bool foreground) const;

        float       dpi_scaling_;   // window content scale (framebuffer pixels / logical pixels)

		// enable/disable event processing
		bool	process_events_;

        std::unique_ptr<TextRenderer> texter_;    // HUD text renderer, created with the current dpi_scaling_

		// mouse
		int		modifiers_;		    // for mouse drag
		bool	drag_active_;	    // for mouse drag


		//----------------- viewer data -------------------

		// corner axes
        std::unique_ptr<TrianglesDrawable> drawable_axes_;

        // pivot cross
        std::unique_ptr<LinesDrawable2D> drawable_pivot_;

        // camera path

        // drawables independent of any model

        // Retains the overlay/annotation command sets registered through
        // set_viewer_overlays()/set_viewer_annotations() and replays them in draw_overlays().
        std::unique_ptr<opengl::OverlayRenderer> overlay_renderer_;
        // OpenGL point-selection pass (render points into an id buffer, then read back);
        // lazily created and mutable because selection is queried from const paths.
        mutable std::unique_ptr<opengl::PointSelector> point_selector_;
        // Scratch buffer that clipping_planes() fills with the active ClippingPlane
        // instance's planes, normalized, so the const getter can return them by reference.
        mutable std::vector<vec4> selection_planes_;
        // Viewport tile (x, y, w, h) currently rendered; nonzero only while a tiled
        // snapshot is in flight, and used to keep overlays aligned inside the tile.
        mutable ivec4 overlay_tile_ = ivec4(0);
#endif
};
}
#include <easy3d/viewer/multi_viewer.h>
#include <easy3d/viewer/offscreen.h>
#endif // EASY3D_VIEWER_VIEWER_H
