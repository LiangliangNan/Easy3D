// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2015 Liangliang Nan
#include <easy3d/renderer/texture.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/util/logging.h>
#include <stdexcept>
namespace easy3d {
bool Texture::opengl_upload() {
    const auto& rgb_data = pixels_->bytes;
    const int width = pixels_->width, height = pixels_->height, comp = pixels_->channels;
    const auto wrap_mode = wrap_mode_;
    const auto filter_mode = filter_mode_;
        GLuint tex = 0;
        glGenTextures(1, &tex); easy3d_debug_log_gl_error
        if (!tex) {
            LOG(ERROR) << "failed to generate the requested texture";
            return false;
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
                return false;
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
                return false;
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
                return false;
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

        this->wrap_mode_ = wrap_mode;
        this->filter_mode_ = filter_mode;

        this->sizes_[0] = width;
        this->sizes_[1] = height;
        this->sizes_[2] = comp;
        this->id_ = tex;

    return true;
}
void Texture::opengl_destroy() {
    glDeleteTextures(1, &id_); id_ = 0;
}
void Texture::bind(int unit) const {
    if (!id_) throw std::logic_error("CPU texture has no OpenGL realization");
    glActiveTexture(GL_TEXTURE0+unit); glBindTexture(GL_TEXTURE_2D,id_);
}
void Texture::release() const {
    if (!id_) throw std::logic_error("CPU texture has no OpenGL realization");
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,0);
}
}
