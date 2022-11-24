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


#include <easy3d/renderer/text_renderer.h>
#include <easy3d/util/string.h>


namespace easy3d {

    //  \cond
    namespace internal {

        // Copyright (c) 2020 Liangliang Nan liangliang.nan@gmail.com
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
        // The significant changes are that all the fixed pipeline rendering code has been
        // replaced by shader-based rendering.
        // The original code is available at https://github.com/armadillu/ofxFontStash

        struct sth_stash *sth_create(int cachew, int cacheh, int createMipmaps, int charPadding, float dpiScale);

        int sth_add_font(struct sth_stash *stash, const char *path);

        int sth_add_font_from_memory(struct sth_stash *stash, unsigned char *buffer);

        void sth_draw_text(struct sth_stash *stash,
                           int idx, float size,
                           float x, float y, const wchar_t *string, float *dx);

        void sth_dim_text(struct sth_stash *stash, int idx, float size, const wchar_t *string,
                          float *minx, float *miny, float *maxx, float *maxy);

        void sth_vmetrics(struct sth_stash *stash,
                          int idx, float size,
                          float *ascender, float *descender, float *lineh);

        void sth_delete(struct sth_stash *stash);

        void set_lod_bias(struct sth_stash *stash, float bias);
    }
    //  \endcond
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Here starts the implementation of fontstash

#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/opengl_error.h>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include <3rd_party/stb/stb_truetype.h>


namespace easy3d {

    //  \cond
    namespace internal {

#define HASH_LUT_SIZE 256
#define MAX_ROWS 128
#define VERT_COUNT (6*128 * 100)  /* oriol! x 100 to avoid repeated flush_draw() calls while drawing */

#define TTFONT_FILE 1
#define TTFONT_MEM  2
#define BMFONT      3

        static int font_index = 1;

        static unsigned int hashint(unsigned int a) {
            a += ~(a << 15);
            a ^= (a >> 10);
            a += (a << 3);
            a ^= (a >> 6);
            a += ~(a << 11);
            a ^= (a >> 16);
            return a;
        }


        struct sth_quad {
            float x0, y0, s0, t0;
            float x1, y1, s1, t1;
        };

        struct sth_row {
            short x, y, h;
        };

        struct sth_glyph {
            unsigned int codepoint;
            short size;
            struct sth_texture *texture;
            int x0, y0, x1, y1;
            float xadv, xoff, yoff;
            int next;
        };

        struct sth_font {
            int idx;
            int type;
            stbtt_fontinfo font;
            unsigned char *data;
            struct sth_glyph *glyphs;
            int lut[HASH_LUT_SIZE];
            int nglyphs;
            float ascender;
            float descender;
            float lineh;
            struct sth_font *next;
        };

        struct sth_texture {
            GLuint id;
            // TODO: replace rows with pointer
            struct sth_row rows[MAX_ROWS];
            int nrows;
            float verts[4 * VERT_COUNT];
            int nverts;
            struct sth_texture *next;
        };

        struct sth_stash {
            int tw, th;
            float itw, ith;
            GLubyte *empty_data;
            struct sth_texture *tt_textures;
            struct sth_texture *bm_textures;
            struct sth_font *fonts;
            int drawing;
            int padding; //oriol adding texture padding around chars to avoid mipmap neighbor leaks
            int hasMipMap; //oriol adding optional mipmap generation to each char
            int doKerning; //calc kerning on the fly and offset letters when drawing and / calcing box sizes
            float charSpacing;
            float dpiScale;
        };


        struct sth_stash *sth_create(int cachew, int cacheh, int createMipmaps, int charPadding, float dpiScale) {
            GLubyte *empty_data = nullptr;
            struct sth_texture *texture = nullptr;

            // Allocate memory for the font stash.
            auto stash = (struct sth_stash *) malloc(sizeof(struct sth_stash));
            if (stash == nullptr) goto error;
            memset(stash, 0, sizeof(struct sth_stash));

            // Create data for clearing the textures
            empty_data = (GLubyte *) malloc(cachew * cacheh);
            if (empty_data == nullptr) goto error;
            memset(empty_data, 0, cachew * cacheh);

