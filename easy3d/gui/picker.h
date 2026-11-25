/** \cond LICENSE
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
 \endcond */

#ifndef EASY3D_GUI_PICKER_H
#define EASY3D_GUI_PICKER_H


#include <easy3d/core/types.h>
#include <memory>
#include <easy3d/renderer/camera.h>


namespace easy3d {

    class FramebufferObject;
    class Drawable;
    namespace graphics { class RenderDevice; class DrawableRenderer; struct DrawCommand; }

    /**
     * \brief Base class for the camera-based picking tools.
     * \class Picker easy3d/gui/picker.h
     * \details A picker answers "what is under the cursor?" with the same rendering
     *      pipeline that produced the visible image, instead of tracing rays
     *      against CPU geometry. Picking therefore follows what the user actually
     *      sees: drawable and model visibility, model transforms, clipping planes,
     *      point impostors, and indexed draws.
     *
     *      Two techniques are used, depending on the question:
     *
     *      - **Single-element picking (rasterization).** The drawable is rendered
     *        off screen once more, but instead of colors each triangle or point
     *        writes an identifier into the color target (a "primitive ID buffer").
     *        The depth test stays enabled, so the ID stored at a pixel belongs to
     *        the nearest visible primitive. Reading and decoding that single pixel
     *        yields the picked element; the CPU never traverses the geometry.
     *
     *      - **Region selection (parallel projection).** For a rectangle or lasso,
     *        every point or vertex is transformed to screen space independently.
     *        That work is embarrassingly parallel, so the graphics device evaluates
     *        all elements at once and only a compact result (a packed bitmask) is
     *        read back.
     *
     *      Both backends use the shared graphics::RenderDevice for supported
     *      single-element picks and point-region selection:
     *
     *      - OpenGL borrows the caller's current context, uploads backend-neutral
     *        draw commands, and samples an offscreen ID/depth target. Point-region
     *        selection currently projects and tests CPU-backed point snapshots.
     *      - Vulkan uses its own headless device to sample the ID/depth target and
     *        evaluate point-region selection in a compute shader.
     *
     *      Native fallbacks remain for OpenGL operations outside the shared adapter's
     *      current scope, such as analytic point impostors and clipped picking.
     *
     *      Pickers are scene-independent tools: they take a borrowed Camera and a
     *      caller-supplied model or drawable, and own their offscreen resources.
     *      They do not traverse a Viewer's scene. Viewer::pick() is the scene-wide
     *      counterpart: because the viewer owns the models, drawables and device, it
     *      resolves the nearest model/drawable/primitive in one backend submission
     *      and returns it with the depth and world position. A picker then refines a
     *      given model to a face, vertex or point.
     *      In principle a Viewer could instantiate these pickers (it has both a
     *      camera and the models), but a whole-scene pick must order candidates from
     *      every model in a single depth-aware submission; per-model picker queries
     *      would need an extra arbitration pass. Client applications such as Mapple
     *      use pickers directly because they keep their own scene; a caller that
     *      already has a Viewer should prefer Viewer::pick() and use a picker only
     *      when it needs the model-level element.
     *
     *      The camera is borrowed and must outlive the picker. OpenGL GPU queries
     *      and resource destruction require the owning context to be current.
     *
     *      **Coordinate systems.** All picker methods accept screen coordinates
     *      expressed in logical window units (top-left origin, x right, y down),
     *      matching ViewerBase::world_to_window() and mouse event callbacks. The
     *      camera's screen_width()/screen_height() define the logical extent; on HiDPI
     *      displays this is smaller than the framebuffer pixel dimensions by the
     *      DPI scaling factor. Internally, single-element picks convert the logical
     *      coordinate to a framebuffer pixel before sampling; region selections
     *      evaluate the polygon/rectangle directly in logical coordinates. Both
     *      backends present the same coordinate contract to callers, so client code
     *      is backend-independent.
     * \see ModelPicker, PointCloudPicker, SurfaceMeshPicker, Viewer::pick()
     */
    class Picker {
    public:
        /**
         * \brief Constructor.
         * \param cam Non-null camera used for picking; it is borrowed and must outlive the picker.
         */
        explicit Picker(const Camera *cam);
        /**
         * \brief Destructor.
         */
        virtual ~Picker();
        /// Pickers own backend query resources, which are context-local and cannot be shared; copying disabled.
        Picker(const Picker&) = delete;
        /// Assignment would duplicate the backend pick resources owned by the picker.
        Picker& operator=(const Picker&) = delete;

