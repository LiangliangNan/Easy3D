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
 ********************************************************************/
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
 * \see Viewer, ViewerBase, MultiViewer
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
        explicit OffScreen(int width = 800,
                           int height = 600,
#ifdef EASY3D_HAS_VULKAN
                           const VulkanOptions &options = {});
#else
                           const OpenGLOptions &options = {});
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
        bool render(const std::string &filename,
                    float scaling = 1,
                    int samples = 4,
                    int background = 0,
                    bool expand = true) const;

        /**
     * \brief Resize the offscreen canvas.
     * \param width New positive canvas width in pixels.
     * \param height New positive canvas height in pixels.
     * \throws std::invalid_argument if either dimension is not positive.
     */
        void resize(int width, int height) override;

        /**
     * \brief Query the logical canvas size; for OffScreen it equals the pixel size.
     * \param width Receives the canvas width.
     * \param height Receives the canvas height.
     */
        void viewer_size(int &width, int &height) const override;

        /**
     * \brief Query the framebuffer size; for OffScreen it equals the canvas size.
     * \param width Receives the canvas width in pixels.
     * \param height Receives the canvas height in pixels.
     */
        void framebuffer_size(int &width, int &height) const override;

        // Convenience single-value forms of framebuffer_size() above; the canvas
        // has no separate logical/DPI extent, so these return canvas pixels.
        using Viewer::framebuffer_width;
        using Viewer::framebuffer_height;

        /// Offscreen canvas uses a fixed pixel scale of one.
        /// \return Always 1.
        float dpi_scaling() const override {
            return 1;
        } // fixed-size pixel canvas

        /**
     * \brief Load and add a model file.
     * \param file Path to the model file.
     * \param create Whether to create standard rendering drawables.
     * \return The added model, or nullptr if loading or addition fails. The returned
     *         pointer is a non-owning observer; do not delete it.
     * \details The OffScreen host owns the loaded model (held internally by a
     *      shared_ptr). A model with the same file name already present is returned
     *      unchanged. The camera is fitted to the added model.
     */
        Model *add_model(const std::string &file, bool create = true) override;

        /**
     * \brief Add a shared model.
     * \param model Model to add.
     * \param create Whether to create standard rendering drawables.
     * \return The added model, or nullptr if the model is null or empty, or if
     *         addition fails. The returned pointer is a non-owning observer; do not
     *         delete it.
     * \details The OffScreen host retains shared ownership: it stores a copy of the
     *      shared_ptr, so the model survives until both you and the host release it.
     *      Do not delete it manually. Use this form when your code keeps using the
     *      model after adding it. As in Viewer, adding installs a *new* Renderer
     *      (replacing any existing renderer and drawables, so add custom drawables
     *      afterwards), fits the camera to the scene, and returns the existing entry
     *      unchanged if the same model is added again. The legacy raw-pointer form
     *      (exposed via Viewer::add_model) instead transfers ownership to the host;
     *      after passing a raw Model* you must not delete it or wrap it in another
     *      shared_ptr.
     */
        Model *add_model(std::shared_ptr<Model> model, bool create = true) override;

        /**
     * \brief Add a shared standalone drawable.
     * \param drawable Drawable to add.
     * \return The added drawable, or nullptr if the pointer is null. The returned
     *         pointer is a non-owning observer; do not delete it.
     * \details The OffScreen host retains shared ownership: it stores a copy of the
     *      shared_ptr, so the drawable stays alive until both you and the host release
     *      it. Use this form to keep the drawable alive independently in your own
     *      code. Unlike add_model, this creates no renderer and does not fit the
     *      camera; an already-managed drawable is returned unchanged. The legacy
     *      raw-pointer form (exposed via Viewer::add_drawable) transfers ownership to
     *      the host, so the caller must not delete it afterwards.
     */
        Drawable *add_drawable(std::shared_ptr<Drawable> drawable) override;

        /// \name Exposed scene and rendering operations
        /// These inherited Viewer operations are public on OffScreen.
        //@{
        using Viewer::add_drawable;
        using Viewer::add_model;
        using Viewer::background_color;
        using Viewer::camera;
        using Viewer::capture;
        using Viewer::clear_scene;
        using Viewer::clipping_planes;
        using Viewer::current_model;
        using Viewer::delete_drawable;
        using Viewer::delete_model;
        using Viewer::drawables;
        using Viewer::fit_screen;
        using Viewer::height;
        using Viewer::models;
        using Viewer::pick;
        using Viewer::PickResult;
        using Viewer::point_resource;
        using Viewer::point_under_pixel;
        using Viewer::pixel_to_world;
        using Viewer::select_points;
        using Viewer::set_background_color;
        using Viewer::set_overlays;
        using Viewer::window_to_world;
        using Viewer::width;
        using Viewer::world_to_pixel;
        using Viewer::world_to_window;
#ifdef EASY3D_HAS_VULKAN
        // Transitional native scene operations, retained until the picker migration.
        using Viewer::device;
        using Viewer::scan_visible_points;
        using Viewer::set_clipping_planes;
#endif
        //@}
    private:
        // Keep this part of code minimum
#ifndef EASY3D_HAS_VULKAN
        /**
         * \brief Make the hidden OpenGL context current for the calling thread.
         * \details Required before any GL resource use or capture; Vulkan builds
         *      manage the device explicitly and need no context activation.
         */
        void activate() const;
#endif
    };
} // namespace easy3d
#endif
