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


#include <easy3d/algo/text_mesher.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/curve.h>
#include <easy3d/algo/extrusion.h>
#include <easy3d/algo/tessellator.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/string.h>
#include <easy3d/util/progress.h>

#define STB_TRUETYPE_IMPLEMENTATION

#include <3rd_party/stb/stb_truetype.h>


namespace easy3d {


#define get_font(x)      (reinterpret_cast<stbtt_fontinfo*>(x))


    TextMesher::TextMesher(const std::string &font_file, unsigned short quality)
        : bezier_steps_(quality)
        , ready_(false)
    {
        font_ = new stbtt_fontinfo;
        set_font(font_file);
    }


    TextMesher::~TextMesher() {
        delete get_font(font_);
    }


    void TextMesher::set_font(const std::string &font_file) {
        if (!file_system::is_file(font_file)) {
            LOG(WARNING) << "font file does not exist: " << font_file;
            return;
        }

        if (font_file == font_file_)
            return;

        ready_ = false;
        font_data_.clear();

        auto read_font_file = [](const std::string &font_file, std::vector<unsigned char> &data) -> std::size_t {
            auto fp = fopen(font_file.c_str(), "rb");
            if (!fp)
                return 0;
            fseek(fp, 0, SEEK_END);
            auto size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            data.resize(size);
            if (fread(data.data(), 1, size, fp) != size) {
                fclose(fp);
                return 0;
            } else {
                fclose(fp);
                return size;
            }
        };

        // load font
        if (read_font_file(font_file, font_data_) == 0) {
            LOG(ERROR) << "failed loading font file: " << font_file;
            return;
        }

        int font_offset = stbtt_GetFontOffsetForIndex(font_data_.data(), 0);
        if (font_offset != 0) {
            LOG(ERROR) << "invalid font file";
            return;
        }

        if (stbtt_InitFont(get_font(font_), font_data_.data(), font_offset) == 0) {
            LOG(ERROR) << "init font (building font cache) failed";
            return;
        }

        font_file_ = font_file;
        ready_ = true;
    }


    bool
    TextMesher::_generate_contours(int codepoint, float &x, float &y, int font_size, std::vector<Polygon2> &contours) {
        int glyph_index = stbtt_FindGlyphIndex(get_font(font_), codepoint);
        if (glyph_index == 0) {
            LOG(WARNING) << "given font does not support character " << string::from_wstring({wchar_t(codepoint)});
            return false;
        }

        // a scale factor to produce a font whose "height" is 'pixels' tall
        float scale = stbtt_ScaleForPixelHeight(get_font(font_), static_cast<float>(font_size));

        std::size_t old_num = contours.size();

        stbtt_vertex *vertices = nullptr;
        const int num_verts = stbtt_GetGlyphShape(get_font(font_), glyph_index, &vertices);

        long contour_begin_index = 0;
        while (contour_begin_index < num_verts) {
            stbtt_vertex *next_contour_begin = std::find_if(
                    vertices + contour_begin_index + 1, vertices + num_verts,
                    [](const stbtt_vertex &p) {
                        return p.type == STBTT_vmove;
                    });
            const auto next_contour_first_index = std::distance(vertices, next_contour_begin);
            const auto current_contour_last_index = next_contour_first_index - 1;

            Polygon2 contour;
            for (long vi = contour_begin_index; vi < current_contour_last_index; ++vi) {
                stbtt_vertex *v1 = vertices + vi;
                stbtt_vertex *v2 = vertices + vi + 1;

                const vec2 p1 = vec2(static_cast<float>(v1->x) * scale + x, static_cast<float>(v1->y) * scale + y);
                const vec2 p2 = vec2(static_cast<float>(v2->x) * scale + x, static_cast<float>(v2->y) * scale + y);
                const vec2 pc = vec2(static_cast<float>(v2->cx) * scale + x, static_cast<float>(v2->cy) * scale + y);
                const vec2 pc1 = vec2(static_cast<float>(v2->cx1) * scale + x, static_cast<float>(v2->cy1) * scale + y);

                if (v2->type == STBTT_vline) // line
                    contour.push_back(p1);
                else if (v2->type == STBTT_vcurve)  //quadratic Bezier
                    curve::quadratic(p1, pc, p2, contour, bezier_steps_);
                else if (v2->type == STBTT_vcubic)  //cubic Bezier
                    curve::cubic(p1, pc, pc1, p2, contour, bezier_steps_);
                else
                    LOG(ERROR) << "unrecognized contour point type";
            }

#if 0
            // ignore tiny contours (some fonts even have degenerate contours)
            if (contour.area() >= font_size_ * font_size_ * 0.001)
#endif
            contours.push_back(contour);

            contour_begin_index = next_contour_first_index;
        }

        stbtt_FreeShape(get_font(font_), vertices);

        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(get_font(font_), codepoint, &advanceWidth, &leftSideBearing);
        x += static_cast<float>(advanceWidth - leftSideBearing) * scale;

//        int ascent, descent, line_gap;
//        stbtt_GetFontVMetrics(get_font(font_), &ascent, &descent, &line_gap);
//        y += (ascent - descent + line_gap) * scale;

        return contours.size() > old_num;
    }


