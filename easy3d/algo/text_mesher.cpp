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


#include <easy3d/algo/text_mesher.h>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/manifold_builder.h>
#include <easy3d/algo/tessellator.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/string.h>
#include <easy3d/util/progress.h>


namespace easy3d {

    namespace details {

        /**
         * [Liangliang(20200506)]:
         * The font vectorization is done using FTGL. I found FTGL is quite buggy. I fixed a few of them, modified and
         * enhanced it. To keep minimum dependence, I put the modified code here.
         */
        /*
         * FTGL - OpenGL font library
         * Copyright (c) 2020 Liangliang Nan <liangliang.nan@gmail.com>
         * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
         * Copyright (c) 2008 Éric Beets <ericbeets@free.fr>
         * Copyright (c) 2008 Sam Hocevar <sam@zoy.org>
         *
         * Permission is hereby granted, free of charge, to any person obtaining
         * a copy of this software and associated documentation files (the
         * "Software"), to deal in the Software without restriction, including
         * without limitation the rights to use, copy, modify, merge, publish,
         * distribute, sublicense, and/or sell copies of the Software, and to
         * permit persons to whom the Software is furnished to do so, subject to
         * the following conditions:
         *
         * The above copyright notice and this permission notice shall be
         * included in all copies or substantial portions of the Software.
         *
         * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
         * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
         * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
         * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
         * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
         * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
         * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
         */

        static vec2 to_vec2(const FT_Vector &v) {
            return vec2(v.x, v.y);
        }

        /**
         * Contour class is a container of points that describe a vector font
         * outline. It is used as a container for the output of the bezier curve
         * evaluator in Vectoriser.
         *
         * @see OutlineGlyph
         * @see PolygonGlyph
         * @see vec2
         */

        class Contour {
        public:
            /**
             * Constructor
             *
             * @param contour
             * @param pointTags
             * @param numberOfPoints
             */
            Contour(FT_Vector *contour, char *tags, unsigned int n, unsigned short bezierSteps) {
                vec2 prev, cur = to_vec2(contour[(n - 1) % n]), next = to_vec2(contour[0]);
                vec2 a, b = next - cur;
                double olddir, dir = atan2((next - cur).y, (next - cur).x);
                double angle = 0.0;

                // See http://freetype.sourceforge.net/freetype2/docs/glyphs/glyphs-6.html
                // for a full description of FreeType tags.
                for (unsigned int i = 0; i < n; i++) {
                    prev = cur;
                    cur = next;
                    next = to_vec2(contour[(i + 1) % n]);
                    olddir = dir;
                    dir = atan2((next - cur).y, (next - cur).x);

                    // Compute our path's new direction.
                    double t = dir - olddir;
                    if (t < -M_PI) t += 2 * M_PI;
                    if (t > M_PI) t -= 2 * M_PI;
                    angle += t;

                    // Only process point tags we know.
                    if (n < 2 || FT_CURVE_TAG(tags[i]) == FT_Curve_Tag_On) {
                        AddPoint(cur);
                    } else if (FT_CURVE_TAG(tags[i]) == FT_Curve_Tag_Conic) {
                        vec2 prev2 = prev, next2 = next;

                        // Previous point is either the real previous point (an "on"
                        // point), or the midpoint between the current one and the
                        // previous "conic off" point.
                        if (FT_CURVE_TAG(tags[(i - 1 + n) % n]) == FT_Curve_Tag_Conic) {
                            prev2 = (cur + prev) * 0.5;
                            AddPoint(prev2);
                        }

                        // Next point is either the real next point or the midpoint.
                        if (FT_CURVE_TAG(tags[(i + 1) % n]) == FT_Curve_Tag_Conic) {
                            next2 = (cur + next) * 0.5;
                        }

                        evaluateQuadraticCurve(prev2, cur, next2, bezierSteps);
                    } else if (FT_CURVE_TAG(tags[i]) == FT_Curve_Tag_Cubic
                               && FT_CURVE_TAG(tags[(i + 1) % n]) == FT_Curve_Tag_Cubic) {
                        evaluateCubicCurve(prev, cur, next, to_vec2(contour[(i + 2) % n]), bezierSteps);
                    }
                }

                // If final angle is positive (+2PI), it's an anti-clockwise contour,
                // otherwise (-2PI) it's clockwise.
                clockwise = (angle < 0.0);
            }


