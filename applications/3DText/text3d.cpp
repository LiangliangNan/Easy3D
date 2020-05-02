
#include "text3d.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include "ftgl/Vectoriser.h"
#include "poly2tri/poly2tri.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/logging.h>


#ifdef _WIN32
#include <Windows.h>
#else
#define CALLBACK
#endif

#  if defined(__APPLE__) && defined(__MACH__)
#    include <OpenGL/glu.h>
#  else
#    include <GL/glu.h>
#  endif



#define get(x)      (reinterpret_cast<FT_Face>(x))
#define get_ptr(x)  (reinterpret_cast<FT_Face*>(&x))


namespace easy3d {


    Text3D::Text3D(const std::string &font_file, int font_height) {
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

        if (FT_Set_Char_Size(get(face_), font_height << 6, font_height << 6, 96, 96)) {
            LOG(ERROR) << "failed requesting the nominal size (in points) of the characters)";
            ready_ = false;
            return;
        }

        ready_ = true;
    }

    Text3D::~Text3D() {
        delete get(face_);
    }


    int USE_OPENGL = 0;

    struct Vector3df
    {
        float x, y, z;
    };

    struct Tri
    {
        Vector3df a, b, c;
    };

    std::vector<Tri> tris;



    void beginCallback(GLenum which)
    {
        printf("Unknown tesselasion enum: %d\n", which);
        glBegin(GL_TRIANGLES);
    }

    void endCallback(void)
    {
        glEnd();
    }

    void flagCallback( GLboolean )
    {

    }

    void errorCallback(GLenum errorCode)
    {
        const GLubyte *estring;

        estring = gluErrorString(errorCode);
        fprintf (stderr, "Tessellation Error: %s\n", estring);
    }

    static int count = 0;
    static Tri t;

    void vertexCallback(GLvoid *vertex)
    {
        const GLdouble *d;
        d = (GLdouble *) vertex;

        if(count == 0) {
            t.a.x = d[0];
            t.a.y = d[1];
            t.a.z = d[2];
            count++;
        } else if(count == 1) {
            t.c.x = d[0];
            t.c.y = d[1];
            t.c.z = d[2];
            count++;
        } else if(count == 2) {
            t.b.x = d[0];
            t.b.y = d[1];
            t.b.z = d[2];
            tris.push_back(t);
            count = 0;
            printf("Creating triangles\n");
        }
    }