            // Allocate memory for the first texture
            texture = (struct sth_texture *) malloc(sizeof(struct sth_texture));
            if (texture == nullptr) goto error;
            memset(texture, 0, sizeof(struct sth_texture));

            // Create first texture for the cache.
            stash->tw = cachew;
            stash->th = cacheh;
            stash->itw = 1.0f / static_cast<float>(cachew);
            stash->ith = 1.0f / static_cast<float>(cacheh);
            stash->empty_data = empty_data;
            stash->tt_textures = texture;
            stash->dpiScale = dpiScale;
            glGenTextures(1, &texture->id);
            easy3d_debug_log_gl_error
            if (!texture->id) goto error;
            glBindTexture(GL_TEXTURE_2D, texture->id);
            easy3d_debug_log_gl_error
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            easy3d_debug_log_gl_error
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            easy3d_debug_log_gl_error
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, cachew, cacheh, 0, GL_RED, GL_UNSIGNED_BYTE, empty_data);
            easy3d_debug_log_gl_error
            glBindTexture(GL_TEXTURE_2D, 0);
            easy3d_debug_log_gl_error

            stash->hasMipMap = createMipmaps;
            stash->padding = charPadding;
            stash->charSpacing = 0.0f;
            stash->doKerning = 0;
            return stash;

            error:
            if (stash != nullptr)
                free(stash);
            if (texture != nullptr)
                free(texture);
            return nullptr;
        }

        int sth_add_font_from_memory(struct sth_stash *stash, unsigned char *buffer) {
            int i, ascent, descent, fh, lineGap;
            struct sth_font *fnt = nullptr;

            fnt = (struct sth_font *) malloc(sizeof(struct sth_font));
            if (fnt == nullptr) goto error;
            memset(fnt, 0, sizeof(struct sth_font));

            // Init hash lookup.
            for (i = 0; i < HASH_LUT_SIZE; ++i) fnt->lut[i] = -1;

            fnt->data = buffer;


            // Init stb_truetype
            if (!stbtt_InitFont(&fnt->font, fnt->data, 0)) goto error;

            // Store normalized line height. The real line height is got
            // by multiplying the lineh by font size.
            stbtt_GetFontVMetrics(&fnt->font, &ascent, &descent, &lineGap);
            fh = ascent - descent;
            fnt->ascender = (float) ascent / (float) fh;
            fnt->descender = (float) descent / (float) fh;
            fnt->lineh = (float) (fh + lineGap) / (float) fh;

            fnt->idx = font_index;
            fnt->type = TTFONT_MEM;
            fnt->next = stash->fonts;
            stash->fonts = fnt;

            return font_index++;

            error:
            if (fnt) {
                if (fnt->glyphs) free(fnt->glyphs);
                free(fnt);
            }
            return 0;
        }

        int sth_add_font(struct sth_stash *stash, const char *path) {
            FILE *fp = nullptr;
            int datasize;
            unsigned char *data = nullptr;
            int idx;

            // Read in the font data.
            fp = fopen(path, "rb");
            if (!fp) goto error;
            fseek(fp, 0, SEEK_END);
            datasize = (int) ftell(fp);
            fseek(fp, 0, SEEK_SET);
            data = (unsigned char *) malloc(datasize);
            if (data == nullptr) goto error;
            if (fread(data, 1, datasize, fp) != datasize) {
                free(data);
                goto error;
            }
            fclose(fp);
            fp = nullptr;

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
            return 0;
        }