            /**
             * Destructor
             */
            ~Contour() {
                pointList.clear();
                outsetPointList.clear();
                frontPointList.clear();
                backPointList.clear();
            }

            /**
             * Return a point at index.
             *
             * @param index of the point in the curve.
             * @return const point reference
             */
            const vec2 &GetPoint(size_t index) const { return pointList[index]; }

            /**
             * How many points define this contour
             *
             * @return the number of points in this contour
             */
            size_t PointCount() const { return pointList.size(); }

            /**
             * Make sure the glyph has the proper parity and create the front/back
             * outset contour.
             *
             * @param parity  The contour's parity within the glyph.
             */
            void SetParity(int parity) {
                size_t size = PointCount();
                vec2 vOutset;

                if (((parity & 1) && clockwise) || (!(parity & 1) && !clockwise)) {
                    // Contour orientation is wrong! We must reverse all points.
                    // FIXME: could it be worth writing FTVector::reverse() for this?
                    for (size_t i = 0; i < size / 2; i++) {
                        vec2 tmp = pointList[i];
                        pointList[i] = pointList[size - 1 - i];
                        pointList[size - 1 - i] = tmp;
                    }

                    clockwise = !clockwise;
                }

                for (size_t i = 0; i < size; i++) {
                    size_t prev, cur, next;

                    prev = (i + size - 1) % size;
                    cur = i;
                    next = (i + size + 1) % size;

                    vOutset = ComputeOutsetPoint(GetPoint(prev), GetPoint(cur), GetPoint(next));
                    outsetPointList.push_back(vOutset);
                }
            }


        private:
            /**
             * Add a point to this contour. This function tests for duplicate
             * points.
             *
             * @param point The point to be added to the contour.
             */
            void AddPoint(vec2 point) {
                if (pointList.empty() || (point != pointList[pointList.size() - 1]
                                          && point != pointList[0])) {
                    pointList.push_back(point);
                }
            }

            /**
             * De Casteljau (bezier) algorithm contributed by Jed Soane
             * Evaluates a quadratic or conic (second degree) curve
             */
            void evaluateQuadraticCurve(vec2 A, vec2 B, vec2 C, unsigned short bezierSteps) {
                for (unsigned int i = 1; i < bezierSteps; i++) {
                    float t = static_cast<float>(i) / bezierSteps;

                    vec2 U = (1.0f - t) * A + t * B;
                    vec2 V = (1.0f - t) * B + t * C;

                    AddPoint((1.0f - t) * U + t * V);
                }
            }

            /**
             * De Casteljau (bezier) algorithm contributed by Jed Soane
             * Evaluates a cubic (third degree) curve
             */
            void evaluateCubicCurve(vec2 A, vec2 B, vec2 C, vec2 D, unsigned short bezierSteps) {
                for (unsigned int i = 0; i < bezierSteps; i++) {
                    float t = static_cast<float>(i) / bezierSteps;

                    vec2 U = (1.0f - t) * A + t * B;
                    vec2 V = (1.0f - t) * B + t * C;
                    vec2 W = (1.0f - t) * C + t * D;

                    vec2 M = (1.0f - t) * U + t * V;
                    vec2 N = (1.0f - t) * V + t * W;

                    AddPoint((1.0f - t) * M + t * N);
                }
            }

