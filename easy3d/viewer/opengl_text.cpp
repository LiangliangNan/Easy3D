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


#include <easy3d/viewer/opengl_text.h>
#include <easy3d/core/types.h>


namespace easy3d {

    namespace details {

        // Copyright (c) 2020 Liangliang Nan liangliang.nan@gmail.com
        //     The main changes are that all fixed pipeline rendering code has been
        //     replaced by rendering using shaders.
        //
        // Copyright (c) 2011 Andreas Krinke andreas.krinke@gmx.de
        //
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
         *  // cleaning
         *  sth_delete(stash);
         */
        struct sth_stash *sth_create(int cachew, int cacheh, bool mipmap = true);

        int sth_add_font(struct sth_stash *stash, const char *path);

        int sth_add_font_from_memory(struct sth_stash *stash, unsigned char *buffer);

        int sth_add_bitmap_font(struct sth_stash *stash, int ascent, int descent, int line_gap);

        int sth_add_glyph_for_codepoint(struct sth_stash *stash, int idx, unsigned int id, unsigned int codepoint,
                                        short size, short base, int x, int y, int w, int h,
                                        float xoffset, float yoffset, float xadvance);

        int sth_add_glyph_for_char(struct sth_stash *stash, int idx, unsigned int id, const char *s,
                                   short size, short base, int x, int y, int w, int h,
                                   float xoffset, float yoffset, float xadvance);

        void sth_begin_draw(struct sth_stash *stash);

        void sth_end_draw(struct sth_stash *stash);

        void sth_draw_text(struct sth_stash *stash,
                           int idx, float size,
                           float x, float y, const char *string, float *dx);

        void sth_dim_text(struct sth_stash *stash, int idx, float size, const char *string,
                          float *minx, float *miny, float *maxx, float *maxy);

        void sth_vmetrics(struct sth_stash *stash,
                          int idx, float size,
                          float *ascender, float *descender, float *lineh);

        void sth_delete(struct sth_stash *stash);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Here starts the implementation of fontstash

#include <easy3d/viewer/opengl.h>
#include <easy3d/viewer/opengl_error.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/shader_manager.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <3rd_party/stb/stb_truetype.h>


namespace easy3d {

    namespace details {

#define HASH_LUT_SIZE 256
#define MAX_ROWS 128
#define VERT_COUNT (6*128 * 100)  /* oriol! x 100 to avoid repeated flush_draw() calls while drawing */

#define TTFONT_FILE 1
#define TTFONT_MEM  2
#define BMFONT      3

        static int idx = 1;

        static unsigned int hashint(unsigned int a)
        {
            a += ~(a<<15);
            a ^=  (a>>10);
            a +=  (a<<3);
            a ^=  (a>>6);
            a += ~(a<<11);
            a ^=  (a>>16);
            return a;
        }


        struct sth_quad
        {
            float x0,y0,s0,t0;
            float x1,y1,s1,t1;
        };

        struct sth_row
        {
            short x,y,h;
        };

        struct sth_glyph
        {
            unsigned int codepoint;
            short size;
            struct sth_texture* texture;
            int x0,y0,x1,y1;
            float xadv,xoff,yoff;
            int next;
        };

        struct sth_font
        {
            int idx;
            int type;
            stbtt_fontinfo font;
            unsigned char* data;
            struct sth_glyph* glyphs;
            int lut[HASH_LUT_SIZE];
            int nglyphs;
            float ascender;
            float descender;
            float lineh;
            struct sth_font* next;
        };

        struct sth_texture
        {
            GLuint id;
            // TODO: replace rows with pointer
            struct sth_row rows[MAX_ROWS];
            int nrows;
            float verts[4*VERT_COUNT];
            int nverts;
            struct sth_texture* next;
        };

        struct sth_stash
        {
            int tw,th;
            float itw,ith;
            GLubyte *empty_data;
            struct sth_texture* tt_textures;
            struct sth_texture* bm_textures;
            struct sth_font* fonts;
            int drawing;
            int mipmap; //Liangliang: optional mipmap generation to each char
        };



// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

        static const unsigned char utf8d[] = {
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
                7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
                8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
                0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
                0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
                0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
                1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
                1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
                1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
        };

