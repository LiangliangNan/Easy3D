
#include "text_mesher.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include "ftgl/Vectoriser.h"
#include "poly2tri/poly2tri.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/logging.h>


namespace easy3d {

#define get(x)      (reinterpret_cast<FT_Face>(x))
#define get_ptr(x)  (reinterpret_cast<FT_Face*>(&x))

// The resolution in dpi.
#define RESOLUTION          96

// Used to convert actual font size to nominal size, in 26.6 fractional points.
#define SCALE_TO_F26DOT6    64


    TextMesher::TextMesher(const std::string &font_file, int font_height, unsigned short bezier_steps)
            : bezier_steps_(bezier_steps), prev_char_index_(0), cur_char_index_(0), prev_rsb_delta_(0) {
        FT_Library library;
        if (FT_Init_FreeType(&library)) {
            LOG(ERROR) << "failed initializing the FreeType library.";
            ready_ = false;
            return;
        }

        face_ = new FT_FaceRec_;
        if (FT_New_Face(library, font_file.c_str(), 0, get_ptr(face_))) {
            LOG(ERROR) << "failed creating FreeType face (probably a problem with your font file)";
            ready_ = false;
            return;
        }

        if (FT_Set_Char_Size(get(face_), font_height * SCALE_TO_F26DOT6, font_height * SCALE_TO_F26DOT6, RESOLUTION,
                             RESOLUTION)) {
            LOG(ERROR) << "failed requesting the nominal size (in points) of the characters)";
            ready_ = false;
            return;
        }

        ready_ = true;
    }


    TextMesher::~TextMesher() {
        delete get(face_);
    }


    TextMesher::CharContour TextMesher::generate_contours(char ch, float& x, float& y) {
        CharContour char_contour;
        char_contour.character = ch;

        cur_char_index_ = FT_Get_Char_Index(get(face_), ch);
        if (FT_Load_Glyph(get(face_), cur_char_index_, FT_LOAD_DEFAULT)) {
            LOG_FIRST_N(ERROR, 1) << "failed loading glyph";
            return char_contour;
        }

        FT_Glyph glyph;
        if (FT_Get_Glyph(get(face_)->glyph, &glyph)) {
            LOG_FIRST_N(ERROR, 1) << "failed getting glyph";
            return char_contour;
        }

        if (glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
            LOG_FIRST_N(ERROR, 1) << "invalid glyph Format";
            return char_contour;
        }

        if (FT_HAS_KERNING(get(face_)) && prev_char_index_) {
            FT_Vector kerning;
            FT_Get_Kerning(get(face_), prev_char_index_, cur_char_index_, FT_KERNING_DEFAULT, &kerning);
            x += kerning.x / SCALE_TO_F26DOT6;
        }

        if (prev_rsb_delta_ - get(face_)->glyph->lsb_delta >= 32)
            x -= 1.0f;
        else if (prev_rsb_delta_ - get(face_)->glyph->lsb_delta < -32)
            x += 1.0f;

        prev_rsb_delta_ = get(face_)->glyph->rsb_delta;

        Vectoriser vectoriser(get(face_)->glyph, bezier_steps_);
        for (std::size_t c = 0; c < vectoriser.ContourCount(); ++c) {
            const ::Contour *contour = vectoriser.GetContour(c);

            Contour polygon(contour->PointCount());
            polygon.clockwise = contour->GetDirection();

            for (std::size_t p = 0; p < contour->PointCount(); ++p) {
                const double *d = contour->GetPoint(p);
                polygon[p] = vec2(d[0] / SCALE_TO_F26DOT6 + x, d[1] / SCALE_TO_F26DOT6 + y);
            }
            char_contour.push_back(polygon);
        }

        prev_char_index_ = cur_char_index_;
        x += get(face_)->glyph->advance.x / SCALE_TO_F26DOT6;

        return char_contour;
    }


    void TextMesher::generate_contours(const std::string &text, float x, float y, std::vector<CharContour> &contours) {
        if (!ready_)
            return;

        prev_char_index_ = 0;
        cur_char_index_ = 0;
        prev_rsb_delta_ = 0;

        for (int i = 0; i < text.size(); ++i) {
            const auto &char_contour = generate_contours(text[i], x, y);
            contours.push_back(char_contour);
        }
    }


    bool TextMesher::generate(SurfaceMesh* mesh, const std::string &text, float x, float y, float extrude) {
        if (!ready_)
            return false;

        std::vector<CharContour> characters;
        generate_contours(text, x, y, characters);

        if (characters.empty()) {
            LOG(ERROR) << "no contour generated from the text using the specified font";
            return false;
        }

        auto contour_to_p2t_format = [](const Contour &contour) -> std::vector<p2t::Point *> {
            std::vector<p2t::Point *> polyline;
            for (const auto &p : contour)
                polyline.push_back(new p2t::Point((p.x), p.y));
            return polyline;
        };

        auto is_inside = [](const Contour &contour_a, const Contour &contour_b) -> bool {
            for (const auto &p : contour_a) {
                if (!geom::point_in_polygon(p, contour_b))
                    return false;
            }
            return true;
        };

        for (const auto &ch :characters) {
            for (int index = 0; index < ch.size(); ++index) {
                const Contour &contour = ch[index];
                for (int p = 0; p < contour.size(); ++p) {
                    const vec3 a(contour[p], 0.0f);
                    const vec3 b(contour[(p + 1) % contour.size()], 0.0f);
                    const vec3 c = a + vec3(0, 0, extrude);
                    const vec3 d = b + vec3(0, 0, extrude);
                    mesh->add_triangle(mesh->add_vertex(c), mesh->add_vertex(b), mesh->add_vertex(a));
                    mesh->add_triangle(mesh->add_vertex(c), mesh->add_vertex(d), mesh->add_vertex(b));
                }

                // create faces for the bottom and top
                if (contour.clockwise) {  // according to FTGL, outer contours are clockwise (and inner ones are counter-clockwise)
                    const std::vector<p2t::Point *> &polyline = contour_to_p2t_format(contour);
                    p2t::CDT cdt(polyline);

                    for (std::size_t inner_index = 0; inner_index < ch.size(); ++inner_index) {
                        const Contour &inner_contour = ch[inner_index];
                        if (inner_index != index &&
                            inner_contour.clockwise != contour.clockwise &&
                            is_inside(inner_contour, contour)) {
                            const std::vector<p2t::Point *> &pl = contour_to_p2t_format(inner_contour);
                            cdt.AddHole(pl);
                        }
                    }

                    cdt.Triangulate();
                    const std::vector<p2t::Triangle *> &triangles = cdt.GetTriangles();
                    for (const auto &t : triangles) {
                        mesh->add_triangle( // bottom one
                                mesh->add_vertex(vec3(t->GetPoint(2)->x, t->GetPoint(2)->y, 0.0f)),
                                mesh->add_vertex(vec3(t->GetPoint(1)->x, t->GetPoint(1)->y, 0.0f)),
                                mesh->add_vertex(vec3(t->GetPoint(0)->x, t->GetPoint(0)->y, 0.0f))
                        );
                        mesh->add_triangle( // top one
                                mesh->add_vertex(vec3(t->GetPoint(0)->x, t->GetPoint(0)->y, extrude)),
                                mesh->add_vertex(vec3(t->GetPoint(1)->x, t->GetPoint(1)->y, extrude)),
                                mesh->add_vertex(vec3(t->GetPoint(2)->x, t->GetPoint(2)->y, extrude))
                        );
                    }
                }
            }
        }

        return true;
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