            /**
             * Compute the outset point coordinates
             */
            // This function is a bit tricky. Given a path ABC, it returns the
            // coordinates of the outset point facing B on the left at a distance
            // of 64.0.
            //                                         M
            //                            - - - - - - X
            //                             ^         / '
            //                             | 64.0   /   '
            //  X---->-----X     ==>    X--v-------X     '
            // A          B \          A          B \   .>'
            //               \                       \<'  64.0
            //                \                       \                  .
            //                 \                       \                 .
            //                C X                     C X
            //
            vec2 ComputeOutsetPoint(vec2 A, vec2 B, vec2 C) {
                /* Build the rotation matrix from 'ba' vector */
                vec2 ba = (A - B).normalize();
                vec2 bc = C - B;

                /* Rotate bc to the left */
                vec2 tmp(bc.x * -ba.x + bc.y * -ba.y,
                         bc.x * ba.y + bc.y * -ba.x);

                /* Compute the vector bisecting 'abc' */
                double norm = sqrt(tmp.x * tmp.x + tmp.y * tmp.y);
                double dist = 64.0 * sqrt((norm - tmp.x) / (norm + tmp.x));
                tmp.x = (tmp.y < 0.0 ? dist : -dist);
                tmp.y = (64.0);

                /* Rotate the new bc to the right */
                return vec2(tmp.x * -ba.x + tmp.y * ba.y,
                            tmp.x * -ba.y + tmp.y * -ba.x);
            }


            /**
             *  The list of points in this contour
             */
            typedef std::vector<vec2> PointVector;
            PointVector pointList;
            PointVector outsetPointList;
            PointVector frontPointList;
            PointVector backPointList;

            /**
             *  Is this contour clockwise or anti-clockwise?
             */
            bool clockwise;
        };


        /**
         * Vectoriser class is a helper class that converts font outlines into
         * point data.
         *
         * @see ExtrudeGlyph
         * @see OutlineGlyph
         * @see PolygonGlyph
         * @see Contour
         * @see Point
         *
         */
        class Vectoriser {
        public:
            /**
             * Constructor
             *
             * @param glyph The freetype glyph to be processed
             */
            Vectoriser(const FT_GlyphSlot glyph, unsigned short bezierSteps)
                    : contourList(0),
                      ftContourCount(0),
                      contourFlag(0) {
                if (glyph) {
                    outline = glyph->outline;

                    ftContourCount = outline.n_contours;
                    contourList = 0;
                    contourFlag = outline.flags;

                    ProcessContours(bezierSteps);
                }
            }


            /**
             *  Destructor
             */
            virtual ~Vectoriser() {
                for (size_t c = 0; c < ContourCount(); ++c) {
                    delete contourList[c];
                }

                delete[] contourList;
            }

            /**
             * Get the count of contours in this outline
             *
             * @return the number of contours
             */
            size_t ContourCount() const { return ftContourCount; }

            /**
             * Return a contour at index
             *
             * @return the number of contours
             */
            const Contour *const GetContour(size_t index) const {
                return (index < ContourCount()) ? contourList[index] : NULL;
            }


        private:
            /**
             * Process the freetype outline data into contours of points
             *
             * @param front front outset distance
             * @param back back outset distance
             */
            void ProcessContours(unsigned short bezierSteps) {
                short contourLength = 0;
                short startIndex = 0;
                short endIndex = 0;

                contourList = new Contour *[ftContourCount];

                for (int i = 0; i < ftContourCount; ++i) {
                    FT_Vector *pointList = &outline.points[startIndex];
                    char *tagList = &outline.tags[startIndex];

                    endIndex = outline.contours[i];
                    contourLength = (endIndex - startIndex) + 1;

                    Contour *contour = new Contour(pointList, tagList, contourLength, bezierSteps);

                    contourList[i] = contour;

                    startIndex = endIndex + 1;
                }

                // Compute each contour's parity. FIXME: see if FT_Outline_Get_Orientation
                // can do it for us.
                for (int i = 0; i < ftContourCount; i++) {
                    Contour *c1 = contourList[i];

                    // 1. Find the leftmost point.
                    vec2 leftmost(65536.0, 0.0);

                    for (size_t n = 0; n < c1->PointCount(); n++) {
                        vec2 p = c1->GetPoint(n);
                        if (p.x < leftmost.x) {
                            leftmost = p;
                        }
                    }

                    // 2. Count how many other contours we cross when going further to
                    // the left.
                    int parity = 0;

                    for (int j = 0; j < ftContourCount; j++) {
                        if (j == i) {
                            continue;
                        }

                        Contour *c2 = contourList[j];

                        for (size_t n = 0; n < c2->PointCount(); n++) {
                            vec2 p1 = c2->GetPoint(n);
                            vec2 p2 = c2->GetPoint((n + 1) % c2->PointCount());

                            /* FIXME: combinations of >= > <= and < do not seem stable */
                            if ((p1.y < leftmost.y && p2.y < leftmost.y)
                                || (p1.y >= leftmost.y && p2.y >= leftmost.y)
                                || (p1.x > leftmost.x && p2.x > leftmost.x)) {
                                continue;
                            } else if (p1.x < leftmost.x && p2.x < leftmost.x) {
                                parity++;
                            } else {
                                vec2 a = p1 - leftmost;
                                vec2 b = p2 - leftmost;
                                if (b.x * a.y > b.y * a.x) {
                                    parity++;
                                }
                            }
                        }
                    }

                    // 3. Make sure the glyph has the proper parity.
                    c1->SetParity(parity);
                }
            }

