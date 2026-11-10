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

#ifndef EASY3D_GUI_PICKER_H
#define EASY3D_GUI_PICKER_H


#include <easy3d/core/types.h>
#include <memory>
#include <easy3d/renderer/camera.h>


namespace easy3d {

    class FramebufferObject;
    class Drawable;
    namespace graphics { class RenderDevice; struct DrawCommand; }
    namespace vulkan { class DrawableRenderer; }

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
     *      Each technique has one implementation per backend:
     *
     *      - OpenGL renders into a private FramebufferObject with selection
     *        shaders, reads the pixel with glReadPixels, and evaluates regions in a
     *        vertex shader with transform feedback (each invocation owns one 32-bit
     *        selection word, avoiding atomics on OpenGL 3.2).
     *      - Vulkan delegates to the backend-neutral graphics::RenderDevice, which
     *        re-submits the draw command and samples the ID/depth pixel, and runs
     *        region selection in a compute shader.
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
     *      camera's screenWidth()/screenHeight() define the logical extent; on HiDPI
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
        Picker(const Picker&) = delete;
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
         *       the logical extent is camera()->screenWidth()/Height(), which is smaller than the
         *       framebuffer pixel dimensions by the DPI scaling factor.
         */
        Line3 picking_line(int x, int y) const {
            const vec3 p_near = camera()->unprojectedCoordinatesOf(vec3(float(x), float(y), 0));
            const vec3 p_far = camera()->unprojectedCoordinatesOf(vec3(float(x), float(y), 1));
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
         *       primarily used internally by OpenGL backend pickers to convert logical window
         *       coordinates to framebuffer pixels before calling glReadPixels(). Vulkan pickers sample the
         *       logical coordinate directly through RenderDevice::sample_pixel().
         * \throws std::invalid_argument if viewport or camera screen dimensions are not positive.
         * \throws std::out_of_range if the converted coordinate exceeds the integer range.
         */
        void screen_to_opengl(int x, int y, int &gl_x, int &gl_y, int width, int height) const;

    protected:
#ifdef EASY3D_HAS_VULKAN
        graphics::RenderDevice& render_device();
        graphics::DrawCommand draw_command(const std::shared_ptr<Drawable>& drawable, unsigned id);
#else
        /**
         * \brief Prepare a frame buffer for the offscreen picking pass.
         * \param width The width of the frame buffer.
         * \param height The height of the frame buffer.
         */
        void setup_framebuffer(int width, int height);
#endif

    protected:
        const Camera *camera_;

#ifdef EASY3D_HAS_VULKAN
    private:
        std::unique_ptr<graphics::RenderDevice> device_;
        std::unique_ptr<vulkan::DrawableRenderer> drawable_renderer_;
#else
        // A picker owns its framebuffer; GL framebuffer names are context-local.
        std::unique_ptr<FramebufferObject> fbo_;
#endif
    };

}


#endif  // EASY3D_GUI_PICKER_H