    void combineCallback(GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4], GLdouble **dataOut)
    {
        printf("combineCallback\n");
        GLdouble *vertex;
        int i;

        vertex = (GLdouble *) malloc(6 * sizeof(GLdouble));
        vertex[0] = coords[0];
        vertex[1] = coords[1];
        vertex[2] = coords[2];
        for (i = 3; i < 7; i++)
            vertex[i] = weight[0] * vertex_data[0][i] + weight[1] * vertex_data[1][i] + weight[2] * vertex_data[2][i] + weight[3] * vertex_data[3][i];
        *dataOut = vertex;
    }


    std::vector<p2t::Point*> triangulateContour(Vectoriser *vectoriser, int c, float offset) {
        std::vector<p2t::Point*> polyline;
        const Contour* contour = vectoriser->GetContour(c);
        for(size_t p = 0; p < contour->PointCount(); ++p) {
            const double* d = contour->GetPoint(p);
            polyline.push_back(new p2t::Point((d[0]/64.0f) + offset, d[1]/64.0f));
        }
        return polyline;
    }


    float AddCharacter(FT_Face face, FT_ULong ch, unsigned short bezierSteps, float offset, float extrude) {
        static FT_UInt prevCharIndex = 0, curCharIndex = 0;
        static FT_Pos prev_rsb_delta = 0;

        curCharIndex = FT_Get_Char_Index(face, ch);
        if (FT_Load_Glyph(face, curCharIndex, FT_LOAD_DEFAULT))
            printf("FT_Load_Glyph failed\n");

        FT_Glyph glyph;
        if (FT_Get_Glyph(face->glyph, &glyph))
            printf("FT_Get_Glyph failed\n");

        if (glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
            printf("Invalid Glyph Format\n");
            exit(0);
        }

        short nCountour = 0;
        nCountour = face->glyph->outline.n_contours;

        int startIndex = 0, endIndex = 0;
        FT_Outline *o = &face->glyph->outline;

        if (FT_HAS_KERNING(face) && prevCharIndex) {
            FT_Vector kerning;
            FT_Get_Kerning(face, prevCharIndex, curCharIndex, FT_KERNING_DEFAULT, &kerning);
            offset += kerning.x >> 6;
        }

        if (prev_rsb_delta - face->glyph->lsb_delta >= 32)
            offset -= 1.0f;
        else if (prev_rsb_delta - face->glyph->lsb_delta < -32)
            offset += 1.0f;

        prev_rsb_delta = face->glyph->rsb_delta;

        Vectoriser *vectoriser = new Vectoriser(face->glyph, bezierSteps);
        for (size_t c = 0; c < vectoriser->ContourCount(); ++c) {
            const Contour *contour = vectoriser->GetContour(c);

            for (size_t p = 0; p < contour->PointCount() - 1; ++p) {
                const double *d1 = contour->GetPoint(p);
                const double *d2 = contour->GetPoint(p + 1);
                Tri t1;
                t1.a.x = (d1[0] / 64.0f) + offset;
                t1.a.y = d1[1] / 64.0f;
                t1.a.z = 0.0f;
                t1.b.x = (d2[0] / 64.0f) + offset;
                t1.b.y = d2[1] / 64.0f;
                t1.b.z = 0.0f;
                t1.c.x = (d1[0] / 64.0f) + offset;
                t1.c.y = d1[1] / 64.0f;
                t1.c.z = extrude;
                tris.push_back(t1);

                Tri t2;
                t2.a.x = (d1[0] / 64.0f) + offset;
                t2.a.y = d1[1] / 64.0f;
                t2.a.z = extrude;
                t2.c.x = (d2[0] / 64.0f) + offset;
                t2.c.y = d2[1] / 64.0f;
                t2.c.z = extrude;
                t2.b.x = (d2[0] / 64.0f) + offset;
                t2.b.y = d2[1] / 64.0f;
                t2.b.z = 0.0f;

                tris.push_back(t2);
            }

            if (contour->GetDirection()) {

                if (USE_OPENGL) {
                    GLUtesselator *tobj = gluNewTess();

                    gluTessCallback(tobj, GLU_TESS_VERTEX, (GLvoid(*)()) & vertexCallback);
                    gluTessCallback(tobj, GLU_TESS_BEGIN, (GLvoid(*)()) & beginCallback);
                    gluTessCallback(tobj, GLU_TESS_END, (GLvoid(*)()) & endCallback);
                    gluTessCallback(tobj, GLU_TESS_ERROR, (GLvoid(*)()) & errorCallback);
                    gluTessCallback(tobj, GLU_TESS_COMBINE, (GLvoid(*)()) & combineCallback);
                    gluTessCallback(tobj, GLU_TESS_EDGE_FLAG, (GLvoid(*)()) & flagCallback);

                    gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);

                    gluTessProperty(tobj, GLU_TESS_TOLERANCE, 0);
                    gluTessNormal(tobj, 0.0f, 0.0f, 0.0f);

                    gluTessBeginPolygon(tobj, NULL);
                    for (size_t c = 0; c < vectoriser->ContourCount(); ++c) {
                        const Contour *contour = vectoriser->GetContour(c);
                        gluTessBeginContour(tobj);
                        for (size_t p = 0; p < contour->PointCount(); ++p) {
                            const double *d1 = contour->GetPoint(p);
                            double *d = new double[3];
                            d[0] = d1[0] / 64.0f + offset;
                            d[1] = d1[1] / 64.0f;
                            d[2] = d1[2] / 64.0f;
                            gluTessVertex(tobj, (GLdouble *) d, (GLvoid *) d);
                        }

                        gluTessEndContour(tobj);
                    }
                    gluTessEndPolygon(tobj);

                } else {
                    std::vector<p2t::Point *> polyline = triangulateContour(vectoriser, c, offset);
                    p2t::CDT *cdt = new p2t::CDT(polyline);

                    for (size_t cm = 0; cm < vectoriser->ContourCount(); ++cm) {
                        const Contour *sm = vectoriser->GetContour(cm);
                        if (c != cm && !sm->GetDirection() && sm->IsInside(contour)) {
                            std::vector<p2t::Point *> pl = triangulateContour(vectoriser, cm, offset);
                            cdt->AddHole(pl);
                        }
                    }

                    cdt->Triangulate();
                    std::vector<p2t::Triangle *> ts = cdt->GetTriangles();
                    for (int i = 0; i < ts.size(); i++) {
                        p2t::Triangle *ot = ts[i];

                        Tri t1;
                        t1.a.x = ot->GetPoint(0)->x;
                        t1.a.y = ot->GetPoint(0)->y;
                        t1.a.z = 0.0f;
                        t1.b.x = ot->GetPoint(1)->x;
                        t1.b.y = ot->GetPoint(1)->y;
                        t1.b.z = 0.0f;
                        t1.c.x = ot->GetPoint(2)->x;
                        t1.c.y = ot->GetPoint(2)->y;
                        t1.c.z = 0.0f;
                        tris.push_back(t1);

                        Tri t2;
                        t2.a.x = ot->GetPoint(1)->x;
                        t2.a.y = ot->GetPoint(1)->y;
                        t2.a.z = extrude;
                        t2.b.x = ot->GetPoint(0)->x;
                        t2.b.y = ot->GetPoint(0)->y;
                        t2.b.z = extrude;
                        t2.c.x = ot->GetPoint(2)->x;
                        t2.c.y = ot->GetPoint(2)->y;
                        t2.c.z = extrude;
                        tris.push_back(t2);
                    }
                    delete cdt;
                }
            }
        }

        delete vectoriser;
        vectoriser = NULL;

        prevCharIndex = curCharIndex;
        float chSize = face->glyph->advance.x >> 6;
        return offset + chSize;
    }


    SurfaceMesh *Text3D::generate(const std::string text, float extrude, unsigned short bezierSteps) {
        if (!ready_)
            return nullptr;

        float offset = 0;
        for (int i = 0; i < text.size(); i++) {
            offset = AddCharacter(get(face_), text[i], bezierSteps, offset, extrude);
        }

        if (tris.empty()) {
            LOG(ERROR) << "no triangles generated from the text using the specified font";
            return nullptr;
        }

        SurfaceMesh *mesh = new SurfaceMesh;
        for (int i = 0; i < tris.size(); i++) {
            const Tri &tri = tris[i];
            mesh->add_triangle(
                    mesh->add_vertex({ tri.a.x, tri.a.y, tri.a.z }),
                    mesh->add_vertex({ tri.b.x, tri.b.y, tri.b.z }),
                    mesh->add_vertex({ tri.c.x, tri.c.y, tri.c.z })
            );
        }

        return mesh;
    }
}