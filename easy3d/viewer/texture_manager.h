/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#ifndef EASY3D_OPENGL_TEXTURE_MANAGER_H
#define EASY3D_OPENGL_TEXTURE_MANAGER_H


#include <easy3d/viewer/texture.h>


#include <string>
#include <unordered_map>


namespace easy3d {

    class Texture;

    /**
     * A texture can be shared by multiple models, so it is wise to have a single instance of a texture.
     * TextureManager manages the texture creation and memories. It ensures no duplication.
     * NOTE: make sure to call terminate() to destroy existing textures before the OpenGL context is deleted.
     */
    class TextureManager
    {
    public:
        /**
         * Request a texture from the image file.
         * @param image_file The full path to the image file.
         * @return The texture created from the image.
         * TODO: allow to change the wrap/filter mode (i.e., multiple textures of different parameters from the same image).
         */
        static Texture* request(const std::string& image_file, Texture::WrapMode wrap = Texture::CLAMP_TO_EDGE, Texture::FilterMode filter = Texture::LINEAR);

        /**
         * Request a random color texture.
         * Internally, it first generates a 1D random color image (i.e, height = 1 and width = number_colors) and the
         * texture is created from the image data.
         * @param num_colors The number of colors in the texture.
         * @return The texture created.
         */
        static Texture* request(int num_colors, Texture::WrapMode wrap = Texture::CLAMP_TO_EDGE, Texture::FilterMode filter = Texture::LINEAR);

        /**
         * Release a texture (deallocate its memory).
         * @param texture The texture to be released.
         */
        static void release(const Texture* texture);

        // destroy all textures.
        static void terminate();

    private:
        // maps of std::string can be super slow when calling find with a string literal or const char*
        // as find forces construction/copy/destruction of a std::sting copy of the const char*.
        static std::unordered_map<std::string, Texture*>	textures_;
        static std::unordered_map<std::string, bool>		attempt_load_texture_; // avoid multiple attempt
    };

} // namespace easy3d


#endif  // EASY3D_OPENGL_TEXTURE_MANAGER_H