        static unsigned int decutf8(unsigned int* state, unsigned int* codep, unsigned int byte)
        {
            unsigned int type = utf8d[byte];
            *codep = (*state != UTF8_ACCEPT) ?
                     (byte & 0x3fu) | (*codep << 6) :
                     (0xff >> type) & (byte);
            *state = utf8d[256 + *state*16 + type];
            return *state;
        }


        struct sth_stash* sth_create(int cachew, int cacheh, bool mipmap)
        {
            struct sth_stash* stash = NULL;
            GLubyte* empty_data = NULL;
            struct sth_texture* texture = NULL;

            // Allocate memory for the font stash.
            stash = (struct sth_stash*)malloc(sizeof(struct sth_stash));
            if (stash == NULL) goto error;
            memset(stash,0,sizeof(struct sth_stash));

            // Create data for clearing the textures
            empty_data = (GLubyte*)malloc(cachew * cacheh);
            if (empty_data == NULL) goto error;
            memset(empty_data, 0, cachew * cacheh);

            // Allocate memory for the first texture
            texture = (struct sth_texture*)malloc(sizeof(struct sth_texture));
            if (texture == NULL) goto error;
            memset(texture,0,sizeof(struct sth_texture));

            // Create first texture for the cache.
            stash->tw = cachew;
            stash->th = cacheh;
            stash->itw = 1.0f/cachew;
            stash->ith = 1.0f/cacheh;
            stash->empty_data = empty_data;
            stash->tt_textures = texture;
            glGenTextures(1, &texture->id); easy3d_debug_log_gl_error;
            if (!texture->id) goto error;
            glBindTexture(GL_TEXTURE_2D, texture->id);    easy3d_debug_log_gl_error;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);    easy3d_debug_log_gl_error;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    easy3d_debug_log_gl_error;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, cachew, cacheh, 0, GL_RED, GL_UNSIGNED_BYTE, empty_data);    easy3d_debug_log_gl_error;
            glBindTexture(GL_TEXTURE_2D, 0);    easy3d_debug_log_gl_error;

            return stash;

            error:
            if (stash != NULL)
                free(stash);
            if (empty_data != NULL)
                free(empty_data);
            if (texture != NULL)
                free(texture);
            return NULL;
        }

        int sth_add_font_from_memory(struct sth_stash* stash, unsigned char* buffer)
        {
            int ret, i, ascent, descent, fh, lineGap;
            struct sth_font* fnt = NULL;

            fnt = (struct sth_font*)malloc(sizeof(struct sth_font));
            if (fnt == NULL)
            {
                ret = STH_ENOMEM;
                goto error;
            }
            memset(fnt,0,sizeof(struct sth_font));

            // Init hash lookup.
            for (i = 0; i < HASH_LUT_SIZE; ++i) fnt->lut[i] = -1;

            fnt->data = buffer;

            // Init stb_truetype
            if (!stbtt_InitFont(&fnt->font, fnt->data, 0))
            {
                ret = STH_ETTFINIT;
                goto error;
            }

            // Store normalized line height. The real line height is got
            // by multiplying the lineh by font size.
            stbtt_GetFontVMetrics(&fnt->font, &ascent, &descent, &lineGap);
            fh = ascent - descent;
            fnt->ascender = (float)ascent / (float)fh;
            fnt->descender = (float)descent / (float)fh;
            fnt->lineh = (float)(fh + lineGap) / (float)fh;

            fnt->idx = idx;
            fnt->type = TTFONT_MEM;
            fnt->next = stash->fonts;
            stash->fonts = fnt;

            return idx++;

            error:
            if (fnt) {
                if (fnt->glyphs) free(fnt->glyphs);
                free(fnt);
            }
            return ret;
        }