        static struct sth_glyph *
        get_glyph(struct sth_stash *stash, struct sth_font *fnt, unsigned int codepoint, short isize) {
            int i, g, advance, lsb, x0, y0, x1, y1, gw, gh;
            float scale;
            struct sth_texture *texture = nullptr;
            struct sth_glyph *glyph = nullptr;
            unsigned char *bmp = nullptr;
            unsigned int h;
            float size = static_cast<float>(isize) / 10.0f;
            int rh;
            struct sth_row *br = nullptr;

            // Find code point and size.
            h = hashint(codepoint) & (HASH_LUT_SIZE - 1);
            i = fnt->lut[h];
            while (i != -1) {
                if (fnt->glyphs[i].codepoint == codepoint && (fnt->type == BMFONT || fnt->glyphs[i].size == isize))
                    return &fnt->glyphs[i];
                i = fnt->glyphs[i].next;
            }
            // Could not find glyph.

            // For bitmap fonts: ignore this glyph.
            if (fnt->type == BMFONT) return nullptr;

            // For truetype fonts: create this glyph.
            scale = stash->dpiScale * stbtt_ScaleForPixelHeight(&fnt->font, size);
            g = stbtt_FindGlyphIndex(&fnt->font, static_cast<int>(codepoint));
            if (!g) {
//                if (codepoint != '\n')
//                    LOG_N_TIMES(3, WARNING) << "given font does not support character '" << string::from_wstring({wchar_t(codepoint)}) << "'. " << COUNTER;
//                else
//                    LOG_N_TIMES(3, WARNING) << "current implementation ignores new line character '\\n'. " << COUNTER;
                return nullptr;
            }
            stbtt_GetGlyphHMetrics(&fnt->font, g, &advance, &lsb);
            stbtt_GetGlyphBitmapBox(&fnt->font, g, scale, scale, &x0, &y0, &x1, &y1);

            gw = x1 - x0 + stash->padding;
            gh = y1 - y0 + stash->padding;

            // Check if glyph is larger than maximum texture size
            if (gw >= stash->tw || gh >= stash->th)
                return nullptr;

            // Find texture and row where the glyph can be fit.
            br = nullptr;
            rh = (gh + 7) & ~7;
            texture = stash->tt_textures;
            while (br == nullptr) {
                for (i = 0; i < texture->nrows; ++i) {
                    if (texture->rows[i].h == rh && texture->rows[i].x + gw + 1 <= stash->tw)
                        br = &texture->rows[i];
                }

                // If no row is found, there are 3 possibilities:
                //   - add new row
                //   - try next texture
                //   - create new texture
                if (br == nullptr) {
                    short py = 0;
                    // Check that there is enough space.
                    if (texture->nrows) {
                        py = texture->rows[texture->nrows - 1].y + texture->rows[texture->nrows - 1].h + 1;
                        if (py + rh > stash->th) {
                            if (texture->next != nullptr) {
                                texture = texture->next;
                            } else {
                                // Create new texture
                                texture->next = (struct sth_texture *) malloc(sizeof(struct sth_texture));
                                texture = texture->next;
                                if (texture == nullptr) goto error;
                                memset(texture, 0, sizeof(struct sth_texture));
                                //oriol counting how many we have created so far!
                                int numTex = 1;
                                struct sth_texture *tex = stash->tt_textures;
                                while (tex->next != nullptr) {
                                    numTex++;
                                    tex = tex->next;
                                }

                                VLOG(1) << "allocating a new texture of " << stash->tw << " x " << stash->th << " ("
                                        << numTex << " used so far)";
                                glGenTextures(1, &texture->id);
                                if (!texture->id) goto error;
                                glBindTexture(GL_TEXTURE_2D, texture->id);
                                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, stash->tw, stash->th, 0, GL_RED,
                                             GL_UNSIGNED_BYTE, stash->empty_data);
                                easy3d_debug_log_gl_error
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                                easy3d_debug_log_gl_error
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                                easy3d_debug_log_gl_error
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
            fnt->glyphs = (struct sth_glyph *) realloc(fnt->glyphs, fnt->nglyphs *
                                                                    sizeof(struct sth_glyph)); /* @rlyeh: explicit cast needed in C++ */
            if (!fnt->glyphs) return nullptr;

            // Init glyph.
            glyph = &fnt->glyphs[fnt->nglyphs - 1];
            memset(glyph, 0, sizeof(struct sth_glyph));
            glyph->codepoint = codepoint;
            glyph->size = isize;
            glyph->texture = texture;
            glyph->x0 = br->x;
            glyph->y0 = br->y;
            glyph->x1 = glyph->x0 + gw;
            glyph->y1 = glyph->y0 + gh;
            glyph->xadv = scale * static_cast<float>(advance);
            glyph->xoff = static_cast<float>(x0);
            glyph->yoff = static_cast<float>(y0);
            glyph->next = 0;

            // Advance row location.
            br->x += gw + 1;

            // Insert char to hash lookup.
            glyph->next = fnt->lut[h];
            fnt->lut[h] = fnt->nglyphs - 1;

            // Rasterize
            bmp = (unsigned char *) malloc(gw * gh);
            if (bmp) {
                stbtt_MakeGlyphBitmap(&fnt->font, bmp, gw, gh, gw, scale, scale, g);
                // Update texture
                glBindTexture(GL_TEXTURE_2D, texture->id);
                easy3d_debug_log_gl_error

                // Liangliang: I need to restore the modified OpenGL state
                int align;
                glGetIntegerv(GL_UNPACK_ALIGNMENT, &align);

                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                                GL_CLAMP_TO_EDGE);    //oriol trying to get rid of halos when rotating font
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);    //
                glTexSubImage2D(GL_TEXTURE_2D, 0, glyph->x0, glyph->y0, gw, gh, GL_RED, GL_UNSIGNED_BYTE, bmp);
                if (stash->hasMipMap > 0) {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 8); //TODO check for hw support!
#if defined(__ANDROID__) || defined(TARGET_OPENGLES) || defined(TARGET_RASPBERRY_PI)
                    // OpenGLES 1.0 does not support the following.
#else
//			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.0); //shoot for sharper test
//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3); // pick mipmap level 7 or lower
//			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.0);
                    glGenerateMipmap(GL_TEXTURE_2D);
#endif
                }

                glPixelStorei(GL_UNPACK_ALIGNMENT, align);

                glBindTexture(GL_TEXTURE_2D, 0);
                easy3d_debug_log_gl_error
                free(bmp);
            }

            return glyph;

            error:
            if (texture)
                free(texture);
            return nullptr;
        }

        static int
        get_quad(struct sth_stash *stash, struct sth_font *fnt, struct sth_glyph *glyph, short isize, float *x,
                 float *y, struct sth_quad *q) {
            int rx, ry;
            float scale = 1.0f;

            if (fnt->type == BMFONT) scale = static_cast<float>(isize) / (static_cast<float>(glyph->size) * 10.0f);

            rx = static_cast<int>(floorf(*x + scale * glyph->xoff));
            ry = static_cast<int>(floorf(*y - scale * glyph->yoff));

            q->x0 = static_cast<float>(rx);
            q->y0 = static_cast<float>(ry);
            q->x1 = static_cast<float>(rx) + scale * static_cast<float>(glyph->x1 - glyph->x0);
            q->y1 = static_cast<float>(ry) - scale * static_cast<float>(glyph->y1 - glyph->y0);

            q->s0 = static_cast<float>(glyph->x0) * stash->itw;
            q->t0 = static_cast<float>(glyph->y0) * stash->ith;
            q->s1 = static_cast<float>(glyph->x1) * stash->itw;
            q->t1 = static_cast<float>(glyph->y1) * stash->ith;

            *x += scale * glyph->xadv;

            return 1;
        }

        static float *setv(float *v, float x, float y, float s, float t) {
            v[0] = x;
            v[1] = y;
            v[2] = s;
            v[3] = t;
            return v + 4;
        }


        void set_lod_bias(struct sth_stash *stash, float bias) {

            struct sth_texture *texture = stash->tt_textures;
            if (stash->hasMipMap > 0) {
                while (texture) {
                    glBindTexture(GL_TEXTURE_2D, texture->id);
#ifndef TARGET_OPENGLES
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, bias);
#endif
                    glBindTexture(GL_TEXTURE_2D, 0);
                    texture = texture->next;
                }
            }
        }