        /**
         * \brief Returns the pointer of the camera.
         * \return The camera pointer.
         */
        const Camera *camera() const { return camera_; }

        /**
         * \brief Construct a picking line.
         * \param x The cursor x-coordinate in logical window units (top-left origin, x right).
         * \param y The cursor y-coordinate in logical window units (top-left origin, y down).
         * \return The picking line from the near plane through the cursor position into the scene.
         * \note The coordinate must lie within the camera screen extent and is in logical window units, not framebuffer pixels. On HiDPI displays
         *       the logical extent is camera()->screen_width()/Height(), which is smaller than the
         *       framebuffer pixel dimensions by the DPI scaling factor.
         */
        Line3 picking_line(int x, int y) const {
            const vec3 p_near = camera()->unprojected_coordinates_of(vec3(float(x), float(y), 0));
            const vec3 p_far = camera()->unprojected_coordinates_of(vec3(float(x), float(y), 1));
            return Line3::from_two_points(p_near, p_far);
        }

        /**
         * \brief Convert a point expressed in the screen coordinate system (with an origin in the upper left corner)
         *      into the OpenGL coordinate system (with an origin in the lower left corner). The high DPI scaling is
         *      also taken into consideration, so the result always corresponds to its image pixel.
         * \param x The x-coordinate in logical window units (top-left origin, x right).
         * \param y The y-coordinate in logical window units (top-left origin, y down).
         * \param gl_x Returns the x component of the point in OpenGL framebuffer coordinates (bottom-left origin).
         * \param gl_y Returns the y component of the point in OpenGL framebuffer coordinates (bottom-left origin).
         * \param width The width of the OpenGL viewport in framebuffer pixels.
         * \param height The height of the OpenGL viewport in framebuffer pixels.
         * \note The logical coordinate and framebuffer dimensions must describe the same viewport. This method is
         *       primarily used internally by native OpenGL fallbacks to convert logical
         *       window coordinates to bottom-left framebuffer pixels before glReadPixels().
         *       Shared RenderDevice queries accept top-left framebuffer indices directly.
         * \throws std::invalid_argument if viewport or camera screen dimensions are not positive.
         * \throws std::out_of_range if the converted coordinate exceeds the integer range.
         */
        void screen_to_opengl(int x, int y, int &gl_x, int &gl_y, int width, int height) const;

    protected:
        /**
         * \brief Return the private render device, creating it on first use.
         * \details Prunes stale cached drawables so a pick never submits freed resources.
         */
        graphics::RenderDevice& render_device();
        /**
         * \brief Build a backend-neutral pick command for a drawable.
         */
        graphics::DrawCommand draw_command(const std::shared_ptr<Drawable>& drawable, unsigned id);
#ifndef EASY3D_HAS_VULKAN
        /**
         * \brief Prepare a frame buffer for legacy OpenGL picking fallbacks.
         */
        void setup_framebuffer(int width, int height);
#endif

        // Borrowed camera; must outlive the picker (see constructor).
        const Camera *camera_;
#ifndef EASY3D_HAS_VULKAN
        // A picker owns its fallback framebuffer; GL framebuffer names are context-local.
        std::unique_ptr<FramebufferObject> fbo_;
#endif

    private:
        // Headless device used only for query passes; it owns no window or swapchain.
        std::unique_ptr<graphics::RenderDevice> device_;
        // Uploads drawable geometry and encodes primitive IDs into the pick command.
        std::unique_ptr<graphics::DrawableRenderer> drawable_renderer_;
    };

}


#endif  // EASY3D_GUI_PICKER_H
