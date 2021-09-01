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

#ifndef EASY3D_RENDERER_TEXTURE_MANAGER_H
#define EASY3D_RENDERER_TEXTURE_MANAGER_H


#include <string>
#include <unordered_map>

#include <easy3d/renderer/texture.h>


namespace easy3d {

    class Texture;

    /**
     * \brief Management of OpenGL textures.
     *
     * \class TextureManager easy3d/renderer/texture_manager.h
     *
     * \details A texture can be shared by multiple models, so it is wise to have a single instance of a texture.
     * TextureManager manages the texture creation and memories. It ensures no duplication.
     *
     * \note Make sure to call terminate() to destroy existing textures before the OpenGL context is deleted.
     *
     * \todo Allow to change the wrap/filter mode (i.e., multiple textures of different parameters from the same image).
     */
    class TextureManager {
    public:
        /**
         * @brief Request a texture from the image file.
         * @details The generated texture has a unique identifier of its full-path file name. If a texture with the same
         *          identifier is requested again, no new texture will be generated and the existing texture is returned.
         * @param image_file The full path to the image file.
         * @param wrap The wrap mode.
         * @param filter The filter mode.
         * @return The texture created from the image.
         */
        static Texture *request(const std::string &image_file,
                                Texture::WrapMode wrap = Texture::CLAMP_TO_EDGE,
                                Texture::FilterMode filter = Texture::LINEAR);

        /**
         * @brief Request a texture for rendering scalar fields from a gradually varying (from left to right) color
         *        image given its file name.
         * @details This method allows to sample the original image into a specified number of vertical stripes. The
         *          generated texture has a unique identifier in the format: "file_name|num_stripes". If a texture with
         *          the same identifier is requested again, no new texture will be generated and the existing one is
         *          returned.
         * @param image_file The full path to the image file.
         * @param num_stripes The number of stripes. If num_stripes == image_width, no sampling will occur.
         * @param wrap The wrap mode.
         * @param filter The filter mode.
         * @return The texture created from the image.
         */
        static Texture *request(const std::string &image_file, int num_stripes,
                                Texture::WrapMode wrap = Texture::CLAMP_TO_EDGE,
                                Texture::FilterMode filter = Texture::LINEAR);

        /**
         * @brief Request a random color texture for rendering scalar fields. The texture will have a set of uniformly
         *        colored vertical stripes.
         * @details It first generates a random-colored image, where each row has "num_colors" color and each color
         *          repeat "stride" times. Then, a texture is created from this image data. The generated texture has a
         *          unique identifier in the format: "random|num_stripes|stride", where num_stripes and stride are
         *          integer numbers. If the requested number of stripes and the stride match a previously generated
         *          texture, no new texture will be generated and the existing texture is returned.
         * @param num_stripes The number of stripes in the texture.
         * @param stride The width of each vertical strip. So the total width of the texture is: num_stripes * stride.
         * @return The texture created.
         */
        static Texture *request(int num_stripes, int stride,
                                Texture::WrapMode wrap = Texture::CLAMP_TO_EDGE,
                                Texture::FilterMode filter = Texture::LINEAR);

        /**
         * Release a texture (deallocate its memory).
         * @param texture The texture to be released.
         */
        static void release(const Texture *texture);

        /// destroy all textures.
        static void terminate();

    private:
        // maps of std::string can be super slow when calling find with a string literal or const char*
        // as find forces construction/copy/destruction of a std::sting copy of the const char*.
        static std::unordered_map<std::string, Texture *> textures_;
        static std::unordered_map<std::string, bool> attempt_load_texture_; // avoid multiple attempt
    };


} // namespace easy3d


#endif  // EASY3D_RENDERER_TEXTURE_MANAGER_H