        void sth_draw_text(struct sth_stash *stash,
                           int idx, float size,
                           float x, float y,
                           const wchar_t *s, float *dx) {
            unsigned int codepoint;
            struct sth_glyph *glyph = nullptr;
            struct sth_texture *texture = nullptr;
            struct sth_quad q{};
            auto isize = static_cast<short>(size * 10.0f);
            float *v;
            struct sth_font *fnt = nullptr;

            if (stash == nullptr) return;

            fnt = stash->fonts;
            while (fnt != nullptr && fnt->idx != idx) fnt = fnt->next;
            if (fnt == nullptr) return;
            if (fnt->type != BMFONT && !fnt->data) return;

            int len = static_cast<int>(wcslen(s));
            float scale = stbtt_ScaleForPixelHeight(&fnt->font, size);
            int c = 0;
            float spacing = stash->charSpacing;
            int doKerning = stash->doKerning;
            float dpiScale = stash->dpiScale;

            for (; *s; ++s) {
                codepoint = static_cast<unsigned int>(*s);

                glyph = get_glyph(stash, fnt, codepoint, isize);
                if (!glyph) continue;
                texture = glyph->texture;

                if (!get_quad(stash, fnt, glyph, isize, &x, &y, &q)) continue;

                int diff = 0;
                if (c < len && doKerning > 0) {
                    diff = stbtt_GetCodepointKernAdvance(&fnt->font, *(s), *(s + 1));
                    //printf("diff '%c' '%c' = %d\n", *(s-1), *s, diff);
                    x += static_cast<float>(diff) * scale;
                }
                x += dpiScale * spacing;

                v = &texture->verts[texture->nverts * 4];

                v = setv(v, q.x0, q.y0, q.s0, q.t0);
                v = setv(v, q.x1, q.y0, q.s1, q.t0);
                v = setv(v, q.x1, q.y1, q.s1, q.t1);
                v = setv(v, q.x0, q.y1, q.s0, q.t1);

                texture->nverts += 4;
                c++;
            }

            if (dx) *dx = x / dpiScale;
        }