        int sth_add_font(struct sth_stash* stash, const char* path)
        {
            FILE* fp = 0;
            int ret, datasize;
            unsigned char* data = NULL;
            int idx;

            // Read in the font data.
            fp = fopen(path, "rb");
            if (!fp)
            {
                ret = STH_EFILEIO;
                printf("failed load font: %s\n", path);
                goto error;
            }
            fseek(fp,0,SEEK_END);
            datasize = (int)ftell(fp);
            fseek(fp,0,SEEK_SET);
            data = (unsigned char*)malloc(datasize);
            if (data == NULL)
            {
                ret = STH_ENOMEM;
                goto error;
            }
            fread(data, 1, datasize, fp);
            fclose(fp);
            fp = 0;

            idx = sth_add_font_from_memory(stash, data);
            // Modify type of the loaded font.
            if (idx)
                stash->fonts->type = TTFONT_FILE;
            else
                free(data);

            return idx;

            error:
            if (data) free(data);
            if (fp) fclose(fp);
            return ret;
        }

        int sth_add_bitmap_font(struct sth_stash* stash, int ascent, int descent, int line_gap)
        {
            int ret, i, fh;
            struct sth_font* fnt = NULL;

            fnt = (struct sth_font*)malloc(sizeof(struct sth_font));
            if (fnt == NULL)
            {
                ret = STH_ENOMEM;
                goto error;
            }
            memset(fnt,0,sizeof(struct sth_font));

            // Init hash lookup.
            for (i = 0; i < HASH_LUT_SIZE; ++i) fnt->lut[i] = -1;

            // Store normalized line height. The real line height is got
            // by multiplying the lineh by font size.
            fh = ascent - descent;
            fnt->ascender = (float)ascent / (float)fh;
            fnt->descender = (float)descent / (float)fh;
            fnt->lineh = (float)(fh + line_gap) / (float)fh;

            fnt->idx = idx;
            fnt->type = BMFONT;
            fnt->next = stash->fonts;
            stash->fonts = fnt;

            return idx++;

            error:
            if (fnt) free(fnt);
            return ret;
        }

        int sth_add_glyph_for_codepoint(struct sth_stash* stash,
                                        int idx,
                                        GLuint id,
                                        unsigned int codepoint,
                                        short size, short base,
                                        int x, int y, int w, int h,
                                        float xoffset, float yoffset, float xadvance)
        {
            struct sth_texture* texture = NULL;
            struct sth_font* fnt = NULL;
            struct sth_glyph* glyph = NULL;

            if (stash == NULL)
                return STH_EINVAL;
            texture = stash->bm_textures;
            while (texture != NULL && texture->id != id) texture = texture->next;
            if (texture == NULL)
            {
                // Create new texture
                texture = (struct sth_texture*)malloc(sizeof(struct sth_texture));
                if (texture == NULL)
                    return STH_ENOMEM;
                memset(texture, 0, sizeof(struct sth_texture));
                texture->id = id;
                texture->next = stash->bm_textures;
                stash->bm_textures = texture;
            }

            fnt = stash->fonts;
            while (fnt != NULL && fnt->idx != idx) fnt = fnt->next;
            if (fnt == NULL)
                return STH_EINVAL;
            if (fnt->type != BMFONT)
                return STH_EINVAL;

            // Alloc space for new glyph.
            fnt->nglyphs++;
            fnt->glyphs = (struct sth_glyph *)realloc(fnt->glyphs, fnt->nglyphs*sizeof(struct sth_glyph)); /* @rlyeh: explicit cast needed in C++ */
            if (!fnt->glyphs)
                return STH_ENOMEM;

            // Init glyph.
            glyph = &fnt->glyphs[fnt->nglyphs-1];
            memset(glyph, 0, sizeof(struct sth_glyph));
            glyph->codepoint = codepoint;
            glyph->size = size;
            glyph->texture = texture;
            glyph->x0 = x;
            glyph->y0 = y;
            glyph->x1 = glyph->x0+w;
            glyph->y1 = glyph->y0+h;
            glyph->xoff = xoffset;
            glyph->yoff = yoffset - base;
            glyph->xadv = xadvance;

            // Find code point and size.
            h = hashint(codepoint) & (HASH_LUT_SIZE-1);
            // Insert char to hash lookup.
            glyph->next = fnt->lut[h];
            fnt->lut[h] = fnt->nglyphs-1;

            return STH_ESUCCESS;
        }

