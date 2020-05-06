
#include "text_mesher.h"
#include "ftgl/Vectoriser.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/manifold_builder.h>
#include <easy3d/viewer/tessellator.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>


namespace easy3d {

#define get_face(x)      (reinterpret_cast<FT_Face>(x))
#define get_face_ptr(x)  (reinterpret_cast<FT_Face*>(&x))

#define get_library(x)      (reinterpret_cast<FT_Library>(x))
#define get_library_ptr(x)  (reinterpret_cast<FT_Library*>(&x))

// The resolution in dpi.
#define RESOLUTION          96

// Used to convert actual font size to nominal size, in 26.6 fractional points. The default value is 64.
// I choose a very large value for robustness (just in case a user requests a very small font size).
#define SCALE_TO_F26DOT6    6400


    TextMesher::TextMesher(const std::string &font_file, int font_height)
            : font_library_(nullptr), font_face_(nullptr), bezier_steps_(4), prev_char_index_(0), cur_char_index_(0),
              prev_rsb_delta_(0) {
        set_font(font_file, font_height);
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


    void TextMesher::set_font(const std::string &font_file, int font_height) {
        if (!file_system::is_file(font_file)) {
            LOG(WARNING) << "font file does not exist: " << font_file;
            return;
        }

        if (font_file == font_file_ && font_height == font_height_)
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

        if (FT_Set_Char_Size(get_face(font_face_), font_height * SCALE_TO_F26DOT6,
                             font_height * SCALE_TO_F26DOT6, RESOLUTION, RESOLUTION)) {
            LOG(ERROR) << "failed requesting the nominal size (in points) of the characters)";
            ready_ = false;
            return;
        }

        font_file_ = font_file;
        font_height_ = font_height;
        ready_ = true;
    }


    TextMesher::CharContour TextMesher::generate_contours(char ch, float &x, float &y) {
        CharContour char_contour;
        char_contour.character = ch;

        cur_char_index_ = FT_Get_Char_Index(get_face(font_face_), ch);
        if (FT_Load_Glyph(get_face(font_face_), cur_char_index_, FT_LOAD_DEFAULT)) {
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
            FT_Get_Kerning(get_face(font_face_), prev_char_index_, cur_char_index_, FT_KERNING_DEFAULT, &kerning);
            x += kerning.x / SCALE_TO_F26DOT6;
        }

        if (prev_rsb_delta_ - get_face(font_face_)->glyph->lsb_delta >= 32)
            x -= 1.0f;
        else if (prev_rsb_delta_ - get_face(font_face_)->glyph->lsb_delta < -32)
            x += 1.0f;

        prev_rsb_delta_ = get_face(font_face_)->glyph->rsb_delta;

        Vectoriser vectoriser(get_face(font_face_)->glyph, bezier_steps_);
        for (std::size_t c = 0; c < vectoriser.ContourCount(); ++c) {
            const ::Contour *contour = vectoriser.GetContour(c);

            Contour polygon(contour->PointCount());
            for (std::size_t p = 0; p < contour->PointCount(); ++p) {
                const double *d = contour->GetPoint(p);
                polygon[p] = vec2(d[0] / SCALE_TO_F26DOT6 + x, d[1] / SCALE_TO_F26DOT6 + y);
            }

            // ignore tiny contours (some fonts even have degenarate countours)
            if (polygon.area() >= font_height_ * font_height_ * 0.001) {
                // The FTGL library must have a bug!!!
                //polygon.clockwise = contour->GetDirection();
                polygon.clockwise = polygon.is_clockwise();

                char_contour.push_back(polygon);
            }
        }

        prev_char_index_ = cur_char_index_;
        x += get_face(font_face_)->glyph->advance.x / SCALE_TO_F26DOT6;

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


    bool TextMesher::generate(SurfaceMesh *mesh, const std::string &text, float x, float y, float extrude) {
        if (!ready_)
            return false;

        std::vector<CharContour> characters;
        generate_contours(text, x, y, characters);

        if (characters.empty()) {
            LOG(ERROR) << "no contour generated from the text using the specified font";
            return false;
        }

        Tessellator tess_face;
        for (auto &ch : characters) {
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
                        int id = contours[index][j];
                        const Tessellator::Vertex *v = vertices[id];
                        const vec2 p = vec2(v->data());
                        contour[j] = p;
                        Tessellator::Vertex vt(vec3(p, extrude)); vt.push_back(index); // one extra bit to allow stitch within a coutour.
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
                        Tessellator::Vertex vt(vec3(contour[j], 0)); vt.push_back(index); // one extra bit to allow stitch within a coutour.
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
                            Tessellator::Vertex vta(a); vta.push_back(index); // one extra bit to allow stitch within a coutour.
                            tess_face.add_vertex(vta);
                            Tessellator::Vertex vtc(c); vtc.push_back(index); // one extra bit to allow stitch within a coutour.
                            tess_face.add_vertex(vtc);
                            Tessellator::Vertex vtd(d); vtd.push_back(index); // one extra bit to allow stitch within a coutour.
                            tess_face.add_vertex(vtd);
                            Tessellator::Vertex vtb(b); vtb.push_back(index); // one extra bit to allow stitch within a coutour.
                            tess_face.add_vertex(vtb);
                            tess_face.end_contour();
                            tess_face.end_polygon();
                        } else { // if counter-clockwise: a -> b -> d -> c
                            vec3 n = cross(b - a, c - a);
                            n.normalize();
                            tess_face.begin_polygon(n);
                            tess_face.begin_contour();
                            Tessellator::Vertex vta(a); vta.push_back(index); // one extra bit to allow stitch within a coutour.
                            tess_face.add_vertex(vta);
                            Tessellator::Vertex vtb(b); vtb.push_back(index); // one extra bit to allow stitch within a coutour.
                            tess_face.add_vertex(vtb);
                            Tessellator::Vertex vtd(d); vtd.push_back(index); // one extra bit to allow stitch within a coutour.
                            tess_face.add_vertex(vtd);
                            Tessellator::Vertex vtc(c); vtc.push_back(index); // one extra bit to allow stitch within a coutour.
                            tess_face.add_vertex(vtc);
                            tess_face.end_contour();
                            tess_face.end_polygon();
                        }
                    }
                }
            }

            //-------------------------------------------------------------------------------------

            // now we can collect the triangle faces for this character
            // the vertex index starts from 0 for each character.
            const int offset = mesh->n_vertices();

            // use ManifoldBuilder (just in case there were self-intersecting contours).
            ManifoldBuilder builder(mesh);
            builder.begin_surface();

            const auto &final_vertices = tess_face.vertices();
            for (const auto v : final_vertices) {
                builder.add_vertex(vec3(v->data()));
            }

            const auto& elements = tess_face.elements();
            for (const auto& e : elements) {
                auto va = SurfaceMesh::Vertex(e[0] + offset);
                auto vb = SurfaceMesh::Vertex(e[1] + offset);
                auto vc = SurfaceMesh::Vertex(e[2] + offset);
                builder.add_triangle(va, vb, vc);
            }

            builder.end_surface(false);

            // the tessellator runs for each character
            tess_face.reset();
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