        void sth_dim_text(struct sth_stash *stash,
                          int idx, float size,
                          const wchar_t *s,
                          float *minx, float *miny, float *maxx, float *maxy) {
            unsigned int codepoint;
            struct sth_glyph *glyph = nullptr;
            struct sth_quad q{};
            auto isize = static_cast<short>(size * 10.0f);
            struct sth_font *fnt = nullptr;
            float x = 0, y = 0;

            *minx = *maxx = *miny = *maxy = 0;    /* @rlyeh: reset vars before failing */

            if (stash == nullptr) return;
            fnt = stash->fonts;
            while (fnt != nullptr && fnt->idx != idx) fnt = fnt->next;
            if (fnt == nullptr) return;
            if (fnt->type != BMFONT && !fnt->data) return;

            int len = static_cast<int>(wcslen(s));
            float scale = stbtt_ScaleForPixelHeight(&fnt->font, size);
            int c = 0;
            float spacing = stash->charSpacing;
            int doKerning = stash->doKerning;

            for (; *s; ++s) {
                codepoint = static_cast<unsigned int>(*s);
                glyph = get_glyph(stash, fnt, codepoint, isize);
                if (!glyph) continue;
                if (!get_quad(stash, fnt, glyph, isize, &x, &y, &q)) continue;

                int diff = 0;
                if (c < len && doKerning > 0) {
                    diff = stbtt_GetCodepointKernAdvance(&fnt->font, *(s), *(s + 1));
                    //printf("diff '%c' '%c' = %d\n", *(s-1), *s, diff);
                    x += static_cast<float>(diff) * scale;
                }
                x += spacing;

                if (q.x0 < *minx) *minx = q.x0;
                if (q.x1 > *maxx) *maxx = q.x1;
                if (q.y1 < *miny) *miny = q.y1;
                if (q.y0 > *maxy) *maxy = q.y0;
                c++;
            }
            if (x > *maxx) *maxx = x;
        }

        void sth_vmetrics(struct sth_stash *stash,
                          int idx, float size,
                          float *ascender, float *descender, float *lineh) {
            struct sth_font *fnt = nullptr;

            if (stash == nullptr) return;
            fnt = stash->fonts;
            while (fnt != nullptr && fnt->idx != idx) fnt = fnt->next;
            if (fnt == nullptr) return;
            if (fnt->type != BMFONT && !fnt->data) return;
            if (ascender)
                *ascender = fnt->ascender * size;
            if (descender)
                *descender = fnt->descender * size;
            if (lineh)
                *lineh = fnt->lineh * size;
        }