        inline int sth_add_glyph_for_char(struct sth_stash* stash,
                                          int idx,
                                          GLuint id,
                                          const char* s,
                                          short size, short base,
                                          int x, int y, int w, int h,
                                          float xoffset, float yoffset, float xadvance)
        {
            unsigned int codepoint;
            unsigned int state = 0;

            for (; *s; ++s)
            {
                if (!decutf8(&state, &codepoint, *(unsigned char*)s))
                    break;
            }
            if (state != UTF8_ACCEPT)
                return STH_EINVAL;

            return sth_add_glyph_for_codepoint(stash, idx, id, codepoint, size, base, x, y, w, h, xoffset, yoffset, xadvance);
        }

        static struct sth_glyph* get_glyph(struct sth_stash* stash, struct sth_font* fnt, unsigned int codepoint, short isize)
        {
            int i,g,advance,lsb,x0,y0,x1,y1,gw,gh;
            float scale;
            struct sth_texture* texture = NULL;
            struct sth_glyph* glyph = NULL;
            unsigned char* bmp = NULL;
            unsigned int h;
            float size = isize/10.0f;
            int rh;
            struct sth_row* br = NULL;

            // Find code point and size.
            h = hashint(codepoint) & (HASH_LUT_SIZE-1);
            i = fnt->lut[h];
            while (i != -1)
            {
                if (fnt->glyphs[i].codepoint == codepoint && (fnt->type == BMFONT || fnt->glyphs[i].size == isize))
                    return &fnt->glyphs[i];
                i = fnt->glyphs[i].next;
            }
            // Could not find glyph.

            // For bitmap fonts: ignore this glyph.
            if (fnt->type == BMFONT)
                return 0;

            // For truetype fonts: create this glyph.
            scale = stbtt_ScaleForPixelHeight(&fnt->font, size);
            g = stbtt_FindGlyphIndex(&fnt->font, codepoint);
            if(!g)
                return 0; /* @rlyeh: glyph not found, ie, arab chars */
            stbtt_GetGlyphHMetrics(&fnt->font, g, &advance, &lsb);
            stbtt_GetGlyphBitmapBox(&fnt->font, g, scale,scale, &x0,&y0,&x1,&y1);
            gw = x1-x0;
            gh = y1-y0;

            // Check if glyph is larger than maximum texture size
            if (gw >= stash->tw || gh >= stash->th)
                return 0;

