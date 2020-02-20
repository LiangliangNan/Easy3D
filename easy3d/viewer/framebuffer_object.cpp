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

#include <easy3d/viewer/framebuffer_object.h>

#include <algorithm>
#include <cstring>

#include <easy3d/viewer/opengl_error.h>
#include <easy3d/viewer/opengl_info.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    bool FramebufferObject::is_supported() {
        return OpenglInfo::is_supported("GL_VERSION_3_2") ||
                OpenglInfo::is_supported("GL_ARB_framebuffer_object") ||
                OpenglInfo::has_entension("GL_EXT_framebuffer_object");
    }


    FramebufferObject::FramebufferObject(int w, int h, int samples /* = 0 */)
    {
        if (!is_supported()) {
            LOG_FIRST_N(WARNING, 1) << "frame buffer object not supported on this platform";
            return;
        }

        init(w, h, samples);
    }


    void FramebufferObject::init(int w, int h, int samples)
    {
        fbo_id_ = 0;
        prev_draw_fbo_ = 0;
        prev_read_fbo_ = 0;

        width_ = w;
        height_ = h;
        samples_ = 0;		// actual value will be decided later
        resolved_fbo_ = nullptr;
        depth_buffer_ = 0;
        depth_texture_ = 0;
        depth_internal_format_ = GL_DEPTH24_STENCIL8;
        depth_texture_filter_ = GL_NEAREST;
        depth_texture_compare_mode_ = GL_NONE;
        depth_texture_compare_func_ = GL_LEQUAL;
        texture_target_ = GL_TEXTURE_2D;

        valid_ = is_supported();
        if (!valid_)
            return;

        if (samples > 0) {// multismaple requested
            // using the core prifile, multisampling is for sure supported.
			if (samples > 0) {
				GLint max_samples;
				glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
				samples_ = std::max(0, std::min(samples, max_samples));

                // warn the user if the expected request was not satisfied
                if (samples_ != samples)
                    LOG_IF(WARNING, samples != samples_) << "MSAA is available with "
                                                         << samples_ << " samples (" << samples
                                                         << " requested, max support is " << max_samples << ")";
			}
        }

        glGenFramebuffers(1, &fbo_id_);		easy3d_debug_log_gl_error; easy3d_debug_log_frame_buffer_error;
    }


    FramebufferObject::~FramebufferObject()
    {
        clear();
    }


    void FramebufferObject::clear() {
        if (is_bound())
            release();

        for (std::size_t i = 0; i < color_attachments_.size(); ++i) {
            GLuint color_texture = color_attachments_[i].texture;
            if (glIsTexture(color_texture))
                glDeleteTextures(1, &color_texture);		easy3d_debug_log_gl_error;

            GLuint color_buffer = color_attachments_[i].buffer;
            if (color_buffer)
                glDeleteRenderbuffers(1, &color_buffer);	easy3d_debug_log_gl_error;
        }
        color_attachments_.clear();

        if (glIsTexture(depth_texture_)) {
            glDeleteTextures(1, &depth_texture_);		easy3d_debug_log_gl_error;
            depth_texture_ = 0;
        }

        if (depth_buffer_) {
            glDeleteRenderbuffers(1, &depth_buffer_);	easy3d_debug_log_gl_error;
            depth_buffer_ = 0;
        }

        if (fbo_id_) {
            glDeleteFramebuffers(1, &fbo_id_);			easy3d_debug_log_gl_error;
            fbo_id_ = 0;
        }

        if (resolved_fbo_) {
            delete resolved_fbo_;						easy3d_debug_log_gl_error;
            resolved_fbo_ = nullptr;
        }
    }


    void FramebufferObject::ensure_size(int w, int h) {
        if (w == width() && h == height())
            return;

        // backup before clearing
        std::vector<ColorAttachment> color_attachments = color_attachments_;
        bool depth_as_texture = (depth_texture_ != 0);
        bool need_depth = (depth_buffer_ != 0) || depth_as_texture;

        //////////////////////////////////////////////////////////////////////////

        clear();

        init(w, h, samples_);

        for (std::size_t i = 0; i < color_attachments.size(); ++i) {
            const ColorAttachment& attachment = color_attachments[i];
            bool color_as_texture = (attachment.texture != 0);
            if (color_as_texture)
                add_color_texture(attachment.internal_format, attachment.format, attachment.type, attachment.texture_filter);
            else
                add_color_buffer(attachment.internal_format, attachment.format, attachment.type);
        }

        if (need_depth) {
            if (depth_as_texture)
                add_depth_texture(depth_internal_format_, depth_texture_filter_, depth_texture_compare_mode_, depth_texture_compare_func_);
            else
                add_depth_buffer(depth_internal_format_);
        }

        valid_ = check_status();
        if (!valid_) {
            glDeleteFramebuffers(1, &fbo_id_);		easy3d_debug_log_gl_error;
        }
    }


    static bool color_format_compatible(GLenum internal_format, GLenum format, GLenum type)
    {
        if (format != GL_RED && format != GL_RG && format != GL_RGB && format != GL_BGR && format != GL_RGBA && format != GL_BGRA) {
            LOG(ERROR) << "the provided format is not accepted";
            return false;
        }

        //////////////////////////////////////////////////////////////////////////

        if (type != GL_UNSIGNED_BYTE && type != GL_BYTE && type != GL_UNSIGNED_SHORT && type != GL_SHORT && type != GL_UNSIGNED_INT && type != GL_INT &&
            type != GL_FLOAT && type != GL_UNSIGNED_BYTE_3_3_2 && type != GL_UNSIGNED_BYTE_2_3_3_REV && type != GL_UNSIGNED_SHORT_5_6_5 &&
            type != GL_UNSIGNED_SHORT_5_6_5_REV && type != GL_UNSIGNED_SHORT_4_4_4_4 && type != GL_UNSIGNED_SHORT_4_4_4_4_REV &&
            type != GL_UNSIGNED_SHORT_5_5_5_1 && type != GL_UNSIGNED_SHORT_1_5_5_5_REV && type != GL_UNSIGNED_INT_8_8_8_8 &&
            type != GL_UNSIGNED_INT_8_8_8_8_REV && type != GL_UNSIGNED_INT_10_10_10_2 && type != GL_UNSIGNED_INT_2_10_10_10_REV)
        {
            LOG(ERROR) << "the provided type is not accepted";
            return false;
        }

        //////////////////////////////////////////////////////////////////////////

        if (internal_format != GL_R8 && internal_format != GL_R8I && internal_format != GL_R8UI &&
            internal_format != GL_R16 && internal_format != GL_R16I && internal_format != GL_R16UI && internal_format != GL_R16F &&
            internal_format != GL_R32I && internal_format != GL_R32UI && internal_format != GL_R32F &&
            internal_format != GL_RG8 && internal_format != GL_RG8I && internal_format != GL_RG8UI &&
            internal_format != GL_RG16 && internal_format != GL_RG16I && internal_format != GL_RG16UI && internal_format != GL_RG16F &&
            internal_format != GL_RG32I && internal_format != GL_RG32UI && internal_format != GL_RG32F &&
            internal_format != GL_RGB8 && internal_format != GL_RGB8I && internal_format != GL_RGB8UI &&
            internal_format != GL_RGB16 && internal_format != GL_RGB16I && internal_format != GL_RGB16UI && internal_format != GL_RGB16F &&
            internal_format != GL_RGB32I && internal_format != GL_RGB32UI && internal_format != GL_RGB32F &&
            internal_format != GL_RGBA8 && internal_format != GL_RGBA8I && internal_format != GL_RGBA8UI &&
            internal_format != GL_RGBA16 && internal_format != GL_RGBA16I && internal_format != GL_RGBA16UI && internal_format != GL_RGBA16F &&
            internal_format != GL_RGBA32I && internal_format != GL_RGBA32UI && internal_format != GL_RGBA32F
            )
        {
            LOG(ERROR) << "internal format must be in GL_[components][size][type] format";
            return false;
        }

        //////////////////////////////////////////////////////////////////////////

        if (internal_format == GL_R8 || internal_format == GL_R8I || internal_format == GL_R8UI ||
            internal_format == GL_R16 || internal_format == GL_R16I || internal_format == GL_R16UI || internal_format == GL_R16F ||
            internal_format == GL_R32I || internal_format == GL_R32UI || internal_format == GL_R32F
            )
        {
            if (format != GL_RED) {
                LOG(ERROR) << "color format must be GL_R";
                return false;
            }
        }
        else if (internal_format == GL_RG8 || internal_format == GL_RG8I || internal_format == GL_RG8UI ||
            internal_format == GL_RG16 || internal_format == GL_RG16I || internal_format == GL_RG16UI || internal_format == GL_RG16F ||
            internal_format == GL_RG32I || internal_format == GL_RG32UI || internal_format == GL_RG32F
            )
        {
            if (format != GL_RG) {
                LOG(ERROR) << "color format must be GL_RG";
                return false;
            }
        }
        else if (internal_format == GL_RGB8 || internal_format == GL_RGB8I || internal_format == GL_RGB8UI ||
            internal_format == GL_RGB16 || internal_format == GL_RGB16I || internal_format == GL_RGB16UI || internal_format == GL_RGB16F ||
            internal_format == GL_RGB32I || internal_format == GL_RGB32UI || internal_format == GL_RGB32F
            )
        {
            if (format != GL_RGB) {
                LOG(ERROR) << "color format must be GL_RGB";
                return false;
            }
        }
        else if (internal_format == GL_RGBA8 || internal_format == GL_RGBA8I || internal_format == GL_RGBA8UI ||
            internal_format == GL_RGBA16 || internal_format == GL_RGBA16I || internal_format == GL_RGBA16UI || internal_format == GL_RGBA16F ||
            internal_format == GL_RGBA32I || internal_format == GL_RGBA32UI || internal_format == GL_RGBA32F
            )
        {
            if (format != GL_RGBA) {
                LOG(ERROR) << "color format must be GL_RGBA";
                return false;
            }
        }

        //////////////////////////////////////////////////////////////////////////

        // TODO: check signed, unsigned, byte, int, float compatibility...

        return true;
    }


    bool FramebufferObject::add_color_texture(
        GLenum internal_format /* = GL_RGBA8*/,	// GL_[components​][size​][type​], e.g., GL_RG8, GL_RGBA16, GL_R16F, GL_RG16, GL_RGBA32F ...
        GLenum format /* = GL_RGBA*/,			// The format of the pixel data (GL_RED, GL_RG, GL_RGB, GL_BGR, GL_BGRA ...)
        GLenum type /* = GL_UNSIGNED_BYTE*/,	// The data type of the pixel data (GL_BYTE, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT ...)
        GLenum filter /* = GL_NEAREST*/			// The texture minifying/magnification functions. e.g., GL_NEAREST, GL_LINEAR
    )
    {
        if (fbo_id_ == 0) {
            LOG(ERROR) << "fbo not created";
            return false;
        }

        // check the compatibility of the provided parameters.
        if (!color_format_compatible(internal_format, format, type)) {
            LOG(ERROR) << "attaching color buffer failed";
            return false;
        }

        GLint max_attachments;
        // How many color attachment points an FBO can have.
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_attachments);
        if (color_attachments_.size() >= max_attachments) {
            LOG(ERROR) << "maximum color attachment reached";
            return false;
        }

        int index = static_cast<int>(color_attachments_.size());

        ColorAttachment attachment;
        attachment.internal_format = internal_format;
        attachment.format = format;
        attachment.type = type;
        attachment.texture_filter = filter;

        //----------------------------------------------------------------------------------------------
        // color render buffer bound to texture

        // see https://www.khronos.org/opengl/wiki/Framebuffer_Object_Extension_Examples#Quick_example.2C_render_to_texture_.282D.29
        glGenTextures(1, &(attachment.texture));		easy3d_debug_log_gl_error;

        if (samples_ > 0) {
            texture_target_ = GL_TEXTURE_2D_MULTISAMPLE;
            glBindTexture(texture_target_, attachment.texture);		easy3d_debug_log_gl_error;
            glTexImage2DMultisample(texture_target_, samples_, internal_format, width_, height_, GL_TRUE);		easy3d_debug_log_gl_error;
        }
        else {
            texture_target_ = GL_TEXTURE_2D;
            glBindTexture(texture_target_, attachment.texture);		easy3d_debug_log_gl_error;

            glTexParameteri(texture_target_, GL_TEXTURE_MIN_FILTER, filter);		easy3d_debug_log_gl_error;
            glTexParameteri(texture_target_, GL_TEXTURE_MAG_FILTER, filter);		easy3d_debug_log_gl_error;
            glTexParameteri(texture_target_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	easy3d_debug_log_gl_error;
            glTexParameteri(texture_target_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	easy3d_debug_log_gl_error;
            glTexImage2D(texture_target_, 0, internal_format, width_, height_, 0, format, type, nullptr);	easy3d_debug_log_gl_error;
         }

        //----------------------------------------------------------------------------------------------

        GLuint currentFbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
        if (fbo_id_ != currentFbo)
            glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);				easy3d_debug_log_gl_error;

        // attach texture
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, texture_target_, attachment.texture, 0);		easy3d_debug_log_gl_error;

        if (fbo_id_ != currentFbo)
            glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);			easy3d_debug_log_gl_error;

        //----------------------------------------------------------------------------------------------

        valid_ = check_status();
        if (valid_) 
			color_attachments_.push_back(attachment);
		else
			glDeleteTextures(1, &(attachment.texture));		easy3d_debug_log_gl_error;

        glBindTexture(texture_target_, 0);		easy3d_debug_log_gl_error;

        return valid_;
    }


    bool FramebufferObject::add_color_buffer(
        GLenum internal_format /* = GL_RGBA8*/,	// GL_[components​][size​][type​], e.g., GL_RG8, GL_RGBA16, GL_R16F, GL_RG16, GL_RGBA32F ...
        GLenum format /* = GL_RGBA*/,			// The format of the pixel data (GL_RED, GL_RG, GL_RGB, GL_BGR, GL_BGRA ...)
        GLenum type /* = GL_UNSIGNED_BYTE*/)	// The data type of the pixel data (GL_BYTE, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT ...)
    {
        if (fbo_id_ == 0) {
            LOG(ERROR) << "fbo not generated";
            return false;
        }

        // check the compatibility of the provided parameters.
        if (!color_format_compatible(internal_format, format, type)) {
            LOG(ERROR) << "attaching color buffer failed";
            return false;
        }

        GLint max_attachments;
        // How many color attachment points an FBO can have.
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_attachments);
        if (color_attachments_.size() >= max_attachments) {
            LOG(ERROR) << "maximum color attachment reached";
            return false;
        }

        const int index = static_cast<int>(color_attachments_.size());

        ColorAttachment attachment;
        attachment.internal_format = internal_format;
        attachment.format = format;
        attachment.type = type;
        attachment.texture_filter = GL_NEAREST;	// set to default value

        //----------------------------------------------------------------------------------------------

        // color render buffer not bound to texture

        glGenRenderbuffers(1, &(attachment.buffer));				easy3d_debug_log_gl_error;
        glBindRenderbuffer(GL_RENDERBUFFER, attachment.buffer);		easy3d_debug_log_gl_error;

        if (samples_ > 0) {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples_, internal_format, width_, height_);	easy3d_debug_log_gl_error;
        }
        else {
            glRenderbufferStorage(GL_RENDERBUFFER, internal_format, width_, height_);						easy3d_debug_log_gl_error;
        }

        //----------------------------------------------------------------------------------------------

        GLuint currentFbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
        if (fbo_id_ != currentFbo)
            glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);				easy3d_debug_log_gl_error;

        // attach color render buffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, attachment.buffer);		easy3d_debug_log_gl_error;

        if (fbo_id_ != currentFbo)
            glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);			easy3d_debug_log_gl_error;

        //----------------------------------------------------------------------------------------------

        valid_ = check_status();
        if (valid_) {
            // Query the actual number of samples. This can be greater than the requested
            // value since the typically supported values are 0, 4, 8, ..., and the
            // requests are mapped to the next supported value.
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples_);	easy3d_debug_log_gl_error;
			color_attachments_.push_back(attachment);
       }
        else {
            glDeleteRenderbuffers(1, &(attachment.buffer));	easy3d_debug_log_gl_error;
        }

        glBindRenderbuffer(GL_RENDERBUFFER, 0);		easy3d_debug_log_gl_error;

        return valid_;
    }


    // GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT32F, GL_DEPTH24_STENCIL8, GL_DEPTH32F_STENCIL8, GL_DEPTH_COMPONENT16
    bool FramebufferObject::add_depth_texture(
        GLenum internal_format /* = GL_DEPTH24_STENCIL8 */,
        GLenum filter /* = GL_NEAREST */,
        GLenum compare_mode /* = GL_NONE*/,
        GLenum compare_func /* = GL_LEQUAL*/)
    {
        if (fbo_id_ == 0) {
            LOG(ERROR) << "fbo not generated";
            return false;
        }

        bool combined_stencil = (internal_format == GL_DEPTH24_STENCIL8) || (internal_format == GL_DEPTH32F_STENCIL8);
        bool float_type = (internal_format == GL_DEPTH_COMPONENT32F) || (internal_format == GL_DEPTH32F_STENCIL8);

        // determine the combination of the parameters
        GLenum format = combined_stencil ? GL_DEPTH_STENCIL : GL_DEPTH_COMPONENT;

        GLenum type, attachment;
        if (combined_stencil) {
            type = float_type ? GL_FLOAT_32_UNSIGNED_INT_24_8_REV : GL_UNSIGNED_INT_24_8;
            attachment = GL_DEPTH_STENCIL_ATTACHMENT;
        }
        else {
            type = float_type ? GL_FLOAT : GL_UNSIGNED_INT;
            attachment = GL_DEPTH_ATTACHMENT;
        }

        depth_internal_format_ = internal_format;
        depth_texture_filter_ = filter;
        depth_texture_compare_mode_ = compare_mode;
        depth_texture_compare_func_ = compare_func;

        //----------------------------------------------------------------------------------------------

        // depth render buffer bound to texture

        // see https://www.khronos.org/opengl/wiki/Framebuffer_Object_Extension_Examples#Quick_example.2C_render_to_texture_.282D.29
        glGenTextures(1, &depth_texture_);		easy3d_debug_log_gl_error;

        if (samples_ > 0) {
            texture_target_ = GL_TEXTURE_2D_MULTISAMPLE;

            glBindTexture(texture_target_, depth_texture_);		easy3d_debug_log_gl_error;
            // Liangliang: It seems no parameters need to be set for multisample depth texture.
            //glTexParameteri(texture_target_, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);		easy3d_debug_log_gl_error;

            glTexImage2DMultisample(texture_target_, samples_, internal_format, width_, height_, GL_TRUE);		easy3d_debug_log_gl_error;
         }
        else {
            texture_target_ = GL_TEXTURE_2D;

            glBindTexture(texture_target_, depth_texture_);		easy3d_debug_log_gl_error;

            glTexParameteri(texture_target_, GL_TEXTURE_MIN_FILTER, filter);		easy3d_debug_log_gl_error;
            glTexParameteri(texture_target_, GL_TEXTURE_MAG_FILTER, filter);		easy3d_debug_log_gl_error;

            glTexParameteri(texture_target_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	easy3d_debug_log_gl_error;
            glTexParameteri(texture_target_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	easy3d_debug_log_gl_error;
            //glTexParameteri(texture_target_, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);	easy3d_debug_log_gl_error;

            glTexParameteri(texture_target_, GL_TEXTURE_COMPARE_MODE, compare_mode);		easy3d_debug_log_gl_error;
            if (compare_mode != GL_NONE) {
                glTexParameteri(texture_target_, GL_TEXTURE_COMPARE_FUNC, compare_func);	easy3d_debug_log_gl_error;
                // GL_COMPARE_REF_TO_TEXTURE sets the comparison function to 'less than or equal', so the
                // result of the sample operation will be 1.0 if the reference value is less than or equal
                // to the value in the texture and zero otherwise.
            }

            glTexImage2D(texture_target_, 0, internal_format, width_, height_, 0, format, type, nullptr);	easy3d_debug_log_gl_error;
        }

        //----------------------------------------------------------------------------------------------

        GLuint currentFbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
        if (fbo_id_ != currentFbo)
            glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);				easy3d_debug_log_gl_error;

        // attach texture
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texture_target_, depth_texture_, 0);		easy3d_debug_log_gl_error;

        if (fbo_id_ != currentFbo)
            glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);			easy3d_debug_log_gl_error;

        //----------------------------------------------------------------------------------------------

        valid_ = check_status();
        if (!valid_) {
            glDeleteTextures(1, &depth_texture_);	easy3d_debug_log_gl_error;
        }

        glBindTexture(texture_target_, 0);			easy3d_debug_log_gl_error;

        return valid_;
    }


    // GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT32F, GL_DEPTH24_STENCIL8, GL_DEPTH32F_STENCIL8, GL_DEPTH_COMPONENT16
    bool FramebufferObject::add_depth_buffer(GLenum internal_format /* = GL_DEPTH24_STENCIL8*/) {
        if (fbo_id_ == 0) {
            LOG(ERROR) << "fbo not created";
            return false;
        }

        GLenum attachment;
        if ((internal_format == GL_DEPTH24_STENCIL8) || (internal_format == GL_DEPTH32F_STENCIL8)) {
            attachment = GL_DEPTH_STENCIL_ATTACHMENT;
        }
        else {
            attachment = GL_DEPTH_ATTACHMENT;
        }

        depth_internal_format_ = internal_format;
        depth_texture_filter_ = GL_NEAREST;	// set to default value

        //----------------------------------------------------------------------------------------------

        // depth render buffer not bound to texture

        glGenRenderbuffers(1, &depth_buffer_);				easy3d_debug_log_gl_error;
        glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_);	easy3d_debug_log_gl_error;

        // In practice, a combined depth-stencil buffer is supported by all desktop platforms, while a
        // separate stencil buffer is not. On embedded devices however, a combined depth-stencil buffer
        // might not be supported while separate buffers are, according to QTBUG-12861.

        if (samples_ > 0) {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples_, internal_format, width_, height_);		easy3d_debug_log_gl_error;
        }
        else {
            glRenderbufferStorage(GL_RENDERBUFFER, internal_format, width_, height_);		easy3d_debug_log_gl_error;
        }

        //----------------------------------------------------------------------------------------------

        GLuint currentFbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
        if (fbo_id_ != currentFbo)
            glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);				easy3d_debug_log_gl_error;

        // attach depth render buffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, depth_buffer_);		easy3d_debug_log_gl_error;

        if (fbo_id_ != currentFbo)
            glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);	easy3d_debug_log_gl_error;

        //----------------------------------------------------------------------------------------------

        valid_ = check_status();
        if (!valid_) {
            glDeleteRenderbuffers(1, &depth_buffer_);		easy3d_debug_log_gl_error;
            depth_buffer_ = 0;
        }

        glBindRenderbuffer(GL_RENDERBUFFER, 0);		easy3d_debug_log_gl_error;

        return valid_;
    }


	// attach an existing color texture to the framebuffer
	bool FramebufferObject::attach_color_texture(GLenum target, GLuint texture_id, GLenum attachment) {
		GLuint currentFbo = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
		if (fbo_id_ != currentFbo)
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);				easy3d_debug_log_gl_error;

		// attach texture
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, texture_id, 0);		easy3d_debug_log_gl_error;

		if (fbo_id_ != currentFbo)
			glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);			easy3d_debug_log_gl_error;

        ColorAttachment attach;
        attach.texture = texture_id;
