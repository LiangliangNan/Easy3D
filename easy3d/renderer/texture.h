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

#ifndef EASY3D_RENDERER_OPENGL_TEXTURE_H
#define EASY3D_RENDERER_OPENGL_TEXTURE_H


#include <string>
#include <vector>


namespace easy3d {

    /**
     * \brief OpenGL texture.
     * \class Texture easy3d/renderer/texture.h
     * \details currently only TEXTURE_2D is supported
     */
    class Texture {
    public:
        /// The wrap mode of the texture.
        enum WrapMode {
            CLAMP_TO_EDGE,  ///< Clamp the texture coordinate to the range [0, 1]
            REPEAT          ///< Repeat the texture coordinate
        };
        /// The filter mode of the texture.
        enum FilterMode {
            NEAREST,  ///< Nearest neighbor interpolation
            LINEAR    ///< Linear interpolation
        };

        /**
         * \brief Creates a texture from an image file.
         * \param image_file The full path to the image file.
         * \param wrap The wrap mode of the texture.
         * \param filter The filter mode of the texture.
         * \return The created texture.
         */
        static Texture* create(const std::string &image_file, WrapMode wrap = CLAMP_TO_EDGE, FilterMode filter = LINEAR);

        /**
         * \brief Creates a texture from the given image data.
         * \param rgb_data The image data.
         * \param width The width (i.e., number of columns) of the image.
         * \param height The height (i.e., number of rows) of the image.
         * \param comp The number of components for each pixel (e.g., 3 for RGB).
         * \param wrap The wrap mode of the texture.
         * \param filter The filter mode of the texture.
         * \return The created texture.
         */
        static Texture *create(const std::vector<unsigned char> &rgb_data, int width, int height, int comp,
                               WrapMode wrap = CLAMP_TO_EDGE, FilterMode filter = LINEAR);

        /**
         * \brief Destructor.
         */
        ~Texture();

        /**
         * \brief Get the OpenGL texture ID.
         * \return The texture ID.
         */
        unsigned int id() const { return id_; }
        /**
         * \brief Get the name of the texture.
         * \return The name of the texture.
         */
        const std::string &name() const { return name_; }
        /**
         * \brief Bind the texture to a texture unit.
         * \param unit The texture unit to bind to.
         */
        void bind(int unit = 0) const;
        /**
         * \brief Release the texture.
         */
        void release() const;
        /**
         * \brief Get the width of the texture.
         * \return The width of the texture.
         */
        int width() const { return sizes_[0]; }
        /**
         * \brief Get the height of the texture.
         * \return The height of the texture.
         */
        int height() const { return sizes_[1]; }
        /**
         * \brief Get the number of channels of the texture.
         * \return The number of channels of the texture.
         */
        int channels() const { return sizes_[2]; }
        /**
         * \brief Get the wrap mode of the texture.
         * \return The wrap mode of the texture.
         */
        WrapMode wrap_mode() const { return wrap_mode_; }
        /**
         * \brief Get the filter mode of the texture.
         * \return The filter mode of the texture.
         */
        FilterMode filter_mode() const { return filter_mode_; }

    private:
        unsigned int id_;
        int sizes_[3];

        std::string name_;
        WrapMode wrap_mode_;
        FilterMode filter_mode_;

    private:
        /**
         * \brief Private constructor to enforce the use of the create() function.
         */
        Texture();

        //copying disabled
        Texture(const Texture &);

        Texture &operator=(const Texture &);

        friend class TextureManager;
    };


    /**
     * \brief Discretize a gradually varying-color (from left to right) image into a set of uniform colored vertical
     *      stripes.
     * \param data The input data to be discrete.
     * \param width The width of the image (i.e., number of pixels in a row).
     * \param height The height of the image (i.e., number of pixels in a column).
     * \param channels The number of color components per pixel.
     * \param num_stripes The number of stripes. The image will remain unmodified if num_stripes >= image width.
     */
    void discretize_image(
            std::vector<unsigned char> &data,
            int width,
            int height,
            int channels,
            int num_stripes
    );

} // namespace easy3d


#endif  // EASY3D_RENDERER_OPENGL_TEXTURE_H

