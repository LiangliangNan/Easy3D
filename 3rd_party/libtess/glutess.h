/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice including the dates of first publication and
 * either this permission notice or a reference to
 * http://oss.sgi.com/projects/FreeB/
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * SILICON GRAPHICS, INC. BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Silicon Graphics, Inc.
 * shall not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization from
 * Silicon Graphics, Inc.
 */

/* ftg.mlabbe -- I renamed this file from glu.h to glutess.h in order to
   avoid any future situations where a compile environment puts another glu.h
   in the search path ahead of this file.

   It is also worth noting that this is a rough rip of the glu tesselation
   functionality, and as such, not all of the things declared below actually
   exist.  It is possible to get link errors, so check if a function exists
   before counting on your grand designs.
   */

#ifndef __glu_h__
#define __glu_h__

typedef int GLint;
typedef unsigned int GLenum;

// redefined to match glew.h's -ml
typedef int GLsizei;
//typedef unsigned int GLsizei;
typedef float GLfloat;
typedef double GLdouble;
typedef unsigned char GLubyte;

// redefined to match glew.h's
typedef unsigned char GLboolean;
//typedef int GLboolean;
typedef void GLvoid;

#define GL_FALSE 0
#define GL_TRUE 1
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006

// #if (defined(_MSC_VER) || defined(__MINGW32__)) && defined(BUILD_GLU32)
// # undef GLAPI
// # define __declspec(dllexport)
// #elif (defined(_MSC_VER) || defined(__MINGW32__)) && defined(_DLL)
// /* tag specifying we're building for DLL runtime support */
// # undef GLAPI
// # define __declspec(dllimport)
// #elif !defined(GLAPI)
// /* for use with static link lib build of Win32 edition only */
// # define extern
// #endif /* _STATIC_MESA support */

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************/

/* ErrorCode */
#define GLU_INVALID_ENUM                   100900
#define GLU_INVALID_VALUE                  100901
#define GLU_OUT_OF_MEMORY                  100902

/* TessCallback */
#define GLU_TESS_BEGIN                     100100
#define GLU_TESS_VERTEX                    100101
#define GLU_TESS_END                       100102
#define GLU_TESS_ERROR                     100103
#define GLU_TESS_EDGE_FLAG                 100104
#define GLU_TESS_COMBINE                   100105
#define GLU_TESS_BEGIN_DATA                100106
#define GLU_TESS_VERTEX_DATA               100107
#define GLU_TESS_END_DATA                  100108
#define GLU_TESS_ERROR_DATA                100109
#define GLU_TESS_EDGE_FLAG_DATA            100110
#define GLU_TESS_COMBINE_DATA              100111

/* TessProperty */
#define GLU_TESS_WINDING_RULE              100140
#define GLU_TESS_BOUNDARY_ONLY             100141
#define GLU_TESS_TOLERANCE                 100142

/* TessError */
#define GLU_TESS_MISSING_BEGIN_POLYGON     100151
#define GLU_TESS_MISSING_BEGIN_CONTOUR     100152
#define GLU_TESS_MISSING_END_POLYGON       100153
#define GLU_TESS_MISSING_END_CONTOUR       100154
#define GLU_TESS_COORD_TOO_LARGE           100155
#define GLU_TESS_NEED_COMBINE_CALLBACK     100156

/* TessWinding */
#define GLU_TESS_WINDING_ODD               100130
#define GLU_TESS_WINDING_NONZERO           100131
#define GLU_TESS_WINDING_POSITIVE          100132
#define GLU_TESS_WINDING_NEGATIVE          100133
#define GLU_TESS_WINDING_ABS_GEQ_TWO       100134

/*************************************************************/

#ifdef __cplusplus
class GLUtesselator;
#else
typedef struct GLUtesselator GLUtesselator;
#endif


#define GLU_TESS_MAX_COORD 1.0e150

/* Internal convenience typedefs */
typedef void (_GLUfuncptr)(void);

void DeleteTess (GLUtesselator* tess);
GLUtesselator* NewTess (void);
void TessBeginContour (GLUtesselator* tess);
void TessBeginPolygon (GLUtesselator* tess, GLvoid* data);
void TessCallback (GLUtesselator* tess, GLenum which, _GLUfuncptr CallBackFunc);
void TessEndContour (GLUtesselator* tess);
void TessEndPolygon (GLUtesselator* tess);
void TessNormal (GLUtesselator* tess, GLdouble valueX, GLdouble valueY, GLdouble valueZ);
void TessProperty (GLUtesselator* tess, GLenum which, GLdouble data);
void TessVertex (GLUtesselator* tess, GLdouble *location, GLvoid* data);

#ifdef __cplusplus
}
#endif

#endif /* __glu_h__ */