            /**
             * The list of contours in the glyph
             */
            Contour **contourList;

            /**
             * The number of contours reported by Freetype
             */
            short ftContourCount;

            /**
             * A flag indicating the tesselation rule for the glyph
             */
            int contourFlag;

            /**
             * A Freetype outline
             */
            FT_Outline outline;
        };


    }


#define get_face(x)      (reinterpret_cast<FT_Face>(x))
#define get_face_ptr(x)  (reinterpret_cast<FT_Face*>(&x))

#define get_library(x)      (reinterpret_cast<FT_Library>(x))
#define get_library_ptr(x)  (reinterpret_cast<FT_Library*>(&x))

// The resolution in dpi.
#define RESOLUTION          96

// Used to convert actual font size to nominal size, in 26.6 fractional points. The default value is 64.
// I choose a very large value for robustness (just in case a user requests a very small font size).
#define SCALE_TO_F26DOT6    6400


    TextMesher::TextMesher(const std::string &font_file, int font_size)
            : font_library_(nullptr), font_face_(nullptr), bezier_steps_(4), prev_char_index_(0),
              prev_rsb_delta_(0) {
        set_font(font_file, font_size);
    }


    TextMesher::~TextMesher() {
        cleanup();
    }


    void TextMesher::cleanup() {
        FT_Done_Face(get_face(font_face_));
        FT_Done_FreeType(get_library(font_library_));
        font_face_ = nullptr;
        font_library_ = nullptr;
    }


    void TextMesher::set_font(const std::string &font_file, int font_size) {
        if (!file_system::is_file(font_file)) {
            LOG(WARNING) << "font file does not exist: " << font_file;
            return;
        }

        if (font_file == font_file_ && font_size == font_size_)
            return;

        cleanup();

        if (FT_Init_FreeType(get_library_ptr(font_library_))) {
            LOG(ERROR) << "failed initializing the FreeType library.";
            ready_ = false;
            return;
        }

        if (FT_New_Face(get_library(font_library_), font_file.c_str(), 0, get_face_ptr(font_face_))) {
            LOG(ERROR) << "failed creating FreeType face (probably a problem with your font file)";
            ready_ = false;
            return;
        }

        if (FT_Set_Char_Size(get_face(font_face_), font_size * SCALE_TO_F26DOT6,
                             font_size * SCALE_TO_F26DOT6, RESOLUTION, RESOLUTION)) {
            LOG(ERROR) << "failed requesting the nominal size (in points) of the characters)";
            ready_ = false;
            return;
        }

        font_file_ = font_file;
        font_size_ = font_size;
        ready_ = true;
    }


