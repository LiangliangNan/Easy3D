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

#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    Texture::Texture()
            : id_(0), name_(""), wrap_mode_(CLAMP_TO_EDGE), filter_mode_(LINEAR) {
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


    Texture *Texture::create(const std::string &file_name, WrapMode wrap_mode, FilterMode filter_mode) {
        if (!file_system::is_file(file_name)) {
            LOG(ERROR) << "file does not exist: " << file_name;
            return nullptr;
        }

        int width = 0, height = 0, comp = 0;
        std::vector<unsigned char> data;
        // flip the image vertically, so the first pixel in the output array is the bottom left
        bool success = ImageIO::load(file_name, data, width, height, comp, 0, true);
        if (!success || data.empty())
            return nullptr;

        Texture *texture = create(data, width, height, comp, wrap_mode, filter_mode);
        if (texture) {
            texture->name_ = file_name;
            LOG(INFO) << "a texture (id " << texture->id() << ") generated from image \'" << file_system::simple_name(file_name) << "\'";
        }

        return texture;
    }


    Texture *
    Texture::create(const std::vector<unsigned char> &rgb_data, int width, int height, int comp, WrapMode wrap_mode,
                    FilterMode filter_mode) {
        if (rgb_data.empty()) {
            LOG(ERROR) << "empty image data provided";
            return nullptr;
        } else if (rgb_data.size() != width * height * comp) {
            LOG(ERROR) << "image data does not match the given size";
            return nullptr;
        }

        GLuint tex = 0;
        glGenTextures(1, &tex); easy3d_debug_log_gl_error
        if (!tex) {
            LOG(ERROR) << "failed to generate the requested texture";
            return nullptr;
        }

        GLenum wrap, filter;
        switch (wrap_mode) {
            case CLAMP_TO_EDGE:
                wrap = GL_CLAMP_TO_EDGE;
                break;
            case REPEAT:
                wrap = GL_REPEAT;
                break;
            default: {
                LOG(ERROR) << "invalid wrap mode";
                return nullptr;
            }
        }
        switch (filter_mode) {
            case LINEAR:
                filter = GL_LINEAR;
                break;
            case NEAREST:
                filter = GL_NEAREST;
                break;
            default: {
                LOG(ERROR) << "invalid filter mode";
                return nullptr;
            }
        }

        GLenum internal_format, format;
        switch (comp) {
            case 4:
                internal_format = GL_RGBA8;
                format = GL_RGBA;
                break;
            case 3:
                internal_format = GL_RGB8;
                format = GL_RGB;
                break;
            case 2:
                internal_format = GL_RG8;
                format = GL_RG;
                break;
            case 1:
                internal_format = GL_R8;
                format = GL_RED;
                break;
            default: {
                LOG(ERROR) << "invalid format";
                return nullptr;
            }
        }

        glBindTexture(GL_TEXTURE_2D, tex);
        easy3d_debug_log_gl_error

        // To be robust to handle
        // - R, RG or RGB textures which are not 4-bytes floats, or the width is not divisible by 4.
        // - image pixels are tightly packed.
        // Liangliang: I need to restore the modified OpenGL state
        int align;
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &align);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<int>(filter));
        easy3d_debug_log_gl_error
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<int>(filter));
        easy3d_debug_log_gl_error
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<int>(wrap));
        easy3d_debug_log_gl_error
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<int>(wrap));
        easy3d_debug_log_gl_error

        glTexImage2D(GL_TEXTURE_2D, 0, static_cast<int>(internal_format), width, height, 0, format, GL_UNSIGNED_BYTE, rgb_data.data());
        easy3d_debug_log_gl_error
        glBindTexture(GL_TEXTURE_2D, 0);
        easy3d_debug_log_gl_error

        glPixelStorei(GL_UNPACK_ALIGNMENT, align);

        auto texture = new Texture;
        texture->wrap_mode_ = wrap_mode;
        texture->filter_mode_ = filter_mode;

        texture->sizes_[0] = width;
        texture->sizes_[1] = height;
        texture->sizes_[2] = comp;
        texture->id_ = tex;

        return texture;
    }


    void Texture::bind(int unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, id_);
    }


    void Texture::release() const {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }


    void discretize_image(std::vector<unsigned char> &data, int width, int height, int channels, int num_stripes) {
        if (num_stripes >= width)
            return;

        const int stride = width / num_stripes;
        for (int i = 0; i < num_stripes; ++i) {
            for (int j = 0; j < height; ++j) {
                for (int m = 0; m < stride; ++m)
                    for (int k = 0; k < channels; ++k)
                        data[j * (width * channels) + i * (stride * channels) + m * channels + k] =
                                data[j * (width * channels) + i * stride * channels + k];
            }
        }
    }


} // namespace easy3d