            // Find texture and row where the glyph can be fit.
            br = NULL;
            rh = (gh+7) & ~7;
            texture = stash->tt_textures;
            while(br == NULL)
            {
                for (i = 0; i < texture->nrows; ++i)
                {
                    if (texture->rows[i].h == rh && texture->rows[i].x+gw+1 <= stash->tw)
                        br = &texture->rows[i];
                }

                // If no row is found, there are 3 possibilities:
                //   - add new row
                //   - try next texture
                //   - create new texture
                if (br == NULL)
                {
                    short py = 0;
                    // Check that there is enough space.
                    if (texture->nrows)
                    {
                        py = texture->rows[texture->nrows-1].y + texture->rows[texture->nrows-1].h+1;
                        if (py+rh > stash->th)
                        {
                            if (texture->next != NULL)
                            {
                                texture = texture->next;
                            }
                            else
                            {
                                // Create new texture
                                texture->next = (struct sth_texture*)malloc(sizeof(struct sth_texture));
                                texture = texture->next;
                                if (texture == NULL) goto error;
                                memset(texture,0,sizeof(struct sth_texture));
                                glGenTextures(1, &texture->id);
                                if (!texture->id) goto error;
                                glBindTexture(GL_TEXTURE_2D, texture->id);
                                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, stash->tw,stash->th, 0, GL_RED, GL_UNSIGNED_BYTE, stash->empty_data); easy3d_debug_log_gl_error;
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); easy3d_debug_log_gl_error;
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); easy3d_debug_log_gl_error;
                                glBindTexture(GL_TEXTURE_2D, 0);
                            }
                            continue;
                        }
                    }
                    // Init and add row
                    br = &texture->rows[texture->nrows];
                    br->x = 0;
                    br->y = py;
                    br->h = rh;
                    texture->nrows++;
                }
            }

            // Alloc space for new glyph.
            fnt->nglyphs++;
            fnt->glyphs = (struct sth_glyph *)realloc(fnt->glyphs, fnt->nglyphs*sizeof(struct sth_glyph)); /* @rlyeh: explicit cast needed in C++ */
            if (!fnt->glyphs)
                return 0;

            // Init glyph.
            glyph = &fnt->glyphs[fnt->nglyphs-1];
            memset(glyph, 0, sizeof(struct sth_glyph));
            glyph->codepoint = codepoint;
            glyph->size = isize;
            glyph->texture = texture;
            glyph->x0 = br->x;
            glyph->y0 = br->y;
            glyph->x1 = glyph->x0+gw;
            glyph->y1 = glyph->y0+gh;
            glyph->xadv = scale * advance;
            glyph->xoff = (float)x0;
            glyph->yoff = (float)y0;
            glyph->next = 0;

            // Advance row location.
            br->x += gw+1;

            // Insert char to hash lookup.
            glyph->next = fnt->lut[h];
            fnt->lut[h] = fnt->nglyphs-1;

            // Rasterize
            bmp = (unsigned char*)malloc(gw*gh);
            if (bmp)
            {
                stbtt_MakeGlyphBitmap(&fnt->font, bmp, gw,gh,gw, scale,scale, g);
                // Update texture
                glBindTexture(GL_TEXTURE_2D, texture->id);
                glPixelStorei(GL_UNPACK_ALIGNMENT,1);
                glTexSubImage2D(GL_TEXTURE_2D, 0, glyph->x0,glyph->y0, gw,gh, GL_RED,GL_UNSIGNED_BYTE,bmp);   easy3d_debug_log_gl_error;
                if(stash->mipmap){
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 8); //TODO check for hw support!
#if defined(__ANDROID__) || defined(TARGET_OPENGLES) || defined(TARGET_RASPBERRY_PI)
                    // OpenGLES 1.0 does not support the following.
#else
//                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.0); //shoot for sharper test
//                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3); // pick mipmap level 7 or lower
//                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.0);
                    glGenerateMipmap(GL_TEXTURE_2D);
#endif
                }
                glBindTexture(GL_TEXTURE_2D, 0);   easy3d_debug_log_gl_error;
                free(bmp);
            }

            return glyph;

            error:
            if (texture)
                free(texture);
            return 0;
        }

        static int get_quad(struct sth_stash* stash, struct sth_font* fnt, struct sth_glyph* glyph, short isize, float* x, float* y, struct sth_quad* q)
        {
            int rx,ry;
            float scale = 1.0f;

            if (fnt->type == BMFONT) scale = isize/(glyph->size*10.0f);

            rx = floorf(*x + scale * glyph->xoff);
            ry = floorf(*y - scale * glyph->yoff);

            q->x0 = rx;
            q->y0 = ry;
            q->x1 = rx + scale * (glyph->x1 - glyph->x0);
            q->y1 = ry - scale * (glyph->y1 - glyph->y0);

            q->s0 = (glyph->x0) * stash->itw;
            q->t0 = (glyph->y0) * stash->ith;
            q->s1 = (glyph->x1) * stash->itw;
            q->t1 = (glyph->y1) * stash->ith;

            *x += scale * glyph->xadv;

            return 1;
        }

        static float* setv(float* v, float x, float y, float s, float t)
        {
            v[0] = x;
            v[1] = y;
            v[2] = s;
            v[3] = t;
            return v+4;
        }

        static void flush_draw(struct sth_stash* stash)
        {
            struct sth_texture* texture = stash->tt_textures;
            short tt = 1;
            while (texture)
            {
                if (texture->nverts > 0)
                {
                    int viewport[4];
                    glGetIntegerv(GL_VIEWPORT, viewport);
                    int w = viewport[2];
                    int h = viewport[3];

                    std::vector<vec3> verts(texture->nverts);
                    std::vector<vec2> texcoords(texture->nverts);
                    for (int i=0;i<texture->nverts; ++i) {
                        float x = texture->verts[i * 4];
                        float y = texture->verts[i * 4 + 1];
                        verts[i] = vec3(2.0f * x / w - 1.0f, 2.0f * y / h - 1.0f, -0.9f);
                        texcoords[i] = vec2(texture->verts + i * 4 + 2);
                    }

                    std::vector<unsigned int> indices; // = { 0, 1, 2, 0, 2, 3 };
                    for (int j = 0; j < texture->nverts/4; ++j) {
                        indices.push_back(j * 4);
                        indices.push_back(j * 4 + 1);
                        indices.push_back(j * 4 + 2);
                        indices.push_back(j * 4);
                        indices.push_back(j * 4 + 2);
                        indices.push_back(j * 4 + 3);
                    }

                    TrianglesDrawable drawable;
                    drawable.update_vertex_buffer(verts);
                    drawable.update_texcoord_buffer(texcoords);
                    drawable.update_index_buffer(indices);

                    const std::string name = "text/text";
                    auto program = ShaderManager::get_program(name);
                    if (!program) {
                        std::vector<ShaderProgram::Attribute> attributes = {
                                ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                                ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "tex_coord")
                        };
                        program = ShaderManager::create_program_from_files(name, attributes);
                    }
                    if (!program) {
                        LOG_FIRST_N(ERROR, 1) << "shader doesn't exist: " << name << " (this is the first record)";
                        return;
                    }

                    program->bind();
                    program->bind_texture("textureID", texture->id, 0);
                    drawable.gl_draw(false);
                    program->release_texture();
                    program->release();

                    texture->nverts = 0;
                }
                texture = texture->next;
                if (!texture && tt)
                {
                    texture = stash->bm_textures;
                    tt = 0;
                }
            }
        }

        void sth_begin_draw(struct sth_stash* stash)
        {
            if (stash == NULL)
                return;
            if (stash->drawing)
                flush_draw(stash);
            stash->drawing = 1;
        }

        void sth_end_draw(struct sth_stash* stash)
        {
            if (stash == NULL)
                return;
            if (!stash->drawing)
                return;

            flush_draw(stash);
            stash->drawing = 0;
        }

        void sth_draw_text(struct sth_stash* stash,
                           int idx, float size,
                           float x, float y,
                           const char* s, float* dx)
        {
            unsigned int codepoint;
            struct sth_glyph* glyph = NULL;
            struct sth_texture* texture = NULL;
            unsigned int state = 0;
            struct sth_quad q;
            short isize = (short)(size*10.0f);
            float* v;
            struct sth_font* fnt = NULL;

            if (stash == NULL)
                return;

            fnt = stash->fonts;
            while(fnt != NULL && fnt->idx != idx) fnt = fnt->next;
            if (fnt == NULL)
                return;
            if (fnt->type != BMFONT && !fnt->data)
                return;

            for (; *s; ++s)
            {
                if (decutf8(&state, &codepoint, *(unsigned char*)s))
                    continue;
                glyph = get_glyph(stash, fnt, codepoint, isize);
                if (!glyph)
                    continue;
                texture = glyph->texture;
                if (texture->nverts+4 >= VERT_COUNT)
                    flush_draw(stash);

                if (!get_quad(stash, fnt, glyph, isize, &x, &y, &q))
                    continue;

                v = &texture->verts[texture->nverts*4];

                v = setv(v, q.x0, q.y0, q.s0, q.t0);
                v = setv(v, q.x1, q.y0, q.s1, q.t0);
                v = setv(v, q.x1, q.y1, q.s1, q.t1);
                v = setv(v, q.x0, q.y1, q.s0, q.t1);

                texture->nverts += 4;
            }

            if (dx) *dx = x;
        }

        void sth_dim_text(struct sth_stash* stash,
                          int idx, float size,
                          const char* s,
                          float* minx, float* miny, float* maxx, float* maxy)
        {
            unsigned int codepoint;
            struct sth_glyph* glyph = NULL;
            unsigned int state = 0;
            struct sth_quad q;
            short isize = (short)(size*10.0f);
            struct sth_font* fnt = NULL;
            float x = 0, y = 0;

            *minx = *maxx = *miny = *maxy = 0;	/* @rlyeh: reset vars before failing */

            if (stash == NULL)
                return;
            fnt = stash->fonts;
            while(fnt != NULL && fnt->idx != idx)
                fnt = fnt->next;
            if (fnt == NULL)
                return;
            if (fnt->type != BMFONT && !fnt->data)
                return;

            for (; *s; ++s)
            {
                if (decutf8(&state, &codepoint, *(unsigned char*)s))
                    continue;
                glyph = get_glyph(stash, fnt, codepoint, isize);
                if (!glyph)
                    continue;
                if (!get_quad(stash, fnt, glyph, isize, &x, &y, &q))
                    continue;
                if (q.x0 < *minx) *minx = q.x0;
                if (q.x1 > *maxx) *maxx = q.x1;
                if (q.y1 < *miny) *miny = q.y1;
                if (q.y0 > *maxy) *maxy = q.y0;
            }
            if (floorf(x) > *maxx) *maxx = floorf(x);
        }

        void sth_vmetrics(struct sth_stash* stash,
                          int idx, float size,
                          float* ascender, float* descender, float* lineh)
        {
            struct sth_font* fnt = NULL;

            if (stash == NULL)
                return;
            fnt = stash->fonts;
            while(fnt != NULL && fnt->idx != idx) fnt = fnt->next;
            if (fnt == NULL)
                return;
            if (fnt->type != BMFONT && !fnt->data)
                return;
            if (ascender)
                *ascender = fnt->ascender*size;
            if (descender)
                *descender = fnt->descender*size;
            if (lineh)
                *lineh = fnt->lineh*size;
        }

        void sth_delete(struct sth_stash* stash)
        {
            struct sth_texture* tex = NULL;
            struct sth_texture* curtex = NULL;
            struct sth_font* fnt = NULL;
            struct sth_font* curfnt = NULL;

            if (!stash)
                return;

            tex = stash->tt_textures;
            while(tex != NULL) {
                curtex = tex;
                tex = tex->next;
                if (curtex->id)
                    glDeleteTextures(1, &curtex->id);
                free(curtex);
            }

            tex = stash->bm_textures;
            while(tex != NULL) {
                curtex = tex;
                tex = tex->next;
                if (curtex->id)
                    glDeleteTextures(1, &curtex->id);
                free(curtex);
            }

            fnt = stash->fonts;
            while(fnt != NULL) {
                curfnt = fnt;
                fnt = fnt->next;
                if (curfnt->glyphs)
                    free(curfnt->glyphs);
                if (curfnt->type == TTFONT_FILE && curfnt->data)
                    free(curfnt->data);
                free(curfnt);
            }
            free(stash->empty_data);
            free(stash);
        }
    }
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Here starts the implementation of OpenGLText