        void sth_delete(struct sth_stash *stash) {
            struct sth_texture *tex = nullptr;
            struct sth_texture *curtex = nullptr;
            struct sth_font *fnt = nullptr;
            struct sth_font *curfnt = nullptr;

            if (!stash) return;

            tex = stash->tt_textures;
            while (tex != nullptr) {
                curtex = tex;
                tex = tex->next;
                if (curtex->id)
                    glDeleteTextures(1, &curtex->id);
                free(curtex);
            }

            tex = stash->bm_textures;
            while (tex != nullptr) {
                curtex = tex;
                tex = tex->next;
                if (curtex->id)
                    glDeleteTextures(1, &curtex->id);
                free(curtex);
            }

            fnt = stash->fonts;
            while (fnt != nullptr) {
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
    //  \endcond
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Here starts the implementation of TextRenderer

#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/vertex_array_object.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>


#define get_stash(x) (reinterpret_cast<internal::sth_stash *>(x))


namespace easy3d {


    TextRenderer::TextRenderer(float dpi_scale, int texture_size, bool mipmaps) {
        texture_size_ = next_pow2(texture_size);
        stash_ = internal::sth_create(texture_size_, texture_size_, mipmaps, 0, dpi_scale);
        easy3d_log_gl_error
        if (stash_ == nullptr) {
            LOG(ERROR) << "construction of TextRenderer failed";
        } else {
            get_stash(stash_)->doKerning = 0; //kerning disabled by default
            get_stash(stash_)->charSpacing = 0.0; //spacing neutral by default
        }
    }


    TextRenderer::~TextRenderer() {
        sth_delete(get_stash(stash_));
    }


    bool TextRenderer::add_font(const std::string &font_file) {
        if (stash_ == nullptr) {
            LOG(ERROR) << "construction of TextRenderer failed";
            return false;
        }

        const int id = sth_add_font(get_stash(stash_), font_file.c_str());
        if (id <= 0) {
            LOG(ERROR) << "could not load font: " << font_file;
            return false;
        }

        font_ids_.push_back(id);
        const std::string simple_name = file_system::simple_name(font_file);
        font_names_.push_back(simple_name);
        VLOG(1) << "loaded font '" << simple_name << "' in texture (" << texture_size_ << " x " << texture_size_ << ")";
        return true;
    }


    void TextRenderer::set_character_spacing(float spacing) {
        if (stash_)
            get_stash(stash_)->charSpacing = spacing;
    }


    float TextRenderer::character_spacing() const {
        if (stash_)
            return get_stash(stash_)->charSpacing;
        else
            return 0.0f;
    }


    void TextRenderer::set_kerning(bool kerning) {
        if (stash_)
            get_stash(stash_)->doKerning = kerning;
    }


    bool TextRenderer::kerning() const {
        if (stash_)
            return get_stash(stash_)->doKerning;
        else
            return false;
    }


    float TextRenderer::font_height(float font_size) const {
        if (font_ids_.empty()) {
            LOG_N_TIMES(3, ERROR) << "no font exists. To add a font, please call add_font(). " << COUNTER;
            return 0.0f;
        }
        float asc, desc, lineh;
        sth_vmetrics(get_stash(stash_), font_ids_[0], font_size, &asc, &desc, &lineh);
        return asc - desc;
    }


    float TextRenderer::string_width(const std::string& str, float font_size) const {
        const Rect& rect = _get_bbox(str, font_size, 0, 0, ALIGN_LEFT, 0);
        return rect.width();
    }


    float TextRenderer::string_height(const std::string& str, float font_size) const {
        const Rect& rect = _get_bbox(str, font_size, 0, 0, ALIGN_LEFT, 0);
        return rect.height();
    }


    Rect TextRenderer::string_bounding_rect(const std::string& str, float x, float y, float font_size) const {
        return _get_bbox(str, font_size, 0, 0, ALIGN_LEFT, 0);
    }


    float
    TextRenderer::draw(const std::string &text, float x, float y, float font_size, int font_id, const vec3 &font_color,
                     bool upper_left) const {
        float end_x = 0.0f;
        if (!stash_) {
            LOG_N_TIMES(3, ERROR) << "couldn't draw() due to the failure in initialization. " << COUNTER;
            return end_x;
        }
        if (font_id >= font_ids_.size()) {
            if (font_ids_.empty()) {
                LOG_N_TIMES(3, ERROR) << "no font exists. To add a font, please call add_font()";
            } else {
                LOG_N_TIMES(3, ERROR) << "font (ID: " << font_id << ") does not exist. " << COUNTER;
            }
            return end_x;
        }

        if (upper_left) { // upper_left corner is the origin
            int viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);
            const auto h = static_cast<float>(viewport[3]);
            y = h - y - 1 - font_height(font_size);
        }

        std::wstring the_text = string::to_wstring(text);
        // compute all necessary vertex/texture coordinates
        sth_draw_text(get_stash(stash_), font_ids_[font_id], font_size, x, y, the_text.c_str(), &end_x);
        easy3d_debug_log_gl_error

        // the actual rendering
        flush_draw(font_color);
        easy3d_debug_log_gl_error

        return end_x;
    }


    void TextRenderer::flush_draw(const vec3 &font_color) const {
        const std::string name = "text/text";
        auto program = ShaderManager::get_program(name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                    ShaderProgram::Attribute(ShaderProgram::POSITION, "coords")};
            program = ShaderManager::create_program_from_files(name, attributes);
        }
        if (!program) {
            LOG_N_TIMES(3, ERROR) << "shader doesn't exist: " << name << ". " << COUNTER;
            return;
        }

        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        const int w = viewport[2];
        const int h = viewport[3];

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        program->bind();

        struct internal::sth_texture *texture = get_stash(stash_)->tt_textures;
        short tt = 1;
        while (texture) {
            if (texture->nverts > 0) {
                // each vec4 represent x, y, u, and v
                std::vector<vec4> vertices(texture->nverts);
                for (int i = 0; i < texture->nverts; ++i) {
                    vertices[i] = vec4(texture->verts + i * 4);
                    vertices[i].x = 2.0f * vertices[i].x / static_cast<float>(w) - 1.0f;
                    vertices[i].y = 2.0f * vertices[i].y / static_cast<float>(h) - 1.0f;
                }

                std::vector<unsigned int> indices(texture->nverts / 4 * 6);
                for (int j = 0; j < texture->nverts / 4; ++j) {
                    indices[j * 6 + 0] = j * 4;
                    indices[j * 6 + 1] = j * 4 + 1;
                    indices[j * 6 + 2] = j * 4 + 2;
                    indices[j * 6 + 3] = j * 4;
                    indices[j * 6 + 4] = j * 4 + 2;
                    indices[j * 6 + 5] = j * 4 + 3;
                }

                unsigned int vertex_buffer = 0, element_buffer = 0;
                VertexArrayObject vao;
                vao.create_array_buffer(vertex_buffer, ShaderProgram::POSITION, vertices.data(),
                                        vertices.size() * sizeof(vec4), 4, true);
                vao.create_element_buffer(element_buffer, indices.data(), indices.size() * sizeof(unsigned int), true);

                program->bind_texture("textureID", texture->id, 0)->set_uniform("font_color", font_color);
                vao.bind();
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
                glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, nullptr);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                vao.release();
                program->release_texture();

                VertexArrayObject::release_buffer(vertex_buffer);
                VertexArrayObject::release_buffer(element_buffer);

                texture->nverts = 0;
            }
            texture = texture->next;
            if (!texture && tt) {
                texture = get_stash(stash_)->bm_textures;
                tt = 0;
            }
        }
        program->release();
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }


#define FONT_STASH_LINE_HEIGHT_MULT    0.9f