    TextMesher::CharContour TextMesher::_generate_contours(wchar_t c, float &x, float &y) {
        CharContour char_contour;
        char_contour.character = static_cast<unsigned int>(c);

        unsigned int cur_char_index = FT_Get_Char_Index(get_face(font_face_), c);
        if (cur_char_index == 0) {
            LOG(WARNING) << "undefined character code for character " << string::to_string({c})
                         << " (your font may not support this character)";
            return char_contour;
        }

        if (FT_Load_Glyph(get_face(font_face_), cur_char_index, FT_LOAD_DEFAULT)) {
            LOG_FIRST_N(ERROR, 1) << "failed loading glyph";
            return char_contour;
        }

        FT_Glyph glyph;
        if (FT_Get_Glyph(get_face(font_face_)->glyph, &glyph)) {
            LOG_FIRST_N(ERROR, 1) << "failed getting glyph";
            return char_contour;
        }

        if (glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
            LOG_FIRST_N(ERROR, 1) << "invalid glyph Format";
            return char_contour;
        }

        if (FT_HAS_KERNING(get_face(font_face_)) && prev_char_index_) {
            FT_Vector kerning;
            FT_Get_Kerning(get_face(font_face_), prev_char_index_, cur_char_index, FT_KERNING_DEFAULT, &kerning);
            x += kerning.x / SCALE_TO_F26DOT6;
        }

        if (prev_rsb_delta_ - get_face(font_face_)->glyph->lsb_delta >= 32)
            x -= 1.0f;
        else if (prev_rsb_delta_ - get_face(font_face_)->glyph->lsb_delta < -32)
            x += 1.0f;

        prev_rsb_delta_ = get_face(font_face_)->glyph->rsb_delta;

        details::Vectoriser vectoriser(get_face(font_face_)->glyph, bezier_steps_);
        for (std::size_t c = 0; c < vectoriser.ContourCount(); ++c) {
            const details::Contour *contour = vectoriser.GetContour(c);

            Contour polygon(contour->PointCount());
            for (std::size_t p = 0; p < contour->PointCount(); ++p) {
                const float *d = contour->GetPoint(p);
                polygon[p] = vec2(d[0] / SCALE_TO_F26DOT6 + x, d[1] / SCALE_TO_F26DOT6 + y);
            }

            // ignore tiny contours (some fonts even have degenarate countours)
            if (polygon.area() >= font_size_ * font_size_ * 0.001) {
                // The FTGL library must have a bug!!!
                //polygon.clockwise = contour->GetDirection();
                polygon.clockwise = polygon.is_clockwise();

                char_contour.push_back(polygon);
            }
        }

        prev_char_index_ = cur_char_index;
        x += get_face(font_face_)->glyph->advance.x / SCALE_TO_F26DOT6;

        return char_contour;
    }


    void TextMesher::_generate_contours(const std::wstring &text, float x, float y, std::vector<CharContour> &contours) {
        if (!ready_)
            return;

        prev_char_index_ = 0;
        prev_rsb_delta_ = 0;

        for (int i = 0; i < text.size(); ++i) {
            const auto &char_contour = _generate_contours(text[i], x, y);
            // std::cout << i << ": " << string::to_string({wchar_t(char_contour.character)}) << ", to int: " << int(text[i]) << std::endl;
            if (!char_contour.empty())
                contours.push_back(char_contour);
        }
    }


    void TextMesher::generate(const std::string &text, float x, float y, std::vector<CharContour> &contours) {
        const std::wstring the_text = string::to_wstring(text);
        _generate_contours(the_text, x, y, contours);
    }


    bool TextMesher::generate(SurfaceMesh *mesh, const std::string &text, float x, float y, float extrude) {
        // The std::string class handles bytes independently of the encoding used. If used to handle sequences of
        // multi-byte or variable-length characters (such as UTF-8), all members of this class (such as length or size),
        // as well as its iterators, will still operate in terms of bytes (not actual encoded characters).
        // So I convert it to a muilti-byte character string
        const std::wstring the_text = string::to_wstring(text);
        return _generate(mesh, the_text, x, y, extrude);
    }


