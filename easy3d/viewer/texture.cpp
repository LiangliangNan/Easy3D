/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: a lightweight, easy-to-use, and efficient C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include <easy3d/viewer/texture.h>
#include <easy3d/util/file.h>
#include <easy3d/viewer/opengl_error.h>

#define STB_IMAGE_IMPLEMENTATION
#include <3rd_party/stb/stb_image.h>

#include <iostream>


namespace easy3d {

    Texture::Texture()
        : id_(0)
        , dimension_(0)
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
        if (!file::is_file(file_name)) {
            std::cerr << "file \'" << file_name << "\' does not exist" << std::endl;
            return nullptr;
        }

        int width, height, comp;
        const auto data = stbi_load(file_name.data(), &width, &height, &comp, STBI_rgb);
        if (data) {
			GLenum internal_format, format;
			switch (comp)
			{
			case STBI_rgb_alpha:  internal_format = GL_RGBA8; format = GL_RGBA; break;
			case STBI_rgb:		  internal_format = GL_RGB8; format = GL_RGB; break;	
			case STBI_grey:		  internal_format = GL_R8; format = GL_RED; break;		
			case STBI_grey_alpha: internal_format = GL_RG8; format = GL_RG; break;
			default: throw std::runtime_error("invalid format");
			}

            GLuint tex = 0;
            glCreateTextures(GL_TEXTURE_2D, 1, &tex);	easy3d_debug_gl_error;
            glTextureStorage2D(tex, 1, internal_format, width, height);	easy3d_debug_gl_error;

            glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, filter);	easy3d_debug_gl_error;
            glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, filter);	easy3d_debug_gl_error;
            glTextureParameteri(tex, GL_TEXTURE_WRAP_S, wrap);	easy3d_debug_gl_error;
            glTextureParameteri(tex, GL_TEXTURE_WRAP_T, wrap);	easy3d_debug_gl_error;
            glTextureParameteri(tex, GL_TEXTURE_WRAP_R, wrap);	easy3d_debug_gl_error;

			if (data) {
				glTextureSubImage2D(tex, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);	easy3d_debug_gl_error;
			}

            Texture* result = new Texture;
            result->sizes_[0] = width;
            result->sizes_[1] = height;
            result->sizes_[2] = comp;
            result->id_ = tex;
            stbi_image_free(data);
            return result;
        }
        return nullptr;
    }


    void Texture::bind() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, id_);

        {
            //glEnable(GL_TEXTURE_2D);		// deprecated function since 3.0 version of OpenGL
            //glEnable(GL_ALPHA_TEST);		// deprecated function since 3.1 version of OpenGL
            //glAlphaFunc(GL_GREATER, 0);	// deprecated function since 3.1 version of OpenGL
        }
    }


    void Texture::unbind() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);

        {
            //glDisable(GL_ALPHA_TEST);		// deprecated function since 3.1 version of OpenGL
            //glDisable(GL_TEXTURE_2D);		// deprecated function since 3.0 version of OpenGL
        }
    }

} // namespace easy3d