//        attach.internal_format = internal_format;
//        attach.format = format;
//        attach.type = type;
//        attach.texture_filter = GL_NEAREST;
        LOG(WARNING) << "attach an externally created texture not fully supported";
        color_attachments_.push_back(attach);

		//----------------------------------------------------------------------------------------------

		valid_ = check_status();
		if (!valid_) 
			glDeleteTextures(1, &texture_id);	easy3d_debug_log_gl_error;

		return valid_;
	}


	// attach an existing depth texture to the framebuffer
	bool FramebufferObject::attach_depth_texture(GLenum target, GLuint texture_id, GLenum attachment) {
		//----------------------------------------------------------------------------------------------

		GLuint currentFbo = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
		if (fbo_id_ != currentFbo)
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);				easy3d_debug_log_gl_error;

		// attach texture
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, texture_id, 0);		easy3d_debug_log_gl_error;

		if (fbo_id_ != currentFbo)
			glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);			easy3d_debug_log_gl_error;

		//----------------------------------------------------------------------------------------------

		valid_ = check_status();
		if (!valid_)
			glDeleteTextures(1, &texture_id);	easy3d_debug_log_gl_error;

		return valid_;
	}


    bool FramebufferObject::check_status() const
    {
        GLuint currentFbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

        //////////////////////////////////////////////////////////////////////////

        bool complete = false;

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        switch (status)
        {
        case GL_NO_ERROR:
        case GL_FRAMEBUFFER_COMPLETE:
            complete = true;
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            LOG(ERROR) << "Unsupported framebuffer format.";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            LOG(ERROR) << "Framebuffer incomplete attachment.";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            LOG(ERROR) << "Framebuffer incomplete, missing attachment.";
            break;
    #ifdef GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT
        case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT:
            LOG(ERROR) << "Framebuffer incomplete, duplicate attachment.";
            break;
    #endif
    #ifdef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
            LOG(ERROR) << "Framebuffer incomplete, attached images must have same dimensions.";
            break;
    #endif
    #ifdef GL_FRAMEBUFFER_INCOMPLETE_FORMATS
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
            LOG(ERROR) << "Framebuffer incomplete, attached images must have same format.";
            break;
    #endif
    #ifdef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            LOG(ERROR) << "Framebuffer incomplete, missing draw buffer.";
            break;
    #endif
    #ifdef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            LOG(ERROR) << "Framebuffer incomplete, missing read buffer.";
            break;
    #endif
    #ifdef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            LOG(ERROR) << "Framebuffer incomplete, attachments must have same number of samples per pixel.";
            break;
    #endif
        default:
            LOG(ERROR) << "An undefined error has occurred: " << status;
            break;
        }

        //////////////////////////////////////////////////////////////////////////

        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;

        return complete;
    }


    /*!
    Returns true if the framebuffer object is valid.

    The framebuffer can become invalid if the initialization process
    fails, the user attaches an invalid buffer to the framebuffer
    object, or a non-power of two width/height is specified as the
    texture size if the texture target is \c{GL_TEXTURE_2D}.
    The non-power of two limitation does not apply if the OpenGL version
    is 2.0 or higher, or if the GL_ARB_texture_non_power_of_two extension
    is present.

    The framebuffer can also become invalid if the OpenGL context that the
    framebuffer was created within is destroyed and there are no other
    shared contexts that can take over ownership of the framebuffer.
    */
    bool FramebufferObject::is_valid() const {
        return valid_ && (fbo_id_ != 0);
    }


    bool FramebufferObject::is_bound(GLenum target /* = GL_FRAMEBUFFER */) const
    {
        if (!is_valid()) {
            LOG(ERROR) << "framebuffer not valid";
            return false;
        }

        GLint currentFbo = 0;
        switch (target)
        {
        case GL_DRAW_FRAMEBUFFER:
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
            break;

        case GL_READ_FRAMEBUFFER:
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
            break;

        case GL_FRAMEBUFFER:
        default:
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);			easy3d_debug_log_gl_error;
            break;
        }

        return GLuint(currentFbo) == fbo_id_;
    }


    bool FramebufferObject::bind(GLenum target /* = GL_FRAMEBUFFER */)
    {
        if (!is_valid()) {
            LOG(ERROR) << "framebuffer not valid";
            return false;
        }

        GLuint currentFbo = 0;
        switch (target)
        {
        case GL_DRAW_FRAMEBUFFER:
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
            if (fbo_id_ != currentFbo) {
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id_);				easy3d_debug_log_gl_error;
                prev_draw_fbo_ = currentFbo;
            }
            break;

        case GL_READ_FRAMEBUFFER:
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
            if (fbo_id_ != currentFbo) {
                glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id_);				easy3d_debug_log_gl_error;
                prev_read_fbo_ = currentFbo;
            }
            break;

        case GL_FRAMEBUFFER:
        default:
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);			easy3d_debug_log_gl_error;
            if (fbo_id_ != currentFbo) {
                glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);				easy3d_debug_log_gl_error;
                prev_draw_fbo_ = currentFbo;
                prev_read_fbo_ = currentFbo;
            }
            break;
        }

        return true;
    }


    bool FramebufferObject::release(GLenum target /* = GL_FRAMEBUFFER */)
    {
        if (!is_bound())
            return true;

        GLuint currentFbo = 0;
        switch (target)
        {
        case GL_DRAW_FRAMEBUFFER:
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
            break;

        case GL_READ_FRAMEBUFFER:
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
            break;

        case GL_FRAMEBUFFER:
        default:
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);			easy3d_debug_log_gl_error;
            break;
        }

        bool drawing = (target == GL_DRAW_FRAMEBUFFER) || (target == GL_FRAMEBUFFER);
        GLuint prevFbo = (drawing ? prev_draw_fbo_ : prev_read_fbo_);
        if (currentFbo != prevFbo) {
            if (glIsFramebuffer(prevFbo)) { // always test in case prevFbo has been deleted
                glBindFramebuffer(target, prevFbo);	easy3d_debug_log_gl_error;
            }
            else {
                glBindFramebuffer(target, 0);	easy3d_debug_log_gl_error;
            }
        }

        return true;
    }


    void FramebufferObject::activate_draw_buffer(unsigned int index) {
        //activate_draw_buffers(1, &index);

        GLuint currentFbo = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

        //////////////////////////////////////////////////////////////////////////

        index += GL_COLOR_ATTACHMENT0;
        glDrawBuffers(1, &index);			easy3d_debug_log_gl_error;

        //////////////////////////////////////////////////////////////////////////

        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;
    }


    void FramebufferObject::activate_draw_buffers(unsigned int numbuffers, unsigned int indices[]) {
        GLuint currentFbo = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

        //////////////////////////////////////////////////////////////////////////

        GLint maxbuffers;
        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxbuffers);

        GLenum *buffers = new GLenum[maxbuffers];
        GLint count = 0;
        for (std::size_t cc = 0; cc < numbuffers && count < maxbuffers; cc++) {
            buffers[cc] = GL_COLOR_ATTACHMENT0 + indices[cc];
            count++;
        }

        glDrawBuffers(count, buffers);			easy3d_debug_log_gl_error;

        delete[] buffers;

        //////////////////////////////////////////////////////////////////////////

        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;
    }


    void FramebufferObject::activate_draw_buffers(unsigned int minId, unsigned int maxId) {
        GLuint currentFbo = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

        //////////////////////////////////////////////////////////////////////////

        GLuint maxbuffers;
        glGetIntegerv(GL_MAX_DRAW_BUFFERS, (GLint*)&maxbuffers);

        GLenum *buffers = new GLenum[maxbuffers];
        unsigned int count = 0;
        for (unsigned int cc = minId; cc <= maxId && count < maxbuffers; cc++) {
            buffers[cc] = GL_COLOR_ATTACHMENT0 + cc;
            count++;
        }

        glDrawBuffers(count, buffers);			easy3d_debug_log_gl_error;

        delete[] buffers;

        //////////////////////////////////////////////////////////////////////////

        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;
    }


    void FramebufferObject::deactivate_draw_buffers() {
        GLuint currentFbo = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

        //////////////////////////////////////////////////////////////////////////

        GLenum att = GL_NONE;
        glDrawBuffers(1, &att);	easy3d_debug_log_gl_error;
        // or
        //glDrawBuffer(GL_NONE);	easy3d_debug_log_gl_error;

        //////////////////////////////////////////////////////////////////////////

        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;
    }


    void FramebufferObject::activate_read_buffer(unsigned int index) {
        GLuint currentFbo = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

        //////////////////////////////////////////////////////////////////////////

        index += GL_COLOR_ATTACHMENT0;
        glReadBuffer((GLenum)index);			easy3d_debug_log_gl_error;

        //////////////////////////////////////////////////////////////////////////

        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_READ_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;
    }


    void FramebufferObject::deactivate_read_buffer() {
        GLuint currentFbo = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

        //////////////////////////////////////////////////////////////////////////

        glReadBuffer(GL_NONE);	easy3d_debug_log_gl_error;

        //////////////////////////////////////////////////////////////////////////

        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_READ_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;
    }


    int FramebufferObject::num_color_attachements() const {
        return static_cast<int>(color_attachments_.size());
    }


    bool FramebufferObject::has_color_attachment(unsigned int index) const {
        return (/*index >= 0 && */index < color_attachments_.size());
    }


    bool FramebufferObject::has_color_texture(unsigned int index) const {
        if (has_color_attachment(index)) {
            GLuint texture = color_attachments_[index].texture;
            if (texture != 0 && glIsTexture(texture))
                return true;
        }
        return false;
    }


    void FramebufferObject::_prepare_resolve_fbo() {
        if (!resolved_fbo_) {
            resolved_fbo_ = new FramebufferObject(width(), height(), 0);	easy3d_debug_log_gl_error;
            for (std::size_t i = 0; i < color_attachments_.size(); ++i) {
                const ColorAttachment& att = color_attachments_[i];
                if (att.texture)
                    resolved_fbo_->add_color_texture(att.internal_format, att.format, att.type);
                else
                    resolved_fbo_->add_color_buffer(att.internal_format, att.format, att.type);
            }

            bool need_depth = (depth_buffer_ != 0) || (depth_texture_ != 0);
            if (need_depth) {
                if (depth_texture_)
                    resolved_fbo_->add_depth_texture(depth_internal_format_, depth_texture_filter_, depth_texture_compare_mode_, depth_texture_compare_func_);
                else
                    resolved_fbo_->add_depth_buffer(depth_internal_format_);
            }
            resolved_fbo_->check_status();
        }
        if (resolved_fbo_->num_color_attachements() != num_color_attachements()) {
            delete resolved_fbo_;
            resolved_fbo_ = nullptr;
            _prepare_resolve_fbo();
        }

        resolved_fbo_->ensure_size(width(), height());	easy3d_debug_log_gl_error;
    }


    GLuint FramebufferObject::color_texture(unsigned int index /* = 0 */, bool resolve /* = true */) const {
        if (!has_color_attachment(index)) {
            LOG(ERROR) << "color attachment " << index << " does not exist";
            return 0;
        }

        GLuint texture = color_attachments_[index].texture;
        if (texture && glIsTexture(texture)) {
            if (samaples() == 0 || !resolve)
                return texture;
            else { // resolve
                const_cast<FramebufferObject*>(this)->_prepare_resolve_fbo();
                blit_framebuffer(const_cast<FramebufferObject*>(resolved_fbo_), this, index, index, GL_COLOR_BUFFER_BIT);	easy3d_debug_log_gl_error;
                return resolved_fbo_->color_texture(index);
            }
        }
        else {
            LOG(ERROR) << "color attachment " << index << " is not a texture";
            return 0;
        }
    }


    GLuint FramebufferObject::depth_texture(bool resolve /* = true */) const {
        if (depth_texture_ && glIsTexture(depth_texture_)) {
			if (samaples() == 0 || !resolve)
                return depth_texture_;
            else { // resolve
                const_cast<FramebufferObject*>(this)->_prepare_resolve_fbo();
                blit_framebuffer(const_cast<FramebufferObject*>(resolved_fbo_), this, GL_DEPTH_BUFFER_BIT);	easy3d_debug_log_gl_error;
                return resolved_fbo_->depth_texture();
            }
        }
        else {
            LOG(ERROR) << "depth attachment is not a texture";
            return 0;
        }
    }


    GLenum FramebufferObject::texture_target() const {
        return texture_target_;
    }


    bool FramebufferObject::has_stencil() const {
        return (depth_internal_format_ == GL_DEPTH24_STENCIL8) || (depth_internal_format_ == GL_DEPTH32F_STENCIL8);
    }


    bool FramebufferObject::has_depth_attachment() const {
        return (depth_texture_ != 0) || (depth_buffer_ != 0);
    }


    bool FramebufferObject::has_depth_texture() const {
        return (depth_texture_ != 0);
    }


    int FramebufferObject::depth_bits() const {
        switch (depth_internal_format_)
        {
        case GL_DEPTH_COMPONENT16:
            return 16;
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH24_STENCIL8:
            return 24;
        case GL_DEPTH_COMPONENT32:
        case GL_DEPTH_COMPONENT32F:
        case GL_DEPTH32F_STENCIL8:
            return 32;
        }

        LOG(ERROR) << "unknown depth internal format";
        return 0; // should not reach here
    }


    void FramebufferObject::print_attachments() const {
        GLuint currentFbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

        //////////////////////////////////////////////////////////////////////////

        LOG(INFO) << "current framebuffer is bound to framebuffer object " << fbo_id_;

        GLint maxColorAttachments;
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
        int index = 0;
        while (index < maxColorAttachments) {
            LOG(INFO) << "color attachment " << index << ":";
            _print_attachment(GL_COLOR_ATTACHMENT0 + index);
            ++index;
        }

        LOG(INFO) << "depth attachment:";
        _print_attachment(GL_DEPTH_ATTACHMENT);

        LOG(INFO) << "stencil attachment:";
        _print_attachment(GL_STENCIL_ATTACHMENT);

        //////////////////////////////////////////////////////////////////////////

        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;
    }


    void FramebufferObject::_print_attachment(unsigned int index) const {
        GLuint currentFbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

        //////////////////////////////////////////////////////////////////////////

        GLenum attachment = static_cast<GLenum>(index);

        GLint params;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &params);

        switch (params)
        {
        case GL_NONE:
            LOG(INFO) << "\tthis attachment is empty";
            break;

        case GL_TEXTURE:
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &params);
            LOG(INFO) << "\tthis attachment is a texture with name: " << params;

            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, &params);
            LOG(INFO) << "\tits mipmap level is: " << params;

            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE, &params);
            if (params == 0)
                LOG(INFO) << "\tthis is not a cube map texture.";
            else
                LOG(INFO) << "\tthis is a cube map texture and the image is contained in face " << params;

    #ifdef GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET, &params);
            if (params == 0)
                LOG(INFO) << "\tthis is not 3D texture.";
            else
                LOG(INFO) << "\tthis is a 3D texture and the z-offset of the attached image is " << params;
            break;
    #endif

        case GL_RENDERBUFFER:
            LOG(INFO) << "\tthis attachment is a render buffer";

            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &params);
            LOG(INFO) << "\tthis attachment is a render buffer with name: " << params;

            glBindRenderbuffer(GL_RENDERBUFFER, params);
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &params);
            LOG(INFO) << "\trender buffer width = " << params;

            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &params);
            LOG(INFO) << "\trender buffer height = " << params;

            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &params);
            LOG(INFO) << "\trender buffer samples = " << params;

            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &params);
            LOG(INFO) << "\trender buffer internal format = 0x" << std::hex << params << std::dec;

            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_RED_SIZE, &params);
            LOG(INFO) << "\trender buffer actual resolution for the red component = " << params;

            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_GREEN_SIZE, &params);
            LOG(INFO) << "\trender buffer actual resolution for the green component = " << params;

            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_BLUE_SIZE, &params);
            LOG(INFO) << "\trender buffer actual resolution for the blue component = " << params;

            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_ALPHA_SIZE, &params);
            LOG(INFO) << "\trender buffer actual resolution for the alpha component = " << params;

            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_DEPTH_SIZE, &params);
            LOG(INFO) << "\trender buffer actual resolution for the depth component = " << params;

            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_STENCIL_SIZE, &params);
            LOG(INFO) << "\trender buffer actual resolution for the stencil component = " << params;
            break;

        default:
            LOG(INFO) << "\tunexpected value.";
            break;
        }

        //////////////////////////////////////////////////////////////////////////

        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;
    }


    void FramebufferObject::print_draw_buffers() const {
        GLuint currentFbo = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

        //////////////////////////////////////////////////////////////////////////

        GLint ivalue = 1;
        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &ivalue);

        LOG(INFO) << "num of draw buffers: " << ivalue;

        GLint index = 0;
        int c = ivalue;
        while (index < c) {
            glGetIntegerv(GL_DRAW_BUFFER0 + index, &ivalue);

            LOG(INFO) << "draw buffer[" << index << "] = ";
            _print_buffer(ivalue);
            ++index;
        }

        //////////////////////////////////////////////////////////////////////////

        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;
    }


    void FramebufferObject::print_read_buffer() const {
        GLuint currentFbo = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

        //////////////////////////////////////////////////////////////////////////

        GLint ivalue;
        glGetIntegerv(GL_READ_BUFFER, &ivalue);

        LOG(INFO) << "read buffer = ";
        _print_buffer(ivalue);

        //////////////////////////////////////////////////////////////////////////

        if (currentFbo != fbo_id_)
            glBindFramebuffer(GL_READ_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;
    }


    void FramebufferObject::_print_buffer(int value) const {
        if (value >= static_cast<int>(GL_COLOR_ATTACHMENT0) &&
            value <= static_cast<int>(GL_COLOR_ATTACHMENT0 + 15))
        {
            LOG(INFO) << "\tGL_COLOR_ATTACHMENT" << (value - GL_COLOR_ATTACHMENT0);
        }
        else {
            if (value >= GL_AUX0) {
                int b = value - GL_AUX0;
                GLint ivalue;
                glGetIntegerv(GL_AUX_BUFFERS, &ivalue);
                if (b < ivalue) {
                    LOG(INFO) << "\tGL_AUX" << b;
                }
                else {
                    LOG(INFO) << "\tinvalid aux buffer: " << b << ", upper limit is "
                        << (ivalue - 1) << ", raw value is 0x" << std::hex << (GL_AUX0 + b)
                        << std::dec;
                }
            }
            else {
                switch (value)
                {
                case GL_NONE:
                    LOG(INFO) << "\tGL_NONE";
                    break;
                case GL_FRONT_LEFT:
                    LOG(INFO) << "\tGL_FRONT_LEFT";
                    break;
                case GL_FRONT_RIGHT:
                    LOG(INFO) << "\tGL_FRONT_RIGHT";
                    break;
                case GL_BACK_LEFT:
                    LOG(INFO) << "\tGL_BACK_LEFT";
                    break;
                case GL_BACK_RIGHT:
                    LOG(INFO) << "\tGL_BACK_RIGHT";
                    break;
                case GL_FRONT:
                    LOG(INFO) << "\tGL_FRONT";
                    break;
                case GL_BACK:
                    LOG(INFO) << "\tGL_BACK";
                    break;
                case GL_LEFT:
                    LOG(INFO) << "\tGL_LEFT";
                    break;
                case GL_RIGHT:
                    LOG(INFO) << "\tGL_RIGHT";
                    break;
                case GL_FRONT_AND_BACK:
                    LOG(INFO) << "\tGL_FRONT_AND_BACK";
                    break;
                default:
                    LOG(INFO) << "\tunknown 0x" << std::hex << value << std::dec;
                    break;
                }
            }
        }
    }


    bool FramebufferObject::read_color(unsigned int index, unsigned char* buffer, unsigned int format, bool flip_vertically /* = true */) const
    {
        if (!has_color_attachment(index)) {
            LOG(ERROR) << "color attachment " << index << " does not exist";
            return false;
        }

        unsigned int bytes_per_pixel = 0;
        if (format == GL_RGB || format == GL_BGR)
            bytes_per_pixel = 3;
        else if (format == GL_RGBA || format == GL_BGRA)
            bytes_per_pixel = 4;
        else {
            LOG(ERROR) << "to read color buffer, the format must be one of GL_RGB, GL_BGR, GL_RGBA, and GL_BGRA.";
            return false;
        }

        glFinish();
        if (samples_ <= 0) {
            GLuint currentFbo = 0;
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
            if (currentFbo != fbo_id_)
                glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

            const_cast<FramebufferObject*>(this)->activate_read_buffer(index);

            // We need to configure how glReadPixels will behave with respect to memory alignment.
            // See http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-an-opengl-hack/
            // and: https://www.khronos.org/opengl/wiki/Common_Mistakes
            glPixelStorei(GL_PACK_ALIGNMENT, 1);				easy3d_debug_log_gl_error;
            // Bind to 0 for conventional pixel operation. See http://www.songho.ca/opengl/gl_pbo.html
            // I don't need it because no PBO is used.
            //glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);				easy3d_debug_log_gl_error;

            glReadPixels(0, 0, width_, height_, format, GL_UNSIGNED_BYTE, buffer);	easy3d_debug_log_gl_error;

            if (currentFbo != fbo_id_)
                glBindFramebuffer(GL_READ_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;

            // flip vertically
            if (flip_vertically) {
                unsigned int row_size = width_ * bytes_per_pixel;
                unsigned int half_height = height_ / 2;
#if 0
                for (int j = 0; j < half_height; ++j) {
                    // get a pointer to the two lines we will swap
                    unsigned char* row1 = buffer + j * row_size;
                    unsigned char* row2 = buffer + (height_ - 1 - j) * row_size;
                    // for each point on line, swap all the channels
                    for (int i = 0; i < width_; ++i) {
                        for (int k = 0; k < bytes_per_pixel; k++) {
                            std::swap(row1[bytes_per_pixel*i + k], row2[bytes_per_pixel*i + k]);
                        }
                    }
                }
#else
                unsigned char *line = new unsigned char[row_size];
                for (unsigned int i=0, j=height_-1; i<half_height; ++i) {
                    memcpy(line, buffer + i * row_size, row_size);
                    memcpy(buffer + i * row_size, buffer + j * row_size, row_size);
                    memcpy(buffer + j * row_size, line, row_size);
                    j--;
                }
                delete[] line;
#endif
            }
        }

        else {
            const_cast<FramebufferObject*>(this)->_prepare_resolve_fbo();
            blit_framebuffer(const_cast<FramebufferObject*>(resolved_fbo_), this, index, index, GL_COLOR_BUFFER_BIT);	easy3d_debug_log_gl_error;
            resolved_fbo_->read_color(index, buffer, format, flip_vertically);
        }

        return true;
    }


    bool FramebufferObject::read_color(unsigned int index, std::vector<unsigned char>& buffer, unsigned int format, bool flip_vertically /* = true */) const {
        unsigned int bytes_per_pixel = 0;
        if (format == GL_RGB || format == GL_BGR)
            bytes_per_pixel = 3;
        else if (format == GL_RGBA || format == GL_BGRA)
            bytes_per_pixel = 4;
        else {
            LOG(ERROR) << "to read color buffer, the format must be one of GL_RGB, GL_BGR, GL_RGBA, and GL_BGRA.";
            return false;
        }

        buffer.resize(bytes_per_pixel * width_ * height_);
        return read_color(index, buffer.data(), format, flip_vertically);
    }


    bool FramebufferObject::snapshot_color(unsigned int index, const std::string& file_name) const {
        if (!is_valid()) {
            LOG(ERROR) << "framebuffer not valid";
            return false;
        }

        // I can acutally alway ask for RGBA format and use my ImageIO for saving.
        // But for ppm, bmp and tga formats, this causes extra internal
        // formatting (in stb_image_write).

        std::vector<unsigned char> bits;

        const std::string& ext = file_system::extension(file_name, true);
        if (ext == "png" || ext == "jpg") {
            if (!read_color(index, bits, GL_RGBA, true))
                return false;
            return ImageIO::save(file_name, bits, width_, height_, 4);
        }
        else if (ext == "ppm") {
            std::vector<unsigned char> bits;
            if (!read_color(index, bits, GL_RGB, true))
                return false;
            return io::save_ppm(file_name, bits, width_, height_);
        }
        else if (ext == "bmp") {
            if (!read_color(index, bits, GL_BGRA, false))   // bmp is alway flipped?
                return false;
            return io::save_bmp(file_name, bits, width_, height_);
        }
        else if (ext == "tga") {
            if (!read_color(index, bits, GL_BGRA, true))
                return false;
            return io::save_tga(file_name, bits, width_, height_);
        }
        else {
            LOG(ERROR) << "unknown file format: " << ext;
            return false;
        }
    }


    // read the depth data of the framebuffer into a specified buffer.
    bool FramebufferObject::read_depth(float* buffer, bool flip_vertically /* = true */) const
    {
        if (!has_depth_attachment()) {
            LOG(ERROR) << "depth attachment does not exist";
            return false;
        }

        glFinish();
        if (samples_ <= 0) {
            GLuint currentFbo = 0;
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
            if (currentFbo != fbo_id_)
                glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

            // We need to configure how glReadPixels will behave with respect to memory alignment.
            // See http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-an-opengl-hack/
            // and: https://www.khronos.org/opengl/wiki/Common_Mistakes
            glPixelStorei(GL_PACK_ALIGNMENT, 1);				easy3d_debug_log_gl_error;
            glReadPixels(0, 0, width_, height_, GL_DEPTH_COMPONENT, GL_FLOAT, buffer);	easy3d_debug_log_gl_error;

            if (currentFbo != fbo_id_)
                glBindFramebuffer(GL_READ_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;

            // flip vertically
            if (flip_vertically) {
                int half_height = height_ / 2;
                for (int j = 0; j < half_height; ++j) {
                    // get a pointer to the two lines we will swap
                    float* row1 = &(buffer[0]) + j * width_;
                    float* row2 = &(buffer[0]) + (height_ - 1 - j) * width_;
                    // swap for each point on line
                    for (int i = 0; i < width_; ++i) {
                        std::swap(row1[i], row2[i]);
                    }
                }
            }
        }

        else {
            const_cast<FramebufferObject*>(this)->_prepare_resolve_fbo();
            blit_framebuffer(const_cast<FramebufferObject*>(resolved_fbo_), this, GL_DEPTH_BUFFER_BIT);	easy3d_debug_log_gl_error;
            resolved_fbo_->read_depth(buffer, flip_vertically);
        }

        return true;
    }


    // read the depth data of the framebuffer into a specified buffer.
    bool FramebufferObject::read_depth(std::vector<float>& buffer, bool flip_vertically /* = true */) const {
        buffer.resize(width_ * height_);
        return read_depth(buffer.data(), flip_vertically);
    }


    bool FramebufferObject::snapshot_depth(const std::string& file_name) const {
        std::vector<float> depths;
        bool got = read_depth(depths, true);
        if (!got)
            return false;

        // convert the depth values to unsigned char RGB values
        std::vector<unsigned char> bits(depths.size() * 3);
        for (std::size_t i = 0; i < depths.size(); ++i) {
            bits[i * 3] = bits[i * 3 + 1] = bits[i * 3 + 2] = static_cast<unsigned char>(depths[i] * 255);
        }

        const std::string& ext = file_system::extension(file_name, true);
        if (ext == "ppm")
            return io::save_ppm(file_name, bits, width_, height_);
        else
            return ImageIO::save(file_name, bits, width_, height_, 3);
    }


    bool FramebufferObject::read_color(unsigned char rgba[4], int x, int y, int index /* = 0*/) const {
        if (!has_color_attachment(index)) {
            LOG(ERROR) << "color attachment " << index << " does not exist";
            return false;
        }

        glFinish();
        if (samples_ <= 0) {
            GLuint currentFbo = 0;
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
            if (currentFbo != fbo_id_)
                glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

            //////////////////////////////////////////////////////////////////////////

            const_cast<FramebufferObject*>(this)->activate_read_buffer(index);

            // We need to configure how glReadPixels will behave with respect to memory alignment.
            // See http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-an-opengl-hack/
            // and: https://www.khronos.org/opengl/wiki/Common_Mistakes
            glPixelStorei(GL_PACK_ALIGNMENT, 1);				easy3d_debug_log_gl_error;
            glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);	easy3d_debug_log_gl_error;

            if (currentFbo != fbo_id_)
                glBindFramebuffer(GL_READ_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;
        }

        else {
            const_cast<FramebufferObject*>(this)->_prepare_resolve_fbo();
            blit_framebuffer(const_cast<FramebufferObject*>(resolved_fbo_), this, index, index, GL_COLOR_BUFFER_BIT);	easy3d_debug_log_gl_error;
            resolved_fbo_->read_color(rgba, x, y, index);
        }

        return true;
    }


    // read the depth at pixel (x, y).
    bool FramebufferObject::read_depth(float& depth, int x, int y) const {
        if (!has_depth_attachment()) {
            LOG(ERROR) << "depth attachment does not exist";
            return false;
        }

        glFinish();
        if (samples_ <= 0) {
            GLuint currentFbo = 0;
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
            if (currentFbo != fbo_id_)
                glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

            // We need to configure how glReadPixels will behave with respect to memory alignment.
            // See http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-an-opengl-hack/
            // and: https://www.khronos.org/opengl/wiki/Common_Mistakes
            glPixelStorei(GL_PACK_ALIGNMENT, 1);				easy3d_debug_log_gl_error;
            glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);	easy3d_debug_log_gl_error;

            if (currentFbo != fbo_id_)
                glBindFramebuffer(GL_READ_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;
        }

        else {
            const_cast<FramebufferObject*>(this)->_prepare_resolve_fbo();
            blit_framebuffer(const_cast<FramebufferObject*>(resolved_fbo_), this, GL_DEPTH_BUFFER_BIT);	easy3d_debug_log_gl_error;
            resolved_fbo_->read_depth(depth, x, y);
        }

        return true;
    }


    // the whole sized buffer
    void FramebufferObject::blit_framebuffer(
        FramebufferObject* target,
        const FramebufferObject* source,
        GLbitfield buffers,
        GLenum filter /* = GL_NEAREST*/)
    {
        blit_framebuffer(target, source, 0, 0, buffers, filter);
    }


    // the whole sized buffer
    void FramebufferObject::blit_framebuffer(
        FramebufferObject* target,
        const FramebufferObject* source,
        int target_color_attachment_index, int source_color_attachment_index,
        GLbitfield buffers,
        GLenum filter /* = GL_NEAREST*/)
    {
        blit_framebuffer(
            target, 0, 0, target->width(), target->height(),
            source, 0, 0, source->width(), source->height(),
            target_color_attachment_index, source_color_attachment_index,
            buffers, filter
        );
    }


    // a specified region
    void FramebufferObject::blit_framebuffer(
        FramebufferObject* target, int tx0, int ty0, int tx1, int ty1,
        const FramebufferObject* source, int sx0, int sy0, int sx1, int sy1,
        GLbitfield buffers,
        GLenum filter /* = GL_NEAREST*/)
    {
        blit_framebuffer(
            target, tx0, ty0, tx1, ty1,
            source, sx0, sy0, sx1, sy1,
            0, 0, buffers, filter
        );
    }

    // a specified region
    void FramebufferObject::blit_framebuffer(
        FramebufferObject* target, int tx0, int ty0, int tx1, int ty1,
        const FramebufferObject* source, int sx0, int sy0, int sx1, int sy1,
        int target_color_attachment_index, int source_color_attachment_index,
        GLbitfield buffers,
        GLenum filter /* = GL_NEAREST*/)
    {
        if (!source->is_valid()) {
            LOG(ERROR) << "source framebuffer not valid";
            return;
        }

        if (!target->is_valid()) {
            LOG(ERROR) << "target framebuffer not valid";
            return;
        }

//        if (!GLEW_EXT_framebuffer_blit) {
//            LOG(ERROR) << "FramebufferBlit not supported";
//            return;
//        }

        bool blit_color = false;

        // check if source and target FBOs both have the required buffer(s)

        // color
        if (buffers == GL_COLOR_BUFFER_BIT ||
            buffers == (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) ||
            buffers == (GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT) ||
            buffers == (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)
            )
        {
            if (source_color_attachment_index >= source->num_color_attachements()) {
                LOG(ERROR) << "source color attachment " << source_color_attachment_index << " does not exist";
                return;
            }
            if (target_color_attachment_index >= target->num_color_attachements()) {
                LOG(ERROR) << "target color attachment " << target_color_attachment_index << " does not exist";
                return;
            }

            blit_color = true;
        }
        // depth
        else if (buffers == GL_DEPTH_BUFFER_BIT ||
            buffers == (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT) ||
            buffers == (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT) ||
            buffers == (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)
            )
        {
            if (!source->has_depth_attachment()) {
                LOG(ERROR) << "the source FBO does not have depth attachment";
                return;
            }
            if (!target->has_depth_attachment()) {
                LOG(ERROR) << "the target FBO does not have depth attachment";
                return;
            }
            if (tx1 - tx0 != sx1 - sx0 || ty1 - ty0 != sy1 - sy0) {
                LOG(ERROR) << "source and target FBO regions should have the same size";
                return;
            }

            //For GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT, the filter parameter must be GL_NEAREST.
            if (filter != GL_NEAREST) {
                LOG(ERROR) << "filter must be GL_NEAREST for depth blit";
                filter = GL_NEAREST;
            }
        }
        // stencil
        else if (buffers == GL_STENCIL_BUFFER_BIT ||
            buffers == (GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT) ||
            buffers == (GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) ||
            buffers == (GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
            )
        {
            if (!source->has_stencil()) {
                LOG(ERROR) << "the source FBO doesn't have a stencil buffer";
                return;
            }
            if (!target->has_stencil()) {
                LOG(ERROR) << "the target FBO doesn't have a stencil buffer";
                return;
            }
            if (tx1 - tx0 != sx1 - sx0 || ty1 - ty0 != sy1 - sy0) {
                LOG(ERROR) << "source and target FBO regions should have the same size";
                return;
            }

            //For GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT, the filter parameter must be GL_NEAREST.
            if (filter != GL_NEAREST) {
                LOG(ERROR) << "filter must be GL_NEAREST for stencil blit";
                filter = GL_NEAREST;
            }
        }

        // The sizes must also be the same if any of the framebuffer objects are multisample framebuffers.
        if (source->samaples() > 0 || target->samaples() > 0) {
            if (tx1 - tx0 != sx1 - sx0 || ty1 - ty0 != sy1 - sy0) {
                LOG(ERROR) << "source and target FBO regions should have the same size";
                return;
            }
        }

        GLuint prevReadFbo, prevDrawFbo;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&prevReadFbo);	easy3d_debug_log_gl_error;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&prevDrawFbo);	easy3d_debug_log_gl_error;

        // Bind the source FBO for reading
        glBindFramebuffer(GL_READ_FRAMEBUFFER, source ? source->handle() : prevReadFbo);	easy3d_debug_log_gl_error;
        // Bind the target FBO for reading
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target ? target->handle() : prevDrawFbo);	easy3d_debug_log_gl_error;

        if (blit_color) {
            glReadBuffer(GL_COLOR_ATTACHMENT0 + source_color_attachment_index);	easy3d_debug_log_gl_error;
            if (target) {
                GLenum drawBuf = GL_COLOR_ATTACHMENT0 + target_color_attachment_index;
                glDrawBuffers(1, &drawBuf);		easy3d_debug_log_gl_error;
                // or
                //glDrawBuffer(GL_COLOR_ATTACHMENT0 + target_color_attachment_index);	easy3d_debug_log_gl_error;
            }
        }

        glBlitFramebuffer(sx0, sy0, sx1, sy1, tx0, ty0, tx1, ty1, buffers, filter);			easy3d_debug_log_gl_error;

        // restore both READ and DRAW
        glBindFramebuffer(GL_READ_FRAMEBUFFER, prevReadFbo); 	easy3d_debug_log_gl_error;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDrawFbo); 	easy3d_debug_log_gl_error;
    }


    bool FramebufferObject::copy_color_to_texture(
        GLuint& texture_handle,
        unsigned int index /* = 0 */,
        int internalFormat /* = GL_RGBA8 */,
        GLenum format /* = GL_RGBA */,
        GLenum type /* = GL_UNSIGNED_BYTE */,
        GLenum filter /* = GL_NEAREST*/)
    {
        if (!has_color_attachment(index)) {
            LOG(ERROR) << "color attachment " << index << " does not exist";
            return false;
        }

        if (samaples() <= 0) {
            GLuint currentFbo = 0;
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
            if (currentFbo != fbo_id_)
                glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

            //////////////////////////////////////////////////////////////////////////

            if (!glIsTexture(texture_handle)) {
                glGenTextures(1, &texture_handle);				easy3d_debug_log_gl_error;
                glBindTexture(texture_target_, texture_handle);	easy3d_debug_log_gl_error;
                glTexParameteri(texture_target_, GL_TEXTURE_MIN_FILTER, filter);		easy3d_debug_log_gl_error;
                glTexParameteri(texture_target_, GL_TEXTURE_MAG_FILTER, filter);		easy3d_debug_log_gl_error;
                glTexParameteri(texture_target_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	easy3d_debug_log_gl_error;
                glTexParameteri(texture_target_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	easy3d_debug_log_gl_error;
                glTexImage2D(texture_target_, 0, internalFormat, width_, height_, 0, format, type, nullptr);	easy3d_debug_log_gl_error;
            }
            else
                glBindTexture(texture_target_, texture_handle);	easy3d_debug_log_gl_error;

            // if the format has changed, you have to call glTexImage2D() again.
            // glTexImage2D(texture_target_, 0, internalFormat, width_, height_, 0, format, type, nullptr);	easy3d_debug_log_gl_error;
            glCopyTexSubImage2D(texture_target_, 0, 0, 0, 0, 0, width_, height_);		easy3d_debug_log_gl_error;

            glBindTexture(texture_target_, 0);	easy3d_debug_log_gl_error;

            //////////////////////////////////////////////////////////////////////////

            if (currentFbo != fbo_id_)
                glBindFramebuffer(GL_READ_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;

            return true;
        }
        else {
            _prepare_resolve_fbo();
            return resolved_fbo_->copy_color_to_texture(texture_handle, index, internalFormat, format, type, filter);
        }
    }


    bool FramebufferObject::copy_depth_to_texture(
        GLuint& texture_handle,
        unsigned int internal_format /* = GL_DEPTH24_STENCIL8 */,
        GLenum filter /* = GL_NEAREST */ )
    {
        if (!has_depth_attachment()) {
            LOG(ERROR) << "depth attachment does not exist";
            return false;
        }

        if (samaples() <= 0) {
            GLuint currentFbo = 0;
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&currentFbo);	easy3d_debug_log_gl_error;
            if (currentFbo != fbo_id_)
                glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id_);		easy3d_debug_log_gl_error;

            //----------------------------------------------------------------------------------------------

            bool combined_stencil = (internal_format == GL_DEPTH24_STENCIL8) || (internal_format == GL_DEPTH32F_STENCIL8);
            GLenum format = combined_stencil ? GL_DEPTH_STENCIL : GL_DEPTH_COMPONENT;
            bool float_type = (internal_format == GL_DEPTH_COMPONENT32F) || (internal_format == GL_DEPTH32F_STENCIL8);
            GLenum type;
            if (combined_stencil)
                type = float_type ? GL_FLOAT_32_UNSIGNED_INT_24_8_REV : GL_UNSIGNED_INT_24_8;
            else
                type = float_type ? GL_FLOAT : GL_UNSIGNED_INT;

            //----------------------------------------------------------------------------------------------

            if (!glIsTexture(texture_handle)) {
                glGenTextures(1, &texture_handle);				easy3d_debug_log_gl_error;
                glBindTexture(texture_target_, texture_handle);	easy3d_debug_log_gl_error;

                glTexParameteri(texture_target_, GL_TEXTURE_MIN_FILTER, filter);		easy3d_debug_log_gl_error;
                glTexParameteri(texture_target_, GL_TEXTURE_MAG_FILTER, filter);		easy3d_debug_log_gl_error;
                glTexParameteri(texture_target_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	easy3d_debug_log_gl_error;
                glTexParameteri(texture_target_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	easy3d_debug_log_gl_error;
                //glTexParameteri(texture_target_, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);	easy3d_debug_log_gl_error;
                glTexParameteri(texture_target_, GL_TEXTURE_COMPARE_MODE, GL_NONE);		easy3d_debug_log_gl_error;

                glTexImage2D(texture_target_, 0, internal_format, width_, height_, 0, format, type, nullptr);	easy3d_debug_log_gl_error;
            }
            else
                glBindTexture(texture_target_, texture_handle);	easy3d_debug_log_gl_error;

            // if the format has changed, you have to call glTexImage2D() again.
            //glTexImage2D(texture_target_, 0, internal_format, width_, height_, 0, format, type, nullptr);	easy3d_debug_log_gl_error;
            glCopyTexSubImage2D(texture_target_, 0, 0, 0, 0, 0, width_, height_);		easy3d_debug_log_gl_error;

            glBindTexture(texture_target_, 0);	easy3d_debug_log_gl_error;

            //////////////////////////////////////////////////////////////////////////

            if (currentFbo != fbo_id_)
                glBindFramebuffer(GL_READ_FRAMEBUFFER, currentFbo);		easy3d_debug_log_gl_error;

            return true;
        }
        else {
            _prepare_resolve_fbo();
            return resolved_fbo_->copy_depth_to_texture(texture_handle, internal_format, filter);
        }
    }

} // namespace easy3d
