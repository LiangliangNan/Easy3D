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

#include <easy3d/viewer/texture.h>
#include <easy3d/viewer/opengl_error.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include <iostream>


namespace easy3d {

    Texture::Texture()
        : id_(0)
    {
        sizes_[0] = 0;
        sizes_[1] = 0;
        sizes_[2] = 0;
    }


    Texture::~Texture() {
        if (id_ != 0) {
            glDeleteTextures(1, &id_);
            id_ = 0;
        }
    }


    Texture* Texture::create(const std::string& file_name, GLenum wrap /* = GL_CLAMP_TO_EDGE*/, GLenum filter /* = GL_NEAREST*/) {
        if (!file_system::is_file(file_name)) {
            LOG(ERROR) << "file does not exist: " << file_name;
            return nullptr;
        }

        int width, height, comp;
        std::vector<unsigned char> data;
        // flip the image vertically, so the first pixel in the output array is the bottom left
        bool success = ImageIO::load(file_name, data, width, height, comp, 0, true);
        if (!success || data.empty())
            return nullptr;

        GLenum internal_format, format;
        switch (comp)
        {
        case 4: internal_format = GL_RGBA8; format = GL_RGBA; break;
        case 3: internal_format = GL_RGB8; format = GL_RGB; break;
        case 2: internal_format = GL_RG8; format = GL_RG; break;
        case 1: internal_format = GL_R8; format = GL_RED; break;
        default: throw std::runtime_error("invalid format");
        }

        GLuint tex = 0;
        glGenTextures(1, &tex);	easy3d_debug_log_gl_error;
        glBindTexture(GL_TEXTURE_2D, tex);	easy3d_debug_log_gl_error;

        // To be robust to handle
        // - R, RG or RGB textures which are not 4-bytes floats, or the width is not divisible by 4.
        // - image pixels are tightly packed.
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);	easy3d_debug_log_gl_error;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);	easy3d_debug_log_gl_error;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);	easy3d_debug_log_gl_error;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);	easy3d_debug_log_gl_error;

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data.data());	easy3d_debug_log_gl_error;
        glBindTexture(GL_TEXTURE_2D, 0);

        Texture* result = new Texture;
        result->sizes_[0] = width;
        result->sizes_[1] = height;
        result->sizes_[2] = comp;
        result->id_ = tex;
        result->file_name_ = file_name;

        return result;
    }


    void Texture::bind(int unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, id_);
    }


    void Texture::release() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

} // namespace easy3d

