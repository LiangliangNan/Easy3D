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

#ifndef EASY3D_RENDERER_DRAWABLE_LINES_2D_H
#define EASY3D_RENDERER_DRAWABLE_LINES_2D_H

#include <easy3d/renderer/drawable.h>


namespace easy3d {

    /**
     * \brief The drawable for rendering a set of line segments in the screen space.
     * \details This drawable is used to render a set of line segments in the screen space, e.g., the pivot mark.
     *      Imposter, texturing, per-element color, and scalar attributes are not implemented given
     *      the limited usage cases.
     * \class LinesDrawable2D easy3d/renderer/drawable_lines_2D.h
     * \sa Drawable, LinesDrawable
     * \sa LinesDrawable the 3D version of lines drawables.
     */
	class LinesDrawable2D : public Drawable {
	public:
		/**
		 * \brief Constructor that initializes the drawable with a name.
		 * \param name The name of the drawable.
		 */
        explicit LinesDrawable2D(const std::string& name = "");
        /**
         * \brief Destructor.
         */
        ~LinesDrawable2D() override;

        /**
         * \brief Returns the type of the drawable.
         * \return The type of the drawable, which is DT_LINES.
         */
        Type type() const override { return DT_LINES; }

        /**
         * \brief Draws the drawable.
         * \param camera The camera used for rendering.
         */
        void draw(const Camera* camera) const override;

        /// \name Screen-space line width
        ///@{

        /**
         * \brief Get the line width in framebuffer pixels.
         * \return Current line width (default 1). Must be positive and finite.
         * \details The width is relative to the actual framebuffer, not to the viewport passed to
         *      update_vertex_buffer(). For a DPI-independent overlay pass dpi_scaling() here and
         *      framebuffer dimensions there; a width of one is a device-pixel hairline.
         */
        float line_width() const { return line_width_; }

        /**
         * \brief Set the framebuffer-pixel line width.
         * \param width Positive finite width in pixels.
         * \details See line_width() for how this relates to the update_vertex_buffer() viewport.
         */
        void set_line_width(float width);
        ///@}

        /// \name Buffer access and management
        ///@{

        /**
         * \brief Creates/Updates the vertex buffer.
         * \param vertices The vertices of the line segments, represented in screen coordinates.
         * \param width The width of the screen/viewport, in the same pixels as `vertices`.
         * \param height The height of the screen/viewport, in the same pixels as `vertices`.
         * \param dynamic If true, the buffer is dynamic and can be updated.
         * \details Coordinates are normalized using the supplied viewport. Resizing scales their
         *      relative placement, matching the OpenGL implementation. Vulkan renders variable-width
         *      triangle ribbons as camera-independent overlays, without scene depth or picking IDs.
         *      Attach to a native host before uploading so CPU buffers are selected in dual builds.
         *      Pass framebuffer dimensions together with framebuffer-pixel coordinates (and a
         *      framebuffer-pixel line width, see line_width()) to draw a DPI-independent overlay;
         *      logical coordinates with a DPI-scaled width would mix two pixel spaces.
         * \attention In the current implementation, each pair of consecutive vertices represent a line. Thus for
         *      polylines or polygons, the shared vertices has to be duplicated in the vertex buffer. This is not
         *      optimal in terms of memory usage, but it is easier to implement and use.
         *      For a more memory efficient implementation, an element buffer should be used.
         */
        void update_vertex_buffer(const std::vector<vec2> &vertices, int width, int height, bool dynamic = false);
        ///@}

    private:
#ifndef EASY3D_HAS_VULKAN
        // Uploads the already normalized device coordinates into the position
        // buffer bound to the shared vertex array object and refreshes num_vertices_.
        void opengl_update_vertex_buffer(const std::vector<vec2>& ndc, bool dynamic);

        // Deletes the texel buffers and the extra vertex array object used by the
        // variable-width path, and zeroes the cached OpenGL names.
        void opengl_release_line_resources();

        // OpenGL names backing the variable-width line path: position and index
        // texel buffers plus the extra vertex array object that binds them.
        mutable unsigned int position_texture_ = 0, index_texture_ = 0, line_vao_ = 0;
#endif

        // Framebuffer-pixel line width (see line_width()).
        float line_width_ = 1.0f;
    };

}


#endif  // EASY3D_RENDERER_DRAWABLE_LINES_2D_H