    bool TextMesher::_generate_contours(const std::wstring &text, float x, float y, int font_size,
                                        std::vector<std::vector<Polygon2> > &results, bool collision_free) {
        if (!ready_)
            return false;

        if (collision_free) {
            std::vector<Polygon2> all_contours;
            for (const auto& c : text) {
                const int codepoint = static_cast<int>(c);
//                std::cout << i << ": " << string::from_wstring({c}) << ", codepoint: " << codepoint << std::endl;
                std::vector<Polygon2> contours;
                if (_generate_contours(codepoint, x, y, font_size, contours)) {
                    if (collision_free) {
                        // resolve intersections and determine interior/exterior for each char.
                        csg::tessellate(contours, Tessellator::WINDING_ODD);
                        all_contours.insert(all_contours.end(), contours.begin(), contours.end());
                    }
                }
            }
            // compute the union of all characters.
            csg::tessellate(all_contours, Tessellator::WINDING_NONZERO); // the union of the neighboring chars
            results.push_back(all_contours);
        } else {
            for (const auto& c : text) {
                const int codepoint = static_cast<int>(c);
//                std::cout << i << ": " << string::from_wstring({c}) << ", codepoint: " << codepoint << std::endl;
                std::vector<Polygon2> contours;
                if (_generate_contours(codepoint, x, y, font_size, contours)) {
                    // resolve intersections and determine interior/exterior for each char.
                    csg::tessellate(contours, Tessellator::WINDING_ODD);
                    results.push_back(contours);
                }
            }
        }

#if 0   // for debugging: save the contours as a graph
        Graph g;
        std::size_t offset_index = 0;
        for (const auto &contour : contours) {
            for (const auto &p : contour)
                g.add_vertex(vec3(p, 0.0));
            for (std::size_t j = 0; j < contour.size(); ++j)
                g.add_edge(Graph::Vertex(offset_index + j), Graph::Vertex(offset_index + (j + 1) % contour.size()));

            offset_index += contour.size();
        }
        GraphIO::save("contours.ply", &g);
#endif
        return !results.empty();
    }


    bool TextMesher::generate(const std::string &text, float x, float y, int font_size,
                              std::vector<std::vector<Polygon2> > &contours, bool collision_free) {
        const std::wstring the_text = string::to_wstring(text);
        return _generate_contours(the_text, x, y, font_size, contours, collision_free);
    }


    bool TextMesher::generate(SurfaceMesh *mesh, const std::string &text, float x, float y, int font_size, float height,
                              bool collision_free) {
        if (!mesh)
            return false;

        // The std::string class handles bytes independently of the encoding used. If used to handle sequences of
        // multibyte or variable-length characters (such as UTF-8), all members of this class (such as length or size),
        // as well as its iterators, will still operate in terms of bytes (not actual encoded characters).
        // So let's convert it to a multibyte character string
        const std::wstring the_text = string::to_wstring(text);
        return _generate(mesh, the_text, x, y, font_size, height, collision_free);
    }


    bool
    TextMesher::_generate(SurfaceMesh *mesh, const std::wstring &text, float x, float y, int font_size, float height,
                          bool collision_free) {
        if (!ready_)
            return false;

        std::vector<std::vector<Polygon2> > contours;
        if (!_generate_contours(text, x, y, font_size, contours, collision_free)) {
            LOG(WARNING) << "no contour generated from the text using the specified font";
            return false;
        }

        ProgressLogger progress(contours.size(), false, false);
        for (const auto &contour : contours) {
            if (progress.is_canceled()) {
                LOG(WARNING) << "generating surface mesh from text cancelled";
                return false;
            }
            extrude(mesh, contour, height);
            progress.next();
        }

        return mesh->n_faces() > 0;
    }


    SurfaceMesh *
    TextMesher::generate(const std::string &text, float x, float y, int font_size, float extrude, bool collision_free) {
        if (!ready_)
            return nullptr;

        auto mesh = new SurfaceMesh;
        if (generate(mesh, text, x, y, font_size, extrude, collision_free))
            return mesh;
        else {
            delete mesh;
            return nullptr;
        }
    }

}