    Rect TextRenderer::draw(const std::string &text, float x0, float y0, float font_size, Align align,
                          int font_id, const vec3 &font_color, float line_spacing, bool upper_left) const {
        Rect rect(0.0f, 0.0f, 0.0f, 0.0f);
        if (!stash_) {
            LOG_N_TIMES(3, ERROR) << "couldn't draw() due to the failure in initialization. " << COUNTER;
            return rect;
        }

        if (font_id >= font_ids_.size()) {
            if (font_ids_.empty()) {
                LOG_N_TIMES(3, ERROR) << "no font exists. To add a font, please call add_font()";
            } else {
                LOG_N_TIMES(3, ERROR) << "font (ID: " << font_id << ") does not exist. " << COUNTER;
            }
            return rect;
        }

        // get height of the viewport
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        const int h = viewport[3];

        std::stringstream ss(text);
        std::string s;
        int line = 0;
        std::vector<std::string> lines;
        std::vector<float> widths;
        std::vector<float> ys;
        float maxW = 0.0f;

        const float lineHeight = 1.0f + line_spacing; // as percent, 1.0 would be normal

        while (getline(ss, s, '\n')) {
            lines.push_back(s);
            float yy = font_size * lineHeight * FONT_STASH_LINE_HEIGHT_MULT * static_cast<float>(line) * get_stash(stash_)->dpiScale;
            ys.push_back(yy);
            const Rect& dim = _get_bbox(s, font_size, x0, y0 + yy / get_stash(stash_)->dpiScale, ALIGN_LEFT, line_spacing);

            if (line == 0) {
                rect = dim;
            } else {
                rect = Rect(std::min(rect.x_min(), dim.x_min()),
                            std::max(rect.x_max(), dim.x_max()),
                            std::min(rect.y_min(), dim.y_min()),
                            std::max(rect.y_max(), dim.y_max()));
            }
            widths.push_back(dim.width());
            maxW = std::max(maxW, dim.width());

            line++;
        }

        // compute all necessary vertex/texture coordinates
        {
            float minDiffX = FLT_MAX;
            for (std::size_t i = 0; i < lines.size(); i++) {
                float dx = 0;
                float x = 0;
                switch (align) {
                    case ALIGN_LEFT:
                        break;
                    case ALIGN_RIGHT:
                        x = maxW - widths[i];
                        break;
                    case ALIGN_CENTER:
                        x = (maxW - widths[i]) * 0.5f;
                        break;
                    default:
                        break;
                }
                if (minDiffX > x) minDiffX = x;

                std::wstring the_text = string::to_wstring(lines[i]);
                sth_draw_text(get_stash(stash_),
                              font_ids_[font_id],
                              font_size,
                              x0 + x * get_stash(stash_)->dpiScale,
                              upper_left ? (static_cast<float>(h) - ys[i] - 1 - font_height(font_size) - y0) : (ys[i] + y0),
                              the_text.c_str(),
                              &dx
                );
            }
            rect.x() += minDiffX;
        }

        // the actual rendering
        flush_draw(font_color);
        easy3d_debug_log_gl_error

        return rect;
    }