#include <easy3d/util/logging.h>


namespace easy3d {


    OpenGLText::OpenGLText(int texture_size, bool mipmaps) {
        texture_size_ = geom::next_pow2(texture_size);
        stash_ = details::sth_create(texture_size_, texture_size_, mipmaps);
    }

    OpenGLText::~OpenGLText() {
        if (stash_ != nullptr)
            sth_delete(stash_);
    }

    bool OpenGLText::add_font(const std::string &font_file) {
        if (stash_ == nullptr) {
            LOG(ERROR) << "addFont() error: font stash_ not initialized, call setup first";
            return false;
        }

        const int id = sth_add_font(stash_, font_file.c_str());
        if (id <= 0) {
            LOG(ERROR) << "could not load font: " << font_file;
            return false;
        }

        font_ids_.push_back(id);
        LOG(INFO) << "loaded font '" << font_file << "' in texture (" << texture_size_ << " x " << texture_size_ << ")";
        return true;
    }


    float OpenGLText::draw(const std::string &text, float x, float y, float font_size, int fontID) const {
        float dx = 0.0f;
        if (stash_ != nullptr) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            sth_begin_draw(stash_);
            sth_draw_text(stash_, font_ids_[fontID], font_size, x, y, text.c_str(), &dx); //this might draw
            sth_end_draw(stash_); // this actually draws
            glDisable(GL_BLEND);
        } else {
            LOG(ERROR) << "couldn't draw() due to the failure in initialization";
        }
        return dx;
    }


    float OpenGLText::font_height(float font_size) const {
        float asc, desc, lineh;
        sth_vmetrics(stash_, font_ids_[0], font_size, &asc, &desc, &lineh);
        return asc - desc;
    }

}