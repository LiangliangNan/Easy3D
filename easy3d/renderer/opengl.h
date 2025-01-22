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

#ifndef EASY3D_RENDERER_OPENGL_H
#define EASY3D_RENDERER_OPENGL_H


/**
 * Make it easier to include the OpenGL header file.
 *
 * Always include this file, i.e.,
 *      #include <easy3d/opengl/opengl.h>
 * instead of any of the following:
 *      #include <OpenGL/gl.h>
 *      #include <GL/gl.h>
 *      #include <GL/glew.h>
 */

#include <3rd_party/glew/include/GL/glew.h>

#if defined(_WIN32) || defined(MSVC)
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX            1
#include <Windows.h>
#endif


#define BACKUP_OPENGL_STATES                                                            \
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);                      \
    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);                                \
    GLenum last_blend_src; glGetIntegerv(GL_BLEND_SRC, (GLint*)&last_blend_src);        \
    GLenum last_blend_dst; glGetIntegerv(GL_BLEND_DST, (GLint*)&last_blend_dst);

#define RESTORE_OPENGL_STATES                                                           \
    if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);                \
    if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST); \
    glBlendFunc(last_blend_src, last_blend_dst);


#endif // EASY3D_RENDERER_OPENGL_H