    Rect TextRenderer::_get_bbox(const std::string &text, float font_size, float xx, float yy, Align align,
                              float line_spacing) const {
        const float lineHeight = 1.0f + line_spacing; // as percent, 1.0 would be normal

        std::stringstream ss(text);
        std::string s;
        int line = 0;
        std::vector<Rect> rects;

        Rect totalArea(0.0f, 0.0f, 0.0f, 0.0f);
        while (getline(ss, s, '\n')) {
            float w, h, x, y;
            std::wstring the_text = string::to_wstring(s);
            sth_dim_text(get_stash(stash_), font_ids_[0], font_size / get_stash(stash_)->dpiScale, the_text.c_str(), &x, &y, &w, &h);

            totalArea.x() = x + xx;
            totalArea.y() = yy + y;
            w = std::fabs(w - x);
            h = std::fabs(y - h);
            if (w > totalArea.width()) totalArea.x_max() = totalArea.x() + w;
            if (h > totalArea.height()) totalArea.y_max() = totalArea.y() + h;
            Rect r2 = totalArea;
            r2.y() -= r2.height();
            r2.y() += ((font_size * lineHeight)) * FONT_STASH_LINE_HEIGHT_MULT * static_cast<float>(line);
            rects.push_back(r2);

            ++line;
        }

        if (line > 1)  //if multiline
            totalArea.y() -= rects[0].height();
        else
            totalArea.y() -= totalArea.height();

        if (align != ALIGN_LEFT) {
            if (align == ALIGN_RIGHT)
                totalArea.x() -= totalArea.width();
            else
                totalArea.x() -= totalArea.width() * 0.5f;
        }

        return totalArea;
    }

}
