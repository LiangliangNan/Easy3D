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
     *
     * \class Texture easy3d/renderer/texture.h
     *
     * \details currently only TEXTURE_2D is supported
     */
    class Texture {
    public:
        enum WrapMode {
            CLAMP_TO_EDGE, REPEAT
        };
        enum FilterMode {
            NEAREST, LINEAR
        };

        /**
         * Creates a texture from an image file.
         * @image_file The full path to the image file.
         * @return The created texture.
         */
        static Texture *
        create(const std::string &image_file, WrapMode wrap = CLAMP_TO_EDGE, FilterMode filter = LINEAR);

        /**
         * Creates a texture from the given image data.
         * @param rgb_data The image data.
         * @param width The width (i.e., number of columns) of the image.
         * @param height The rows (i.e., number of rows) of the image.
         * @param comp The number of components for each pixel (e.g., 3 for RGB)
         * @return The created texture.
         */
        static Texture *create(const std::vector<unsigned char> &rgb_data, int width, int height, int comp,
                               WrapMode wrap = CLAMP_TO_EDGE, FilterMode filter = LINEAR);

        ~Texture();

        unsigned int id() const { return id_; }

        const std::string &name() const { return name_; }

        void bind(int unit = 0) const;

        void release() const;

        int width() const { return sizes_[0]; }

        int height() const { return sizes_[1]; }

        int channels() const { return sizes_[2]; }

        WrapMode wrap_mode() const { return wrap_mode_; }

        FilterMode filter_mode() const { return filter_mode_; }

    private:
        unsigned int id_;
        int sizes_[3];

        std::string name_;
        WrapMode wrap_mode_;
        FilterMode filter_mode_;

    private:
        /** The creation of a texture is only allowed by using the create() function */
        Texture();

        //copying disabled
        Texture(const Texture &);

        Texture &operator=(const Texture &);

        friend class TextureManager;
    };


    /**
     * @brief Discretize a gradually varying-color (from left to right) image into a set of uniform colored vertical
     *        stripes.
     * @param data The input data to be discretized.
     * @param width The width of the image (i.e., number of pixels in a row).
     * @param height The height of the image (i.e., number of pixels in a column).
     * @param channels The number or color component per pixel.
     * @param num_stripes The number of stripes. The image will remain unmodified if num_stripes >= image width.
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

