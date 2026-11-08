// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_OFFSCREEN_H
#define EASY3D_VIEWER_OFFSCREEN_H
#include <easy3d/viewer/viewer.h>
namespace easy3d {
/**
 * \brief Render an Easy3D scene into an image without showing a window.
 *
 * \details OffScreen exposes the scene, camera, picking, overlay, and capture
 * operations needed for programmatic rendering while keeping Viewer window
 * controls protected. OpenGL builds use an offscreen context; Vulkan builds
 * create a device without a GLFW window or presentation surface. Backend
 * selection is determined by the Easy3D build.
 *
 * The canvas has a fixed pixel size and DPI scale 1. Resize it explicitly with
 * resize(). File rendering is performed by render(); this class does not
 * provide an interactive run loop.
 */
class OffScreen : protected Viewer {
public:
    /**
     * \brief Create an offscreen rendering canvas.
     * \param width Positive canvas width in pixels.
     * \param height Positive canvas height in pixels.
     * \param options Backend-specific Vulkan device or OpenGL context options.
     * \details OpenGL construction creates a hidden context. Vulkan
     * construction creates no window or surface.
     */
    explicit OffScreen(int width = 800, int height = 600,
#ifdef EASY3D_HAS_VULKAN
                       const VulkanOptions& options = {});
#else
                       const OpenGLOptions& options = {});
#endif
    /// Release the offscreen canvas and its graphics resources.
    ~OffScreen() override;

    /**
     * \brief Render the current scene to an image file.
     * \param filename Destination image path.
     * \param scaling Output-size multiplier relative to the canvas size.
     * \param samples Requested antialiasing sample count.
     * \param background Background mode: 0 uses the configured background,
     *        1 uses white, and 2 uses transparency.
     * \param expand If true, expand the frustum as needed for the output aspect ratio.
     * \return True on success, false if the request or image write fails.
     * \details Rendering does not display a window or change the live camera,
     * visibility, or animation state. Output dimensions are constrained by
     * available memory and backend limits.
     */
    bool render(const std::string& filename, float scaling = 1, int samples = 4,
                int background = 0, bool expand = true) const;

    /**
     * \brief Resize the offscreen canvas.
     * \param width New positive canvas width in pixels.
     * \param height New positive canvas height in pixels.
     * \throws std::invalid_argument if either dimension is not positive.
     */
    void resize(int width, int height) override;

    /**
     * \brief Query the framebuffer size; for OffScreen it equals the canvas size.
     * \param width Receives the canvas width in pixels.
     * \param height Receives the canvas height in pixels.
     */
    void framebuffer_size(int& width, int& height) const override;

    /**
     * \brief Query the logical canvas size; for OffScreen it equals the pixel size.
     * \param width Receives the canvas width.
     * \param height Receives the canvas height.
     */
    void viewer_size(int& width, int& height) const override;

    /// Offscreen canvas uses a fixed pixel scale of one.
    /// \return Always 1.
    float dpi_scaling() const override { return 1; } // fixed-size pixel canvas

    /// Framebuffer width in pixels; equal to width().
    /// \return Canvas width.
    int framebuffer_width() const override { return width(); }

    /// Framebuffer height in pixels; equal to height().
    /// \return Canvas height.
    int framebuffer_height() const override { return height(); }

    /**
     * \brief Load and add a model file.
     * \param file Path to the model file.
     * \param create Whether to create standard rendering drawables.
     * \return The added model, or nullptr if loading or addition fails.
     */
    Model* add_model(const std::string& file, bool create = true) override;

    /**
     * \brief Add a shared model.
     * \param model Model to add.
     * \param create Whether to create standard rendering drawables.
     * \return The added model, or nullptr if addition fails.
     * \details The OffScreen host retains shared ownership.
     */
    Model* add_model(std::shared_ptr<Model> model, bool create = true) override;

    /**
     * \brief Add a shared standalone drawable.
     * \param drawable Drawable to add.
     * \return The added drawable, or nullptr if addition fails.
     * \details The OffScreen host retains shared ownership.
     */
    Drawable* add_drawable(std::shared_ptr<Drawable> drawable) override;

    /// \name Exposed scene and rendering operations
    /// These inherited Viewer operations are public on OffScreen.
    //@{
    using Viewer::camera;
    using Viewer::width;
    using Viewer::height;
    using Viewer::set_background_color;
    using Viewer::background_color;
    using Viewer::add_model;
    using Viewer::delete_model;
    using Viewer::models;
    using Viewer::current_model;
    using Viewer::add_drawable;
    using Viewer::delete_drawable;
    using Viewer::drawables;
    using Viewer::clear_scene;
    using Viewer::fit_screen;
    using Viewer::PickResult;
    using Viewer::pick;
    using Viewer::select_points;
    using Viewer::point_resource;
    using Viewer::unproject_pixel;
    using Viewer::project_point;
    using Viewer::unproject_point;
    using Viewer::point_under_pixel;
    using Viewer::set_overlays;
    using Viewer::clipping_planes;
    using Viewer::capture;
    //@}
#ifdef EASY3D_HAS_VULKAN
    // Transitional native scene operations, retained until the picker migration.
    using Viewer::scan_visible_points;
    using Viewer::device;
    using Viewer::set_clipping_planes;
#endif
private:
#ifdef EASY3D_HAS_VULKAN
    // Keep this part of code minimum
#else
    // Keep this part of code minimum
    void activate() const;
#endif
};
}
#endif
