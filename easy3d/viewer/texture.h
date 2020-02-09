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

#ifndef EASY3D_OPENGL_TEXTURE_H
#define EASY3D_OPENGL_TEXTURE_H


#include <string>

#include <easy3d/viewer/opengl.h>


namespace easy3d {

    // currently only TEXTURE_2D is supported
    class Texture
    {
    public:
        // wrap: GL_CLAMP_TO_EDGE, GL_REPEAT
        // filter: GL_NEAREST, GL_LINEAR
        static Texture* create(const std::string& image_file, GLenum wrap = GL_CLAMP_TO_EDGE, GLenum filter = GL_LINEAR);
        ~Texture();

        GLuint id() const { return id_; }

        void bind(int unit = 0);
        void release();

        int width() const { return sizes_[0]; }
        int height() const { return sizes_[1]; }
        int channels() const { return sizes_[2]; }

        const std::string& file_name() const { return file_name_; }

    private:
        GLuint	id_;
        int sizes_[3];

        std::string file_name_;

    private:
        //can only be created by using the create() function
        Texture();

        //copying disabled
        Texture(const Texture&);
        Texture& operator=(const Texture&);
    };

} // namespace easy3d


#endif  // EASY3D_OPENGL_TEXTURE_H

