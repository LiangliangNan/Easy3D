//
// Copyright (c) 2011 Andreas Krinke andreas.krinke@gmx.de
// Copyright (c) 2009 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef FONTSTASH_H
#define FONTSTASH_H

#include <easy3d/viewer/opengl.h>

#define STH_ESUCCESS 0
// error opening file
#define STH_EFILEIO -1
// error initializing truetype font
#define STH_ETTFINIT -2
// invalid argument
#define STH_EINVAL -3
// not enough memory
#define STH_ENOMEM -4


/**
 * Example:
 *  ------------------------------------------------------------------------------------
 *  // create a font stash with a maximum texture size of 512 x 512
 *  struct sth_stash *stash = sth_create(512, 512);
 *  // load true type font
 *  const std::string font_file = resource::directory() + "/fonts/zachary.ttf";
 *  const int droid = sth_add_font(stash, font_file.c_str());
 *  ------------------------------------------------------------------------------------
 *  glEnable(GL_BLEND);
 *  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 *  // draw text during your OpenGL render loop
 *  sth_begin_draw(stash);
 *  // position: (x, y); font size: 24
 *  sth_draw_text(stash, droid, 24, x, y, "Hello world! ", &x);
 *  // now, the float x contains the x position of the next char
 *  sth_end_draw(stash);
 *  glDisable(GL_BLEND);
 *  ------------------------------------------------------------------------------------
 *  // clean
 */

struct sth_stash* sth_create(int cachew, int cacheh, bool mipmap = true);

int sth_add_font(struct sth_stash* stash, const char* path);
int sth_add_font_from_memory(struct sth_stash* stash, unsigned char* buffer);

int sth_add_bitmap_font(struct sth_stash* stash, int ascent, int descent, int line_gap);
int sth_add_glyph_for_codepoint(struct sth_stash* stash, int idx, GLuint id, unsigned int codepoint,
                                short size, short base, int x, int y, int w, int h,
                                float xoffset, float yoffset, float xadvance);
int sth_add_glyph_for_char(struct sth_stash* stash, int idx, GLuint id, const char* s,
                           short size, short base, int x, int y, int w, int h,
                           float xoffset, float yoffset, float xadvance);

void sth_begin_draw(struct sth_stash* stash);
void sth_end_draw(struct sth_stash* stash);

void sth_draw_text(struct sth_stash* stash,
				   int idx, float size,
				   float x, float y, const char* string, float* dx);

void sth_dim_text(struct sth_stash* stash, int idx, float size, const char* string,
				  float* minx, float* miny, float* maxx, float* maxy);

void sth_vmetrics(struct sth_stash* stash,
				  int idx, float size,
				  float* ascender, float* descender, float * lineh);

void sth_delete(struct sth_stash* stash);

#endif // FONTSTASH_H
