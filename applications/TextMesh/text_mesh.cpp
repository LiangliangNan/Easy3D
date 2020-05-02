
#include "text_mesh.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include "ftgl/Vectoriser.h"
#include "poly2tri/poly2tri.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/logging.h>


#define get(x)      (reinterpret_cast<FT_Face>(x))
#define get_ptr(x)  (reinterpret_cast<FT_Face*>(&x))


namespace easy3d {


    TextMesh::TextMesh(const std::string &font_file, int font_height, unsigned short bezier_steps)
            : bezier_steps_(bezier_steps)
            , scaling_(64)
            , prev_char_index_(0)
            , cur_char_index_(0)
            , prev_rsb_delta_(0)
    {
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

        if (FT_Set_Char_Size(get(face_), font_height * scaling_, font_height * scaling_, 96, 96)) {
            LOG(ERROR) << "failed requesting the nominal size (in points) of the characters)";
            ready_ = false;
            return;
        }

        ready_ = true;
    }

    TextMesh::~TextMesh() {
        delete get(face_);
    }


    SurfaceMesh *TextMesh::generate_mesh(const std::string &text, float extrude) {
        if (!ready_)
            return nullptr;

        std::vector<CharContour> contours;
        generate_contours(text, contours);

        if (contours.empty()) {
            LOG(ERROR) << "no contour generated from the text using the specified font";
            return nullptr;
        }

#if 0
            SurfaceMesh *mesh = new SurfaceMesh;
            for (int i = 0; i < tris.size(); i++) {
                const Tri &tri = tris[i];
                mesh->add_triangle(
                        mesh->add_vertex({tri.a.x, tri.a.y, tri.a.z}),
                        mesh->add_vertex({tri.b.x, tri.b.y, tri.b.z}),
                        mesh->add_vertex({tri.c.x, tri.c.y, tri.c.z})
                );
            }

            return mesh;
#else
        LOG(ERROR) << "failed generating a surface mesh from the text";
        return nullptr;
#endif
    }


    void TextMesh::generate_contours(const std::string &text, std::vector<CharContour> &contours) {
        if (!ready_)
            return;

        prev_char_index_ = 0;
        cur_char_index_ = 0;
        prev_rsb_delta_ = 0;

        float offset = 0;
        for (int i = 0; i < text.size(); ++i) {
            const auto& char_contour = generate_contours(text[i], offset);
            contours.push_back(char_contour);
        }
    }


    TextMesh::CharContour TextMesh::generate_contours(char ch, float& offset) {
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
            offset += kerning.x >> 6;
        }

        if (prev_rsb_delta_ - get(face_)->glyph->lsb_delta >= 32)
            offset -= 1.0f;
        else if (prev_rsb_delta_ - get(face_)->glyph->lsb_delta < -32)
            offset += 1.0f;

        prev_rsb_delta_ = get(face_)->glyph->rsb_delta;

        Vectoriser vectoriser(get(face_)->glyph, bezier_steps_);
        for (std::size_t c = 0; c < vectoriser.ContourCount(); ++c) {
            const ::Contour *contour = vectoriser.GetContour(c);

            TextMesh::Contour polygon(contour->PointCount());
            polygon.clockwise = contour->GetDirection();

            for (std::size_t p = 0; p < contour->PointCount(); ++p) {
                const double *d = contour->GetPoint(p);
                polygon[p] = vec2(d[0] / scaling_ + offset, d[1] / scaling_);
            }
            char_contour.push_back(polygon);
        }

        prev_char_index_ = cur_char_index_;
        offset += get(face_)->glyph->advance.x / scaling_;

        return char_contour;
    }

}