    bool TextMesher::_generate(SurfaceMesh *mesh, const std::wstring &text, float x, float y, float extrude) {
        if (!ready_)
            return false;

        std::vector<CharContour> all_character_contours;
        _generate_contours(text, x, y, all_character_contours);
        if (all_character_contours.empty()) {
            LOG(WARNING) << "no contour generated from the text using the specified font";
            return false;
        }

        ProgressLogger progress(text.size());
        Tessellator tess_face;
        for (auto &ch : all_character_contours) {
            //-------------------------------------------------------------------------------------
            // First, use another tessellator to generate simple contours

            Tessellator tess_contour;
            tess_contour.set_bounary_only(true);
            vec3 normal(0, 0, 1);
            tess_contour.begin_polygon(normal);
            for (std::size_t id = 0; id < ch.size(); ++id) {
                const Contour &contour = ch[id];
                tess_contour.begin_contour();
                for (const auto &p : contour)
                    tess_contour.add_vertex(vec3(p, 0.0));
                tess_contour.end_contour();
            }
            tess_contour.end_polygon();

            ch.clear();
            const auto &vertices = tess_contour.vertices();
            const auto &contours = tess_contour.elements();

            //-------------------------------------------------------------------------------------
            // Second, generate top faces

            tess_face.set_bounary_only(false);
            if (1) {
                tess_face.begin_polygon(vec3(0, 0, 1));
                for (std::size_t index = 0; index < contours.size(); ++index) {
                    Contour contour(contours[index].size());
                    tess_face.set_winding_rule(Tessellator::WINDING_ODD);
                    tess_face.begin_contour();
                    for (int j = 0; j < contours[index].size(); ++j) {
                        const int id = contours[index][j];
                        const Tessellator::Vertex *v = vertices[id];
                        const vec2 p = vec2(v->data());
                        contour[j] = p;
                        Tessellator::Vertex vt(vec3(p, extrude));
                        vt.push_back(index); // one extra bit to allow stitch within a coutour.
                        tess_face.add_vertex(vt);
                    }
                    contour.clockwise = contour.is_clockwise();
                    ch.push_back(contour);
                    tess_face.end_contour();
                }
                tess_face.end_polygon();
            }

            //-------------------------------------------------------------------------------------
            // Third, generate bottom faces
            if (1) {
                tess_face.begin_polygon(vec3(0, 0, -1));
                for (std::size_t index = 0; index < ch.size(); ++index) {
                    const Contour &contour = ch[index];
                    tess_face.set_winding_rule(Tessellator::WINDING_ODD);
                    tess_face.begin_contour();
                    for (int j = 0; j < contour.size(); ++j) {
                        Tessellator::Vertex vt(vec3(contour[j], 0));
                        vt.push_back(index); // one extra bit to allow stitch within a coutour.
                        tess_face.add_vertex(vt);
                    }
                    tess_face.end_contour();
                }
                tess_face.end_polygon();
            }


            //-------------------------------------------------------------------------------------
            if (1) { // Fourth, generate the side faces
                // generate the side faces for this character.
                // test if a contains the majority of b
                auto contains = [](const Contour &contour_a, const Contour &contour_b) -> bool {
                    int num_contained = 0;
                    for (const auto &p : contour_b) {
                        if (contour_a.contains(p))
                            ++num_contained;
                    }
                    return num_contained > (contour_b.size() - num_contained);
                };

                // how many other contours contain a contour (given it index)
                auto num_outer_contours = [&](int cur_idx, const CharContour &cha) -> int {
                    const Contour &contour = cha[cur_idx];
                    int num = 0;
                    for (int idx = 0; idx < cha.size(); ++idx) {
                        if (idx != cur_idx && contains(cha[idx], contour))
                            ++num;
                    }
                    return num;
                };

                for (std::size_t index = 0; index < contours.size(); ++index) {
                    const Contour &contour = ch[index];
                    for (int j = 0; j < contours[index].size(); ++j) {
                        int id_a = contours[index][j];
                        int id_b = contours[index][(j + 1 + contours[index].size()) % contours[index].size()];
                        const Tessellator::Vertex *va = vertices[id_a];
                        const Tessellator::Vertex *vb = vertices[id_b];
                        const vec3 a = vec3(vec2(va->data()), 0);
                        const vec3 b = vec3(vec2(vb->data()), 0);
                        const vec3 c = a + vec3(0, 0, extrude);
                        const vec3 d = b + vec3(0, 0, extrude);
                        // Though I've already known the vertex indices for the character's side triangles, I still use my
                        // tessellator, which allows me to stitch the triangles into a closed mesh.
                        if ((contour.clockwise && num_outer_contours(index, ch) % 2 == 0) ||
                            (!contour.clockwise && num_outer_contours(index, ch) % 2 == 1)) {
                            // if counter-clockwise: a -> b -> d -> c
                            vec3 n = cross(c - a, b - a);
                            n.normalize();
                            tess_face.begin_polygon(n);
                            tess_face.begin_contour();
                            // if clockwise: a -> c -> d -> b
                            Tessellator::Vertex vta(a);
                            vta.push_back(index); // one extra bit to allow stitch within a contour.
                            tess_face.add_vertex(vta);
                            Tessellator::Vertex vtc(c);
                            vtc.push_back(index); // one extra bit to allow stitch within a contour.
                            tess_face.add_vertex(vtc);
                            Tessellator::Vertex vtd(d);
                            vtd.push_back(index); // one extra bit to allow stitch within a contour.
                            tess_face.add_vertex(vtd);
                            Tessellator::Vertex vtb(b);
                            vtb.push_back(index); // one extra bit to allow stitch within a contour.
                            tess_face.add_vertex(vtb);
                            tess_face.end_contour();
                            tess_face.end_polygon();
                        } else { // if counter-clockwise: a -> b -> d -> c
                            vec3 n = cross(b - a, c - a);
                            n.normalize();
                            tess_face.begin_polygon(n);
                            tess_face.begin_contour();
                            Tessellator::Vertex vta(a);
                            vta.push_back(index); // one extra bit to allow stitch within a contour.
                            tess_face.add_vertex(vta);
                            Tessellator::Vertex vtb(b);
                            vtb.push_back(index); // one extra bit to allow stitch within a contour.
                            tess_face.add_vertex(vtb);
                            Tessellator::Vertex vtd(d);
                            vtd.push_back(index); // one extra bit to allow stitch within a contour.
                            tess_face.add_vertex(vtd);
                            Tessellator::Vertex vtc(c);
                            vtc.push_back(index); // one extra bit to allow stitch within a contour.
                            tess_face.add_vertex(vtc);
                            tess_face.end_contour();
                            tess_face.end_polygon();
                        }
                    }
                }
            }

            //-------------------------------------------------------------------------------------

            // now we can collect the triangle faces for this character

            const auto &elements = tess_face.elements();
            if (elements.empty()) {
                LOG(WARNING) << "failed generating surface for character " << string::to_string({wchar_t(ch.character)});
            }
            else {
                // the vertex index starts from 0 for each character.
                const int offset = mesh->n_vertices();

                // store the character as a vertex property.
                auto prop_char = mesh->vertex_property<unsigned int>("v:character");

                // use ManifoldBuilder (just in case there were self-intersecting contours).
                ManifoldBuilder builder(mesh);
                builder.begin_surface();

                const auto &final_vertices = tess_face.vertices();
                for (const auto v : final_vertices) {
                    auto vtx = builder.add_vertex(vec3(v->data()));
                    prop_char[vtx] = ch.character;
                }

                for (const auto &e : elements) {
                    builder.add_triangle(SurfaceMesh::Vertex(e[0] + offset),
                                         SurfaceMesh::Vertex(e[1] + offset),
                                         SurfaceMesh::Vertex(e[2] + offset));
                }

                builder.end_surface(false);
            }

            // the tessellator runs for each character
            tess_face.reset();
            progress.next();
        }

        return mesh->n_faces() > 0;
    }


    SurfaceMesh *TextMesher::generate(const std::string &text, float x, float y, float extrude) {
        if (!ready_)
            return nullptr;

        SurfaceMesh *mesh = new SurfaceMesh;
        if (generate(mesh, text, x, y, extrude))
            return mesh;
        else {
            delete mesh;
            return nullptr;
        }
    }

}